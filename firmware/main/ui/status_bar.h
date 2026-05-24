#pragma once

// Persistent top status bar — lives above every app's root container,
// updates itself on a 2 s lv_timer. Shows uptime (hh:mm placeholder until
// SNTP lands in P5) on the left and battery percentage on the right.

struct _lv_obj_t;
typedef struct _lv_obj_t lv_obj_t;

namespace pocket {

void status_bar_init();

}  // namespace pocket
