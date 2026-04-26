"""
Batch 2 holiday renderers — April Fool, Easter, Earth Day, Star Wars,
Cinco de Mayo, Mother's Day. Each holiday has 3 variants:

  v1 — literal: closest reading of the design doc concept
  v2 — minimalist: stripped-down silhouette / icon style
  v3 — busy/scenic: more elements, full island scene

All sprites are 4-bit indexed (16-color shared CLUT from holidays_art_lib).
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
# id 14: April Fool's Day  (80×64, palette: yellow/red/blue)
# Concept: Whoopee cushion under Johnny's hammock; sun has a clown nose;
# palm leaves upside-down.
# ---------------------------------------------------------------------------

def aprilfool_v1(h):
    """Literal — hammock, whoopee cushion, clown-nose sun, upside-down palm."""
    sp = Sprite(80, 64, fill=SKY)
    compose_sand_strip(sp, 50)
    # Sun, top-left, with a red clown-nose blob hanging under it
    sp.ellipse(6, 4, 18, 16, YELLOW, outline=BLACK)
    # Clown nose (red ball protruding from the bottom of the sun)
    sp.ellipse(10, 13, 14, 17, RED, outline=BLACK)
    # Two upside-down palms — trunk on top, fronds at bottom
    # Left palm
    sp.line(28, 0, 28, 22, TRUNK)
    sp.line(29, 0, 29, 22, TRUNK)
    for dx, dy in [(-7, 2), (-4, 5), (0, 8), (4, 5), (7, 2),
                   (-4, -3), (4, -3)]:
        sp.line(28, 22, 28 + dx, 22 + dy, GREEN)
        sp.line(28, 23, 28 + dx, 23 + dy, DGREEN)
    # Right palm — also flipped
    sp.line(64, 0, 64, 18, TRUNK)
    sp.line(65, 0, 65, 18, TRUNK)
    for dx, dy in [(-6, 2), (-3, 4), (0, 7), (3, 4), (6, 2)]:
        sp.line(64, 18, 64 + dx, 18 + dy, GREEN)
        sp.line(64, 19, 64 + dx, 19 + dy, DGREEN)
    # Hammock between palms (slung under, sand-anchored)
    sp.line(30, 38, 63, 38, RED)
    sp.line(30, 39, 63, 39, RED)
    sp.line(31, 40, 62, 40, RED)
    sp.line(32, 41, 61, 41, RED)
    # Hammock attachment cords
    sp.line(28, 22, 30, 38, BLACK)
    sp.line(64, 18, 63, 38, BLACK)
    # Johnny lying in the hammock
    sp.rect(40, 36, 53, 38, SKIN)
    sp.px(43, 35, BLACK); sp.px(45, 35, BLACK)  # eyes
    # Whoopee cushion under hammock — red disc on sand
    sp.ellipse(42, 48, 54, 54, RED, outline=BLACK)
    sp.line(54, 51, 58, 50, RED)  # nozzle
    # "PFFT" puff lines
    sp.px(58, 49, WHITE); sp.px(60, 48, WHITE); sp.px(62, 49, WHITE)
    sp.px(60, 51, WHITE); sp.px(62, 52, WHITE)
    return sp


def aprilfool_v2(h):
    """Minimalist — single huge clown nose on a yellow field."""
    sp = Sprite(80, 64, fill=YELLOW)
    compose_horizon(sp, 50)
    compose_sand_strip(sp, 50)
    # Big red clown nose, centered
    sp.ellipse(28, 14, 52, 38, RED, outline=BLACK)
    # White highlight
    sp.ellipse(33, 18, 38, 22, WHITE)
    # Tiny "!" mark above
    sp.rect(38, 4, 41, 9, BLACK)
    sp.rect(38, 11, 41, 12, BLACK)
    return sp


def aprilfool_v3(h):
    """Busy — everything at once: pranks all over the beach."""
    sp = Sprite(80, 64, fill=SKY)
    # Sun with clown nose
    sp.ellipse(2, 2, 12, 12, YELLOW, outline=BLACK)
    sp.ellipse(5, 9, 9, 13, RED, outline=BLACK)
    # Three upside-down palms across the top
    for x_anchor in [22, 44, 66]:
        sp.line(x_anchor, 0, x_anchor, 16, TRUNK)
        sp.line(x_anchor + 1, 0, x_anchor + 1, 16, TRUNK)
        for dx, dy in [(-5, 2), (-3, 4), (0, 6), (3, 4), (5, 2)]:
            sp.line(x_anchor, 16, x_anchor + dx, 16 + dy, GREEN)
            sp.line(x_anchor, 17, x_anchor + dx, 17 + dy, DGREEN)
    compose_sand_strip(sp, 48)
    # Johnny startled center, hat askew
    compose_johnny_simple(sp, 36, 60, hat_color=RED, shirt_color=YELLOW)
    # Whoopee cushion left of him
    sp.ellipse(18, 56, 30, 62, RED, outline=BLACK)
    sp.px(31, 58, WHITE); sp.px(33, 57, WHITE)
    # Banana-peel prank to the right
    sp.line(52, 60, 58, 58, YELLOW)
    sp.line(52, 61, 58, 59, YELLOW)
    sp.px(58, 57, YELLOW); sp.px(59, 57, YELLOW)
    # Speech bubble with "?!"
    compose_speech_bubble(sp, 46, 30, 16, 10, "?!")
    # Confetti drifting
    for x, y, c in [(8, 24, RED), (16, 18, YELLOW), (28, 28, RED),
                    (50, 22, RED), (62, 18, YELLOW), (72, 26, DEEPBLUE),
                    (74, 38, RED), (10, 38, YELLOW)]:
        sp.px(x, y, c); sp.px(x + 1, y, c)
    return sp


# ---------------------------------------------------------------------------
# id 15: Easter  (72×64, palette: pastel purple / pastel green / pastel yellow)
# Concept: Pastel eggs hidden in sand, bunny ears on Johnny, basket woven
# from palm fronds.
# ---------------------------------------------------------------------------

def easter_v1(h):
    """Literal — Johnny w/ bunny ears, basket of eggs, eggs in sand."""
    sp = Sprite(72, 64, fill=SKY)
    compose_sand_strip(sp, 40)
    # Palm tree on the left
    compose_palm_tree(sp, 12, 40, trunk_h=24, frond_r=8)
    # Johnny in the middle with white bunny ears
    compose_johnny_simple(sp, 32, 54, hat_color=None, shirt_color=PINK)
    # Bunny ears — two upright white ovals atop the head
    sp.rect(32, 38, 33, 42, WHITE)
    sp.rect(36, 38, 37, 42, WHITE)
    sp.px(32, 37, WHITE); sp.px(33, 37, WHITE)
    sp.px(36, 37, WHITE); sp.px(37, 37, WHITE)
    # Pink inner ear tint
    sp.px(32, 40, PINK); sp.px(36, 40, PINK)
    # Basket on the sand to his right
    sp.rect(46, 48, 60, 56, TRUNK, outline=BLACK)
    # Basket weave lines
    for x in range(48, 60, 3):
        sp.line(x, 48, x, 56, DGREEN)
    # Basket handle (palm-frond)
    sp.line(46, 48, 49, 44, GREEN)
    sp.line(49, 44, 57, 44, GREEN)
    sp.line(57, 44, 60, 48, GREEN)
    # Eggs in basket — three pastel
    sp.ellipse(48, 44, 51, 48, PINK, outline=BLACK)
    sp.ellipse(52, 43, 55, 47, YELLOW, outline=BLACK)
    sp.ellipse(56, 44, 59, 48, PURPLE, outline=BLACK)
    # Eggs hidden in the sand
    sp.ellipse(4, 56, 7, 60, PINK, outline=BLACK)
    sp.ellipse(20, 58, 23, 62, YELLOW, outline=BLACK)
    sp.ellipse(64, 56, 67, 60, PURPLE, outline=BLACK)
    return sp


def easter_v2(h):
    """Minimalist — one large pastel egg, decorated."""
    sp = Sprite(72, 64, fill=SKY)
    compose_sand_strip(sp, 52)
    # Big egg
    sp.ellipse(22, 8, 50, 48, PINK, outline=BLACK)
    # Decorative bands
    sp.line(22, 20, 50, 20, PURPLE)
    sp.line(22, 21, 50, 21, PURPLE)
    sp.line(22, 34, 50, 34, YELLOW)
    sp.line(22, 35, 50, 35, YELLOW)
    # Zig-zag in middle band
    for x in range(24, 50, 4):
        sp.px(x, 27, PURPLE)
        sp.px(x + 1, 28, PURPLE)
        sp.px(x + 2, 27, PURPLE)
    return sp


def easter_v3(h):
    """Busy — egg hunt scene, multiple eggs, bunny-Johnny, basket, palm."""
    sp = Sprite(72, 64, fill=SKY)
    compose_sand_strip(sp, 38)
    # Two palms framing
    compose_palm_tree(sp, 8, 38, trunk_h=22, frond_r=7)
    compose_palm_tree(sp, 64, 38, trunk_h=22, frond_r=7)
    # Johnny center w/ bunny ears
    compose_johnny_simple(sp, 32, 56, hat_color=None, shirt_color=PURPLE)
    sp.rect(32, 40, 33, 44, WHITE)
    sp.rect(36, 40, 37, 44, WHITE)
    sp.px(32, 39, WHITE); sp.px(33, 39, WHITE)
    sp.px(36, 39, WHITE); sp.px(37, 39, WHITE)
    sp.px(32, 42, PINK); sp.px(36, 42, PINK)
    # Lots of pastel eggs scattered on sand
    egg_spots = [
        (3, 48, PINK), (12, 58, YELLOW), (20, 50, PURPLE),
        (22, 60, PINK), (44, 50, YELLOW), (52, 58, PURPLE),
        (60, 52, PINK), (66, 60, YELLOW), (4, 60, PURPLE),
    ]
    for x, y, c in egg_spots:
        sp.ellipse(x, y, x + 3, y + 4, c, outline=BLACK)
    # Tiny basket between palms
    sp.rect(14, 44, 22, 48, TRUNK, outline=BLACK)
    sp.line(14, 44, 18, 41, GREEN)
    sp.line(18, 41, 22, 44, GREEN)
    return sp


# ---------------------------------------------------------------------------
# id 16: Earth Day  (88×72, palette: forest green / ocean blue / earth brown)
# Concept: Johnny planting a sapling, globe-shaped buoy on shore, recycling
# bin from a barrel.
# ---------------------------------------------------------------------------

def earthday_v1(h):
    """Literal — Johnny planting sapling, globe buoy, recycling barrel."""
    sp = Sprite(88, 72, fill=SKY)
    compose_horizon(sp, 36)
    # Water band
    for y in range(36, 48):
        sp.line(0, y, 87, y, DEEPBLUE)
    compose_sand_strip(sp, 48)
    # Globe-buoy washed ashore — left
    sp.ellipse(6, 44, 22, 60, SKY, outline=BLACK)
    # Continents (blobby green shapes)
    sp.rect(10, 48, 14, 52, DGREEN)
    sp.line(15, 51, 18, 53, DGREEN)
    sp.px(11, 54, DGREEN); sp.px(12, 55, DGREEN)
    sp.line(16, 56, 19, 57, DGREEN)
    # Equator line
    sp.line(7, 52, 21, 52, BLACK)
    # Johnny center, kneeling to plant a sapling
    compose_johnny_simple(sp, 38, 64, hat_color=GREEN, shirt_color=DGREEN)
    # Sapling — small green stem with two leaves
    sp.line(50, 60, 50, 66, TRUNK)
    sp.line(51, 60, 51, 66, TRUNK)
    # Leaves
    sp.ellipse(46, 56, 50, 60, GREEN, outline=DGREEN)
    sp.ellipse(51, 56, 55, 60, GREEN, outline=DGREEN)
    # Mound of dirt around stem
    sp.ellipse(46, 64, 56, 68, TRUNK, outline=BLACK)
    # Recycling-barrel bin on the right
    sp.rect(68, 48, 80, 64, GREEN, outline=BLACK)
    # Barrel hoops
    sp.line(68, 52, 80, 52, DGREEN)
    sp.line(68, 60, 80, 60, DGREEN)
    # Recycling triangle (3 chevrons)
    sp.line(72, 54, 76, 54, WHITE)
    sp.line(76, 54, 74, 58, WHITE)
    sp.line(74, 58, 72, 54, WHITE)
    # Sun in the sky
    sp.ellipse(74, 4, 84, 14, YELLOW, outline=BLACK)
    return sp


def earthday_v2(h):
    """Minimalist — single big globe with green continents."""
    sp = Sprite(88, 72, fill=SKY)
    compose_sand_strip(sp, 60)
    # Big globe
    cx, cy, r = 44, 34, 26
    sp.ellipse(cx - r, cy - r, cx + r, cy + r, DEEPBLUE, outline=BLACK)
    # Continents (rough green blobs)
    sp.rect(28, 22, 38, 30, GREEN)
    sp.line(38, 26, 44, 28, GREEN)
    sp.rect(48, 26, 58, 34, GREEN)
    sp.rect(34, 38, 44, 46, GREEN)
    sp.line(50, 40, 56, 44, GREEN)
    # Equator line
    sp.line(cx - r, cy, cx + r, cy, BLACK)
    # Meridian
    sp.line(cx, cy - r, cx, cy + r, BLACK)
    return sp


def earthday_v3(h):
    """Busy — full eco scene: planted trees, beach cleanup, ocean."""
    sp = Sprite(88, 72, fill=SKY)
    compose_horizon(sp, 30)
    for y in range(30, 44):
        sp.line(0, y, 87, y, DEEPBLUE)
    compose_sand_strip(sp, 44)
    # Two saplings planted on left
    for x in [10, 22]:
        sp.line(x, 50, x, 60, TRUNK)
        sp.ellipse(x - 3, 44, x + 3, 50, GREEN, outline=DGREEN)
        sp.ellipse(x - 5, 58, x + 5, 62, TRUNK)
    # Johnny in the middle planting another
    compose_johnny_simple(sp, 38, 64, hat_color=GREEN, shirt_color=DGREEN)
    sp.line(50, 56, 50, 64, TRUNK)
    sp.ellipse(46, 50, 54, 56, GREEN, outline=DGREEN)
    # Recycling barrel right
    sp.rect(64, 48, 76, 64, GREEN, outline=BLACK)
    sp.line(64, 52, 76, 52, DGREEN)
    sp.line(64, 60, 76, 60, DGREEN)
    sp.line(67, 54, 72, 54, WHITE)
    sp.line(72, 54, 70, 58, WHITE)
    sp.line(70, 58, 67, 54, WHITE)
    # Globe-buoy half-buried at far right
    sp.ellipse(78, 56, 86, 64, SKY, outline=BLACK)
    sp.line(78, 60, 86, 60, BLACK)
    sp.px(80, 58, GREEN); sp.px(83, 62, GREEN)
    # Sun
    sp.ellipse(2, 2, 12, 12, YELLOW, outline=BLACK)
    # A few sea birds — simple "M" shapes
    for cx, cy in [(26, 12), (44, 8), (60, 14)]:
        sp.line(cx - 2, cy + 1, cx, cy - 1, BLACK)
        sp.line(cx, cy - 1, cx + 2, cy + 1, BLACK)
    return sp


# ---------------------------------------------------------------------------
# id 17: Star Wars Day  (72×96, palette: jedi tan / starfield / saber green)
# Concept: Johnny in brown Jedi robe wielding a glowing palm-frond
# lightsaber, tiny droid silhouette in the surf.
# ---------------------------------------------------------------------------

def starwars_v1(h):
    """Literal — Jedi-robed Johnny, palm-frond lightsaber, droid in surf."""
    sp = Sprite(72, 96, fill=DEEPBLUE)
    # Starfield
    star_pts = [(8, 6), (20, 14), (32, 4), (48, 12), (60, 8), (66, 22),
                (4, 28), (52, 30), (16, 34), (40, 40)]
    for x, y in star_pts:
        sp.px(x, y, WHITE)
    # A couple bigger stars
    compose_star(sp, 12, 18, 2, WHITE)
    compose_star(sp, 56, 26, 2, YELLOW)
    # Horizon and sand
    compose_horizon(sp, 60)
    for y in range(60, 72):
        sp.line(0, y, 71, y, DEEPBLUE)
    compose_sand_strip(sp, 72)
    # Johnny center — Jedi robe drawn manually (brown rectangle)
    base_y = 90
    x = 30
    # Robe (brown trunk-color tunic)
    sp.rect(x - 4, base_y - 18, x + 8, base_y, TRUNK, outline=BLACK)
    # Head
    sp.rect(x + 1, base_y - 23, x + 5, base_y - 19, SKIN)
    sp.px(x + 2, base_y - 21, BLACK); sp.px(x + 4, base_y - 21, BLACK)
    # Hood — darker over head
    sp.line(x, base_y - 24, x + 6, base_y - 24, DGREEN)
    sp.line(x - 1, base_y - 23, x - 1, base_y - 19, DGREEN)
    sp.line(x + 7, base_y - 23, x + 7, base_y - 19, DGREEN)
    # Arm extending up-right
    sp.line(x + 8, base_y - 14, x + 14, base_y - 22, TRUNK)
    sp.line(x + 9, base_y - 14, x + 15, base_y - 22, TRUNK)
    # Lightsaber hilt
    sp.rect(x + 14, base_y - 24, x + 16, base_y - 22, GRAY)
    # Glowing green palm-frond saber blade — stretches up
    sp.line(x + 15, base_y - 24, x + 15, base_y - 56, GREEN)
    sp.line(x + 14, base_y - 24, x + 14, base_y - 54, GREEN)
    sp.line(x + 16, base_y - 24, x + 16, base_y - 54, GREEN)
    # White hot-core
    sp.line(x + 15, base_y - 25, x + 15, base_y - 50, WHITE)
    # Tiny droid silhouette in the surf — simple R2-ish (dome on cylinder)
    dx = 56
    sp.rect(dx, 64, dx + 6, 70, WHITE, outline=BLACK)
    sp.ellipse(dx, 60, dx + 6, 64, WHITE, outline=BLACK)
    sp.px(dx + 3, 62, GREEN)  # eye
    sp.line(dx, 70, dx, 72, BLACK)
    sp.line(dx + 6, 70, dx + 6, 72, BLACK)
    return sp


def starwars_v2(h):
    """Minimalist — single vertical lightsaber on a starfield."""
    sp = Sprite(72, 96, fill=DEEPBLUE)
    # Starfield
    for x, y in [(6, 8), (12, 20), (24, 12), (40, 6), (56, 18), (64, 30),
                 (10, 42), (50, 50), (20, 60), (42, 70)]:
        sp.px(x, y, WHITE)
    # Big saber, vertical, centered
    cx = 36
    # Hilt
    sp.rect(cx - 2, 70, cx + 2, 84, GRAY, outline=BLACK)
    sp.line(cx - 3, 74, cx + 3, 74, BLACK)
    sp.px(cx - 3, 78, BLACK); sp.px(cx + 3, 78, BLACK)
    # Pommel
    sp.rect(cx - 2, 84, cx + 2, 88, BLACK)
    # Blade — green core, white hot
    sp.rect(cx - 2, 14, cx + 2, 70, GREEN)
    sp.rect(cx - 1, 12, cx + 1, 70, WHITE)
    sp.px(cx, 10, WHITE)
    return sp


def starwars_v3(h):
    """Busy — two Johnnys dueling, droid in surf, twin moons, starfield."""
    sp = Sprite(72, 96, fill=DEEPBLUE)
    # Stars
    for x, y in [(4, 6), (10, 14), (20, 4), (32, 10), (44, 6), (56, 14),
                 (66, 8), (8, 26), (28, 22), (48, 26), (60, 30),
                 (16, 38), (40, 36), (52, 42)]:
        sp.px(x, y, WHITE)
    # Twin moons
    sp.ellipse(8, 4, 16, 12, YELLOW, outline=BLACK)
    sp.ellipse(56, 8, 64, 16, ORANGE, outline=BLACK)
    # Horizon, water, sand
    compose_horizon(sp, 60)
    for y in range(60, 72):
        sp.line(0, y, 71, y, DEEPBLUE)
    compose_sand_strip(sp, 72)
    # Two Jedi figures dueling in foreground
    for x, sab_color in [(14, GREEN), (52, RED)]:
        base_y = 92
        sp.rect(x - 4, base_y - 16, x + 4, base_y, TRUNK, outline=BLACK)
        sp.rect(x - 1, base_y - 21, x + 3, base_y - 17, SKIN)
        sp.px(x, base_y - 19, BLACK); sp.px(x + 2, base_y - 19, BLACK)
        sp.line(x - 1, base_y - 22, x + 3, base_y - 22, DGREEN)
        # Saber in hand — angled into center
        if x == 14:
            sp.line(x + 4, base_y - 14, x + 22, base_y - 30, sab_color)
            sp.line(x + 5, base_y - 14, x + 23, base_y - 30, sab_color)
            sp.line(x + 4, base_y - 15, x + 22, base_y - 31, WHITE)
        else:
            sp.line(x - 4, base_y - 14, x - 22, base_y - 30, sab_color)
            sp.line(x - 5, base_y - 14, x - 23, base_y - 30, sab_color)
            sp.line(x - 4, base_y - 15, x - 22, base_y - 31, WHITE)
    # Sabers crossing in the middle — small flash
    compose_star(sp, 36, 62, 3, WHITE)
    # Droid in the surf, small
    dx = 32
    sp.rect(dx, 66, dx + 4, 70, WHITE, outline=BLACK)
    sp.ellipse(dx, 63, dx + 4, 66, WHITE, outline=BLACK)
    sp.px(dx + 2, 64, RED)
    return sp


# ---------------------------------------------------------------------------
# id 18: Cinco de Mayo  (128×64, palette: red / green / yellow)
# Concept: Sombrero on Johnny, papel picado bunting between palms, maraca
# (coconut) in his hand.
# ---------------------------------------------------------------------------

def cincomayo_v1(h):
    """Literal — sombrero, bunting, coconut maraca."""
    sp = Sprite(128, 64, fill=SKY)
    compose_sand_strip(sp, 40)
    # Two palms
    compose_palm_tree(sp, 16, 40, trunk_h=28, frond_r=10)
    compose_palm_tree(sp, 112, 40, trunk_h=28, frond_r=10)
    # Bunting between palms — papel picado triangles
    bunt_y = 12
    for i, x in enumerate(range(20, 112, 8)):
        c = [RED, GREEN, YELLOW][i % 3]
        sp.line(x, bunt_y, x + 4, bunt_y + 8, c)
        sp.line(x + 4, bunt_y + 8, x + 8, bunt_y, c)
        sp.line(x, bunt_y, x + 8, bunt_y, c)
    # String
    sp.line(20, bunt_y, 112, bunt_y, BLACK)
    # Johnny center
    compose_johnny_simple(sp, 60, 50, hat_color=None, shirt_color=GREEN)
    # Sombrero — wide brim + crown
    sp.rect(54, 36, 70, 38, RED)  # brim
    sp.line(53, 37, 71, 37, RED)
    sp.rect(58, 32, 66, 36, RED)  # crown
    # Yellow band
    sp.line(58, 35, 66, 35, YELLOW)
    # Coconut maraca in his hand — to his right
    sp.ellipse(70, 46, 76, 52, TRUNK, outline=BLACK)
    sp.px(72, 48, BLACK); sp.px(74, 50, BLACK)  # texture
    sp.line(73, 52, 73, 56, TRUNK)  # handle
    return sp


def cincomayo_v2(h):
    """Minimalist — one giant sombrero centered."""
    sp = Sprite(128, 64, fill=SKY)
    compose_sand_strip(sp, 50)
    # Giant sombrero
    cx = 64
    # Brim
    sp.ellipse(cx - 40, 30, cx + 40, 46, RED, outline=BLACK)
    # Crown
    sp.ellipse(cx - 14, 12, cx + 14, 36, RED, outline=BLACK)
    # Yellow + green band
    sp.line(cx - 14, 30, cx + 14, 30, YELLOW)
    sp.line(cx - 14, 31, cx + 14, 31, YELLOW)
    sp.line(cx - 12, 32, cx + 12, 32, GREEN)
    # Pom-poms hanging from brim
    for x in [cx - 36, cx + 36]:
        sp.ellipse(x - 1, 46, x + 1, 50, YELLOW)
    return sp


def cincomayo_v3(h):
    """Busy fiesta — multiple sombreros, bunting, dancing Johnnys, maracas."""
    sp = Sprite(128, 64, fill=SKY)
    compose_sand_strip(sp, 40)
    # Three palms
    for x_anchor in [12, 64, 116]:
        compose_palm_tree(sp, x_anchor, 40, trunk_h=26, frond_r=8)
    # Bunting in two layers
    for layer, by in [(0, 8), (1, 18)]:
        for i, x in enumerate(range(8, 120, 6)):
            c = [RED, GREEN, YELLOW, WHITE][(i + layer) % 4]
            sp.line(x, by, x + 3, by + 5, c)
            sp.line(x + 3, by + 5, x + 6, by, c)
        sp.line(8, by, 120, by, BLACK)
    # Three dancing Johnnys with sombreros
    for i, x in enumerate([28, 60, 92]):
        compose_johnny_simple(sp, x, 56, hat_color=None,
                              shirt_color=[RED, GREEN, YELLOW][i])
        # Mini sombrero
        sp.rect(x - 2, 44, x + 7, 45, RED)
        sp.rect(x + 1, 41, x + 4, 44, RED)
        sp.px(x + 1, 43, YELLOW); sp.px(x + 4, 43, YELLOW)
        # Maraca
        sp.ellipse(x + 8, 50, x + 11, 53, TRUNK)
        sp.line(x + 9, 53, x + 9, 56, TRUNK)
    # Confetti
    for x, y, c in [(8, 28, RED), (24, 32, YELLOW), (40, 28, GREEN),
                    (56, 32, RED), (72, 28, YELLOW), (88, 32, GREEN),
                    (104, 28, RED), (120, 32, YELLOW)]:
        sp.px(x, y, c); sp.px(x + 1, y, c)
    return sp


# ---------------------------------------------------------------------------
# id 19: Mother's Day  (96×56, palette: pink / white / soft yellow)
# Concept: Bouquet of tropical flowers in a coconut vase; "MOM" drawn
# large in the sand with shells.
# ---------------------------------------------------------------------------

def mothersday_v1(h):
    """Literal — coconut vase with bouquet, MOM in shells on sand."""
    sp = Sprite(96, 56, fill=PINK)
    compose_sand_strip(sp, 24)
    # Coconut vase — left side
    sp.ellipse(8, 28, 28, 48, TRUNK, outline=BLACK)
    sp.line(8, 32, 28, 32, BLACK)  # rim
    # Bouquet — flowers above the vase
    flowers = [(14, 20, PINK), (18, 14, YELLOW), (22, 18, WHITE),
               (12, 18, RED), (24, 22, PINK), (16, 22, YELLOW)]
    for cx, cy, c in flowers:
        sp.ellipse(cx - 2, cy - 2, cx + 2, cy + 2, c, outline=BLACK)
        sp.px(cx, cy, YELLOW)  # center
    # Stems
    for cx, cy in [(14, 20), (18, 14), (22, 18), (12, 18), (24, 22), (16, 22)]:
        sp.line(cx, cy + 2, 18, 28, GREEN)
    # Leaves
    sp.ellipse(10, 24, 14, 28, GREEN)
    sp.ellipse(22, 24, 26, 28, GREEN)
    # "MOM" written in shells on the sand — letter shapes from white dots
    # M
    mx = 40
    for x, y in [(mx, 32), (mx, 36), (mx, 40), (mx, 44), (mx, 48),
                 (mx + 1, 34), (mx + 2, 36), (mx + 3, 38),
                 (mx + 4, 36), (mx + 5, 34),
                 (mx + 6, 32), (mx + 6, 36), (mx + 6, 40), (mx + 6, 44), (mx + 6, 48)]:
        sp.px(x, y, WHITE)
    # O
    ox = 52
    for x, y in [(ox + 1, 32), (ox + 2, 32), (ox + 3, 32), (ox + 4, 32),
                 (ox + 1, 48), (ox + 2, 48), (ox + 3, 48), (ox + 4, 48),
                 (ox, 34), (ox, 38), (ox, 42), (ox, 46),
                 (ox + 5, 34), (ox + 5, 38), (ox + 5, 42), (ox + 5, 46)]:
        sp.px(x, y, WHITE)
    # M (second)
    mx2 = 64
    for x, y in [(mx2, 32), (mx2, 36), (mx2, 40), (mx2, 44), (mx2, 48),
                 (mx2 + 1, 34), (mx2 + 2, 36), (mx2 + 3, 38),
                 (mx2 + 4, 36), (mx2 + 5, 34),
                 (mx2 + 6, 32), (mx2 + 6, 36), (mx2 + 6, 40),
                 (mx2 + 6, 44), (mx2 + 6, 48)]:
        sp.px(x, y, WHITE)
    # A few extra scattered shells
    for x, y in [(78, 36), (84, 44), (90, 32), (76, 50), (88, 50)]:
        sp.px(x, y, WHITE)
        sp.px(x + 1, y, WHITE)
    return sp


def mothersday_v2(h):
    """Minimalist — single big flower (hibiscus) silhouette."""
    sp = Sprite(96, 56, fill=PINK)
    compose_sand_strip(sp, 46)
    # Big stylized hibiscus, centered
    cx, cy = 48, 26
    # Five petals
    for ang_dx, ang_dy in [(-14, -8), (14, -8), (-16, 6), (16, 6), (0, -16)]:
        sp.ellipse(cx + ang_dx - 8, cy + ang_dy - 6,
                   cx + ang_dx + 8, cy + ang_dy + 6, WHITE, outline=BLACK)
    # Center
    sp.ellipse(cx - 5, cy - 5, cx + 5, cy + 5, YELLOW, outline=BLACK)
    sp.px(cx, cy, RED)
    # Stamen lines
    for dx in [-2, 0, 2]:
        sp.line(cx + dx, cy, cx + dx, cy - 8, BLACK)
    return sp


def mothersday_v3(h):
    """Busy — full beach scene: bouquet, MOM in shells, palm, flowers, Johnny."""
    sp = Sprite(96, 56, fill=PINK)
    compose_sand_strip(sp, 22)
    compose_palm_tree(sp, 84, 22, trunk_h=18, frond_r=8)
    # Coconut vase + bouquet on left
    sp.ellipse(4, 24, 18, 36, TRUNK, outline=BLACK)
    sp.line(4, 27, 18, 27, BLACK)
    flowers = [(8, 14, WHITE), (12, 8, YELLOW), (16, 14, PINK),
               (10, 18, RED), (14, 20, YELLOW)]
    for cx, cy, c in flowers:
        sp.ellipse(cx - 2, cy - 2, cx + 2, cy + 2, c, outline=BLACK)
        sp.px(cx, cy, RED)
    for cx, cy in [(8, 14), (12, 8), (16, 14), (10, 18), (14, 20)]:
        sp.line(cx, cy + 2, 11, 24, GREEN)
    # Johnny right of vase, holding a flower
    compose_johnny_simple(sp, 28, 50, hat_color=None, shirt_color=PINK)
    sp.px(35, 41, RED); sp.px(35, 42, GREEN)
    # MOM in shells, smaller
    mx = 44
    for x, y in [(mx, 32), (mx, 36), (mx, 40),
                 (mx + 1, 34), (mx + 2, 36), (mx + 3, 34),
                 (mx + 4, 32), (mx + 4, 36), (mx + 4, 40)]:
        sp.px(x, y, WHITE)
    ox = 52
    for x, y in [(ox + 1, 32), (ox + 2, 32), (ox + 3, 32),
                 (ox + 1, 40), (ox + 2, 40), (ox + 3, 40),
                 (ox, 34), (ox, 36), (ox, 38),
                 (ox + 4, 34), (ox + 4, 36), (ox + 4, 38)]:
        sp.px(x, y, WHITE)
    mx2 = 60
    for x, y in [(mx2, 32), (mx2, 36), (mx2, 40),
                 (mx2 + 1, 34), (mx2 + 2, 36), (mx2 + 3, 34),
                 (mx2 + 4, 32), (mx2 + 4, 36), (mx2 + 4, 40)]:
        sp.px(x, y, WHITE)
    # Shells scattered
    for x, y in [(70, 38), (76, 46), (82, 40), (90, 48)]:
        sp.px(x, y, WHITE); sp.px(x + 1, y, WHITE)
    # Heart in the sky
    compose_heart(sp, 70, 8, 4, RED)
    return sp


# ---------------------------------------------------------------------------
# Batch 2 RENDERERS dict
# ---------------------------------------------------------------------------
RENDERERS_BATCH2 = {
    14: (aprilfool_v1, aprilfool_v2, aprilfool_v3),
    15: (easter_v1,    easter_v2,    easter_v3),
    16: (earthday_v1,  earthday_v2,  earthday_v3),
    17: (starwars_v1,  starwars_v2,  starwars_v3),
    18: (cincomayo_v1, cincomayo_v2, cincomayo_v3),
    19: (mothersday_v1, mothersday_v2, mothersday_v3),
}
