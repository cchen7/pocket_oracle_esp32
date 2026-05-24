# Checkpoint - 2026-05-25

> Local snapshot only. **Do not push** to GitHub until next session.

## Today's Progress (P0 → P5 in flight)

| Phase | Status | Last commit |
|---|---|---|
| P0 Scaffold + USB-CDC | ✅ done | `f726ccb` |
| P1 Hardware bring-up | ✅ done | `006db56` |
| P1 Buttons identified | ✅ done | `fb102f4` |
| P2 LVGL pipeline | ✅ done | `d94c7d5` |
| P2 Input + router + home + status | ✅ done | `a0b2fa1` |
| P3 5 decision apps | ✅ done | `05c38a8` |
| P4 MBTI + Fortune | ✅ done (pushed) | `a0f24a0` |
| **P5 Tools** | **🟡 in progress** | uncommitted |
| P6 BLE HID | ⏳ pending | — |
| P7 Settings + WiFi | ⏳ pending | — |
| P8 Low power | ⏳ pending | — |
| P9 Polish + CJK fonts | ⏳ pending | — |

## P5 State (uncommitted)

Code in working tree but not yet committed:

- `firmware/main/storage/stats.{h,cc}` — NVS u32 counter helper (works)
- `firmware/main/apps/tools/tool_apps.{h,cc}` — Clock + Muyu apps
- `firmware/main/apps/home_menu/home_menu.cc` — wired cursor 7=Clock, 8=Muyu

### Verified on hardware today:
- ✅ Clock app: HH:MM big-font + date below. Shows uptime-based time until SNTP lands.
- ✅ Muyu app: tap/shake increments merit counter, persists across reboots via NVS (commit every 5 strikes).

### Open Muyu issue (need to re-test tomorrow):
- 喇叭只有电流声，没有清脆 click. First attempt at 800Hz/30ms failed; bumped to 800Hz/120ms + `M5.Speaker.setVolume(160)` but **not yet verified on hardware**.
- Possible fixes if 120ms+vol still doesn't work:
  - Speaker amp needs explicit enable (`M5.Speaker.begin()` or similar)?
  - Try different freq (1500Hz?) — wooden fish sound is sharper than 800Hz
  - Check if PMIC L3B power gating speaker amp when idle (would explain why static-only)

## Hardware Truths Locked Down

Things we discovered on real hardware that ripple through PRD/code:

1. **No discrete RTC chip on M5StickS3** (PMIC has no RTC either). Time relies on SNTP after WiFi up; lost on PMIC power-off.
2. **Only 1 user button (G11 / front bar)** + 1 PMIC side button (left small, handles power/boot) + 1 decorative/IR-window button (right medium, electrically not connected).
3. **PSRAM must be Octal mode** for PICO-1-N8R8. Quad config = brick.
4. **PM_ENABLE + USB-Serial-JTAG conflict** — auto light sleep tears down CDC. Disabled PM_ENABLE through P7; will re-enable in P8 with PM locks.
5. **M5GFX ships its own `<lvgl.h>` shim that conflicts with real LVGL types**. Fixed with `lvgl_alias/lvgl/lvgl.h` so M5GFX's `__has_include` check passes.
6. **lvgl ESP-IDF component has `LV_CONF_SKIP=y` default**, ignoring local lv_conf.h. Set `CONFIG_LV_CONF_SKIP=n` in sdkconfig.defaults to use project config.
7. **Shake threshold 1.35g + 400ms cooldown** is the right calibration for this stick form factor (hands-on tested by user).

## Next Session Start Points

1. **Verify Muyu sound** with current 120ms/vol-160 build. If still bad, investigate M5.Speaker init or try different freq.
2. **Commit P5** once Muyu is good.
3. Continue **P5.x**: add SNTP fetch (depends on WiFi which is P7 territory — may defer to P7 first).
4. Or jump to **P6 BLE HID翻页器** — independent of WiFi.

## Repo State Summary

- 7 commits pushed to `cchen7/pocket_oracle_esp32` main branch
- ~30 source files / ~2200 lines firmware
- Builds clean on ESP-IDF v5.4.2 + ESP32-S3
- ~900 KB binary, well within 7 MB factory partition

## Local-only Files (uncommitted)

```
firmware/main/storage/stats.{h,cc}
firmware/main/apps/tools/tool_apps.{h,cc}
firmware/main/apps/home_menu/home_menu.cc   (modified to wire P5)
firmware/main/CMakeLists.txt                (modified to add P5 sources)
```

These exist on disk but were intentionally not pushed at end of day per user request.
