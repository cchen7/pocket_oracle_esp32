#pragma once

// Idle-driven power state machine.
//
// Watches for input inactivity. As idle time grows we step down through
// brightness levels and eventually drop the device into deep sleep, from
// which a BtnA (G11) press wakes the chip via EXT0.
//
//   0s        Active        full brightness
//   30s       Dim           brightness reduced
//   60s       Blank         brightness 0 (backlight off, CPU still running)
//   300s      Deep sleep    everything down; EXT0 on G11 wakes us
//
// Any emitted input event calls power::poke() to reset the idle timer.
// Apps don't need to know about this — it sits behind the input pipeline.

#include <cstdint>

namespace pocket {
namespace power {

// Start the idle-watcher task. Idempotent. Defaults are encoded in the
// .cc file; expose setters later if/when a settings UI needs them.
void init();

// Reset the idle counter. Called by input_manager on every emitted event.
// Cheap, safe from any task.
void poke();

// Immediately drop into deep sleep. The Settings -> Power menu (V2)
// could call this directly. Right now only the idle watcher does.
[[noreturn]] void deep_sleep_now();

}  // namespace power
}  // namespace pocket
