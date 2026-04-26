"""
Reference renderer implementations — examples for the sub-agents that
will fill in the remaining 29 holidays.

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
# id 8: Valentine's Day  (72×80, palette: pink/red/white)
# Concept: Heart on palm trunk, rose in Johnny's teeth, pink heart-shaped
# clouds on a sunset sky.
# ---------------------------------------------------------------------------

def valentine_v1(h):
    sp = Sprite(72, 80, fill=PINK)
    # Sunset gradient — pink top, peach mid, sand
    for y in range(0, 30):
        sp.line(0, y, 71, y, PINK)
    for y in range(30, 50):
        sp.line(0, y, 71, y, ORANGE)
    compose_sand_strip(sp, 50)
    # Three heart "clouds" in the sky
    compose_heart(sp, 12, 10, 4, RED)
    compose_heart(sp, 36, 6,  3, RED)
    compose_heart(sp, 58, 14, 5, WHITE)
    # Palm tree — left side
    compose_palm_tree(sp, 16, 50, trunk_h=28, frond_r=10)
    # Heart carved into the trunk
    compose_heart(sp, 16, 38, 2, RED)
    # Johnny on the right with a rose (red dot at his face)
    compose_johnny_simple(sp, 44, 50, hat_color=None, shirt_color=RED)
    sp.px(50, 41, RED)  # rose
    sp.px(51, 41, GREEN)
    return sp


def valentine_v2(h):
    """Minimalist — single big heart over a horizon line."""
    sp = Sprite(72, 80, fill=PINK)
    compose_horizon(sp, 50)
    compose_sand_strip(sp, 50)
    # Big centered heart
    compose_heart(sp, 36, 30, 16, RED)
    # Inner highlight
    compose_heart(sp, 32, 26, 4, WHITE)
    return sp


def valentine_v3(h):
    """Busy — multiple hearts, palm, Johnny, rose petals scattered."""
    sp = Sprite(72, 80, fill=PINK)
    for y in range(0, 50):
        sp.line(0, y, 71, y, PINK if y < 30 else ORANGE)
    compose_sand_strip(sp, 50)
    # Cloud hearts
    for cx, cy, r, c in [(12, 8, 4, RED), (32, 5, 3, WHITE),
                          (52, 12, 5, RED), (64, 6, 3, WHITE)]:
        compose_heart(sp, cx, cy, r, c)
    # Two palms with heart between them
    compose_palm_tree(sp, 14, 50, trunk_h=30, frond_r=10)
    compose_palm_tree(sp, 60, 50, trunk_h=28, frond_r=10)
    compose_heart(sp, 36, 35, 6, RED)
    # Johnny center
    compose_johnny_simple(sp, 33, 60, hat_color=PINK, shirt_color=RED)
    # Rose petals on sand
    for x, y in [(8, 70), (20, 75), (45, 72), (60, 76)]:
        sp.px(x, y, RED)
        sp.px(x + 1, y, RED)
    return sp


# ---------------------------------------------------------------------------
# id 11: Mardi Gras  (144×72, palette: purple/green/gold)
# Concept: Johnny in feathered mask + beads, coconut maracas, jazz notes
# drifting on the breeze.
# ---------------------------------------------------------------------------

def mardi_v1(h):
    sp = Sprite(144, 72, fill=PURPLE)
    # Sky gradient: deep purple top, lighter purple mid
    for y in range(0, 40):
        sp.line(0, y, 143, y, PURPLE if y < 25 else PINK)
    compose_sand_strip(sp, 40)
    # Bunting — alternating colored flags strung across the top
    for i, x in enumerate(range(8, 140, 12)):
        c = [PURPLE, GREEN, YELLOW][i % 3]
        sp.line(x, 4, x + 4, 14, c)
        sp.line(x + 4, 14, x + 8, 4, c)
        sp.px(x + 4, 14, c)
    # Twin palms
    compose_palm_tree(sp, 24, 40, trunk_h=30, frond_r=12)
    compose_palm_tree(sp, 120, 40, trunk_h=30, frond_r=12)
    # Johnny center, masked, with bead chains
    compose_johnny_simple(sp, 70, 50, hat_color=PURPLE, shirt_color=GREEN)
    # Mask — yellow/purple feathers over face
    sp.rect(69, 39, 76, 41, YELLOW)
    sp.px(68, 38, PURPLE); sp.px(77, 38, PURPLE)
    sp.px(67, 37, YELLOW); sp.px(78, 37, YELLOW)
    # Beads — three loops of mixed-color dots
    for cx, cy in [(72, 47), (73, 49), (72, 51)]:
        for dx, c in [(-3, PURPLE), (-2, GREEN), (-1, YELLOW),
                       (1, YELLOW), (2, GREEN), (3, PURPLE)]:
            sp.px(cx + dx, cy, c)
    # Maracas (coconuts on sticks) on either side
    sp.ellipse(56, 50, 62, 56, TRUNK, outline=BLACK)
    sp.line(59, 56, 59, 60, TRUNK)
    sp.ellipse(82, 50, 88, 56, TRUNK, outline=BLACK)
    sp.line(85, 56, 85, 60, TRUNK)
    # Jazz notes drifting
    for cx, cy in [(96, 14), (104, 22), (112, 16)]:
        sp.line(cx, cy, cx, cy + 4, BLACK)
        sp.ellipse(cx - 2, cy + 3, cx, cy + 5, BLACK)
    return sp


def mardi_v2(h):
    """Minimalist — just the mask and beads on a clean color field."""
    sp = Sprite(144, 72, fill=PURPLE)
    compose_sand_strip(sp, 56)
    # Big centered mask
    cx = 72
    sp.ellipse(cx - 32, 20, cx + 32, 48, YELLOW, outline=BLACK)
    sp.ellipse(cx - 22, 28, cx - 8,  40, BLACK)
    sp.ellipse(cx +  8, 28, cx + 22, 40, BLACK)
    # Purple feathers on top
    for i in range(5):
        x = cx - 20 + i * 10
        sp.line(x, 20, x + 4, 4, PURPLE if i % 2 == 0 else GREEN)
        sp.line(x, 20, x + 6, 8, PURPLE if i % 2 == 0 else GREEN)
    # Bead chain across bottom
    for i, x in enumerate(range(20, 124, 4)):
        c = [PURPLE, GREEN, YELLOW][i % 3]
        sp.ellipse(x, 56, x + 3, 59, c)
    return sp


def mardi_v3(h):
    """Busy parade scene — multiple Johnnys in a line, confetti, jazz notes."""
    sp = Sprite(144, 72, fill=PURPLE)
    for y in range(0, 40):
        sp.line(0, y, 143, y, PURPLE if y < 20 else PINK)
    compose_sand_strip(sp, 40)
    # Confetti — random-ish dots in 3 colors
    confetti_pts = [(10, 5, PURPLE), (20, 10, GREEN), (30, 6, YELLOW),
                    (50, 8, PURPLE), (60, 14, GREEN), (75, 5, YELLOW),
                    (90, 12, PURPLE), (105, 7, GREEN), (120, 15, YELLOW),
                    (135, 9, PURPLE), (15, 20, YELLOW), (45, 25, GREEN),
                    (95, 28, PURPLE), (115, 32, YELLOW)]
    for x, y, c in confetti_pts:
        sp.px(x, y, c); sp.px(x + 1, y, c)
    # Three Johnnys parading across the sand
    for i, x in enumerate([20, 60, 100]):
        hat = [PURPLE, GREEN, YELLOW][i]
        compose_johnny_simple(sp, x, 60, hat_color=hat, shirt_color=GREEN if i != 1 else PURPLE)
    # Single big palm on right
    compose_palm_tree(sp, 132, 40, trunk_h=32, frond_r=10)
    # Jazz notes
    for cx, cy in [(40, 25), (80, 22), (120, 28)]:
        sp.line(cx, cy, cx, cy + 4, BLACK)
        sp.ellipse(cx - 2, cy + 3, cx, cy + 5, BLACK)
    return sp


# ---------------------------------------------------------------------------
# Reference RENDERERS dict — sub-agents extend this with their batches.
# ---------------------------------------------------------------------------
RENDERERS_REFERENCE = {
    8:  (valentine_v1, valentine_v2, valentine_v3),
    11: (mardi_v1, mardi_v2, mardi_v3),
}
