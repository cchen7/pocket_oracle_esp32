#pragma once

// AppBase — abstract lifecycle for every screen.
//
// app_router owns exactly one active AppBase at a time. Lifecycle:
//
//   1. router calls on_enter(parent_screen).  App attaches widgets to the
//      passed-in lv_obj_t (caller-managed; router clears it on exit).
//   2. router pumps on_event(...) for every input event.
//   3. router calls on_exit() before destroying the screen.
//
// Apps do not own the LVGL mutex; the router holds it for the duration of
// on_enter / on_event / on_exit. So they can call lv_* freely.

#include "../app/input_manager.h"

struct _lv_obj_t;  // fwd decl, avoid pulling lvgl.h into every header
typedef struct _lv_obj_t lv_obj_t;

namespace pocket {

class AppBase {
public:
    virtual ~AppBase() = default;

    // Human-readable name shown in the home menu / app switcher.
    virtual const char* name() const = 0;

    // Build the UI into the given root container.
    virtual void on_enter(lv_obj_t* root) = 0;

    // Handle a normalized input event from input_manager.
    virtual void on_event(InputEvent /*event*/) {}

    // Cleanup before the router tears down the screen.
    virtual void on_exit() {}
};

}  // namespace pocket
