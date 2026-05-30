// Pocket Oracle — entry point.
//
// P2 milestone:
//   - NVS, M5Unified board init, USB-CDC service console.
//   - LVGL v9 with M5GFX flush backend (full-screen PSRAM framebuffer).
//   - input_manager: single button + IMU shake → InputEvent.
//   - app_router: pushes home menu, dispatches events, handles
//     long-press = back-to-home globally.

#include "esp_log.h"
#include "nvs_flash.h"

#include <M5Unified.h>

#include "app/app_router.h"
#include "app/input_manager.h"
#include "app/power_manager.h"
#include "console/console.h"
#include "lvgl_port/lvgl_init.h"
#include "ui/status_bar.h"
#include "wifi/wifi_sta.h"

static const char* TAG = "MAIN";

extern "C" void app_main(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS partition was truncated, erasing");
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    auto cfg = M5.config();
    M5.begin(cfg);
    M5.Display.setRotation(1);  // Landscape: 240 (W) x 135 (H)

    ESP_LOGI(TAG, "M5Unified up — display %dx%d, board=%d",
             static_cast<int>(M5.Display.width()),
             static_cast<int>(M5.Display.height()),
             static_cast<int>(M5.getBoard()));

    pocket::lvgl_init();
    pocket::status_bar_init();
    pocket::input_manager_init();
    pocket::app_router_init();   // pushes the home menu
    pocket::power::init();       // idle -> dim -> blank -> deep sleep

    // STA auto-connect if NVS has credentials; no-op + kNoCreds otherwise.
    // SNTP starts inside wifi_sta on first IP_EVENT_STA_GOT_IP, so the
    // Clock app will pick up wall time without any further wiring.
    pocket::wifi::sta_init();

    pocket::console_start();
}

