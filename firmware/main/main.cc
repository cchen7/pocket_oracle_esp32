// Pocket Oracle — entry point.
//
// P0 scaffold:
//   - Initialize NVS, M5Unified (board + display).
//   - Show "Pocket Oracle" on the LCD.
//   - Start the USB-CDC service console (help/version/reboot).
// Later phases will add input manager, LVGL UI, app router, etc.

#include "esp_log.h"
#include "nvs_flash.h"

#include <M5Unified.h>

#include "console/console.h"

static const char* TAG = "MAIN";

extern "C" void app_main(void)
{
    // NVS — required by Wi-Fi and used by settings/stats partitions.
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS partition was truncated, erasing");
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    // M5Unified handles PMIC, IMU, audio, display init for the M5StickS3 board.
    auto cfg = M5.config();
    M5.begin(cfg);

    // Landscape orientation; LCD is 240(W) x 135(H) when rotated.
    M5.Display.setRotation(1);
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextDatum(middle_center);
    M5.Display.setFont(&fonts::Font4);
    M5.Display.drawString("Pocket Oracle",
                          M5.Display.width() / 2,
                          M5.Display.height() / 2);

    ESP_LOGI(TAG, "Pocket Oracle booted — display %dx%d",
             M5.Display.width(), M5.Display.height());

    // USB-CDC service console (B2: enabled by default during development).
    pocket::console_start();
}
