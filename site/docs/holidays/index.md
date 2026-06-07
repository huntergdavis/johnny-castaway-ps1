---
layout: page
title: Holidays
eyebrow: Reference
subtitle: From four shipped holidays to thirty-six. Date algorithms, codegen, and the emblem sheet.
description: How the Johnny Castaway PS1 port grew from four holidays to thirty-six — Meeus / Zeller / Nth-weekday algorithms, the holidays.yml codegen pipeline, and the 32 emblem sprites.
---

A labor of love by Hunter Davis. The original Sierra screensaver shipped
four holidays — New Year's, St. Patrick's, Halloween, Christmas. The PS1
port keeps those four sprites untouched and adds **thirty-two more**, for
**thirty-six total**, distributed across the calendar so every month has
at least one. The added holidays are 32x32 transparent emblem sprites, not
full island reskins — small overlay props that honor each day without
re-drawing Johnny.

If you paid for this, you were cheated. Open source and free.

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

## Distribution at a glance

| Month | Count | Examples |
|-------|------:|----------|
| Jan   | 3     | New Year's, MLK Day, Elvis's Birthday |
| Feb   | 5     | Groundhog Day, Super Bowl, Valentine's, Presidents' Day, Mardi Gras |
| Mar   | 3     | Pi Day, **St. Patrick's** *(original)*, First Day of Spring |
| Apr   | 4     | April Fool's, 4/20, Easter, Earth Day |
| May   | 4     | Star Wars Day, Cinco de Mayo, Mother's Day, Memorial Day |
| Jun   | 3     | Father's Day, First Day of Summer, Pride Day |
| Jul   | 2     | Independence Day, Moon Landing Day |
| Aug   | 3     | National Watermelon Day, Left-Handers Day, Hawaii Statehood |
| Sep   | 3     | Labor Day, Talk Like a Pirate Day, First Day of Autumn |
| Oct   | 2     | Indigenous Peoples' Day, **Halloween** *(original)* |
| Nov   | 3     | Election Day, Veterans Day, Thanksgiving |
| Dec   | 1     | **Christmas** *(original)* |
| **Total** | **36** | Original four marked in bold. |

The four original holidays keep their original full-scene sprites and IDs
(1–4). The thirty-two new IDs (5–36) are 32x32 transparent emblems —
[`docs/ps1/holidays-emblems/holiday-emblems-sheet.png`]({{ site.github_url }}/blob/main/docs/ps1/holidays-emblems/holiday-emblems-sheet.png) is the
packed sprite sheet, eight columns of 32x32 cells.

## The date-algorithm core

The PS1 has no internet, no Network Time Protocol, no licensed calendar
library. To make a holiday like Easter "just work" in 1995 *and* 2095, the
runtime computes movable feasts directly. The whole algorithm core lives in
[`src/scene/holidays.c`]({{ site.github_url }}/blob/main/src/scene/holidays.c) and
exposes pure functions — no allocation, no PS1 dependencies, year range
**1583 through 4099**:

```c
int  holidayDayOfWeek      (int year, int month, int day);
int  holidayNthWeekdayOfMonth(int n, int weekday, int month, int year);
void holidayEasterSunday   (int year, int *out_month, int *out_day);
void holidayDateAdd        (int y, int m, int d, int delta_days,
                            int *out_y, int *out_m, int *out_d);
void holidayWinterSolstice (int year, int *out_month, int *out_day);
void holidaySummerSolstice (int year, int *out_month, int *out_day);
void holidayVernalEquinox  (int year, int *out_month, int *out_day);
void holidayAutumnalEquinox(int year, int *out_month, int *out_day);
int  holidayForDate        (int year, int month, int day);
```

Two algorithms do most of the work:

- **Tomohiko Sakamoto's variant of Zeller's congruence** for day-of-week.
  Used for *every* Nth-weekday holiday — Thanksgiving (4th Thursday of
  November), MLK Day (3rd Monday of January), Memorial Day (last Monday
  of May), Labor Day, Mother's Day, Father's Day, Pride Day, Election Day
  (a special case: 1st Tuesday after the 1st Monday of November).
- **The Meeus / Jones / Butcher Gregorian algorithm** for Easter Sunday,
  valid 1583–4099. Mardi Gras chains off it as Easter minus 47 days via
  `holidayDateAdd()`. The same offset machinery handles other movable
  feasts that anchor to Easter.

`holidayForDate(year, month, day)` iterates `gHolidays[]` and dispatches on
each row's `kind`:

