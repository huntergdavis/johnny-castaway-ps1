"""
holidays_art_lib — shared building blocks for Johnny Castaway PS1 holiday art.

The Johnny Castaway PS1 port has 4 hand-drawn holiday decorations baked into
HOLIDAY.PSB. This library is the substrate for generating the added holiday
emblems in a similar visual register: 4-bit indexed pixel art using a
16-color shared CLUT. Palette index 0 is transparent.

Public API
----------

  PALETTE         — 16-color shared CLUT (RGB tuples). Index 0 is transparent.
  PALETTE_NAMES   — human-readable name per index (for debugging).
  Sprite(w, h)    — wrapper around a PIL "P"-mode image, palette pre-applied.
  compose_*       — small shape helpers used by the emblem generator.
  save_4bit_png(sprite, path) — write a 4-bit indexed PNG suitable for
                                feeding into the existing transcode-bmp-ps1
                                BMP/PSB pipeline.

Style notes
-----------

The original 4 sprites (Halloween / St Patrick / Christmas / New Year) lean
into:
  * Heavy black or dark outlines around shapes (1px line)
  * Saturated, slightly desaturated colors — not full-bright neon
  * Compact compositions (~40-150 px wide) anchored to specific island
    coordinates so they sit beside the palm tree
  * Minimal but readable detail at PS1 viewing distance

This library mirrors that aesthetic with a small primitive set. The active
holiday art generator creates 32x32 transparent emblems, not full island
scenes.
"""

from __future__ import annotations

from PIL import Image, ImageDraw

# ---------------------------------------------------------------------------
# Palette — the actual Johnny Castaway 16-color CLUT from JOHNNYCAST.PAL.
# Index 0 is the transparent magenta key; every BMP/SCR sprite in-game uses
# this same palette, so holiday emblems must use these exact indices.
# ---------------------------------------------------------------------------

PALETTE = [
    (168,   0, 168),  # 0  transparent magenta
    (  0,   0, 168),  # 1  dark blue
    (  0, 168,   0),  # 2  dark green
    (  0, 168, 168),  # 3  dark cyan
    (168,   0,   0),  # 4  dark red
    (  0,   0,   0),  # 5  black
    (168, 168,   0),  # 6  olive / dark yellow
    (212, 212, 212),  # 7  light gray
    (128, 128, 128),  # 8  medium gray
    (  0,   0, 252),  # 9  bright blue
    (  0, 252,   0),  # 10 bright green
    (  0, 252, 252),  # 11 bright cyan
    (252,   0,   0),  # 12 bright red
    (252,   0, 252),  # 13 bright magenta
    (252, 252,   0),  # 14 bright yellow
    (252, 252, 252),  # 15 white
]

PALETTE_NAMES = [
    "transparent", "darkblue", "darkgreen", "darkcyan", "darkred", "black",
    "olive", "lightgray", "gray", "blue", "green", "cyan", "red",
    "magenta", "yellow", "white",
]

# Convenience indices
TRANSPARENT = 0
WHITE       = 15
BLACK       = 5
SKIN        = 14
TRUNK       = 6
GREEN       = 10
DGREEN      = 2
SKY         = 11
DEEPBLUE    = 1
SAND        = 14
RED         = 12
YELLOW      = 14
ORANGE      = 14
PINK        = 13
PURPLE      = 13
GRAY        = 8


def _flat_palette() -> list[int]:
    """PIL "P" mode wants a flat 768-byte palette — 256 entries × 3 channels.
    We have 16 colors; the rest stay at zero."""
    flat = []
    for r, g, b in PALETTE:
        flat.extend([r, g, b])
    flat.extend([0] * (768 - len(flat)))
    return flat


PIL_PALETTE = _flat_palette()


# ---------------------------------------------------------------------------
# Sprite — thin wrapper around an indexed-mode PIL image
# ---------------------------------------------------------------------------

