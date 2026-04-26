# Holiday Sprite Style Guide

Visual reference for generating the 31 new holiday sprites that supplement
the original 4 (Halloween / St Patrick / Christmas / New Year) baked into
`HOLIDAY.PSB`.

## Goals

- **Recognizable at PS1 viewing distance** — small details matter less than
  silhouette and palette.
- **Consistent with the original 4** — same visual register: heavy outlines,
  saturated-but-not-neon colors, compact compositions anchored to specific
  island coordinates.
- **Variable per-holiday dimensions** — sized to fit the concept, between
  ~32×32 and ~152×80. Don't waste pixels on whitespace.
- **16-color shared CLUT** — every sprite picks from the same palette
  defined in `scripts/holidays_art_lib.py`. No per-frame CLUTs in this
  iteration (keeps the asset pipeline simple).

## Original-4 reference

| ID  | Holiday      | Size   | Island anchor | Visual                           |
|-----|--------------|--------|---------------|----------------------------------|
|  1  | Halloween    |  40×34 | (410, 298)    | jack-o'-lantern next to palm     |
|  2  | St Patrick's | 120×47 | (333, 286)    | leprechaun + pot of gold + bow   |
|  3  | Christmas    |  56×65 | (404, 267)    | snowman + tree on island         |
|  4  | New Year     | 152×47 | (361, 155)    | banner + balloons + confetti     |

These are preserved exactly. The 31 new holidays should sit alongside
without clashing — same color saturation, similar silhouette weight.

## Shared 16-color CLUT

Defined in `scripts/holidays_art_lib.py` as `PALETTE`:

```
 0 transparent          8 deep blue / night
 1 white                9 sand / gold
 2 black (outlines)    10 red / cardinal
 3 light skin          11 sunshine yellow
 4 dark skin / trunk   12 orange / pumpkin
 5 palm green          13 pink
 6 dark green          14 purple
 7 sky blue / water    15 gray
```

Each holiday's `palette` field in `holidays.yml` is a 3-color hint from
the design doc — those are the FOCUS colors that should dominate the
composition. Pick the closest CLUT indices.

## Composition pattern

Every holiday sprite typically combines:

1. **Background fill** — sky color (often blue, but can be sunset orange,
   night-purple, etc. depending on holiday). Index 0 = transparent for
   areas where the underlying island should show through.
2. **Sand strip / island base** — bottom portion of the sprite, sand-colored
   (index 9). Optional — small holidays may have no base.
3. **Palm tree** (optional) — `compose_palm_tree(sprite, x, y)` from
   `holidays_art_lib`. Most "Johnny on the island" holidays include one.
4. **Johnny silhouette** (optional) — `compose_johnny_simple(sprite, x, y,
   hat_color, shirt_color)`. Tiny 6×12 px figure. Costume changes via
   `hat_color` and `shirt_color`.
5. **Holiday-specific decoration** — the actual differentiator. A pumpkin,
   a heart, a top hat, a podium, a giant pi symbol, etc. Use the library
   primitives (`compose_star`, `compose_heart`, `compose_outlined_rect`,
   etc.) plus per-pixel art for unique elements.
6. **Outline** — heavy 1-px black or dark outlines around major shapes
   for legibility.

## Per-holiday data

The renderer reads each holiday from `holidays.yml`:

```yaml
- id: 6
  name: "Valentine's Day"
  short_name: "VALENTINE"
  description: "Heart carved into the palm trunk, rose in Johnny's teeth, pink heart-shaped clouds."
  date_rule: { kind: fixed, month: 2, day: 14 }
  sprite:
    width: 56
    height: 48
    island_x: 360
    island_y: 280
  palette: ["#FFC0CB", "#DC143C", "#FFFFFF"]
  existing_sprite: null
```

The renderer's job:
- Output a 56×48 indexed-mode PNG
- Use pink + red + white from the palette CLUT
- Express the concept: heart on palm, pink heart clouds, etc.

## Variants

Each holiday gets **3 variants** for owner review. The variants should
explore different compositions:

- **Variant 1 — literal**: closest reading of the design doc concept.
- **Variant 2 — minimalist**: stripped-down silhouette / icon-style
  rendering. Good for small sprites.
- **Variant 3 — busy/scenic**: more elements, full island scene if
  dimensions permit. Good for large sprites.

Owner picks one (or none → iterate) per holiday during HTML review.

## Output layout

```
scratch/holidays-art/
  <id>-<short_name>-v1.png
  <id>-<short_name>-v2.png
  <id>-<short_name>-v3.png
  ...
```

Filename example: `06-VALENTINE-v1.png`.

Reviewer-facing HTML at `scratch/holidays-preview.html` shows all
variants side-by-side per holiday.

## Implementation contract

- Implementation file: `scripts/holidays_concepts.py`
- Public dict: `RENDERERS = { id: [variant1_fn, variant2_fn, variant3_fn], ... }`
- Each `variant_fn` takes a single arg (the holiday entry from
  `holidays.yml` parsed dict) and returns a `Sprite` instance.
- Runner `scripts/holidays-generate-art.py` invokes each renderer and
  writes the PNG.

## Out of scope (this iteration)

- Animated icons (e.g. multi-frame Mardi Gras parade) — single static
  frame per holiday.
- Per-frame CLUTs — shared 16-color palette across all sprites.
- Real Sierra-style hand-drawn pixel art — current generation aims for
  RECOGNIZABLE not MASTERPIECE. Iterate later.