| `HolidayDateKind`               | Rule fields used        | Example |
|---------------------------------|-------------------------|---------|
| `HOLIDAY_KIND_FIXED`            | `month`, `day`          | New Year's = `month=1, day=1` |
| `HOLIDAY_KIND_NTH_WEEKDAY`      | `month`, `n`, `weekday` | MLK Day = `month=1, n=3, weekday=1` |
| `HOLIDAY_KIND_EASTER_OFFSET`    | `easter_offset`         | Mardi Gras = `easter_offset=-47` |
| `HOLIDAY_KIND_SOLSTICE_WIN/SUM` | (none)                  | First Day of Winter, First Day of Summer |
| `HOLIDAY_KIND_EQUINOX_VER/AUT`  | (none)                  | First Day of Spring, First Day of Autumn |
| `HOLIDAY_KIND_ELECTION_DAY`     | (none)                  | 1st Tue after 1st Mon of November |

`n` is `1..5` for first through fifth, or `-1` for "last occurrence in the
month" (used by Memorial Day and Pride Day).

## Codegen pipeline

The holiday table is generated, not hand-written. Source of truth:
[`holidays.yml`]({{ site.github_url }}/blob/main/holidays.yml) at the repo
root. Codegen script: `scripts/holidays-codegen.py`. Output:
`src/scene/holidays_table.c`, an array of `struct Holiday` rows that the C code
references via `gHolidays[]` / `gHolidayCount`.

```
holidays.yml
   └─► scripts/holidays-codegen.py
         └─► src/scene/holidays_table.c   (gHolidays[])
               └─► linked into jcreborn.exe
```

Each row carries the holiday's display title, a 12-character `short_name`
for the pause-menu cycler, a compact `date_label` ("3rd Mon Jan", "Easter
−47", etc.), the date-rule fields, and sprite metadata (width, height,
island anchor x/y, sprite index).

The accompanying art pipeline is intentionally small. One transparent 32x32
emblem per added holiday, packed into a single sheet by
`scripts/holidays-emblem-sheet.py`. Run:

```bash
./scripts/holidays-build-all.sh --clean
```

That writes individual PNGs, the packed `holiday-emblems-sheet.png`, a
checkerboard preview, and a `manifest.json` to `scratch/holidays-emblems/`.
Tracked review copies live at
[`docs/ps1/holidays-emblems/`]({{ site.github_url }}/tree/main/docs/ps1/holidays-emblems).
The sheet uses the shared 16-color CLUT from
`scripts/holidays_art_lib.py` so every emblem indexes into the same
palette; index 0 is reserved for transparent.

## Adding a holiday

Three steps:

1. **Edit `holidays.yml`** to add the row. The schema is:

   ```yaml
   - id: 6                          # stable id; do not reuse 1..4 (originals)
     name: "MLK Jr. Day"
     short_name: "MLK DAY"          # 12 chars max for pause menu cycler
     description: "Tiny podium with speech paper and dove."
     date_rule:
       kind: nth_weekday
       month: 1
       n: 3                         # third
       weekday: 1                   # Monday (0=Sun..6=Sat)
     sprite:
       width: 32
       height: 32
       island_x: 404
       island_y: 284
     palette:
       - "#7B1F2A"
       - "#F5E8C8"
       - "#C9A227"
     existing_sprite: null          # null for new emblems; 0..3 for originals
   ```

   For a fixed-date holiday: `kind: fixed`, `month: M`, `day: D`. For an
   Easter-offset: `kind: easter_offset`, `offset: ±N`. For solstice /
   equinox: `kind: solstice` or `kind: equinox` with no extra fields.

2. **Draw the emblem** in `scripts/holidays-emblem-sheet.py`. Each renderer
   returns a `Sprite(32, 32, fill=TRANSPARENT)`; only `PALETTE` indices are
   allowed. Style guide:

   - About 20–30 active pixels across.
   - 1px dark outlines where they help readability at PS1 viewing distance.
   - No Johnny, no palm reskins, no sand or sky strips.
   - Saturated but limited colors from the shared CLUT.

3. **Rebuild** the emblem sheet (`./scripts/holidays-build-all.sh --clean`)
   and the PS1 disc image (see
   [Build & toolchain]({{ '/docs/build/' | relative_url }})). The codegen
   step is run automatically as part of the PS1 build.

## What was *removed* on the way here

