#pragma once

// Pocket Oracle design tokens.
//
// Single source of truth for every screen. App code must reach for these
// constants instead of hardcoding lv_color_hex(...) / pixel sizes. See
// docs/UI_DESIGN_LANGUAGE.md for the design system the values implement.

#include "lvgl.h"

namespace pocket {
namespace theme {

// ---------- Layout ----------

constexpr int32_t SCREEN_W = 240;
constexpr int32_t SCREEN_H = 135;

// 8 px baseline grid.
constexpr int32_t SPACE_XS = 4;
constexpr int32_t SPACE_S  = 8;
constexpr int32_t SPACE_M  = 16;
constexpr int32_t SPACE_L  = 24;
constexpr int32_t SPACE_XL = 32;

// Reserved vertical bands.
constexpr int32_t STATUS_BAR_H  = 16;
constexpr int32_t BUTTON_HINT_H = 14;
constexpr int32_t CONTENT_TOP   = STATUS_BAR_H;
constexpr int32_t CONTENT_BOTTOM= SCREEN_H - BUTTON_HINT_H;
constexpr int32_t CONTENT_H     = CONTENT_BOTTOM - CONTENT_TOP;

// ---------- Palette ----------

// Dark mode (default for now; P7 settings will toggle between dark/light/auto).
constexpr uint32_t COLOR_BG_PRIMARY    = 0x1B1A18;  // 深墨
constexpr uint32_t COLOR_BG_ELEVATED   = 0x26241F;  // 暖深灰
constexpr uint32_t COLOR_INK_PRIMARY   = 0xF2EEE6;  // 米白
constexpr uint32_t COLOR_INK_SECONDARY = 0xA09C95;  // 中灰
constexpr uint32_t COLOR_ACCENT_MAIN   = 0xD9B978;  // 提亮暖金
constexpr uint32_t COLOR_ACCENT_WARN   = 0xC26B62;  // 提亮朱砂
constexpr uint32_t COLOR_ACCENT_CALM   = 0x8FA09D;  // 提亮青灰

inline lv_color_t bg_primary()    { return lv_color_hex(COLOR_BG_PRIMARY); }
inline lv_color_t bg_elevated()   { return lv_color_hex(COLOR_BG_ELEVATED); }
inline lv_color_t ink_primary()   { return lv_color_hex(COLOR_INK_PRIMARY); }
inline lv_color_t ink_secondary() { return lv_color_hex(COLOR_INK_SECONDARY); }
inline lv_color_t accent_main()   { return lv_color_hex(COLOR_ACCENT_MAIN); }
inline lv_color_t accent_warn()   { return lv_color_hex(COLOR_ACCENT_WARN); }
inline lv_color_t accent_calm()   { return lv_color_hex(COLOR_ACCENT_CALM); }

// ---------- Type ----------

// Until we ship Puhui glyph subsets in P3, the built-in Montserrat sizes are
// the closest stand-ins. Replace each accessor when the CJK fonts land.
inline const lv_font_t* font_body()    { return &lv_font_montserrat_14; }
inline const lv_font_t* font_caption() { return &lv_font_montserrat_14; }
inline const lv_font_t* font_title()   { return &lv_font_montserrat_24; }
inline const lv_font_t* font_display() { return &lv_font_montserrat_24; }

// ---------- Motion ----------

constexpr uint32_t ANIM_FAST_MS    = 100;   // press feedback
constexpr uint32_t ANIM_DEFAULT_MS = 300;   // screen transitions
constexpr uint32_t ANIM_REVEAL_MS  = 1500;  // text-reveal cumulative target

}  // namespace theme
}  // namespace pocket
