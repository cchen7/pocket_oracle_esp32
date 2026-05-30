#include "settings.h"

#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"

#include <cstring>

namespace pocket {
namespace settings {

namespace {
constexpr const char* TAG       = "SETTINGS";
constexpr const char* NAMESPACE = "settings";

bool open_ro(nvs_handle_t* h)
{
    return nvs_open(NAMESPACE, NVS_READONLY, h) == ESP_OK;
}

bool open_rw(nvs_handle_t* h, const char* op_key)
{
    esp_err_t err = nvs_open(NAMESPACE, NVS_READWRITE, h);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "open RW for %s failed: %s", op_key, esp_err_to_name(err));
        return false;
    }
    return true;
}
}  // namespace

bool has(const char* key)
{
    nvs_handle_t h;
    if (!open_ro(&h)) return false;
    size_t sz = 0;
    bool present = (nvs_get_str(h, key, nullptr, &sz) == ESP_OK) ||
                   (nvs_get_u32(h, key, nullptr)       == ESP_OK);
    nvs_close(h);
    return present;
}

bool erase(const char* key)
{
    nvs_handle_t h;
    if (!open_rw(&h, key)) return false;
    esp_err_t err = nvs_erase_key(h, key);
    if (err == ESP_OK || err == ESP_ERR_NVS_NOT_FOUND) {
        nvs_commit(h);
        nvs_close(h);
        return true;
    }
    ESP_LOGW(TAG, "erase(%s): %s", key, esp_err_to_name(err));
    nvs_close(h);
    return false;
}

uint32_t get_u32(const char* key, uint32_t fallback)
{
    nvs_handle_t h;
    if (!open_ro(&h)) return fallback;
    uint32_t v = fallback;
    nvs_get_u32(h, key, &v);  // not-found leaves v at fallback
    nvs_close(h);
    return v;
}

void set_u32(const char* key, uint32_t value)
{
    nvs_handle_t h;
    if (!open_rw(&h, key)) return;
    esp_err_t err = nvs_set_u32(h, key, value);
    if (err == ESP_OK) err = nvs_commit(h);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "set_u32(%s) failed: %s", key, esp_err_to_name(err));
    }
    nvs_close(h);
}

bool get_str(const char* key, char* out_buf, std::size_t buf_len)
{
    if (!out_buf || buf_len == 0) return false;
    out_buf[0] = '\0';

    nvs_handle_t h;
    if (!open_ro(&h)) return false;

    size_t needed = buf_len;
    esp_err_t err = nvs_get_str(h, key, out_buf, &needed);
    nvs_close(h);
    if (err != ESP_OK) {
        out_buf[0] = '\0';
        return false;
    }
    return true;
}

std::string get_str(const char* key)
{
    char buf[kMaxStrLen] = {0};
    get_str(key, buf, sizeof(buf));
    return std::string(buf);
}

void set_str(const char* key, const char* value)
{
    if (!value) value = "";
    nvs_handle_t h;
    if (!open_rw(&h, key)) return;
    esp_err_t err = nvs_set_str(h, key, value);
    if (err == ESP_OK) err = nvs_commit(h);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "set_str(%s) failed: %s", key, esp_err_to_name(err));
    }
    nvs_close(h);
}

}  // namespace settings
}  // namespace pocket
