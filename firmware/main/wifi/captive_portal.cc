// Captive-portal provisioner — see captive_portal.h.
//
// Components:
//   1. SoftAP (open, "PocketOracle-XXXX"), max 4 clients.
//   2. DNS hijack task on UDP/53: every query is answered with our AP
//      IP. This makes iOS/Android/Win think they're behind a captive
//      portal and silently open a browser pointed at us.
//   3. esp_http_server with three handlers:
//        GET  /       -> setup form HTML
//        POST /save   -> URL-decoded ssid + password, write to NVS,
//                        set state = kCredsReceived
//        *  (anything) -> 302 to /
//   4. State machine the caller polls; idempotent start/stop.
//
// Caller (apps/wifi_setup) handles UI + transitions back to STA after
// kCredsReceived.

#include "captive_portal.h"

#include "../storage/settings.h"

#include "esp_event.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "lwip/err.h"
#include "lwip/sockets.h"

#include <atomic>
#include <cstdio>
#include <cstring>

namespace pocket {
namespace wifi {

namespace {

constexpr const char* TAG = "PORTAL";

std::atomic<PortalState> s_state{PortalState::kInactive};
char         s_ap_ssid[24]   = {0};
bool         s_started       = false;
esp_netif_t* s_ap_netif      = nullptr;
httpd_handle_t s_httpd       = nullptr;
TaskHandle_t   s_dns_task    = nullptr;
volatile bool  s_dns_run     = false;

// AP IP — ESP-IDF default for SoftAP gateway is 192.168.4.1.
constexpr uint8_t kApIp[4] = {192, 168, 4, 1};

// ---- HTML form ----
// Kept tiny so it fits in one TCP segment and renders instantly even on
// degraded captive-portal browsers.
constexpr const char kFormHtml[] =
    "<!DOCTYPE html><html lang=en><head>"
    "<meta charset=utf-8>"
    "<meta name=viewport content='width=device-width,initial-scale=1'>"
    "<title>Pocket Oracle Setup</title>"
    "<style>"
    "body{font-family:-apple-system,sans-serif;background:#1B1A18;color:#F2EEE6;"
    "max-width:380px;margin:40px auto;padding:24px}"
    "h1{font-size:22px;color:#D9B978;margin:0 0 4px}"
    "p{color:#A09C95;margin:0 0 20px;font-size:14px}"
    "label{display:block;font-size:13px;color:#A09C95;margin:14px 0 6px}"
    "input{width:100%;box-sizing:border-box;padding:11px;font-size:16px;"
    "background:#26241F;border:1px solid #3a3833;color:#F2EEE6;border-radius:6px}"
    "button{width:100%;margin-top:24px;padding:13px;font-size:16px;"
    "background:#D9B978;color:#1B1A18;border:0;border-radius:6px;font-weight:600}"
    "</style></head><body>"
    "<h1>Pocket Oracle</h1>"
    "<p>Enter your WiFi network so the clock can sync time.</p>"
    "<form method=POST action=/save>"
    "<label>Network name (SSID)</label>"
    "<input name=ssid required autocapitalize=off autocorrect=off spellcheck=false>"
    "<label>Password</label>"
    "<input name=pass type=password>"
    "<button>Save &amp; Connect</button>"
    "</form></body></html>";

constexpr const char kDoneHtml[] =
    "<!DOCTYPE html><html><head><meta charset=utf-8>"
    "<title>Saved</title>"
    "<style>body{font-family:-apple-system,sans-serif;background:#1B1A18;"
    "color:#F2EEE6;max-width:380px;margin:60px auto;padding:24px;text-align:center}"
    "h1{color:#D9B978}p{color:#A09C95}</style></head><body>"
    "<h1>Saved</h1><p>The device is reconnecting. You can close this page.</p>"
    "</body></html>";

// ---- URL decoder ----
// In-place decode of "%xx" and '+' -> ' '. Returns new length.
size_t url_decode(char* s)
{
    char* dst = s;
    for (const char* src = s; *src; ++src) {
        if (*src == '+') {
            *dst++ = ' ';
        } else if (*src == '%' && src[1] && src[2]) {
            auto hex = [](char c) {
                if (c >= '0' && c <= '9') return c - '0';
                if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
                if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
                return 0;
            };
            *dst++ = static_cast<char>((hex(src[1]) << 4) | hex(src[2]));
            src += 2;
        } else {
            *dst++ = *src;
        }
    }
    *dst = '\0';
    return dst - s;
}

// Find "key=value" segment in URL-encoded body. Writes value to out and
// URL-decodes it. Returns true if key was found.
bool parse_form_field(const char* body, const char* key,
                      char* out, size_t out_len)
{
    const size_t key_len = std::strlen(key);
    const char* p = body;
    while (*p) {
        if (std::strncmp(p, key, key_len) == 0 && p[key_len] == '=') {
            const char* v = p + key_len + 1;
            const char* end = std::strchr(v, '&');
            if (!end) end = v + std::strlen(v);
            size_t n = std::min(static_cast<size_t>(end - v), out_len - 1);
            std::memcpy(out, v, n);
            out[n] = '\0';
            url_decode(out);
            return true;
        }
        // Advance to next field
        const char* amp = std::strchr(p, '&');
        if (!amp) break;
        p = amp + 1;
    }
    return false;
}

// ---- HTTP handlers ----
esp_err_t h_root(httpd_req_t* req)
{
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, kFormHtml, HTTPD_RESP_USE_STRLEN);
}

esp_err_t h_save(httpd_req_t* req)
{
    char body[256];
    int total = 0;
    while (total < (int)sizeof(body) - 1) {
        int n = httpd_req_recv(req, body + total, sizeof(body) - 1 - total);
        if (n <= 0) break;
        total += n;
    }
    body[total] = '\0';

    char ssid[33] = {0};
    char pass[65] = {0};
    bool has_ssid = parse_form_field(body, "ssid", ssid, sizeof(ssid));
    parse_form_field(body, "pass", pass, sizeof(pass));

    if (!has_ssid || ssid[0] == '\0') {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "missing ssid");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "creds received, ssid='%s' (pass %d chars)", ssid,
             (int)std::strlen(pass));
    settings::set_str("wifi_ssid", ssid);
    settings::set_str("wifi_pass", pass);
    s_state.store(PortalState::kCredsReceived, std::memory_order_relaxed);

    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, kDoneHtml, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t h_redirect(httpd_req_t* req)
{
    // Catch-all: redirect to root so iOS/Android captive-portal detection
    // probes (/generate_204, /hotspot-detect.html, etc.) land on the form.
    httpd_resp_set_status(req, "302 Found");
    httpd_resp_set_hdr(req, "Location", "/");
    httpd_resp_send(req, nullptr, 0);
    return ESP_OK;
}

httpd_handle_t start_httpd()
{
    httpd_config_t cfg = HTTPD_DEFAULT_CONFIG();
    cfg.uri_match_fn   = httpd_uri_match_wildcard;
    cfg.max_uri_handlers = 8;
    cfg.lru_purge_enable = true;

    httpd_handle_t srv = nullptr;
    if (httpd_start(&srv, &cfg) != ESP_OK) {
        ESP_LOGE(TAG, "httpd_start failed");
        return nullptr;
    }

    httpd_uri_t u_root = {"/",     HTTP_GET,  h_root,     nullptr};
    httpd_uri_t u_save = {"/save", HTTP_POST, h_save,     nullptr};
    httpd_uri_t u_any  = {"/*",    HTTP_GET,  h_redirect, nullptr};
    httpd_register_uri_handler(srv, &u_root);
    httpd_register_uri_handler(srv, &u_save);
    httpd_register_uri_handler(srv, &u_any);
    return srv;
}

// ---- DNS hijack task ----
// Minimal DNS responder: every query gets one A-record answer pointing
// to our AP IP. Doesn't validate type/class — the trick is just to give
// the OS *something* to make it stop nagging.
void dns_task(void* /*arg*/)
{
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        ESP_LOGE(TAG, "dns socket failed");
        vTaskDelete(nullptr);
        return;
    }

