# Holiday emblem sheet pipeline

> 🌐 **Rendered version:** **[/docs/holidays/](https://hunterdavis.com/johnny-castaway-ps1/docs/holidays/)** — this doc rendered on the project website with cross-links and prose context. The GitHub copy here is the source.


The holiday art pipeline is now deliberately small: one transparent 32x32
emblem for each added holiday, packed into a single sprite sheet.

The previous five-variant scene pipeline, HTML picker, final-review page,
contact sheet, automatic night pass, and red-team suite were removed because
they optimized for full island scenes instead of the small overlay props the
PS1 port needs.

## Command

```bash
cd /home/hunter/workspace/jc_ps1_sandbox
./scripts/holidays-build-all.sh --clean
```

That runs:

```bash
python3 scripts/holidays-emblem-sheet.py
```

## Outputs

```text
scratch/holidays-emblems/
  05-ELVIS_BDAY.png
  06-MLK_DAY.png
  ...
  35-THANKSGIVE.png
  36-420_DAY.png
  holiday-emblems-sheet.png
  holiday-emblems-preview.png
  review.html
  manifest.json
```

`holiday-emblems-sheet.png` is the transparent packed sheet. The sheet uses
8 columns of 32x32 cells. `holiday-emblems-preview.png` is only a human review
image with checkerboard cells and labels. `review.html` displays the sheet and
all individual icons at review scale.

Tracked review copies live in:

```text
docs/ps1/holidays-emblems/holiday-emblems-sheet.png
docs/ps1/holidays-emblems/holiday-emblems-preview.png
docs/ps1/holidays-emblems/manifest.json
```

## Contract

- Added holidays only. IDs 1-4 keep the original PS1 sprites.
- One emblem per holiday.
- Each emblem is 32x32 pixels.
- Palette is the shared 16-color CLUT in `scripts/holidays_art_lib.py`.
- Palette index 0 is transparent.
- No Johnny, no palm reskins, no sand/sky scene backgrounds.
- The emblems should read as simple, family-friendly holiday props.

## Renderer Location

All emblem renderers live in:

```text
scripts/holidays-emblem-sheet.py
```

Each renderer returns a `Sprite(32, 32, fill=TRANSPARENT)`. The script saves
both individual PNGs and the packed sheet.
