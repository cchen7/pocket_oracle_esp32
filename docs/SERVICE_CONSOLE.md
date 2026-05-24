# USB-CDC Service Console

> 开发期默认启用的设备终端。通过 USB-C 接 Mac/PC，直接进 shell 调试，无需重烧。

## 启用方式

`sdkconfig.defaults.esp32s3`：
```ini
CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG=y
```

ESP32-S3 内置 USB-Serial-JTAG，不占额外 GPIO，与烧录共用一个 USB-C。

## 连接

```bash
# macOS
ls /dev/cu.usbmodem*
screen /dev/cu.usbmodem* 115200
# 或 idf.py monitor
```

提示符：`pocket> `

## 命令清单（P0 实装 help/version/reboot；其他逐步加）

| 命令 | 用途 |
|---|---|
| `help` | 列出所有命令 + 简短说明 |
| `version` | 固件版本、构建时间、IDF 版本 |
| `status` | uptime、电池、BLE 连接、当前 app、堆栈余量 |
| `bat` | 电池电压 (V) + 充电状态 + 估算百分比 |
| `heap` | 总堆 / 当前剩余 / 最少历史剩余（internal & PSRAM 分开） |
| `tasks` | FreeRTOS 任务列表 + 状态 + stack 高水位 |
| `log set <tag> <level>` | 运行时改某个 tag 的日志级别 (none/error/warn/info/debug/verbose) |
| `log list` | 列出当前所有 tag + 级别 |
| `brightness <0-100>` | 设置 LCD 亮度 |
| `volume <0-100>` | 设置喇叭音量 |
| `app go <name>` | 强制切到指定 app（debug 用） |
| `app list` | 列所有可用 app |
| `ble status` | BLE 广播 / 连接状态 / 配对设备 |
| `ble disconnect` | 主动断开 BLE |
| `wifi scan` | 扫 WiFi |
| `wifi connect <ssid> <pwd>` | 连接（仅 debug 用，正式用 AP 配网） |
| `wifi status` | 当前 WiFi 连接状态 |
| `nvs reset` | 清空所有 NVS（恢复出厂） |
| `nvs get <ns> <key>` | 读 NVS 项 |
| `reboot` | 软重启 |
| `poweroff` | 完全关机（M5.Power.powerOff） |

## 实现要点

- 用 ESP-IDF 自带 `esp_console` + `linenoise`（自带 history + autocomplete）
- 控制台 task 优先级低（不抢 UI/BLE）
- 命令处理函数必须**短**（< 100ms），不能阻塞 UI
- 涉及 LVGL 的命令必须用 `lvgl_lock/unlock` 保护
- 不暴露写 flash / NVS 危险操作的命令（除 `nvs reset` 外）

## 量产前关闭

`sdkconfig.defaults.esp32s3`（量产分支）：
```ini
# CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG=y  ← 注释掉
CONFIG_ESP_CONSOLE_NONE=y
```

或 menuconfig：Component config → ESP System Settings → Channel for console output → No output。

关掉后控制台不占任何 CPU/RAM，串口完全静默。
