# Pocket Oracle ESP32 — TODO

> Updated: 2026-05-24

整体计划见 plan.md（在 Claude 会话内）。本文件追踪逐阶段执行进度。

---

## Phase 0 — 仓库脚手架 + USB-CDC 控制台

- [x] **P0.1** 创建仓库目录结构 (2026-05-24)
- [x] **P0.2** 顶层文档：README / CLAUDE.md / PRD.md / LICENSE / .gitignore (2026-05-24)
- [x] **P0.3** 子文档：docs/HARDWARE / UI_DESIGN_LANGUAGE / SERVICE_CONSOLE / DEV_GUIDE (2026-05-24)
- [x] **P0.4** 固件骨架：CMakeLists / sdkconfig.defaults / partitions.csv / build/flash/monitor.sh (2026-05-24)
- [x] **P0.5** main/main.cc + console REPL 骨架（help/version/reboot 三个命令） (2026-05-24)
- [x] **P0.6** Git init + push 到 GitHub (2026-05-24)
- [ ] **P0.7** 实机烧录验证：LCD 显示 "Pocket Oracle"，USB-C 连 Mac 看到 `pocket> ` 提示符

---

## Phase 1 — 硬件 Bring-Up

- [ ] **P1.1** M5.Display 全屏色块（验证色序）
- [ ] **P1.2** M5.BtnA / M5.BtnB 按下打日志
- [ ] **P1.3** M5.Imu 周期读 accel/gyro（识别摇一摇阈值）
- [ ] **P1.4** M5.Speaker 短 beep
- [ ] **P1.5** M5.Power 读电池电压 + 充电状态
- [ ] **P1.6** M5.Rtc 读 BM8563
- [ ] **P1.7** docs/HARDWARE_M5STICKS3.md 补全引脚表

---

## Phase 2 — LVGL UI 框架

- [ ] **P2.1** 加 lvgl/lvgl + esp_lvgl_port 依赖
- [ ] **P2.2** lvgl_port：display + tick + task + mutex（PSRAM 双 framebuffer）
- [ ] **P2.3** M5GFX flush backend (pushImageDMA)
- [ ] **P2.4** ui/theme.h 全部色板/字号常量
- [ ] **P2.5** ui/widgets：status_bar / button_hints / title / list / card
- [ ] **P2.6** app/input_manager：按键长短按状态机
- [ ] **P2.7** app/app_router：应用栈 + push/pop 过渡
- [ ] **P2.8** apps/home_menu：3×4 卡片网格 + 12 个 app 桩

---

## Phase 3 — 决策类应用

- [ ] **P3.1** apps/answer_book + 350 条中英答案数据
- [ ] **P3.2** apps/coin（硬币翻转动画）
- [ ] **P3.3** apps/dice（1/3/5/9 切换）
- [ ] **P3.4** apps/random10
- [ ] **P3.5** apps/yesno
- [ ] **P3.6** 集成 bmi270_sensor 摇一摇 → input_manager

---

## Phase 4 — 仪式 / 运势

- [ ] **P4.1** apps/mbti + 16 人格 × ~30 提示数据
- [ ] **P4.2** apps/fortune + 宜/忌/色数据
- [ ] **P4.3** util/rand_daily（基于日期 + MBTI 的确定性随机）

---

## Phase 5 — 实用小工具

- [ ] **P5.1** apps/clock + util/time_sync（SNTP → BM8563）
- [ ] **P5.2** apps/muyu + 木鱼 PCM 音效
- [ ] **P5.3** storage/stats（功德数 NVS 持久）

---

## Phase 6 — BLE HID 翻页器

- [ ] **P6.1** 加 esp-nimble-cpp 依赖
- [ ] **P6.2** ble/hid_report_map（Keyboard + Consumer Control）
- [ ] **P6.3** ble/ble_hid 初始化 + 广播 + 配对
- [ ] **P6.4** apps/ble_remote 3 子模式
- [ ] **P6.5** 与 WiFi 互斥（进入关 WiFi，退出恢复）

---

## Phase 7 — 设置与系统

- [ ] **P7.1** apps/settings 子菜单（MBTI/亮度/音量/暗色模式/语言/WiFi/关于）
- [ ] **P7.2** storage/settings（NVS 偏好持久）
- [ ] **P7.3** 首次开机引导流程

---

## Phase 8 — 低功耗

- [ ] **P8.1** app/power_manager（idle timer → 屏暗 → light sleep → deep sleep）
- [ ] **P8.2** KEY1 (G11 RTC GPIO) EXT0 wakeup
- [ ] **P8.3** 未用 GPIO 浮空、未用外设 deinit
- [ ] **P8.4** 万用表实测各状态电流
- [ ] **P8.5** 7 天/30 天续航测试

---

## Phase 9 — 打磨与开发文档

- [ ] **P9.1** 动画/文案/错误处理打磨
- [ ] **P9.2** docs/DEV_GUIDE.md 完整版
- [ ] **P9.3** docs/UX_FLOWS.md 配截图
- [ ] **P9.4** README gif demo
- [ ] **P9.5** 量产前 checklist（关 USB-CDC、关日志）
- [ ] **P9.6** v1.0 release tag
