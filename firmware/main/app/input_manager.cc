// Single-button + shake input pipeline. See input_manager.h.

#include "input_manager.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <M5Unified.h>

#include <atomic>
#include <cmath>

namespace pocket {

namespace {

constexpr const char* TAG = "INPUT";
constexpr TickType_t  kPollPeriod      = pdMS_TO_TICKS(20);  // 50 Hz
constexpr uint32_t    kLongPressMs     = 800;
constexpr uint32_t    kVeryLongPressMs = 2500;   // ceiling; PMIC owns >= 3 s

// Shake = brief spike in linear acceleration magnitude.
// At rest |a| ~= 1.0 g (gravity). A purposeful shake easily clears 1.8 g.
constexpr float    kShakeThresholdG  = 1.7f;
constexpr uint32_t kShakeCooldownMs  = 500;

InputListener s_listener = nullptr;
void*         s_user     = nullptr;

void emit(InputEvent ev)
{
    InputListener cb = s_listener;
    void* user = s_user;
    if (cb) cb(ev, user);
}

void input_task(void* /*arg*/)
{
    ESP_LOGI(TAG, "input task started on core %d", xPortGetCoreID());

    bool      btn_was_down  = false;
    uint32_t  btn_press_ms  = 0;
    bool      long_emitted  = false;

    uint32_t  shake_cooldown_until = 0;

    TickType_t last = xTaskGetTickCount();
    for (;;) {
        M5.update();
        const uint32_t now_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;

        // ---- Button ----
        const bool down = M5.BtnA.isPressed();
        if (down && !btn_was_down) {
            btn_press_ms = now_ms;
            long_emitted = false;
        } else if (down && btn_was_down) {
            const uint32_t held = now_ms - btn_press_ms;
            if (!long_emitted && held >= kLongPressMs && held < kVeryLongPressMs) {
                emit(InputEvent::kButtonLongPress);
                long_emitted = true;
            }
        } else if (!down && btn_was_down) {
            const uint32_t held = now_ms - btn_press_ms;
            if (!long_emitted && held < kLongPressMs) {
                emit(InputEvent::kButtonShortPress);
            }
            // Released after kVeryLongPressMs without firing — assume the
            // user is on their way to the PMIC bootloader hold, suppress.
        }
        btn_was_down = down;

        // ---- Shake ----
        float ax = 0, ay = 0, az = 0;
        if (M5.Imu.getAccelData(&ax, &ay, &az)) {
            const float mag = std::sqrt(ax * ax + ay * ay + az * az);
            if (mag >= kShakeThresholdG && now_ms >= shake_cooldown_until) {
                emit(InputEvent::kShake);
                shake_cooldown_until = now_ms + kShakeCooldownMs;
            }
        }

        vTaskDelayUntil(&last, kPollPeriod);
    }
}

}  // namespace

void input_manager_init()
{
    BaseType_t ok = xTaskCreatePinnedToCore(
        input_task, "input", 4096, nullptr, 4, nullptr, 0);
    if (ok != pdPASS) {
        ESP_LOGE(TAG, "failed to spawn input task");
    }
}

void input_manager_set_listener(InputListener cb, void* user)
{
    s_listener = cb;
    s_user     = user;
}

}  // namespace pocket
