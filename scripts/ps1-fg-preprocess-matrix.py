#!/usr/bin/env python3
"""Rank foreground packs for host-side graphics preprocessing experiments.

This is a host-only planning tool. It reads the all-scene performance matrix,
parses the current FG2/FGP3 packs, and estimates where pack-emitted
upload-ready bands or cleanup/restore metadata have the largest upside.
"""

from __future__ import annotations

import argparse
import csv
import json
import struct
from dataclasses import dataclass
from pathlib import Path
from typing import Any


HEADER = "<4sHHHHHHHHHHIIIHH"
ENTRY = "<HhhHHHII"
HEADER_SIZE = struct.calcsize(HEADER)
ENTRY_SIZE = struct.calcsize(ENTRY)

SCREEN_W = 640
SCREEN_H = 480
TILE_W = 320
TILE_H = 240
TILE_COUNT = 4
FLAG_SCENE_RELATIVE = 0x0008

UPLOAD_MAX_RECTS = 8
UPLOAD_BAND_MERGE_GAP = 0


@dataclass(frozen=True)
class Header:
    magic: bytes
    version: int
    frame_count: int
    display_vblanks: int
    flags: int
    screen_width: int
    screen_height: int
    union_x: int
    union_y: int
    union_width: int
    union_height: int
    table_offset: int
    data_offset: int
    sound_events_offset: int
    sound_event_count: int
    reserved1: int


@dataclass(frozen=True)
class Entry:
    index: int
    source_frame: int
    x: int
    y: int
    width: int
    height: int
    hold_vblanks: int
    data_offset: int
    data_size: int


Rows = list[list[list[tuple[int, int]]]]


def read_u16(data: bytes, offset: int) -> int:
    return struct.unpack_from("<H", data, offset)[0]


def read_compact_u16(data: bytes, offset: int, limit: int) -> tuple[int | None, int]:
    if offset >= limit:
        return None, offset
    value = data[offset]
    offset += 1
    if value != 0xFF:
        return value, offset
    if offset + 2 > limit:
        return None, limit
    return read_u16(data, offset), offset + 2


def safe_int(value: str) -> int:
    if value == "":
        return 0
    return int(float(value))


def safe_float(value: str) -> float:
    if value == "":
        return 0.0
    return float(value)


def merge_intervals(intervals: list[tuple[int, int]]) -> list[tuple[int, int]]:
    if not intervals:
        return []
    ordered = sorted(intervals)
    merged: list[tuple[int, int]] = []
    start, end = ordered[0]
    for next_start, next_end in ordered[1:]:
        if next_start <= end:
            end = max(end, next_end)
        else:
            merged.append((start, end))
            start, end = next_start, next_end
    merged.append((start, end))
    return merged


def empty_rows() -> Rows:
    return [[[] for _ in range(TILE_H)] for _ in range(TILE_COUNT)]


def full_rows() -> Rows:
    return [[[(0, TILE_W)] for _ in range(TILE_H)] for _ in range(TILE_COUNT)]


def add_screen_interval(rows: Rows, x0: int, x1: int, y: int) -> None:
    if y < 0 or y >= SCREEN_H:
        return
    x0 = max(0, x0)
    x1 = min(SCREEN_W, x1)
    if x0 >= x1:
        return

    base_tile = 0 if y < TILE_H else 2
    local_y = y if y < TILE_H else y - TILE_H
    if x0 < TILE_W:
        rows[base_tile][local_y].append((x0, min(x1, TILE_W)))
    if x1 > TILE_W:
        rows[base_tile + 1][local_y].append((max(x0, TILE_W) - TILE_W, x1 - TILE_W))


def merge_rows(rows: Rows) -> Rows:
    out = empty_rows()
    for tile in range(TILE_COUNT):
        for y in range(TILE_H):
            out[tile][y] = merge_intervals(rows[tile][y])
    return out


def union_rows(*row_sets: Rows) -> Rows:
    out = empty_rows()
    for rows in row_sets:
        for tile in range(TILE_COUNT):
            for y in range(TILE_H):
                out[tile][y].extend(rows[tile][y])
    return merge_rows(out)


def interval_bytes(rows: Rows) -> int:
    total = 0
    for tile in range(TILE_COUNT):
        for y in range(TILE_H):
            for start, end in merge_intervals(rows[tile][y]):
                total += (end - start) * 2
    return total


