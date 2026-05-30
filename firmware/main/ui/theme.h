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

// Ink on white rice paper. Same hex values as the cover PNGs in
// tools/gen_covers.py so the home carousel and per-app screens read as
// one continuous surface.
constexpr uint32_t COLOR_BG_PRIMARY    = 0xF0E8D8;  // 暖纸白
constexpr uint32_t COLOR_BG_ELEVATED   = 0xE6DCC8;  // 略深的纸（卡片/分割）
constexpr uint32_t COLOR_INK_PRIMARY   = 0x1A1814;  // 墨黑
constexpr uint32_t COLOR_INK_SECONDARY = 0x7C7468;  // 灰墨
constexpr uint32_t COLOR_ACCENT_MAIN   = 0xA8362E;  // 朱砂 — 主强调（印章/重点）
constexpr uint32_t COLOR_ACCENT_WARN   = 0xA8362E;  // 朱砂 — 警示与主强调同色（水墨配色简洁）
constexpr uint32_t COLOR_ACCENT_CALM   = 0x3A4A4F;  // 青墨 — 副色

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
inline const lv_font_t* font_display() { return &lv_font_montserrat_48; }

// ---------- Motion ----------

constexpr uint32_t ANIM_FAST_MS    = 100;   // press feedback
constexpr uint32_t ANIM_DEFAULT_MS = 300;   // screen transitions
constexpr uint32_t ANIM_REVEAL_MS  = 1500;  // text-reveal cumulative target

}  // namespace theme
}  // namespace pocket
