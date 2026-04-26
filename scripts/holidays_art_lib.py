"""
holidays_art_lib — shared building blocks for Johnny Castaway PS1 holiday art.

The Johnny Castaway PS1 port has 4 hand-drawn holiday decorations baked into
HOLIDAY.PSB. This library is the substrate for generating 31 NEW holiday
sprites in a similar visual register: 4-bit indexed pixel art using a
16-color shared CLUT, drawn at variable per-holiday dimensions.

Public API
----------

  PALETTE         — 16-color shared CLUT (RGB tuples). Index 0 is transparent.
  PALETTE_NAMES   — human-readable name per index (for debugging).
  Sprite(w, h)    — wrapper around a PIL "P"-mode image, palette pre-applied.
  draw_*          — low-level primitive drawing on a Sprite.
  compose_*       — higher-level scene fragments (palm tree, Johnny, sand).
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

This library mirrors that aesthetic: the primitive set covers what most
holidays need (palm tree, Johnny silhouette, simple props, sky tints).
Variants are produced via deterministic per-holiday compose functions
elsewhere; this file is just the toolkit.
"""

from __future__ import annotations

from PIL import Image, ImageDraw

# ---------------------------------------------------------------------------
# Palette — chosen to cover the full design-doc palette hints across 35
# holidays while staying within PS1's 16-color-per-sprite CLUT budget. We
# share a single CLUT across all holidays for simplicity (alternative would
# be per-frame CLUTs in PSB; deferred). Each holiday composes from this set.
# ---------------------------------------------------------------------------

PALETTE = [
    (  0,   0,   0),  # 0  transparent (the loader treats index 0 as see-through)
    (255, 255, 255),  # 1  white
    (  0,   0,   0),  # 2  black (outlines)
    (224, 192, 144),  # 3  light skin
    (160, 110,  72),  # 4  dark skin / palm trunk
    ( 56, 132,  64),  # 5  palm green / grass
    ( 24,  72,  32),  # 6  dark green
    ( 64, 128, 224),  # 7  sky blue / water
    ( 16,  32, 104),  # 8  deep blue / night
    (228, 200, 120),  # 9  sand / gold
    (200,  56,  56),  # 10 red / cardinal
    (240, 200,  64),  # 11 sunshine yellow
    (224, 120,  64),  # 12 orange / pumpkin
    (216, 144, 192),  # 13 pink
    (160,  72, 168),  # 14 purple
    (128, 128, 128),  # 15 gray
]

PALETTE_NAMES = [
    "transparent", "white", "black", "skin", "darkskin/trunk", "green",
    "darkgreen", "sky", "deepblue", "sand/gold", "red", "yellow",
    "orange", "pink", "purple", "gray",
]

