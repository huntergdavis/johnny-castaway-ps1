/*
 * Holiday date-algorithm core. Pure C, no allocation, no PS1 deps.
 *
 * Goal: `holidayForDate(year, month, day)` returns a holiday id
 * for any Gregorian date 1583..4099 without any year-specific data
 * tables — we don't ship calendars that go stale. Movable feasts
 * (Easter, Mardi Gras, Thanksgiving, etc.) are computed by algorithm.
 *
 * The actual holiday list (`gHolidays[]`) is generated from
 * `holidays.yml` into `holidays_table.c` by `scripts/holidays-codegen.py`.
 */

#include "holidays.h"

/* --- Day of week (Tomohiko Sakamoto's variant, 1583+). --- */

int holidayDayOfWeek(int year, int month, int day)
{
    /* Adjust for January / February (treat them as months 13 / 14 of
     * the previous year). */
    if (month < 3) {
        month += 12;
        year  -= 1;
    }
    int K = year % 100;
    int J = year / 100;
    /* Zeller's congruence (Gregorian). h = 0 → Saturday, 1 → Sunday, ... */
    int h = (day + (13 * (month + 1)) / 5 + K + K / 4 + J / 4 + 5 * J) % 7;
    /* Convert to 0=Sunday, 1=Monday, ... 6=Saturday. */
    return (h + 6) % 7;
}

/* --- Nth weekday of a month. --- */

static int holidayDaysInMonth(int month, int year)
{
    static const int dim[13] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if (month < 1 || month > 12) return 0;
    if (month == 2) {
        int leap = ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0);
        return leap ? 29 : 28;
    }
    return dim[month];
}

int holidayNthWeekdayOfMonth(int n, int weekday, int month, int year)
{
    if (month < 1 || month > 12)            return 0;
    if (weekday < 0 || weekday > 6)         return 0;
    int dim = holidayDaysInMonth(month, year);
    int firstDow = holidayDayOfWeek(year, month, 1);
    /* Day of month for the FIRST occurrence of `weekday`. */
    int first = 1 + ((weekday - firstDow + 7) % 7);
    if (n == -1) {
        /* Last occurrence: walk forward by 7s until past dim, back up. */
        int d = first;
        while (d + 7 <= dim) d += 7;
        return d;
    }
    if (n < 1 || n > 5)                     return 0;
    int d = first + (n - 1) * 7;
    return (d <= dim) ? d : 0;
}

/* --- Meeus / Jones / Butcher Gregorian Easter (valid 1583..4099). --- */

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

/* --- Date arithmetic (delta days, handles year rollover). --- */

void holidayDateAdd(int year, int month, int day, int delta_days,
                    int *out_year, int *out_month, int *out_day)
{
    /* Apply day delta with month/year carry. Slow but simple — fine
     * for the small offsets we use (Mardi Gras = Easter - 47). */
    day += delta_days;
    while (day < 1) {
        month -= 1;
        if (month < 1) { month = 12; year -= 1; }
        day += holidayDaysInMonth(month, year);
    }
    while (day > holidayDaysInMonth(month, year)) {
        day -= holidayDaysInMonth(month, year);
        month += 1;
        if (month > 12) { month = 1; year += 1; }
    }
    if (out_year)  *out_year  = year;
    if (out_month) *out_month = month;
    if (out_day)   *out_day   = day;
}

/* --- Solstices / equinoxes — approximate but consistent.
 *
 * Real astronomical computation is overkill for "is today the solstice"
 * since dates drift only ±1 day across centuries. We use a fixed
 * canonical date that's been correct most years 2000–2100; visual
 * decoration tolerance is fine.
 *
 * Spring equinox  ≈ Mar 20
 * Summer solstice ≈ Jun 21
 * Autumn equinox  ≈ Sep 22
 * Winter solstice ≈ Dec 21
 *
 * If precision becomes important we can swap for Meeus's astronomical
 * algorithms — but they need year-specific tables to be perfect, which
 * defeats the point. */

void holidayWinterSolstice(int year, int *out_month, int *out_day)
{ (void)year; if (out_month) *out_month = 12; if (out_day) *out_day = 21; }

void holidaySummerSolstice(int year, int *out_month, int *out_day)
{ (void)year; if (out_month) *out_month =  6; if (out_day) *out_day = 21; }

void holidayVernalEquinox(int year, int *out_month, int *out_day)
{ (void)year; if (out_month) *out_month =  3; if (out_day) *out_day = 20; }

void holidayAutumnalEquinox(int year, int *out_month, int *out_day)
{ (void)year; if (out_month) *out_month =  9; if (out_day) *out_day = 22; }

/* --- Holiday lookup. Iterates gHolidays[] from holidays_table.c.
 *
 * For each row, evaluates the date rule and compares against the
 * passed date. First match wins; returns 0 if no match.
 *
 * Cost: one cheap arithmetic pass per holiday. With a few dozen holidays this
 * is negligible — called once per scene start. No need to memoize. */