class Sprite:
    """An indexed-mode PIL image whose pixel values are 0..15 palette indices."""

    def __init__(self, w: int, h: int, fill: int = TRANSPARENT) -> None:
        self.image = Image.new("P", (w, h), color=fill)
        self.image.putpalette(PIL_PALETTE)
        self.draw = ImageDraw.Draw(self.image)
        self.w = w
        self.h = h

    def px(self, x: int, y: int, color: int) -> None:
        """Set a single pixel by palette index. No-op if out of bounds."""
        if 0 <= x < self.w and 0 <= y < self.h:
            self.image.putpixel((x, y), color)

    def rect(self, x0: int, y0: int, x1: int, y1: int, color: int,
             outline: int | None = None) -> None:
        """Inclusive rectangle, optionally with a 1px outline."""
        self.draw.rectangle([x0, y0, x1, y1], fill=color, outline=outline)

    def outline_rect(self, x0: int, y0: int, x1: int, y1: int,
                     color: int) -> None:
        """1px hollow rectangle that does NOT fill the interior. Use this
        when you want to draw a border on top of existing pixels — `rect`
        with `fill=TRANSPARENT` would overwrite the interior with palette
        index 0, erasing whatever was drawn there."""
        self.draw.line([x0, y0, x1, y0], fill=color)
        self.draw.line([x0, y1, x1, y1], fill=color)
        self.draw.line([x0, y0, x0, y1], fill=color)
        self.draw.line([x1, y0, x1, y1], fill=color)

    def line(self, x0: int, y0: int, x1: int, y1: int, color: int) -> None:
        self.draw.line([x0, y0, x1, y1], fill=color)

    def ellipse(self, x0: int, y0: int, x1: int, y1: int, color: int,
                outline: int | None = None) -> None:
        self.draw.ellipse([x0, y0, x1, y1], fill=color, outline=outline)

    def text(self, x: int, y: int, s: str, color: int = BLACK) -> None:
        """Tiny text using PIL's default bitmap font (5px tall, ASCII only).
        Useful for tiny marks such as pi symbols or labels when they remain
        readable at 32x32."""
        self.draw.text((x, y), s, fill=color)

    def save(self, path: str) -> None:
        """Save as a 4-bit indexed PNG with palette index 0 marked
        transparent. Browsers/Pillow display real transparency for the
        TRANSPARENT pixels."""
        self.image.save(path, optimize=False, transparency=TRANSPARENT)


# ---------------------------------------------------------------------------
# Primitive composition — common Johnny Castaway scene fragments. Each takes
# a Sprite and an XY anchor and draws onto it. Designed to be combined.
# ---------------------------------------------------------------------------

def compose_sand_strip(sp: Sprite, y_top: int) -> None:
    """Sand from y_top to bottom. Index 9 (sand)."""
    sp.rect(0, y_top, sp.w - 1, sp.h - 1, SAND)


def compose_sky(sp: Sprite, y_bot: int, color: int = SKY) -> None:
    """Sky band from top to y_bot."""
    sp.rect(0, 0, sp.w - 1, y_bot, color)


