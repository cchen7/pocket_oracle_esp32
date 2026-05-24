#include "stats.h"

#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"

namespace pocket {
namespace stats {

namespace {
constexpr const char* TAG = "STATS";
constexpr const char* NAMESPACE = "stats";
}  // namespace

uint32_t get(const char* key)
{
    nvs_handle_t h;
    if (nvs_open(NAMESPACE, NVS_READONLY, &h) != ESP_OK) {
        return 0;
    }
    uint32_t v = 0;
    nvs_get_u32(h, key, &v);  // unset = leaves v at 0
    nvs_close(h);
    return v;
}

void set(const char* key, uint32_t value)
{
    nvs_handle_t h;
    esp_err_t err = nvs_open(NAMESPACE, NVS_READWRITE, &h);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "open(%s) failed: %s", key, esp_err_to_name(err));
        return;
    }
    err = nvs_set_u32(h, key, value);
    if (err == ESP_OK) err = nvs_commit(h);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "set(%s) failed: %s", key, esp_err_to_name(err));
    }
    nvs_close(h);
}

}  // namespace stats
}  // namespace pocket
