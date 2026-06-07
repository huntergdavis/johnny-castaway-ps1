---
layout: page
title: Date-algorithm core
eyebrow: Reference
subtitle: How the 36 holidays compute their dates with no expiring tables.
description: Zeller, Meeus / Jones / Butcher Easter, Nth-weekday-of-month, and the holidayForDate dispatcher used by the Johnny Castaway PS1 port to resolve 36 holidays from 1583 to 4099.
---

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

## The problem

The original 1992 Sierra screensaver shipped four holidays — New Year's
Day, St. Patrick's Day, Halloween, Christmas. Three of those are fixed
calendar dates and the fourth is a fixed calendar date too. They never
move. A two-line lookup table is enough.

This port carries 36 holidays. Several of them move year to year:
Easter, Mardi Gras, Thanksgiving, MLK Day, Memorial Day, Labor Day,
Election Day, both equinoxes, both solstices, Mother's Day, Father's
Day, Pride Day, Super Bowl Sunday, Presidents' Day, Columbus /
Indigenous Peoples' Day. The PS1 has no internet, no NTP, no licensed
calendar library. It cannot phone home for a date table, and a baked-in
table would expire — the port targets a 100+ year service window
(1583–4099 is the supported Gregorian range), and shipping a stale CSV
that needs annual republishing was not acceptable.

So every date is computed from a pure algorithm at runtime. No
year-specific data, no embedded calendar. The whole core lives in
[`src/scene/holidays.c`]({{ site.github_url }}/blob/main/src/scene/holidays.c) —
roughly 320 lines of C, no allocations, no PS1 dependencies.

## Day-of-week from (month, day, year)

The foundation. Used directly by every Nth-weekday holiday and by
Election Day. The implementation is the Tomohiko Sakamoto variant of
Zeller's congruence for the Gregorian calendar. January and February are
treated as months 13 and 14 of the previous year, which removes the
leap-year branch from the inner formula.

```c
int holidayDayOfWeek(int year, int month, int day)
{
    if (month < 3) {
        month += 12;
        year  -= 1;
    }
    int K = year % 100;
    int J = year / 100;
    /* h = 0 -> Saturday, 1 -> Sunday, ... */
    int h = (day + (13 * (month + 1)) / 5 + K + K / 4 + J / 4 + 5 * J) % 7;
    /* Convert to 0=Sunday, 1=Monday, ... 6=Saturday. */
    return (h + 6) % 7;
}
```

The native Zeller convention is `0=Saturday`. The function shifts to
`0=Sunday` because the rest of the code (and `weekday` fields in
`holidays.yml`) use the Sunday-zero convention.

**Worked example.** What day of the week was 2026-04-26?

April is month 4, no shift. `K = 26`, `J = 20`.

```
h = (26 + (13 * 5) / 5 + 26 + 26/4 + 20/4 + 5*20) % 7
  = (26 + 13 + 26 + 6 + 5 + 100) % 7
  = 176 % 7 = 1   (Sunday in Zeller's convention)

(1 + 6) % 7 = 0   -> Sunday. Correct.
```

## Nth weekday of month

The pattern that drives most US movable holidays.

| Holiday | Rule |
|---|---|
| MLK Day | 3rd Monday of January |
| Super Bowl Sunday | 2nd Sunday of February |
| Presidents' Day | 3rd Monday of February |
| Mother's Day | 2nd Sunday of May |
| Memorial Day | Last Monday of May |
| Father's Day | 3rd Sunday of June |
| Pride Day | Last Sunday of June |
| Labor Day | 1st Monday of September |
| Columbus / Indigenous Peoples' Day | 2nd Monday of October |
| Thanksgiving | 4th Thursday of November |

The algorithm: find the first day-of-month that lands on the target
weekday, then add `(n - 1) * 7`. For `n == -1` ("last"), walk forward by
7 until the next step would overshoot the month length.

```c
int holidayNthWeekdayOfMonth(int n, int weekday, int month, int year)
{
    if (month < 1 || month > 12)            return 0;
    if (weekday < 0 || weekday > 6)         return 0;
    int dim = holidayDaysInMonth(month, year);
    int firstDow = holidayDayOfWeek(year, month, 1);
    int first = 1 + ((weekday - firstDow + 7) % 7);
    if (n == -1) {
        int d = first;
        while (d + 7 <= dim) d += 7;
        return d;
    }
    if (n < 1 || n > 5)                     return 0;
    int d = first + (n - 1) * 7;
    return (d <= dim) ? d : 0;
}
```

The function returns 0 for "no such Nth occurrence" — e.g. asking for
the 5th Wednesday of a month with only four. Callers treat 0 as
"holiday does not match today."

**Worked example.** When is Thanksgiving 2050?