Earlier iterations of the holiday pipeline tried to deliver each new
holiday as a five-variant full island scene with night passes, an HTML
review picker, a final-review page, contact sheets, and a red-team suite.
That was the wrong artifact shape for the PS1 port — the emblems are
overlay props, not scenes. The five-variant pipeline, the HTML picker, the
automatic night pass, the contact sheet, the final-review page, and the
red-team suite were all deleted. What remains is the single
emblem-per-holiday pipeline above, which fits the actual contract.

## Memcard preservation

IDs 1–4 are pinned to the original Sierra holidays so any save state or
memcard reference to a holiday id from earlier work still resolves to the
same visual. New IDs start at 5 and run through 36 in calendar order
beginning with January.

## Related pages

- [Pause menu]({{ '/docs/pause-menu/' | relative_url }}) — the Options
  sub-screen has a `Holiday: <name>` cycler that walks `gHolidays[]` via
  `holidayNextId` / `holidayPrevId`.
- [Build & toolchain]({{ '/docs/build/' | relative_url }}) — how the
  codegen step plugs into the cmake build.
- [Lab: 35 holidays in 4 weeks]({{ '/lab/35-holidays-codegen/' | relative_url }}) —
  the retrospective on building the codegen pipeline; same machinery,
  written from the inside.
- [AI sub-agents on this project]({{ '/docs/agents/' | relative_url }}) —
  the holiday-emblem sprites were drafted by an LLM sub-agent against
  the style guide; this page records what that did and didn't cover.

## Per-holiday calendar

One page per holiday, grouped by month. Each page covers when it falls
(with worked dates for 2026, 2030, and 2050), the island visual, sprite
metadata, palette hint, and a link to the date algorithm that
computes it.

- [Date-algorithm core]({{ '/docs/holidays/algorithm/' | relative_url }}) —
  Zeller, Meeus / Jones / Butcher, Nth-weekday-of-month, the dispatcher.
- [Emblem gallery]({{ '/docs/holidays/emblems/' | relative_url }}) —
  the 32-icon sheet with per-emblem index.

### January