def compose_palm_tree(sp: Sprite, anchor_x: int, base_y: int,
                       trunk_h: int = 18, frond_r: int = 8) -> None:
    """Stylized coconut palm tree, base at (anchor_x, base_y), trunk_h px
    tall, fronds spread frond_r px around the top.

    Trunk is a 2-px-wide banana curve — base sits at anchor_x, top
    leans ~3 px toward the center for a Sierra-Castaway silhouette.
    Fronds radiate from the trunk top; each line gets a 1-px DGREEN
    shadow underneath for depth.
    """
    # Trunk curve: parabolic offset so the tree leans gently. dx is
    # capped to 3 so very tall palms don't become L-shaped.
    bend = min(3, trunk_h // 6)
    pts = []
    for i in range(trunk_h + 1):
        # i=0 at base, i=trunk_h at top. Quadratic ease so most of the
        # curve is in the upper half.
        t = i / max(1, trunk_h)
        dx = int(round(bend * (t * t)))
        pts.append((anchor_x - dx, base_y - i))
    # Draw the 2-px wide trunk by tracing the curve twice.
    for (px, py) in pts:
        if 0 <= px < sp.w and 0 <= py < sp.h:
            sp.px(px, py, TRUNK)
            if px + 1 < sp.w:
                sp.px(px + 1, py, TRUNK)
    # Lateral notches every 4 pixels for Sierra-style trunk segmentation.
    for i in range(2, trunk_h - 2, 4):
        nx = pts[i][0]
        ny = pts[i][1]
        if 0 <= nx + 2 < sp.w and 0 <= ny < sp.h:
            sp.px(nx + 2, ny, DGREEN)
    top_x, top_y = pts[-1]
    # Fronds — line strokes radiating from the trunk top.
    for dx, dy in [(-frond_r, -2), (-frond_r // 2, -frond_r // 2),
                   ( 0, -frond_r), ( frond_r // 2, -frond_r // 2),
                   ( frond_r, -2), (-frond_r // 2, frond_r // 4),
                   ( frond_r // 2, frond_r // 4)]:
        sp.line(top_x, top_y, top_x + dx, top_y + dy, GREEN)
        # Darker shadow line one pixel below for depth
        sp.line(top_x, top_y + 1, top_x + dx, top_y + dy + 1, DGREEN)


def compose_johnny_simple(sp: Sprite, x: int, base_y: int,
                           hat_color: int | None = None,
                           shirt_color: int = RED) -> None:
    """Tiny Johnny silhouette — 6 px wide × 12 px tall stick figure.
    Optional hat (index color) sits on top.

    Layout:
      ##    head (skin) with eyes + smile
      ##
      :##:  shirt body (shirt_color)
      ####
      ####
      :##:  legs (darkskin)
    """
    # Head
    sp.rect(x + 1, base_y - 11, x + 4, base_y - 9, SKIN)
    # Hair scruff: a couple of dark-brown pixels above the forehead so
    # Johnny has a recognizable shaggy look even without a hat.
    if hat_color is None:
        sp.px(x + 1, base_y - 12, TRUNK)
        sp.px(x + 2, base_y - 12, TRUNK)
        sp.px(x + 3, base_y - 12, TRUNK)
    else:
        sp.rect(x + 0, base_y - 13, x + 5, base_y - 12, hat_color)
        sp.rect(x + 1, base_y - 12, x + 4, base_y - 12, hat_color)
    # Eyes (top row of the face)
    sp.px(x + 2, base_y - 10, BLACK)
    sp.px(x + 3, base_y - 10, BLACK)
    # Tiny smile — a single black pixel under the eyes
    sp.px(x + 2, base_y - 9, BLACK)
    sp.px(x + 3, base_y - 9, BLACK)
    # Shirt body
    sp.rect(x + 0, base_y - 8, x + 5, base_y - 4, shirt_color)
    # Legs (skin/dark)
    sp.rect(x + 1, base_y - 3, x + 2, base_y, TRUNK)
    sp.rect(x + 3, base_y - 3, x + 4, base_y, TRUNK)


def compose_speech_bubble(sp: Sprite, cx: int, cy: int, w: int = 14,
                           h: int = 10, text: str = "",
                           tail_dir: str = "down") -> None:
    """A tiny speech bubble centered at (cx, cy) with an optional tail.

    tail_dir: "down" (default), "down-left", "down-right", or "none".
    Tail points outward from the lower edge of the bubble — useful when
    the bubble is above a speaking character. Pass "none" to skip.
    """
    x0, y0 = cx - w // 2, cy - h // 2
    x1, y1 = x0 + w, y0 + h
    sp.ellipse(x0, y0, x1, y1, WHITE, outline=BLACK)
    if text:
        sp.text(x0 + 2, y0 + 1, text[:3], BLACK)
    # Tail — three pixels from the bubble edge to a point below.
    if tail_dir == "none":
        return
    tail_x = {
        "down":       cx,
        "down-left":  cx - w // 4,
        "down-right": cx + w // 4,
    }.get(tail_dir, cx)
    sp.line(tail_x, y1, tail_x - 1, y1 + 2, BLACK)
    sp.line(tail_x - 1, y1 + 2, tail_x + 2, y1 + 1, BLACK)
    # Fill the tiny tail interior with WHITE so it looks attached.
    sp.px(tail_x, y1 + 1, WHITE)


def compose_outlined_rect(sp: Sprite, x0: int, y0: int, x1: int, y1: int,
                           fill: int, outline: int = BLACK) -> None:
    sp.rect(x0, y0, x1, y1, fill, outline=outline)


def compose_star(sp: Sprite, cx: int, cy: int, r: int, color: int) -> None:
    """Star centered at (cx, cy) with outer radius r.

    For r ≤ 2, draw a 4-point cross (anything more detailed pixelates
    into mush). For r ≥ 3, draw a real 5-point star by filling the
    pentagram polygon — the points line up so the top tip is straight
    up, and the lower two tips spread outward, the recognizable
    holiday-card shape.
    """
    if r <= 2:
        sp.line(cx - r, cy, cx + r, cy, color)
        sp.line(cx, cy - r, cx, cy + r, color)
        sp.px(cx, cy, color)
        return
    # 5-point star polygon: alternate outer (radius r) and inner
    # (radius r/2.5) vertices, starting at the top.
    import math
    pts: list[tuple[int, int]] = []
    for i in range(10):
        angle = -math.pi / 2 + i * math.pi / 5
        rr = r if i % 2 == 0 else r / 2.5
        pts.append((cx + int(round(rr * math.cos(angle))),
                    cy + int(round(rr * math.sin(angle)))))
    sp.draw.polygon(pts, fill=color)


def compose_heart(sp: Sprite, cx: int, cy: int, r: int, color: int) -> None:
    """Tiny heart shape — two circles + downward triangle."""
    sp.ellipse(cx - r, cy - r // 2, cx, cy + r // 2, color)
    sp.ellipse(cx, cy - r // 2, cx + r, cy + r // 2, color)
    for i in range(r):
        x0 = cx - r + i
        x1 = cx + r - i
        sp.line(x0, cy + i, x1, cy + i, color)


def compose_horizon(sp: Sprite, y: int) -> None:
    """A 1-px horizon line — separates sky from water."""
    sp.line(0, y, sp.w - 1, y, DEEPBLUE)


# ---------------------------------------------------------------------------
# Save helpers
# ---------------------------------------------------------------------------

def save_png(sprite: Sprite, path: str) -> None:
    """Save the sprite as a 4-bit indexed PNG (16-color palette)."""
    sprite.save(path)


# ---------------------------------------------------------------------------
# Self-test
# ---------------------------------------------------------------------------

if __name__ == "__main__":
    import os
    out = "/tmp/holidays_art_lib_selftest.png"
    sp = Sprite(32, 32, fill=TRANSPARENT)
    compose_heart(sp, 16, 14, 7, RED)
    compose_star(sp, 24, 7, 3, YELLOW)
    sp.rect(7, 24, 24, 27, TRUNK, outline=BLACK)
    sp.save(out)
    print(f"Self-test sprite saved to {out}")
    print(f"Palette: {len(PALETTE)} colors, {len(PALETTE_NAMES)} names")
