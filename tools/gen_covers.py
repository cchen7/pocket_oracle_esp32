#!/usr/bin/env python3
"""Render 12 home-menu cover PNGs in ink-wash style.

Each cover is 240x135 (Pocket Oracle LCD landscape). The persistent
status bar overlays the top 16 px (clock + battery) regardless of
which screen is showing, so the cover only owns y=16..135. Design:

  +-------------------------------------------+
  | (status bar drawn separately, owns 16px)  |
  |                                            |
  |              答                            |   center: huge brush char
  |                                            |
  | 答案之书           Answer        [印]      |   bottom: CN+EN+seal row
  +-------------------------------------------+
"""

import os
import random
import sys
from pathlib import Path

from PIL import Image, ImageDraw, ImageFilter, ImageFont

# Project palette (ink on white rice paper).
BG       = (240, 232, 216)     # #F0E8D8 暖纸白
INK_FG   = (26, 24, 20)        # #1A1814 墨黑
INK_DIM  = (124, 116, 104)     # #7C7468 灰墨
SEAL_RED = (168, 54, 46)       # #A8362E 朱砂
WIDTH, HEIGHT = 240, 135

# Order matches home_menu cursor indices 0..11.
COVERS = [
    ("答", "答案之书", "Answer"),
    ("卜", "抛硬币",   "Coin"),
    ("骰", "掷骰子",   "Dice"),
    ("数", "取一数",   "Random"),
    ("否", "是与否",   "Yes/No"),
    ("性", "人格签",   "MBTI"),
    ("运", "今日运势", "Fortune"),
    ("时", "时辰",     "Clock"),
    ("鱼", "电子木鱼", "Muyu"),
    ("蓝", "蓝牙翻页", "BLE Remote"),
    ("灯", "电量",     "Battery"),
    ("设", "设置",     "Settings"),
]

FONT_PATH = os.path.expanduser(
    "<fonts-dir>/fonts/LxgwWenKai-Regular.ttf")
HUGE_FONT_PATH = os.path.expanduser(
    "<fonts-dir>/fonts/MaShanZheng-Regular.ttf")


def paper_texture(img: Image.Image) -> None:
    """Add subtle noise speckles so the bg doesn't look flat-LCD."""
    rnd = random.Random(2026)
    px = img.load()
    for _ in range(360):
        x = rnd.randint(0, WIDTH - 1)
        y = rnd.randint(0, HEIGHT - 1)
        # Sprinkle slightly lighter / darker pixels.
        delta = rnd.choice([-6, -4, -2, 2, 4, 6])
        r, g, b = px[x, y]
        px[x, y] = (max(0, min(255, r + delta)),
                    max(0, min(255, g + delta)),
                    max(0, min(255, b + delta)))


def draw_seal(draw: ImageDraw.ImageDraw, x: int, y: int, idx: int,
              big_font: ImageFont.FreeTypeFont) -> None:
    """Tiny vermilion seal square with the cover index inside."""
    size = 16
    draw.rectangle([x, y, x + size, y + size],
                   outline=SEAL_RED, width=2)
    # Slight inset block of color so it reads as a stamp at a glance.
    draw.rectangle([x + 3, y + 3, x + size - 3, y + size - 3],
                   fill=SEAL_RED)
    # Index numeral in paper color, centered.
    txt = str(idx + 1)
    tw = draw.textlength(txt, font=big_font)
    draw.text((x + size / 2 - tw / 2, y + 1),
              txt, font=big_font, fill=INK_FG)


def render_cover(big_ch: str, cn_name: str, en_name: str,
                 idx: int) -> Image.Image:
    img = Image.new("RGB", (WIDTH, HEIGHT), BG)
    paper_texture(img)
    draw = ImageDraw.Draw(img)

    f_huge   = ImageFont.truetype(HUGE_FONT_PATH, 100)  # cover character — Ma Shan Zheng brush
    f_name   = ImageFont.truetype(FONT_PATH, 14)   # CN name
    f_sub    = ImageFont.truetype(FONT_PATH, 11)   # EN subtitle
    f_seal   = ImageFont.truetype(FONT_PATH, 11)   # seal index

    # Big brush char vertically centered in the area below the status
    # bar (status bar owns y=0..16). Shift slightly above the visual
    # midline of the content band so the bottom name row has breathing
    # room.
    content_top    = 16
    content_bottom = HEIGHT - 22   # leave 22px for bottom row
    content_h      = content_bottom - content_top

    bbox = draw.textbbox((0, 0), big_ch, font=f_huge)
    bw = bbox[2] - bbox[0]
    bh = bbox[3] - bbox[1]
    cx = (WIDTH - bw) / 2 - bbox[0]
    cy = content_top + (content_h - bh) / 2 - bbox[1]

    # Soft ink-bleed glow first so the main glyph sits on top.
    glow = Image.new("RGBA", (WIDTH, HEIGHT), (0, 0, 0, 0))
    glow_draw = ImageDraw.Draw(glow)
    glow_draw.text((cx, cy), big_ch, font=f_huge, fill=(*INK_FG, 38))
    glow = glow.filter(ImageFilter.GaussianBlur(radius=2))
    img.paste(glow, (0, 0), glow)

    draw.text((cx, cy), big_ch, font=f_huge, fill=INK_FG)

    # Bottom row: CN name on the left, EN subtitle right-of-center,
    # red seal in the bottom-right corner. The seal is small so it
    # reads as accent, not headline.
    bottom_y = HEIGHT - 18
    draw.text((8, bottom_y), cn_name, font=f_name, fill=INK_FG)
    # EN subtitle dim, right-aligned to a fixed column so all 12 line up.
    en_w = draw.textlength(en_name, font=f_sub)
    draw.text((WIDTH - 28 - en_w, bottom_y + 2), en_name,
              font=f_sub, fill=INK_DIM)
    draw_seal(draw, WIDTH - 20, bottom_y - 1, idx, f_seal)

    return img


def main() -> None:
    if len(sys.argv) < 2:
        print("usage: gen_covers.py <output_dir>", file=sys.stderr)
        sys.exit(2)
    out_dir = Path(sys.argv[1])
    out_dir.mkdir(parents=True, exist_ok=True)

    for i, (ch, cn, en) in enumerate(COVERS):
        img = render_cover(ch, cn, en, i)
        path = out_dir / f"cover_{i:02d}_{en.replace('/', '').replace(' ', '_').lower()}.png"
        img.save(path, "PNG")
        print(f"{path}")


if __name__ == "__main__":
    main()
