---
layout: page
title: "First Day of Spring"
eyebrow: "Holiday 13"
subtitle: "Vernal equinox"
description: "First Day of Spring: Vernal equinox. Blossom branch with butterfly."
---

## When it falls

Vernal equinox (around March 20) — fixed canonical date, ±1 day tolerance.

Worked dates:

| Year | Date |
|------|------|
| 2026 | March 20 |
| 2030 | March 20 |
| 2050 | March 20 |

## What it shows on the island

From [`holidays.yml`]({{ site.github_url }}/blob/main/holidays.yml):

> Blossom branch with butterfly.

## Sprite

Dimensions: **32 × 32** pixels. Anchored at island position **(404, 284)**.

Added by this port via the [holiday expansion]({{ '/docs/holidays/' | relative_url }}). `existing_sprite: null`. The visual is a 32x32 transparent emblem authored by an AI sub-agent in the shared 16-color CLUT defined in `scripts/holidays_art_lib.py`. It overlays the island corner during the holiday's window rather than reskinning the scene.

## Palette

<span style="display:inline-block;width:1em;height:1em;background:#FFC9D9;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#FFC9D9</code>, <span style="display:inline-block;width:1em;height:1em;background:#B8E0A8;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#B8E0A8</code>, <span style="display:inline-block;width:1em;height:1em;background:#9AD0E8;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#9AD0E8</code>

Three-color hint from `holidays.yml`. This is a guide for the codegen step and the emblem authoring pass; the actual emblem samples from the shared 16-color CLUT, and the original Sierra sprites use the original game palette directly.

## Algorithm

Returned by `holidayVernalEquinox` / `holidayAutumnalEquinox` as a fixed canonical date. See [Date-algorithm core]({{ '/docs/holidays/algorithm/' | relative_url }}#solstice-and-equinox).

## Emblem

This holiday has an emblem on the shared sheet at cell 8, pixel offset (0, 32). View the packed sheet on GitHub: [`docs/ps1/holidays-emblems/holiday-emblems-sheet.png`]({{ site.github_url }}/blob/main/docs/ps1/holidays-emblems/holiday-emblems-sheet.png). Full index: [Emblem gallery]({{ '/docs/holidays/emblems/' | relative_url }}).

## Source

- Row in [`holidays.yml`]({{ site.github_url }}/blob/main/holidays.yml) (search for `id: 13` or `short_name: "SPRING"`).
- Generated row in [`src/scene/holidays_table.c`]({{ site.github_url }}/blob/main/src/scene/holidays_table.c).
