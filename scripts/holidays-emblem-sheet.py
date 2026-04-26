#!/usr/bin/env python3
"""
Generate one small transparent emblem sprite for each new holiday.

This replaces the older five-variant concept-art pipeline. The output here is
a simple 32x32 icon set: one family-friendly, emblematic prop per added
holiday, packed into a sprite sheet.

Outputs:
  scratch/holidays-emblems/<id>-<short>.png
  scratch/holidays-emblems/holiday-emblems-sheet.png
  scratch/holidays-emblems/holiday-emblems-preview.png
  scratch/holidays-emblems/review.html
  scratch/holidays-emblems/manifest.json
"""

from __future__ import annotations

import json
import sys
from pathlib import Path

try:
    import yaml
    from PIL import Image, ImageDraw, ImageFont
except ImportError as e:
    sys.stderr.write(f"error: {e}\n")
    sys.exit(2)

REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO / "scripts"))

from holidays_art_lib import (  # noqa: E402
    BLACK,
    DEEPBLUE,
    DGREEN,
    GRAY,
    GREEN,
    ORANGE,
    PINK,
    PURPLE,
    RED,
    SAND,
    SKIN,
    SKY,
    Sprite,
    TRANSPARENT,
    TRUNK,
    WHITE,
    YELLOW,
    compose_heart,
    compose_star,
)

YAML_PATH = REPO / "holidays.yml"
OUT_DIR = REPO / "scratch" / "holidays-emblems"
ICON = 32
SHEET_COLS = 8


def slugify(name: str) -> str:
    return name.replace(" ", "_").replace("'", "").replace(".", "")


def icon() -> Sprite:
    return Sprite(ICON, ICON, fill=TRANSPARENT)


def poly(sp: Sprite, pts: list[tuple[int, int]], fill: int,
         outline: int | None = BLACK) -> None:
    sp.draw.polygon(pts, fill=fill)
    if outline is not None:
        sp.draw.line(pts + [pts[0]], fill=outline)


def small_note(sp: Sprite, x: int, y: int, color: int = BLACK) -> None:
    sp.line(x + 3, y, x + 3, y + 8, color)
    sp.line(x + 3, y, x + 8, y + 2, color)
    sp.ellipse(x, y + 7, x + 4, y + 11, color)


def tiny_flower(sp: Sprite, cx: int, cy: int, petal: int,
                center: int = YELLOW) -> None:
    sp.px(cx, cy - 2, petal)
    sp.px(cx - 2, cy, petal)
    sp.px(cx + 2, cy, petal)
    sp.px(cx, cy + 2, petal)
    sp.px(cx, cy, center)


def tiny_leaf(sp: Sprite, cx: int, cy: int, color: int = GREEN) -> None:
    sp.ellipse(cx - 3, cy - 2, cx + 3, cy + 2, color, outline=DGREEN)
    sp.line(cx - 2, cy + 1, cx + 2, cy - 1, DGREEN)


