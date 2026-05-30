# Pocket Oracle ESP32 — 烧录指南（终端用户）

> 给已经买了 **M5Stack StickS3**、想直接烧入本固件体验的用户。
> 不需要装 ESP-IDF / 编译工具链；只需要 Python 3 + esptool。
> 5-10 分钟搞定。

## 你需要

- M5Stack StickS3 一台
- USB-C 数据线（**必须支持数据**，纯充电线不行）
- 一台电脑（macOS / Linux / Windows）
- Python 3.10 或更高

## 1. 装 esptool

```bash
pip install --upgrade esptool
# 或者 (推荐隔离环境):
python3 -m pip install --user esptool
```

验证：
```bash
esptool --version
# esptool.py vX.X.X
```

## 2. 下载 release 固件

到本仓库 GitHub Releases 页面下载最新的 `pocket_oracle_v*.zip`，解压得到 3 个文件：

```
bootloader.bin       (~20 KB)
partition-table.bin  (~3 KB)
pocket_oracle.bin    (~5.6 MB)
```

把它们放到同一个目录下。

## 3. 连接 StickS3

用 USB-C 线连接 StickS3 和电脑。

**找设备串口号**：

- **macOS** ：`ls /dev/cu.usbmodem*`  → 类似 `/dev/cu.usbmodem21101`
- **Linux** ：`ls /dev/ttyACM*`  → 类似 `/dev/ttyACM0`
- **Windows** ：设备管理器 → 端口（COM 和 LPT）→ "USB 串行设备 (COM3)"

**如果没找到设备**：
- StickS3 进入深睡眠时 USB 不枚举，按一下前面板大长条键（BtnA）唤醒
- 仍找不到 → 换数据线 / 换 USB 口

## 4. 烧录

进入 release 文件所在目录，执行（替换 `<PORT>` 为你的串口号）：

```bash
esptool --chip esp32s3 -p <PORT> -b 460800 \
  --before default_reset --after hard_reset write_flash \
  --flash_mode dio --flash_size 8MB --flash_freq 80m \
  0x0     bootloader.bin \
  0x8000  partition-table.bin \
  0x10000 pocket_oracle.bin
```

成功输出末尾：
```
Wrote 5509248 bytes (1648508 compressed) at 0x00010000 in 32.3 seconds (effective 1365.9 kbit/s)...
Hash of data verified.
Leaving...
Hard resetting via RTS pin...
```

烧完后 StickS3 自动重启，屏幕进入主菜单（水墨封面）。

## 5. 第一次使用

- **BtnA**（前面板大长条）：切换 app 卡片 / app 内"重新抽签"等
- **BtnB**（右侧小键）：进入 app / app 内"确认"或第二动作
- **长按 BtnA 或 BtnB ~1 秒**：从 app 退回主菜单
- **PMIC 键**（左侧小键）：长按 6 秒关机；不要双击（厂家保留）

**配 WiFi（可选）**：
1. 主菜单滚到 "设置" → 进入
2. "无线网络" → 设备启 SoftAP "PocketOracle-XXXX"
3. 手机 WiFi 连接此热点（开放，无密码）
4. 弹出 captive portal 网页填家庭 WiFi 名 + 密码 → 保存
5. 设备连上后顶部状态栏开始显示真实时间（SNTP 同步）

WiFi 不配也能用，只是时钟显示 uptime 不是 wall time。

## 6. 故障排查

| 现象 | 原因 / 处理 |
|---|---|
| esptool 报 `serial port not found` | 串口号错；重新 `ls /dev/cu.usbmodem*` |
| esptool 报 `Failed to connect` | 烧录时手动按住 PMIC 键 6+ 秒进入 BOOT 模式后再烧 |
| 屏幕黑屏 | 长按 PMIC 关机 → 短按开机 |
| 屏幕 30s 后变暗、60s 后全黑 | 正常省电（dim → blank）；按任意键恢复 |
| 5 分钟无操作完全断电 | 深睡眠；按 BtnA 唤醒（约 3 秒启动）|
| 串口 monitor 看不到日志 | 生产固件 log level = WARN，不输出 INFO/DEBUG。需要详细日志请用源码版本自己编译 |
| 蓝牙翻页配对失败 | iPhone 需要先在"设置→蓝牙"里"忽略此设备"再重新配对 |

## 7. 想自己改源码？

去看 [`docs/DEV_GUIDE.md`](DEV_GUIDE.md) — 编译环境搭建 + 项目结构 + 主题/字体生成。

## 8. 反馈

issues 请到本仓库 GitHub Issues 提交。附上：
- StickS3 硬件批次（看包装条码末几位）
- 固件版本（设置 → 关于 → 固件字段）
- 复现步骤
