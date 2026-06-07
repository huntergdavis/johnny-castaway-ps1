---
layout: page
title: "Easter"
eyebrow: "Holiday 15"
subtitle: "Easter Sunday"
description: "Easter: Easter Sunday. Decorated Easter eggs."
---

## When it falls

Easter Sunday — computed via the Meeus / Jones / Butcher Gregorian algorithm.

Worked dates:

| Year | Date |
|------|------|
| 2026 | April 5 |
| 2030 | April 21 |
| 2050 | April 10 |

For reference, Easter Sunday in those years falls on April 5 (2026), April 21 (2030), and April 10 (2050). The offset from `holidays.yml` is `0`.

## What it shows on the island

From [`holidays.yml`]({{ site.github_url }}/blob/main/holidays.yml):

> Decorated Easter eggs.

## Sprite

Dimensions: **32 × 32** pixels. Anchored at island position **(404, 284)**.

Added by this port via the [holiday expansion]({{ '/docs/holidays/' | relative_url }}). `existing_sprite: null`. The visual is a 32x32 transparent emblem authored by an AI sub-agent in the shared 16-color CLUT defined in `scripts/holidays_art_lib.py`. It overlays the island corner during the holiday's window rather than reskinning the scene.

## Palette

<span style="display:inline-block;width:1em;height:1em;background:#C8B6FF;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#C8B6FF</code>, <span style="display:inline-block;width:1em;height:1em;background:#B6FFC8;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#B6FFC8</code>, <span style="display:inline-block;width:1em;height:1em;background:#FFF6B6;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#FFF6B6</code>

Three-color hint from `holidays.yml`. This is a guide for the codegen step and the emblem authoring pass; the actual emblem samples from the shared 16-color CLUT, and the original Sierra sprites use the original game palette directly.

## Algorithm

Computed by `holidayEasterSunday(year, ...)` (Meeus / Jones / Butcher) followed by `holidayDateAdd` for the offset. See [Date-algorithm core]({{ '/docs/holidays/algorithm/' | relative_url }}#easter-sunday--meeus--jones--butcher).

## Emblem

This holiday has an emblem on the shared sheet at cell 11, pixel offset (96, 32). View the packed sheet on GitHub: [`docs/ps1/holidays-emblems/holiday-emblems-sheet.png`]({{ site.github_url }}/blob/main/docs/ps1/holidays-emblems/holiday-emblems-sheet.png). Full index: [Emblem gallery]({{ '/docs/holidays/emblems/' | relative_url }}).

## Source

- Row in [`holidays.yml`]({{ site.github_url }}/blob/main/holidays.yml) (search for `id: 15` or `short_name: "EASTER"`).
- Generated row in [`src/scene/holidays_table.c`]({{ site.github_url }}/blob/main/src/scene/holidays_table.c).
