---
layout: page
title: "Halloween"
eyebrow: "Holiday 1"
subtitle: "October 31 — fixed date"
description: "Halloween: October 31 — fixed date. Johnny in witch hat, jack-o'-lantern beside the palm, bats silhouetted against a full moon."
---

## When it falls

October 31 — fixed date.

Worked dates:

| Year | Date |
|------|------|
| 2026 | October 31 |
| 2030 | October 31 |
| 2050 | October 31 |

## What it shows on the island

From [`holidays.yml`]({{ site.github_url }}/blob/main/holidays.yml):

> Johnny in witch hat, jack-o'-lantern beside the palm, bats silhouetted against a full moon.

## Sprite

Dimensions: **40 × 34** pixels. Anchored at island position **(410, 298)**.

This is one of the four **original Sierra sprites** preserved from the 1992 game. `existing_sprite` index `0`. The asset is the original full-island scene — Johnny appears in costume, the palm or message bottle is reskinned. The four originals are New Year's Day, St. Patrick's Day, Halloween, and Christmas; together they pin holiday IDs 1–4. See [the holidays landing]({{ '/docs/holidays/' | relative_url }}) for the full 36-entry list including the 32 added by this port.

## Palette

<span style="display:inline-block;width:1em;height:1em;background:#FF7A1A;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#FF7A1A</code>, <span style="display:inline-block;width:1em;height:1em;background:#000000;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#000000</code>, <span style="display:inline-block;width:1em;height:1em;background:#F0F0F0;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#F0F0F0</code>

Three-color hint from `holidays.yml`. This is a guide for the codegen step and the emblem authoring pass; the actual emblem samples from the shared 16-color CLUT, and the original Sierra sprites use the original game palette directly.

## Algorithm

Fixed date — no computation; the `HOLIDAY_KIND_FIXED` dispatch in `holidayForDate` compares `month` and `day` directly. See [Date-algorithm core]({{ '/docs/holidays/algorithm/' | relative_url }}).

## Emblem

No 32x32 emblem entry; the visual for this holiday is the original Sierra full-island sprite, not a sheet emblem.

## Source

- Row in [`holidays.yml`]({{ site.github_url }}/blob/main/holidays.yml) (search for `id: 1` or `short_name: "HALLOWEEN"`).
- Generated row in [`src/scene/holidays_table.c`]({{ site.github_url }}/blob/main/src/scene/holidays_table.c).
- Original Sierra sprite (1992 game). Sprite frame index `0`.
