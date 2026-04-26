"""
Batch 4 — holiday concept renderers for ids 27-35.

Holidays covered:
  27 Left-Handers Day            (96 x 64)
  28 Hawaii Statehood Day        (80 x 80)
  29 Labor Day                   (96 x 80)
  30 Talk Like a Pirate Day      (88 x 96)
  31 First Day of Autumn         (112 x 80)
  32 Columbus / Indigenous Day   (96 x 56)
  33 Election Day                (80 x 64)
  34 Veterans Day                (64 x 80)
  35 Thanksgiving                (112 x 72)

Each holiday has 3 variants:
  v1 — literal: closest reading of the design doc concept
  v2 — minimalist: stripped-down silhouette / icon style
  v3 — busy/scenic: more elements, full island scene

Election Day & Columbus Day are intentionally framed neutrally to respect
their dual-meaning observance.
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
# id 27: Left-Handers Day  (96 x 64, palette: blue/yellow/white)
# Concept: Johnny mirror-flipped, writing left-handed in the sand. Backwards
# 'L' shapes drifting in the breeze.
# ---------------------------------------------------------------------------

def lefthand_v1(h):
    sp = Sprite(96, 64, fill=SKY)
    compose_sand_strip(sp, 44)
    # Backwards L shapes drifting in the sky
    for cx, cy in [(14, 8), (38, 14), (66, 6), (84, 18)]:
        # backwards L = horizontal top-bar going left, vertical going down
        sp.line(cx, cy, cx, cy + 5, YELLOW)
        sp.line(cx, cy, cx - 4, cy, YELLOW)
    # Single palm on the right
    compose_palm_tree(sp, 80, 44, trunk_h=22, frond_r=9)
    # Johnny center-left, "mirrored" — left arm extended to write
    compose_johnny_simple(sp, 40, 56, hat_color=YELLOW, shirt_color=SKY)
    # Left arm reaching down/forward to the sand (left side relative to viewer)
    sp.line(40, 50, 34, 54, SKIN)
    sp.px(33, 55, SKIN)
    # The thing he wrote in the sand: a backwards 'L'
    sp.line(26, 58, 26, 61, BLACK)
    sp.line(26, 58, 23, 58, BLACK)
    return sp


def lefthand_v2(h):
    """Minimalist — one giant backwards L on a clean field."""
    sp = Sprite(96, 64, fill=SKY)
    compose_sand_strip(sp, 50)
    # Big backwards L, centered
    cx, cy = 52, 18
    # Vertical bar
    sp.rect(cx, cy, cx + 6, cy + 28, YELLOW, outline=BLACK)
    # Horizontal bar going LEFT (backwards)
    sp.rect(cx - 18, cy, cx, cy + 6, YELLOW, outline=BLACK)
    # Tiny "this side up" arrow above for charm
    sp.line(34, 10, 30, 14, WHITE)
    sp.line(30, 14, 34, 18, WHITE)
    return sp


def lefthand_v3(h):
    """Busy — confetti of backwards Ls, two palms, Johnny writing."""
    sp = Sprite(96, 64, fill=SKY)
    # Sky gradient
    for y in range(0, 16):
        sp.line(0, y, 95, y, SKY)
    for y in range(16, 44):
        sp.line(0, y, 95, y, WHITE if (y % 6) < 1 else SKY)
    compose_sand_strip(sp, 44)
    # Backwards L confetti scattered through sky
    L_pts = [(8, 4), (22, 10), (36, 6), (50, 14), (66, 8),
             (78, 16), (88, 4), (14, 22), (44, 26), (72, 28)]
    for cx, cy in L_pts:
        c = YELLOW if (cx + cy) % 3 == 0 else WHITE
        sp.line(cx, cy, cx, cy + 4, c)
        sp.line(cx, cy, cx - 3, cy, c)
    # Two palms
    compose_palm_tree(sp, 12, 44, trunk_h=18, frond_r=8)
    compose_palm_tree(sp, 84, 44, trunk_h=22, frond_r=9)
    # Johnny — left-handed writer
    compose_johnny_simple(sp, 44, 58, hat_color=YELLOW, shirt_color=SKY)
    # Extended left arm
    sp.line(44, 52, 38, 56, SKIN)
    sp.px(37, 57, SKIN)
    # Sand writing — multiple backwards Ls
    for cx in [28, 60, 70]:
        sp.line(cx, 56, cx, 60, BLACK)
        sp.line(cx, 56, cx - 3, 56, BLACK)
    return sp


# ---------------------------------------------------------------------------
# id 28: Hawaii Statehood Day  (80 x 80, palette: pink/green/yellow)
# Concept: Hibiscus flowers in Johnny's hair; ukulele on palm; surfboard
# stuck in the sand.
# ---------------------------------------------------------------------------

def hawaii_v1(h):
    sp = Sprite(80, 80, fill=SKY)
    compose_sand_strip(sp, 56)
    # Palm tree on left
    compose_palm_tree(sp, 16, 56, trunk_h=30, frond_r=10)
    # Ukulele leaning on the palm trunk (small body)
    sp.ellipse(20, 42, 28, 52, YELLOW, outline=BLACK)
    sp.rect(24, 30, 25, 42, TRUNK)  # neck
    # Strings (3 lines)
    sp.line(24, 32, 24, 50, BLACK)
    sp.line(25, 32, 25, 50, BLACK)
    # Surfboard stuck in sand on right (vertical)
    sp.ellipse(60, 38, 66, 70, PINK, outline=BLACK)
    sp.line(63, 40, 63, 68, RED)  # center stripe
    # Johnny center, with hibiscus in hair
    compose_johnny_simple(sp, 40, 70, hat_color=None, shirt_color=GREEN)
    # Hibiscus over head (pink flower with yellow center)
    sp.px(40, 56, PINK); sp.px(41, 56, PINK); sp.px(42, 56, PINK)
    sp.px(40, 57, PINK); sp.px(42, 57, PINK)
    sp.px(41, 57, YELLOW)  # stamen
    return sp


def hawaii_v2(h):
    """Minimalist — single big hibiscus flower."""
    sp = Sprite(80, 80, fill=SKY)
    compose_sand_strip(sp, 64)
    # Five-petal hibiscus, centered
    cx, cy = 40, 36
    # Petals (each ~10 px ellipse)
    sp.ellipse(cx - 14, cy - 14, cx - 2, cy - 2, PINK, outline=BLACK)
    sp.ellipse(cx + 2, cy - 14, cx + 14, cy - 2, PINK, outline=BLACK)
    sp.ellipse(cx - 16, cy, cx - 4, cy + 12, PINK, outline=BLACK)
    sp.ellipse(cx + 4, cy, cx + 16, cy + 12, PINK, outline=BLACK)
    sp.ellipse(cx - 6, cy + 8, cx + 6, cy + 20, PINK, outline=BLACK)
    # Yellow stamen center
    sp.ellipse(cx - 4, cy - 4, cx + 4, cy + 4, YELLOW, outline=BLACK)
    # Tiny pollen tip
    sp.px(cx, cy - 8, RED)
    return sp


def hawaii_v3(h):
    """Busy — leis, multiple hibiscus, surfboard, ukulele, palm."""
    sp = Sprite(80, 80, fill=SKY)
    # Tropical sunset sky
    for y in range(0, 30):
        sp.line(0, y, 79, y, PINK if y < 12 else SKY)
    compose_sand_strip(sp, 56)
    # Palm tree on left
    compose_palm_tree(sp, 12, 56, trunk_h=32, frond_r=10)
    # Hibiscus flowers scattered
    for cx, cy in [(8, 18), (32, 8), (62, 14), (72, 26)]:
        sp.ellipse(cx - 2, cy - 2, cx + 2, cy + 2, PINK)
        sp.px(cx, cy, YELLOW)
    # Surfboard stuck in sand (right)
    sp.ellipse(60, 30, 66, 64, PINK, outline=BLACK)
    sp.line(63, 32, 63, 62, RED)
    # Ukulele leaning on palm
    sp.ellipse(16, 44, 24, 54, YELLOW, outline=BLACK)
    sp.rect(20, 32, 21, 44, TRUNK)
    # Johnny center with hibiscus + lei
    compose_johnny_simple(sp, 38, 70, hat_color=None, shirt_color=GREEN)
    # Hibiscus in hair
    sp.px(38, 58, PINK); sp.px(39, 58, PINK); sp.px(40, 58, PINK)
    sp.px(39, 59, YELLOW)
    # Lei around neck (alternating pink/yellow dots)
    for i, dx in enumerate([-1, 0, 1, 2, 3, 4, 5]):
        c = PINK if i % 2 == 0 else YELLOW
        sp.px(38 + dx, 63, c)
    # Tiny ocean ripples
    for y in [48, 50]:
        for x in range(40, 76, 4):
            sp.px(x, y, WHITE)
    return sp


# ---------------------------------------------------------------------------
# id 29: Labor Day  (96 x 80, palette: blue/white/red)
# Concept: Hammock between palms with hard hat resting on it; Johnny
# finally relaxed.
# ---------------------------------------------------------------------------

def labor_v1(h):
    sp = Sprite(96, 80, fill=SKY)
    compose_sand_strip(sp, 60)
    # Two palms framing the hammock
    compose_palm_tree(sp, 16, 60, trunk_h=36, frond_r=10)
    compose_palm_tree(sp, 80, 60, trunk_h=36, frond_r=10)
    # Hammock — slung between trunks, sagging in the middle
    # Top edge
    sp.line(18, 30, 30, 38, RED)
    sp.line(30, 38, 50, 44, RED)
    sp.line(50, 44, 66, 40, RED)
    sp.line(66, 40, 78, 30, RED)
    # Bottom edge (parallel sag)
    sp.line(18, 32, 32, 42, WHITE)
    sp.line(32, 42, 50, 48, WHITE)
    sp.line(50, 48, 66, 44, WHITE)
    sp.line(66, 44, 78, 32, WHITE)
    # Fill the hammock body
    for x in range(20, 78):
        # Lerp the sag
        if x < 50:
            yt = 30 + (x - 18) * (44 - 30) // 32
        else:
            yt = 44 - (x - 50) * (44 - 30) // 28
        sp.line(x, yt, x, yt + 2, RED)
    # Hard hat resting on the hammock (yellow dome)
    sp.ellipse(44, 38, 56, 44, YELLOW, outline=BLACK)
    sp.rect(43, 43, 57, 45, YELLOW, outline=BLACK)
    # Johnny standing nearby, relaxed
    compose_johnny_simple(sp, 40, 72, hat_color=None, shirt_color=RED)
    return sp


def labor_v2(h):
    """Minimalist — silhouette of a hard hat on sand."""
    sp = Sprite(96, 80, fill=SKY)
    compose_sand_strip(sp, 56)
    # Big hard hat centered
    cx, cy = 48, 40
    # Dome
    sp.ellipse(cx - 18, cy - 12, cx + 18, cy + 8, YELLOW, outline=BLACK)
    # Brim
    sp.rect(cx - 22, cy + 6, cx + 22, cy + 10, YELLOW, outline=BLACK)
    # Center ridge
    sp.line(cx, cy - 12, cx, cy + 8, BLACK)
    return sp


def labor_v3(h):
    """Busy — full vacation scene: hammock, drink, Johnny, palms, sun."""
    sp = Sprite(96, 80, fill=SKY)
    # Sun in upper right
    sp.ellipse(76, 6, 90, 20, YELLOW, outline=BLACK)
    compose_sand_strip(sp, 60)
    # Two palms
    compose_palm_tree(sp, 12, 60, trunk_h=42, frond_r=10)
    compose_palm_tree(sp, 84, 60, trunk_h=42, frond_r=10)
    # Hammock
    for x in range(16, 80):
        if x < 48:
            yt = 28 + (x - 14) * 18 // 34
        else:
            yt = 46 - (x - 48) * 18 // 32
        sp.line(x, yt, x, yt + 3, RED)
    # Hammock outline
    sp.line(14, 28, 48, 46, BLACK)
    sp.line(48, 46, 82, 28, BLACK)
    sp.line(14, 31, 48, 49, BLACK)
    sp.line(48, 49, 82, 31, BLACK)
    # Johnny lounging IN the hammock (head poking out left)
    sp.ellipse(20, 30, 26, 36, SKIN, outline=BLACK)
    sp.px(22, 33, BLACK); sp.px(24, 33, BLACK)
    # Hard hat at his feet (right side of hammock)
    sp.ellipse(70, 38, 80, 42, YELLOW, outline=BLACK)
    sp.rect(68, 42, 82, 44, YELLOW, outline=BLACK)
    # Discarded tools on sand
    sp.line(30, 70, 38, 72, GRAY)  # wrench
    sp.px(38, 71, GRAY); sp.px(39, 71, GRAY)
    # Tropical drink with umbrella
    sp.rect(50, 64, 56, 72, RED, outline=BLACK)
    sp.line(53, 56, 53, 64, TRUNK)  # umbrella stick
    sp.line(48, 58, 58, 58, RED)    # umbrella top
    sp.line(50, 56, 56, 56, RED)
    return sp


# ---------------------------------------------------------------------------
# id 30: Talk Like a Pirate Day  (88 x 96, palette: navy/gold/red)
# Concept: Johnny in tricorn hat with eye patch & parrot on shoulder; X
# marks the spot map in the sand.
# ---------------------------------------------------------------------------

def pirate_v1(h):
    sp = Sprite(88, 96, fill=DEEPBLUE)
    compose_sand_strip(sp, 64)
    # Palm
    compose_palm_tree(sp, 72, 64, trunk_h=36, frond_r=10)
    # Treasure map laid out on the sand
    sp.rect(8, 70, 40, 88, YELLOW, outline=BLACK)
    # X marks the spot
    sp.line(20, 76, 28, 84, RED)
    sp.line(28, 76, 20, 84, RED)
    # Dotted path
    for x in [10, 14, 17]:
        sp.px(x, 80, BLACK)
    # Johnny on the right with full pirate getup
    jx = 48
    compose_johnny_simple(sp, jx, 64, hat_color=None, shirt_color=RED)
    # Tricorn hat — wide brim with three peaks
    sp.rect(jx - 2, 50, jx + 7, 51, BLACK)
    sp.line(jx - 2, 50, jx + 1, 47, BLACK)
    sp.line(jx + 1, 47, jx + 4, 50, BLACK)
    sp.line(jx + 4, 47, jx + 7, 50, BLACK)
    sp.px(jx + 1, 47, BLACK); sp.px(jx + 4, 47, BLACK)
    # Eye patch
    sp.px(jx + 2, 54, BLACK)
    sp.px(jx + 3, 54, BLACK)
    sp.line(jx, 54, jx + 5, 54, BLACK)
    # Parrot on shoulder (red body, yellow beak)
    sp.rect(jx + 5, 56, jx + 8, 60, RED)
    sp.px(jx + 8, 58, YELLOW)  # beak
    sp.px(jx + 6, 57, BLACK)   # eye
    return sp


def pirate_v2(h):
    """Minimalist — skull and crossed bones (jolly roger)."""
    sp = Sprite(88, 96, fill=DEEPBLUE)
    compose_sand_strip(sp, 76)
    # Crossed bones (drawn first, behind skull)
    cx, cy = 44, 48
    sp.line(cx - 22, cy + 12, cx + 22, cy - 12, WHITE)
    sp.line(cx - 22, cy + 13, cx + 22, cy - 11, WHITE)
    sp.line(cx - 22, cy - 12, cx + 22, cy + 12, WHITE)
    sp.line(cx - 22, cy - 11, cx + 22, cy + 13, WHITE)
    # Bone knobs at ends
    for ex, ey in [(cx - 22, cy - 12), (cx - 22, cy + 12),
                   (cx + 22, cy - 12), (cx + 22, cy + 12)]:
        sp.ellipse(ex - 2, ey - 2, ex + 2, ey + 2, WHITE, outline=BLACK)
    # Skull
    sp.ellipse(cx - 14, cy - 16, cx + 14, cy + 12, WHITE, outline=BLACK)
    # Eye sockets
    sp.ellipse(cx - 9, cy - 8, cx - 3, cy - 2, BLACK)
    sp.ellipse(cx + 3, cy - 8, cx + 9, cy - 2, BLACK)
    # Nose
    sp.px(cx, cy + 2, BLACK)
    sp.px(cx - 1, cy + 3, BLACK)
    sp.px(cx + 1, cy + 3, BLACK)
    # Teeth
    sp.line(cx - 6, cy + 8, cx + 6, cy + 8, BLACK)
    for tx in [cx - 4, cx, cx + 4]:
        sp.line(tx, cy + 6, tx, cy + 10, BLACK)
    return sp


def pirate_v3(h):
    """Busy — pirate ship offshore, Johnny in full pirate, treasure chest."""
    sp = Sprite(88, 96, fill=DEEPBLUE)
    # Sky/water gradient
    for y in range(0, 40):
        sp.line(0, y, 87, y, DEEPBLUE)
    for y in range(40, 64):
        sp.line(0, y, 87, y, SKY)
    compose_sand_strip(sp, 64)
    # Pirate ship silhouette on the horizon
    sp.rect(54, 36, 78, 40, BLACK)  # hull
    sp.line(60, 36, 60, 22, BLACK)  # mast 1
    sp.line(70, 36, 70, 22, BLACK)  # mast 2
    sp.rect(57, 24, 64, 34, WHITE)  # sail 1
    sp.rect(67, 24, 74, 34, WHITE)  # sail 2
    # Skull on biggest sail
    sp.px(60, 28, BLACK); sp.px(61, 28, BLACK)
    # Flag
    sp.rect(70, 22, 75, 24, RED)
    # Treasure chest on sand
    sp.rect(8, 76, 28, 88, TRUNK, outline=BLACK)
    sp.rect(8, 74, 28, 78, TRUNK, outline=BLACK)
    sp.line(18, 74, 18, 88, BLACK)
    # Gold spilling out
    for cx, cy in [(12, 72), (16, 70), (22, 71), (26, 73)]:
        sp.px(cx, cy, YELLOW)
        sp.px(cx + 1, cy, YELLOW)
    # Map next to chest
    sp.rect(32, 80, 50, 90, YELLOW, outline=BLACK)
    sp.line(38, 83, 44, 89, RED)
    sp.line(44, 83, 38, 89, RED)
    # Palm
    compose_palm_tree(sp, 80, 64, trunk_h=30, frond_r=9)
    # Johnny
    jx = 56
    compose_johnny_simple(sp, jx, 80, hat_color=None, shirt_color=RED)
    # Tricorn hat
    sp.rect(jx - 2, 66, jx + 7, 67, BLACK)
    sp.line(jx - 2, 66, jx + 1, 63, BLACK)
    sp.line(jx + 1, 63, jx + 4, 66, BLACK)
    sp.line(jx + 4, 63, jx + 7, 66, BLACK)
    # Eye patch
    sp.line(jx, 70, jx + 5, 70, BLACK)
    sp.px(jx + 2, 70, BLACK)
    # Parrot
    sp.rect(jx + 5, 72, jx + 8, 76, RED)
    sp.px(jx + 8, 74, YELLOW)
    return sp


# ---------------------------------------------------------------------------
# id 31: First Day of Autumn  (112 x 80, palette: brown/dark-red/gold)
# Concept: Palm fronds turning gold; pumpkins replacing coconuts; Johnny
# in tiny scarf.
# ---------------------------------------------------------------------------

def autumn_v1(h):
    sp = Sprite(112, 80, fill=ORANGE)
    # Soft autumn sky — orange to peach
    for y in range(0, 40):
        sp.line(0, y, 111, y, ORANGE if y < 20 else YELLOW)
    compose_sand_strip(sp, 56)
    # Palm trunk
    palm_x = 32
    base_y = 56
    sp.line(palm_x, base_y, palm_x, base_y - 28, TRUNK)
    sp.line(palm_x + 1, base_y - 1, palm_x + 1, base_y - 28, TRUNK)
    # Gold/orange fronds (replacing the usual green)
    top_y = base_y - 28
    for dx, dy, c in [(-9, -2, YELLOW), (-5, -8, ORANGE), (0, -10, YELLOW),
                       (5, -8, ORANGE), (9, -2, YELLOW),
                       (-5, 4, RED), (5, 4, RED)]:
        sp.line(palm_x, top_y, palm_x + dx, top_y + dy, c)
    # Pumpkins hanging like coconuts
    for cx, cy in [(palm_x - 4, top_y + 4), (palm_x + 4, top_y + 6)]:
        sp.ellipse(cx - 2, cy - 2, cx + 2, cy + 2, ORANGE, outline=BLACK)
        sp.px(cx, cy - 3, GREEN)  # stem
    # Johnny center-right with scarf
    jx = 70
    compose_johnny_simple(sp, jx, 70, hat_color=None, shirt_color=RED)
    # Scarf (red around neck, with one trailing end)
    sp.rect(jx - 1, 62, jx + 6, 64, RED, outline=BLACK)
    sp.rect(jx + 5, 64, jx + 7, 68, RED)
    # Falling leaves drifting
    for cx, cy in [(50, 14), (84, 22), (96, 8), (60, 30)]:
        sp.px(cx, cy, RED)
        sp.px(cx + 1, cy, ORANGE)
        sp.px(cx, cy + 1, YELLOW)
    return sp


def autumn_v2(h):
    """Minimalist — single big maple-style leaf."""
    sp = Sprite(112, 80, fill=YELLOW)
    compose_sand_strip(sp, 64)
    # Stylized leaf, centered
    cx, cy = 56, 36
    # Leaf body — diamond-ish
    sp.ellipse(cx - 18, cy - 14, cx + 18, cy + 14, ORANGE, outline=BLACK)
    # Five points hinted by darker spikes
    for dx, dy in [(0, -14), (-14, -8), (14, -8), (-12, 12), (12, 12)]:
        sp.line(cx, cy, cx + dx, cy + dy, RED)
    # Center vein
    sp.line(cx, cy - 14, cx, cy + 14, BLACK)
    # Stem
    sp.line(cx, cy + 14, cx, cy + 22, TRUNK)
    return sp


def autumn_v3(h):
    """Busy — full autumn scene: pumpkins, leaves, scarecrow palm, Johnny."""
    sp = Sprite(112, 80, fill=ORANGE)
    for y in range(0, 30):
        sp.line(0, y, 111, y, ORANGE if y < 15 else YELLOW)
    compose_sand_strip(sp, 56)
    # Two palms with autumn fronds
    for px in [16, 96]:
        sp.line(px, 56, px, 28, TRUNK)
        sp.line(px + 1, 55, px + 1, 28, TRUNK)
        for dx, dy, c in [(-8, -2, YELLOW), (-4, -7, RED), (0, -9, ORANGE),
                           (4, -7, RED), (8, -2, YELLOW)]:
            sp.line(px, 28, px + dx, 28 + dy, c)
    # Pumpkins on the sand (a small patch)
    for cx, cy, r in [(40, 64, 4), (50, 66, 3), (66, 65, 4), (76, 67, 3)]:
        sp.ellipse(cx - r, cy - r, cx + r, cy + r, ORANGE, outline=BLACK)
        sp.line(cx - r, cy, cx + r, cy, RED)  # ridges
        sp.px(cx, cy - r - 1, GREEN)  # stem
    # Falling leaves in the air
    for cx, cy in [(20, 6), (32, 14), (54, 8), (72, 18), (86, 6), (102, 16)]:
        c = [RED, ORANGE, YELLOW][(cx + cy) % 3]
        sp.px(cx, cy, c); sp.px(cx + 1, cy, c)
        sp.px(cx, cy + 1, c)
    # Johnny center with scarf
    jx = 56
    compose_johnny_simple(sp, jx, 76, hat_color=None, shirt_color=RED)
    sp.rect(jx - 1, 68, jx + 6, 70, RED, outline=BLACK)
    sp.rect(jx + 5, 70, jx + 7, 73, RED)
    # Cup of cocoa in his hand
    sp.rect(jx - 4, 72, jx - 1, 75, WHITE, outline=BLACK)
    sp.px(jx - 3, 70, WHITE)  # steam
    sp.px(jx - 2, 69, WHITE)
    return sp


# ---------------------------------------------------------------------------
# id 32: Columbus Day / Indigenous Peoples' Day  (96 x 56, palette: navy/red/cream)
# Concept: Small ship silhouette on horizon; Johnny waving from shore.
# Neutral framing — the ships AND the shore both shown.
# ---------------------------------------------------------------------------

def columbus_v1(h):
    sp = Sprite(96, 56, fill=DEEPBLUE)
    # Sky
    sp.rect(0, 0, 95, 28, SKY)
    # Water
    sp.rect(0, 28, 95, 44, DEEPBLUE)
    compose_horizon(sp, 28)
    # Sand
    compose_sand_strip(sp, 44)
    # Ship silhouette on horizon (neutral — small, distant)
    sp.rect(54, 24, 70, 28, BLACK)  # hull
    sp.line(58, 24, 58, 14, BLACK)  # mast 1
    sp.line(64, 24, 64, 14, BLACK)  # mast 2
    sp.rect(56, 16, 60, 24, WHITE)  # sail 1
    sp.rect(62, 16, 66, 24, WHITE)  # sail 2
    # Johnny on shore, waving
    jx = 18
    compose_johnny_simple(sp, jx, 50, hat_color=None, shirt_color=RED)
    # Raised arm waving
    sp.line(jx + 5, 44, jx + 9, 40, SKIN)
    sp.px(jx + 9, 39, SKIN)
    # Small palm
    compose_palm_tree(sp, 32, 44, trunk_h=14, frond_r=6)
    return sp


def columbus_v2(h):
    """Minimalist — sail and shoreline horizon line, no people."""
    sp = Sprite(96, 56, fill=SKY)
    compose_horizon(sp, 32)
    sp.rect(0, 32, 95, 44, DEEPBLUE)
    compose_sand_strip(sp, 44)
    # Single sail centered on horizon
    cx = 48
    sp.rect(cx - 1, 16, cx + 1, 32, TRUNK)  # mast/hull
    # Triangle sail
    sp.line(cx + 1, 18, cx + 12, 30, WHITE)
    sp.line(cx + 12, 30, cx + 1, 30, WHITE)
    sp.line(cx + 1, 18, cx + 1, 30, WHITE)
    # Fill the sail
    for y in range(19, 30):
        sp.line(cx + 2, y, cx + 1 + (y - 18), y, WHITE)
    return sp


def columbus_v3(h):
    """Busy — three ships on horizon, beach with footprints, palms, Johnny."""
    sp = Sprite(96, 56, fill=SKY)
    sp.rect(0, 0, 95, 26, SKY)
    sp.rect(0, 26, 95, 42, DEEPBLUE)
    compose_horizon(sp, 26)
    compose_sand_strip(sp, 42)
    # Three ships on horizon
    for sx in [20, 48, 72]:
        sp.rect(sx, 22, sx + 10, 26, BLACK)
        sp.line(sx + 3, 22, sx + 3, 14, BLACK)
        sp.line(sx + 7, 22, sx + 7, 14, BLACK)
        sp.rect(sx + 1, 16, sx + 5, 22, WHITE)
        sp.rect(sx + 5, 16, sx + 9, 22, WHITE)
    # Footprints on sand
    for x in [10, 16, 22, 28]:
        sp.px(x, 50, BLACK)
        sp.px(x, 52, BLACK)
    # Palms
    compose_palm_tree(sp, 8, 42, trunk_h=12, frond_r=5)
    compose_palm_tree(sp, 88, 42, trunk_h=12, frond_r=5)
    # Johnny waving on shore
    jx = 40
    compose_johnny_simple(sp, jx, 50, hat_color=None, shirt_color=RED)
    sp.line(jx + 5, 44, jx + 9, 40, SKIN)
    sp.px(jx + 9, 39, SKIN)
    # Small clouds
    for cx, cy in [(14, 6), (60, 4), (84, 8)]:
        sp.ellipse(cx, cy, cx + 6, cy + 3, WHITE)
    return sp


# ---------------------------------------------------------------------------
# id 33: Election Day  (80 x 64, palette: navy/red/white)
# Concept: 'I VOTED' sticker on Johnny's shirt; ballot box from coconut
# crate; star-shaped 'vote here' sign. Strictly non-partisan.
# ---------------------------------------------------------------------------

def election_v1(h):
    sp = Sprite(80, 64, fill=SKY)
    compose_sand_strip(sp, 44)
    # Ballot box (coconut crate) on the left
    sp.rect(8, 32, 28, 50, TRUNK, outline=BLACK)
    # Slot in top
    sp.line(12, 32, 24, 32, BLACK)
    sp.rect(13, 30, 23, 32, BLACK)
    # Wood grain stripes
    sp.line(8, 38, 28, 38, BLACK)
    sp.line(8, 44, 28, 44, BLACK)
    # Star "vote here" sign on a stick (right side)
    sp.line(58, 30, 58, 56, TRUNK)  # post
    compose_star(sp, 64, 24, 6, YELLOW)
    sp.ellipse(56, 18, 72, 30, WHITE, outline=BLACK)
    compose_star(sp, 64, 24, 5, RED)
    # Johnny center
    jx = 36
    compose_johnny_simple(sp, jx, 56, hat_color=None, shirt_color=WHITE)
    # "I VOTED" sticker — small red oval on shirt
    sp.ellipse(jx + 1, 50, jx + 5, 52, RED, outline=BLACK)
    sp.px(jx + 3, 51, WHITE)
    return sp


def election_v2(h):
    """Minimalist — single star inside a circle (a 'vote' icon)."""
    sp = Sprite(80, 64, fill=WHITE)
    compose_sand_strip(sp, 56)
    # Big circle
    cx, cy = 40, 28
    sp.ellipse(cx - 18, cy - 18, cx + 18, cy + 18, RED, outline=BLACK)
    sp.ellipse(cx - 14, cy - 14, cx + 14, cy + 14, WHITE, outline=BLACK)
    # Big star inside
    compose_star(sp, cx, cy, 10, RED)
    # Reinforce star with second pass
    sp.line(cx - 8, cy - 4, cx + 8, cy - 4, RED)
    sp.line(cx - 6, cy + 4, cx + 6, cy + 4, RED)
    return sp


def election_v3(h):
    """Busy — ballot box, sign, multiple Johnnys queueing, sticker, palm."""
    sp = Sprite(80, 64, fill=SKY)
    compose_sand_strip(sp, 44)
    # Palm on the left
    compose_palm_tree(sp, 8, 44, trunk_h=20, frond_r=7)
    # Ballot box center
    sp.rect(28, 30, 48, 50, TRUNK, outline=BLACK)
    sp.rect(33, 28, 43, 30, BLACK)  # slot
    sp.line(28, 36, 48, 36, BLACK)
    sp.line(28, 42, 48, 42, BLACK)
    # A tiny ballot being inserted
    sp.rect(35, 24, 41, 30, WHITE, outline=BLACK)
    # Star "vote here" sign on a post (top right)
    sp.line(64, 8, 64, 30, TRUNK)
    sp.rect(58, 4, 76, 14, RED, outline=BLACK)
    compose_star(sp, 67, 9, 3, WHITE)
    sp.px(72, 9, WHITE)
    sp.px(60, 9, WHITE)
    # Two Johnnys queued up to the box
    compose_johnny_simple(sp, 14, 56, hat_color=None, shirt_color=WHITE)
    sp.ellipse(15, 50, 19, 52, RED, outline=BLACK)  # sticker on first
    compose_johnny_simple(sp, 56, 56, hat_color=None, shirt_color=WHITE)
    sp.ellipse(57, 50, 61, 52, RED, outline=BLACK)
    return sp


# ---------------------------------------------------------------------------
# id 34: Veterans Day  (64 x 80, palette: navy/red/white)
# Concept: Johnny salutes; American flag on driftwood pole; small poppy
# at his feet.
# ---------------------------------------------------------------------------

def veterans_v1(h):
    sp = Sprite(64, 80, fill=SKY)
    compose_sand_strip(sp, 60)
    # Driftwood flagpole on left
    pole_x = 18
    sp.line(pole_x, 8, pole_x, 60, TRUNK)
    sp.line(pole_x + 1, 8, pole_x + 1, 60, TRUNK)
    # Flag (simplified — red & white stripes, blue canton)
    fx0, fy0 = pole_x + 2, 10
    fx1, fy1 = fx0 + 18, fy0 + 12
    sp.rect(fx0, fy0, fx1, fy1, WHITE, outline=BLACK)
    # Stripes (alternating red/white horizontal bands)
    for i, y in enumerate(range(fy0, fy1, 2)):
        if i % 2 == 0:
            sp.line(fx0, y, fx1, y, RED)
            sp.line(fx0, y + 1, fx1, y + 1, RED)
    # Canton (blue square upper-left with small white dots for stars)
    sp.rect(fx0, fy0, fx0 + 8, fy0 + 6, DEEPBLUE)
    for dx, dy in [(2, 1), (5, 1), (3, 3), (6, 3), (2, 5), (5, 5)]:
        sp.px(fx0 + dx, fy0 + dy, WHITE)
    # Johnny saluting (right side)
    jx = 38
    compose_johnny_simple(sp, jx, 70, hat_color=None, shirt_color=DEEPBLUE)
    # Saluting arm — bent up to forehead
    sp.line(jx + 4, 64, jx + 6, 60, SKIN)
    sp.px(jx + 6, 60, SKIN)
    # Poppy at his feet
    sp.ellipse(jx - 2, 73, jx + 1, 76, RED)
    sp.px(jx, 74, BLACK)
    return sp


def veterans_v2(h):
    """Minimalist — single big poppy flower."""
    sp = Sprite(64, 80, fill=WHITE)
    compose_sand_strip(sp, 64)
    # Centered poppy
    cx, cy = 32, 36
    # Four red petals
    sp.ellipse(cx - 14, cy - 14, cx, cy, RED, outline=BLACK)
    sp.ellipse(cx, cy - 14, cx + 14, cy, RED, outline=BLACK)
    sp.ellipse(cx - 14, cy, cx, cy + 14, RED, outline=BLACK)
    sp.ellipse(cx, cy, cx + 14, cy + 14, RED, outline=BLACK)
    # Black center
    sp.ellipse(cx - 4, cy - 4, cx + 4, cy + 4, BLACK)
    # Stem
    sp.line(cx, cy + 14, cx, cy + 28, GREEN)
    return sp


def veterans_v3(h):
    """Busy — flag, saluting Johnny, poppies, sunset."""
    sp = Sprite(64, 80, fill=ORANGE)
    # Sunset
    for y in range(0, 30):
        sp.line(0, y, 63, y, ORANGE if y < 12 else YELLOW)
    for y in range(30, 56):
        sp.line(0, y, 63, y, SKY)
    compose_sand_strip(sp, 56)
    # Flagpole + flag (centered)
    pole_x = 30
    sp.line(pole_x, 6, pole_x, 56, TRUNK)
    sp.line(pole_x + 1, 6, pole_x + 1, 56, TRUNK)
    fx0, fy0 = pole_x + 2, 8
    fx1, fy1 = fx0 + 20, fy0 + 14
    sp.rect(fx0, fy0, fx1, fy1, WHITE, outline=BLACK)
    for i, y in enumerate(range(fy0, fy1, 2)):
        if i % 2 == 0:
            sp.line(fx0, y, fx1, y, RED)
            sp.line(fx0, y + 1, fx1, y + 1, RED)
    sp.rect(fx0, fy0, fx0 + 9, fy0 + 7, DEEPBLUE)
    for dx, dy in [(2, 1), (5, 1), (7, 1), (3, 3), (6, 3), (2, 5), (5, 5), (7, 5)]:
        sp.px(fx0 + dx, fy0 + dy, WHITE)
    # Johnny saluting on the left
    jx = 6
    compose_johnny_simple(sp, jx, 70, hat_color=None, shirt_color=DEEPBLUE)
    sp.line(jx + 4, 64, jx + 6, 60, SKIN)
    sp.px(jx + 6, 60, SKIN)
    # Field of poppies on the sand
    for cx, cy in [(14, 72), (24, 76), (40, 74), (50, 76), (58, 72)]:
        sp.ellipse(cx - 1, cy - 1, cx + 1, cy + 1, RED)
        sp.px(cx, cy, BLACK)
    return sp


# ---------------------------------------------------------------------------
# id 35: Thanksgiving  (112 x 72, palette: brown/dark-red/wheat)
# Concept: Coconut-roast turkey on driftwood platter; cornucopia of
# tropical fruits; Johnny in pilgrim hat.
# ---------------------------------------------------------------------------

def thanksgive_v1(h):
    sp = Sprite(112, 72, fill=ORANGE)
    # Warm sky
    for y in range(0, 32):
        sp.line(0, y, 111, y, ORANGE if y < 16 else YELLOW)
    compose_sand_strip(sp, 48)
    # Driftwood platter (long brown plank)
    sp.rect(28, 52, 78, 60, TRUNK, outline=BLACK)
    # Coconut-roast "turkey" — round brown body
    sp.ellipse(40, 38, 64, 56, TRUNK, outline=BLACK)
    # Tail feathers (fan of colored arcs)
    for i, c in enumerate([RED, ORANGE, YELLOW, ORANGE, RED]):
        sp.line(64, 47, 76 + i, 38 + i * 3, c)
    # Drumstick handles
    sp.line(40, 50, 36, 54, TRUNK)
    sp.line(64, 50, 68, 54, TRUNK)
    # Johnny right side with pilgrim hat
    jx = 86
    compose_johnny_simple(sp, jx, 64, hat_color=None, shirt_color=RED)
    # Pilgrim hat — tall cylinder with brim and buckle
    sp.rect(jx - 1, 48, jx + 6, 50, BLACK)  # brim
    sp.rect(jx, 42, jx + 5, 48, BLACK)      # crown
    sp.rect(jx + 1, 45, jx + 4, 47, YELLOW) # buckle
    return sp


def thanksgive_v2(h):
    """Minimalist — single big pumpkin/cornucopia silhouette."""
    sp = Sprite(112, 72, fill=YELLOW)
    compose_sand_strip(sp, 56)
    # Big stylized cornucopia (curved horn shape)
    cx, cy = 56, 36
    # Horn body — series of stacked ellipses tapering
    for i, r in enumerate([20, 18, 15, 12, 9, 6]):
        x = cx - 14 + i * 5
        sp.ellipse(x - r // 2, cy - r // 2 - i, x + r // 2, cy + r // 2 - i,
                   TRUNK, outline=BLACK)
    # Fruits spilling from the wide end (left side)
    sp.ellipse(cx - 28, cy - 4, cx - 22, cy + 2, RED, outline=BLACK)  # apple
    sp.ellipse(cx - 26, cy + 4, cx - 20, cy + 10, ORANGE, outline=BLACK)  # orange
    sp.ellipse(cx - 22, cy - 10, cx - 16, cy - 4, YELLOW, outline=BLACK)  # banana
    sp.ellipse(cx - 18, cy + 2, cx - 12, cy + 8, GREEN, outline=BLACK)  # leaf
    return sp


def thanksgive_v3(h):
    """Busy — full feast: turkey, cornucopia, pumpkins, Johnny, palm."""
    sp = Sprite(112, 72, fill=ORANGE)
    for y in range(0, 28):
        sp.line(0, y, 111, y, ORANGE if y < 14 else YELLOW)
    compose_sand_strip(sp, 44)
    # Palm on the left
    compose_palm_tree(sp, 8, 44, trunk_h=22, frond_r=8)
    # Long table (driftwood)
    sp.rect(20, 50, 100, 56, TRUNK, outline=BLACK)
    # Turkey center-left on table
    sp.ellipse(28, 38, 48, 50, TRUNK, outline=BLACK)
    for i, c in enumerate([RED, ORANGE, YELLOW, ORANGE, RED]):
        sp.line(48, 42, 56 + i, 36 + i * 2, c)
    # Cornucopia center-right
    for i, r in enumerate([14, 11, 8, 6]):
        x = 60 + i * 3
        sp.ellipse(x, 42 - i, x + r, 48 - i, TRUNK, outline=BLACK)
    # Fruits spilling
    sp.ellipse(56, 44, 60, 48, RED)
    sp.ellipse(60, 48, 64, 52, ORANGE)
    sp.ellipse(54, 48, 58, 52, YELLOW)
    # Pumpkins on the sand
    for cx, cy, r in [(86, 60, 4), (96, 62, 3)]:
        sp.ellipse(cx - r, cy - r, cx + r, cy + r, ORANGE, outline=BLACK)
        sp.line(cx - r, cy, cx + r, cy, RED)
        sp.px(cx, cy - r - 1, GREEN)
    # Johnny on far right with pilgrim hat
    jx = 100
    compose_johnny_simple(sp, jx, 68, hat_color=None, shirt_color=RED)
    sp.rect(jx - 1, 52, jx + 6, 54, BLACK)
    sp.rect(jx, 46, jx + 5, 52, BLACK)
    sp.rect(jx + 1, 49, jx + 4, 51, YELLOW)
    # A few autumn leaves drifting
    for cx, cy in [(30, 8), (60, 14), (88, 6)]:
        sp.px(cx, cy, RED)
        sp.px(cx + 1, cy, ORANGE)
    return sp


# ---------------------------------------------------------------------------
# v4 PLAYFUL variants — exaggerated/comedic. Tilted, oversized, sweat drops,
# motion lines, exclamation marks. Same dimensions as v1 of each holiday.
# ---------------------------------------------------------------------------

def lefthand_v4(h):
    """PLAYFUL — Johnny vigorously writing left-handed, ink-splat everywhere."""
    sp = Sprite(96, 64, fill=SKY)
    compose_sand_strip(sp, 44)
    # Backwards-L symbols flying around
    for x, y in [(8, 8), (24, 4), (44, 12), (66, 6), (84, 14)]:
        sp.line(x, y, x, y + 6, BLACK)
        sp.line(x, y + 6, x - 4, y + 6, BLACK)
    # Big triumphant Johnny center
    compose_johnny_simple(sp, 44, 56, hat_color=YELLOW, shirt_color=BLACK)
    # Pencil/quill in left hand (left side)
    sp.line(38, 50, 30, 44, TRUNK)
    sp.px(29, 43, BLACK)
    # Ink splats around
    for cx, cy in [(20, 38), (60, 32), (78, 50), (12, 50)]:
        sp.ellipse(cx, cy, cx + 3, cy + 2, BLACK)
        sp.px(cx + 5, cy, BLACK); sp.px(cx - 2, cy + 4, BLACK)
    # Motion lines
    for x in [26, 32, 56, 62]:
        sp.line(x, 30, x + 4, 26, GRAY)
    # Exclamation
    sp.line(72, 8, 72, 18, RED); sp.px(72, 22, RED)
    return sp


def hawaii_v4(h):
    """PLAYFUL — Johnny doing a goofy hula dance, surrounded by flying flowers."""
    sp = Sprite(80, 80, fill=PINK)
    compose_sand_strip(sp, 56)
    compose_palm_tree(sp, 16, 56, trunk_h=32, frond_r=10)
    compose_palm_tree(sp, 64, 56, trunk_h=32, frond_r=10)
    # Tilted Johnny mid-hula
    compose_johnny_simple(sp, 36, 64, hat_color=GREEN, shirt_color=YELLOW)
    # Hibiscus flowers swirling around — bigger and tilted
    for cx, cy in [(8, 16), (28, 8), (52, 12), (72, 20), (16, 30), (60, 32)]:
        sp.ellipse(cx - 2, cy - 2, cx + 2, cy + 2, RED)
        sp.px(cx, cy, YELLOW)
    # Motion arcs
    sp.line(26, 38, 22, 32, BLACK)
    sp.line(46, 38, 50, 32, BLACK)
    # Smile (eyes wide!)
    sp.px(38, 56, BLACK)
    return sp


def labor_v4(h):
    """PLAYFUL — Johnny so relaxed in hammock, ZZZ rising, hard hat fallen."""
    sp = Sprite(96, 80, fill=SKY)
    compose_sand_strip(sp, 56)
    compose_palm_tree(sp, 14, 56, trunk_h=36, frond_r=10)
    compose_palm_tree(sp, 84, 56, trunk_h=36, frond_r=10)
    # Hammock between palms
    sp.line(20, 40, 80, 40, TRUNK)
    sp.line(20, 40, 28, 48, TRUNK)
    sp.line(80, 40, 72, 48, TRUNK)
    # Sleeping Johnny in hammock — horizontal
    sp.rect(34, 41, 66, 47, RED)
    sp.ellipse(60, 38, 66, 44, SKIN)  # head sticking out
    sp.px(63, 40, BLACK)  # closed eye
    # ZZZ rising
    for i, (x, y) in enumerate([(70, 20), (76, 14), (84, 8)]):
        sp.line(x, y, x + 4, y, BLACK)
        sp.line(x + 4, y, x, y + 4, BLACK)
        sp.line(x, y + 4, x + 4, y + 4, BLACK)
    # Fallen hard hat below hammock
    sp.ellipse(40, 60, 50, 64, YELLOW, outline=BLACK)
    sp.line(42, 60, 48, 60, BLACK)
    return sp


def pirate_v4(h):
    """PLAYFUL — Johnny shouting 'ARRR!', parrot wide-eyed, treasure exploding."""
    sp = Sprite(88, 96, fill=DEEPBLUE)
    compose_sand_strip(sp, 72)
    compose_palm_tree(sp, 16, 72, trunk_h=42, frond_r=10)
    # Big tricorn-hat Johnny center
    compose_johnny_simple(sp, 40, 80, hat_color=BLACK, shirt_color=RED)
    # Eye patch
    sp.px(43, 70, BLACK); sp.px(44, 70, BLACK)
    sp.line(42, 71, 45, 71, BLACK)
    # Parrot on shoulder — exaggerated (wide eyes)
    sp.ellipse(48, 64, 56, 70, GREEN, outline=BLACK)
    sp.px(54, 66, RED); sp.px(54, 67, BLACK)  # wide eye
    sp.px(57, 67, YELLOW)  # beak
    # ARRR! speech bubble
    compose_speech_bubble(sp, 72, 24, 28, 14, "ARR")
    # Treasure chest exploding open
    sp.rect(60, 80, 80, 90, TRUNK, outline=BLACK)
    sp.line(60, 80, 80, 76, YELLOW)  # lid up
    # Coins flying out
    for cx, cy in [(70, 70), (76, 64), (66, 60), (82, 70)]:
        sp.ellipse(cx, cy, cx + 3, cy + 3, YELLOW, outline=BLACK)
    # Skull on flag
    sp.rect(20, 24, 28, 32, BLACK)
    sp.px(22, 26, WHITE); sp.px(25, 26, WHITE)
    sp.px(23, 28, WHITE); sp.px(24, 28, WHITE)
    return sp


def autumn_v4(h):
    """PLAYFUL — leaves blowing wildly, scarf flapping, Johnny clutching a pumpkin."""
    sp = Sprite(112, 80, fill=ORANGE)
    compose_sand_strip(sp, 56)
    # Tilted palm with autumn fronds
    compose_palm_tree(sp, 24, 56, trunk_h=38, frond_r=12)
    # Manic Johnny with scarf flapping — center
    compose_johnny_simple(sp, 56, 64, hat_color=RED, shirt_color=YELLOW)
    sp.line(58, 56, 70, 50, RED)  # flapping scarf
    sp.line(70, 50, 76, 56, RED)
    # Pumpkin clutched
    sp.ellipse(48, 64, 58, 72, ORANGE, outline=BLACK)
    sp.line(53, 62, 53, 64, GREEN)
    # Leaves blowing across
    for cx, cy, c in [(8, 12, ORANGE), (20, 24, RED), (40, 8, YELLOW),
                       (60, 16, ORANGE), (80, 10, RED), (96, 24, YELLOW),
                       (12, 36, RED), (88, 38, ORANGE), (104, 18, YELLOW)]:
        sp.ellipse(cx, cy, cx + 4, cy + 3, c, outline=BLACK)
        sp.px(cx + 2, cy + 1, BLACK)  # vein
    # Wind motion lines
    for y in (20, 30, 40):
        sp.line(0, y, 8, y + 1, GRAY)
        sp.line(100, y, 108, y - 1, GRAY)
    return sp


def columbus_v4(h):
    """PLAYFUL — silly seagulls flock around the ship, Johnny wide-eyed."""
    sp = Sprite(96, 56, fill=SKY)
    compose_horizon(sp, 38)
    # Sand at bottom right (foreground shore)
    compose_sand_strip(sp, 38)
    # Tiny ship on horizon, but oversized for comedy
    sp.rect(48, 28, 78, 38, TRUNK, outline=BLACK)
    sp.line(56, 28, 56, 14, BLACK)  # mast
    sp.rect(50, 14, 64, 28, WHITE)  # sail
    sp.line(70, 28, 70, 18, BLACK)  # 2nd mast
    sp.rect(64, 18, 76, 28, WHITE)  # 2nd sail
    # Seagull flock — comedic mass
    for cx, cy in [(10, 8), (18, 12), (26, 6), (34, 10), (42, 4),
                    (60, 4), (72, 8), (84, 12), (12, 20), (88, 18)]:
        sp.line(cx, cy, cx + 2, cy - 1, BLACK)
        sp.line(cx + 2, cy - 1, cx + 4, cy, BLACK)
    # Wide-eyed Johnny on the shore
    compose_johnny_simple(sp, 6, 50, hat_color=None, shirt_color=RED)
    # ?! over Johnny's head
    sp.line(11, 38, 11, 42, BLACK); sp.px(11, 44, BLACK)
    return sp


def election_v4(h):
    """PLAYFUL — Johnny stuffs many ballots dramatically, oversized ballot box."""
    sp = Sprite(80, 64, fill=WHITE)
    compose_sand_strip(sp, 44)
    # Oversized ballot box center
    sp.rect(20, 28, 60, 56, TRUNK, outline=BLACK)
    sp.rect(28, 26, 52, 30, BLACK)  # slot
    # Star on box
    compose_star(sp, 40, 42, 4, YELLOW)
    # Excited Johnny on left throwing ballots
    compose_johnny_simple(sp, 6, 56, hat_color=GRAY, shirt_color=RED)
    # Ballots flying around in the air
    for cx, cy in [(12, 16), (20, 8), (28, 12), (36, 6),
                    (60, 14), (68, 8), (74, 18), (50, 4)]:
        sp.rect(cx, cy, cx + 4, cy + 5, WHITE, outline=BLACK)
        sp.line(cx + 1, cy + 2, cx + 3, cy + 2, BLACK)
    # I VOTED sticker on Johnny
    sp.ellipse(2, 50, 12, 56, RED, outline=BLACK)
    return sp


def veterans_v4(h):
    """PLAYFUL — Johnny salutes so hard his hat flies off, poppy bouquet."""
    sp = Sprite(64, 80, fill=SKY)
    compose_sand_strip(sp, 56)
    compose_palm_tree(sp, 12, 56, trunk_h=36, frond_r=8)
    # Johnny saluting on right
    compose_johnny_simple(sp, 36, 64, hat_color=None, shirt_color=DEEPBLUE)
    # Hat flying off (above his head)
    sp.rect(34, 38, 44, 42, GREEN, outline=BLACK)
    # Salute hand
    sp.px(40, 56, SKIN); sp.px(41, 55, SKIN)
    # Flag pole
    sp.line(54, 16, 54, 56, TRUNK)
    # Big flag with stars
    sp.rect(40, 16, 54, 30, RED)
    sp.rect(40, 16, 47, 23, DEEPBLUE)
    sp.px(42, 18, WHITE); sp.px(45, 20, WHITE); sp.px(43, 21, WHITE)
    sp.line(40, 23, 54, 23, WHITE)
    sp.line(40, 27, 54, 27, WHITE)
    # Poppy at base
    sp.ellipse(20, 70, 28, 76, RED, outline=BLACK)
    sp.px(24, 73, BLACK)
    # Motion lines from salute
    for y in (50, 52, 54):
        sp.line(46, y, 50, y - 1, GRAY)
    return sp


def thanksgive_v4(h):
    """PLAYFUL — comedic huge turkey, Johnny rubbing belly, food coma."""
    sp = Sprite(112, 72, fill=ORANGE)
    compose_sand_strip(sp, 50)
    compose_palm_tree(sp, 16, 50, trunk_h=34, frond_r=10)
    # Big platter center with HUGE turkey
    sp.ellipse(40, 44, 86, 60, TRUNK, outline=BLACK)
    sp.ellipse(48, 32, 80, 48, TRUNK)  # turkey body
    # Drumstick legs sticking up
    sp.line(58, 32, 58, 22, BLACK); sp.px(58, 20, RED)
    sp.line(70, 32, 70, 22, BLACK); sp.px(70, 20, RED)
    # Steam swirls
    for x in (52, 60, 68, 76):
        sp.line(x, 28, x + 2, 18, GRAY)
        sp.line(x + 2, 18, x, 12, GRAY)
    # Stuffed Johnny on right, leaning back
    compose_johnny_simple(sp, 92, 60, hat_color=ORANGE, shirt_color=YELLOW)
    # Belly bump
    sp.ellipse(89, 56, 99, 62, YELLOW, outline=BLACK)
    # Z's
    sp.line(96, 38, 100, 38, BLACK)
    sp.line(100, 38, 96, 42, BLACK)
    sp.line(96, 42, 100, 42, BLACK)
    # Cornucopia spilling on left
    sp.ellipse(2, 52, 18, 62, TRUNK, outline=BLACK)
    sp.ellipse(8, 50, 12, 54, RED)
    sp.ellipse(14, 56, 18, 60, YELLOW)
    return sp


# ---------------------------------------------------------------------------
# Batch 4 RENDERERS dict (now with v4 PLAYFUL)
# ---------------------------------------------------------------------------

RENDERERS_BATCH4 = {
    27: (lefthand_v1,  lefthand_v2,  lefthand_v3,  lefthand_v4),
    28: (hawaii_v1,    hawaii_v2,    hawaii_v3,    hawaii_v4),
    29: (labor_v1,     labor_v2,     labor_v3,     labor_v4),
    30: (pirate_v1,    pirate_v2,    pirate_v3,    pirate_v4),
    31: (autumn_v1,    autumn_v2,    autumn_v3,    autumn_v4),
    32: (columbus_v1,  columbus_v2,  columbus_v3,  columbus_v4),
    33: (election_v1,  election_v2,  election_v3,  election_v4),
    34: (veterans_v1,  veterans_v2,  veterans_v3,  veterans_v4),
    35: (thanksgive_v1,thanksgive_v2,thanksgive_v3,thanksgive_v4),
}