def interval_count(rows: Rows) -> int:
    total = 0
    for tile in range(TILE_COUNT):
        for y in range(TILE_H):
            total += len(merge_intervals(rows[tile][y]))
    return total


def row_presence_bounds(rows: Rows) -> list[tuple[int, int] | None]:
    bounds: list[tuple[int, int] | None] = []
    for tile in range(TILE_COUNT):
        dirty = [y for y in range(TILE_H) if rows[tile][y]]
        bounds.append((min(dirty), max(dirty)) if dirty else None)
    return bounds


def build_upload_bands(rows: Rows) -> tuple[list[tuple[int, int, int]], bool]:
    bounds = row_presence_bounds(rows)
    bands: list[tuple[int, int, int]] = []
    capped = False

    for tile in range(TILE_COUNT):
        bound = bounds[tile]
        if bound is None or capped:
            continue
        y, max_y = bound
        while y <= max_y:
            while y <= max_y and not rows[tile][y]:
                y += 1
            if y > max_y:
                break
            start_y = y
            scan_y = y + 1
            last_dirty_y = y
            clean_gap = 0
            while scan_y <= max_y:
                if rows[tile][scan_y]:
                    last_dirty_y = scan_y
                    clean_gap = 0
                else:
                    clean_gap += 1
                    if clean_gap > UPLOAD_BAND_MERGE_GAP:
                        break
                scan_y += 1
            if len(bands) >= UPLOAD_MAX_RECTS:
                capped = True
                break
            bands.append((tile, start_y, last_dirty_y))
            y = last_dirty_y + 1

    return bands, capped


def full_width_upload_plan(rows: Rows) -> dict[str, int]:
    bands, capped = build_upload_bands(rows)
    if not capped:
        uploaded_rows = sum(end_y - start_y + 1 for _tile, start_y, end_y in bands)
        return {
            "bytes": uploaded_rows * TILE_W * 2,
            "rows": uploaded_rows,
            "rects": len(bands),
            "cap_hits": 0,
        }

    uploaded_rows = 0
    rects = 0
    for bound in row_presence_bounds(rows):
        if bound is None:
            continue
        uploaded_rows += bound[1] - bound[0] + 1
        rects += 1
    return {
        "bytes": uploaded_rows * TILE_W * 2,
        "rows": uploaded_rows,
        "rects": rects,
        "cap_hits": 1,
    }