    sockaddr_in addr = {};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port        = htons(53);
    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        ESP_LOGE(TAG, "dns bind failed");
        close(sock);
        vTaskDelete(nullptr);
        return;
    }

    timeval tv = {.tv_sec = 1, .tv_usec = 0};
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    uint8_t buf[256];
    while (s_dns_run) {
        sockaddr_in src = {};
        socklen_t src_len = sizeof(src);
        int n = recvfrom(sock, buf, sizeof(buf), 0, (sockaddr*)&src, &src_len);
        if (n < 12) continue;  // smaller than a DNS header — junk

        // Set flags: standard response, no error.
        buf[2] = 0x81;
        buf[3] = 0x80;
        // ANCOUNT = 1
        buf[6] = 0x00; buf[7] = 0x01;
        // NSCOUNT / ARCOUNT = 0
        buf[8] = buf[9] = buf[10] = buf[11] = 0x00;

        // Walk past the question to find the byte offset where we'll
        // append the answer.
        int q = 12;
        while (q < n && buf[q] != 0) q += buf[q] + 1;
        q += 1;          // null terminator of QNAME
        q += 4;          // QTYPE + QCLASS

        if (q + 16 > (int)sizeof(buf)) continue;

        // Answer: pointer to name (0xC00C), TYPE A, CLASS IN, TTL 60, RDLEN 4, IP.
        buf[q++] = 0xC0; buf[q++] = 0x0C;     // name pointer back to QNAME
        buf[q++] = 0x00; buf[q++] = 0x01;     // TYPE A
        buf[q++] = 0x00; buf[q++] = 0x01;     // CLASS IN
        buf[q++] = 0x00; buf[q++] = 0x00;     // TTL hi
        buf[q++] = 0x00; buf[q++] = 0x3C;     //   = 60s
        buf[q++] = 0x00; buf[q++] = 0x04;     // RDLENGTH = 4
        buf[q++] = kApIp[0]; buf[q++] = kApIp[1];
        buf[q++] = kApIp[2]; buf[q++] = kApIp[3];

        sendto(sock, buf, q, 0, (sockaddr*)&src, src_len);
    }
    close(sock);
    s_dns_task = nullptr;
    vTaskDelete(nullptr);
}

