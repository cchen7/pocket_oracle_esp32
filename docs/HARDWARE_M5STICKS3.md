# Hardware — M5Stack StickS3

> 来源：[M5Stack 官方文档](https://docs.m5stack.com/en/core/StickS3)

## 基本规格

| 项 | 参数 |
|---|---|
| SoC | ESP32-S3-PICO-1-N8R8 (dual-core Xtensa LX7 @ 240MHz) |
| Flash | 8 MB (QIO @ 80MHz) |
| PSRAM | 8 MB Octal @ 80MHz |
| 屏幕 | 1.14" IPS LCD, ST7789P3 驱动, 135×240 |
| IMU | BMI270 (6 轴 accel + gyro，含 INT) |
| 音频 | ES8311 codec (I2S 24-bit) + AW8737 功放 + 8Ω 1W 喇叭 + MEMS mic |
| 按键 | KEY1 (G11), KEY2 (G12) |
| 红外 | TX (G46), RX (G42) |
| 无线 | 2.4G WiFi 4 + Bluetooth 5 LE + Mesh |
| 电源 | USB-C 5V + 250mAh 电池 + PMIC |
| 扩展 | HAT2-Bus 2.54mm 16P + Grove HY2.0 4P |
| 尺寸 | 48 × 24 × 15 mm |
| 重量 | 20 g |

## 引脚映射

### LCD (ST7789P3, 135×240)

| ESP32-S3 GPIO | 功能 |
|---|---|
| G39 | MOSI |
| G40 | SCK |
| G45 | RS / DC |
| G41 | CS |
| G21 | RST |
| G38 | BL（背光，可 PWM 调亮度） |

### I2C 共享总线（IMU + PMIC + ES8311 控制）

| ESP32-S3 GPIO | 功能 |
|---|---|
| G47 | SDA |
| G48 | SCL |

I2C 设备地址：
- BMI270 IMU = **0x68**
- PMIC (M5PM1) = **0x6E**
- ES8311 codec = **0x18**

⚠️ **三个设备共享一条 bus**。任何自定义驱动必须复用 M5Unified 的 `M5.In_I2C` 实例，不要新建 `i2c_master_bus`，否则总线冲突。

⚠️ **板上没有独立 RTC 芯片**（PMIC 也无 RTC 功能）。时钟依赖 SNTP；ESP32-S3 内部 RTC 可在 deep sleep 间保时，**但 PMIC 关机 (L0) 会丢失，重新开机后需要 WiFi 校时**。

### I2S Audio (ES8311)

| ESP32-S3 GPIO | 功能 |
|---|---|
| G18 | MCLK |
| G14 | DOUT (麦克风) |
| G17 | BCLK |
| G15 | LRCK / WS |
| G16 | DIN (扬声器) |

### 按键

M5StickS3 只有 **1 个用户可编程按键**，加 1 个硬件复位键：

| ESP32-S3 GPIO | 标识 | 物理位置 | 功能 | RTC IO? |
|---|---|---|---|---|
| G11 | M5 BTN | **前面板正下方大长条**（屏幕正下方） | 用户按键。PMIC 同时监听同一信号：**单按=ON/wake、双按=OFF、长按=BOOT 模式** | ✅ |
| — | EN/RST | 右侧中等按键 | 硬件复位（拉低 EN 直接重启 ESP32-S3） | — |

⚠️ 由于 PMIC 抢占 double-press 和 long-press，业务层**只能用单按和中等长按 (1-2s)**。**双击不能作业务手势**（会关机）。
⚠️ G12 在 M5Unified 代码里读取，但 M5StickS3 实际没接按键，因此 `M5.BtnB` 永远不会触发。
⚠️ V1 的全部业务交互都靠 **G11 单按 + 摇一摇（BMI270 shake）**。

G11 是 RTC IO，可作深睡 EXT0 唤醒源。

### 红外

| GPIO | 功能 |
|---|---|
| G46 | IR TX |
| G42 | IR RX |

⚠️ **使用 IR 接收时必须关掉喇叭功放**（板载耦合）。

### PMIC（M5PM1，I2C 0x6E）

PMIC 内部 PYG0-PYG4 5 路 GPIO：

| PYG | 用途 |
|---|---|
| PYG0 | Battery Charge Status（充电中输出） |
| PYG1 | IRQ → ESP32-S3 中断 |
| PYG2 | L3B Power EN（控制 LCD/MIC/SPK 电源） |
| PYG3 | Speaker Pulse |
| PYG4 | IMU INT 输入（连接 BMI270 INT1） |

## 电源状态（PMIC 控制）

| 级别 | 典型电流 | 状态 |
|---|---|---|
| OFF | 14.02 µA | PMIC 自身 + 电池监控，ESP32 完全断电 |
| L1 | 52.47 µA | PMIC + IMU 供电（IMU 监测唤醒）；ESP32 仍 off |
| L2 | 102.40 µA | PMIC + IMU + ESP32 deep/light sleep |
| L3A | 36.69 mA | ESP32 active（modem off），LCD/MIC/SPK 关 |
| L3B | 80-200 mA | 全部外设亮（LCD 全亮 + 麦克风 + 喇叭） |

完全关机：KEY1 长按 ≥5s → `M5.Power.powerOff()` → 进 OFF。

## 深睡唤醒（V1 仅 KEY1）

V1 决策：**仅 KEY1 (G11) EXT0 RTC GPIO 唤醒**。

不做摇一摇唤醒（涉及 BMI270 INT → PMIC PYG4 → PMIC PYG1_IRQ → ESP32 GPIO 路由 + PMIC 寄存器读取识别中断源，复杂度高）。

```c
// 进入 deep sleep 前
esp_sleep_enable_ext0_wakeup(GPIO_NUM_11, 0);  // KEY1 按下 = 拉低唤醒
esp_deep_sleep_start();
```

## USB-C

- USB-C 5V 输入 → PMIC 充电
- USB-Serial-JTAG 内置（不占额外 GPIO，可同时跑 CDC 控制台）
- 不支持 USB HID 上行（用 BLE HID 替代）

## 扩展口

### HAT2-Bus（顶部 16P 排针，2.54mm）

| Pin | 信号 | Pin | 信号 |
|---|---|---|---|
| 1 | GND | 2 | G5 |
| 3 | EXT_5V | 4 | G4 |
| 5 | Boot | 6 | G6 |
| 7 | G1 | 8 | G7 |
| 9 | G8 | 10 | G43 |
| 11 | BAT | 12 | G44 |
| 13 | 3V3_L2 | 14 | G2 |
| 15 | 5V_IN | 16 | G3 |

### Grove PORT.A（HY2.0 4P）

| 颜色 | 信号 |
|---|---|
| Black | GND |
| Red | 5V (可配输入/输出) |
| Yellow | G10 (默认 I2C SDA) |
| White | G9 (默认 I2C SCL) |

⚠️ Grove 默认输入模式（外部供 5V）。配置为输出时只能 USB 或 HAT2-Bus 5V_IN 供电，不可双向。

## 注意事项

- 电池供电时，喇叭音量不要超过 75%，否则瞬时电流过大可能复位
- IR 接收期间务必关喇叭功放
- PSRAM 是 **Octal 模式**（PICO-1-N8R8），sdkconfig 必须 `CONFIG_SPIRAM_MODE_OCT=y`，配 Quad 会卡 PSRAM 初始化
