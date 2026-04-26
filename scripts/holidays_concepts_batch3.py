"""
Batch 3 holiday renderers — IDs 20..26 (Memorial through Watermelon).

Each render function takes a single arg: the holiday entry dict from
holidays.yml. It returns a `Sprite` (16-color indexed) at the dimensions
specified in the entry.

Variants:
  v1 — literal: closest reading of the design doc concept
  v2 — minimalist: stripped-down silhouette / icon style
  v3 — busy/scenic: more elements, full island scene
"""

from holidays_art_lib import (
    Sprite, PALETTE,
    TRANSPARENT, WHITE, BLACK, SKIN, TRUNK, GREEN, DGREEN,
    SKY, DEEPBLUE, SAND, RED, YELLOW, ORANGE, PINK, PURPLE, GRAY,
    compose_sand_strip, compose_sky, compose_palm_tree,
    compose_johnny_simple, compose_speech_bubble, compose_outlined_rect,
    compose_star, compose_heart, compose_horizon,
)


# ---------------------------------------------------------------------------
# id 20: Memorial Day  (56×80, palette: navy/red/cream)
# Concept: American flag at half-mast on a driftwood pole; Johnny saluting;
# small white crosses in the sand.
# ---------------------------------------------------------------------------

def memorial_v1(h):
    sp = Sprite(56, 80, fill=SKY)
    compose_sand_strip(sp, 56)
    compose_horizon(sp, 56)
    # Driftwood flagpole — left side, weathered (TRUNK color)
    pole_x = 14
    sp.line(pole_x,     8, pole_x,     56, TRUNK)
    sp.line(pole_x + 1, 8, pole_x + 1, 56, TRUNK)
    # Knob on top
    sp.px(pole_x, 7, YELLOW); sp.px(pole_x + 1, 7, YELLOW)
    # Flag at half-mast — drawn at mid-pole, not the top
    fx, fy = pole_x + 2, 28
    sp.rect(fx,      fy,     fx + 18, fy + 11, WHITE, outline=BLACK)
    # Red stripes
    for i, sy in enumerate(range(fy + 1, fy + 11, 2)):
        sp.line(fx + 1, sy, fx + 17, sy, RED)
    # Blue canton
    sp.rect(fx + 1, fy + 1, fx + 8, fy + 5, DEEPBLUE)
    # Tiny stars on canton
    for sx in range(fx + 2, fx + 8, 2):
        for sy in range(fy + 2, fy + 5, 2):
            sp.px(sx, sy, WHITE)
    # Johnny saluting (right side)
    compose_johnny_simple(sp, 36, 56, hat_color=DEEPBLUE, shirt_color=DEEPBLUE)
    # Salute arm — diagonal line from shoulder up to forehead
    sp.line(40, 49, 42, 46, SKIN)
    sp.px(42, 45, SKIN)
    # Three small white crosses in the sand
    for cx in [6, 26, 48]:
        cy = 66
        sp.line(cx, cy, cx, cy + 5, WHITE)
        sp.line(cx - 1, cy + 2, cx + 1, cy + 2, WHITE)
    return sp


def memorial_v2(h):
    """Minimalist — single big flag silhouette over a horizon."""
    sp = Sprite(56, 80, fill=SKY)
    compose_horizon(sp, 56)
    compose_sand_strip(sp, 56)
    # Large centered flag
    fx, fy = 8, 18
    sp.rect(fx, fy, fx + 38, fy + 24, WHITE, outline=BLACK)
    # Red stripes
    for i in range(7):
        sy = fy + 1 + i * 3
        if sy < fy + 24:
            sp.rect(fx + 1, sy, fx + 37, sy + 1, RED)
    # Blue canton
    sp.rect(fx + 1, fy + 1, fx + 16, fy + 12, DEEPBLUE)
    # Stars on canton (5 dots)
    for cx, cy in [(fx + 4, fy + 4), (fx + 8, fy + 4), (fx + 12, fy + 4),
                   (fx + 6, fy + 8), (fx + 10, fy + 8)]:
        sp.px(cx, cy, WHITE)
    # Pole
    sp.line(fx - 1, fy - 4, fx - 1, 56, TRUNK)
    return sp


def memorial_v3(h):
    """Busy — full memorial scene with flag, Johnny, multiple crosses, palm."""
    sp = Sprite(56, 80, fill=SKY)
    # Sky gradient — pale top to fuller blue at horizon
    for y in range(0, 50):
        sp.line(0, y, 55, y, SKY)
    compose_horizon(sp, 50)
    # Sea band
    sp.rect(0, 50, 55, 55, DEEPBLUE)
    compose_sand_strip(sp, 56)
    # Palm tree on the right
    compose_palm_tree(sp, 46, 56, trunk_h=20, frond_r=8)
    # Flagpole (driftwood)
    pole_x = 12
    sp.line(pole_x,     6, pole_x,     56, TRUNK)
    sp.line(pole_x + 1, 6, pole_x + 1, 56, TRUNK)
    sp.px(pole_x, 5, YELLOW); sp.px(pole_x + 1, 5, YELLOW)
    # Flag half-mast
    fx, fy = pole_x + 2, 24
    sp.rect(fx, fy, fx + 16, fy + 10, WHITE, outline=BLACK)
    for sy in range(fy + 1, fy + 10, 2):
        sp.line(fx + 1, sy, fx + 15, sy, RED)
    sp.rect(fx + 1, fy + 1, fx + 7, fy + 4, DEEPBLUE)
    sp.px(fx + 3, fy + 2, WHITE); sp.px(fx + 5, fy + 2, WHITE)
    sp.px(fx + 4, fy + 3, WHITE)
    # Johnny saluting
    compose_johnny_simple(sp, 26, 64, hat_color=DEEPBLUE, shirt_color=DEEPBLUE)
    sp.line(30, 57, 32, 54, SKIN)
    # Crosses in sand
    for cx in [4, 18, 36, 50]:
        cy = 70
        sp.line(cx, cy, cx, cy + 5, WHITE)
        sp.line(cx - 1, cy + 2, cx + 1, cy + 2, WHITE)
    # Floating poppy specks
    for x, y in [(8, 60), (40, 62), (52, 68)]:
        sp.px(x, y, RED)
    return sp


