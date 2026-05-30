# Checkpoint — 2026-05-30 (UI polish in flight)

> Local-only. **23 commits ahead of origin/main, not pushed.**
> Previous checkpoint: `CHECKPOINT_2026-05-25.md` (P5 in flight).

## Where we are

P0–P9 base done. The bulk of today (2026-05-30) was the **国风水墨 UI
overhaul** on top of the working V1 firmware.

| Phase | Status | Notes |
|---|---|---|
| P0–P5 base | ✓ | Decision/MBTI/Fortune/Clock/Muyu working |
| P5 Muyu sound | ✓ | Synthesized PCM (`tools/gen_muyu_pcm.py`) — replaced phone-dial tone |
| **P1.9 CORRIGENDUM** | ✓ | BtnB on G12 is REAL (was wrong in original P1.9). 2-button input model. |
| P6 BLE HID Remote | ✓ | Arrow keys for iOS compatibility, bonded, deinit on exit |
| P7 Settings + WiFi + SNTP | ✓ | Captive portal provisioning; "caption" SSID verified |
| P8 power | ✓ | 30s dim / 60s blank / 5min deep sleep with BtnA wake |
| **P9 UI overhaul** | 🟡 in flight | See "What's done / what's left" below |

## Commits unpushed (newest first)

```
e52c40d UI polish: per-app ink colors, 14 px hints, CN copy unification
1294fc5 Per-theme display + title fonts on Coin/Dice/Random10/Yes-No
db7ab05 Settings/theme polish: CN labels, status-bar follows theme, distinct skins
7fea1e2 Multi-theme system: 4 ink-painting skins (水墨/绢本/竹简/拓片)
fc6b673 P9 home: ink-on-paper carousel with brush covers
cb79b35 P8 power: idle dim/blank + deep sleep with BtnA wake
3224858 Per-app BtnB integration
9c5683f Muyu: synthesized wooden-fish PCM (was: phone-dial tone)
7c3c830 status_bar: show wall-clock time once SNTP has synced
db78ee1 P7 Settings + WiFi captive-portal provisioning + SNTP
1ba2a3f 2-button input model — BtnB on G12 is real, P1.9 was wrong
fa7a2e7 ble_remote: arrow keys instead of PgDn/PgUp for iOS compatibility
f3d0e98 P6 BLE HID Remote — V1 PPT mode
1e70b89 Home menu: add navigation hint footer + diagnostic log hooks
390cfc5 SNAPSHOT 2026-05-25 (previous checkpoint)
... (older P0–P4)
```

## P9 UI overhaul — what's done

**Carousel home menu** (replaces 4×3 grid):
- Single full-screen cover per app, BtnA = slide-next, BtnB = enter.
- 12 covers × 4 themes pre-rendered as RGB565 PNG, embedded via
  `firmware/main/assets/home_covers.h` (~3 MB pixel data).
- Generator: `tools/gen_covers.py`. Re-run after editing themes.
- Status bar overlays top 16 px, covers leave that band empty by design.

**Multi-theme system** (`水墨 / 绢本 / 竹简 / 拓片`):
- `theme::Theme` struct in `firmware/main/ui/theme.h` with palette + cover
  bundle + per-theme `title_font` (28 px) + `display_font` (48 px) +
  `is_dark` flag.
- Registry in `firmware/main/ui/theme.cc`. Current theme persisted in NVS
  (`settings::set_u32("theme", n)`) via `theme::set_current(idx)`.
- Settings → 主题 picker (`firmware/main/apps/settings/settings_app.cc`)
  changes theme + repaints itself in the new palette + calls
  `status_bar_apply_theme()` so the persistent top bar redraws.

**Per-theme brush fonts** (4 themes × 2 sizes = 8 files):
- `assets/fonts/theme_<id>_title_28.c` (Ma Shan Zheng / ZCOOL XiaoWei /
  Liu Jian Mao Cao / Long Cang)
- `assets/fonts/theme_<id>_display_48.c` (same fonts at 48 px for big
  result words)