`holidayDayOfWeek(2050, 11, 1)` returns Tuesday (`2`). Thursday is
`weekday=4`. `first = 1 + ((4 - 2 + 7) % 7) = 1 + 2 = 3`. So Nov 3 is
the first Thursday. Add `(4 - 1) * 7 = 21`. Result: November 24, 2050.

## Easter Sunday — Meeus / Jones / Butcher

The largest single block. Gregorian Easter via the
Meeus / Jones / Butcher algorithm, valid 1583 through 4099. The
implementation is straight off Astronomical Algorithms, second edition.

```c
void holidayEasterSunday(int year, int *out_month, int *out_day)
{
    int a = year % 19;
    int b = year / 100;
    int c = year % 100;
    int d = b / 4;
    int e = b % 4;
    int f = (b + 8) / 25;
    int g = (b - f + 1) / 3;
    int h = (19 * a + b - d - g + 15) % 30;
    int i = c / 4;
    int k = c % 4;
    int l = (32 + 2 * e + 2 * i - h - k) % 7;
    int m = (a + 11 * h + 22 * l) / 451;
    int month = (h + l - 7 * m + 114) / 31;
    int day   = ((h + l - 7 * m + 114) % 31) + 1;
    if (out_month) *out_month = month;
    if (out_day)   *out_day   = day;
}
```

Easter offsets (Mardi Gras = Easter − 47, Ash Wednesday = Easter − 46,
Good Friday = Easter − 2, Pentecost = Easter + 49) chain through
`holidayDateAdd`, which carries day deltas across month and year
boundaries:

```c
void holidayDateAdd(int year, int month, int day, int delta_days,
                    int *out_year, int *out_month, int *out_day);
```

In this corpus only Easter itself (offset 0) and Mardi Gras
(offset −47) are used; the machinery handles the rest if added.

**Worked example.** Easter 2026.

`a = 2026 % 19 = 12`, `b = 20`, `c = 26`, `d = 5`, `e = 0`, `f = 1`,
`g = 6`, `h = (228 + 20 - 5 - 6 + 15) % 30 = 252 % 30 = 12`, `i = 6`,
`k = 2`, `l = (32 + 0 + 12 - 12 - 2) % 7 = 30 % 7 = 2`, `m = 0`.
`month = (12 + 2 - 0 + 114) / 31 = 128 / 31 = 4`,
`day = (128 % 31) + 1 = 4 + 1 = 5`.

Easter 2026 = April 5. Mardi Gras 2026 = April 5 minus 47 days =
February 17.

A few more cross-checks: Easter 2030 = April 21. Easter 2050 = April 10.
Easter 2099 = April 12.

## Solstice and equinox

Real astronomical solstice and equinox calculation needs Meeus's
year-specific tables — the kind of data table the rest of this design
deliberately avoids. The dates drift by at most ±1 day across centuries,
and the holiday is a visual decoration, not a navigation aid. So the
runtime returns canonical fixed dates that are correct most years
2000–2100:

```c
void holidayWinterSolstice (int y, int *m, int *d) { *m = 12; *d = 21; }
void holidaySummerSolstice (int y, int *m, int *d) { *m =  6; *d = 21; }
void holidayVernalEquinox  (int y, int *m, int *d) { *m =  3; *d = 20; }
void holidayAutumnalEquinox(int y, int *m, int *d) { *m =  9; *d = 22; }
```

