// LVGL v9 bring-up on M5StickS3.
//
// We use M5GFX as the flush backend rather than esp_lvgl_port because
// esp_lvgl_port expects an esp_lcd_panel_handle_t, and M5GFX owns the
// SPI bus + DMA channel directly.
//
// Layout: single full-screen framebuffer in PSRAM (8 MB Octal available).
// Render mode: FULL — simplest, no tearing for our 135x240 panel.

#include "lvgl_init.h"

#include "lvgl.h"

#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include <M5Unified.h>

#include <cassert>

namespace pocket {

namespace {

constexpr const char* TAG = "LVGL";
constexpr uint32_t kTickPeriodUs = 2000;      // 2 ms — also passed to lv_tick_inc
constexpr UBaseType_t kTaskPriority = 5;
constexpr uint32_t kTaskStackSize = 8192;
constexpr BaseType_t kTaskCore = 1;            // leave core 0 for WiFi/BLE

SemaphoreHandle_t s_mutex;
lv_display_t* s_disp = nullptr;
lv_color_t* s_framebuffer = nullptr;

void IRAM_ATTR lvgl_tick_cb(void* /*arg*/)
{
    lv_tick_inc(kTickPeriodUs / 1000);
}

void disp_flush(lv_display_t* disp, const lv_area_t* area, uint8_t* px_map)
{
    const int32_t w = lv_area_get_width(area);
    const int32_t h = lv_area_get_height(area);

    // LVGL renders to LV_COLOR_FORMAT_RGB565 (uint16_t per pixel, native LE).
    // M5GFX's pushImage with lgfx::rgb565_t* handles byte ordering for ST7789.
    M5.Display.startWrite();
    M5.Display.pushImage(area->x1, area->y1, w, h,
                         reinterpret_cast<const lgfx::rgb565_t*>(px_map));
    M5.Display.endWrite();

    lv_display_flush_ready(disp);
}

void lvgl_task(void* /*arg*/)
{
    ESP_LOGI(TAG, "LVGL task started on core %d", xPortGetCoreID());
    while (true) {
        uint32_t next_ms = 5;
        if (xSemaphoreTakeRecursive(s_mutex, portMAX_DELAY) == pdTRUE) {
            next_ms = lv_timer_handler();
            xSemaphoreGiveRecursive(s_mutex);
        }
        // Clamp so we never busy-spin and never hold the CPU too long.
        if (next_ms > 30) next_ms = 30;
        if (next_ms < 2)  next_ms = 2;
        vTaskDelay(pdMS_TO_TICKS(next_ms));
    }
}

}  // namespace

void lvgl_init()
{
    s_mutex = xSemaphoreCreateRecursiveMutex();
    assert(s_mutex);

    lv_init();

    const int32_t w = M5.Display.width();
    const int32_t h = M5.Display.height();
    const size_t fb_bytes = static_cast<size_t>(w) * h * sizeof(lv_color_t);

    // 240 * 135 * 2 = 64 800 bytes. Fits internal SRAM, but we deliberately
    // put it in PSRAM so internal SRAM stays available for FreeRTOS stacks,
    // WiFi/BLE buffers, and audio.
    s_framebuffer = static_cast<lv_color_t*>(
        heap_caps_malloc(fb_bytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
    assert(s_framebuffer);
    ESP_LOGI(TAG, "Allocated %u-byte framebuffer in PSRAM at %p",
             static_cast<unsigned>(fb_bytes), s_framebuffer);

    s_disp = lv_display_create(w, h);
    lv_display_set_flush_cb(s_disp, disp_flush);
    lv_display_set_color_format(s_disp, LV_COLOR_FORMAT_RGB565);
    lv_display_set_buffers(s_disp, s_framebuffer, nullptr, fb_bytes,
                           LV_DISPLAY_RENDER_MODE_FULL);

    const esp_timer_create_args_t tick_args = {
        .callback = &lvgl_tick_cb,
        .arg = nullptr,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "lvgl_tick",
        .skip_unhandled_events = true,
    };
    esp_timer_handle_t tick_timer = nullptr;
    ESP_ERROR_CHECK(esp_timer_create(&tick_args, &tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(tick_timer, kTickPeriodUs));

    BaseType_t ok = xTaskCreatePinnedToCore(
        lvgl_task, "lvgl", kTaskStackSize, nullptr,
        kTaskPriority, nullptr, kTaskCore);
    assert(ok == pdPASS);

    ESP_LOGI(TAG, "LVGL initialized — %ld x %ld, tick=%uus",
             (long)w, (long)h, (unsigned)kTickPeriodUs);
}

void lvgl_lock()   { xSemaphoreTakeRecursive(s_mutex, portMAX_DELAY); }
void lvgl_unlock() { xSemaphoreGiveRecursive(s_mutex); }

}  // namespace pocket
