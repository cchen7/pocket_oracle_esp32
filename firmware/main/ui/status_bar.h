#pragma once

// Persistent top status bar — lives on lv_layer_top above every app
// root, updates itself on a 2 s lv_timer. Shows wall-clock HH:MM
// (or boot uptime mm:ss until SNTP syncs) on the left and battery
// percentage on the right. Colors and font follow theme::current().

struct _lv_obj_t;
typedef struct _lv_obj_t lv_obj_t;

namespace pocket {

void status_bar_init();

// Force the bar to re-pick up theme colors + font right now. Call this
// after theme::set_current() so the change is visible immediately
// instead of waiting for the next 2 s refresh tick.
void status_bar_apply_theme();

}  // namespace pocket
