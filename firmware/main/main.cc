// Pocket Oracle — entry point.
//
// P2 scaffold:
//   - Initialize NVS, M5Unified, USB-CDC service console.
//   - Bring up LVGL on top of M5GFX (full-screen framebuffer in PSRAM).
//   - Draw a smoke-test screen: title + animated bar to prove the LVGL
//     tick + flush pipeline is alive.

#include "esp_log.h"
#include "nvs_flash.h"

#include <M5Unified.h>
#include <lvgl.h>

#include "console/console.h"
#include "lvgl_port/lvgl_init.h"

static const char* TAG = "MAIN";

namespace {

void show_smoke_test_screen()
{
    pocket::lvgl_lock();

    lv_obj_t* scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x1B1A18), LV_PART_MAIN);

    lv_obj_t* title = lv_label_create(scr);
    lv_label_set_text(title, "Pocket Oracle");
    lv_obj_set_style_text_color(title, lv_color_hex(0xC9A968), LV_PART_MAIN);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_CENTER, 0, -20);

    lv_obj_t* subtitle = lv_label_create(scr);
    lv_label_set_text(subtitle, "LVGL ready");
    lv_obj_set_style_text_color(subtitle, lv_color_hex(0x8E8B85), LV_PART_MAIN);
    lv_obj_align(subtitle, LV_ALIGN_CENTER, 0, 14);

    // Animated bar — smoke-tests that the LVGL tick + redraw + flush
    // pipeline is alive end-to-end.
    lv_obj_t* bar = lv_bar_create(scr);
    lv_obj_set_size(bar, 160, 6);
    lv_obj_align(bar, LV_ALIGN_BOTTOM_MID, 0, -14);
    lv_obj_set_style_bg_color(bar, lv_color_hex(0x26241F), LV_PART_MAIN);
    lv_obj_set_style_bg_color(bar, lv_color_hex(0xC9A968), LV_PART_INDICATOR);
    lv_bar_set_range(bar, 0, 100);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, bar);
    lv_anim_set_values(&a, 0, 100);
    lv_anim_set_time(&a, 1500);
    lv_anim_set_playback_time(&a, 1500);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_exec_cb(&a, [](void* obj, int32_t v) {
        lv_bar_set_value(static_cast<lv_obj_t*>(obj), v, LV_ANIM_OFF);
    });
    lv_anim_start(&a);

    pocket::lvgl_unlock();
}

}  // namespace

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
    show_smoke_test_screen();

    pocket::console_start();
}

