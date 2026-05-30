# Checkpoint — 2026-05-31 (P9 V1 内容全 CN，完结)

> Local-only. **27 commits ahead of origin/main, not pushed.**
> Previous checkpoint: `CHECKPOINT_2026-05-30.md`.

## Where we are

P0–P9 base 全部完成。本次 (2026-05-31) 在昨天 P9 UI overhaul 之上把所有剩余内容全 CN 化、补齐 Battery 真页面、把 font subset 改成自动化。

| Phase | Status | Notes |
|---|---|---|
| P0–P5 base | ✓ | Decision/MBTI/Fortune/Clock/Muyu |
| P6 BLE HID Remote | ✓ | iOS arrow keys, bonded, deinit on exit |
| P7 Settings + WiFi + SNTP | ✓ | Captive portal "caption" SSID |
| P8 power | ✓ | 30s dim / 60s blank / 5min deep sleep + BtnA wake |
| **P9 UI overhaul + 内容 CN** | ✓ | 4 主题 / Carousel / 9 ink colors / 394 答案 / 128 MBTI prompts |
| P9.8-12 docs/release | ⏳ | 文档 + 发布工序，未启动 |

## 本次新增 (5 个 commit)

```
46467f1 P9.7 Battery: real status page replaces 'Coming soon' stub
3011d29 P9.5/P9.6: 394 CN answers + 128 CN MBTI prompts + font auto-extract
579e6f9 P9.1-P9.4: Fortune/Settings/About/WiFi/MBTI CN polish
3965a47 Checkpoint 2026-05-30: P9 UI overhaul in flight
... (5月30日及之前的 22 commits)
```

## 本次具体修改

### P9.1 Fortune
- `fortune_data.h`: DO/AVOID 20+20 EN → CN (启程/聆听/.../急躁/攀比/...)
- 12 lucky color names CN (朱砂/珊瑚/琥珀/橄榄/苍青/鸭青/天青/靛蓝/紫绛/蔷薇/缃色/月白)
- `ritual_apps.cc::FortuneApp` 重写：宜/忌 双列 themed-title 28 px 笔法 + 松绿/朱砂 InkColor + result body 16 px LXGW + 幸运色块 + 中文 hint

### P9.2 Clock / About / WiFi
- `tool_apps.cc::ClockApp` 周几 EN → 周日..周六
- `settings_app.cc::AboutApp` fw/wifi/ip/free/uptime → 固件/网络/IP/内存/运行；wifi 状态 off/no WiFi/connecting/failed → 关闭/未配置/连接中/失败
- `wifi_setup_app.cc` 标题 "WiFi Setup" → "无线配网"；指引 "用手机连接此 WiFi 热点 / 弹出网页填家庭密码"；hint "长按返回"；状态 "已保存 正在重连" / "配网启动失败"

### P9.3 MBTI chrome
- code label 颜色 accent_main → 兰花紫
- hint EN → "短按换签  侧键换型  长按返回"

### P9.4 Themed title 补齐
- Settings 根 / Theme picker / About / WiFi Setup 4 个标题从 `font_title()` (Montserrat ASCII-only — 渲染中文是方块) 切到 `font_title_themed()` + 远墨 + `lv_obj_invalidate`
- **关键 bug**：用户先发现 "无线配网" 显示成 4 个 ∞ 方块，定位到 font_title 不支持 CJK。同时 About/Settings/ThemePicker 也都有同样问题，本次合并修

### P9.5 Answer Book
- `answer_book_data.h` 50 EN → **394 条 CN**，七类语气：肯定/否定/中性/引导/行动/警示/古风
- `answer_book.cc` 标题脱主题：`font_title_themed` → `font_body`（用户要求 "省内存"），其它不变
- 用户原话 "和 MBTI 一起结合起来考虑"：两批内容一次性入库，单次字体重生

### P9.6 MBTI 128 prompts
- `mbti_data.h` 16 类 × 8 prompts 全 CN
- 翻译策略：保留每个类型的语气特征（INTJ 简洁决断、INFP 温和、ENTP 跳脱等）
- 16 个 nickname 也 CN 化（Architect → 谋略家 / Logician → 思辨者 / Commander → 主帅 / ...）

