#!/usr/bin/env python3
"""Build PS1 pause-menu text offsets and disc blob."""

import json
import re
import struct
from pathlib import Path

import yaml

ROOT = Path(__file__).resolve().parent.parent
SOURCE = ROOT / "data/ps1/menu_text.json"
HOLIDAYS_SOURCE = ROOT / "holidays.yml"
HEADER_OUTPUT = ROOT / "generated/ps1/ps1_menu_text_data.h"
DAT_OUTPUT = ROOT / "generated/ps1/MENUTEXT.DAT"

MONTH_ABBR = [
    "", "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
    "JUL", "AUG", "SEP", "OCT", "NOV", "DEC",
]
WEEKDAY_ABBR = ["SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"]
NTH_LABEL = {-1: "LAST", 1: "1ST", 2: "2ND", 3: "3RD", 4: "4TH", 5: "5TH"}

FREEPLAY_GAGS = [
    ("Scratch", "Johnny scratches and settles in.", "GJFFFOOD.BMP"),
    ("Hot", "Johnny wipes out in the heat.", "GJHOT.BMP"),
    ("Idea", "A light bulb appears over Johnny.", "LITEBULB.BMP"),
    ("Angry", "Johnny throws a small tantrum.", "GJANGRY.BMP"),
    ("Bonk", "A coconut clocks Johnny.", "COCOHEAD.BMP"),
    ("Strut", "Johnny takes a proud little walk.", "MEXCWALK.BMP"),
    ("Runaway", "Johnny bolts across the island.", "GJRUNAWA.BMP"),
]

FREEPLAY_VISITORS = [
    ("Seagull", "A small bird visits the palm.", "GJGULL1.BMP"),
    ("Liliputs", "Tiny visitors cross the sand.", "LILIPUTS.BMP"),
    ("Biplane", "A plane sweeps past the island.", "GJBIPLAN.BMP"),
    ("Canoe", "A native canoe stops by.", "GJNAT1.BMP"),
    ("Boat", "A boat appears offshore.", "BOAT.BMP"),
    ("King Kong", "A skyline-sized visitor looms.", "GJKINGKO.BMP"),
    ("Mary", "Mary drops into the scene.", "MJBATH.BMP"),
    ("Pirate", "The fish-man pirate appears.", "FISHMAN.BMP"),
    ("Flock", "A small flock passes overhead.", "GJGULL3.BMP"),
    ("Meanwhile", "A comic meanwhile card appears.", "MEANWHIL.BMP"),
    ("Cloud", "A cloud floats over the island.", "CLOUDS.BMP"),
]


def c_id(raw: str) -> str:
    value = re.sub(r"[^A-Za-z0-9]+", "_", raw).strip("_").upper()
    if not value:
        raise RuntimeError(f"invalid menu text id: {raw!r}")
    if value[0].isdigit():
        value = "_" + value
    return "PMT_" + value


def normalize_holiday_kind(rule: dict) -> dict:
    rule = dict(rule or {})
    kind = rule.get("kind")
    month = rule.get("month")
    if kind == "equinox":
        rule["kind"] = "equinox_vernal" if month == 3 else "equinox_autumnal"
    elif kind == "solstice":
        rule["kind"] = "solstice_summer" if month == 6 else "solstice_winter"
    return rule


def holiday_date_label(rule: dict) -> str:
    rule = normalize_holiday_kind(rule)
    kind = rule.get("kind", "fixed")
    month = int(rule.get("month", 0))
    if kind == "fixed":
        return f"{MONTH_ABBR[month]} {int(rule.get('day', 0))}"
    if kind == "nth_weekday":
        n = int(rule.get("n", 0))
        weekday = int(rule.get("weekday", 0))
        return f"{NTH_LABEL.get(n, '?')} {WEEKDAY_ABBR[weekday]} {MONTH_ABBR[month]}"
    if kind == "easter_offset":
        offset = int(rule.get("offset", 0))
        return "EASTER" if offset == 0 else f"EASTER{offset:+d}"
    if kind == "solstice_winter":
        return "DEC 21"
    if kind == "solstice_summer":
        return "JUN 21"
    if kind == "equinox_vernal":
        return "MAR 20"
    if kind == "equinox_autumnal":
        return "SEP 22"
    if kind == "election_day":
        return "NOV 2-8"
    return "?"