- Generator: `tools/gen_themed_display_fonts.py`.
- Accessors: `theme::font_title_themed()` and `font_display_themed()`.

**Runtime CJK body font** (shared across themes — readability over flair):
- `assets/fonts/lxgw_wenkai_cjk_14.c` (hints)
- `assets/fonts/lxgw_wenkai_cjk_16.c` (body / sub-result)
- Generator: `tools/gen_cjk_font.py`. ~122 chars subset.
- Accessors: `theme::font_body()` (16) / `font_caption()` (14).

**Per-app ink colors** (signature seal color per app, stable across themes):
- `theme::InkColor` palette in `theme.h`: 朱砂/青墨/黛蓝/赭石/黛紫/兰花紫/松绿/苍翠/远墨.
- Each color has `on_light` + `on_dark` variant; `theme::ink_color(c)`
  picks based on current theme's `is_dark`.
- Applied: Coin (正=朱砂/反=青墨), Yes/No (是=苍翠/否=朱砂), Random10 (黛蓝),
  Dice (赭石), Answer (黛紫), Muyu (朱砂), BLE (3 states three colors).

**CN copy** for everything shipped — Settings/Theme/About chrome, Coin,
Yes/No, Random10, Dice, Answer Book chrome, Muyu, BLE Remote, WiFi setup
(partial — see below), all hint footers using consistent "短按 / 侧键 /
长按返回" vocabulary. BtnB referred to as "侧键" everywhere.

**LVGL invalidate pattern**: `font BEFORE text + lv_obj_invalidate()`.
Required for themed labels to actually repaint on theme change — the
gotcha I lost an hour to. Applied to all relevant `on_enter` paths.

## P9 — what's left (in priority order, user agreed list)

1. **Fortune UI + content data** — "DO"/"AVOID" → 宜/忌, 12+12 DOs/AVOIDs
   + 8 lucky color names in `firmware/main/data/fortune_data.h` to translate.
   Wire 宜=松绿 / 忌=朱砂 ink colors. Apply themed title font.
2. **Clock weekday + About labels + WiFi Setup chrome** (small):
   - `tool_apps.cc` Clock weekday array: Sun/Mon/Tue → 日/一/二/三/四/五/六
   - `settings_app.cc` About fields: fw/wifi/ip/free/uptime → 固件/网络/IP/内存/运行
   - `apps/wifi_setup/wifi_setup_app.cc` title "WiFi Setup" → "无线配网" +
     instruction lines + status text → CN.
3. **MBTI UI hint + nickname**: don't touch 128 prompts yet, just the
   chrome (hint footer + maybe Chinese nickname strings if the type table
   has them).
4. **Per-app ink color + themed title on the apps we missed**:
   MBTI=兰花紫, Fortune (DO=松绿/AVOID=朱砂), Clock=远墨, Settings/About/WiFi
   = 远墨. Apply `font_title_themed()` + `lv_obj_invalidate` to their
   header labels.
5. **Answer Book 50 phrases** — `data/answer_book_data.h`. CN translation
   pass. Add new chars to LXGW subset + regen.
6. **MBTI 128 prompts** — biggest single piece. Each of 16 types ×
   8 prompts in `data/mbti_data.h`. Has to feel idiomatic per type
   (INTJ vs ENFP vs etc).
7. **Battery (cursor=10) stub** — still says "Coming soon". Translate or
   wire to actual battery view.

## File layout for resume

