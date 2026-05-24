# Pocket Oracle ESP32

> 一个开源硬件「口袋先知」复刻 — 基于 M5Stack StickS3 (ESP32-S3) 的轻决策包挂。

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
| 按键 | KEY1 (G11，侧面小按键，PMIC 电源/Boot) + KEY2 (G12，前面板大按键，用户按键) |
| 无线 | WiFi 2.4G + Bluetooth 5 LE |
| 电源 | USB-C + 250mAh 电池 + 板载 PMIC (M5PM1) |
| 红外 | TX/RX |
| 扩展 | HAT2-Bus 16P + Grove 4P |

## 功能（V1）

| 类别 | 应用 |
|---|---|
| 随机决策 | 答案之书 (350 条) · 翻硬币 · 掷骰子 (1/3/5/9) · 十以内 · 快速决策 |
| 仪式/运势 | MBTI 16 人格今日提示 · 每日运势（宜/忌/幸运色） |
| 实用工具 | 大字时钟 · 电子木鱼（计功德） |
| 蓝牙 HID | PPT 翻页器 / 相机快门 / 音乐控制 |

均**纯本地**运行，开机后无需联网（仅时钟需 WiFi 首次校时）。

## 交互

M5StickS3 只有 1 个用户按键（前面板大长条 = G11）加 IMU 摇一摇。
双击和长按被 PMIC 占用（关机 / boot），所以全部业务交互靠**单按 + 中等长按 + 摇一摇**。

| 手势 | 主菜单 | 应用内 |
|---|---|---|
| BTN 单按 | 进入选中 app | 主操作（翻页/摇一次/敲一下） |
| BTN 中等长按 (1~2s) | 锁屏 | 返回主菜单 |
| 摇一摇 | 下一个 app（光标移动） | 等同单按 / 模式切换 |

## 状态

🚧 **早期开发中**。当前进度见 [`tasks/todo.md`](tasks/todo.md)。

- [x] P0 仓库脚手架 + USB-CDC 控制台
- [ ] P1 硬件 bring-up
- [ ] P2 LVGL UI 框架
- [ ] P3-P7 应用与设置
- [ ] P8 低功耗
- [ ] P9 文档与打磨

## 构建

需要 [ESP-IDF v5.4.2](https://docs.espressif.com/projects/esp-idf/zh_CN/v5.4.2/esp32s3/get-started/index.html)。

```bash
cd firmware
./build.sh        # idf.py build
./flash.sh        # idf.py -p /dev/cu.usbmodem* flash
./monitor.sh      # idf.py monitor (Ctrl+]退出)
```

详细开发指南：[`docs/DEV_GUIDE.md`](docs/DEV_GUIDE.md)。

## 文档

- [PRD.md](PRD.md) — 产品需求与功能定义
- [docs/HARDWARE_M5STICKS3.md](docs/HARDWARE_M5STICKS3.md) — 引脚 / PMIC / 电源状态
- [docs/UI_DESIGN_LANGUAGE.md](docs/UI_DESIGN_LANGUAGE.md) — 视觉设计系统
- [docs/SERVICE_CONSOLE.md](docs/SERVICE_CONSOLE.md) — USB-CDC 控制台命令
- [docs/DEV_GUIDE.md](docs/DEV_GUIDE.md) — 开发者上手指南

## 许可

[MIT](LICENSE)。欢迎 fork、改进、做自己的版本。
