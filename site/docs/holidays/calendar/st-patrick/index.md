---
layout: page
title: "St. Patrick's Day"
eyebrow: "Holiday 2"
subtitle: "March 17 — fixed date"
description: "St. Patrick's Day: March 17 — fixed date. Johnny in green leprechaun hat, shamrocks sprouting around the palm, rainbow over the horizon."
---

## When it falls

March 17 — fixed date.

Worked dates:

| Year | Date |
|------|------|
| 2026 | March 17 |
| 2030 | March 17 |
| 2050 | March 17 |

## What it shows on the island

From [`holidays.yml`]({{ site.github_url }}/blob/main/holidays.yml):

> Johnny in green leprechaun hat, shamrocks sprouting around the palm, rainbow over the horizon.

## Sprite

Dimensions: **120 × 47** pixels. Anchored at island position **(333, 286)**.

This is one of the four **original Sierra sprites** preserved from the 1992 game. `existing_sprite` index `1`. The asset is the original full-island scene — Johnny appears in costume, the palm or message bottle is reskinned. The four originals are New Year's Day, St. Patrick's Day, Halloween, and Christmas; together they pin holiday IDs 1–4. See [the holidays landing]({{ '/docs/holidays/' | relative_url }}) for the full 36-entry list including the 32 added by this port.

## Palette

<span style="display:inline-block;width:1em;height:1em;background:#1F7A3A;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#1F7A3A</code>, <span style="display:inline-block;width:1em;height:1em;background:#D4AF37;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#D4AF37</code>, <span style="display:inline-block;width:1em;height:1em;background:#FFFFFF;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#FFFFFF</code>

Three-color hint from `holidays.yml`. This is a guide for the codegen step and the emblem authoring pass; the actual emblem samples from the shared 16-color CLUT, and the original Sierra sprites use the original game palette directly.

## Algorithm

Fixed date — no computation; the `HOLIDAY_KIND_FIXED` dispatch in `holidayForDate` compares `month` and `day` directly. See [Date-algorithm core]({{ '/docs/holidays/algorithm/' | relative_url }}).

## Emblem

No 32x32 emblem entry; the visual for this holiday is the original Sierra full-island sprite, not a sheet emblem.

## Source

- Row in [`holidays.yml`]({{ site.github_url }}/blob/main/holidays.yml) (search for `id: 2` or `short_name: "ST PATRICK"`).
- Generated row in [`src/scene/holidays_table.c`]({{ site.github_url }}/blob/main/src/scene/holidays_table.c).
- Original Sierra sprite (1992 game). Sprite frame index `1`.