static int holidayForDateFiltered(int year, int month, int day, int original4Only)
{
    if (month < 1 || month > 12) return 0;
    if (day < 1   || day > 31)   return 0;

    for (int i = 0; i < gHolidayCount; i++) {
        const struct Holiday *h = &gHolidays[i];
        int rm = 0, rd = 0;
        int em, ed, ey;
        if (original4Only && !holidayIsOriginalId(h->id))
            continue;
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
            /* Easter offsets within ~50 days of Easter shouldn't cross
             * year boundaries for practical purposes; but if they do
             * we just lose this match for the boundary year. */
            if (ey != year) continue;
            break;
        case HOLIDAY_KIND_SOLSTICE_WIN:
            holidayWinterSolstice(year, &rm, &rd);
            break;
        case HOLIDAY_KIND_SOLSTICE_SUM:
            holidaySummerSolstice(year, &rm, &rd);
            break;
        case HOLIDAY_KIND_EQUINOX_VER:
            holidayVernalEquinox(year, &rm, &rd);
            break;
        case HOLIDAY_KIND_EQUINOX_AUT:
            holidayAutumnalEquinox(year, &rm, &rd);
            break;
        case HOLIDAY_KIND_ELECTION_DAY: {
            /* 1st Tuesday after the 1st Monday of November.
             * If Nov 1 is Tuesday, Election Day is Nov 8 (the Mon
             * comes Nov 7, then Tue Nov 8). Always Nov 2..8. */
            int firstMon = holidayNthWeekdayOfMonth(1, 1 /*Mon*/, 11, year);
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

int holidayForDate(int year, int month, int day)
{
    return holidayForDateFiltered(year, month, day, 0);
}

int holidayForDateOriginal4(int year, int month, int day)
{
    return holidayForDateFiltered(year, month, day, 1);
}

const struct Holiday *holidayById(int id)
{
    for (int i = 0; i < gHolidayCount; i++) {
        if (gHolidays[i].id == id) return &gHolidays[i];
    }
    return 0;
}

int holidayMaxId(void)
{
    int maxId = 0;
    for (int i = 0; i < gHolidayCount; i++) {
        if (gHolidays[i].id > maxId) maxId = gHolidays[i].id;
    }
    return maxId;
}

int holidayIsOriginalId(int id)
{
    return id >= 1 && id <= 4 && holidayById(id) != 0;
}

int holidayFirstExpandedId(void)
{
    for (int i = 0; i < gHolidayCount; i++) {
        if (gHolidays[i].id > 4)
            return gHolidays[i].id;
    }
    return 0;
}

int holidayModeFromOverride(int holidayOverride)
{
    if (holidayOverride < 0)
        return HOLIDAY_MODE_AUTO_ALL;
    if (holidayOverride == 0)
        return HOLIDAY_MODE_NONE;
    if (holidayIsOriginalId(holidayOverride))
        return HOLIDAY_MODE_MANUAL_ORIG4;
    return HOLIDAY_MODE_MANUAL_EXPANDED;
}

int holidayModeIsManual(int mode)
{
    return mode == HOLIDAY_MODE_MANUAL_ORIG4 ||
           mode == HOLIDAY_MODE_MANUAL_EXPANDED;
}

int holidayNextId(int current)
{
    if (current < 0) return 0;
    if (current == 0) {
        return (gHolidayCount > 0) ? gHolidays[0].id : -1;
    }

    for (int i = 0; i < gHolidayCount; i++) {
        if (gHolidays[i].id == current) {
            if (i + 1 < gHolidayCount) return gHolidays[i + 1].id;
            return -1;
        }
    }
    return -1;
}

/* Reverse cycle: AUTO(-1) -> last id -> ... -> first id -> NONE(0) -> AUTO. */
int holidayPrevId(int current)
{
    if (current < 0) {
        return (gHolidayCount > 0) ? gHolidays[gHolidayCount - 1].id : 0;
    }
    if (current == 0) return -1;

    for (int i = 0; i < gHolidayCount; i++) {
        if (gHolidays[i].id == current) {
            if (i == 0) return 0;        /* first id steps back to NONE */
            return gHolidays[i - 1].id;
        }
    }
    return -1;
}

int holidaySpriteIndex(int id)
{
    const struct Holiday *h = holidayById(id);
    return h ? h->sprite_index : -1;
}

#ifndef PS1_BUILD
static const struct HolidayDisplay *holidayDisplayById(int id)
{
    for (int i = 0; i < gHolidayDisplayCount; i++) {
        if (gHolidayDisplays[i].id == id)
            return &gHolidayDisplays[i];
    }
    return 0;
}
#endif

const char *holidayTitle(int id)
{
#ifndef PS1_BUILD
    const struct HolidayDisplay *h = holidayDisplayById(id);
    return h ? h->title : "?";
#else
    (void)id;
    return "?";
#endif
}

const char *holidayShortName(int id)
{
#ifndef PS1_BUILD
    const struct HolidayDisplay *h = holidayDisplayById(id);
    return h ? h->short_name : "?";
#else
    (void)id;
    return "?";
#endif
}

const char *holidayDateLabel(int id)
{
#ifndef PS1_BUILD
    const struct HolidayDisplay *h = holidayDisplayById(id);
    return h ? h->date_label : "?";
#else
    (void)id;
    return "?";
#endif
}