```
firmware/main/
  ui/theme.{h,cc}            Theme registry + palette + InkColor +
                              font accessors. **Single source of truth
                              for everything visual**.
  apps/
    home_menu/                Carousel home (assets::kHomeCoversByTheme)
    decision/decision_apps.cc Coin / Random10 / YesNo / Dice — model of
                              the polished pattern (themed title +
                              themed display + per-app ink color +
                              invalidate)
    answer_book/              Polished pattern applied
    ble_remote/               Polished pattern applied
    tools/tool_apps.cc        Muyu polished, Clock partial (weekday TODO)
    ritual/ritual_apps.cc     MBTI + Fortune **— NOT POLISHED YET**
    settings/settings_app.cc  Settings root + ThemePicker + About
                              (About labels TODO)
    wifi_setup/wifi_setup_app.cc  WiFi captive portal UI — **NOT
                              POLISHED YET**
  assets/
    home_covers.h             12 × 4 RGB565 covers (~3 MB)
    fonts/
      lxgw_wenkai_cjk_14.c    hint footers
      lxgw_wenkai_cjk_16.c    body / sub-result
      theme_<id>_title_28.c   per-theme brush, app titles
      theme_<id>_display_48.c per-theme brush, big result words
  data/
    answer_book_data.h        50 English phrases — translate
    mbti_data.h               16 types × (code + nickname + 8 prompts) —
                              translate
    fortune_data.h            DO/AVOID/lucky colors — translate
tools/
  gen_covers.py               renders 4×12 cover PNGs (PIL)
  png_to_lvgl_img.py          PNGs -> RGB565 header
  gen_cjk_font.py             LXGW WenKai subset, 14 + 16 px
  gen_themed_display_fonts.py 4 brush fonts subset, 28 + 48 px
  gen_muyu_pcm.py             Muyu sound (still relevant for tweaks)
```

## Binary state

- 5.22 MB / 7 MB factory partition (≈ 75% used).
- Bulk: 4 themes × 12 covers ≈ 3 MB pixel data. The brush font subsets
  total another ~1 MB across 8 .c files.
- Plenty of room for the remaining translations.

## Workflow notes that bit me today

- **LVGL widgets don't auto-repaint on style font change.** Need
  `lv_obj_invalidate(widget)` after `lv_obj_set_style_text_font`. Worse:
  if you set text BEFORE font, the old glyphs are cached. Always:
  `set_font → set_color → set_text → invalidate`.
- **Status bar lives on `lv_layer_top`**, separate from app screens.
  Doesn't pick up theme changes unless explicitly told. We added
  `status_bar_apply_theme()` callable from Settings.
- **Per-theme font subsets need every CN char to be in the symbol list.**
  Forgetting one char → tofu box in the rendered widget. The script is
  `tools/gen_themed_display_fonts.py::DISPLAY_SYMBOLS_BUCKETS`. Same for
  `tools/gen_cjk_font.py::SYMBOL_BUCKETS` (body/hint scale).
- **Full-width ？ / ， / 。** are NOT in the ASCII range and NOT in any
  brush subset right now. Either drop them from copy or add to the
  symbol list.
- **Flash hangs when monitor task holds the port.** TaskStop the
  monitor first, then `./flash.sh`, then restart monitor.
- **Device sleeps after 5 min idle in production build.** USB-CDC port
  disappears. User press BtnA to wake; port re-enumerates as the same
  `/dev/cu.usbmodem21101`.

## Fonts (gitignored — re-download from upstream)

Live under `<fonts-dir>/fonts/`:
- LXGW WenKai Regular — `https://github.com/lxgw/LxgwWenKai/releases/latest/download/LxgwWenKai-Regular.ttf`
- Ma Shan Zheng — `https://github.com/google/fonts/raw/main/ofl/mashanzheng/MaShanZheng-Regular.ttf`
- ZCOOL XiaoWei — `https://github.com/google/fonts/raw/main/ofl/zcoolxiaowei/ZCOOLXiaoWei-Regular.ttf`
- Liu Jian Mao Cao — `https://github.com/google/fonts/raw/main/ofl/liujianmaocao/LiuJianMaoCao-Regular.ttf`
- Long Cang — `https://github.com/google/fonts/raw/main/ofl/longcang/LongCang-Regular.ttf`

Python venv: `<fonts-dir>/.venv/` (Pillow).
Node tooling: `npx lv_font_conv` (auto-resolved by the generators).

## Next session entry point

Open `tasks/todo.md` + this file. Resume from priority #1 — Fortune.
Pattern to follow: copy the polished structure from
`apps/decision/decision_apps.cc` SimpleResultApp (themed title +
themed display + per-app ink color + invalidate + CN hints).