def align_down(value: int, align: int) -> int:
    return (value // align) * align


def align_up(value: int, align: int) -> int:
    return ((value + align - 1) // align) * align


def xband_upload_plan(rows: Rows, align: int = 4) -> dict[str, int]:
    bands, capped = build_upload_bands(rows)
    if capped:
        fallback = full_width_upload_plan(rows)
        fallback["xband_cap_hits"] = 1
        return fallback

    total_bytes = 0
    total_rows = 0
    for tile, start_y, end_y in bands:
        min_x: int | None = None
        max_x: int | None = None
        for y in range(start_y, end_y + 1):
            for start, end in rows[tile][y]:
                min_x = start if min_x is None else min(min_x, start)
                max_x = end if max_x is None else max(max_x, end)
        if min_x is None or max_x is None:
            continue
        min_x = max(0, align_down(min_x, align))
        max_x = min(TILE_W, align_up(max_x, align))
        h = end_y - start_y + 1
        total_rows += h
        total_bytes += (max_x - min_x) * h * 2

    return {
        "bytes": total_bytes,
        "rows": total_rows,
        "rects": len(bands),
        "cap_hits": 0,
        "xband_cap_hits": 0,
    }


def parse_pack(path: Path) -> tuple[bytes, Header, list[Entry]]:
    payload = path.read_bytes()
    if len(payload) < HEADER_SIZE:
        raise ValueError(f"pack too small: {path}")
    unpacked = struct.unpack_from(HEADER, payload, 0)
    header = Header(*unpacked)
    if header.magic not in (b"FGP2", b"FGP3") or header.version not in (1, 2, 3, 4):
        raise ValueError(f"unsupported pack magic/version: {path}")
    if header.table_offset + header.frame_count * ENTRY_SIZE > len(payload):
        raise ValueError(f"entry table extends beyond pack: {path}")

    entries: list[Entry] = []
    for index in range(header.frame_count):
        values = struct.unpack_from(ENTRY, payload, header.table_offset + index * ENTRY_SIZE)
        entries.append(Entry(index, *values))
    return payload, header, entries


def parse_span_rows(data: bytes,
                    offset: int,
                    limit: int,
                    version: int,
                    base_x: int,
                    base_y: int,
                    with_pixel_payload: bool) -> tuple[Rows, int, int, int]:
    rows = empty_rows()
    spans = 0
    pixels = 0
    if offset + 2 > limit:
        return rows, offset, spans, pixels
    row_count = read_u16(data, offset)
    offset += 2
    for _row in range(row_count):
        if offset + 4 > limit:
            break
        rel_y = read_u16(data, offset)
        span_count = read_u16(data, offset + 2)
        offset += 4
        y = base_y + rel_y
        for _span in range(span_count):
            if offset + 4 > limit:
                break
            rel_x = read_u16(data, offset)
            pixel_count = read_u16(data, offset + 2)
            offset += 4
            add_screen_interval(rows, base_x + rel_x, base_x + rel_x + pixel_count, y)
            spans += 1
            pixels += pixel_count
            if with_pixel_payload:
                offset += (pixel_count + 1) // 2 if version == 1 else pixel_count
            if offset > limit:
                offset = limit
                break
    return merge_rows(rows), offset, spans, pixels


def parse_compact_span_rows(data: bytes,
                            offset: int,
                            limit: int,
                            base_x: int,
                            base_y: int,
                            with_pixel_payload: bool) -> tuple[Rows, int, int, int]:
    rows = empty_rows()
    spans = 0
    pixels = 0
    if offset + 2 > limit:
        return rows, offset, spans, pixels
    row_count = read_u16(data, offset)
    offset += 2
    for _row in range(row_count):
        rel_y, offset = read_compact_u16(data, offset, limit)
        span_count, offset = read_compact_u16(data, offset, limit)
        if rel_y is None or span_count is None:
            break
        y = base_y + rel_y
        for _span in range(span_count):
            rel_x, offset = read_compact_u16(data, offset, limit)
            pixel_count, offset = read_compact_u16(data, offset, limit)
            if rel_x is None or pixel_count is None:
                break
            add_screen_interval(rows, base_x + rel_x, base_x + rel_x + pixel_count, y)
            spans += 1
            pixels += pixel_count
            if with_pixel_payload:
                offset += (pixel_count + 1) // 2
            if offset > limit:
                offset = limit
                break
    return merge_rows(rows), offset, spans, pixels


def parse_compact_cleanup_rows(data: bytes,
                               offset: int,
                               limit: int,
                               base_x: int,
                               base_y: int) -> tuple[Rows, int, int, int]:
    return parse_compact_span_rows(
        data,
        offset,
        limit,
        base_x,
        base_y,
        with_pixel_payload=False,
    )


def active_entries(entries: list[Entry]) -> list[Entry]:
    if entries and entries[0].data_size == 0 and entries[0].width == 0 and entries[0].height == 0:
        return entries[1:]
    return entries


def analyze_pack(path: Path, island_x: int, island_y: int) -> dict[str, Any]:
    payload, header, entries = parse_pack(path)
    scene_relative = bool(header.flags & FLAG_SCENE_RELATIVE)
    scene_dx = island_x if scene_relative else 0
    scene_dy = island_y if scene_relative else 0

    totals = {
        "payload_bytes": 0,
        "restore_bytes": 0,
        "restore_intervals": 0,
        "compose_spans": 0,
        "compose_pixels": 0,
        "upload_bytes": 0,
        "upload_rows": 0,
        "upload_rects": 0,
        "upload_cap_hits": 0,
        "exact_upload_bytes": 0,
        "exact_upload_intervals": 0,
        "xband_align4_bytes": 0,
        "xband_align4_rects": 0,
        "xband_align4_cap_hits": 0,
    }
    maxima = {
        "upload_bytes": 0,
        "upload_rects": 0,
        "xband_align4_bytes": 0,
        "xband_align4_rects": 0,
        "exact_upload_intervals": 0,
        "restore_bytes": 0,
        "restore_intervals": 0,
    }

    prev_rows = full_rows()
    first = True
    for entry in active_entries(entries):
        entry_start = entry.data_offset
        entry_end = min(len(payload), entry.data_offset + entry.data_size)
        if entry.data_size == 0 or entry.width == 0 or entry.height == 0 or entry_start >= entry_end:
            continue

        base_x = entry.x + scene_dx
        base_y = entry.y + scene_dy
        totals["payload_bytes"] += entry.data_size

        if header.magic == b"FGP3":
            if header.version in (3, 4):
                cleanup_rows, offset, cleanup_spans, cleanup_pixels = parse_compact_cleanup_rows(
                    payload,
                    entry_start,
                    entry_end,
                    base_x,
                    base_y,
                )
            else:
                cleanup_rows, offset, cleanup_spans, cleanup_pixels = parse_span_rows(
                    payload,
                    entry_start,
                    entry_end,
                    header.version,
                    base_x,
                    base_y,
                    with_pixel_payload=False,
                )
            if header.version == 4:
                draw_rows, _offset, draw_spans, draw_pixels = parse_compact_span_rows(
                    payload,
                    offset,
                    entry_end,
                    base_x,
                    base_y,
                    with_pixel_payload=True,
                )
            else:
                draw_rows, _offset, draw_spans, draw_pixels = parse_span_rows(
                    payload,
                    offset,
                    entry_end,
                    1 if header.version == 3 else header.version,
                    base_x,
                    base_y,
                    with_pixel_payload=True,
                )
            dirty_rows = union_rows(full_rows() if first else empty_rows(), cleanup_rows, draw_rows)
            restore_bytes = interval_bytes(cleanup_rows)
            restore_intervals = interval_count(cleanup_rows)
            totals["compose_spans"] += draw_spans
            totals["compose_pixels"] += draw_pixels
            totals["restore_bytes"] += restore_bytes
            totals["restore_intervals"] += restore_intervals
            maxima["restore_bytes"] = max(maxima["restore_bytes"], restore_bytes)
            maxima["restore_intervals"] = max(maxima["restore_intervals"], restore_intervals)
        else:
            draw_rows, _offset, draw_spans, draw_pixels = parse_span_rows(
                payload,
                entry_start,
                entry_end,
                header.version,
                base_x,
                base_y,
                with_pixel_payload=True,
            )
            dirty_rows = union_rows(prev_rows, draw_rows)
            restore_bytes = interval_bytes(prev_rows)
            restore_intervals = interval_count(prev_rows)
            totals["compose_spans"] += draw_spans
            totals["compose_pixels"] += draw_pixels
            totals["restore_bytes"] += restore_bytes
            totals["restore_intervals"] += restore_intervals
            maxima["restore_bytes"] = max(maxima["restore_bytes"], restore_bytes)
            maxima["restore_intervals"] = max(maxima["restore_intervals"], restore_intervals)
            prev_rows = draw_rows

        full_upload = full_width_upload_plan(dirty_rows)
        exact_upload_bytes = interval_bytes(dirty_rows)
        exact_upload_intervals = interval_count(dirty_rows)
        xband = xband_upload_plan(dirty_rows, align=4)

        totals["upload_bytes"] += full_upload["bytes"]
        totals["upload_rows"] += full_upload["rows"]
        totals["upload_rects"] += full_upload["rects"]
        totals["upload_cap_hits"] += full_upload["cap_hits"]
        totals["exact_upload_bytes"] += exact_upload_bytes
        totals["exact_upload_intervals"] += exact_upload_intervals
        totals["xband_align4_bytes"] += xband["bytes"]
        totals["xband_align4_rects"] += xband["rects"]
        totals["xband_align4_cap_hits"] += xband.get("xband_cap_hits", 0)
        maxima["upload_bytes"] = max(maxima["upload_bytes"], full_upload["bytes"])
        maxima["upload_rects"] = max(maxima["upload_rects"], full_upload["rects"])
        maxima["xband_align4_bytes"] = max(maxima["xband_align4_bytes"], xband["bytes"])
        maxima["xband_align4_rects"] = max(maxima["xband_align4_rects"], xband["rects"])
        maxima["exact_upload_intervals"] = max(
            maxima["exact_upload_intervals"],
            exact_upload_intervals,
        )
        first = False

    upload_saved = totals["upload_bytes"] - totals["xband_align4_bytes"]
    upload_saved_percent = (
        upload_saved * 100.0 / totals["upload_bytes"]
        if totals["upload_bytes"] else 0.0
    )
    exact_saved = totals["upload_bytes"] - totals["exact_upload_bytes"]
    exact_saved_percent = (
        exact_saved * 100.0 / totals["upload_bytes"]
        if totals["upload_bytes"] else 0.0
    )
    upload_ready_payload = totals["xband_align4_bytes"] + totals["xband_align4_rects"] * 8
    payload_growth_percent = (
        (upload_ready_payload - totals["payload_bytes"]) * 100.0 / totals["payload_bytes"]
        if totals["payload_bytes"] else 0.0
    )
    xband_rect_pressure = (
        totals["xband_align4_rects"] / len(active_entries(entries))
        if active_entries(entries) else 0.0
    )

    return {
        "magic": header.magic.decode("ascii"),
        "encoding": "pal4" if header.version in (1, 3, 4) else "indexed8",
        "frame_count": header.frame_count,
        "active_frames": len(active_entries(entries)),
        "pack_bytes": len(payload),
        "payload_bytes": totals["payload_bytes"],
        "scene_relative": scene_relative,
        "sound_event_count": header.sound_event_count,
        "totals": totals,
        "maxima": maxima,
        "upload_xband_align4_saved_percent": upload_saved_percent,
        "upload_exact_saved_percent": exact_saved_percent,
        "upload_ready_payload_bytes": upload_ready_payload,
        "upload_ready_payload_growth_percent": payload_growth_percent,
        "xband_align4_rect_pressure": xband_rect_pressure,
    }


def make_output_row(matrix_row: dict[str, str],
                    analysis: dict[str, Any] | None,
                    error: str | None) -> dict[str, Any]:
    over_target_vb = safe_int(matrix_row.get("over_target_vb", ""))
    over_target_percent = safe_float(matrix_row.get("over_target_percent", ""))
    blocking_vb = safe_int(matrix_row.get("blocking_vb", ""))
    prefetch_overrun_vb = safe_int(matrix_row.get("prefetch_overrun_vb", ""))

    if analysis is None:
        return {
            "rank_score": 0.0,
            "scene_slug": matrix_row.get("scene_slug", ""),
            "tide": matrix_row.get("tide", ""),
            "source_pack": matrix_row.get("source_pack", ""),
            "status": matrix_row.get("status", ""),
            "stats_version": matrix_row.get("stats_version", ""),
            "loop_vb": matrix_row.get("loop_vb", ""),
            "target_vb": matrix_row.get("target_vb", ""),
            "over_target_vb": matrix_row.get("over_target_vb", ""),
            "over_target_percent": matrix_row.get("over_target_percent", ""),
            "blocking_vb": matrix_row.get("blocking_vb", ""),
            "prefetch_overrun_vb": matrix_row.get("prefetch_overrun_vb", ""),
            "magic": "",
            "encoding": "",
            "pack_bytes": matrix_row.get("pack_bytes", ""),
            "payload_bytes": "",
            "upload_bytes": "",
            "upload_rects": "",
            "max_upload_bytes": "",
            "max_upload_rects": "",
            "xband_align4_bytes": "",
            "xband_align4_saved_percent": "",
            "exact_upload_bytes": "",
            "exact_upload_saved_percent": "",
            "upload_ready_payload_bytes": "",
            "upload_ready_payload_growth_percent": "",
            "restore_bytes": "",
            "restore_intervals": "",
            "notes": error or "analysis unavailable",
        }

    totals = analysis["totals"]
    maxima = analysis["maxima"]
    visible_pressure = blocking_vb + prefetch_overrun_vb
    active_frames = analysis["active_frames"]
    xband_rect_pressure = (
        totals["xband_align4_rects"] / active_frames
        if active_frames else 0.0
    )
    rank_score = (
        over_target_vb * (analysis["upload_xband_align4_saved_percent"] / 100.0) +
        visible_pressure * 0.25
    )
    notes = "candidate"
    if totals["xband_align4_cap_hits"]:
        notes = "x-band rect cap pressure; needs selective bands"
    elif analysis["upload_ready_payload_growth_percent"] > 500.0:
        notes = "large upload-ready payload; needs compression/selective bands"
    elif xband_rect_pressure > 6.0:
        notes = "high x-band rect pressure; needs rect coalescing"
    elif matrix_row.get("status") != "measured":
        notes = "pending timing row; pack-only estimate"

    return {
        "rank_score": round(rank_score, 2),
        "scene_slug": matrix_row.get("scene_slug", ""),
        "tide": matrix_row.get("tide", ""),
        "source_pack": matrix_row.get("source_pack", ""),
        "status": matrix_row.get("status", ""),
        "stats_version": matrix_row.get("stats_version", ""),
        "loop_vb": matrix_row.get("loop_vb", ""),
        "target_vb": matrix_row.get("target_vb", ""),
        "over_target_vb": matrix_row.get("over_target_vb", ""),
        "over_target_percent": matrix_row.get("over_target_percent", ""),
        "blocking_vb": matrix_row.get("blocking_vb", ""),
        "prefetch_overrun_vb": matrix_row.get("prefetch_overrun_vb", ""),
        "magic": analysis["magic"],
        "encoding": analysis["encoding"],
        "pack_bytes": analysis["pack_bytes"],
        "payload_bytes": analysis["payload_bytes"],
        "upload_bytes": totals["upload_bytes"],
        "upload_rects": totals["upload_rects"],
        "max_upload_bytes": maxima["upload_bytes"],
        "max_upload_rects": maxima["upload_rects"],
        "xband_align4_bytes": totals["xband_align4_bytes"],
        "xband_align4_rects": totals["xband_align4_rects"],
        "xband_align4_cap_hits": totals["xband_align4_cap_hits"],
        "xband_align4_rect_pressure": round(analysis["xband_align4_rect_pressure"], 2),
        "max_xband_align4_bytes": maxima["xband_align4_bytes"],
        "max_xband_align4_rects": maxima["xband_align4_rects"],
        "xband_align4_saved_percent": round(analysis["upload_xband_align4_saved_percent"], 2),
        "exact_upload_bytes": totals["exact_upload_bytes"],
        "exact_upload_intervals": totals["exact_upload_intervals"],
        "max_exact_upload_intervals": maxima["exact_upload_intervals"],
        "exact_upload_saved_percent": round(analysis["upload_exact_saved_percent"], 2),
        "upload_ready_payload_bytes": analysis["upload_ready_payload_bytes"],
        "upload_ready_payload_growth_percent": round(analysis["upload_ready_payload_growth_percent"], 2),
        "restore_bytes": totals["restore_bytes"],
        "restore_intervals": totals["restore_intervals"],
        "notes": notes,
    }


def write_markdown(path: Path, rows: list[dict[str, Any]], csv_path: Path) -> None:
    measured = [
        row for row in rows
        if row["status"] == "measured" and
        row["upload_bytes"] != "" and
        row["over_target_percent"] != ""
    ]
    top = sorted(measured, key=lambda row: row["rank_score"], reverse=True)[:20]
    def timing_gap_percent(row: dict[str, Any]) -> float:
        loop_vb = safe_int(str(row.get("loop_vb", "")))
        target_vb = safe_int(str(row.get("target_vb", "")))
        if loop_vb > 0 and target_vb > 0:
            return (loop_vb - target_vb) * 100.0 / target_vb
        return float(row["over_target_percent"])

    avg_gap = (
        sum(timing_gap_percent(row) for row in measured) / len(measured)
        if measured else 0.0
    )
    avg_xband = (
        sum(float(row["xband_align4_saved_percent"]) for row in measured) / len(measured)
        if measured else 0.0
    )

    lines = [
        "# PS1 Foreground Preprocess Opportunity Matrix",
        "",
        "Status: generated host-side planning sheet",
        "",
        "This sheet ranks current FG2/FGP3 foreground packs for pack-time graphics",
        "preprocessing experiments. It does not change runtime behavior or the",
        "accepted performance baseline.",
        "",
        f"- Source CSV: `{csv_path.as_posix()}`",
        f"- Measured timing rows included: `{len(measured)}`",
        f"- Average measured timing gap: `{avg_gap:.4f}%` over target",
        f"- Average estimated align4 x-band upload byte saving: `{avg_xband:.2f}%`",
        "",
        "## Top Upload-Ready Candidates",
        "",
        "| Rank | Scene | Tide | Score | Gap | Upload Save | Payload Growth | Visible VB | Notes |",
        "|---:|---|---|---:|---:|---:|---:|---:|---|",
    ]
    for idx, row in enumerate(top, 1):
        visible = safe_int(str(row["blocking_vb"])) + safe_int(str(row["prefetch_overrun_vb"]))
        rect_pressure = row.get("xband_align4_rect_pressure", "")
        note = row["notes"]
        if rect_pressure != "":
            note = f"{note}; rect/frame {rect_pressure}"
        lines.append(
            f"| {idx} | `{row['scene_slug']}` | `{row['tide']}` | "
            f"{row['rank_score']} | {row['over_target_percent']}% | "
            f"{row['xband_align4_saved_percent']}% | "
            f"{row['upload_ready_payload_growth_percent']}% | {visible} | "
            f"{note} |"
        )
    lines.extend([
        "",
        "## Read Before Acting",
        "",
        "- High upload-byte savings are not automatically promotable; naive direct16",
        "  expansion already regressed WALKSTUF1 low by adding too much CD pressure.",
        "- Rows with large payload growth need selective bands, compression, or setup",
        "  residency before runtime promotion.",
        "- Use `scripts/analyze-fg2-preprocess-plans.py --hotspot-count N` on the",
        "  selected pack before a runtime probe. VISITOR3 now proves why: cap-hit frames",
        "  `134..136` save `0%` under blanket x-band, while nearby non-cap frames carry",
        "  most of the useful byte saving.",
        "- The current VISITOR3 detail sheet is",
        "  `docs/ps1/performance-preprocess-visitor3-hotspots.csv`. Its default",
        "  threshold plan selects `96 / 144` frames, excludes `3` cap-hit frames, and",
        "  estimates `6114568` selected-subset upload bytes saved.",
        "- This matrix should guide the next generated pack-format experiment, not",
        "  more hand-authored scene branches.",
        "",
    ])
    path.write_text("\n".join(lines), encoding="utf-8")


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Build an all-scene FG preprocessing opportunity matrix."
    )
    parser.add_argument("--matrix", type=Path, default=Path("docs/ps1/performance-scene-matrix.csv"))
    parser.add_argument("--output-csv", type=Path, default=Path("docs/ps1/performance-preprocess-opportunities.csv"))
    parser.add_argument("--output-md", type=Path, default=Path("docs/ps1/performance-preprocess-opportunities.md"))
    parser.add_argument("--island-x", type=int, default=-154)
    parser.add_argument("--island-y", type=int, default=54)
    parser.add_argument("--json", type=Path, default=None)
    args = parser.parse_args()

    with args.matrix.open(newline="", encoding="utf-8") as f:
        matrix_rows = list(csv.DictReader(f))

    cache: dict[str, tuple[dict[str, Any] | None, str | None]] = {}
    output_rows: list[dict[str, Any]] = []
    for matrix_row in matrix_rows:
        source_pack = matrix_row.get("source_pack", "")
        if source_pack not in cache:
            try:
                cache[source_pack] = (analyze_pack(Path(source_pack), args.island_x, args.island_y), None)
            except Exception as exc:  # noqa: BLE001 - report per-pack failures in the CSV.
                cache[source_pack] = (None, str(exc))
        analysis, error = cache[source_pack]
        output_rows.append(make_output_row(matrix_row, analysis, error))

    output_rows.sort(
        key=lambda row: (
            row["status"] != "measured",
            -float(row["rank_score"]),
            row["scene_slug"],
            row["tide"],
        )
    )

    fieldnames = [
        "rank_score",
        "scene_slug",
        "tide",
        "source_pack",
        "status",
        "stats_version",
        "loop_vb",
        "target_vb",
        "over_target_vb",
        "over_target_percent",
        "blocking_vb",
        "prefetch_overrun_vb",
        "magic",
        "encoding",
        "pack_bytes",
        "payload_bytes",
        "upload_bytes",
        "upload_rects",
        "max_upload_bytes",
        "max_upload_rects",
        "xband_align4_bytes",
        "xband_align4_rects",
        "xband_align4_cap_hits",
        "xband_align4_rect_pressure",
        "max_xband_align4_bytes",
        "max_xband_align4_rects",
        "xband_align4_saved_percent",
        "exact_upload_bytes",
        "exact_upload_intervals",
        "max_exact_upload_intervals",
        "exact_upload_saved_percent",
        "upload_ready_payload_bytes",
        "upload_ready_payload_growth_percent",
        "restore_bytes",
        "restore_intervals",
        "notes",
    ]
    args.output_csv.parent.mkdir(parents=True, exist_ok=True)
    with args.output_csv.open("w", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames, lineterminator="\n")
        writer.writeheader()
        writer.writerows(output_rows)

    write_markdown(args.output_md, output_rows, args.output_csv)
    if args.json is not None:
        args.json.write_text(json.dumps(output_rows, indent=2), encoding="utf-8")

    print(f"Wrote {len(output_rows)} rows to {args.output_csv}")
    print(f"Wrote summary to {args.output_md}")


if __name__ == "__main__":
    main()