def draw_flag(sp: Sprite, x: int, y: int, w: int, h: int) -> None:
    sp.rect(x, y, x + w, y + h, WHITE, outline=BLACK)
    for yy in range(y + 1, y + h, 3):
        sp.line(x + 1, yy, x + w - 1, yy, RED)
    sp.rect(x + 1, y + 1, x + max(5, w // 2), y + max(4, h // 2),
            DEEPBLUE)
    for sx, sy in [(x + 3, y + 3), (x + 6, y + 3), (x + 4, y + 5)]:
        sp.px(sx, sy, WHITE)


def draw_pi_symbol(sp: Sprite, x: int, y: int, color: int = WHITE) -> None:
    sp.rect(x, y, x + 13, y + 2, color)
    sp.rect(x + 2, y + 2, x + 4, y + 12, color)
    sp.rect(x + 9, y + 2, x + 11, y + 12, color)
    sp.px(x + 12, y + 12, color)


def elvis() -> Sprite:
    sp = icon()
    sp.ellipse(5, 15, 18, 27, ORANGE, outline=BLACK)
    sp.ellipse(9, 18, 14, 23, BLACK)
    sp.line(17, 16, 25, 6, TRUNK)
    sp.line(18, 16, 26, 6, TRUNK)
    sp.rect(23, 4, 29, 8, TRUNK, outline=BLACK)
    for off in (0, 2, 4):
        sp.line(10 + off, 17, 25, 6 + off // 2, WHITE)
    small_note(sp, 22, 17, BLACK)
    compose_star(sp, 8, 7, 2, YELLOW)
    return sp


def mlk() -> Sprite:
    sp = icon()
    sp.rect(7, 18, 24, 27, RED, outline=BLACK)
    sp.rect(5, 15, 26, 19, RED, outline=BLACK)
    sp.rect(10, 20, 21, 25, WHITE, outline=BLACK)
    for yy in (21, 23):
        sp.line(12, yy, 19, yy, BLACK)
    sp.line(15, 10, 15, 15, GRAY)
    sp.ellipse(13, 7, 17, 11, BLACK)
    sp.ellipse(18, 7, 28, 14, WHITE, outline=BLACK)
    sp.ellipse(24, 5, 28, 9, WHITE, outline=BLACK)
    sp.px(29, 8, YELLOW)
    for x in (5, 27):
        compose_star(sp, x, 9, 2, YELLOW)
    return sp


def groundhog() -> Sprite:
    sp = icon()
    sp.ellipse(7, 20, 25, 29, BLACK)
    sp.ellipse(9, 19, 23, 28, TRUNK, outline=BLACK)
    sp.ellipse(10, 8, 22, 21, TRUNK, outline=BLACK)
    sp.ellipse(8, 7, 12, 11, TRUNK, outline=BLACK)
    sp.ellipse(20, 7, 24, 11, TRUNK, outline=BLACK)
    sp.px(13, 13, BLACK)
    sp.px(19, 13, BLACK)
    sp.px(16, 16, BLACK)
    sp.px(15, 18, WHITE)
    sp.px(17, 18, WHITE)
    sp.line(23, 24, 30, 26, GRAY)
    sp.line(23, 25, 29, 27, GRAY)
    return sp


def valentine() -> Sprite:
    sp = icon()
    sp.rect(13, 5, 20, 29, TRUNK, outline=BLACK)
    for y in range(7, 27, 5):
        sp.line(13, y, 20, y + 2, SAND)
    compose_heart(sp, 16, 14, 4, RED)
    compose_heart(sp, 7, 8, 3, PINK)
    compose_heart(sp, 25, 10, 2, PINK)
    compose_heart(sp, 8, 22, 2, RED)
    return sp


def superbowl() -> Sprite:
    sp = icon()
    sp.ellipse(4, 9, 28, 23, TRUNK, outline=BLACK)
    sp.line(9, 10, 23, 22, BLACK)
    sp.line(10, 21, 22, 10, BLACK)
    sp.rect(12, 15, 20, 17, WHITE)
    for x in (13, 15, 17, 19):
        sp.line(x, 14, x, 18, WHITE)
    sp.line(5, 16, 1, 16, WHITE)
    sp.line(27, 16, 31, 16, WHITE)
    return sp


def presidents() -> Sprite:
    sp = icon()
    sp.rect(9, 8, 23, 21, DEEPBLUE, outline=BLACK)
    sp.rect(5, 20, 27, 24, DEEPBLUE, outline=BLACK)
    sp.rect(10, 15, 22, 18, RED)
    sp.rect(10, 11, 22, 13, WHITE)
    compose_star(sp, 16, 6, 3, WHITE)
    compose_star(sp, 16, 16, 2, YELLOW)
    return sp


def mardi() -> Sprite:
    sp = icon()
    sp.ellipse(4, 10, 28, 24, PURPLE, outline=BLACK)
    sp.ellipse(9, 14, 13, 18, BLACK)
    sp.ellipse(19, 14, 23, 18, BLACK)
    sp.rect(14, 19, 18, 21, YELLOW)
    for x, c in [(7, GREEN), (12, YELLOW), (20, GREEN), (25, YELLOW)]:
        sp.px(x, 8, c)
        sp.px(x, 25, c)
    for x in range(8, 25, 3):
        sp.ellipse(x, 27, x + 2, 29, YELLOW if x % 2 else GREEN)
    return sp


def piday() -> Sprite:
    sp = icon()
    sp.rect(5, 6, 27, 24, DGREEN, outline=BLACK)
    sp.rect(8, 25, 24, 27, TRUNK, outline=BLACK)
    draw_pi_symbol(sp, 9, 10, WHITE)
    sp.ellipse(20, 20, 29, 28, ORANGE, outline=BLACK)
    sp.line(22, 22, 27, 27, TRUNK)
    return sp


def spring() -> Sprite:
    sp = icon()
    sp.line(5, 25, 25, 6, TRUNK)
    sp.line(6, 26, 26, 7, TRUNK)
    for cx, cy in [(10, 21), (15, 16), (21, 10), (25, 7)]:
        tiny_flower(sp, cx, cy, PINK, WHITE)
    sp.ellipse(5, 6, 12, 13, YELLOW, outline=BLACK)
    sp.ellipse(12, 6, 19, 13, YELLOW, outline=BLACK)
    sp.px(12, 10, BLACK)
    return sp


def aprilfool() -> Sprite:
    sp = icon()
    sp.ellipse(6, 14, 23, 25, RED, outline=BLACK)
    sp.line(23, 19, 29, 17, RED)
    sp.px(29, 17, BLACK)
    sp.ellipse(10, 16, 14, 19, WHITE)
    sp.rect(8, 10, 21, 13, RED, outline=BLACK)
    for x, y in [(24, 11), (27, 13), (25, 23), (29, 22)]:
        sp.px(x, y, WHITE)
    return sp


def april20() -> Sprite:
    sp = icon()
    # Stylized leaf fan: readable as "pot holiday" without drawing smoking
    # paraphernalia.
    for pts in [
        [(16, 5), (12, 16), (16, 15)],
        [(16, 5), (16, 15), (20, 16)],
        [(10, 7), (10, 18), (15, 16)],
        [(22, 7), (17, 16), (22, 18)],
        [(6, 12), (12, 20), (15, 17)],
        [(26, 12), (17, 17), (20, 20)],
        [(16, 15), (14, 27), (18, 27)],
    ]:
        poly(sp, pts, GREEN, DGREEN)
    sp.line(16, 14, 16, 28, DGREEN)
    # Peace medallion in front for the family-friendly read.
    sp.ellipse(9, 15, 23, 29, YELLOW, outline=BLACK)
    sp.ellipse(11, 17, 21, 27, TRANSPARENT, outline=BLACK)
    sp.line(16, 18, 16, 26, BLACK)
    sp.line(16, 22, 12, 26, BLACK)
    sp.line(16, 22, 20, 26, BLACK)
    for x, y in [(5, 6), (27, 7), (6, 24), (25, 25)]:
        sp.px(x, y, WHITE)
    return sp


def easter() -> Sprite:
    sp = icon()
    eggs = [(5, 13, 14, 27, PINK), (13, 8, 23, 27, YELLOW),
            (21, 14, 30, 27, PURPLE)]
    for x0, y0, x1, y1, color in eggs:
        sp.ellipse(x0, y0, x1, y1, color, outline=BLACK)
    sp.line(6, 20, 13, 20, WHITE)
    sp.line(14, 15, 22, 15, GREEN)
    sp.line(22, 21, 29, 21, WHITE)
    for x in range(15, 22, 2):
        sp.px(x, 19, PINK)
    return sp


def earthday() -> Sprite:
    sp = icon()
    sp.ellipse(4, 4, 28, 28, SKY, outline=BLACK)
    sp.rect(8, 10, 14, 15, GREEN)
    sp.line(14, 14, 20, 17, GREEN)
    sp.rect(18, 8, 24, 13, GREEN)
    sp.rect(11, 20, 18, 24, GREEN)
    sp.line(4, 16, 28, 16, BLACK)
    sp.line(16, 4, 16, 28, BLACK)
    sp.line(24, 25, 24, 31, TRUNK)
    tiny_leaf(sp, 21, 23, GREEN)
    tiny_leaf(sp, 27, 23, GREEN)
    return sp


def starwars() -> Sprite:
    sp = icon()
    sp.rect(14, 21, 18, 29, GRAY, outline=BLACK)
    sp.rect(13, 18, 19, 21, BLACK)
    sp.rect(13, 4, 19, 18, GREEN)
    sp.rect(15, 2, 17, 18, WHITE)
    for y in range(5, 18, 4):
        sp.px(11, y, SKY)
        sp.px(21, y, SKY)
    for x, y in [(5, 6), (25, 8), (8, 20), (27, 23)]:
        sp.px(x, y, WHITE)
    return sp


def cincomayo() -> Sprite:
    sp = icon()
    sp.ellipse(4, 18, 28, 25, RED, outline=BLACK)
    sp.ellipse(10, 8, 22, 22, RED, outline=BLACK)
    sp.line(10, 19, 22, 19, YELLOW)
    sp.line(12, 21, 20, 21, GREEN)
    sp.ellipse(23, 6, 30, 13, TRUNK, outline=BLACK)
    sp.line(26, 13, 26, 23, TRUNK)
    sp.px(25, 9, BLACK)
    sp.px(28, 10, BLACK)
    return sp


def mothersday() -> Sprite:
    sp = icon()
    sp.ellipse(9, 21, 22, 30, TRUNK, outline=BLACK)
    stems = [(12, 21, 8, 12), (14, 21, 14, 9), (16, 21, 20, 11),
             (18, 21, 24, 15)]
    for x0, y0, x1, y1 in stems:
        sp.line(x0, y0, x1, y1, GREEN)
    for cx, cy, c in [(8, 12, PINK), (14, 9, YELLOW),
                      (20, 11, WHITE), (24, 15, RED)]:
        tiny_flower(sp, cx, cy, c, YELLOW)
    compose_heart(sp, 6, 25, 2, PINK)
    return sp


def memorial() -> Sprite:
    sp = icon()
    sp.line(8, 4, 8, 30, TRUNK)
    sp.line(9, 4, 9, 30, TRUNK)
    draw_flag(sp, 10, 12, 17, 10)
    sp.line(4, 28, 14, 28, GRAY)
    for cx, cy in [(21, 26), (24, 20), (27, 26)]:
        sp.ellipse(cx - 2, cy - 2, cx + 2, cy + 2, RED, outline=BLACK)
        sp.px(cx, cy, BLACK)
    return sp


def fathersday() -> Sprite:
    sp = icon()
    poly(sp, [(15, 5), (20, 5), (22, 10), (18, 14), (14, 10)], ORANGE)
    poly(sp, [(14, 10), (22, 10), (24, 26), (17, 31), (10, 26)], DEEPBLUE)
    compose_heart(sp, 17, 18, 2, RED)
    sp.line(6, 24, 12, 21, GRAY)
    sp.rect(4, 23, 8, 25, GRAY, outline=BLACK)
    return sp


def summer() -> Sprite:
    sp = icon()
    for dx, dy in [(-12, 0), (12, 0), (0, -12), (0, 12),
                   (-8, -8), (8, -8), (-8, 8), (8, 8)]:
        sp.line(16, 16, 16 + dx, 16 + dy, YELLOW)
    sp.ellipse(6, 6, 26, 26, YELLOW, outline=ORANGE)
    sp.rect(9, 14, 14, 17, BLACK)
    sp.rect(18, 14, 23, 17, BLACK)
    sp.line(14, 15, 18, 15, BLACK)
    sp.line(11, 22, 21, 22, ORANGE)
    return sp


def pride() -> Sprite:
    sp = icon()
    stripes = [RED, ORANGE, YELLOW, GREEN, SKY, PURPLE]
    x0, y0, x1, y1 = 6, 9, 25, 22
    for i, c in enumerate(stripes):
        sp.rect(x0, y0 + i * 2, x1, y0 + i * 2 + 1, c)
    sp.outline_rect(x0, y0, x1, y1, BLACK)
    sp.line(6, 8, 6, 29, TRUNK)
    compose_heart(sp, 24, 25, 3, PINK)
    return sp


def july4th() -> Sprite:
    sp = icon()
    for dx, dy in [(-10, 0), (10, 0), (0, -10), (0, 10),
                   (-7, -7), (7, -7), (-7, 7), (7, 7)]:
        sp.line(16, 14, 16 + dx, 14 + dy, RED)
    for dx, dy in [(-5, -10), (5, -10), (-10, 5), (10, 5)]:
        sp.line(16, 14, 16 + dx, 14 + dy, WHITE)
    compose_star(sp, 16, 14, 3, YELLOW)
    draw_flag(sp, 5, 23, 20, 7)
    return sp


def moonland() -> Sprite:
    sp = icon()
    sp.ellipse(20, 3, 30, 13, GRAY, outline=BLACK)
    sp.px(24, 8, WHITE)
    poly(sp, [(14, 5), (22, 24), (8, 24)], WHITE)
    sp.rect(13, 13, 17, 18, SKY, outline=BLACK)
    poly(sp, [(10, 20), (5, 28), (12, 24)], RED)
    poly(sp, [(20, 20), (27, 28), (20, 24)], RED)
    sp.line(15, 24, 15, 29, ORANGE)
    sp.px(14, 30, YELLOW)
    sp.px(16, 30, YELLOW)
    return sp


def watermelon() -> Sprite:
    sp = icon()
    sp.ellipse(3, 7, 29, 29, GREEN, outline=BLACK)
    sp.ellipse(6, 10, 26, 26, RED, outline=BLACK)
    sp.rect(2, 3, 30, 16, TRANSPARENT)
    sp.line(5, 16, 27, 16, GREEN)
    for x, y in [(12, 20), (17, 19), (21, 23)]:
        sp.px(x, y, BLACK)
        sp.px(x + 1, y + 1, BLACK)
    return sp


def lefthand() -> Sprite:
    sp = icon()
    sp.ellipse(8, 12, 22, 28, SKIN, outline=BLACK)
    for x in (7, 10, 13, 16):
        sp.rect(x, 6, x + 3, 16, SKIN, outline=BLACK)
    sp.rect(20, 14, 26, 19, SKIN, outline=BLACK)
    sp.line(5, 24, 3, 30, TRUNK)
    sp.line(6, 24, 4, 30, TRUNK)
    sp.rect(3, 29, 8, 31, GRAY, outline=BLACK)
    sp.line(23, 8, 29, 4, YELLOW)
    sp.line(29, 4, 30, 5, BLACK)
    return sp


def hawaii() -> Sprite:
    sp = icon()
    cx, cy = 16, 16
    for x0, y0, x1, y1 in [
        (6, 5, 16, 15), (16, 5, 26, 15), (4, 15, 14, 25),
        (18, 15, 28, 25), (11, 20, 21, 30),
    ]:
        sp.ellipse(x0, y0, x1, y1, PINK, outline=BLACK)
    sp.ellipse(cx - 3, cy - 3, cx + 3, cy + 3, YELLOW, outline=BLACK)
    sp.line(16, 16, 23, 8, RED)
    sp.px(24, 7, YELLOW)
    return sp


def labor() -> Sprite:
    sp = icon()
    sp.ellipse(6, 10, 26, 23, YELLOW, outline=BLACK)
    sp.rect(4, 21, 28, 25, YELLOW, outline=BLACK)
    sp.line(16, 10, 16, 23, BLACK)
    sp.line(9, 23, 23, 23, ORANGE)
    sp.line(6, 29, 26, 9, GRAY)
    sp.rect(3, 27, 10, 30, TRUNK, outline=BLACK)
    return sp


def pirate() -> Sprite:
    sp = icon()
    sp.rect(8, 15, 24, 17, BLACK)
    poly(sp, [(7, 16), (13, 8), (18, 15)], BLACK, None)
    poly(sp, [(14, 15), (20, 8), (25, 16)], BLACK, None)
    sp.line(10, 19, 22, 19, RED)
    sp.ellipse(10, 22, 15, 27, WHITE, outline=BLACK)
    sp.ellipse(17, 22, 22, 27, WHITE, outline=BLACK)
    sp.px(12, 24, BLACK)
    sp.px(19, 24, BLACK)
    sp.line(13, 28, 20, 28, BLACK)
    sp.px(16, 12, WHITE)
    return sp


def autumn() -> Sprite:
    sp = icon()
    poly(sp, [(16, 3), (24, 10), (21, 12), (29, 17), (20, 17),
              (22, 25), (16, 20), (10, 25), (12, 17), (3, 17),
              (11, 12), (8, 10)], ORANGE)
    sp.line(16, 5, 16, 28, TRUNK)
    sp.line(16, 18, 7, 14, TRUNK)
    sp.line(16, 16, 25, 13, TRUNK)
    sp.ellipse(22, 22, 28, 29, TRUNK, outline=BLACK)
    sp.rect(24, 20, 26, 23, GREEN, outline=BLACK)
    return sp


def columbus() -> Sprite:
    sp = icon()
    sp.rect(5, 7, 27, 25, SAND, outline=BLACK)
    sp.line(8, 11, 14, 9, TRUNK)
    sp.line(14, 9, 19, 13, TRUNK)
    sp.line(19, 13, 24, 12, TRUNK)
    sp.ellipse(12, 12, 22, 22, DEEPBLUE, outline=BLACK)
    compose_star(sp, 17, 17, 5, WHITE)
    compose_star(sp, 17, 17, 2, RED)
    return sp


def election() -> Sprite:
    sp = icon()
    sp.rect(6, 14, 26, 27, WHITE, outline=BLACK)
    sp.rect(9, 11, 23, 15, DEEPBLUE, outline=BLACK)
    sp.rect(12, 5, 24, 13, WHITE, outline=BLACK)
    sp.line(14, 9, 17, 12, RED)
    sp.line(17, 12, 23, 6, RED)
    sp.line(10, 19, 22, 19, GRAY)
    sp.px(13, 23, RED)
    sp.px(19, 23, SKY)
    return sp


def veterans() -> Sprite:
    sp = icon()
    sp.rect(10, 4, 14, 15, DEEPBLUE, outline=BLACK)
    sp.rect(18, 4, 22, 15, RED, outline=BLACK)
    sp.rect(14, 4, 18, 16, WHITE)
    sp.ellipse(7, 14, 25, 30, YELLOW, outline=BLACK)
    compose_star(sp, 16, 22, 5, DEEPBLUE)
    for cx, cy in [(6, 20), (5, 24), (27, 20), (28, 24)]:
        tiny_leaf(sp, cx, cy, GREEN)
    return sp


def thanksgiving() -> Sprite:
    sp = icon()
    poly(sp, [(5, 12), (20, 18), (8, 28)], TRUNK)
    sp.ellipse(7, 13, 22, 26, ORANGE, outline=BLACK)
    sp.ellipse(17, 8, 27, 16, YELLOW, outline=BLACK)
    sp.ellipse(22, 15, 30, 22, RED, outline=BLACK)
    sp.ellipse(19, 21, 27, 28, GREEN, outline=BLACK)
    sp.line(8, 17, 4, 13, BLACK)
    sp.line(8, 22, 3, 25, BLACK)
    return sp


RENDERERS = {
    5: elvis,
    6: mlk,
    7: groundhog,
    8: valentine,
    9: superbowl,
    10: presidents,
    11: mardi,
    12: piday,
    13: spring,
    14: aprilfool,
    36: april20,
    15: easter,
    16: earthday,
    17: starwars,
    18: cincomayo,
    19: mothersday,
    20: memorial,
    21: fathersday,
    22: summer,
    23: pride,
    24: july4th,
    25: moonland,
    26: watermelon,
    27: lefthand,
    28: hawaii,
    29: labor,
    30: pirate,
    31: autumn,
    32: columbus,
    33: election,
    34: veterans,
    35: thanksgiving,
}


def rgba_for_preview(sp: Sprite) -> Image.Image:
    rgba = sp.image.convert("RGBA")
    alpha = sp.image.point(lambda p: 0 if p == TRANSPARENT else 255, "L")
    rgba.putalpha(alpha)
    return rgba


def build_preview(icons: list[tuple[dict, Sprite]], out_path: Path) -> None:
    cell = 72
    label_h = 22
    cols = SHEET_COLS
    rows = (len(icons) + cols - 1) // cols
    preview = Image.new("RGB", (cols * cell, rows * (cell + label_h)),
                        (28, 28, 36))
    draw = ImageDraw.Draw(preview)
    try:
        font = ImageFont.load_default()
    except Exception:
        font = None
    for idx, (h, sp) in enumerate(icons):
        col = idx % cols
        row = idx // cols
        x = col * cell
        y = row * (cell + label_h)
        for yy in range(0, cell, 8):
            for xx in range(0, cell, 8):
                c = (58, 58, 68) if ((xx + yy) // 8) % 2 else (42, 42, 52)
                draw.rectangle([x + xx, y + yy, x + xx + 7, y + yy + 7],
                               fill=c)
        draw.rectangle([x, y, x + cell - 1, y + cell - 1],
                       outline=(90, 90, 105))
        scaled = rgba_for_preview(sp).resize((ICON * 2, ICON * 2),
                                             Image.Resampling.NEAREST)
        preview.paste(scaled, (x + 4, y + 4), scaled)
        label = f"{h['id']:02d} {h['short_name'][:9]}"
        draw.text((x + 3, y + cell + 4), label, fill=(230, 230, 238),
                  font=font)
    preview.save(out_path, optimize=True)


def build_html(icons: list[tuple[dict, Sprite]], manifest: list[dict],
               out_path: Path) -> None:
    tiles = []
    for entry in manifest:
        label = f"{entry['id']:02d} {entry['short_name']}"
        src = Path(entry["file"]).name
        tiles.append(
            f'<div class="tile"><img src="{src}" alt="">'
            f'<div class="label">{label}</div></div>'
        )

    html = f"""<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Johnny Castaway Holiday Emblems</title>
  <style>
    :root {{
      color-scheme: dark;
      --bg: #181820;
      --panel: #262631;
      --text: #ededf4;
      --muted: #a4a4b8;
      --line: #3b3b4a;
      --accent: #f0c840;
    }}
    * {{ box-sizing: border-box; }}
    body {{
      margin: 0;
      background: var(--bg);
      color: var(--text);
      font: 14px/1.4 system-ui, -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif;
    }}
    header {{
      position: sticky;
      top: 0;
      z-index: 2;
      padding: 14px 20px;
      background: rgba(24, 24, 32, 0.96);
      border-bottom: 1px solid var(--line);
    }}
    h1 {{
      margin: 0;
      font-size: 18px;
      color: var(--accent);
      font-weight: 700;
    }}
    main {{
      padding: 18px 20px 28px;
      display: grid;
      gap: 20px;
    }}
    section {{ display: grid; gap: 10px; }}
    h2 {{
      margin: 0;
      font-size: 14px;
      color: var(--muted);
      font-weight: 700;
      text-transform: uppercase;
      letter-spacing: 0.04em;
    }}
    .checker {{
      display: inline-block;
      width: fit-content;
      max-width: 100%;
      padding: 10px;
      border: 1px solid var(--line);
      background:
        linear-gradient(45deg, #3a3a46 25%, transparent 25%),
        linear-gradient(-45deg, #3a3a46 25%, transparent 25%),
        linear-gradient(45deg, transparent 75%, #3a3a46 75%),
        linear-gradient(-45deg, transparent 75%, #3a3a46 75%),
        #2b2b36;
      background-size: 16px 16px;
      background-position: 0 0, 0 8px, 8px -8px, -8px 0;
      overflow: auto;
    }}
    img {{
      display: block;
      image-rendering: pixelated;
      image-rendering: crisp-edges;
    }}
    .sheet {{
      width: {SHEET_COLS * ICON * 4}px;
      height: {((len(icons) + SHEET_COLS - 1) // SHEET_COLS) * ICON * 4}px;
      max-width: none;
    }}
    .preview {{
      width: {SHEET_COLS * 72}px;
      max-width: 100%;
      height: auto;
    }}
    .grid {{
      display: grid;
      grid-template-columns: repeat(auto-fill, minmax(132px, 1fr));
      gap: 10px;
      max-width: 1180px;
    }}
    .tile {{
      background: var(--panel);
      border: 1px solid var(--line);
      padding: 8px;
      display: grid;
      gap: 7px;
      justify-items: center;
    }}
    .tile img {{
      width: 96px;
      height: 96px;
    }}
    .label {{
      width: 100%;
      color: var(--text);
      font: 11px/1.2 ui-monospace, SFMono-Regular, Menlo, Consolas, monospace;
      text-align: center;
      white-space: nowrap;
      overflow: hidden;
      text-overflow: ellipsis;
    }}
  </style>
</head>
<body>
  <header><h1>Johnny Castaway Holiday Emblems</h1></header>
  <main>
    <section>
      <h2>Packed Transparent Sheet, 4x</h2>
      <div class="checker">
        <img class="sheet" src="holiday-emblems-sheet.png" alt="Packed holiday emblem sprite sheet">
      </div>
    </section>
    <section>
      <h2>Labeled Preview</h2>
      <div class="checker">
        <img class="preview" src="holiday-emblems-preview.png" alt="Labeled holiday emblem preview">
      </div>
    </section>
    <section>
      <h2>Individual Icons, 3x</h2>
      <div class="grid">{''.join(tiles)}</div>
    </section>
  </main>
</body>
</html>
"""
    out_path.write_text(html, encoding="utf-8")


def main() -> int:
    holidays = yaml.safe_load(open(YAML_PATH, "r", encoding="utf-8"))
    reviewable = [h for h in holidays if h.get("existing_sprite") is None]
    missing = [h["id"] for h in reviewable if h["id"] not in RENDERERS]
    if missing:
        sys.stderr.write(f"error: missing emblem renderers for ids {missing}\n")
        return 1

    OUT_DIR.mkdir(parents=True, exist_ok=True)

    rendered: list[tuple[dict, Sprite]] = []
    manifest = []
    for h in reviewable:
        sp = RENDERERS[h["id"]]()
        slug = slugify(h["short_name"])
        out = OUT_DIR / f"{h['id']:02d}-{slug}.png"
        sp.save(str(out))
        rendered.append((h, sp))
        manifest.append({
            "id": h["id"],
            "name": h["name"],
            "short_name": h["short_name"],
            "file": str(out.relative_to(REPO)),
            "size": [ICON, ICON],
        })

    rows = (len(rendered) + SHEET_COLS - 1) // SHEET_COLS
    sheet = Sprite(SHEET_COLS * ICON, rows * ICON, fill=TRANSPARENT)
    for idx, (h, sp) in enumerate(rendered):
        x = (idx % SHEET_COLS) * ICON
        y = (idx // SHEET_COLS) * ICON
        sheet.image.paste(sp.image, (x, y))
        manifest[idx]["sheet"] = {
            "file": str((OUT_DIR / "holiday-emblems-sheet.png").relative_to(REPO)),
            "cell": idx,
            "x": x,
            "y": y,
            "w": ICON,
            "h": ICON,
        }

    sheet_path = OUT_DIR / "holiday-emblems-sheet.png"
    sheet.save(str(sheet_path))
    build_preview(rendered, OUT_DIR / "holiday-emblems-preview.png")
    build_html(rendered, manifest, OUT_DIR / "review.html")
    (OUT_DIR / "manifest.json").write_text(
        json.dumps({"icons": manifest}, indent=2) + "\n",
        encoding="utf-8",
    )

    print(f"Wrote {len(rendered)} 32x32 emblems")
    print(f"Sheet:   {sheet_path.relative_to(REPO)}")
    print(f"Preview: {(OUT_DIR / 'holiday-emblems-preview.png').relative_to(REPO)}")
    print(f"Review:  {(OUT_DIR / 'review.html').relative_to(REPO)}")
    print(f"Icons:   {OUT_DIR.relative_to(REPO)}/")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
