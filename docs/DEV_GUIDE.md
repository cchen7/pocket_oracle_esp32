# Developer Guide

> 后续二次开发者上手指南。V1 不做 OTA，所以**功能改进 = 改源码 + USB 烧录**。

## 环境

需要：
- macOS / Linux / Windows
- Python 3.10+
- Git
- USB-C 数据线

## 安装 ESP-IDF v5.4.2

```bash
mkdir -p ~/esp && cd ~/esp
git clone -b v5.4.2 --recursive https://github.com/espressif/esp-idf.git esp-idf-v5.4.2
cd esp-idf-v5.4.2
./install.sh esp32s3
# 启用环境
source ./export.sh
idf.py --version
# 应输出 ESP-IDF v5.4.2
```

建议加到 `~/.zshrc`：
```bash
alias idf-env='source ~/esp/esp-idf-v5.4.2/export.sh'
```

## Clone 与首次构建

```bash
git clone https://github.com/cchen7/pocket_oracle_esp32.git
cd pocket_oracle_esp32/firmware
idf-env
./build.sh
```

首次构建约 5-10 分钟（拉 managed_components 包括 M5Unified、LVGL、NimBLE）。

## 烧录

接上 M5StickS3 USB-C，找端口：
```bash
ls /dev/cu.usbmodem*   # macOS
ls /dev/ttyACM*        # Linux
```

```bash
./flash.sh             # 默认自动检测端口
# 或指定
./flash.sh /dev/cu.usbmodem14101
```

## 监控日志 / 进 USB-CDC 控制台

```bash
./monitor.sh
# Ctrl + ] 退出
```

出现 `pocket> ` 提示符即可输命令。详见 [SERVICE_CONSOLE.md](SERVICE_CONSOLE.md)。

## 项目结构

```
firmware/main/
├── main.cc                 # app_main 入口
├── app/                    # 系统级（router、input、power、application）
├── apps/                   # 各业务 app（每个一文件夹）
├── ui/                     # 主题、控件、过渡
├── data/                   # 内容包（编译时确定）
├── ble/                    # NimBLE HID
├── storage/                # NVS 偏好/统计
├── console/                # USB-CDC 控制台命令
├── lvgl_port/              # LVGL 集成（flush/tick/task）
└── util/                   # 工具函数
```

## 加一个新 app

1. 在 `firmware/main/apps/<name>/` 建文件夹
2. 实现 `class XxxApp : public AppBase`：
   - `void onEnter()` — 进入时调用，构建 UI
   - `void onExit()` — 退出时调用，清理
   - `void onTick(uint32_t dt_ms)` — 每帧调用
   - `void onEvent(const InputEvent& e)` — 按键/摇一摇事件
3. 在 `apps/CMakeLists.txt` 加源文件
4. 在 `app/app_router.cc` 注册到主菜单
5. 在 `data/icons/<name>.png` 放 32×32 图标
6. 重新构建烧录

详细骨架见 `apps/answer_book/` 作为参考。

## 加新的内容

### 答案之书 / MBTI / 运势 加新条目

内容**直接写在头文件**里（不走 YAML 中间格式 — 简化构建链）：
- `firmware/main/data/answer_book_data.h` — 当前 560 条
- `firmware/main/data/mbti_data.h` — 16 类 × 8 prompts
- `firmware/main/data/fortune_data.h` — 宜 / 忌 / 幸运色

加完中文条目后**必须重新生成 LXGW 字体子集**（否则新字会显示成方框）：

```bash
# 自动从 data/*.h 抽 CJK 字符，与 UI bucket 合并生成
python tools/gen_cjk_font.py firmware/main/assets/fonts
```

然后正常 `./build.sh && ./flash.sh`。

### 改主题（封面 + 笔法字体）

```bash
# 重新渲染 4 主题 × 12 cover PNGs，写入 assets/home_covers.h
python tools/gen_covers.py

# 重新生成 4 主题 × {28px title, 48px display} 笔法字体
python tools/gen_themed_display_fonts.py firmware/main/assets/fonts
```

字体源文件（笔法字 + LXGW）放在 `<fonts-dir>/fonts/` —
.gitignore 排除（开源字体加上非开源备用，自取自管）。
DEV_GUIDE 第 1 节有下载链接。