# ---------------------------------------------------------------------------
# id 21: Father's Day  (80×56, palette: blue/brown/orange)
# Concept: Grilling barbecue on the beach; Johnny in 'World's Best Dad' apron
# flipping a coconut burger.
# ---------------------------------------------------------------------------

def fathersday_v1(h):
    sp = Sprite(80, 56, fill=SKY)
    compose_sand_strip(sp, 38)
    compose_horizon(sp, 38)
    # Palm tree on right
    compose_palm_tree(sp, 68, 38, trunk_h=20, frond_r=8)
    # Grill — brown body on three legs
    gx0, gy0, gx1, gy1 = 30, 26, 50, 36
    sp.rect(gx0, gy0, gx1, gy1, TRUNK, outline=BLACK)
    # Grill grate lines
    for sy in range(gy0 + 2, gy0 + 5, 2):
        sp.line(gx0 + 1, sy, gx1 - 1, sy, BLACK)
    # Legs
    sp.line(gx0 + 2, gy1, gx0 + 2, gy1 + 8, BLACK)
    sp.line(gx1 - 2, gy1, gx1 - 2, gy1 + 8, BLACK)
    # Flames/smoke rising orange
    for x, y in [(34, 22), (38, 20), (42, 22), (46, 19)]:
        sp.px(x, y, ORANGE)
        sp.px(x, y - 1, YELLOW)
    # Coconut burger on top of grill
    sp.ellipse(38, 23, 44, 26, TRUNK, outline=BLACK)
    # Johnny on the left, with apron
    compose_johnny_simple(sp, 12, 38, hat_color=None, shirt_color=ORANGE)
    # Apron (orange overlay across body)
    sp.rect(12, 30, 17, 35, ORANGE)
    # Apron straps
    sp.px(13, 28, ORANGE); sp.px(16, 28, ORANGE)
    # Spatula in his "hand" extended toward grill
    sp.line(18, 32, 28, 28, GRAY)
    sp.rect(27, 26, 30, 28, GRAY, outline=BLACK)
    return sp


def fathersday_v2(h):
    """Minimalist — a single grill icon + tie."""
    sp = Sprite(80, 56, fill=SKY)
    compose_sand_strip(sp, 40)
    # Big centered grill
    sp.ellipse(28, 18, 52, 30, TRUNK, outline=BLACK)
    sp.rect(28, 22, 52, 36, TRUNK, outline=BLACK)
    # Grate lines
    for sy in range(24, 36, 3):
        sp.line(30, sy, 50, sy, BLACK)
    # Legs
    sp.line(32, 36, 30, 46, BLACK)
    sp.line(48, 36, 50, 46, BLACK)
    # Flame coming over top — orange teardrop
    sp.ellipse(36, 8, 44, 18, ORANGE, outline=BLACK)
    sp.ellipse(38, 4, 42, 12, YELLOW)
    # Tie under the grill (necktie shape on the apron)
    sp.line(40, 36, 38, 44, ORANGE)
    sp.line(40, 36, 42, 44, ORANGE)
    sp.rect(37, 44, 43, 50, ORANGE, outline=BLACK)
    return sp


def fathersday_v3(h):
    """Busy — full beach BBQ scene with table, drink, smoke trails."""
    sp = Sprite(80, 56, fill=SKY)
    # Sky gradient
    for y in range(0, 38):
        sp.line(0, y, 79, y, SKY)
    compose_horizon(sp, 38)
    sp.rect(0, 38, 79, 40, DEEPBLUE)
    compose_sand_strip(sp, 40)
    # Two palms
    compose_palm_tree(sp, 8, 40, trunk_h=20, frond_r=8)
    compose_palm_tree(sp, 72, 40, trunk_h=22, frond_r=8)
    # Smoke puffs trailing up
    for x, y in [(36, 8), (40, 4), (44, 10), (48, 6), (52, 12)]:
        sp.ellipse(x, y, x + 3, y + 3, GRAY, outline=BLACK)
    # Grill
    sp.rect(34, 28, 52, 38, TRUNK, outline=BLACK)
    for sy in range(30, 36, 2):
        sp.line(35, sy, 51, sy, BLACK)
    sp.line(36, 38, 36, 46, BLACK)
    sp.line(50, 38, 50, 46, BLACK)
    # Flames/coals
    for x, y in [(38, 24), (42, 22), (46, 24), (40, 20), (44, 18)]:
        sp.px(x, y, ORANGE); sp.px(x, y - 1, YELLOW)
    # Burger on grill
    sp.ellipse(40, 26, 46, 29, TRUNK, outline=BLACK)
    # Side table on the left with a drink
    sp.rect(14, 34, 22, 40, TRUNK, outline=BLACK)
    sp.line(15, 40, 15, 46, TRUNK)
    sp.line(21, 40, 21, 46, TRUNK)
    # Drink
    sp.rect(16, 28, 20, 34, ORANGE, outline=BLACK)
    sp.line(18, 24, 18, 28, BLACK)  # straw
    # Johnny center-left in apron
    compose_johnny_simple(sp, 26, 46, hat_color=None, shirt_color=ORANGE)
    sp.rect(26, 38, 31, 43, ORANGE)
    # Spatula
    sp.line(32, 40, 38, 34, GRAY)
    sp.rect(37, 32, 40, 34, GRAY, outline=BLACK)
    # "DAD" sash hint — tiny heart on apron
    compose_heart(sp, 28, 41, 1, RED)
    return sp


