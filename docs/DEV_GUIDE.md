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

### 答案之书加新答案
编辑 `content/answers_zh.yaml`，加条目：
```yaml
- 顺其自然
- 别想太多，去做就好
```
重新生成头文件：
```bash
python tools/gen_answer_book.py
```
然后重新构建固件。

### MBTI / 运势同理（编辑 `content/mbti_zh.yaml` / `fortune_zh.yaml`）

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

- [ ] `sdkconfig`：关 USB-CDC 控制台（`CONFIG_ESP_CONSOLE_NONE=y`）
- [ ] `sdkconfig`：日志级别降到 WARN/ERROR
- [ ] 评估 Secure Boot v2 + Flash Encryption
- [ ] 续航实测 ≥ 设计目标
- [ ] 万用表测各电源状态电流
- [ ] BLE 与三大主流系统（iOS / Android / macOS）配对验证
- [ ] 长时间运行（≥ 48h）无内存泄漏（heap 命令对比）

## 参考资料

- [ESP-IDF v5.4.2 文档](https://docs.espressif.com/projects/esp-idf/zh_CN/v5.4.2/esp32s3/)
- [M5StickS3 官方文档](https://docs.m5stack.com/en/core/StickS3)
- [M5Unified GitHub](https://github.com/m5stack/M5Unified)
- [LVGL v9 文档](https://docs.lvgl.io/master/)
- [esp-nimble-cpp](https://github.com/h2zero/esp-nimble-cpp)
