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


def valentine_v4(h):
    """PLAYFUL — comically lovestruck Johnny: oversized heart-eyes, giant
    rose, sweat drops, tilted heart raining down."""
    sp = Sprite(72, 80, fill=PINK)
    # Sky and sand
    for y in range(0, 50):
        sp.line(0, y, 71, y, PINK if y < 28 else ORANGE)
    compose_sand_strip(sp, 50)
    # Tilted oversized heart in upper-left, dramatic angle
    compose_heart(sp, 18, 14, 7, RED)
    sp.px(11, 9, WHITE); sp.px(12, 8, WHITE)  # shine highlight
    # Smaller bouncing hearts trailing
    compose_heart(sp, 30, 6, 2, WHITE)
    compose_heart(sp, 40, 12, 3, RED)
    # Motion lines around big heart
    sp.line(6, 6, 4, 4, BLACK)
    sp.line(28, 6, 30, 4, BLACK)
    sp.line(6, 22, 4, 24, BLACK)
    # Cupid arrow streaking diagonally
    sp.line(50, 8, 64, 22, TRUNK)
    sp.line(60, 12, 64, 8, TRUNK)
    sp.line(60, 12, 64, 16, TRUNK)
    # Arrow head
    sp.px(64, 22, RED); sp.px(65, 23, RED); sp.px(63, 23, RED)
    # Johnny center — lovestruck, OVERSIZED head with heart-eyes
    jx = 30
    compose_johnny_simple(sp, jx, 70, hat_color=None, shirt_color=RED)
    # Comically large heart eyes overlay
    compose_heart(sp, jx + 1, 60, 1, RED)
    compose_heart(sp, jx + 4, 60, 1, RED)
    # Open mouth (shock at love)
    sp.ellipse(jx + 1, 62, jx + 4, 64, BLACK)
    sp.px(jx + 2, 63, RED)  # tongue
    # Sweat drops flying off head
    sp.px(jx - 2, 56, SKY); sp.px(jx - 3, 55, SKY)
    sp.px(jx + 7, 56, SKY); sp.px(jx + 8, 55, SKY)
    # Giant rose in his "hand" (oversized)
    sp.ellipse(jx + 6, 64, jx + 14, 72, RED, outline=BLACK)
    sp.px(jx + 9, 67, WHITE)  # rose highlight
    sp.line(jx + 10, 72, jx + 12, 78, GREEN)  # stem
    sp.px(jx + 11, 75, GREEN)  # leaf
    # Exclamation marks bursting from his head
    sp.rect(jx - 6, 52, jx - 5, 56, RED)
    sp.px(jx - 5, 58, RED)
    sp.rect(jx + 10, 50, jx + 11, 54, RED)
    sp.px(jx + 11, 56, RED)
    # Floating heart confetti rain
    for cx, cy in [(8, 36), (54, 30), (62, 40), (4, 44)]:
        compose_heart(sp, cx, cy, 1, RED)
    return sp


