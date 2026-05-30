#!/usr/bin/env python3
"""Render the home-menu cover PNGs in multiple ink-painting themes.

Each theme is a (palette + texture + font) bundle. Per theme we render
12 covers (240x135), then tools/png_to_lvgl_img.py packs them into
firmware/main/assets/home_covers.h. The persistent status bar overlays
the top 16 px of every cover.

Themes (kept in sync with theme.cc):
  ink     水墨   warm rice paper, Ma Shan Zheng brush
  silk    绢本   cool ivory + diagonal silk weave, ZCOOL XiaoWei serif
  bamboo  竹简   warm beige + vertical wood grain, Liu Jian Mao Cao grass
  stone   拓片   dark slate + grain, Long Cang brush in white ink

Usage:
    gen_covers.py <output_dir>            # renders all themes into subdirs
    gen_covers.py <output_dir> --theme ink   # one theme
"""

import math
import os
import random
import sys
from pathlib import Path

from PIL import Image, ImageDraw, ImageFilter, ImageFont

WIDTH, HEIGHT = 240, 135
SEAL_RED = (168, 54, 46)

FONTS_DIR = os.path.expanduser("<fonts-dir>/fonts")

# (cover_char, cn_name, en_name) — order matches home_menu cursor indices 0..11.
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


# ------- Texture helpers (applied to the bg before drawing text) -------

