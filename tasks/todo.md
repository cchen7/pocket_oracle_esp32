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
- [x] **P0.7** 实机烧录验证：LCD 显示 "Pocket Oracle"，USB-C 连 Mac 看到 `pocket> ` 提示符 (2026-05-24)
  - 端口 `/dev/cu.usbmodem21101`
  - `help` / `version` 命令响应正常
  - 修正 KEY1/KEY2 映射：KEY1=G11=侧面小按键(PMIC 电源/Boot)，KEY2=G12=前面板大按键(用户按键)
  - PM_ENABLE 暂时关闭（与 USB-Serial-JTAG 在 light sleep 下不兼容），P8 再处理

---

## Phase 1 — 硬件 Bring-Up

- [x] **P1.1** LCD：5 色循环 (RGBWK) 正常显示，landing screen 恢复 (2026-05-24)
- [x] **P1.2** 按键：G11 (M5.BtnA) 检测 OK；M5StickS3 实际**只有 1 个用户按键** (前面板大长条 = G11)，G12 未接 (2026-05-24)
- [x] **P1.3** IMU：BMI270 检测到，加速度 Z 轴 1g / X Y ≈ 0，陀螺仪噪声 < 0.5°/s (2026-05-24)
- [x] **P1.4** Speaker：1kHz + 2kHz beep 播放 (2026-05-24)
- [x] **P1.5** Power：4170 mV (100%) 充电/放电状态正常 (2026-05-24)
- [x] **P1.6** ~~RTC~~ Time：M5StickS3 **无 RTC 芯片**；改成读 ESP32 system time (SNTP 校时后) (2026-05-24)
- [x] **P1.7** docs/HARDWARE_M5STICKS3.md 修正：G11 单按键、无 RTC、PMIC 抢占双击/长按 (2026-05-24)
- [x] **P1.8** 交互方案重新设计：从双按键改为单按键 + 摇一摇 (PRD/README/CLAUDE 同步) (2026-05-24)
- [x] **P1.9** 实测确认 3 个物理"按键"功能：左小=PMIC 电源、中长条=G11 用户按键、右中=非按键空接 (2026-05-24)
  - 通过 `gpioscan` 命令扫描全部用户 GPIO + 观察 PMIC/USB 行为得出
  - 按左小按键端口立即丢失 → PMIC 关机 = 它就是 M5Stack 文档说的 "side button"
  - 按右中按键无任何电气反应 → 排除 RST/GPIO/PMIC，确认为装饰/IR 窗口

---

## Phase 2 — LVGL UI 框架

- [x] **P2.1** 加 lvgl/lvgl 依赖 + lv_conf.h 项目本地配置（CONFIG_LV_CONF_SKIP=n） (2026-05-24)
- [x] **P2.2** lvgl_port：PSRAM 65KB framebuffer + esp_timer tick 2ms + lvgl_task pinned core 1 + recursive mutex (2026-05-24)
- [x] **P2.3** M5GFX flush backend：pushImage rgb565_t* (2026-05-24)
- [x] **P2.4** 修复 M5GFX vs LVGL 类型冲突：lvgl_alias/lvgl/lvgl.h 让 M5GFX `__has_include` 找到真 LVGL (2026-05-24)
- [x] **P2.5** ui/theme.h：色板/字号/间距常量 (2026-05-24)
- [x] **P2.6** ui/status_bar：顶部 uptime + 电池 % (2 s 刷新) (2026-05-24)
- [x] **P2.7** app/input_manager：G11 短按/长按状态机 + BMI270 摇一摇检测 (1.7g 阈值 + 500 ms 冷却) (2026-05-24)
- [x] **P2.8** app/app_router：应用栈 + 全局 long-press = 回主菜单 + LVGL mutex 包装 (2026-05-24)
- [x] **P2.9** apps/home_menu：4x3 卡片网格 + 12 app 桩，摇一摇切光标，单按进入，长按返回 (2026-05-24)
- [ ] **P2.10** UI 优化（用户反馈："拥挤"）— 延后到 P3+ 有真实图标/字体时统一打磨

---

## Phase 3 — 决策类应用

- [x] **P3.1** apps/answer_book (50 条英文 seed；中文与 350 条扩展延后到 P9 字体打磨) (2026-05-24)
- [x] **P3.2** apps/coin（HEADS/TAILS + 颜色区分） (2026-05-24)
- [x] **P3.3** apps/dice（1/3/5/9 切换，shake 切数量、tap 重掷；大字总和 + 小字各颗） (2026-05-24)
- [x] **P3.4** apps/random10 (2026-05-24)
- [x] **P3.5** apps/yesno (2026-05-24)
- [x] **P3.6** util/rng (esp_random 拒绝采样 uniform) (2026-05-24)
- [x] **P3.7** shake 阈值标定：1.35g + 400ms 冷却（用户实测确认手感合适） (2026-05-24)
- [x] **P3.8** Bug fix: flash 动画 opa 256 → 255（uint8_t 回卷使文字隐身） (2026-05-24)
- [x] **P3.9** font_display 升到 montserrat_48（大字号体感正确） (2026-05-24)
- [ ] **P3.10** （延后到 P9）扩展答案池到 350 条 + 中文 + Dice UI 进一步打磨

---

## Phase 4 — 仪式 / 运势

- [x] **P4.1** apps/mbti：16 人格 × 8 提示，shake 切人格、tap 换提示，按 daily seed 选 (2026-05-25)
- [x] **P4.2** apps/fortune：DO/AVOID 双列 + 幸运色，shake/tap 重抽 (2026-05-25)
- [x] **P4.3** util/rand_daily：splitmix64 (today, salt) 确定性哈希，SNTP 未到位前用 uptime 兜底 (2026-05-25)

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
