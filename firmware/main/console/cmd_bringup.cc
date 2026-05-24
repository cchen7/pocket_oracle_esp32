// P1 bring-up commands — exercise each on-board peripheral from the USB-CDC
// console so we can verify LCD/buttons/IMU/speaker/PMIC/RTC without writing
// throwaway app_main code.

#include "cmd_bringup.h"

#include "esp_console.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <M5Unified.h>

#include <cstdio>
#include <ctime>

namespace pocket {

namespace {

constexpr const char* TAG = "BRINGUP";

// Restore the steady-state landing screen after a destructive test like `lcd`.
void redraw_landing_screen()
{
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextDatum(middle_center);
    M5.Display.setFont(&fonts::Font4);
    M5.Display.drawString("Pocket Oracle",
                          M5.Display.width() / 2,
                          M5.Display.height() / 2);
}

int cmd_lcd(int /*argc*/, char** /*argv*/)
{
    struct ColorStep { uint16_t color; const char* name; uint16_t text; };
    const ColorStep steps[] = {
        { TFT_RED,    "RED",   TFT_WHITE },
        { TFT_GREEN,  "GREEN", TFT_BLACK },
        { TFT_BLUE,   "BLUE",  TFT_WHITE },
        { TFT_WHITE,  "WHITE", TFT_BLACK },
        { TFT_BLACK,  "BLACK", TFT_WHITE },
    };
    M5.Display.setTextDatum(middle_center);
    M5.Display.setFont(&fonts::Font4);
    for (const auto& s : steps) {
        M5.Display.fillScreen(s.color);
        M5.Display.setTextColor(s.text);
        M5.Display.drawString(s.name,
                              M5.Display.width() / 2,
                              M5.Display.height() / 2);
        printf("  %-5s 0x%04X\n", s.name, s.color);
        vTaskDelay(pdMS_TO_TICKS(450));
    }
    redraw_landing_screen();
    printf("LCD cycle done. Check that all 5 colors appeared correctly.\n");
    return 0;
}

int cmd_btn(int /*argc*/, char** /*argv*/)
{
    // Configure raw GPIO read on G11 and G12 so we can distinguish whether a
    // press registers via M5.BtnA/B (G11/G12) vs a different pin (board may
    // wire the front button to a non-default GPIO).
    constexpr gpio_num_t kPins[] = { GPIO_NUM_11, GPIO_NUM_12 };
    for (auto pin : kPins) {
        gpio_config_t cfg = {
            .pin_bit_mask = 1ULL << pin,
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_ENABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        gpio_config(&cfg);
    }

    constexpr int kDurationMs = 8000;
    constexpr int kPollMs = 15;
    printf("Press buttons for %d s. We log:\n"
           "  M5.BtnA (G11)  M5.BtnB (G12)  raw G11 level  raw G12 level\n",
           kDurationMs / 1000);
    int presses_a = 0, presses_b = 0;
    int g11_edges = 0, g12_edges = 0;
    int prev_g11 = gpio_get_level(GPIO_NUM_11);
    int prev_g12 = gpio_get_level(GPIO_NUM_12);
    const TickType_t deadline = xTaskGetTickCount() + pdMS_TO_TICKS(kDurationMs);
    while (xTaskGetTickCount() < deadline) {
        M5.update();
        int g11 = gpio_get_level(GPIO_NUM_11);
        int g12 = gpio_get_level(GPIO_NUM_12);
        if (M5.BtnA.wasPressed()) {
            ++presses_a;
            printf("  [evt] BtnA wasPressed  raw G11=%d  G12=%d\n", g11, g12);
        }
        if (M5.BtnB.wasPressed()) {
            ++presses_b;
            printf("  [evt] BtnB wasPressed  raw G11=%d  G12=%d\n", g11, g12);
        }
        if (g11 != prev_g11) {
            ++g11_edges;
            printf("  [raw] G11 -> %d\n", g11);
            prev_g11 = g11;
        }
        if (g12 != prev_g12) {
            ++g12_edges;
            printf("  [raw] G12 -> %d\n", g12);
            prev_g12 = g12;
        }
        vTaskDelay(pdMS_TO_TICKS(kPollMs));
    }
    printf("Summary:\n");
    printf("  M5.BtnA presses=%d, M5.BtnB presses=%d\n", presses_a, presses_b);
    printf("  raw G11 edges=%d, raw G12 edges=%d\n", g11_edges, g12_edges);
    return 0;
}

int cmd_imu(int /*argc*/, char** /*argv*/)
{
    if (!M5.Imu.isEnabled()) {
        printf("ERROR: IMU not detected by M5Unified (board: %d)\n",
               static_cast<int>(M5.getBoard()));
        return 1;
    }
    constexpr int kSamples = 20;
    constexpr int kIntervalMs = 50;
    printf("Reading %d IMU samples @ %d ms (move/tilt the device to see deltas)\n",
           kSamples, kIntervalMs);
    printf("  ax       ay       az       gx       gy       gz\n");
    for (int i = 0; i < kSamples; ++i) {
        float ax = 0, ay = 0, az = 0, gx = 0, gy = 0, gz = 0;
        M5.Imu.getAccelData(&ax, &ay, &az);
        M5.Imu.getGyroData(&gx, &gy, &gz);
        printf("  %+6.3f  %+6.3f  %+6.3f  %+7.2f  %+7.2f  %+7.2f\n",
               ax, ay, az, gx, gy, gz);
        vTaskDelay(pdMS_TO_TICKS(kIntervalMs));
    }
    return 0;
}

int cmd_spk(int /*argc*/, char** /*argv*/)
{
    if (!M5.Speaker.isEnabled()) {
        printf("ERROR: speaker not enabled\n");
        return 1;
    }
    printf("Beep: 1 kHz / 200 ms\n");
    M5.Speaker.tone(1000.0f, 200);
    vTaskDelay(pdMS_TO_TICKS(300));
    printf("Beep: 2 kHz / 200 ms\n");
    M5.Speaker.tone(2000.0f, 200);
    vTaskDelay(pdMS_TO_TICKS(300));
    return 0;
}

int cmd_bat(int /*argc*/, char** /*argv*/)
{
    const int mv = M5.Power.getBatteryVoltage();
    const int pct = M5.Power.getBatteryLevel();
    const auto charging = M5.Power.isCharging();
    const char* chg_str = "unknown";
    switch (charging) {
        case m5::Power_Class::is_charging_t::is_discharging: chg_str = "discharging"; break;
        case m5::Power_Class::is_charging_t::is_charging:    chg_str = "charging"; break;
        case m5::Power_Class::is_charging_t::charge_unknown: chg_str = "unknown"; break;
    }
    printf("Battery: %d mV (%d%%), %s\n", mv, pct, chg_str);
    return 0;
}

int cmd_time(int /*argc*/, char** /*argv*/)
{
    // M5StickS3 has no discrete RTC chip; report ESP32 system time which is
    // populated by SNTP (once WiFi connects) and retained across deep sleep
    // but lost on PMIC power-off.
    time_t now = time(nullptr);
    struct tm tm_utc;
    struct tm tm_local;
    gmtime_r(&now, &tm_utc);
    localtime_r(&now, &tm_local);
    char buf_utc[32], buf_local[32];
    strftime(buf_utc,   sizeof(buf_utc),   "%Y-%m-%d %H:%M:%S", &tm_utc);
    strftime(buf_local, sizeof(buf_local), "%Y-%m-%d %H:%M:%S", &tm_local);
    printf("System time:\n");
    printf("  epoch : %lld\n", static_cast<long long>(now));
    printf("  UTC   : %s\n", buf_utc);
    printf("  local : %s (TZ=%s)\n", buf_local, getenv("TZ") ? getenv("TZ") : "(unset)");
    printf("Note: M5StickS3 has no discrete RTC. Time comes from SNTP once WiFi\n");
    printf("      connects; epoch 0 means we have not synced yet.\n");
    return 0;
}

const esp_console_cmd_t kCommands[] = {
    { "lcd", "LCD bring-up: cycle 5 colors then restore landing screen",
      nullptr, &cmd_lcd, nullptr },
    { "btn", "Button bring-up: poll KEY1/KEY2 for 5 seconds and log presses",
      nullptr, &cmd_btn, nullptr },
    { "imu", "IMU bring-up: print 20 accel+gyro samples (BMI270)",
      nullptr, &cmd_imu, nullptr },
    { "spk", "Speaker bring-up: play 1 kHz and 2 kHz beeps",
      nullptr, &cmd_spk, nullptr },
    { "bat", "Battery bring-up: voltage, level, charging state via PMIC",
      nullptr, &cmd_bat, nullptr },
    { "time", "Time bring-up: read ESP32 system time (SNTP-populated)",
      nullptr, &cmd_time, nullptr },
};

}  // namespace

void register_bringup_commands()
{
    for (const auto& cmd : kCommands) {
        ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
    }
    ESP_LOGI(TAG, "Registered %d bring-up commands",
             static_cast<int>(sizeof(kCommands) / sizeof(kCommands[0])));
}

}  // namespace pocket