def texture_paper(img: Image.Image, rng: random.Random,
                  speckle_count: int = 360, max_delta: int = 6) -> None:
    """Subtle random pixel jitter so the bg doesn't look flat-LCD."""
    px = img.load()
    for _ in range(speckle_count):
        x = rng.randint(0, WIDTH - 1)
        y = rng.randint(0, HEIGHT - 1)
        delta = rng.choice([-max_delta, -max_delta // 2, max_delta // 2, max_delta])
        r, g, b = px[x, y]
        px[x, y] = (max(0, min(255, r + delta)),
                    max(0, min(255, g + delta)),
                    max(0, min(255, b + delta)))


def texture_silk(img: Image.Image, rng: random.Random) -> None:
    """Cross-hatched weave + a touch of noise — pronounced silk look."""
    texture_paper(img, rng, speckle_count=300, max_delta=5)
    draw = ImageDraw.Draw(img, "RGBA")
    # Diagonal warp threads.
    for d in range(-HEIGHT, WIDTH, 3):
        draw.line([(d, 0), (d + HEIGHT, HEIGHT)], fill=(40, 30, 18, 14),
                  width=1)
    # Anti-diagonal weft.
    for d in range(0, WIDTH + HEIGHT, 6):
        draw.line([(d, 0), (d - HEIGHT, HEIGHT)], fill=(0, 0, 0, 8),
                  width=1)


def texture_bamboo(img: Image.Image, rng: random.Random) -> None:
    """Vertical wood-grain stripes simulating a bamboo slip."""
    texture_paper(img, rng, speckle_count=180, max_delta=5)
    draw = ImageDraw.Draw(img, "RGBA")
    # Vertical seams at irregular x positions to fake bamboo slip joints.
    seam_xs = []
    x = rng.randint(20, 40)
    while x < WIDTH:
        seam_xs.append(x)
        x += rng.randint(30, 50)
    for sx in seam_xs:
        draw.line([(sx, 0), (sx, HEIGHT)], fill=(0, 0, 0, 28), width=1)
    # Long faint vertical grain lines.
    for _ in range(28):
        gx = rng.randint(0, WIDTH - 1)
        gy0 = rng.randint(0, 40)
        gy1 = rng.randint(80, HEIGHT - 1)
        draw.line([(gx, gy0), (gx, gy1)], fill=(0, 0, 0, 10), width=1)


def texture_stone(img: Image.Image, rng: random.Random) -> None:
    """Dense grain so the dark slate looks like a stone-rubbing surface."""
    px = img.load()
    for _ in range(2200):
        x = rng.randint(0, WIDTH - 1)
        y = rng.randint(0, HEIGHT - 1)
        delta = rng.choice([-10, -7, -4, 4, 7, 10, 14])
        r, g, b = px[x, y]
        px[x, y] = (max(0, min(255, r + delta)),
                    max(0, min(255, g + delta)),
                    max(0, min(255, b + delta)))


# ------- Theme registry -------

THEMES = {
    "ink": dict(
        bg       = (240, 232, 216),  # #F0E8D8 warm rice paper
        ink_fg   = (26, 24, 20),
        ink_dim  = (124, 116, 104),
        big_font = "MaShanZheng-Regular.ttf",
        big_size = 100,
        texture  = texture_paper,
        glow     = True,
    ),
    "silk": dict(
        bg       = (232, 220, 192),  # #E8DCC0 pale aged silk (light, cool gold)
        ink_fg   = (38, 28, 16),
        ink_dim  = (122, 104, 76),
        big_font = "ZCOOLXiaoWei-Regular.ttf",
        big_size = 100,
        texture  = texture_silk,
        glow     = False,
    ),
    "bamboo": dict(
        bg       = (185, 152, 96),   # #B99860 warm dark bamboo / tea-stained wood
        ink_fg   = (32, 22, 10),
        ink_dim  = (92, 74, 50),
        big_font = "LiuJianMaoCao-Regular.ttf",
        big_size = 110,
        texture  = texture_bamboo,
        glow     = True,
    ),
    "stone": dict(
        bg       = (42, 45, 51),     # #2A2D33 dark slate
        ink_fg   = (232, 228, 218),  # near-white "rubbing" ink
        ink_dim  = (150, 146, 138),
        big_font = "LongCang-Regular.ttf",
        big_size = 110,
        texture  = texture_stone,
        glow     = True,
    ),
}

# Always use LXGW WenKai for the small labels — brush fonts at 11-14 px
# turn into illegible blobs.
SMALL_FONT = "LxgwWenKai-Regular.ttf"


def font(name: str, size: int) -> ImageFont.FreeTypeFont:
    return ImageFont.truetype(os.path.join(FONTS_DIR, name), size)


def draw_seal(draw: ImageDraw.ImageDraw, x: int, y: int, idx: int,
              small_font: ImageFont.FreeTypeFont, ink_fg) -> None:
    """Tiny vermilion seal square with the cover index inside."""
    size = 16
    draw.rectangle([x, y, x + size, y + size],
                   outline=SEAL_RED, width=2)
    draw.rectangle([x + 3, y + 3, x + size - 3, y + size - 3],
                   fill=SEAL_RED)
    txt = str(idx + 1)
    tw = draw.textlength(txt, font=small_font)
    # Pick text color that contrasts with the red square (always paper-ish).
    draw.text((x + size / 2 - tw / 2, y + 1),
              txt, font=small_font, fill=(245, 240, 225))


def render_cover(big_ch: str, cn_name: str, en_name: str,
                 idx: int, theme: dict, rng: random.Random) -> Image.Image:
    img = Image.new("RGB", (WIDTH, HEIGHT), theme["bg"])
    theme["texture"](img, rng)
    draw = ImageDraw.Draw(img)

    f_huge = font(theme["big_font"], theme["big_size"])
    f_name = font(SMALL_FONT, 14)
    f_sub  = font(SMALL_FONT, 11)
    f_seal = font(SMALL_FONT, 11)

    # Big brush char vertically centered in the area below the status
    # bar (status bar owns y=0..16). Shift slightly above the visual
    # midline of the content band so the bottom row has breathing room.
    content_top, content_bottom = 16, HEIGHT - 22
    content_h = content_bottom - content_top

    bbox = draw.textbbox((0, 0), big_ch, font=f_huge)
    bw = bbox[2] - bbox[0]
    bh = bbox[3] - bbox[1]
    cx = (WIDTH - bw) / 2 - bbox[0]
    cy = content_top + (content_h - bh) / 2 - bbox[1]

    if theme["glow"]:
        glow = Image.new("RGBA", (WIDTH, HEIGHT), (0, 0, 0, 0))
        ImageDraw.Draw(glow).text(
            (cx, cy), big_ch, font=f_huge, fill=(*theme["ink_fg"], 38))
        glow = glow.filter(ImageFilter.GaussianBlur(radius=2))
        img.paste(glow, (0, 0), glow)

    draw.text((cx, cy), big_ch, font=f_huge, fill=theme["ink_fg"])

    # Bottom row: CN name left, EN subtitle right-of-center, seal at right.
    bottom_y = HEIGHT - 18
    draw.text((8, bottom_y), cn_name, font=f_name, fill=theme["ink_fg"])
    en_w = draw.textlength(en_name, font=f_sub)
    draw.text((WIDTH - 28 - en_w, bottom_y + 2), en_name,
              font=f_sub, fill=theme["ink_dim"])
    draw_seal(draw, WIDTH - 20, bottom_y - 1, idx, f_seal, theme["ink_fg"])

    return img


def render_theme(theme_name: str, out_dir: Path) -> None:
    theme = THEMES[theme_name]
    out_dir.mkdir(parents=True, exist_ok=True)
    rng = random.Random(0x1B1A18 ^ hash(theme_name))
    for i, (ch, cn, en) in enumerate(COVERS):
        img = render_cover(ch, cn, en, i, theme, rng)
        path = out_dir / f"cover_{i:02d}.png"
        img.save(path, "PNG")
    print(f"theme {theme_name}: rendered {len(COVERS)} covers -> {out_dir}",
          file=sys.stderr)


def main() -> None:
    if len(sys.argv) < 2:
        print("usage: gen_covers.py <out_dir> [--theme NAME]", file=sys.stderr)
        sys.exit(2)
    out_root = Path(sys.argv[1])
    if "--theme" in sys.argv:
        idx = sys.argv.index("--theme")
        names = [sys.argv[idx + 1]]
    else:
        names = list(THEMES.keys())
    for n in names:
        render_theme(n, out_root / n)


if __name__ == "__main__":
    main()
