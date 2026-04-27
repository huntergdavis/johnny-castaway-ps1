# Holiday Art Implementation Plan

> 🌐 **Rendered version:** **[/docs/holidays/](https://hunterdavis.com/johnny-castaway-ps1/docs/holidays/)** — this doc rendered on the project website with cross-links and prose context. The GitHub copy here is the source.


This document supersedes the earlier five-variant concept-art plan.

## Current Scope

Build a simple sprite sheet containing one 32x32 transparent emblem for each
added holiday. The original four holiday sprites remain untouched.

## Active Files

- `holidays.yml` keeps the holiday IDs, names, date rules, and island anchors.
- `scripts/holidays_art_lib.py` provides the shared 16-color CLUT and indexed
  `Sprite` helper.
- `scripts/holidays-emblem-sheet.py` draws the 31 small emblems and writes the
  sheet, preview, individual PNGs, and manifest.
- `scripts/holidays-build-all.sh` is the one-command wrapper for the emblem
  sheet.

## Removed Scope

- No five-variant review pipeline.
- No automatic night pass.
- No HTML picker or picks JSON.
- No final-review page.
- No concept-scene renderers.
- No red-team suite.

## Build

```bash
./scripts/holidays-build-all.sh --clean
```

## Outputs

```text
scratch/holidays-emblems/holiday-emblems-sheet.png
scratch/holidays-emblems/holiday-emblems-preview.png
scratch/holidays-emblems/review.html
scratch/holidays-emblems/manifest.json
scratch/holidays-emblems/<id>-<short>.png
```

## Verification

The useful verification is visual review of
`scratch/holidays-emblems/holiday-emblems-preview.png` plus a quick script run
to confirm the generator completes. The old red-team checks were deleted
because they validated the wrong artifact shape.
