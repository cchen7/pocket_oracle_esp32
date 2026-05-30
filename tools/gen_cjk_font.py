#!/usr/bin/env python3
"""Wrap lv_font_conv to generate a LXGW WenKai LVGL subset font.

V1 only ships a 16 px body font subsetted to the ~150 Chinese chars
the UI strings actually use (settings labels, theme names, about
fields, common app hints). ASCII 0x20-0x7F is also included so the
font is a drop-in replacement for the body slot.

Re-run any time SYMBOLS or the font/size changes. The output lands at
firmware/main/assets/fonts/lxgw_wenkai_cjk_16.c. Add new entries here
when you translate more app screens.
"""

import os
import subprocess
import sys
from pathlib import Path

FONT_PATH = os.path.expanduser(
    "<fonts-dir>/fonts/LxgwWenKai-Regular.ttf")

# Every Chinese character we currently render at runtime. Use raw
# strings so the comma separators stay outside the chars themselves.
# Add new entries when you localize more UI.
SYMBOL_BUCKETS = [
    # Settings sub-menu + chrome
    "主题无线网络关于设置返回",
    # Theme names
    "水墨绢本竹简拓片",
    # Theme picker
    "已生效切换应用按住",
    # About screen
    "固件版本内存运行时间网络已连接未配置离线分秒小时天",
    # Common hints/buttons
    "短按长按摇动光标进入选中下一上一",
    # WiFi setup
    "正在配网请连接手机扫码网页输入密码完成保存",
    # WiFi states
    "连接成功失败重试错误超时",
    # Status bar
    "充电电量",
    # Misc app labels (placeholder, will grow)
    "是否正反个第页颗骰子答案",
]

def main() -> None:
    if len(sys.argv) < 2:
        print("usage: gen_cjk_font.py <output_c_file>", file=sys.stderr)
        sys.exit(2)
    out = Path(sys.argv[1])
    out.parent.mkdir(parents=True, exist_ok=True)

    # Dedupe while preserving order.
    seen = set()
    chars = []
    for bucket in SYMBOL_BUCKETS:
        for c in bucket:
            if c not in seen:
                seen.add(c)
                chars.append(c)
    symbols = "".join(chars)

    cmd = [
        "npx", "--yes", "lv_font_conv",
        "--font", FONT_PATH,
        "--size", "16",
        "--format", "lvgl",
        "--bpp", "4",
        "--no-compress",
        "--no-prefilter",
        "--lv-include", "lvgl.h",
        "--range", "0x20-0x7F",
        "--symbols", symbols,
        "--output", str(out),
    ]
    print(f"chars: {len(chars)} CJK + ASCII range", file=sys.stderr)
    subprocess.run(cmd, check=True)
    size = out.stat().st_size
    print(f"wrote {out} ({size} bytes source)", file=sys.stderr)


if __name__ == "__main__":
    main()