def append_generated_entries(entries: list[dict]) -> list[int]:
    holidays = yaml.safe_load(HOLIDAYS_SOURCE.read_text(encoding="utf-8"))
    holiday_ids = []
    for holiday in holidays:
        holiday_id = int(holiday["id"])
        holiday_ids.append(holiday_id)
        entries.append({
            "id": f"holiday_title_{holiday_id}",
            "text": holiday.get("name", "?"),
        })
        entries.append({
            "id": f"holiday_short_{holiday_id}",
            "text": holiday.get("short_name", holiday.get("name", "?")),
        })
        entries.append({
            "id": f"holiday_date_{holiday_id}",
            "text": holiday_date_label(holiday.get("date_rule") or {}),
        })

    for index, (title, _desc, _bmp) in enumerate(FREEPLAY_GAGS):
        entries.append({"id": f"freeplay_gag_title_{index}", "text": title})
    for index, (_title, desc, _bmp) in enumerate(FREEPLAY_GAGS):
        entries.append({"id": f"freeplay_gag_desc_{index}", "text": desc})
    for index, (_title, _desc, bmp) in enumerate(FREEPLAY_GAGS):
        entries.append({"id": f"freeplay_gag_bmp_{index}", "text": bmp})

    for index, (title, _desc, _bmp) in enumerate(FREEPLAY_VISITORS):
        entries.append({"id": f"freeplay_visitor_title_{index}", "text": title})
    for index, (_title, desc, _bmp) in enumerate(FREEPLAY_VISITORS):
        entries.append({"id": f"freeplay_visitor_desc_{index}", "text": desc})
    for index, (_title, _desc, bmp) in enumerate(FREEPLAY_VISITORS):
        entries.append({"id": f"freeplay_visitor_bmp_{index}", "text": bmp})

    return holiday_ids


def main() -> None:
    entries = json.loads(SOURCE.read_text(encoding="utf-8"))
    holiday_ids = append_generated_entries(entries)
    blob = bytearray()
    offsets = []
    seen = set()

    for entry in entries:
        text_id = entry["id"]
        if text_id in seen:
            raise RuntimeError(f"duplicate menu text id: {text_id}")
        seen.add(text_id)

        encoded = entry["text"].encode("ascii") + b"\0"
        if len(blob) > 0xFFFF:
            raise RuntimeError("menu text blob exceeds 64 KiB")
        offsets.append((text_id, len(blob)))
        blob.extend(encoded)

    HEADER_OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    DAT_OUTPUT.parent.mkdir(parents=True, exist_ok=True)

    out = []
    out.append("/* Generated by scripts/build-menu-text-data.py.")
    out.append(" * Source: data/ps1/menu_text.json.")
    out.append(" * Re-run after menu text changes; do not edit by hand. */")
    out.append("")
    out.append("#ifndef PS1_MENU_TEXT_DATA_H")
    out.append("#define PS1_MENU_TEXT_DATA_H")
    out.append("")
    out.append('#include "mytypes.h"')
    out.append("")
    out.append("enum Ps1MenuTextId {")
    for text_id, _ in offsets:
        out.append(f"    {c_id(text_id)},")
    out.append("    PMT_COUNT")
    out.append("};")
    out.append("")
    out.append(f"#define PS1_HOLIDAY_TEXT_ID_LIMIT {max(holiday_ids) + 1}")
    out.append("#define PS1_HOLIDAY_TEXT_FIELD_COUNT 3")
    out.append("")
    out.append("#ifdef PS1_MENU_TEXT_DEFINE")
    out.append("const uint16 gPs1MenuTextOffsets[PMT_COUNT] = {")
    for text_id, offset in offsets:
        out.append(f"    {offset}, /* {c_id(text_id)} */")
    out.append("};")
    out.append("")
    out.append(
        "const uint16 "
        "gPs1HolidayMenuTextIds[PS1_HOLIDAY_TEXT_ID_LIMIT]"
        "[PS1_HOLIDAY_TEXT_FIELD_COUNT] = {"
    )
    for holiday_id in holiday_ids:
        out.append(
            f"    [{holiday_id}] = {{ "
            f"{c_id(f'holiday_title_{holiday_id}')}, "
            f"{c_id(f'holiday_short_{holiday_id}')}, "
            f"{c_id(f'holiday_date_{holiday_id}')} }},"
        )
    out.append("};")
    out.append("#else")
    out.append("extern const uint16 gPs1MenuTextOffsets[PMT_COUNT];")
    out.append(
        "extern const uint16 "
        "gPs1HolidayMenuTextIds[PS1_HOLIDAY_TEXT_ID_LIMIT]"
        "[PS1_HOLIDAY_TEXT_FIELD_COUNT];"
    )
    out.append("#endif")
    out.append("")
    out.append("#endif")
    out.append("")
    HEADER_OUTPUT.write_text("\n".join(out), encoding="utf-8")

    header = bytearray()
    header.extend(b"JCMT")
    header.extend(struct.pack("<HH", 1, len(offsets)))
    DAT_OUTPUT.write_bytes(bytes(header) + bytes(blob))

    print(
        f"Wrote {HEADER_OUTPUT.relative_to(ROOT)} "
        f"({len(offsets)} entries)"
    )
    print(
        f"Wrote {DAT_OUTPUT.relative_to(ROOT)} "
        f"({len(blob)} text bytes)"
    )


if __name__ == "__main__":
    main()
