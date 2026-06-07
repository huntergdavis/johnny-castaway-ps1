---
layout: page
title: 35 holidays in 4 weeks
eyebrow: Lab · Codegen study
subtitle: A small YAML file, a calendar engine, and a sprite sheet full of tiny props.
description: A codegen retrospective on expanding Johnny Castaway PS1 from four original holiday overlays to a 36-holiday calendar with generated tables and emblem sprites.
date: 2026-04-26
image: /assets/img/help/menu/holidays.png
image_alt: The Holidays submenu inside the in-game pause menu — the runtime surface the codegen pipeline produces, with toggleable overlays for each of the 36 calendar holidays. Captured on PS1 hardware via the validation harness.
image_width: 640
image_height: 448
---

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

## Four was not enough

The original screensaver had four holiday decorations: Halloween, St.
Patrick's Day, Christmas, and New Year. They are charming because they are
small. A pumpkin. A tree. A banner. A few clovers. Not a reskin. Not a
special mode. Just the island noticing the calendar.

The PS1 port keeps those four exactly as they were. Then it adds more.

The final table at this release has 36 holidays, including 4/20 Day because
the user asked for it directly and it made the sheet better. Fixed dates,
movable Sundays, Nth weekday rules, Easter-relative holidays, solstices,
Election Day. A tiny calendar engine hiding behind a tiny beach.

## The source of truth

The source file is `holidays.yml`.

That file names each holiday, its short label, date rule, sprite dimensions,
anchor, palette hints, and description. Codegen turns it into
[`src/scene/holidays_table.c`]({{ site.github_url }}/blob/main/src/scene/holidays_table.c). Tests make sure the original four IDs do not move,
date rules resolve correctly, and duplicate IDs do not sneak in.

That is the right shape for this problem. Calendar rules belong in data.
Runtime lookup belongs in C. The generated file should be boring.

## The sprite-sheet correction

The first AI pass got the problem wrong. It drew scenes. Palm trees, sand,
sky, Johnny. Pretty little concepts, totally wrong for the runtime.

The actual runtime composites an overlay sprite on top of the existing island.
So the art had to be transparent-background props: eggs, lightsaber, tiny
chalkboard, football, medal, peace medallion, hibiscus, ballot box. 32x32
cells in a shared 16-color CLUT. Index 0 transparent. No Johnny. No island.
No sand.

Once that constraint was enforced, the sheet snapped into focus.

## Algorithms over tables

The important decision was not to ship a list of dates.

Easter uses the Meeus / Jones / Butcher algorithm. Nth weekday holidays use a
small date helper. Election Day follows the "Tuesday after the first Monday in
November" rule. Fixed holidays compare month and day. The runtime can answer
future dates without waiting for me to update a table in 2030.

This is overkill in the best way. The visual output is a 32-pixel watermelon.
The date engine under it is correct.

## What codegen bought

It bought fearlessness. Adding a holiday became:

1. Add a row to YAML.
2. Add or adjust the emblem.
3. Regenerate.
4. Run tests.
5. Review in the pause menu date picker.

No hand-edited C table. No stale title string. No date rule duplicated in a
second file. The generated page set under [/docs/holidays/]({{ '/docs/holidays/' | relative_url }})
is part of the same bargain: the data produces the runtime table and the
documentation.

## Cross-links

- [Holidays]({{ '/docs/holidays/' | relative_url }})
- [Date-algorithm core]({{ '/docs/holidays/algorithm/' | relative_url }})
- [Holiday emblem gallery]({{ '/docs/holidays/emblems/' | relative_url }})
- [AI sub-agents on this project]({{ '/docs/agents/' | relative_url }})
  — the sprite primitives that compose into the emblem sheet
  were drafted by an LLM sub-agent; that page records the
  human-edit pass that reviewed them.
- [The LLM pass]({{ '/lab/llm-pass/' | relative_url }})
  — the methodology essay whose loop this codegen story is one
  case study of.
- [4/20 Day]({{ '/docs/holidays/calendar/420-day/' | relative_url }})
