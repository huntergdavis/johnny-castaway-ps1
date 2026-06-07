---
layout: page
title: "Mardi Gras"
eyebrow: "Holiday 11"
subtitle: "47 days before Easter"
description: "Mardi Gras: 47 days before Easter. Mardi Gras mask and beads."
---

## When it falls

47 days before Easter Sunday — anchored to the Meeus / Jones / Butcher Easter computation.

Worked dates:

| Year | Date |
|------|------|
| 2026 | February 17 |
| 2030 | March 5 |
| 2050 | February 22 |

For reference, Easter Sunday in those years falls on April 5 (2026), April 21 (2030), and April 10 (2050). The offset from `holidays.yml` is `-47`.

## What it shows on the island

From [`holidays.yml`]({{ site.github_url }}/blob/main/holidays.yml):

> Mardi Gras mask and beads.

## Sprite

Dimensions: **32 × 32** pixels. Anchored at island position **(404, 284)**.

Added by this port via the [holiday expansion]({{ '/docs/holidays/' | relative_url }}). `existing_sprite: null`. The visual is a 32x32 transparent emblem authored by an AI sub-agent in the shared 16-color CLUT defined in `scripts/holidays_art_lib.py`. It overlays the island corner during the holiday's window rather than reskinning the scene.

## Palette

<span style="display:inline-block;width:1em;height:1em;background:#6A1B9A;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#6A1B9A</code>, <span style="display:inline-block;width:1em;height:1em;background:#1F8A3A;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#1F8A3A</code>, <span style="display:inline-block;width:1em;height:1em;background:#D4AF37;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#D4AF37</code>

Three-color hint from `holidays.yml`. This is a guide for the codegen step and the emblem authoring pass; the actual emblem samples from the shared 16-color CLUT, and the original Sierra sprites use the original game palette directly.

## Algorithm

Computed by `holidayEasterSunday(year, ...)` (Meeus / Jones / Butcher) followed by `holidayDateAdd` for the offset. See [Date-algorithm core]({{ '/docs/holidays/algorithm/' | relative_url }}#easter-sunday--meeus--jones--butcher).

## Emblem

This holiday has an emblem on the shared sheet at cell 6, pixel offset (192, 0). View the packed sheet on GitHub: [`docs/ps1/holidays-emblems/holiday-emblems-sheet.png`]({{ site.github_url }}/blob/main/docs/ps1/holidays-emblems/holiday-emblems-sheet.png). Full index: [Emblem gallery]({{ '/docs/holidays/emblems/' | relative_url }}).

## Source

- Row in [`holidays.yml`]({{ site.github_url }}/blob/main/holidays.yml) (search for `id: 11` or `short_name: "MARDI GRAS"`).
- Generated row in [`src/scene/holidays_table.c`]({{ site.github_url }}/blob/main/src/scene/holidays_table.c).