- [**New Year's Day**]({{ '/docs/holidays/calendar/new-year/' | relative_url }}) — Jan 1 *(original Sierra)*
- [**Elvis's Birthday**]({{ '/docs/holidays/calendar/elvis-bday/' | relative_url }}) — Jan 8
- [**MLK Jr. Day**]({{ '/docs/holidays/calendar/mlk-day/' | relative_url }}) — 3rd Mon Jan

### February

- [**Groundhog Day**]({{ '/docs/holidays/calendar/groundhog/' | relative_url }}) — Feb 2
- [**Valentine's Day**]({{ '/docs/holidays/calendar/valentine/' | relative_url }}) — Feb 14
- [**Super Bowl Sunday**]({{ '/docs/holidays/calendar/super-bowl/' | relative_url }}) — 2nd Sun Feb
- [**Presidents' Day**]({{ '/docs/holidays/calendar/presidents/' | relative_url }}) — 3rd Mon Feb
- [**Mardi Gras**]({{ '/docs/holidays/calendar/mardi-gras/' | relative_url }}) — Easter −47

### March

- [**Pi Day**]({{ '/docs/holidays/calendar/pi-day/' | relative_url }}) — Mar 14
- [**St. Patrick's Day**]({{ '/docs/holidays/calendar/st-patrick/' | relative_url }}) — Mar 17 *(original Sierra)*
- [**First Day of Spring**]({{ '/docs/holidays/calendar/spring/' | relative_url }}) — around Mar 20

### April

- [**April Fool's Day**]({{ '/docs/holidays/calendar/april-fool/' | relative_url }}) — Apr 1
- [**4/20 Day**]({{ '/docs/holidays/calendar/420-day/' | relative_url }}) — Apr 20
- [**Easter**]({{ '/docs/holidays/calendar/easter/' | relative_url }}) — Easter Sunday
- [**Earth Day**]({{ '/docs/holidays/calendar/earth-day/' | relative_url }}) — Apr 22

### May

- [**Star Wars Day**]({{ '/docs/holidays/calendar/star-wars/' | relative_url }}) — May 4
- [**Cinco de Mayo**]({{ '/docs/holidays/calendar/cinco-mayo/' | relative_url }}) — May 5
- [**Mother's Day**]({{ '/docs/holidays/calendar/mothers-day/' | relative_url }}) — 2nd Sun May
- [**Memorial Day**]({{ '/docs/holidays/calendar/memorial/' | relative_url }}) — last Mon May

### June

- [**Father's Day**]({{ '/docs/holidays/calendar/fathers-day/' | relative_url }}) — 3rd Sun Jun
- [**First Day of Summer**]({{ '/docs/holidays/calendar/summer/' | relative_url }}) — around Jun 21
- [**Pride Day**]({{ '/docs/holidays/calendar/pride/' | relative_url }}) — last Sun Jun

### July

- [**Independence Day**]({{ '/docs/holidays/calendar/july-4th/' | relative_url }}) — Jul 4
- [**Moon Landing Day**]({{ '/docs/holidays/calendar/moon-land/' | relative_url }}) — Jul 20

### August

- [**National Watermelon Day**]({{ '/docs/holidays/calendar/watermelon/' | relative_url }}) — Aug 3
- [**Left-Handers Day**]({{ '/docs/holidays/calendar/left-hand/' | relative_url }}) — Aug 13
- [**Hawaii Statehood Day**]({{ '/docs/holidays/calendar/hawaii-day/' | relative_url }}) — Aug 21

### September

- [**Labor Day**]({{ '/docs/holidays/calendar/labor-day/' | relative_url }}) — 1st Mon Sep
- [**Talk Like a Pirate Day**]({{ '/docs/holidays/calendar/pirate-day/' | relative_url }}) — Sep 19
- [**First Day of Autumn**]({{ '/docs/holidays/calendar/autumn/' | relative_url }}) — around Sep 22

### October

- [**Columbus / Indigenous Peoples' Day**]({{ '/docs/holidays/calendar/columbus/' | relative_url }}) — 2nd Mon Oct
- [**Halloween**]({{ '/docs/holidays/calendar/halloween/' | relative_url }}) — Oct 31 *(original Sierra)*

### November

- [**Election Day**]({{ '/docs/holidays/calendar/election/' | relative_url }}) — 1st Tue after 1st Mon Nov
- [**Veterans Day**]({{ '/docs/holidays/calendar/veterans/' | relative_url }}) — Nov 11
- [**Thanksgiving**]({{ '/docs/holidays/calendar/thanksgive/' | relative_url }}) — 4th Thu Nov

### December

- [**Christmas**]({{ '/docs/holidays/calendar/christmas/' | relative_url }}) — Dec 25 *(original Sierra)*

## View source on GitHub

- [`holidays.yml`]({{ site.github_url }}/blob/main/holidays.yml) — source
  of truth for the table.
- [`scripts/holidays-codegen.py`]({{ site.github_url }}/blob/main/scripts/holidays-codegen.py) —
  the codegen step that reads `holidays.yml` and emits the runtime table.
- [`src/scene/holidays_table.c`]({{ site.github_url }}/blob/main/src/scene/holidays_table.c) —
  generated `struct Holiday gHolidays[]` array the C code consumes
  (regenerated by the codegen script; reviewed but not hand-edited).
- [`src/scene/holidays.c`]({{ site.github_url }}/blob/main/src/scene/holidays.c) —
  date-algorithm core.
- [`src/scene/holidays.h`]({{ site.github_url }}/blob/main/src/scene/holidays.h) —
  public API.
- [`scripts/holidays-emblem-sheet.py`]({{ site.github_url }}/blob/main/scripts/holidays-emblem-sheet.py) —
  packs the 32×32 emblem PNGs into the disc-side sheet referenced by
  the per-holiday calendar entries below.
- [`scripts/holidays-build-all.sh`]({{ site.github_url }}/blob/main/scripts/holidays-build-all.sh) —
  one-button rebuild for the whole holiday pipeline (codegen + emblem
  sheet + downstream regen).
- [`docs/ps1/holidays-expansion-design.md`]({{ site.github_url }}/blob/main/docs/ps1/holidays-expansion-design.md) — visual concepts for each added holiday.
- [`docs/ps1/holidays-pipeline.md`]({{ site.github_url }}/blob/main/docs/ps1/holidays-pipeline.md) — emblem build pipeline.
- [`docs/ps1/holidays-style-guide.md`]({{ site.github_url }}/blob/main/docs/ps1/holidays-style-guide.md) — emblem art rules.
- [`docs/ps1/holidays-implementation-plan.md`]({{ site.github_url }}/blob/main/docs/ps1/holidays-implementation-plan.md) — current scope.
- [`docs/ps1/holidays-emblems/holiday-emblems-sheet.png`]({{ site.github_url }}/blob/main/docs/ps1/holidays-emblems/holiday-emblems-sheet.png) — the packed sprite sheet.
