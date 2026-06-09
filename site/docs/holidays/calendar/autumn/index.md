---
layout: page
title: "First Day of Autumn"
eyebrow: "Holiday 31"
subtitle: "Autumnal equinox"
description: "First Day of Autumn: Autumnal equinox. Autumn leaf and acorn."
---

## When it falls

Autumnal equinox (around September 22) — fixed canonical date, ±1 day tolerance.

Worked dates:

| Year | Date |
|------|------|
| 2026 | September 22 |
| 2030 | September 22 |
| 2050 | September 22 |

## What it shows on the island

From [`holidays.yml`]({{ site.github_url }}/blob/main/holidays.yml):

> Autumn leaf and acorn.

## Sprite

Dimensions: **32 × 32** pixels. Anchored at island position **(404, 284)**.

Added by this port via the [holiday expansion]({{ '/docs/holidays/' | relative_url }}). `existing_sprite: null`. The visual is a 32x32 transparent emblem authored by an AI sub-agent in the shared 16-color CLUT defined in `scripts/holidays_art_lib.py`. It overlays the island corner during the holiday's window rather than reskinning the scene.

## Palette

<span style="display:inline-block;width:1em;height:1em;background:#C8531A;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#C8531A</code>, <span style="display:inline-block;width:1em;height:1em;background:#C89A2A;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#C89A2A</code>, <span style="display:inline-block;width:1em;height:1em;background:#8A3A1A;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#8A3A1A</code>

Three-color hint from `holidays.yml`. This is a guide for the codegen step and the emblem authoring pass; the actual emblem samples from the shared 16-color CLUT, and the original Sierra sprites use the original game palette directly.

## Algorithm

Returned by `holidayVernalEquinox` / `holidayAutumnalEquinox` as a fixed canonical date. See [Date-algorithm core]({{ '/docs/holidays/algorithm/' | relative_url }}#solstice-and-equinox).

## Emblem

This holiday has an emblem on the shared sheet at cell 27, pixel offset (96, 96). View the packed sheet on GitHub: [`docs/ps1/holidays-emblems/holiday-emblems-sheet.png`]({{ site.github_url }}/blob/main/docs/ps1/holidays-emblems/holiday-emblems-sheet.png). Full index: [Emblem gallery]({{ '/docs/holidays/emblems/' | relative_url }}).

## Source

- Row in [`holidays.yml`]({{ site.github_url }}/blob/main/holidays.yml) (search for `id: 31` or `short_name: "AUTUMN"`).
- Generated row in [`src/scene/holidays_table.c`]({{ site.github_url }}/blob/main/src/scene/holidays_table.c).
