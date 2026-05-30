# Pocket Oracle ESP32

> 开源「口袋先知」复刻 — 基于 M5Stack StickS3 (ESP32-S3) 的轻决策包挂。
> 国风水墨 UI，纯本地运行。

灵感来自少数派 × 思维重置「[口袋先知](https://sspai.com/create/rand0)」。
目标：用开源硬件 + 开源固件，做一个能日常陪伴、辅助轻决策、还能当蓝牙翻页器的小玩具。

## 硬件

**[M5Stack StickS3](https://docs.m5stack.com/en/core/StickS3)** — ¥150 左右一只，48×24×15mm，250mAh 内置电池。

| 部件 | 规格 |
|---|---|
| MCU | ESP32-S3-PICO-1-N8R8 (8MB Flash + 8MB PSRAM, dual-core 240MHz) |
| 屏幕 | 1.14" IPS LCD 135×240 ST7789P3 |
| IMU | BMI270 (6 轴，含 shake/anymotion 中断) |
| 音频 | ES8311 编解码 + AW8737 功放 + MEMS 麦 |
| 按键 | **BtnA**=G11 前面板大长条 + **BtnB**=G12 右侧小键 + PMIC=左侧小键（电源/Boot） |
| 无线 | WiFi 2.4G + Bluetooth 5 LE |
| 电源 | USB-C + 250mAh 电池 + 板载 PMIC (M5PM1) |

详细引脚：[`docs/HARDWARE_M5STICKS3.md`](docs/HARDWARE_M5STICKS3.md)

## 功能（V1）

12 个 app，单卡 carousel 主菜单（水墨封面 + 4 套主题切换）：

| 类别 | 应用 | 备注 |
|---|---|---|
| 随机决策 | 答案之书 · 翻硬币 · 掷骰子 · 十以内 · 是否 | 答案池 560 条中文（含 dengbuqi 源 172 条 + 古风 388 条）|
| 仪式/运势 | MBTI 16 人格今日提示 · 每日运势（宜/忌/幸运色）| MBTI 128 条中文 prompts，按类型语气调适 |
| 实用工具 | 大字时钟 · 电子木鱼（计功德）· 电量页面 | 木鱼 PCM 合成音；时钟用 SNTP 校时 |
| 蓝牙 HID | PPT 翻页器 | 用 arrow keys，支持 iOS/macOS Keynote |
| 设置 | 主题切换 · 无线网络 · 关于 | WiFi 走 captive portal 配网 |

**4 套国风主题**：水墨 / 绢本 / 竹简 / 拓片，各自不同笔法字体 + 纸纹背景。
**9 种印章色（朱砂/青墨/黛蓝/赭石/黛紫/兰花紫/松绿/苍翠/远墨）**：每个 app 有稳定签名色，跨主题不变。

均**纯本地**运行，开机后无需联网（仅时钟需 WiFi 首次校时）。

## 交互（2 按键 + 摇一摇）

| 手势 | 主菜单 | 应用内 |
|---|---|---|
| **BtnA** 短按 | 下一卡片 | 主操作（翻签/换光标/敲一下）|
| **BtnB** 短按 | 进入选中 app | 次操作（确认/换型/+1）|
| 摇一摇 | = BtnA | = BtnA |
| 长按（任意键 ~1s） | 锁屏 | 返回主菜单 |
| PMIC 键（左侧）| **不要用** — 厂家占用：双击关机 / 长按 6s 进 BOOT | 同左 |

PMIC 双击=OFF / 长按=BOOT 硬件级别占用，业务层无法用，所以全部交互靠 2 个用户键 + 摇。

## 状态

✅ **V1 内容完整**。当前进度见 [`tasks/todo.md`](tasks/todo.md)、最新快照 [`CHECKPOINT_2026-05-31.md`](CHECKPOINT_2026-05-31.md)。

- [x] P0 仓库脚手架 + USB-CDC 控制台
- [x] P1 硬件 bring-up（2 按键模型）
- [x] P2 LVGL UI 框架（PSRAM 双缓 + M5GFX flush）
- [x] P3 决策 5 app
- [x] P4 MBTI + Fortune
- [x] P5 Clock + Muyu
- [x] P6 BLE HID 翻页器
- [x] P7 Settings + WiFi captive portal + SNTP
- [x] P8 低功耗（30s dim / 60s blank / 5min deep sleep）
- [x] P9 UI overhaul + 全部内容 CN 化 + 字体子集自动化
- [ ] P9.8-12 文档 / 截图 / demo gif / v1.0 release tag
- [ ] P10 推送 + GitHub release

## 安装（终端用户）

已有 StickS3，想直接烧入体验：**[docs/FLASH_GUIDE.md](docs/FLASH_GUIDE.md)** — 5-10 分钟，只需 Python + esptool，不需要 ESP-IDF。

GitHub Releases 提供 `.bin` 文件下载。

## 编译（开发者）

需要 [ESP-IDF v5.4.2](https://docs.espressif.com/projects/esp-idf/zh_CN/v5.4.2/esp32s3/get-started/index.html)。

```bash
cd firmware
./build.sh        # idf.py build
./flash.sh        # idf.py -p /dev/cu.usbmodem* flash
./monitor.sh      # idf.py monitor (Ctrl+]退出)
```

完整开发指南：**[docs/DEV_GUIDE.md](docs/DEV_GUIDE.md)** — 环境 / 项目结构 / 加 app / 加内容 / 字体生成 / 主题定制。

## 文档

- [PRD.md](PRD.md) — 产品需求与功能定义
- [docs/FLASH_GUIDE.md](docs/FLASH_GUIDE.md) — 终端用户烧录指南
- [docs/DEV_GUIDE.md](docs/DEV_GUIDE.md) — 开发者上手指南
- [docs/HARDWARE_M5STICKS3.md](docs/HARDWARE_M5STICKS3.md) — 引脚 / PMIC / 电源状态
- [docs/UI_DESIGN_LANGUAGE.md](docs/UI_DESIGN_LANGUAGE.md) — 视觉设计系统
- [docs/SERVICE_CONSOLE.md](docs/SERVICE_CONSOLE.md) — USB-CDC 控制台命令

## 二进制预算

5.62 MB / 7 MB factory partition (77% 满，剩 1.66 MB)。
主要占用：4 主题 × 12 covers RGB565 ≈ 3 MB；4 主题 × 2 size 笔法字 ≈ 3 MB；LXGW body 14+16px ≈ 1.1 MB。

## 许可

[MIT](LICENSE)。欢迎 fork、改进、做自己的版本。

**字体非开源备份**：笔法字（马善政 / ZCOOL 晓伟 / 流坚毛草 / 龙藏）和 LXGW WenKai 楷书源 ttf 文件**不入仓库**，从 release 链接自取（DEV_GUIDE 第 1 节 + tools 脚本顶部都有 URL）。