Tolerance is ±1 day, fine for a screensaver. If precision ever becomes
important the upgrade path is a clean swap to
[Meeus 1998, ch. 27](https://books.google.com/books?id=cjVmDwAAQBAJ).

**Worked dates for 2026.** Vernal equinox: March 20. Summer solstice:
June 21. Autumnal equinox: September 22. Winter solstice: December 21.

## Election Day

A special case worth its own dispatch. US federal Election Day is the
first Tuesday after the first Monday of November — *not* the first
Tuesday of November. The two coincide unless November 1 falls on a
Tuesday, in which case the first Monday is November 7 and Election Day
shifts to November 8. Always falls Nov 2..8.

```c
int firstMon = holidayNthWeekdayOfMonth(1, 1 /*Mon*/, 11, year);
rm = 11;
rd = firstMon + 1;
```

Worked dates: Election Day 2026 = November 3. Election Day 2028 =
November 7.

## Putting it together

`holidayForDate(year, month, day)` walks the holiday table and
dispatches on each row's `kind`. First match wins; returns 0 if no
holiday matches today. Cost is one cheap arithmetic pass per holiday —
called once per scene start, no need to memoize.

```c
int holidayForDate(int year, int month, int day)
{
    if (month < 1 || month > 12) return 0;
    if (day < 1   || day > 31)   return 0;

    for (int i = 0; i < gHolidayCount; i++) {
        const struct Holiday *h = &gHolidays[i];
        int rm = 0, rd = 0;
        int em, ed, ey;
        switch (h->kind) {
        case HOLIDAY_KIND_FIXED:
            rm = h->month;
            rd = h->day;
            break;
        case HOLIDAY_KIND_NTH_WEEKDAY:
            rm = h->month;
            rd = holidayNthWeekdayOfMonth(h->n, h->weekday, h->month, year);
            break;
        case HOLIDAY_KIND_EASTER_OFFSET:
            holidayEasterSunday(year, &em, &ed);
            holidayDateAdd(year, em, ed, h->easter_offset, &ey, &rm, &rd);
            if (ey != year) continue;
            break;
        case HOLIDAY_KIND_SOLSTICE_WIN:
            holidayWinterSolstice(year, &rm, &rd); break;
        case HOLIDAY_KIND_SOLSTICE_SUM:
            holidaySummerSolstice(year, &rm, &rd); break;
        case HOLIDAY_KIND_EQUINOX_VER:
            holidayVernalEquinox(year, &rm, &rd); break;
        case HOLIDAY_KIND_EQUINOX_AUT:
            holidayAutumnalEquinox(year, &rm, &rd); break;
        case HOLIDAY_KIND_ELECTION_DAY: {
            int firstMon = holidayNthWeekdayOfMonth(1, 1, 11, year);
            rm = 11;
            rd = firstMon + 1;
            break;
        }
        default:
            continue;
        }
        if (rm == month && rd == day) return h->id;
    }
    return 0;
}
```

The dispatcher's complement is the codegen output. Each row of
[`gHolidays[]`]({{ site.github_url }}/blob/main/src/scene/holidays_table.c) is
generated from
[`holidays.yml`]({{ site.github_url }}/blob/main/holidays.yml) by
`scripts/holidays-codegen.py`. The C file never lists holidays
literally — only the algorithm. Adding a new holiday is a YAML edit and
a rebuild.

| `HolidayDateKind` | Rule fields | Example |
|---|---|---|
| `HOLIDAY_KIND_FIXED` | `month`, `day` | New Year's = (1, 1) |
| `HOLIDAY_KIND_NTH_WEEKDAY` | `month`, `n`, `weekday` | MLK Day = (1, 3, Mon) |
| `HOLIDAY_KIND_EASTER_OFFSET` | `easter_offset` | Mardi Gras = −47 |
| `HOLIDAY_KIND_SOLSTICE_WIN` | (none) | First Day of Winter |
| `HOLIDAY_KIND_SOLSTICE_SUM` | (none) | First Day of Summer |
| `HOLIDAY_KIND_EQUINOX_VER` | (none) | First Day of Spring |
| `HOLIDAY_KIND_EQUINOX_AUT` | (none) | First Day of Autumn |
| `HOLIDAY_KIND_ELECTION_DAY` | (none) | 1st Tue after 1st Mon Nov |

## Why no Hijri / Lunar New Year / Diwali

Honest answer: the current corpus is US-centric. The 36 holidays cover
the US federal calendar, the four original Sierra holidays, and a
handful of internet-popular dates (Pi Day, Star Wars Day, Pirate Day,
4/20). There is no Lunar New Year, no Hijri calendar holiday, no Diwali,
no Rosh Hashanah, no Lunar Mid-Autumn Festival. The algorithmic spine
above is general — Hindu, Hebrew, and Islamic calendar conversions are
all covered by Meeus's book — and adding a `HOLIDAY_KIND_HIJRI_FIXED`
or `HOLIDAY_KIND_LUNISOLAR` is the obvious extension. The reason it is
not done yet is that nobody has drawn the emblems and nobody has
written the conversion tables. The runtime is the easy part. This is
not a culturally complete calendar; it is the calendar this port
shipped with.

## Related

- [Per-holiday calendar]({{ '/docs/holidays/' | relative_url }}) — every
  holiday's own page with worked dates.
- [Emblem gallery]({{ '/docs/holidays/emblems/' | relative_url }}) —
  the 32-icon sheet.
- [Lab: 35 holidays in 4 weeks]({{ '/lab/35-holidays-codegen/' | relative_url }}) —
  the retrospective on building the codegen pipeline, the date
  algorithms above, and the sprite primitives the emblem sheet
  composes from.
- [`src/scene/holidays.c`]({{ site.github_url }}/blob/main/src/scene/holidays.c) —
  this file's algorithms.
- [`src/scene/holidays.h`]({{ site.github_url }}/blob/main/src/scene/holidays.h) —
  public API.
- [`holidays.yml`]({{ site.github_url }}/blob/main/holidays.yml) —
  source of truth for the table.
