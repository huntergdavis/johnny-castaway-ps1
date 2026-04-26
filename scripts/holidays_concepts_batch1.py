"""
Batch-1 holiday renderers — covers IDs 5, 6, 7, 9, 10, 12, 13.

Three variants per holiday (literal / minimalist / busy-scenic), following
the same contract as scripts/holidays_concepts_reference.py.

Each render function takes a single arg `h` (the holiday entry from
holidays.yml as a parsed dict) and returns a `Sprite` instance at the
holiday's specified dimensions, drawn from the shared 16-color CLUT.
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
# id 5: Elvis's Birthday  (56×70, palette: white / gold / pink)
# Concept: Johnny in pompadour wig + rhinestone collar; tiny guitar leaned
# against the palm.
# ---------------------------------------------------------------------------

def elvis_v1(h):
    sp = Sprite(56, 70, fill=PINK)
    # Stage backdrop: pink upper, sand lower
    for y in range(0, 48):
        sp.line(0, y, 55, y, PINK)
    compose_sand_strip(sp, 48)
    # Spotlight glow from above (yellow rays)
    for x in range(20, 36, 3):
        sp.line(28, 0, x, 18, YELLOW)
    # Palm at left
    compose_palm_tree(sp, 10, 48, trunk_h=24, frond_r=8)
    # Guitar leaning against the palm — body + neck
    sp.ellipse(12, 38, 18, 47, TRUNK, outline=BLACK)
    sp.line(18, 38, 22, 30, TRUNK)
    sp.line(19, 38, 23, 30, TRUNK)
    sp.px(22, 30, WHITE); sp.px(23, 30, WHITE)
    # Strings (light)
    sp.line(13, 41, 21, 33, WHITE)
    # Johnny center stage
    compose_johnny_simple(sp, 28, 60, hat_color=BLACK, shirt_color=WHITE)
    # Pompadour wig — taller black mass on top of head
    sp.rect(27, 46, 33, 49, BLACK)
    sp.px(26, 48, BLACK); sp.px(34, 48, BLACK)
    sp.px(33, 46, BLACK); sp.px(34, 47, BLACK)
    # Rhinestone collar (yellow flecks at neck)
    sp.px(28, 52, YELLOW); sp.px(30, 52, YELLOW); sp.px(32, 52, YELLOW)
    sp.px(29, 53, WHITE);  sp.px(31, 53, WHITE)
    # Bell-bottom flare on legs (white)
    sp.px(27, 60, WHITE); sp.px(34, 60, WHITE)
    # Music notes drifting up
    for cx, cy in [(42, 12), (48, 22)]:
        sp.line(cx, cy, cx, cy + 4, BLACK)
        sp.ellipse(cx - 2, cy + 3, cx, cy + 5, BLACK)
    return sp


def elvis_v2(h):
    """Minimalist — single big pompadour silhouette + guitar icon."""
    sp = Sprite(56, 70, fill=PINK)
    compose_sand_strip(sp, 56)
    # Big stylized head + pompadour (centered)
    cx = 28
    # Pompadour swoop (black)
    sp.ellipse(cx - 12, 8, cx + 12, 22, BLACK)
    sp.rect(cx - 8, 18, cx + 8, 24, BLACK)
    # Face (skin)
    sp.ellipse(cx - 8, 20, cx + 8, 36, SKIN, outline=BLACK)
    # Sunglasses
    sp.rect(cx - 7, 26, cx - 2, 29, BLACK)
    sp.rect(cx + 2, 26, cx + 7, 29, BLACK)
    sp.line(cx - 2, 27, cx + 2, 27, BLACK)
    # Star above (rhinestone)
    compose_star(sp, cx, 5, 3, YELLOW)
    # Tiny guitar icon below the face
    sp.ellipse(cx - 8, 44, cx + 0, 54, TRUNK, outline=BLACK)
    sp.line(cx, 44, cx + 6, 38, TRUNK)
    sp.line(cx + 1, 44, cx + 7, 38, TRUNK)
    sp.px(cx - 4, 49, BLACK)  # sound hole
    return sp


def elvis_v3(h):
    """Busy/scenic — full island stage scene with Johnny, palms, crowd hint."""
    sp = Sprite(56, 70, fill=PINK)
    # Sky band: pink → orange sunset
    for y in range(0, 30):
        sp.line(0, y, 55, y, PINK if y < 15 else ORANGE)
    # Stage area — yellow strip
    sp.rect(0, 30, 55, 38, YELLOW)
    compose_sand_strip(sp, 38)
    # Two palms framing the stage
    compose_palm_tree(sp, 6, 38, trunk_h=16, frond_r=6)
    compose_palm_tree(sp, 50, 38, trunk_h=16, frond_r=6)
    # Stars in the sky
    compose_star(sp, 12, 6, 2, YELLOW)
    compose_star(sp, 30, 4, 2, WHITE)
    compose_star(sp, 46, 8, 2, YELLOW)
    # Johnny center
    compose_johnny_simple(sp, 25, 56, hat_color=BLACK, shirt_color=WHITE)
    # Pompadour
    sp.rect(24, 42, 30, 45, BLACK)
    sp.px(31, 43, BLACK)
    # Microphone in hand
    sp.line(32, 50, 32, 54, GRAY)
    sp.ellipse(31, 48, 33, 51, BLACK)
    # Rhinestones
    sp.px(25, 50, YELLOW); sp.px(28, 50, YELLOW)
    # Guitar in sand
    sp.ellipse(38, 60, 46, 66, TRUNK, outline=BLACK)
    sp.line(46, 60, 52, 54, TRUNK)
    # Crowd silhouette — little dots on near-side sand
    for x in range(2, 22, 3):
        sp.px(x, 65, BLACK)
        sp.px(x, 66, BLACK)
    # Music notes
    for cx, cy in [(40, 14), (48, 22)]:
        sp.line(cx, cy, cx, cy + 3, BLACK)
        sp.ellipse(cx - 1, cy + 2, cx, cy + 4, BLACK)
    return sp


# ---------------------------------------------------------------------------
# id 6: MLK Day  (88×56, palette: maroon / cream / gold)
# Concept: Johnny at attention beside small podium with paper banner.
# ---------------------------------------------------------------------------

def mlk_v1(h):
    sp = Sprite(88, 56, fill=YELLOW)
    # Sky cream/yellow, ground sand
    sp.rect(0, 0, 87, 39, YELLOW)
    compose_sand_strip(sp, 40)
    # Palm at far left
    compose_palm_tree(sp, 10, 40, trunk_h=22, frond_r=7)
    # Podium (center-ish): vertical stand + top
    podium_x = 40
    sp.rect(podium_x, 28, podium_x + 16, 42, RED, outline=BLACK)
    sp.rect(podium_x - 2, 26, podium_x + 18, 30, RED, outline=BLACK)
    # Banner draped on the podium front (cream paper)
    sp.rect(podium_x + 2, 32, podium_x + 14, 40, WHITE, outline=BLACK)
    # Three horizontal lines suggesting text on the banner
    sp.line(podium_x + 4, 34, podium_x + 12, 34, BLACK)
    sp.line(podium_x + 4, 36, podium_x + 12, 36, BLACK)
    sp.line(podium_x + 4, 38, podium_x + 11, 38, BLACK)
    # Microphone on top of podium
    sp.line(podium_x + 8, 22, podium_x + 8, 26, GRAY)
    sp.ellipse(podium_x + 7, 20, podium_x + 9, 23, BLACK)
    # Johnny standing at attention to the right
    compose_johnny_simple(sp, 64, 48, hat_color=None, shirt_color=RED)
    # Subtle ribbon/banner star above
    compose_star(sp, 78, 8, 2, YELLOW)
    return sp


def mlk_v2(h):
    """Minimalist — large microphone silhouette + a star."""
    sp = Sprite(88, 56, fill=YELLOW)
    compose_sand_strip(sp, 50)
    cx = 44
    # Big mic stand
    sp.line(cx, 18, cx, 50, GRAY)
    sp.line(cx + 1, 18, cx + 1, 50, GRAY)
    # Mic head
    sp.ellipse(cx - 6, 6, cx + 6, 22, BLACK)
    sp.ellipse(cx - 4, 8, cx + 4, 18, GRAY)
    # Mesh lines on mic
    sp.line(cx - 4, 12, cx + 4, 12, BLACK)
    sp.line(cx - 4, 15, cx + 4, 15, BLACK)
    # Single star above
    compose_star(sp, cx, 4, 2, YELLOW)
    # Base
    sp.rect(cx - 8, 48, cx + 8, 50, BLACK)
    return sp


def mlk_v3(h):
    """Busy — Johnny addressing a small crowd from a flag-draped podium."""
    sp = Sprite(88, 56, fill=YELLOW)
    sp.rect(0, 0, 87, 39, YELLOW)
    compose_sand_strip(sp, 40)
    # Twin palms
    compose_palm_tree(sp, 6, 40, trunk_h=20, frond_r=6)
    compose_palm_tree(sp, 82, 40, trunk_h=20, frond_r=6)
    # Sun rays (cream) behind podium
    for x in range(20, 70, 6):
        sp.line(44, 4, x, 22, YELLOW)
    # Podium center
    px = 38
    sp.rect(px, 24, px + 12, 42, RED, outline=BLACK)
    sp.rect(px - 2, 22, px + 14, 26, RED, outline=BLACK)
    # Cream banner with three text lines
    sp.rect(px + 1, 28, px + 11, 40, WHITE, outline=BLACK)
    for ty in (30, 33, 36):
        sp.line(px + 3, ty, px + 9, ty, BLACK)
    # Mic
    sp.line(px + 6, 18, px + 6, 22, GRAY)
    sp.ellipse(px + 5, 16, px + 7, 19, BLACK)
    # Johnny behind/beside the podium (tiny head visible)
    sp.rect(px + 3, 18, px + 8, 22, SKIN)
    sp.px(px + 4, 19, BLACK); sp.px(px + 7, 19, BLACK)
    # Crowd silhouettes — heads on the foreground sand
    for x in range(8, 84, 6):
        sp.rect(x, 46, x + 3, 49, BLACK)
        sp.px(x + 1, 45, BLACK); sp.px(x + 2, 45, BLACK)
    # Stars dotted on sky
    compose_star(sp, 14, 8, 2, YELLOW)
    compose_star(sp, 74, 6, 2, YELLOW)
    return sp


# ---------------------------------------------------------------------------
# id 7: Groundhog Day  (64×48, palette: brown / orange / gray)
# Concept: Groundhog popping out of a sand-burrow, casting long shadow;
# Johnny squints skeptically.
# ---------------------------------------------------------------------------

def groundhog_v1(h):
    sp = Sprite(64, 48, fill=ORANGE)
    # Dawn sky orange top, sand below
    for y in range(0, 24):
        sp.line(0, y, 63, y, ORANGE if y < 14 else YELLOW)
    compose_sand_strip(sp, 24)
    # Burrow mound — darker sand bump in center-left
    sp.ellipse(14, 22, 34, 32, TRUNK, outline=BLACK)
    sp.ellipse(18, 24, 30, 30, BLACK)
    # Groundhog head poking out of burrow
    sp.ellipse(20, 18, 28, 26, TRUNK, outline=BLACK)
    # Ears
    sp.px(21, 18, TRUNK); sp.px(27, 18, TRUNK)
    sp.px(21, 17, BLACK); sp.px(27, 17, BLACK)
    # Eyes
    sp.px(22, 21, BLACK); sp.px(26, 21, BLACK)
    # Nose
    sp.px(24, 23, BLACK)
    # Buck teeth (white)
    sp.px(23, 24, WHITE); sp.px(25, 24, WHITE)
    # Long shadow trailing right across sand
    sp.line(28, 30, 56, 30, GRAY)
    sp.line(30, 31, 56, 31, GRAY)
    sp.line(32, 32, 56, 32, GRAY)
    # Johnny on the right squinting
    compose_johnny_simple(sp, 50, 42, hat_color=None, shirt_color=RED)
    # Replace eye dots with squint lines (one px each)
    sp.px(52, 32, BLACK); sp.px(53, 32, BLACK)
    return sp


def groundhog_v2(h):
    """Minimalist — single groundhog head + shadow."""
    sp = Sprite(64, 48, fill=ORANGE)
    compose_sand_strip(sp, 32)
    # Burrow circle
    sp.ellipse(20, 24, 44, 40, BLACK)
    sp.ellipse(22, 26, 42, 38, TRUNK)
    # Big groundhog head emerging
    sp.ellipse(24, 14, 40, 30, TRUNK, outline=BLACK)
    # Ears
    sp.rect(25, 12, 27, 14, TRUNK)
    sp.rect(37, 12, 39, 14, TRUNK)
    # Eyes (big)
    sp.rect(27, 18, 29, 20, BLACK)
    sp.rect(35, 18, 37, 20, BLACK)
    # Nose / mouth
    sp.px(32, 22, BLACK)
    sp.px(31, 24, BLACK); sp.px(33, 24, BLACK)
    # Long shadow stretching right
    sp.rect(40, 36, 60, 38, GRAY)
    return sp


def groundhog_v3(h):
    """Busy — full island scene, multiple burrow mounds, Johnny + palm."""
    sp = Sprite(64, 48, fill=ORANGE)
    for y in range(0, 22):
        sp.line(0, y, 63, y, ORANGE if y < 12 else YELLOW)
    compose_sand_strip(sp, 22)
    # Sun in sky
    sp.ellipse(48, 4, 60, 16, YELLOW, outline=ORANGE)
    # Palm on left
    compose_palm_tree(sp, 8, 22, trunk_h=14, frond_r=5)
    # Three burrow mounds
    for cx in (20, 36, 52):
        sp.ellipse(cx - 5, 22, cx + 5, 28, TRUNK, outline=BLACK)
        sp.ellipse(cx - 3, 24, cx + 3, 28, BLACK)
    # Groundhog popping out of middle burrow
    sp.ellipse(33, 16, 39, 24, TRUNK, outline=BLACK)
    sp.px(34, 16, TRUNK); sp.px(38, 16, TRUNK)
    sp.px(34, 19, BLACK); sp.px(38, 19, BLACK)  # eyes
    sp.px(36, 21, BLACK)  # nose
    # Long shadow
    sp.line(40, 28, 60, 28, GRAY)
    sp.line(40, 29, 58, 29, GRAY)
    # Johnny on right with skeptical posture
    compose_johnny_simple(sp, 56, 42, hat_color=None, shirt_color=RED)
    # Speech bubble with question mark area
    sp.ellipse(40, 32, 52, 40, WHITE, outline=BLACK)
    sp.px(45, 35, BLACK); sp.px(46, 35, BLACK)
    sp.px(46, 36, BLACK)
    sp.px(46, 38, BLACK)
    return sp


# ---------------------------------------------------------------------------
# id 9: Super Bowl  (96×56, palette: green / white / brown)
# Concept: Johnny in face paint with foam finger; football tee'd up;
# tiny driftwood goalposts.
# ---------------------------------------------------------------------------

def superbowl_v1(h):
    sp = Sprite(96, 56, fill=SKY)
    # Sky upper, green field lower
    sp.rect(0, 0, 95, 23, SKY)
    sp.rect(0, 24, 95, 55, GREEN)
    # Yard lines (white horizontals on the field)
    for y in (32, 40, 48):
        sp.line(0, y, 95, y, WHITE)
    # Goalposts driftwood — left end
    sp.line(8, 18, 8, 32, TRUNK)
    sp.line(9, 18, 9, 32, TRUNK)
    sp.line(4, 18, 12, 18, TRUNK)
    sp.line(4, 14, 4, 18, TRUNK)
    sp.line(12, 14, 12, 18, TRUNK)
    # Goalposts driftwood — right end
    sp.line(86, 18, 86, 32, TRUNK)
    sp.line(87, 18, 87, 32, TRUNK)
    sp.line(82, 18, 90, 18, TRUNK)
    sp.line(82, 14, 82, 18, TRUNK)
    sp.line(90, 14, 90, 18, TRUNK)
    # Football teed up center
    sp.ellipse(44, 36, 56, 42, TRUNK, outline=BLACK)
    # Football laces (white)
    sp.line(48, 39, 52, 39, WHITE)
    sp.px(49, 38, WHITE); sp.px(51, 38, WHITE)
    sp.px(49, 40, WHITE); sp.px(51, 40, WHITE)
    # Tee
    sp.line(50, 42, 50, 46, BLACK)
    sp.line(48, 46, 52, 46, BLACK)
    # Johnny on right with foam finger (red big hand)
    compose_johnny_simple(sp, 70, 50, hat_color=None, shirt_color=GREEN)
    # Face paint stripes
    sp.px(71, 40, GREEN); sp.px(74, 40, GREEN)
    # Foam finger (oversized red glove)
    sp.rect(78, 36, 86, 44, RED, outline=BLACK)
    sp.rect(82, 30, 86, 38, RED, outline=BLACK)  # extended index finger
    sp.line(76, 42, 78, 42, RED)
    return sp


def superbowl_v2(h):
    """Minimalist — single big football icon."""
    sp = Sprite(96, 56, fill=GREEN)
    compose_sand_strip(sp, 50)
    # Yard markings
    for y in (12, 24, 36):
        sp.line(0, y, 95, y, WHITE)
    # Big football (centered)
    cx, cy = 48, 28
    sp.ellipse(cx - 22, cy - 10, cx + 22, cy + 10, TRUNK, outline=BLACK)
    # Laces
    sp.line(cx - 6, cy, cx + 6, cy, WHITE)
    for dx in (-4, -2, 0, 2, 4):
        sp.line(cx + dx, cy - 2, cx + dx, cy + 2, WHITE)
    # White stripes at ends
    sp.line(cx - 18, cy - 4, cx - 18, cy + 4, WHITE)
    sp.line(cx + 18, cy - 4, cx + 18, cy + 4, WHITE)
    return sp


def superbowl_v3(h):
    """Busy — beach stadium scene with cheering Johnnys."""
    sp = Sprite(96, 56, fill=SKY)
    # Sky + field
    sp.rect(0, 0, 95, 19, SKY)
    sp.rect(0, 20, 95, 39, GREEN)
    compose_sand_strip(sp, 40)
    # Goalposts both ends
    for x_base in (6, 86):
        sp.line(x_base, 12, x_base, 24, TRUNK)
        sp.line(x_base + 1, 12, x_base + 1, 24, TRUNK)
        sp.line(x_base - 4, 12, x_base + 5, 12, TRUNK)
        sp.line(x_base - 4, 8, x_base - 4, 12, TRUNK)
        sp.line(x_base + 5, 8, x_base + 5, 12, TRUNK)
    # Yard lines on field
    for y in (26, 32):
        sp.line(0, y, 95, y, WHITE)
    # Football mid-field
    sp.ellipse(44, 30, 52, 34, TRUNK, outline=BLACK)
    sp.line(46, 32, 50, 32, WHITE)
    # Three Johnnys cheering on sand (different shirts)
    for i, x in enumerate([18, 44, 70]):
        shirt = [GREEN, WHITE, GREEN][i]
        compose_johnny_simple(sp, x, 52, hat_color=None, shirt_color=shirt)
        # Face-paint streak
        sp.px(x + 1, 42, GREEN); sp.px(x + 4, 42, GREEN)
    # Foam finger on the rightmost Johnny
    sp.rect(78, 38, 84, 46, RED, outline=BLACK)
    sp.rect(80, 32, 84, 40, RED, outline=BLACK)
    # Confetti in air
    for x, y, c in [(10, 4, WHITE), (30, 8, RED), (50, 4, WHITE),
                    (70, 6, GREEN), (88, 10, RED)]:
        sp.px(x, y, c); sp.px(x + 1, y, c)
    return sp


# ---------------------------------------------------------------------------
# id 10: Presidents' Day  (80×64, palette: navy / red / cream)
# Concept: Johnny in tricorn hat saluting tiny stars-and-stripes;
# cherry-tree sapling next to the palm.
# ---------------------------------------------------------------------------

def presidents_v1(h):
    sp = Sprite(80, 64, fill=DEEPBLUE)
    # Sky deep blue top, sand bottom
    sp.rect(0, 0, 79, 39, DEEPBLUE)
    compose_sand_strip(sp, 40)
    # Palm on left
    compose_palm_tree(sp, 10, 40, trunk_h=24, frond_r=8)
    # Cherry-tree sapling next to palm — short trunk + red dots
    sp.line(20, 40, 20, 32, TRUNK)
    sp.line(21, 40, 21, 32, TRUNK)
    sp.ellipse(15, 26, 26, 34, GREEN, outline=DGREEN)
    for cx, cy in [(17, 28), (22, 27), (19, 31), (24, 30)]:
        sp.px(cx, cy, RED); sp.px(cx + 1, cy, RED)
    # Flag on a pole — center-right
    sp.line(48, 18, 48, 50, TRUNK)
    sp.rect(49, 18, 64, 30, WHITE, outline=BLACK)
    # Stripes
    for y in (20, 22, 24, 26, 28):
        sp.line(56, y, 64, y, RED)
    # Canton
    sp.rect(49, 18, 56, 24, DEEPBLUE)
    # Tiny stars on canton
    for cx, cy in [(51, 20), (54, 20), (51, 22), (54, 22)]:
        sp.px(cx, cy, WHITE)
    # Johnny saluting on right
    compose_johnny_simple(sp, 70, 56, hat_color=DEEPBLUE, shirt_color=RED)
    # Tricorn hat — wider points on either side
    sp.px(69, 46, DEEPBLUE); sp.px(76, 46, DEEPBLUE)
    sp.px(68, 47, DEEPBLUE); sp.px(77, 47, DEEPBLUE)
    # Saluting hand — small line at brow
    sp.px(72, 49, SKIN); sp.px(73, 49, SKIN)
    return sp


def presidents_v2(h):
    """Minimalist — single tricorn-hat silhouette + a star."""
    sp = Sprite(80, 64, fill=DEEPBLUE)
    compose_sand_strip(sp, 56)
    # Big tricorn hat centered
    cx, cy = 40, 32
    # Brim — wide black
    sp.ellipse(cx - 24, cy + 4, cx + 24, cy + 14, BLACK)
    # Crown — black with a peak
    sp.rect(cx - 14, cy - 10, cx + 14, cy + 6, BLACK)
    sp.line(cx - 14, cy - 10, cx, cy - 18, BLACK)
    sp.line(cx, cy - 18, cx + 14, cy - 10, BLACK)
    # Cockade (red/white)
    sp.ellipse(cx - 4, cy - 4, cx + 4, cy + 4, RED, outline=WHITE)
    sp.px(cx, cy, WHITE)
    # Star above
    compose_star(sp, cx, 8, 3, WHITE)
    return sp


def presidents_v3(h):
    """Busy — patriotic island scene with multiple flags + cherry tree."""
    sp = Sprite(80, 64, fill=DEEPBLUE)
    # Sky gradient
    for y in range(0, 36):
        sp.line(0, y, 79, y, DEEPBLUE if y < 20 else SKY)
    compose_sand_strip(sp, 36)
    # Stars in sky
    for cx, cy in [(8, 6), (24, 4), (40, 8), (60, 4), (74, 8)]:
        compose_star(sp, cx, cy, 2, WHITE)
    # Palm + cherry sapling on left
    compose_palm_tree(sp, 8, 36, trunk_h=22, frond_r=7)
    sp.line(20, 36, 20, 28, TRUNK)
    sp.ellipse(15, 22, 26, 30, GREEN, outline=DGREEN)
    for cx, cy in [(17, 24), (22, 25), (19, 27), (23, 26)]:
        sp.px(cx, cy, RED)
    # Bunting along top — alternating red/white/blue triangles
    for i, x in enumerate(range(2, 80, 8)):
        c = [RED, WHITE, DEEPBLUE][i % 3]
        sp.line(x, 14, x + 4, 20, c)
        sp.line(x + 4, 20, x + 8, 14, c)
    # Center flag pole + flag
    sp.line(40, 22, 40, 50, TRUNK)
    sp.rect(41, 22, 56, 32, WHITE, outline=BLACK)
    for y in (24, 26, 28, 30):
        sp.line(48, y, 56, y, RED)
    sp.rect(41, 22, 48, 27, DEEPBLUE)
    for cx, cy in [(43, 24), (46, 24), (43, 26), (46, 26)]:
        sp.px(cx, cy, WHITE)
    # Johnny saluting on right
    compose_johnny_simple(sp, 64, 56, hat_color=DEEPBLUE, shirt_color=RED)
    sp.px(63, 46, DEEPBLUE); sp.px(70, 46, DEEPBLUE)
    sp.px(66, 49, SKIN); sp.px(67, 49, SKIN)
    # Smaller secondary flag held aloft on right
    sp.line(72, 50, 72, 42, TRUNK)
    sp.rect(73, 42, 78, 46, RED, outline=BLACK)
    return sp


# ---------------------------------------------------------------------------
# id 12: Pi Day  (48×40, palette: cream / brown / blue)
# Concept: Pie cooling on a rock; pi symbol drawn in sand;
# Johnny in nerd glasses holding slide rule.
# ---------------------------------------------------------------------------

def piday_v1(h):
    sp = Sprite(48, 40, fill=SKY)
    sp.rect(0, 0, 47, 19, SKY)
    compose_sand_strip(sp, 20)
    # Rock (gray) on left
    sp.ellipse(2, 18, 18, 26, GRAY, outline=BLACK)
    # Pie on the rock — circular crust
    sp.ellipse(4, 14, 16, 22, TRUNK, outline=BLACK)
    sp.ellipse(6, 16, 14, 20, ORANGE)  # filling
    # Steam wisps
    for cx in (8, 10, 12):
        sp.px(cx, 12, WHITE)
        sp.px(cx, 10, WHITE)
    # Pi symbol drawn in sand (right side, dark sand-brown lines)
    px = 30
    py = 30
    sp.line(px, py, px + 10, py, BLACK)         # top bar
    sp.line(px + 2, py, px + 2, py + 6, BLACK)  # left leg
    sp.line(px + 8, py, px + 8, py + 6, BLACK)  # right leg
    # Johnny on far right with glasses
    compose_johnny_simple(sp, 38, 36, hat_color=None, shirt_color=PURPLE)
    # Glasses — overdraw on eye line
    sp.rect(38, 27, 40, 29, BLACK)
    sp.rect(42, 27, 44, 29, BLACK)
    sp.line(40, 28, 42, 28, BLACK)
    # Slide rule (small white rectangle in hand)
    sp.rect(36, 32, 39, 33, WHITE)
    return sp


def piday_v2(h):
    """Minimalist — giant pi symbol over sand."""
    sp = Sprite(48, 40, fill=SKY)
    compose_sand_strip(sp, 32)
    # Big pi symbol centered
    cx = 24
    # Top bar
    sp.rect(cx - 14, 10, cx + 14, 14, BLACK)
    # Two legs
    sp.rect(cx - 10, 14, cx - 6, 30, BLACK)
    sp.rect(cx + 6, 14, cx + 10, 30, BLACK)
    # Curl on right leg foot (decorative)
    sp.px(cx + 11, 30, BLACK)
    sp.px(cx + 12, 29, BLACK)
    return sp


def piday_v3(h):
    """Busy — multiple pies, Johnny calculating, palm, big pi sky-writing."""
    sp = Sprite(48, 40, fill=SKY)
    sp.rect(0, 0, 47, 19, SKY)
    compose_sand_strip(sp, 20)
    # Pi sky-written by clouds (white pixels)
    sp.line(4, 4, 14, 4, WHITE)
    sp.line(6, 4, 6, 10, WHITE)
    sp.line(12, 4, 12, 10, WHITE)
    # Palm
    compose_palm_tree(sp, 4, 20, trunk_h=12, frond_r=4)
    # Two pies on rocks
    sp.ellipse(12, 22, 22, 28, GRAY, outline=BLACK)
    sp.ellipse(14, 18, 20, 24, TRUNK, outline=BLACK)
    sp.ellipse(15, 19, 19, 23, ORANGE)
    sp.ellipse(26, 26, 36, 32, GRAY, outline=BLACK)
    sp.ellipse(28, 22, 34, 28, TRUNK, outline=BLACK)
    sp.ellipse(29, 23, 33, 27, RED)  # cherry pie
    # Steam from both
    for cx in (16, 18, 30, 32):
        sp.px(cx, 17, WHITE); sp.px(cx, 15, WHITE)
    # Pi drawn in sand under the right pie
    sp.line(38, 33, 44, 33, BLACK)
    sp.px(39, 34, BLACK); sp.px(39, 35, BLACK)
    sp.px(43, 34, BLACK); sp.px(43, 35, BLACK)
    # Tiny Johnny tucked between pies
    compose_johnny_simple(sp, 22, 38, hat_color=None, shirt_color=PURPLE)
    sp.rect(22, 29, 23, 30, BLACK)
    sp.rect(25, 29, 26, 30, BLACK)
    return sp


# ---------------------------------------------------------------------------
# id 13: First Day of Spring  (96×80, palette: pink / green / blue)
# Concept: Cherry blossoms blooming on the palm; butterflies; Johnny
# stretching in the sun.
# ---------------------------------------------------------------------------

def spring_v1(h):
    sp = Sprite(96, 80, fill=PINK)
    # Sky pale blue, sand bottom
    sp.rect(0, 0, 95, 55, SKY)
    compose_sand_strip(sp, 56)
    # Sun in upper right
    sp.ellipse(76, 6, 90, 20, YELLOW, outline=ORANGE)
    # Palm with blossoms instead of fronds
    compose_palm_tree(sp, 24, 56, trunk_h=36, frond_r=12)
    # Pink blossom dots all around the palm crown
    for cx, cy in [(16, 18), (22, 14), (28, 16), (32, 22),
                   (12, 24), (36, 28), (20, 28), (30, 12)]:
        sp.px(cx, cy, PINK); sp.px(cx + 1, cy, PINK)
        sp.px(cx, cy + 1, PINK); sp.px(cx + 1, cy + 1, PINK)
        sp.px(cx + 1, cy + 1, WHITE)  # center highlight
    # Grass tufts along the sand
    for x in range(2, 96, 8):
        sp.px(x, 56, GREEN); sp.px(x + 1, 55, GREEN); sp.px(x + 2, 56, GREEN)
    # Butterflies — two-circle bodies with antennae
    for cx, cy, c in [(50, 18, YELLOW), (66, 28, PINK), (78, 38, PURPLE)]:
        sp.ellipse(cx - 3, cy - 2, cx, cy + 2, c)
        sp.ellipse(cx, cy - 2, cx + 3, cy + 2, c)
        sp.px(cx, cy, BLACK)
        sp.px(cx - 1, cy - 3, BLACK); sp.px(cx + 1, cy - 3, BLACK)
    # Johnny center-stretching — arms up
    compose_johnny_simple(sp, 56, 70, hat_color=None, shirt_color=GREEN)
    # Arms raised (skin pixels above shoulders)
    sp.px(55, 62, SKIN); sp.px(54, 60, SKIN); sp.px(53, 58, SKIN)
    sp.px(62, 62, SKIN); sp.px(63, 60, SKIN); sp.px(64, 58, SKIN)
    return sp


def spring_v2(h):
    """Minimalist — single cherry blossom branch + butterfly."""
    sp = Sprite(96, 80, fill=SKY)
    compose_sand_strip(sp, 70)
    # Single curved branch from lower left to upper right
    for i in range(30):
        x = 8 + i * 2
        y = 60 - i
        sp.px(x, y, TRUNK)
        sp.px(x, y + 1, TRUNK)
    # Big blossom clusters along branch
    for cx, cy in [(20, 52), (36, 42), (52, 32), (68, 22)]:
        sp.ellipse(cx - 6, cy - 6, cx + 6, cy + 6, PINK, outline=BLACK)
        sp.ellipse(cx - 2, cy - 2, cx + 2, cy + 2, WHITE)
        sp.px(cx, cy, YELLOW)
    # Single big butterfly upper-left
    cx, cy = 16, 16
    sp.ellipse(cx - 6, cy - 5, cx, cy + 5, PURPLE, outline=BLACK)
    sp.ellipse(cx, cy - 5, cx + 6, cy + 5, PURPLE, outline=BLACK)
    sp.px(cx, cy, BLACK)
    sp.px(cx - 1, cy - 6, BLACK); sp.px(cx + 1, cy - 6, BLACK)
    return sp


def spring_v3(h):
    """Busy — full vernal scene: blossoms, butterflies, tulips, Johnny."""
    sp = Sprite(96, 80, fill=SKY)
    sp.rect(0, 0, 95, 55, SKY)
    compose_sand_strip(sp, 56)
    # Big sun
    sp.ellipse(80, 4, 94, 18, YELLOW, outline=ORANGE)
    # Sun rays
    for dx, dy in [(-10, 0), (-7, -7), (0, -10), (7, -7)]:
        sp.line(87, 11, 87 + dx, 11 + dy, YELLOW)
    # Two palms with blossoms
    compose_palm_tree(sp, 16, 56, trunk_h=36, frond_r=10)
    compose_palm_tree(sp, 80, 56, trunk_h=32, frond_r=10)
    # Blossom clusters
    blossoms = [(8, 18), (14, 14), (20, 18), (24, 22), (12, 24),
                (74, 24), (80, 18), (86, 22), (78, 28)]
    for cx, cy in blossoms:
        sp.ellipse(cx - 1, cy - 1, cx + 1, cy + 1, PINK)
        sp.px(cx, cy, WHITE)
    # Tulips on the sand row
    for i, x in enumerate(range(34, 70, 6)):
        c = [RED, YELLOW, PINK, PURPLE, ORANGE, WHITE][i % 6]
        sp.line(x, 60, x, 66, GREEN)
        sp.ellipse(x - 2, 58, x + 2, 62, c, outline=BLACK)
        # Two leaves
        sp.px(x - 2, 64, GREEN); sp.px(x + 2, 64, GREEN)
    # Butterflies dotted around
    for cx, cy, c in [(32, 16, YELLOW), (48, 8, PINK), (60, 20, PURPLE),
                      (44, 30, ORANGE)]:
        sp.ellipse(cx - 3, cy - 2, cx, cy + 2, c)
        sp.ellipse(cx, cy - 2, cx + 3, cy + 2, c)
        sp.px(cx, cy, BLACK)
        sp.px(cx - 1, cy - 3, BLACK); sp.px(cx + 1, cy - 3, BLACK)
    # Johnny center stretching
    compose_johnny_simple(sp, 48, 72, hat_color=None, shirt_color=GREEN)
    sp.px(47, 64, SKIN); sp.px(46, 62, SKIN); sp.px(45, 60, SKIN)
    sp.px(54, 64, SKIN); sp.px(55, 62, SKIN); sp.px(56, 60, SKIN)
    # Grass tufts
    for x in range(2, 96, 5):
        sp.px(x, 56, GREEN); sp.px(x + 1, 55, GREEN)
    return sp


# ---------------------------------------------------------------------------
# Public registry — runner imports this dict and invokes per-id variants.
# ---------------------------------------------------------------------------

RENDERERS_BATCH1 = {
    5:  (elvis_v1, elvis_v2, elvis_v3),
    6:  (mlk_v1, mlk_v2, mlk_v3),
    7:  (groundhog_v1, groundhog_v2, groundhog_v3),
    9:  (superbowl_v1, superbowl_v2, superbowl_v3),
    10: (presidents_v1, presidents_v2, presidents_v3),
    12: (piday_v1, piday_v2, piday_v3),
    13: (spring_v1, spring_v2, spring_v3),
}