# Convenience indices
TRANSPARENT = 0
WHITE       = 1
BLACK       = 2
SKIN        = 3
TRUNK       = 4
GREEN       = 5
DGREEN      = 6
SKY         = 7
DEEPBLUE    = 8
SAND        = 9
RED         = 10
YELLOW      = 11
ORANGE      = 12
PINK        = 13
PURPLE      = 14
GRAY        = 15


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
        Useful for placeholder labels in early variants. Real holiday sprites
        should use composed art, not text."""
        self.draw.text((x, y), s, fill=color)

    def save(self, path: str) -> None:
        """Save as a 4-bit indexed PNG."""
        # PIL auto-detects bit depth from the palette range used.
        self.image.save(path, optimize=False)


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
    """Stylized palm tree centered at (anchor_x, base_y). The base sits at
    base_y; trunk goes upward trunk_h pixels; fronds spread frond_r pixels
    around the top."""
    top_y = base_y - trunk_h
    # Trunk — slight curve achieved with 2-px offset alternating columns.
    sp.line(anchor_x, base_y, anchor_x, top_y, TRUNK)
    sp.line(anchor_x + 1, base_y - 1, anchor_x + 1, top_y, TRUNK)
    # Fronds — a few line strokes radiating from the top.
    for dx, dy in [(-frond_r, -2), (-frond_r // 2, -frond_r // 2),
                   ( 0, -frond_r), ( frond_r // 2, -frond_r // 2),
                   ( frond_r, -2), (-frond_r // 2, frond_r // 4),
                   ( frond_r // 2, frond_r // 4)]:
        sp.line(anchor_x, top_y, anchor_x + dx, top_y + dy, GREEN)
        # Darker shadow line one pixel below for depth
        sp.line(anchor_x, top_y + 1, anchor_x + dx, top_y + dy + 1, DGREEN)


def compose_johnny_simple(sp: Sprite, x: int, base_y: int,
                           hat_color: int | None = None,
                           shirt_color: int = RED) -> None:
    """Tiny Johnny silhouette — 6 px wide × 12 px tall stick figure.
    Optional hat (index color) sits on top.

    Layout:
      ##    head (skin)
      ##
      :##:  shirt body (shirt_color)
      ####
      ####
      :##:  legs (darkskin)
    """
    # Head
    sp.rect(x + 1, base_y - 11, x + 4, base_y - 9, SKIN)
    # Optional hat above the head
    if hat_color is not None:
        sp.rect(x + 0, base_y - 13, x + 5, base_y - 12, hat_color)
        sp.rect(x + 1, base_y - 12, x + 4, base_y - 12, hat_color)
    # Eyes
    sp.px(x + 2, base_y - 10, BLACK)
    sp.px(x + 3, base_y - 10, BLACK)
    # Shirt body
    sp.rect(x + 0, base_y - 8, x + 5, base_y - 4, shirt_color)
    # Legs (skin/dark)
    sp.rect(x + 1, base_y - 3, x + 2, base_y, TRUNK)
    sp.rect(x + 3, base_y - 3, x + 4, base_y, TRUNK)


def compose_speech_bubble(sp: Sprite, cx: int, cy: int, w: int = 14,
                           h: int = 10, text: str = "") -> None:
    """A tiny speech bubble centered at (cx, cy)."""
    x0, y0 = cx - w // 2, cy - h // 2
    x1, y1 = x0 + w, y0 + h
    sp.ellipse(x0, y0, x1, y1, WHITE, outline=BLACK)
    if text:
        sp.text(x0 + 2, y0 + 1, text[:3], BLACK)


def compose_outlined_rect(sp: Sprite, x0: int, y0: int, x1: int, y1: int,
                           fill: int, outline: int = BLACK) -> None:
    sp.rect(x0, y0, x1, y1, fill, outline=outline)


def compose_star(sp: Sprite, cx: int, cy: int, r: int, color: int) -> None:
    """5-point star approximated with a couple of intersecting lines.
    Crude but recognizable at PS1 viewing distance."""
    sp.line(cx - r, cy, cx + r, cy, color)
    sp.line(cx, cy - r, cx, cy + r, color)
    sp.line(cx - r * 3 // 4, cy - r * 3 // 4, cx + r * 3 // 4, cy + r * 3 // 4, color)
    sp.line(cx - r * 3 // 4, cy + r * 3 // 4, cx + r * 3 // 4, cy - r * 3 // 4, color)
    sp.px(cx, cy, color)


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
# Night-variant generator
# ---------------------------------------------------------------------------
#
# v5 NIGHT: same scene, but at midnight. Implemented as a post-pass on a
# day-time sprite — we walk the indexed pixel data, remap a small set of
# "day" indices to their "night" equivalents, and sprinkle stars + a moon
# into whatever region used to be sky. This avoids the cost of authoring
# 31 hand-drawn night sprites while still giving the owner a meaningfully
# different alternate to choose from.

# Per-index recolor used by `as_night`. SKY → DEEPBLUE so any explicit
# daylight sky shifts to navy. ORANGE (sunsets/dawn) becomes PURPLE for
# twilight. The sprite's background color (whatever it is — often YELLOW
# for MLK, ORANGE for thanksgiving, GREEN for super bowl) is detected at
# runtime and ALSO remapped to DEEPBLUE so the night version actually
# reads as night regardless of v1's background choice.
_NIGHT_RECOLOR_FIXED = {
    SKY:    DEEPBLUE,
    ORANGE: PURPLE,
}


def as_night(day_sprite: Sprite, *,
             moon_at: tuple[int, int] | None = None,
             star_count: int = 0,
             keep_sand: bool = True) -> Sprite:
    """Return a new Sprite that is a night-time recolor of `day_sprite`.

    The sprite's background color is detected (top-left corner pixel,
    fallback to most-common index in the upper third) and remapped to
    DEEPBLUE so any sky band — yellow, orange, sky-blue, even green —
    becomes a believable night sky. ORANGE elsewhere (sunsets) becomes
    PURPLE for twilight. Everything else passes through so foreground
    subjects (Johnny, props, palm fronds) still read.

    For sprites whose v1 is ALREADY night (bg = DEEPBLUE), inverting the
    palette would barely move the needle. Those route to a "dawn" mode
    instead: bg → SKY, DEEPBLUE → SKY, with sun-yellow lent to a few
    accents. Either way you get a meaningfully different alternate.

    Then we lay WHITE stars and a small moon over what used to be the
    background band (or a sun, in dawn mode).

    moon_at:     (x, y) for a small WHITE moon disc with a BLACK outline.
                 Defaults to the upper-right corner.
    star_count:  number of WHITE star pixels to sprinkle in what was
                 the sky region. Defaults to ~ (w * h) / 80.
    keep_sand:   leave SAND pixels untouched. If False, sand also dims
                 to TRUNK.
    """
    src = day_sprite.image
    # Detect background color: corner pixel is the safest bet, since the
    # renderers all start with `Sprite(w, h, fill=BG)`. Skip TRANSPARENT.
    bg_idx = src.getpixel((0, 0))
    if bg_idx == TRANSPARENT:
        from collections import Counter
        top_pixels = [src.getpixel((x, y))
                      for y in range(max(1, day_sprite.h // 4))
                      for x in range(day_sprite.w)]
        if top_pixels:
            bg_idx = Counter(top_pixels).most_common(1)[0][0]

    is_already_night = bg_idx in (DEEPBLUE, BLACK)
    if is_already_night:
        return _as_dawn(day_sprite, src, moon_at, star_count, keep_sand)

    new = Sprite(day_sprite.w, day_sprite.h, fill=DEEPBLUE)
    # Build the dynamic recolor map: detected background → DEEPBLUE,
    # plus the fixed mappings. The detected bg wins when both apply.
    recolor = dict(_NIGHT_RECOLOR_FIXED)
    if bg_idx not in (TRANSPARENT, BLACK, DEEPBLUE):
        recolor[bg_idx] = DEEPBLUE
    # Walk every pixel and remap.
    src_pixels = list(src.getdata())
    out = []
    for p in src_pixels:
        if not keep_sand and p == SAND:
            out.append(TRUNK)
        elif p in recolor:
            out.append(recolor[p])
        else:
            out.append(p)
    new.image.putdata(out)

    # Determine star_count default — roughly proportional to area.
    if star_count == 0:
        star_count = max(8, (day_sprite.w * day_sprite.h) // 80)

    # Sprinkle stars in pixels that landed on DEEPBLUE (= former sky) and
    # are above the bottom 25% (don't put stars in the sand/water).
    import random
    rng = random.Random(day_sprite.w * 1009 + day_sprite.h)
    placed = 0
    horizon = int(day_sprite.h * 0.7)
    attempts = 0
    while placed < star_count and attempts < star_count * 8:
        attempts += 1
        x = rng.randrange(day_sprite.w)
        y = rng.randrange(horizon)
        idx = new.image.getpixel((x, y))
        if idx == DEEPBLUE:
            new.px(x, y, WHITE)
            placed += 1

    # A few twinkles — 3-pixel cross
    for _ in range(min(4, star_count // 6)):
        x = rng.randrange(2, day_sprite.w - 2)
        y = rng.randrange(2, horizon - 2)
        if new.image.getpixel((x, y)) == DEEPBLUE:
            new.px(x, y, WHITE)
            new.px(x - 1, y, WHITE); new.px(x + 1, y, WHITE)
            new.px(x, y - 1, WHITE); new.px(x, y + 1, WHITE)

    # Moon — pick the corner with the most DEEPBLUE pixels in a 14×14
    # square so we don't paint over Johnny / a flag / etc. The default
    # `moon_at` overrides the heuristic if the caller is opinionated.
    if moon_at is None:
        candidates = [
            (day_sprite.w - 8, 8),     # upper right (favored)
            (8, 8),                    # upper left
            (day_sprite.w // 2, 8),    # upper center
            (day_sprite.w - 8, day_sprite.h // 4),
        ]
        def open_score(cx, cy):
            score = 0
            for yy in range(max(0, cy - 6), min(day_sprite.h, cy + 7)):
                for xx in range(max(0, cx - 6), min(day_sprite.w, cx + 7)):
                    if new.image.getpixel((xx, yy)) == DEEPBLUE:
                        score += 1
            return score
        mx, my = max(candidates, key=lambda c: open_score(*c))
    else:
        mx, my = moon_at
    if 0 <= mx < day_sprite.w and 0 <= my < day_sprite.h:
        new.ellipse(mx - 5, my - 5, mx + 5, my + 5, WHITE, outline=BLACK)
        # Crescent shadow — a partial DEEPBLUE arc on the right side
        new.ellipse(mx - 1, my - 4, mx + 6, my + 4, DEEPBLUE)

    return new


def _as_dawn(day_sprite: Sprite, src,
             sun_at: tuple[int, int] | None,
             star_count: int,
             keep_sand: bool) -> Sprite:
    """Inverted recolor for v1s that are ALREADY night (bg=DEEPBLUE).
    Maps the night sky to a dawn band (DEEPBLUE→PURPLE→ORANGE→YELLOW
    gradient), darkens stars (WHITE→YELLOW so they read as the last
    morning sparkles), and places a sun in the upper-right with a few
    light rays."""
    new = Sprite(day_sprite.w, day_sprite.h, fill=PURPLE)
    src_pixels = list(src.getdata())
    out = []
    # Recolor: night-blacks/deep-blues become a dawn-purple base. White
    # stars dim to yellow. Black stays black (silhouettes).
    for p in src_pixels:
        if p == DEEPBLUE:
            out.append(PURPLE)
        elif p == WHITE:
            out.append(YELLOW)
        elif p == BLACK:
            # Most BLACK in night sprites is silhouette outlines. Keep it.
            out.append(BLACK)
        else:
            out.append(p)
    new.image.putdata(out)

    # Paint a horizontal dawn gradient across what is still PURPLE: the
    # top is purple, middle ORANGE, bottom YELLOW just above the horizon.
    horizon = int(day_sprite.h * 0.65)
    upper_third = day_sprite.h // 3
    middle_third = day_sprite.h * 2 // 3
    for y in range(day_sprite.h):
        if y >= horizon:
            continue
        if y < upper_third:
            band_color = PURPLE
        elif y < middle_third:
            band_color = ORANGE
        else:
            band_color = YELLOW
        for x in range(day_sprite.w):
            if new.image.getpixel((x, y)) == PURPLE:
                new.px(x, y, band_color)

    # Sun in the corner with the most "still-purple-or-band" space, so
    # it doesn't land on top of Johnny or a tree.
    if sun_at is None:
        candidates = [
            (day_sprite.w - 10, 10),
            (10, 10),
            (day_sprite.w // 2, 12),
            (day_sprite.w - 14, day_sprite.h // 5),
        ]
        def open_score_dawn(cx, cy):
            score = 0
            for yy in range(max(0, cy - 8), min(day_sprite.h, cy + 9)):
                for xx in range(max(0, cx - 8), min(day_sprite.w, cx + 9)):
                    px = new.image.getpixel((xx, yy))
                    if px in (PURPLE, ORANGE, YELLOW):
                        score += 1
            return score
        sx, sy = max(candidates, key=lambda c: open_score_dawn(*c))
    else:
        sx, sy = sun_at
    if 0 <= sx < day_sprite.w and 0 <= sy < day_sprite.h:
        # Sun rays first (so disc paints over them at center)
        for dx, dy in [(-10, 0), (10, 0), (0, -10), (0, 10),
                       (-7, -7), (7, -7), (-7, 7), (7, 7)]:
            new.line(sx, sy, sx + dx, sy + dy, YELLOW)
        new.ellipse(sx - 6, sy - 6, sx + 6, sy + 6, YELLOW, outline=ORANGE)
        new.ellipse(sx - 3, sy - 3, sx + 3, sy + 3, WHITE)

    # A few last-night stars near the top (maybe 4-6, clamped down).
    if star_count == 0:
        star_count = max(4, (day_sprite.w * day_sprite.h) // 320)
    import random
    rng = random.Random(day_sprite.w * 1013 + day_sprite.h * 7)
    placed = 0
    attempts = 0
    while placed < star_count and attempts < star_count * 8:
        attempts += 1
        x = rng.randrange(day_sprite.w)
        y = rng.randrange(max(1, day_sprite.h // 3))
        idx = new.image.getpixel((x, y))
        if idx == PURPLE:
            new.px(x, y, WHITE)
            placed += 1

    return new


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
    sp = Sprite(64, 64, fill=SKY)
    compose_sand_strip(sp, 48)
    compose_palm_tree(sp, 16, 48, trunk_h=18, frond_r=8)
    compose_johnny_simple(sp, 32, 48, hat_color=RED, shirt_color=YELLOW)
    compose_star(sp, 56, 8, 3, YELLOW)
    sp.save(out)
    print(f"Self-test sprite saved to {out}")
    print(f"Palette: {len(PALETTE)} colors, {len(PALETTE_NAMES)} names")