def mardi_v4(h):
    """PLAYFUL — riotous parade scene: Johnny mid-leap, tongue out, beads
    flying everywhere, comically tilted mask, exaggerated motion lines."""
    sp = Sprite(144, 72, fill=PURPLE)
    # Wild sky gradient
    for y in range(0, 40):
        sp.line(0, y, 143, y, PURPLE if y < 16 else PINK)
    compose_sand_strip(sp, 40)
    # Confetti shower with motion streaks
    confetti = [(8, 6, YELLOW), (20, 12, GREEN), (32, 4, PURPLE),
                (46, 14, YELLOW), (60, 6, GREEN), (78, 10, YELLOW),
                (92, 4, PURPLE), (106, 12, GREEN), (122, 6, YELLOW),
                (136, 10, PURPLE), (16, 22, GREEN), (40, 30, PURPLE),
                (70, 24, YELLOW), (98, 32, GREEN), (130, 28, PURPLE)]
    for x, y, c in confetti:
        sp.px(x, y, c); sp.px(x + 1, y, c)
        sp.px(x, y + 1, c)  # tilted square
        # motion streak
        sp.px(x, y + 3, c)
    # Twin tilted palms (leaning into the chaos)
    compose_palm_tree(sp, 20, 40, trunk_h=28, frond_r=12)
    compose_palm_tree(sp, 124, 40, trunk_h=28, frond_r=12)
    # Bunting with extra-bouncy zig-zag
    for i, x in enumerate(range(8, 140, 8)):
        c = [PURPLE, GREEN, YELLOW, PINK][i % 4]
        ybase = 4 if i % 2 == 0 else 8
        sp.line(x, ybase, x + 4, ybase + 8, c)
        sp.line(x + 4, ybase + 8, x + 8, ybase, c)
    # Johnny center, mid-jump (raised position), arms up
    jx = 68
    base_y = 48  # higher than ground = jumping
    compose_johnny_simple(sp, jx, base_y, hat_color=PURPLE, shirt_color=GREEN)
    # Raised arms (skin pixels)
    sp.px(jx - 1, base_y - 8, SKIN); sp.px(jx - 2, base_y - 10, SKIN)
    sp.px(jx + 6, base_y - 8, SKIN); sp.px(jx + 7, base_y - 10, SKIN)
    # Motion lines under feet showing he's airborne
    sp.line(jx, base_y + 4, jx + 5, base_y + 4, BLACK)
    sp.line(jx - 2, base_y + 6, jx + 7, base_y + 6, BLACK)
    sp.line(jx + 1, base_y + 8, jx + 4, base_y + 8, BLACK)
    # Speed lines on each side
    for ox in (-12, -8, 14, 18):
        sp.line(jx + ox, base_y - 6, jx + ox + 3, base_y - 6, BLACK)
        sp.line(jx + ox, base_y - 2, jx + ox + 3, base_y - 2, BLACK)
    # Comically tilted mask (askew)
    sp.rect(jx + 1, base_y - 13, jx + 5, base_y - 11, YELLOW)
    sp.px(jx, base_y - 12, YELLOW); sp.px(jx + 6, base_y - 12, YELLOW)
    sp.px(jx + 1, base_y - 10, PURPLE)  # slipping off
    # Tongue out, big mouth
    sp.ellipse(jx + 1, base_y - 9, jx + 4, base_y - 7, BLACK)
    sp.px(jx + 2, base_y - 8, RED)
    # Beads flying in arcs everywhere
    for x, y, c in [(40, 30, PURPLE), (50, 36, GREEN), (90, 32, YELLOW),
                    (100, 38, PURPLE), (54, 50, YELLOW), (88, 50, GREEN),
                    (44, 54, PURPLE), (96, 54, YELLOW)]:
        sp.px(x, y, c); sp.px(x + 1, y, c)
        sp.px(x, y + 1, c)
    # Maracas thrown high with shake lines
    sp.ellipse(34, 26, 40, 32, TRUNK, outline=BLACK)
    sp.line(34, 24, 32, 22, BLACK)  # shake
    sp.line(40, 24, 42, 22, BLACK)
    sp.ellipse(106, 28, 112, 34, TRUNK, outline=BLACK)
    sp.line(106, 26, 104, 24, BLACK)
    sp.line(112, 26, 114, 24, BLACK)
    # Big jazz notes with motion
    for cx, cy in [(20, 18), (60, 14), (108, 20), (132, 16)]:
        sp.line(cx, cy, cx, cy + 5, BLACK)
        sp.ellipse(cx - 2, cy + 4, cx, cy + 6, BLACK)
        sp.px(cx + 2, cy - 1, BLACK)  # motion tick
    # Big "WHOO!" exclamation cloud (white burst)
    sp.ellipse(112, 42, 132, 56, WHITE, outline=BLACK)
    sp.text(115, 44, "!!!", BLACK)
    return sp


# ---------------------------------------------------------------------------
# Reference RENDERERS dict — sub-agents extend this with their batches.
# ---------------------------------------------------------------------------
RENDERERS_REFERENCE = {
    8:  (valentine_v1, valentine_v2, valentine_v3, valentine_v4),
    11: (mardi_v1, mardi_v2, mardi_v3, mardi_v4),
}