### P9.+ Font 自动化
- 新 `tools/extract_data_chars.py`：扫 .h 文件抽双引号字符串中的 CJK 字（U+3000–303F / U+4E00–9FFF / U+3400–4DBF / U+FF00–FFEF）
- `tools/gen_cjk_font.py` 改成：UI SYMBOL_BUCKETS + auto extract from `data/{answer_book,mbti,fortune}_data.h`，dedupe，sort，传给 lv_font_conv
- **意义**：以后翻译新内容只需写 .h 里的字符串，**不再需要手维护 SYMBOL_BUCKETS**。从今往后字漏了 = 漏掉的字面量在 .h 里没出现，根本原因更好排查
- 子集从 220 → **733 字**（690 来自数据文件），LXGW 14 px 488 KB / 16 px 606 KB

### P9.7 Battery
- 替换 home cursor=10 的 "Coming soon" 桩
- 新 `BatteryApp` in tool_apps.cc：标题 "电量" 远墨笔法 + 大字 % + 副线 "充电中 / 放电 + 电压"
- < 25 % 时 % 变 朱砂警示
- M5.Power.getBatteryLevel / getBatteryVoltage / isCharging
- 修正了 home_menu.cc default 分支的 `-Werror=array-bounds`：原 `stub_app(kStubLabels[idx])` 在 case 10/11 都被显式覆盖后，编译器认为 default 不可达但仍做边界推断，改 `stub_app("?")` 绕过

## Binary

- **5.62 MB / 7 MB factory** (77 % 满，剩 1.66 MB)
- 主要占用：
  - 4 主题 × 12 covers RGB565 ≈ 3 MB
  - 4 主题 × 2 size 笔法字 ≈ 3 MB
  - LXGW body 14+16 px ≈ 1.1 MB
  - 答案/MBTI/Fortune 字符串字面量 ≈ 25 KB（可忽略）
- 还能塞 ~1.6 MB（足够再加几百条内容或第 5 套主题）

## 设计原则记录

- **per-app ink stable，per-theme brush rotates**：用户翻主题不丢失 app 身份感
- **themed font 只覆盖 UI chrome（标题 + 结果词），不覆盖内容池**：内容池字符量大，per-theme × bpp4 会爆。AnswerBook/MBTI 内容都走 LXGW 一套
- **font 必先于 text + invalidate**：LVGL 不会自动 invalidate 字体改动
- **font_title() 是 ASCII-only Montserrat，CJK 标题必须 font_title_themed() 或 font_body()**：这个坑在本次 P9.4 才彻底清理

## 剩余 (P9.8-12 + 推送)

按 todo.md：
1. docs/DEV_GUIDE.md 完整版
2. docs/UX_FLOWS.md 配截图  
3. README + demo gif
4. 量产前 checklist（关 USB-CDC + 关日志）
5. v1.0 tag
6. **推送 27 commits 到 origin/main**（用户决定时点）

## Workflow notes that bit me today

- **`-Werror=array-bounds` 是新 GCC 14 + ESP-IDF 5.4 引入的更严格分析**：default 分支即使逻辑不可达也会被分析。修法：default 不索引数组
- **font subset 自动化的重要前置**：要保证 .h 里所有"会被渲染"的字符串都在双引号内（不是从 sprintf 拼）。当前所有内容都是 const char* 数组，符合
- **每次烧录前要等设备醒**：300s deep sleep + USB-CDC 断口。用户需按 BtnA。也可以改 kSleepMs 临时拉长，但生产值 5 min 是用户选的

## File layout (重要文件无变化，仅补充)

```
firmware/main/apps/tools/tool_apps.{h,cc}
  + BatteryApp                       新增电量页面
firmware/main/apps/home_menu/home_menu.cc
  default → stub_app("?")            绕过 array-bounds 分析
tools/
  extract_data_chars.py              新；scan .h for CJK literals
  gen_cjk_font.py                    改；union UI buckets + extracted
firmware/main/data/
  answer_book_data.h                 50 EN → 394 CN
  mbti_data.h                        128 EN → 128 CN + 中文绰号
  fortune_data.h                     EN → CN (5月30日已落)
```

## Next session entry point

读 `tasks/todo.md` + 本文件。下一步走 P9.8（DEV_GUIDE）或 P10.1（推送）。
推送前最好先回顾一次 git log，本地 27 commits 都没传过远端，到 GitHub 会一次出现一大批。