### 工具脚本概览

| 脚本 | 用途 |
|---|---|
| `tools/gen_covers.py` | 4 主题 × 12 cover PNG → `assets/home_covers.h` (RGB565) |
| `tools/png_to_lvgl_img.py` | 通用 PNG → LVGL RGB565 头文件 |
| `tools/gen_cjk_font.py` | LXGW WenKai body 字 14/16px subset（**自动**包含 data/*.h 中所有 CJK） |
| `tools/gen_themed_display_fonts.py` | 4 主题 × 2 size 笔法字 subset |
| `tools/gen_muyu_pcm.py` | 木鱼 PCM 合成（噪声 + 阻尼正弦 + tanh 软压缩） |
| `tools/extract_data_chars.py` | 从 .h 抽双引号字符串里的 CJK 字（gen_cjk_font.py 自动调用） |

## 调试技巧

### 看堆栈余量
```
pocket> heap
pocket> tasks
```

### 改某个 tag 的日志级别（不用重烧）
```
pocket> log set BLE debug
pocket> log set INPUT verbose
```

### 看为什么 deep sleep 没生效
```
pocket> log set POWER debug
```
然后空闲等待，观察日志中 sleep 决策路径。

### Guru Meditation panic 怎么办
1. 复制完整 panic 输出
2. 找到 `Backtrace:` 行
3. 运行：
   ```bash
   xtensa-esp32s3-elf-addr2line -pfiaC -e build-pocket/pocket_oracle.elf <addr1> <addr2> ...
   ```
4. 对照源码找到崩溃位置

## CI / 验证

目前无 CI。手工 checklist：
- 构建无 warning
- 烧录后开机引导顺畅
- 12 个 app 各打开一次
- USB-CDC `status` 命令响应正常
- `heap` 命令显示余量 > 30%

## 贡献

PR 欢迎。请遵守：
- 一个 PR 解决一个事情（不要混 refactor + 新功能）
- commit message 用英文，jiquan 格式：`<topic>: short summary`
- 改 UI 必须遵守 [UI_DESIGN_LANGUAGE.md](UI_DESIGN_LANGUAGE.md)
- 改硬件相关代码需附带实测笔记

## 量产前 Checklist

- [x] `sdkconfig.defaults`：日志级别 `CONFIG_LOG_DEFAULT_LEVEL_WARN=y` + `CONFIG_LOG_MAXIMUM_LEVEL_INFO=y` (P9.11 完成 2026-05-31)
- [ ] 关 USB-CDC 控制台（`CONFIG_ESP_CONSOLE_NONE=y`）— 当前保留 USB-Serial-JTAG 以便用户烧录后能查看启动日志，关掉后省 ~20KB 但用户失去诊断渠道
- [ ] 评估 Secure Boot v2 + Flash Encryption（V2 范围）
- [ ] 续航实测 ≥ 设计目标
- [ ] 万用表测各电源状态电流
- [ ] BLE 与三大主流系统（iOS / Android / macOS）配对验证
- [ ] 长时间运行（≥ 48h）无内存泄漏（heap 命令对比）

## 想发布给其他用户？

参考 [FLASH_GUIDE.md](FLASH_GUIDE.md) — 给已经买了 StickS3 的终端用户的烧录步骤（不需要装 ESP-IDF）。

GitHub Releases 应附带：
- `bootloader.bin`
- `partition-table.bin`
- `pocket_oracle.bin`
- `FLASH_GUIDE.md` 链接

打包命令：
```bash
cd firmware/build-pocket
zip pocket_oracle_v1.0_stickS3.zip \
  bootloader/bootloader.bin \
  partition_table/partition-table.bin \
  pocket_oracle.bin
```

## 参考资料

- [ESP-IDF v5.4.2 文档](https://docs.espressif.com/projects/esp-idf/zh_CN/v5.4.2/esp32s3/)
- [M5StickS3 官方文档](https://docs.m5stack.com/en/core/StickS3)
- [M5Unified GitHub](https://github.com/m5stack/M5Unified)
- [LVGL v9 文档](https://docs.lvgl.io/master/)
- [esp-nimble-cpp](https://github.com/h2zero/esp-nimble-cpp)
