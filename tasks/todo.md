# Pocket Oracle ESP32 — TODO

> 整体计划见 plan.md。本文件追踪逐阶段执行进度。
> 更新：2026-05-31。

---

## Phase 0 — 仓库脚手架 + USB-CDC 控制台

- [x] **P0.1-P0.7** 仓库 + 文档 + 固件骨架 + console + 实机烧录 (2026-05-24)

## Phase 1 — 硬件 Bring-Up

- [x] **P1.1-P1.8** LCD/按键/IMU/Speaker/Power/Time/文档/交互方案 (2026-05-24)
- [x] **P1.9** 物理按键功能扫描（PMIC + G11 + G12）(2026-05-24)
- [x] **P1.9 CORRIGENDUM** 2026-05-30：M5StickS3 **有 2 个用户按键**（BtnA=G11 / BtnB=G12 / PMIC=左侧电源），P1.9 原结论错。SDK 源码验证 + 实机 12+ 次按键测试

## Phase 2 — LVGL UI 框架

- [x] **P2.1-P2.9** 全部完成 (2026-05-24)
- [x] **P2.10** UI"拥挤"——P9 carousel 重设计后解决 (2026-05-30)

## Phase 3 — 决策类应用

- [x] **P3.1-P3.9** 全部完成 (2026-05-24)
- [x] **P3.10** 答案池扩到 350+ 条中文 — P9 完成时实际 560 (2026-05-31)

## Phase 4 — 仪式 / 运势

- [x] **P4.1-P4.3** MBTI / Fortune / rand_daily (2026-05-25)

## Phase 5 — 实用小工具

- [x] **P5.1** Clock + SNTP (P7 一并完成) (2026-05-30)
- [x] **P5.2** Muyu + PCM 木鱼音效 (2026-05-30)
- [x] **P5.3** storage/stats — 功德数 NVS 持久 (2026-05-30)

## Phase 6 — BLE HID 翻页器

- [x] **P6.1-P6.4** NimBLE + HID 报告映射 + 配对 + V1 PPT 模式 (2026-05-30)
- [ ] **P6.4+** Reader / Media 子模式（推后；单按键 + 摇缺少"切模式"自然手势）
- [ ] **P6.5** 与 WiFi 互斥（V1 实测两者共存 OK；有问题再处理）

### 实机验证 (2026-05-30)
- 广播 / 配对 / 翻页 / 长按返回 / stack 干净 deinit 释放 ~50 KB ✓
- 修复 `setCallbacks` deleteCallbacks=true 对 static 单例 delete 的 crash

## Phase 7 — 设置与系统

- [x] **P7.1** Settings 子菜单 (2026-05-30)
- [x] **P7.2** storage/settings NVS helper (2026-05-30)
- [x] **P7.WiFi** Captive Portal 配网（SoftAP + DNS hijack + esp_http_server）(2026-05-30)
- [x] **P7.SNTP** STA 连上后自动 SNTP，Clock 用 wall time (2026-05-30)
- [ ] **P7.3 / P10.4** 首次开机引导 NoCreds → WiFi Setup（不做；现状下用户从 Settings 进 WiFi Setup 已足够清晰）

## Phase 8 — 低功耗

- [x] **P8.1-P8.2** 30s dim / 60s blank / 5min deep sleep + BtnA wake (2026-05-30)
- [ ] **P8.3** 未用 GPIO 浮空 / 外设 deinit（V1 简化版跳过；S3 自动 gating 大部分外设）
- [ ] **P8.4** 万用表实测各状态电流（用户硬件操作，无法编程验证）
- [ ] **P8.5** 7/30 天续航测试（长周期）

## Phase 9 — 打磨与发布

- [x] **P9.0** UI overhaul 国风水墨 4 套主题 + carousel 主页 + 9 ink colors + 笔法字 (2026-05-30)
- [x] **P9.1** Fortune 双列宜/忌 themed-title + 松绿/朱砂 + 20+20+12 CN (2026-05-30)
- [x] **P9.2** Clock 周几 + About 字段 + WiFi Setup chrome 全 CN (2026-05-31)
- [x] **P9.3** MBTI hint CN + code 兰花紫 + 中文绰号 (2026-05-31)
- [x] **P9.4** Settings/About/ThemePicker/WiFi 标题切到 themed-title（修方块 bug）(2026-05-31)
- [x] **P9.5** Answer Book 50 EN → 560 条 CN（172 from dengbuqi + 388 古风）(2026-05-31)
- [x] **P9.6** MBTI 16 类 × 8 prompts 共 128 全翻 CN + 类型语气 (2026-05-31)
- [x] **P9.+** Font subset 自动化 (extract_data_chars.py) — LXGW 220 → 862 字 (2026-05-31)
- [x] **P9.7** Battery 桩 → 真页面 + 朱砂警示 (2026-05-31)
- [x] **P9.11** 量产 sdkconfig — log default WARN + maximum INFO，strip DEBUG/VERBOSE (2026-05-31)
- [ ] **P9.8** docs/DEV_GUIDE.md 完整版
- [ ] **P9.9** docs/UX_FLOWS.md 配截图（需用户提供 / 录屏）
- [ ] **P9.10** README + demo gif（需录屏）
- [ ] **P9.12** v1.0 release tag

## Phase 10 — 推送 / 发布

- [ ] **P10.1** 推送本地 commits 到 origin/main
- [ ] **P10.2** GitHub release v1.0 + binary + 烧录指南
- [x] **P10.3** docs/FLASH_GUIDE.md 烧录指南（终端用户向）(2026-05-31)
