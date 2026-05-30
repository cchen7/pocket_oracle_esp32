#pragma once

// Single-button + shake input pipeline.
//
// M5StickS3 has TWO user buttons (G11 = BtnA = front large bar,
// G12 = BtnB = right side) + the PMIC power button on the left side
// (owned by the PMIC, not visible to the app layer).
//
// Long-press on either button takes you home (handled by app_router).
// Apps see:
//
//   - kButtonShortPress   : BtnA pressed and released < 800 ms
//   - kButtonLongPress    : BtnA held >= 800 ms
//   - kButtonBShortPress  : BtnB pressed and released < 800 ms
//   - kButtonBLongPress   : BtnB held >= 800 ms
//   - kShake              : BMI270 accel magnitude exceeds threshold + cooldown
//
// Apps consume events through input_manager_register_listener(). Only one
// listener at a time — app_router updates it as it pushes/pops apps.

#include <cstdint>

namespace pocket {

enum class InputEvent : uint8_t {
    kNone,
    kButtonShortPress,
    kButtonLongPress,
    kShake,
    kButtonBShortPress,
    kButtonBLongPress,
};

using InputListener = void (*)(InputEvent event, void* user);

// Start the dedicated input task (priority 4, pinned to core 0). Spawns the
// IMU polling + button state machine. Safe to call once after M5.begin().
void input_manager_init();

// Install (or replace) the active listener. nullptr = no listener.
void input_manager_set_listener(InputListener cb, void* user);

}  // namespace pocket
