#pragma once

// app_router — owns the active AppBase + the LVGL root screen, dispatches
// input events, handles "long press = back to home" globally.

#include <memory>

namespace pocket {

class AppBase;

// Initialize router. Builds the persistent status bar widget and pushes
// the home menu as the first screen. Subscribes to input_manager so every
// input event is routed (a) to the global handler (long-press = home) and
// (b) to the active app.
void app_router_init();

// Replace the active app. Takes ownership; previous app is exited and
// destroyed before the new one's on_enter() runs.
void app_router_push(std::unique_ptr<AppBase> app);

// Pop back to the home menu. Convenience wrapper around app_router_push.
void app_router_go_home();

}  // namespace pocket
