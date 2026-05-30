// Idle -> dim -> blank -> deep sleep. See power_manager.h.

#include "power_manager.h"

#include "esp_log.h"
#include "esp_sleep.h"
#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <M5Unified.h>

#include <atomic>

namespace pocket {
namespace power {

namespace {

constexpr const char* TAG = "POWER";

// Threshold timings (milliseconds since last input).
constexpr uint32_t kDimMs   = 30 * 1000;
constexpr uint32_t kBlankMs = 60 * 1000;
constexpr uint32_t kSleepMs = 300 * 1000;  // 5 minutes — balance battery vs annoyance

// Brightness levels (M5Unified scale 0..255).
constexpr uint8_t kBrightActive = 96;
constexpr uint8_t kBrightDim    = 16;
constexpr uint8_t kBrightOff    = 0;

constexpr gpio_num_t kWakePin = GPIO_NUM_11;  // BtnA — RTC IO, EXT0 capable

enum class Stage : uint8_t { kActive, kDim, kBlank };

std::atomic<uint32_t> s_last_input_ms{0};
std::atomic<Stage>    s_stage{Stage::kActive};
bool                  s_inited = false;

uint32_t now_ms()
{
    return xTaskGetTickCount() * portTICK_PERIOD_MS;
}

void apply_stage(Stage s)
{
    Stage prev = s_stage.exchange(s, std::memory_order_relaxed);
    if (prev == s) return;
    switch (s) {
        case Stage::kActive: M5.Display.setBrightness(kBrightActive); break;
        case Stage::kDim:    M5.Display.setBrightness(kBrightDim);    break;
        case Stage::kBlank:  M5.Display.setBrightness(kBrightOff);    break;
    }
    ESP_LOGI(TAG, "stage=%d brightness=%d",
             static_cast<int>(s),
             s == Stage::kActive ? kBrightActive
             : s == Stage::kDim  ? kBrightDim
             : kBrightOff);
}

void watcher_task(void* /*arg*/)
{
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        const uint32_t idle = now_ms() - s_last_input_ms.load(std::memory_order_relaxed);
        if (idle >= kSleepMs) {
            ESP_LOGI(TAG, "idle %lu ms -> deep sleep", (unsigned long)idle);
            deep_sleep_now();
            // not reached
        } else if (idle >= kBlankMs) {
            apply_stage(Stage::kBlank);
        } else if (idle >= kDimMs) {
            apply_stage(Stage::kDim);
        } else {
            apply_stage(Stage::kActive);
        }
    }
}

}  // namespace

void init()
{
    if (s_inited) return;
    s_last_input_ms.store(now_ms(), std::memory_order_relaxed);
    apply_stage(Stage::kActive);
    xTaskCreatePinnedToCore(watcher_task, "power", 3072, nullptr, 2, nullptr, 0);
    s_inited = true;
    ESP_LOGI(TAG, "power manager up: dim=%lus blank=%lus sleep=%lus",
             (unsigned long)(kDimMs / 1000),
             (unsigned long)(kBlankMs / 1000),
             (unsigned long)(kSleepMs / 1000));
}

void poke()
{
    s_last_input_ms.store(now_ms(), std::memory_order_relaxed);
    // If we were dimmed/blanked, snap back to active. Re-applying when
    // already active is cheap (apply_stage early-outs).
    if (s_stage.load(std::memory_order_relaxed) != Stage::kActive) {
        apply_stage(Stage::kActive);
    }
}

[[noreturn]] void deep_sleep_now()
{
    // Configure G11 as EXT0 wakeup, active-low (BtnA pulls G11 to GND
    // when pressed; pullup keeps it high otherwise). Internal pullup
    // is enabled so the line stays clean during sleep.
    rtc_gpio_pullup_en(kWakePin);
    rtc_gpio_pulldown_dis(kWakePin);
    esp_sleep_enable_ext0_wakeup(kWakePin, 0);

    // Try to leave the screen dark before vanishing so the user doesn't
    // see the last frame held for the few hundred ms before sleep takes.
    M5.Display.setBrightness(0);
    M5.Display.sleep();

    ESP_LOGI(TAG, "entering deep sleep; press BtnA to wake");
    esp_deep_sleep_start();
    // Unreachable. esp_deep_sleep_start resets the SoC.
    while (true) {}
}

}  // namespace power
}  // namespace pocket
