#pragma once

// Single-button + shake input pipeline.
//
// M5StickS3 has one usable user button (G11) and the BMI270 IMU. The PMIC
// owns double-press = OFF and >= 3 s hold = bootloader, so we only get to
// observe single-press and short-to-medium holds. To cover the gesture
// surface, app-layer gestures are:
//
//   - kButtonShortPress  : G11 pressed and released < 800 ms
//   - kButtonLongPress   : G11 held >= 800 ms (fired on release; we never
//                          fire after 2.5 s so the user can still bail out
//                          before the PMIC catches the very-long press)
//   - kShake             : BMI270 accel magnitude exceeds threshold + cooldown
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
};

using InputListener = void (*)(InputEvent event, void* user);

// Start the dedicated input task (priority 4, pinned to core 0). Spawns the
// IMU polling + button state machine. Safe to call once after M5.begin().
void input_manager_init();

// Install (or replace) the active listener. nullptr = no listener.
void input_manager_set_listener(InputListener cb, void* user);

}  // namespace pocket
