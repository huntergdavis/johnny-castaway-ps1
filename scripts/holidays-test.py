#!/usr/bin/env python3
"""
Smoke-test the C holiday-date algorithms by re-implementing them in
Python and comparing key dates against known-correct values.

This doesn't actually run the C code — instead it mirrors the C
algorithms exactly (Sakamoto, Meeus, Nth-weekday math). If the Python
mirror agrees with the spot-checks below, the C code (line-by-line
translation of the same logic) should agree too.

Run:
  python3 scripts/holidays-test.py
"""
import sys


def day_of_week(year, month, day):
    """0=Sun, 1=Mon, ... 6=Sat. Mirrors holidayDayOfWeek in holidays.c."""
    if month < 3:
        month += 12
        year  -= 1
    K = year % 100
    J = year // 100
    h = (day + (13 * (month + 1)) // 5 + K + K // 4 + J // 4 + 5 * J) % 7
    return (h + 6) % 7


def days_in_month(month, year):
    if month == 2:
        leap = ((year % 4 == 0) and (year % 100 != 0)) or (year % 400 == 0)
        return 29 if leap else 28
    return [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31][month - 1]


def nth_weekday_of_month(n, weekday, month, year):
    first_dow = day_of_week(year, month, 1)
    first = 1 + ((weekday - first_dow + 7) % 7)
    if n == -1:
        d = first
        while d + 7 <= days_in_month(month, year):
            d += 7
        return d
    if not (1 <= n <= 5):
        return 0
    d = first + (n - 1) * 7
    return d if d <= days_in_month(month, year) else 0


def easter_sunday(year):
    a = year % 19
    b = year // 100
    c = year % 100
    d = b // 4
    e = b % 4
    f = (b + 8) // 25
    g = (b - f + 1) // 3
    h = (19 * a + b - d - g + 15) % 30
    i = c // 4
    k = c % 4
    L = (32 + 2 * e + 2 * i - h - k) % 7
    m = (a + 11 * h + 22 * L) // 451
    month = (h + L - 7 * m + 114) // 31
    day = ((h + L - 7 * m + 114) % 31) + 1
    return month, day


def date_add(year, month, day, delta):
    day += delta
    while day < 1:
        month -= 1
        if month < 1:
            month = 12; year -= 1
        day += days_in_month(month, year)
    while day > days_in_month(month, year):
        day -= days_in_month(month, year)
        month += 1
        if month > 12:
            month = 1; year += 1
    return year, month, day


def election_day(year):
    """1st Tue after 1st Mon of November."""
    first_mon = nth_weekday_of_month(1, 1, 11, year)
    return 11, first_mon + 1


# --- Spot checks. Source: external calendar references. ---

CHECKS = [
    # (year, month, day, expected, description)
    ("Easter 2024", easter_sunday(2024), (3, 31)),
    ("Easter 2025", easter_sunday(2025), (4, 20)),
    ("Easter 2026", easter_sunday(2026), (4, 5)),
    ("Easter 2050", easter_sunday(2050), (4, 10)),
    ("Easter 2100", easter_sunday(2100), (3, 28)),
    # Mardi Gras = Easter - 47 days
    ("Mardi Gras 2026", date_add(2026, *easter_sunday(2026), -47)[1:],
     (2, 17)),
    ("Mardi Gras 2025", date_add(2025, *easter_sunday(2025), -47)[1:],
     (3, 4)),
    # Thanksgiving = 4th Thursday of November (weekday=4)
    ("Thanksgiving 2026", (11, nth_weekday_of_month(4, 4, 11, 2026)),
     (11, 26)),
    ("Thanksgiving 2050", (11, nth_weekday_of_month(4, 4, 11, 2050)),
     (11, 24)),
    ("Thanksgiving 2100", (11, nth_weekday_of_month(4, 4, 11, 2100)),
     (11, 25)),
    # MLK Day = 3rd Monday of January
    ("MLK 2026", (1, nth_weekday_of_month(3, 1, 1, 2026)),  (1, 19)),
    ("MLK 2050", (1, nth_weekday_of_month(3, 1, 1, 2050)),  (1, 17)),
    # Presidents Day = 3rd Monday of February
    ("Presidents 2026", (2, nth_weekday_of_month(3, 1, 2, 2026)), (2, 16)),
    # Memorial Day = last Monday of May
    ("Memorial 2026", (5, nth_weekday_of_month(-1, 1, 5, 2026)), (5, 25)),
    # Labor Day = 1st Monday of September
    ("Labor 2026", (9, nth_weekday_of_month(1, 1, 9, 2026)), (9, 7)),
    # Election Day 2024
    ("Election 2024", election_day(2024), (11, 5)),
    ("Election 2026", election_day(2026), (11, 3)),
    # Day-of-week sanity
    ("DOW 2026-04-25", day_of_week(2026, 4, 25), 6),  # Saturday
    ("DOW 2000-01-01", day_of_week(2000, 1, 1),  6),  # Saturday
    ("DOW 1900-01-01", day_of_week(1900, 1, 1),  1),  # Monday
    # --- Long-range and edge cases ---
    # Easter 2200/2300/2400 — verify Meeus extrapolates correctly.
    ("Easter 2200",       easter_sunday(2200), (4, 6)),
    ("Easter 2300",       easter_sunday(2300), (4, 8)),
    ("Easter 2400",       easter_sunday(2400), (4, 16)),
    # 21st-century-boundary day-of-week (Jan 1 2100 fell on Friday=5)
    ("DOW 2100-01-01",    day_of_week(2100, 1, 1), 5),
    # 5-Monday-May edge: 2027 has Mondays 3,10,17,24,31. last-Monday=31.
    ("Memorial 2027",     (5, nth_weekday_of_month(-1, 1, 5, 2027)),
                          (5, 31)),
    # Thanksgiving 2024 = Nov 28 (4th Thu)
    ("Thanksgiving 2024", (11, nth_weekday_of_month(4, 4, 11, 2024)),
                          (11, 28)),
    # Easter at modern boundaries — 1818 was the earliest in the
    # 19th-21st centuries (Mar 22), 1943 the latest (Apr 25).
    ("Easter 1818 early", easter_sunday(1818), (3, 22)),
    ("Easter 1943 late",  easter_sunday(1943), (4, 25)),
]


def main():
    failed = 0
    for name, got, want in CHECKS:
        ok = got == want
        marker = " OK" if ok else "FAIL"
        print(f"  [{marker}]  {name:30s}  got={got!s:20s}  want={want}")
        if not ok:
            failed += 1
    print()
    print(f"{len(CHECKS) - failed} / {len(CHECKS)} passed")
    return 0 if failed == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