# ---------------------------------------------------------------------------
# id 22: Summer Solstice  (112×96, palette: yellow/blue/orange)
# Concept: Bright noon sun directly overhead; Johnny napping in a hammock
# between palms; thermometer about to burst.
# ---------------------------------------------------------------------------

def summer_v1(h):
    sp = Sprite(112, 96, fill=SKY)
    compose_sand_strip(sp, 70)
    compose_horizon(sp, 70)
    # Big sun centered overhead
    sun_cx, sun_cy = 56, 18
    sp.ellipse(sun_cx - 12, sun_cy - 12, sun_cx + 12, sun_cy + 12,
               YELLOW, outline=ORANGE)
    # Sun rays radiating
    for dx, dy in [(-20, 0), (20, 0), (0, -18), (0, 18),
                   (-14, -14), (14, -14), (-14, 14), (14, 14)]:
        sp.line(sun_cx, sun_cy, sun_cx + dx, sun_cy + dy, YELLOW)
    # Inner highlight
    sp.ellipse(sun_cx - 4, sun_cy - 6, sun_cx, sun_cy - 2, WHITE)
    # Two palms — left and right
    compose_palm_tree(sp, 14, 70, trunk_h=36, frond_r=10)
    compose_palm_tree(sp, 98, 70, trunk_h=36, frond_r=10)
    # Hammock — sagging line + striped cloth
    hy = 56
    sp.line(16, hy - 4, 96, hy + 2, TRUNK)  # top rope
    sp.line(16, hy + 4, 96, hy + 10, TRUNK) # bottom rope
    # Hammock body — orange/yellow stripes (sagging curve approximated)
    for x in range(20, 94):
        # parabolic sag
        t = (x - 56) / 38
        ys = int(hy + 4 + (1 - t * t) * 6)
        c = ORANGE if ((x // 4) % 2 == 0) else YELLOW
        sp.line(x, hy + 1, x, ys, c)
    # Johnny napping in hammock
    sp.rect(48, 56, 64, 60, RED)  # body
    sp.ellipse(46, 52, 52, 58, SKIN, outline=BLACK)  # head
    sp.px(48, 54, BLACK); sp.px(50, 54, BLACK)  # closed eyes (Z's region)
    # ZZZ above
    sp.text(40, 44, "z", BLACK)
    sp.text(44, 40, "Z", BLACK)
    # Thermometer on right post — nearly bursting
    tx = 102
    sp.rect(tx, 50, tx + 4, 80, WHITE, outline=BLACK)
    sp.rect(tx + 1, 52, tx + 3, 78, RED)
    sp.ellipse(tx - 1, 78, tx + 5, 84, RED, outline=BLACK)
    # Burst ticks
    sp.px(tx + 6, 50, RED); sp.px(tx - 2, 50, RED)
    sp.px(tx + 7, 48, RED); sp.px(tx - 3, 48, RED)
    return sp


def summer_v2(h):
    """Minimalist — giant sun on a clean blue sky, single horizon."""
    sp = Sprite(112, 96, fill=SKY)
    compose_horizon(sp, 76)
    compose_sand_strip(sp, 76)
    # Massive sun centered
    sun_cx, sun_cy = 56, 36
    sp.ellipse(sun_cx - 24, sun_cy - 24, sun_cx + 24, sun_cy + 24,
               YELLOW, outline=ORANGE)
    # Inner core
    sp.ellipse(sun_cx - 14, sun_cy - 14, sun_cx + 14, sun_cy + 14, ORANGE)
    sp.ellipse(sun_cx - 8, sun_cy - 8, sun_cx + 8, sun_cy + 8, YELLOW)
    # Long rays
    for dx, dy in [(-40, 0), (40, 0), (0, -34), (0, 34),
                   (-28, -28), (28, -28), (-28, 28), (28, 28),
                   (-36, -16), (36, -16), (-36, 16), (36, 16)]:
        sp.line(sun_cx, sun_cy, sun_cx + dx, sun_cy + dy, YELLOW)
    # Highlight
    sp.ellipse(sun_cx - 6, sun_cy - 10, sun_cx - 2, sun_cy - 6, WHITE)
    return sp


def summer_v3(h):
    """Busy — full midday beach scene with sun, palms, hammock, drinks, thermometer."""
    sp = Sprite(112, 96, fill=SKY)
    # Sky gradient — brighter near horizon
    for y in range(0, 70):
        sp.line(0, y, 111, y, SKY)
    compose_horizon(sp, 70)
    sp.rect(0, 70, 111, 74, DEEPBLUE)
    compose_sand_strip(sp, 74)
    # Sun overhead
    sun_cx, sun_cy = 56, 16
    sp.ellipse(sun_cx - 10, sun_cy - 10, sun_cx + 10, sun_cy + 10,
               YELLOW, outline=ORANGE)
    for dx, dy in [(-16, 0), (16, 0), (0, -14), (0, 14),
                   (-12, -12), (12, -12), (-12, 12), (12, 12)]:
        sp.line(sun_cx, sun_cy, sun_cx + dx, sun_cy + dy, YELLOW)
    # Heat ripples (orange wavy lines)
    for y in [62, 66]:
        for x in range(20, 100, 8):
            sp.line(x, y, x + 3, y - 1, ORANGE)
            sp.line(x + 3, y - 1, x + 6, y, ORANGE)
    # Two palms
    compose_palm_tree(sp, 14, 74, trunk_h=38, frond_r=10)
    compose_palm_tree(sp, 98, 74, trunk_h=38, frond_r=10)
    # Hammock
    hy = 56
    sp.line(16, hy - 4, 96, hy + 2, TRUNK)
    sp.line(16, hy + 4, 96, hy + 10, TRUNK)
    for x in range(20, 94):
        t = (x - 56) / 38
        ys = int(hy + 4 + (1 - t * t) * 6)
        c = ORANGE if ((x // 4) % 2 == 0) else YELLOW
        sp.line(x, hy + 1, x, ys, c)
    # Johnny napping
    sp.rect(46, 56, 66, 60, RED)
    sp.ellipse(44, 52, 52, 58, SKIN, outline=BLACK)
    sp.px(46, 54, BLACK); sp.px(48, 54, BLACK)
    # Sunglasses on Johnny
    sp.rect(46, 53, 50, 55, BLACK)
    # Drink hanging off hammock
    sp.rect(70, 62, 74, 68, ORANGE, outline=BLACK)
    sp.line(72, 58, 72, 62, BLACK)
    # Thermometer right side
    tx = 104
    sp.rect(tx, 40, tx + 4, 78, WHITE, outline=BLACK)
    sp.rect(tx + 1, 42, tx + 3, 76, RED)
    sp.ellipse(tx - 1, 76, tx + 5, 82, RED, outline=BLACK)
    # Bursting at top
    for dx, dy in [(-3, -4), (-1, -6), (2, -6), (5, -4), (7, -2)]:
        sp.px(tx + 2 + dx, 40 + dy, RED)
    # ZZZ floating
    sp.text(36, 44, "z", BLACK)
    sp.text(40, 40, "Z", BLACK)
    sp.text(46, 36, "Z", BLACK)
    # Sand starfish
    for cx in [22, 88]:
        compose_star(sp, cx, 86, 3, ORANGE)
    return sp


# ---------------------------------------------------------------------------
# id 23: Pride Day  (112×64, palette: red/orange/purple)
# Concept: A rainbow flag flying above the palm; Johnny waving a smaller
# rainbow flag of his own.
# ---------------------------------------------------------------------------

_PRIDE_STRIPES = [RED, ORANGE, YELLOW, GREEN, SKY, PURPLE]


def pride_v1(h):
    sp = Sprite(112, 64, fill=SKY)
    compose_sand_strip(sp, 48)
    compose_horizon(sp, 48)
    # Single tall palm with rainbow flag at top
    px = 56
    compose_palm_tree(sp, px, 48, trunk_h=34, frond_r=10)
    # Rainbow flag attached to palm — to the right of trunk top
    fx, fy = px + 2, 8
    fw, fh = 28, 18
    # 6 stripes
    stripe_h = fh // 6
    for i, c in enumerate(_PRIDE_STRIPES):
        sp.rect(fx, fy + i * stripe_h, fx + fw, fy + (i + 1) * stripe_h - 1, c)
    # Flag outline
    sp.rect(fx, fy, fx + fw, fy + fh, TRANSPARENT, outline=BLACK)
    # Flag pole already drawn by palm trunk; add little flutter
    sp.line(fx + fw, fy, fx + fw + 3, fy + 3, BLACK)
    sp.line(fx + fw, fy + fh, fx + fw + 3, fy + fh - 3, BLACK)
    # Johnny on right, waving smaller flag
    compose_johnny_simple(sp, 84, 48, hat_color=PURPLE, shirt_color=PINK)
    # Small flag in his hand
    sp.line(92, 38, 92, 46, TRUNK)
    mini_h = 1
    for i, c in enumerate(_PRIDE_STRIPES):
        sp.rect(93, 38 + i * mini_h, 100, 38 + i * mini_h, c)
    sp.rect(93, 38, 100, 38 + 5, TRANSPARENT, outline=BLACK)
    return sp


def pride_v2(h):
    """Minimalist — one big rainbow flag filling the frame."""
    sp = Sprite(112, 64, fill=SKY)
    # Rainbow stripes filling the whole sprite
    stripe_h = 64 // 6
    for i, c in enumerate(_PRIDE_STRIPES):
        sp.rect(0, i * stripe_h, 111, (i + 1) * stripe_h - 1, c)
    # Border
    sp.rect(0, 0, 111, 63, TRANSPARENT, outline=BLACK)
    # Tiny heart in the center for affection
    compose_heart(sp, 56, 32, 5, WHITE)
    return sp


def pride_v3(h):
    """Busy — parade scene with flag, multiple Johnnys, confetti, bunting."""
    sp = Sprite(112, 64, fill=SKY)
    compose_sand_strip(sp, 48)
    compose_horizon(sp, 48)
    # Bunting strung across the top — each segment a rainbow color
    for i, x in enumerate(range(4, 108, 8)):
        c = _PRIDE_STRIPES[i % 6]
        sp.line(x, 2, x + 3, 8, c)
        sp.line(x + 3, 8, x + 6, 2, c)
    # Palm with flag (left side)
    compose_palm_tree(sp, 16, 48, trunk_h=34, frond_r=10)
    fx, fy, fw, fh = 18, 12, 20, 12
    stripe_h = fh // 6
    for i, c in enumerate(_PRIDE_STRIPES):
        sp.rect(fx, fy + i * stripe_h, fx + fw, fy + (i + 1) * stripe_h - 1, c)
    sp.rect(fx, fy, fx + fw, fy + fh, TRANSPARENT, outline=BLACK)
    # Three Johnnys parading
    for i, x in enumerate([48, 70, 92]):
        hat = _PRIDE_STRIPES[i * 2]
        shirt = _PRIDE_STRIPES[i * 2 + 1]
        compose_johnny_simple(sp, x, 56, hat_color=hat, shirt_color=shirt)
        # Each waving a tiny flag
        sp.line(x + 6, 46, x + 6, 52, TRUNK)
        for j, c in enumerate(_PRIDE_STRIPES):
            sp.px(x + 7, 46 + j, c)
            sp.px(x + 8, 46 + j, c)
    # Confetti rainbow dots
    confetti = [(8, 14, RED), (24, 18, ORANGE), (44, 12, YELLOW),
                (60, 22, GREEN), (78, 14, SKY), (98, 20, PURPLE),
                (32, 30, RED), (84, 32, YELLOW), (104, 28, PURPLE)]
    for x, y, c in confetti:
        sp.px(x, y, c); sp.px(x + 1, y, c)
    # Hearts floating
    compose_heart(sp, 60, 6, 2, PINK)
    compose_heart(sp, 80, 4, 2, RED)
    return sp


# ---------------------------------------------------------------------------
# id 24: Independence Day  (144×80, palette: navy/red/white)
# Concept: Sparklers held by Johnny; fireworks blooming in the sky over the
# ocean; tiny stars-and-stripes.
# ---------------------------------------------------------------------------

def july4th_v1(h):
    sp = Sprite(144, 80, fill=DEEPBLUE)
    # Night sky already set by fill
    compose_sand_strip(sp, 60)
    # Horizon (ocean reflecting deep blue)
    sp.rect(0, 56, 143, 60, DEEPBLUE)
    sp.line(0, 56, 143, 56, BLACK)
    # Two firework bursts in the sky
    def burst(cx, cy, color, r=8):
        for k in range(8):
            import math
            ang = k * 3.14159 / 4
            dx = int(r * (1 if k in (0,) else (0.7 if k in (1,3,5,7) else 1)) * (1 if k in (0,4) else 0))
        # Use radial line strokes
        sp.px(cx, cy, WHITE)
        for dx, dy in [(-r, 0), (r, 0), (0, -r), (0, r),
                       (-r * 3 // 4, -r * 3 // 4), (r * 3 // 4, -r * 3 // 4),
                       (-r * 3 // 4, r * 3 // 4), (r * 3 // 4, r * 3 // 4)]:
            sp.line(cx, cy, cx + dx, cy + dy, color)
        # tip dots
        for dx, dy in [(-r, 0), (r, 0), (0, -r), (0, r)]:
            sp.px(cx + dx, cy + dy, WHITE)
    burst(32, 18, RED, r=10)
    burst(96, 14, WHITE, r=8)
    burst(120, 26, RED, r=6)
    # Tiny stars sprinkled
    for x, y in [(8, 6), (60, 8), (80, 4), (140, 8), (52, 30), (108, 36)]:
        sp.px(x, y, WHITE)
    # Palm silhouette right side
    compose_palm_tree(sp, 130, 60, trunk_h=24, frond_r=8)
    # Johnny holding sparklers, center-left
    compose_johnny_simple(sp, 60, 60, hat_color=RED, shirt_color=WHITE)
    # Sparkler stick (left hand)
    sp.line(58, 50, 50, 42, GRAY)
    sp.line(50, 42, 48, 40, YELLOW)
    sp.px(46, 38, WHITE); sp.px(48, 36, YELLOW); sp.px(44, 40, RED)
    # Sparkler stick (right hand)
    sp.line(66, 50, 74, 42, GRAY)
    sp.line(74, 42, 76, 40, YELLOW)
    sp.px(78, 38, WHITE); sp.px(76, 36, YELLOW); sp.px(80, 40, RED)
    # Tiny stars-and-stripes flag planted in sand
    pole_x = 100
    sp.line(pole_x, 50, pole_x, 70, TRUNK)
    sp.rect(pole_x + 1, 50, pole_x + 14, 58, WHITE, outline=BLACK)
    for sy in range(51, 58, 2):
        sp.line(pole_x + 1, sy, pole_x + 14, sy, RED)
    sp.rect(pole_x + 1, 50, pole_x + 7, 54, DEEPBLUE)
    sp.px(pole_x + 3, 52, WHITE); sp.px(pole_x + 5, 52, WHITE)
    return sp


def july4th_v2(h):
    """Minimalist — one big firework burst on a night-sky field."""
    sp = Sprite(144, 80, fill=DEEPBLUE)
    compose_sand_strip(sp, 70)
    # Single huge firework
    cx, cy = 72, 36
    # Long radial rays, alternating colors
    rays = [(-32, 0, RED), (32, 0, RED), (0, -28, WHITE), (0, 28, WHITE),
            (-24, -24, RED), (24, -24, RED), (-24, 24, WHITE), (24, 24, WHITE),
            (-30, -12, WHITE), (30, -12, WHITE), (-30, 12, RED), (30, 12, RED),
            (-12, -30, RED), (12, -30, RED), (-12, 28, WHITE), (12, 28, WHITE)]
    for dx, dy, c in rays:
        sp.line(cx, cy, cx + dx, cy + dy, c)
    # Central core
    sp.ellipse(cx - 4, cy - 4, cx + 4, cy + 4, YELLOW, outline=WHITE)
    # Tip stars
    for dx, dy in [(-32, 0), (32, 0), (0, -28), (0, 28),
                   (-24, -24), (24, -24), (-24, 24), (24, 24)]:
        sp.px(cx + dx, cy + dy, WHITE)
    return sp


def july4th_v3(h):
    """Busy — multiple fireworks, Johnny, palms, ocean, flag, crowd-feel."""
    sp = Sprite(144, 80, fill=DEEPBLUE)
    # Night gradient — slightly lighter near bottom for ocean glow
    for y in range(54, 60):
        sp.line(0, y, 143, y, DEEPBLUE)
    compose_sand_strip(sp, 60)
    sp.line(0, 55, 143, 55, BLACK)
    # Stars in night sky
    star_pts = [(6, 4), (16, 10), (28, 6), (44, 4), (58, 12), (72, 6),
                (86, 4), (100, 10), (118, 6), (132, 4), (140, 12),
                (10, 24), (50, 28), (90, 26), (130, 28)]
    for x, y in star_pts:
        sp.px(x, y, WHITE)
    # Multiple firework bursts in different colors
    def burst(cx, cy, color, r=8):
        for dx, dy in [(-r, 0), (r, 0), (0, -r), (0, r),
                       (-r * 3 // 4, -r * 3 // 4), (r * 3 // 4, -r * 3 // 4),
                       (-r * 3 // 4, r * 3 // 4), (r * 3 // 4, r * 3 // 4)]:
            sp.line(cx, cy, cx + dx, cy + dy, color)
        for dx, dy in [(-r, 0), (r, 0), (0, -r), (0, r)]:
            sp.px(cx + dx, cy + dy, WHITE)
        sp.px(cx, cy, YELLOW)
    burst(20, 16, RED, r=10)
    burst(50, 8, WHITE, r=6)
    burst(78, 18, RED, r=8)
    burst(108, 10, WHITE, r=8)
    burst(132, 22, RED, r=6)
    # Two palm silhouettes
    compose_palm_tree(sp, 12, 60, trunk_h=24, frond_r=8)
    compose_palm_tree(sp, 134, 60, trunk_h=24, frond_r=8)
    # Johnny center, sparklers
    compose_johnny_simple(sp, 70, 60, hat_color=RED, shirt_color=WHITE)
    sp.line(68, 50, 60, 42, GRAY)
    sp.px(58, 40, YELLOW); sp.px(56, 38, WHITE); sp.px(60, 38, RED)
    sp.line(76, 50, 84, 42, GRAY)
    sp.px(86, 40, YELLOW); sp.px(88, 38, WHITE); sp.px(84, 38, RED)
    # Multiple tiny flags in sand
    for fx0 in [34, 100, 120]:
        sp.line(fx0, 60, fx0, 72, TRUNK)
        sp.rect(fx0 + 1, 60, fx0 + 9, 65, WHITE, outline=BLACK)
        for sy in range(61, 65, 2):
            sp.line(fx0 + 1, sy, fx0 + 9, sy, RED)
        sp.rect(fx0 + 1, 60, fx0 + 4, 62, DEEPBLUE)
    # Sand text scrawl: USA dot trail
    for x, y in [(40, 76), (44, 76), (48, 76), (54, 75), (58, 76), (62, 75)]:
        sp.px(x, y, RED)
    return sp


# ---------------------------------------------------------------------------
# id 25: Moon Landing Day  (128×80, palette: night/gray/white)
# Concept: Tiny astronaut figure with American flag planted in the sand;
# oversized full moon hanging behind the palm.
# ---------------------------------------------------------------------------

def moonland_v1(h):
    sp = Sprite(128, 80, fill=DEEPBLUE)
    compose_sand_strip(sp, 56)
    compose_horizon(sp, 56)
    # Oversized full moon — left of center
    moon_cx, moon_cy = 44, 28
    sp.ellipse(moon_cx - 18, moon_cy - 18, moon_cx + 18, moon_cy + 18,
               WHITE, outline=GRAY)
    # Crater detail
    sp.ellipse(moon_cx - 8, moon_cy - 6, moon_cx - 4, moon_cy - 2, GRAY)
    sp.ellipse(moon_cx + 4, moon_cy + 2, moon_cx + 10, moon_cy + 8, GRAY)
    sp.px(moon_cx - 2, moon_cy + 6, GRAY)
    sp.px(moon_cx + 12, moon_cy - 8, GRAY)
    # Palm in front of moon (right side of moon)
    compose_palm_tree(sp, 64, 56, trunk_h=30, frond_r=10)
    # Stars in night sky
    for x, y in [(8, 6), (88, 4), (104, 10), (120, 6), (110, 22),
                 (96, 16), (16, 18), (76, 8)]:
        sp.px(x, y, WHITE)
    # Astronaut on right — tiny figure with helmet
    ax, ay = 96, 56
    # Body
    sp.rect(ax - 3, ay - 8, ax + 3, ay - 2, WHITE, outline=BLACK)
    # Helmet (round)
    sp.ellipse(ax - 4, ay - 14, ax + 4, ay - 6, WHITE, outline=BLACK)
    # Visor (dark)
    sp.rect(ax - 3, ay - 12, ax + 3, ay - 9, BLACK)
    sp.px(ax + 1, ay - 11, WHITE)  # reflection highlight
    # Backpack
    sp.rect(ax + 3, ay - 7, ax + 5, ay - 3, GRAY, outline=BLACK)
    # Legs
    sp.rect(ax - 3, ay - 1, ax - 1, ay + 2, WHITE)
    sp.rect(ax + 1, ay - 1, ax + 3, ay + 2, WHITE)
    # Flag planted by astronaut
    fp = ax - 12
    sp.line(fp, ay - 14, fp, ay, GRAY)
    sp.rect(fp + 1, ay - 14, fp + 10, ay - 8, WHITE, outline=BLACK)
    for sy in range(ay - 13, ay - 8, 2):
        sp.line(fp + 1, sy, fp + 10, sy, RED)
    sp.rect(fp + 1, ay - 14, fp + 5, ay - 11, DEEPBLUE)
    return sp


def moonland_v2(h):
    """Minimalist — single huge moon with one footprint."""
    sp = Sprite(128, 80, fill=DEEPBLUE)
    compose_sand_strip(sp, 70)
    # Massive moon
    moon_cx, moon_cy = 64, 36
    sp.ellipse(moon_cx - 30, moon_cy - 30, moon_cx + 30, moon_cy + 30,
               WHITE, outline=GRAY)
    # Several craters
    sp.ellipse(moon_cx - 14, moon_cy - 10, moon_cx - 6, moon_cy - 2, GRAY)
    sp.ellipse(moon_cx + 6, moon_cy + 4, moon_cx + 16, moon_cy + 14, GRAY)
    sp.ellipse(moon_cx - 18, moon_cy + 10, moon_cx - 12, moon_cy + 16, GRAY)
    sp.px(moon_cx + 18, moon_cy - 18, GRAY)
    sp.px(moon_cx - 24, moon_cy - 4, GRAY)
    sp.px(moon_cx + 22, moon_cy + 22, GRAY)
    # Footprint in sand
    sp.ellipse(58, 74, 66, 78, BLACK)
    sp.px(60, 73, BLACK); sp.px(62, 72, BLACK); sp.px(64, 73, BLACK)
    return sp


def moonland_v3(h):
    """Busy — full astronaut beach scene with moon, palms, stars, flag, footprints."""
    sp = Sprite(128, 80, fill=DEEPBLUE)
    compose_sand_strip(sp, 56)
    compose_horizon(sp, 56)
    sp.rect(0, 54, 127, 56, BLACK)
    # Stars
    star_pts = [(8, 4), (20, 10), (32, 4), (88, 6), (100, 12), (114, 4),
                (124, 10), (16, 22), (104, 26), (118, 18), (76, 4), (94, 18)]
    for x, y in star_pts:
        sp.px(x, y, WHITE)
    # Moon left
    mx, my = 32, 22
    sp.ellipse(mx - 16, my - 16, mx + 16, my + 16, WHITE, outline=GRAY)
    sp.ellipse(mx - 6, my - 4, mx - 2, my, GRAY)
    sp.ellipse(mx + 2, my + 4, mx + 8, my + 10, GRAY)
    sp.px(mx + 8, my - 10, GRAY)
    # Two palms
    compose_palm_tree(sp, 60, 56, trunk_h=28, frond_r=10)
    compose_palm_tree(sp, 120, 56, trunk_h=24, frond_r=8)
    # Astronaut center-right
    ax, ay = 88, 60
    sp.rect(ax - 3, ay - 10, ax + 3, ay - 4, WHITE, outline=BLACK)
    sp.ellipse(ax - 4, ay - 16, ax + 4, ay - 8, WHITE, outline=BLACK)
    sp.rect(ax - 3, ay - 14, ax + 3, ay - 11, BLACK)
    sp.px(ax + 1, ay - 13, WHITE)
    sp.rect(ax + 3, ay - 9, ax + 5, ay - 5, GRAY, outline=BLACK)
    sp.rect(ax - 3, ay - 3, ax - 1, ay, WHITE)
    sp.rect(ax + 1, ay - 3, ax + 3, ay, WHITE)
    # Flag planted next to astronaut
    fp = ax - 14
    sp.line(fp, ay - 18, fp, ay, GRAY)
    sp.rect(fp + 1, ay - 18, fp + 10, ay - 11, WHITE, outline=BLACK)
    for sy in range(ay - 17, ay - 11, 2):
        sp.line(fp + 1, sy, fp + 10, sy, RED)
    sp.rect(fp + 1, ay - 18, fp + 5, ay - 14, DEEPBLUE)
    sp.px(fp + 2, ay - 16, WHITE); sp.px(fp + 4, ay - 16, WHITE)
    # Footprints in sand trailing left
    for fx in [62, 70, 78]:
        sp.ellipse(fx, 70, fx + 4, 72, BLACK)
        sp.ellipse(fx + 1, 74, fx + 5, 76, BLACK)
    # Lunar lander silhouette in background?  small triangle shape on sand right
    sp.line(108, 64, 112, 58, GRAY)
    sp.line(112, 58, 116, 64, GRAY)
    sp.line(108, 64, 116, 64, GRAY)
    sp.px(112, 60, WHITE)
    return sp


# ---------------------------------------------------------------------------
# id 26: National Watermelon Day  (56×48, palette: green/red/white)
# Concept: Giant watermelon slice prop on the sand, Johnny taking a bite,
# seeds scattered.
# ---------------------------------------------------------------------------

def watermelon_v1(h):
    sp = Sprite(56, 48, fill=SKY)
    compose_sand_strip(sp, 32)
    compose_horizon(sp, 32)
    # Giant watermelon slice — semicircle, dome-down
    # Outer rind (dark green)
    sp.ellipse(4, 16, 40, 44, DGREEN, outline=BLACK)
    # Inner rind (light green)
    sp.ellipse(6, 18, 38, 42, GREEN)
    # Pink/red flesh
    sp.ellipse(8, 20, 36, 40, RED)
    # Mask out bottom half so it's a slice (cut-flat top)
    sp.rect(0, 16, 55, 28, SKY)
    compose_sand_strip(sp, 32)
    # Now redraw the bottom dome
    sp.ellipse(4, 22, 40, 44, DGREEN, outline=BLACK)
    sp.ellipse(6, 24, 38, 42, GREEN)
    sp.ellipse(8, 26, 36, 40, RED)
    # Flat top of the slice
    sp.line(4, 32, 40, 32, BLACK)
    sp.rect(4, 28, 40, 32, RED, outline=BLACK)
    # Seeds on flesh
    for sx, sy in [(14, 30), (20, 34), (26, 30), (32, 34),
                   (16, 38), (24, 40), (30, 38)]:
        sp.px(sx, sy, BLACK)
        sp.px(sx + 1, sy, BLACK)
    # Bite out of the slice (top edge)
    sp.ellipse(18, 26, 26, 32, SKY)
    # Johnny on the right, taking a bite
    compose_johnny_simple(sp, 44, 44, hat_color=GREEN, shirt_color=RED)
    # His head is biting toward the slice — small pink bite chunk near mouth
    sp.px(43, 35, RED); sp.px(42, 35, RED)
    # Scattered seeds in sand
    for x, y in [(6, 46), (44, 46), (28, 47), (50, 45)]:
        sp.px(x, y, BLACK)
    return sp


def watermelon_v2(h):
    """Minimalist — one big watermelon slice icon centered."""
    sp = Sprite(56, 48, fill=SKY)
    compose_sand_strip(sp, 40)
    # Big slice — semicircle pointing up (triangular slice shape)
    cx = 28
    # Rind arc
    sp.ellipse(cx - 22, 6, cx + 22, 40, DGREEN, outline=BLACK)
    sp.ellipse(cx - 20, 8, cx + 20, 38, GREEN)
    sp.ellipse(cx - 17, 11, cx + 17, 35, RED)
    # Cut the top half off to make a slice (flat bottom)
    sp.rect(0, 24, 55, 47, SAND)
    # Now redraw the top half as a dome
    sp.ellipse(cx - 22, 6, cx + 22, 28, DGREEN, outline=BLACK)
    sp.ellipse(cx - 20, 8, cx + 20, 26, GREEN)
    sp.ellipse(cx - 17, 11, cx + 17, 24, RED)
    # Flat bottom edge
    sp.line(cx - 22, 24, cx + 22, 24, BLACK)
    # Seeds
    for sx, sy in [(cx - 10, 16), (cx - 4, 20), (cx + 4, 18),
                   (cx + 10, 14), (cx, 14), (cx - 8, 22), (cx + 8, 22)]:
        sp.px(sx, sy, BLACK); sp.px(sx + 1, sy, BLACK)
    return sp


def watermelon_v3(h):
    """Busy — multiple slices, Johnny eating, palm, sand seed trail."""
    sp = Sprite(56, 48, fill=SKY)
    compose_sand_strip(sp, 30)
    compose_horizon(sp, 30)
    # Palm tree on the left
    compose_palm_tree(sp, 6, 30, trunk_h=14, frond_r=6)
    # Big slice on the sand (left-center)
    cx = 22
    sp.ellipse(cx - 12, 24, cx + 12, 42, DGREEN, outline=BLACK)
    sp.ellipse(cx - 10, 26, cx + 10, 40, GREEN)
    sp.ellipse(cx - 8, 28, cx + 8, 38, RED)
    sp.line(cx - 12, 30, cx + 12, 30, BLACK)
    sp.rect(cx - 12, 24, cx + 12, 30, RED, outline=BLACK)
    # Bite mark
    sp.ellipse(cx - 4, 24, cx + 4, 30, SKY)
    # Seeds inside slice
    for sx, sy in [(cx - 7, 32), (cx - 2, 34), (cx + 4, 33), (cx + 7, 36)]:
        sp.px(sx, sy, BLACK); sp.px(sx + 1, sy, BLACK)
    # Smaller slice on the right
    cx2 = 44
    sp.ellipse(cx2 - 6, 32, cx2 + 6, 42, DGREEN, outline=BLACK)
    sp.ellipse(cx2 - 5, 33, cx2 + 5, 41, GREEN)
    sp.ellipse(cx2 - 4, 34, cx2 + 4, 40, RED)
    sp.line(cx2 - 6, 36, cx2 + 6, 36, BLACK)
    sp.rect(cx2 - 6, 32, cx2 + 6, 36, RED, outline=BLACK)
    sp.px(cx2 - 2, 38, BLACK); sp.px(cx2 + 2, 38, BLACK)
    # Johnny center, eating
    compose_johnny_simple(sp, 32, 44, hat_color=GREEN, shirt_color=RED)
    sp.px(31, 35, RED); sp.px(30, 35, RED)
    # Seed trail across sand
    for x in [10, 16, 38, 50]:
        sp.px(x, 47, BLACK)
        sp.px(x + 1, 46, BLACK)
    # Tiny watermelon-rind crescent floating top
    sp.ellipse(40, 4, 50, 10, GREEN, outline=BLACK)
    sp.ellipse(41, 6, 49, 11, RED)
    return sp


# ---------------------------------------------------------------------------
# Batch 3 RENDERERS dict
# ---------------------------------------------------------------------------

RENDERERS_BATCH3 = {
    20: (memorial_v1, memorial_v2, memorial_v3),
    21: (fathersday_v1, fathersday_v2, fathersday_v3),
    22: (summer_v1, summer_v2, summer_v3),
    23: (pride_v1, pride_v2, pride_v3),
    24: (july4th_v1, july4th_v2, july4th_v3),
    25: (moonland_v1, moonland_v2, moonland_v3),
    26: (watermelon_v1, watermelon_v2, watermelon_v3),
}
