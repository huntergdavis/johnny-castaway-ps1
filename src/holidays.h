/*
 * Holiday data + date-algorithm core for the Johnny Castaway PS1 port.
 *
 * The holiday table itself is generated from `holidays.yml` by
 * `scripts/holidays-codegen.py` into `src/holidays_table.c`. The
 * algorithm primitives (day-of-week, Nth weekday, Meeus Easter, etc.)
 * live in `src/holidays.c` and are hand-authored. Together they let
 * `holidayForDate(m,d,y)` return a stable holiday id 1..35 for any
 * year from 1583 to 4099 — we don't ship date tables that go stale.
 */
#ifndef HOLIDAYS_H
#define HOLIDAYS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Date-rule kinds (must match the codegen table layout). */
enum HolidayDateKind {
    HOLIDAY_KIND_FIXED         = 0,
    HOLIDAY_KIND_NTH_WEEKDAY   = 1,
    HOLIDAY_KIND_EASTER_OFFSET = 2,
    HOLIDAY_KIND_SOLSTICE_WIN  = 3,
    HOLIDAY_KIND_SOLSTICE_SUM  = 4,
    HOLIDAY_KIND_EQUINOX_VER   = 5,
    HOLIDAY_KIND_EQUINOX_AUT   = 6,
    HOLIDAY_KIND_ELECTION_DAY  = 7   /* 1st Tue after 1st Mon of November */
};

/* One row per holiday, generated into gHolidays[] in holidays_table.c. */
struct Holiday {
    int          id;             /* 1..35 — stable forever */
    const char  *short_name;     /* 12-char max, used by pause menu cycling */
    int          kind;           /* HolidayDateKind */
    /* Date-rule fields. Interpretation depends on `kind`:
     *   FIXED         : month + day used
     *   NTH_WEEKDAY   : month + n + weekday used (n: 1..5; -1 = last)
     *   EASTER_OFFSET : easter_offset used (e.g. -47 for Mardi Gras)
     *   SOLSTICE/EQX  : no rule fields needed
     */
    short        month;
    short        day;
    short        n;
    short        weekday;        /* 0=Sun .. 6=Sat */
    short        easter_offset;
    /* Sprite metadata (used by islandInitHoliday and the asset pipeline). */
    short        sprite_w;
    short        sprite_h;
    short        island_x;
    short        island_y;
    short        existing_sprite_index; /* 0..3 for the original 4, -1 for new */
};

extern const struct Holiday gHolidays[];
extern const int            gHolidayCount;

/* --- Algorithm primitives (pure, year-agnostic, no allocations). --- */

/* Day of week for a Gregorian date. 0=Sunday, 1=Monday, ..., 6=Saturday. */
int holidayDayOfWeek(int year, int month, int day);

/* Day-of-month for the Nth occurrence of `weekday` in `month`/`year`.
 *   n: 1..5 for first..fifth; -1 for "last" occurrence in the month.
 * Returns 0 if no Nth occurrence exists (e.g. asking for the 5th
 * Wednesday of a month with only 4 Wednesdays). */
int holidayNthWeekdayOfMonth(int n, int weekday, int month, int year);

/* Easter Sunday for `year` via the Meeus/Jones/Butcher (Gregorian)
 * algorithm. Valid 1583..4099. Out-params filled with month (3 or 4)
 * and day. */
void holidayEasterSunday(int year, int *out_month, int *out_day);

/* Add `delta_days` (can be negative) to a date. Out-params receive
 * the resulting (year, month, day). Handles month + year rollover. */
void holidayDateAdd(int year, int month, int day, int delta_days,
                    int *out_year, int *out_month, int *out_day);

/* Approximate solstice/equinox dates for `year` (Northern hemisphere).
 * Returns within ±1 day, which is fine for visual decoration. */
void holidayWinterSolstice(int year, int *out_month, int *out_day);
void holidaySummerSolstice(int year, int *out_month, int *out_day);
void holidayVernalEquinox (int year, int *out_month, int *out_day);
void holidayAutumnalEquinox(int year, int *out_month, int *out_day);

/* Is `(year, month, day)` a known holiday? Returns the holiday id
 * (1..35), or 0 if no match. Iterates gHolidays[] applying each
 * row's date rule. */
int holidayForDate(int year, int month, int day);

/* Look up the short_name (≤12 chars, all-caps) for use in the pause
 * menu's Holiday cycling. Returns "?" for invalid id. */
const char *holidayShortName(int id);

#ifdef __cplusplus
}
#endif

#endif /* HOLIDAYS_H */
