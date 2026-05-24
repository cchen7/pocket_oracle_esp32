#pragma once
// Alias header. M5GFX's lvgl.h shim does `#if __has_include("lvgl/lvgl.h")`
// to decide whether to defer to a real LVGL install. The ESP Component
// Manager installs LVGL under managed_components/lvgl__lvgl/, which does
// not match "lvgl/lvgl.h" on any include path — so without this alias,
// M5GFX falls back to declaring its own lv_font_fmt_txt_t types that then
// conflict at link time with the real LVGL ones.
//
// Putting this file behind the project's include path and letting the
// shim find it forces M5GFX into its "real LVGL detected" branch, after
// which both libraries agree on the same typedefs.
#include "lvgl__lvgl/lvgl.h"
