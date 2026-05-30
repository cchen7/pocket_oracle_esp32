#include "wifi_sta.h"

#include "../storage/settings.h"

#include "esp_log.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_netif_sntp.h"
#include "esp_wifi.h"

#include <atomic>
#include <cstring>

namespace pocket {
namespace wifi {

namespace {

constexpr const char* TAG = "WIFI";

std::atomic<StaState> s_state{StaState::kInactive};
bool          s_inited       = false;
esp_netif_t*  s_netif         = nullptr;
char          s_ssid_active[33] = {0};
char          s_ip_str[16]      = {0};
bool          s_sntp_started    = false;

void start_sntp_once()
{
    if (s_sntp_started) return;
    esp_sntp_config_t cfg = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
    cfg.start = true;
    esp_netif_sntp_init(&cfg);
    s_sntp_started = true;
    // CST-8 = UTC+8 (China Standard Time). Hard-coded for V1; settings
    // app can offer a timezone picker later.
    setenv("TZ", "CST-8", 1);
    tzset();
    ESP_LOGI(TAG, "SNTP started, TZ=CST-8");
}

void on_wifi_event(void* /*arg*/, esp_event_base_t base, int32_t id, void* data)
{
    if (base != WIFI_EVENT) return;
    switch (id) {
        case WIFI_EVENT_STA_START:
            s_state.store(StaState::kConnecting, std::memory_order_relaxed);
            esp_wifi_connect();
            ESP_LOGI(TAG, "STA_START, connect issued");
            break;
        case WIFI_EVENT_STA_CONNECTED:
            ESP_LOGI(TAG, "STA_CONNECTED (waiting for IP)");
            break;
        case WIFI_EVENT_STA_DISCONNECTED: {
            auto* d = static_cast<wifi_event_sta_disconnected_t*>(data);
            // Reason codes from esp_wifi_types_generic.h. The big ones:
            //   2   AUTH_EXPIRE       handshake timeout
            //   15  4WAY_HANDSHAKE_TIMEOUT  usually = wrong password
            //   201 NO_AP_FOUND       SSID not visible / out of range
            //   202 AUTH_FAIL         wrong password (newer chips)
            //   205 CONNECTION_FAIL
            ESP_LOGI(TAG, "STA_DISCONNECTED reason=%d, retrying", d->reason);
            s_state.store(StaState::kFailed, std::memory_order_relaxed);
            s_ip_str[0] = '\0';
            esp_wifi_connect();
            s_state.store(StaState::kConnecting, std::memory_order_relaxed);
            break;
        }
        default: break;
    }
}

void on_ip_event(void* /*arg*/, esp_event_base_t base, int32_t id, void* data)
{
    if (base != IP_EVENT || id != IP_EVENT_STA_GOT_IP) return;
    auto* evt = static_cast<ip_event_got_ip_t*>(data);
    std::snprintf(s_ip_str, sizeof(s_ip_str), IPSTR, IP2STR(&evt->ip_info.ip));
    s_state.store(StaState::kConnected, std::memory_order_relaxed);
    ESP_LOGI(TAG, "connected, ip=%s ssid=%s", s_ip_str, s_ssid_active);
    start_sntp_once();
}

}  // namespace

esp_err_t sta_init()
{
    if (s_inited) return ESP_OK;

    // Network stack + default event loop (idempotent; both bail if already
    // initialized by another subsystem).
    esp_netif_init();
    esp_event_loop_create_default();

    s_netif = esp_netif_create_default_wifi_sta();

    wifi_init_config_t init_cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_err_t err = esp_wifi_init(&init_cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_init: %s", esp_err_to_name(err));
        return err;
    }

    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &on_wifi_event, nullptr);
    esp_event_handler_register(IP_EVENT,   IP_EVENT_STA_GOT_IP, &on_ip_event, nullptr);

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_storage(WIFI_STORAGE_RAM);  // we own creds in NVS ourselves

    s_inited = true;

    // Try to connect if we have credentials.
    char ssid[33] = {0};
    char pass[65] = {0};
    bool have_ssid = settings::get_str("wifi_ssid", ssid, sizeof(ssid));
    settings::get_str("wifi_pass", pass, sizeof(pass));

    if (!have_ssid || ssid[0] == '\0') {
        s_state.store(StaState::kNoCreds, std::memory_order_relaxed);
        ESP_LOGI(TAG, "no creds in NVS, staying idle");
        return ESP_OK;
    }

    std::strncpy(s_ssid_active, ssid, sizeof(s_ssid_active) - 1);

    wifi_config_t wc = {};
    std::strncpy(reinterpret_cast<char*>(wc.sta.ssid),     ssid, sizeof(wc.sta.ssid) - 1);
    std::strncpy(reinterpret_cast<char*>(wc.sta.password), pass, sizeof(wc.sta.password) - 1);
    wc.sta.threshold.authmode = WIFI_AUTH_OPEN;  // accept open + any WPA*
    esp_wifi_set_config(WIFI_IF_STA, &wc);
    esp_wifi_start();
    ESP_LOGI(TAG, "STA started, connecting to '%s'", ssid);
    return ESP_OK;
}

void sta_deinit()
{
    if (!s_inited) return;
    esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &on_wifi_event);
    esp_event_handler_unregister(IP_EVENT,   IP_EVENT_STA_GOT_IP, &on_ip_event);
    esp_wifi_stop();
    esp_wifi_deinit();
    if (s_netif) {
        esp_netif_destroy_default_wifi(s_netif);
        s_netif = nullptr;
    }
    s_inited = false;
    s_state.store(StaState::kInactive, std::memory_order_relaxed);
    s_ip_str[0] = '\0';
    ESP_LOGI(TAG, "STA deinit");
}

void sta_reconnect()
{
    sta_deinit();
    sta_init();
}

StaState   state()        { return s_state.load(std::memory_order_relaxed); }
const char* current_ssid() { return s_ssid_active; }
const char* current_ip()   { return s_ip_str; }

}  // namespace wifi
}  // namespace pocket
