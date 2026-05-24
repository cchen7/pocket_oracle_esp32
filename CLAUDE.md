# CLAUDE.md — Pocket Oracle ESP32

## What This Is

开源版「口袋先知」— 基于 M5Stack StickS3 (ESP32-S3) 的轻决策包挂玩具。
灵感来自少数派 × 思维重置的产品「口袋先知」，但完全开源、纯本地、可改造。

## Architecture

- `firmware/` — ESP-IDF v5.4.2 device firmware (C/C++)
- `docs/` — 硬件 / UI / 控制台 / 开发指南
- `content/` — 中文内容源（yaml，由 tools/ 编译为 .h 头文件）
- `tools/` — 内容生成脚本（Python）

## Hardware

**M5Stack StickS3** — ESP32-S3-PICO-1-N8R8, 8MB Flash + 8MB PSRAM。

| 部件 | 引脚 |
|---|---|
| LCD ST7789P3 (135×240) | MOSI=G39, SCK=G40, RS=G45, CS=G41, RST=G21, BL=G38 |
| IMU BMI270 + PMIC (I2C 共享) | SDA=G47, SCL=G48 |
| Audio ES8311 (I2S) | MCLK=G18, DOUT=G14, BCLK=G17, LRCK=G15, DIN=G16 |
| 按键 | KEY1=G11（侧面小按键，PMIC 电源/Boot）, KEY2=G12（前面板大按键，用户按键） |
| 红外 | TX=G46, RX=G42 |

详见 [`docs/HARDWARE_M5STICKS3.md`](docs/HARDWARE_M5STICKS3.md)。

## Build

```bash
cd firmware
source ~/Local/ESP32-proj/esp-idf-v5.4.2/export.sh
./build.sh
./flash.sh
./monitor.sh
```

## Dependency Policy

- **只用上游开源组件** — 不用中国公司二次包装
- Espressif 官方 + M5Stack 官方 + 主流开源（LVGL/NimBLE）OK
- 本地 `firmware/components/` 已固定：fonts (CJK 字体)、wifi-connect (AP+DNS+HTTP 配网)
- managed_components 由 `idf_component.yml` 拉取（不进 git）

## Key Design Decisions

- **V1 无 OTA**（factory 单 app 分区，后续 USB 烧录）
- **V1 默认开 USB-CDC 控制台**（开发期诊断；量产前 sdkconfig 关掉）
- **仅 KEY1 (G11) 深睡唤醒**（不做 IMU 摇一摇唤醒，省 PMIC IRQ 路由）
- **PSRAM 必须 Octal 模式**（PICO-1-N8R8 是 Octal，配 Quad 会砖）
- **UI 用 LVGL v9 + esp_lvgl_port，M5GFX 做 flush backend**
- **BLE 用 NimBLE**（不是 Bluedroid，省 ~50KB RAM）
- **纯本地内容**（答案/MBTI/运势数据编译进固件，无后端依赖）

## UI Design Rule

LCD 是 135×240 IPS 彩屏，素质高。UI 要**精雕细琢**：
- 用 LVGL v9 双缓冲 FULL render（PSRAM 装 framebuffer）
- 字体走 Puhui Sans + 大字号 glyph 子集（数字/标点/高频汉字另存）
- 色板限定 7 个语义色（详见 `docs/UI_DESIGN_LANGUAGE.md`）
- 8px 基线网格、`ease-out-cubic 300ms` 默认动效

不要：
- 山寨"科技感"风格（霓虹蓝、矩阵字体）
- 默认 LVGL 控件直接堆砌不调样式
- 字体糊（必须 anti-aliased）

## File Conventions

- 头文件 `.h`，C++ 实现 `.cc`（与 atoms3r 项目一致）
- 一律 UTF-8 无 BOM
- 日志 tag 短且稳定：`MAIN`, `UI`, `INPUT`, `POWER`, `BLE`, `APP_<NAME>`
- 函数命名 snake_case，类 CamelCase
- 中文注释 OK，但 commit message 用英文

## Out of Scope (V1)

- OTA、NFC、自定义壁纸、多语言扩展、手机 App、Secure Boot — 见 plan.md
