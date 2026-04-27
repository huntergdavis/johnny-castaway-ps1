# Holiday Emblem Style Guide

> 🌐 **Rendered version:** **[/docs/holidays/](https://hunterdavis.com/Johnny-Castaway-PS1/docs/holidays/)** — this doc rendered on the project website with cross-links and prose context. The GitHub copy here is the source.


The added holiday art should look like small Johnny Castaway overlay props,
not full scenes.

## Target

- Simple, emblematic, family-friendly icons.
- About 20-30 active pixels across inside a 32x32 transparent frame.
- Heavy 1px dark outlines where they improve readability.
- Saturated but limited colors from the shared 16-color CLUT.
- Clear silhouettes that read at PS1 viewing distance.

## Hard Rules

- Use `Sprite(32, 32, fill=TRANSPARENT)`.
- Leave all unused pixels as palette index 0.
- Do not draw sky, ocean, sand strips, island bases, or Johnny.
- Do not reskin the palm tree.
- Do not use per-image palettes; all output must use `PALETTE` from
  `scripts/holidays_art_lib.py`.
- IDs 1-4 are original sprites and stay untouched.

## Good Emblem Examples

- Valentine's Day: carved heart on a small trunk sliver plus tiny hearts.
- Easter: decorated eggs.
- Star Wars Day: lightsaber prop.
- 4/20 Day: stylized green leaf with a peace-sign medallion.
- Pi Day: tiny chalkboard with pi.
- Memorial Day: half-mast flag.
- Pride Day: small rainbow flag.
- Thanksgiving: small cornucopia.

## Output

Run:

```bash
./scripts/holidays-build-all.sh --clean
```

Review:

```text
scratch/holidays-emblems/holiday-emblems-preview.png
docs/ps1/holidays-emblems/holiday-emblems-preview.png
```

Use:

```text
scratch/holidays-emblems/holiday-emblems-sheet.png
scratch/holidays-emblems/review.html
scratch/holidays-emblems/manifest.json
docs/ps1/holidays-emblems/holiday-emblems-sheet.png
docs/ps1/holidays-emblems/manifest.json
```
