#!/usr/bin/env python3
"""Wrap lv_font_conv to generate LXGW WenKai LVGL subset fonts.

Emits two sizes:
  - lxgw_wenkai_cjk_14.c  hint footers, small labels
  - lxgw_wenkai_cjk_16.c  body / sub-result text

The char set is the union of:
  - SYMBOL_BUCKETS below (UI strings outside data files)
  - All CJK chars in firmware/main/data/*.h string literals
    (Answer Book / MBTI / Fortune pools — auto-extracted, no need
    to mirror translated content into a bucket).

ASCII 0x20-0x7F is included so the font is a drop-in replacement
for the body slot.

Re-run any time SYMBOL_BUCKETS, the font, or any data file changes.
"""

import os
import subprocess
import sys
from pathlib import Path

from extract_data_chars import extract_from_files

FONT_PATH = os.path.expanduser(
    "<fonts-dir>/fonts/LxgwWenKai-Regular.ttf")

SIZES = [14, 16]

# Project root (one above this script) — used to locate data files.
PROJECT_ROOT = Path(__file__).resolve().parent.parent

# Data files we auto-extract CJK chars from. Adding more data files
# here makes their content automatically renderable without manual
# subset maintenance.
DATA_FILES = [
    PROJECT_ROOT / "firmware/main/data/answer_book_data.h",
    PROJECT_ROOT / "firmware/main/data/mbti_data.h",
    PROJECT_ROOT / "firmware/main/data/fortune_data.h",
]

# Every Chinese character we currently render at runtime in body or
# hint slots. Add new entries when you localize more UI.
SYMBOL_BUCKETS = [
    # Settings sub-menu + chrome
    "主题无线网络关于设置返回",
    # Theme names
    "水墨绢本竹简拓片",
    # Theme picker
    "已生效切换应用按住侧键",
    # About screen
    "固件版本内存运行时间网络已连接未配置离线分秒小时天关闭",
    # Common hints/buttons
    "短按长按摇动光标进入选中下一上一",
    # WiFi setup
    "正在配网请连接手机扫码网页输入密码完成保存用此热点弹出填家庭",
    # WiFi states
    "连接成功失败重试错误超时",
    # Status bar
    "充电电量",
    # Misc app labels
    "是否正反个第页颗骰子答案",
    # Weekday (Clock)
    "周日一二三四五六",
    # Action verbs for hints (consistent CN across apps)
    "或摇投掷数翻次再来重抽换型签蓝牙",
    # Ritual / fortune labels (UI chrome)
    "宜忌幸运色今日勿安",
    # Fortune DO / AVOID / Lucky color names — auto-extracted from
    # fortune_data.h, kept here as redundancy in case of refactor.
    # Lucky color names
    "朱砂珊瑚琥珀橄榄苍青鸭天靛蓝紫绛蔷薇缃月白",
    # Muyu
    "功德",
    # BLE remote
    "蓝牙未启动配对中已连接发送下页上页",
]


def main() -> None:
    if len(sys.argv) < 2:
        print("usage: gen_cjk_font.py <output_dir>", file=sys.stderr)
        sys.exit(2)
    out_dir = Path(sys.argv[1])
    out_dir.mkdir(parents=True, exist_ok=True)

    # Dedupe while preserving order: UI buckets first, then data-file
    # chars (auto-extracted) sorted by codepoint.
    seen = set()
    chars = []
    for bucket in SYMBOL_BUCKETS:
        for c in bucket:
            if c not in seen:
                seen.add(c)
                chars.append(c)
    data_chars = extract_from_files(DATA_FILES)
    for c in sorted(data_chars):
        if c not in seen:
            seen.add(c)
            chars.append(c)
    symbols = "".join(chars)
    print(f"chars: {len(chars)} CJK ({len(data_chars)} from data files) "
          f"+ ASCII range", file=sys.stderr)

    for size in SIZES:
        out_path = out_dir / f"lxgw_wenkai_cjk_{size}.c"
        cmd = [
            "npx", "--yes", "lv_font_conv",
            "--font", FONT_PATH,
            "--size", str(size),
            "--format", "lvgl",
            "--bpp", "4",
            "--no-compress",
            "--no-prefilter",
            "--lv-include", "lvgl.h",
            "--range", "0x20-0x7F",
            "--symbols", symbols,
            "--output", str(out_path),
        ]
        subprocess.run(cmd, check=True)
        print(f"wrote {out_path} ({out_path.stat().st_size} bytes)",
              file=sys.stderr)


if __name__ == "__main__":
    main()

