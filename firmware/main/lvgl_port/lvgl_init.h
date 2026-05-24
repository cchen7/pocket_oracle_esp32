#pragma once

namespace pocket {

// Initialize LVGL: framebuffer (PSRAM full-screen) + ESP timer tick (2 ms)
// + dedicated LVGL task pinned to core 1 + recursive mutex.
//
// Must be called AFTER M5.begin() (we use M5.Display as the flush backend).
void lvgl_init();

// Take/release the LVGL mutex. Every lv_* call from outside the LVGL task
// must be wrapped:
//
//   pocket::lvgl_lock();
//   lv_label_set_text(label, "hello");
//   pocket::lvgl_unlock();
void lvgl_lock();
void lvgl_unlock();

}  // namespace pocket