void compute_ap_ssid()
{
    uint8_t mac[6] = {0};
    esp_read_mac(mac, ESP_MAC_WIFI_SOFTAP);
    std::snprintf(s_ap_ssid, sizeof(s_ap_ssid),
                  "PocketOracle-%02X%02X", mac[4], mac[5]);
}

}  // namespace

esp_err_t portal_start()
{
    if (s_started) return ESP_OK;

    esp_netif_init();
    esp_event_loop_create_default();

    s_ap_netif = esp_netif_create_default_wifi_ap();

    wifi_init_config_t init_cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_err_t err = esp_wifi_init(&init_cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_init: %s", esp_err_to_name(err));
        return err;
    }

    compute_ap_ssid();

    wifi_config_t wc = {};
    std::strncpy(reinterpret_cast<char*>(wc.ap.ssid), s_ap_ssid,
                 sizeof(wc.ap.ssid) - 1);
    wc.ap.ssid_len       = std::strlen(s_ap_ssid);
    wc.ap.channel        = 1;
    wc.ap.max_connection = 4;
    wc.ap.authmode       = WIFI_AUTH_OPEN;

    esp_wifi_set_mode(WIFI_MODE_AP);
    esp_wifi_set_config(WIFI_IF_AP, &wc);
    esp_wifi_start();

    s_httpd = start_httpd();
    if (!s_httpd) {
        esp_wifi_stop();
        esp_wifi_deinit();
        return ESP_FAIL;
    }

    s_dns_run = true;
    xTaskCreatePinnedToCore(dns_task, "dns_hijack", 4096, nullptr, 3,
                            &s_dns_task, 0);

    s_started = true;
    s_state.store(PortalState::kWaitingForUser, std::memory_order_relaxed);
    ESP_LOGI(TAG, "portal up: SSID='%s' IP=%d.%d.%d.%d",
             s_ap_ssid, kApIp[0], kApIp[1], kApIp[2], kApIp[3]);
    return ESP_OK;
}

void portal_stop()
{
    if (!s_started) return;

    s_dns_run = false;
    // dns_task self-deletes on its 1s recv timeout; wait briefly so
    // socket gets closed before we tear the AP down.
    int waited = 0;
    while (s_dns_task && waited < 1500) {
        vTaskDelay(pdMS_TO_TICKS(50));
        waited += 50;
    }

    if (s_httpd) {
        httpd_stop(s_httpd);
        s_httpd = nullptr;
    }

    esp_wifi_stop();
    esp_wifi_deinit();
    if (s_ap_netif) {
        esp_netif_destroy_default_wifi(s_ap_netif);
        s_ap_netif = nullptr;
    }

    s_started = false;
    s_state.store(PortalState::kInactive, std::memory_order_relaxed);
    s_ap_ssid[0] = '\0';
    ESP_LOGI(TAG, "portal stopped");
}

PortalState portal_state()       { return s_state.load(std::memory_order_relaxed); }
const char* portal_ap_ssid()     { return s_ap_ssid; }

}  // namespace wifi
}  // namespace pocket
