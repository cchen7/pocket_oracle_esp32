#!/usr/bin/env python3
"""Generate per-theme LVGL display fonts at multiple sizes.

For each theme we emit two subset fonts:
  - theme_<name>_title_28.c  app title / caption (28 px)
  - theme_<name>_display_48.c big result word (48 px)

Brush characters need surrounding chars to read as calligraphy —
single isolated big chars don't show the style well. The 28 px title
size is where multi-character app titles (e.g. "抛一枚硬币") give the
brush font enough room to express its personality.

Body / hint text continues to use the shared LXGW WenKai 16 px subset
because 草书 / 行书 at 16 px is essentially unreadable on the
M5StickS3.

Add new chars to DISPLAY_SYMBOLS_BUCKETS as more strings get
translated, then re-run.
"""

import os
import subprocess
import sys
from pathlib import Path

FONTS_DIR = os.environ.get(
    "POCKET_FONTS_DIR",
    str(Path(__file__).resolve().parent.parent / "fonts"))

# Map theme id -> brush font filename. Must match the THEMES dict in
# tools/gen_covers.py.
THEMES = {
    "ink":    "MaShanZheng-Regular.ttf",
    "silk":   "ZCOOLXiaoWei-Regular.ttf",
    "bamboo": "LiuJianMaoCao-Regular.ttf",
    "stone":  "LongCang-Regular.ttf",
}

# All CJK chars rendered at display size (>=24 px). Add when more
# headline strings / big result words get translated.
DISPLAY_SYMBOLS_BUCKETS = [
    # App titles
    "答案之书抛硬币掷骰子取一数是与否人格签今日运势时辰电子木鱼蓝牙翻页电量设置",
    # Settings sub-screens (theme picker / about title)
    "主题关于无线网络",
    # Big result words
    "正反是否颗个",
    # Captions / sub-titles
    "抛一枚硬币是非可断随机取一数今日一问换签换型功德",
    # BLE Remote big-state words (also shown title-like)
    "蓝牙翻页未启动配对已连接发送上一下页",
    # Fortune section heads
    "宜忌幸运色",
    # Clock fallback
    "等待同步",
]


def main() -> None:
    if len(sys.argv) < 2:
        print("usage: gen_themed_display_fonts.py <output_dir>",
              file=sys.stderr)
        sys.exit(2)
    out_dir = Path(sys.argv[1])
    out_dir.mkdir(parents=True, exist_ok=True)

    # Dedupe chars across buckets.
    seen = set()
    chars = []
    for bucket in DISPLAY_SYMBOLS_BUCKETS:
        for c in bucket:
            if c not in seen:
                seen.add(c)
                chars.append(c)
    symbols = "".join(chars)
    print(f"display subset: {len(chars)} CJK chars", file=sys.stderr)

    for theme_id, font_file in THEMES.items():
        font_path = os.path.join(FONTS_DIR, font_file)
        for kind, size in [("title", 28), ("display", 48)]:
            out_path = out_dir / f"theme_{theme_id}_{kind}_{size}.c"
            cmd = [
                "npx", "--yes", "lv_font_conv",
                "--font", font_path,
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
            print(f"-> {theme_id} {kind}/{size}px ({font_file})", file=sys.stderr)
            subprocess.run(cmd, check=True)
            print(f"   {out_path} ({out_path.stat().st_size} bytes)",
                  file=sys.stderr)


if __name__ == "__main__":
    main()
