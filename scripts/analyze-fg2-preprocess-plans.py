#!/usr/bin/env python3
"""Estimate host-side preprocessing wins for foreground packs.

This tool is intentionally host-only. It decodes an existing FGP2/FGP3 pack
and replays the runtime dirty-state rules closely enough to compare today's
full-width row uploads against pack-emitted upload/restore plans.
"""

from __future__ import annotations

import argparse
import json
import struct
from dataclasses import dataclass
from pathlib import Path
from typing import Any


FG2_HEADER = "<4sHHHHHHHHHHIIIHH"
FG2_ENTRY = "<HhhHHHII"
FG2_HEADER_SIZE = struct.calcsize(FG2_HEADER)
FG2_ENTRY_SIZE = struct.calcsize(FG2_ENTRY)

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


@dataclass(frozen=True)
class FrameModel:
    entry: Entry
    draw_rows: "RowIntervals"
    draw_extents: "RowExtents"
    cleanup_rows: "RowIntervals"
    cleanup_spans: int
    cleanup_pixels: int
    draw_spans: int
    draw_pixels: int


RowIntervals = list[list[list[tuple[int, int]]]]
RowExtents = list[list[tuple[int, int] | None]]


def read_u16(data: bytes, offset: int) -> int:
    return struct.unpack_from("<H", data, offset)[0]


def merge_intervals(intervals: list[tuple[int, int]]) -> list[tuple[int, int]]:
    if not intervals:
        return []
    intervals = sorted(intervals)
    merged: list[tuple[int, int]] = []
    cur_start, cur_end = intervals[0]
    for start, end in intervals[1:]:
        if start <= cur_end:
            if end > cur_end:
                cur_end = end
        else:
            merged.append((cur_start, cur_end))
            cur_start, cur_end = start, end
    merged.append((cur_start, cur_end))
    return merged


def empty_intervals() -> RowIntervals:
    return [[[] for _ in range(TILE_H)] for _ in range(TILE_COUNT)]


def empty_extents() -> RowExtents:
    return [[None for _ in range(TILE_H)] for _ in range(TILE_COUNT)]


def full_tile_extents() -> RowExtents:
    return [[(0, TILE_W) for _ in range(TILE_H)] for _ in range(TILE_COUNT)]


def intervals_to_extents(rows: RowIntervals) -> RowExtents:
    out = empty_extents()
    for tile in range(TILE_COUNT):
        for y in range(TILE_H):
            merged = merge_intervals(rows[tile][y])
            if merged:
                out[tile][y] = (merged[0][0], merged[-1][1])
    return out


def extents_to_intervals(rows: RowExtents) -> RowIntervals:
    out = empty_intervals()
    for tile in range(TILE_COUNT):
        for y in range(TILE_H):
            extent = rows[tile][y]
            if extent is not None:
                out[tile][y] = [extent]
    return out


def union_intervals(a: RowIntervals, b: RowIntervals) -> RowIntervals:
    out = empty_intervals()
    for tile in range(TILE_COUNT):
        for y in range(TILE_H):
            out[tile][y] = merge_intervals(a[tile][y] + b[tile][y])
    return out


def union_extent_intervals(a: RowExtents, b: RowExtents) -> RowIntervals:
    return union_intervals(extents_to_intervals(a), extents_to_intervals(b))


def interval_bytes(rows: RowIntervals) -> int:
    total = 0
    for tile in range(TILE_COUNT):
        for y in range(TILE_H):
            for start, end in merge_intervals(rows[tile][y]):
                total += (end - start) * 2
    return total


def interval_count(rows: RowIntervals) -> int:
    total = 0
    for tile in range(TILE_COUNT):
        for y in range(TILE_H):
            total += len(merge_intervals(rows[tile][y]))
    return total


def extent_bytes(rows: RowExtents) -> int:
    total = 0
    for tile in range(TILE_COUNT):
        for y in range(TILE_H):
            extent = rows[tile][y]
            if extent is not None:
                total += (extent[1] - extent[0]) * 2
    return total


def dirty_row_count(rows: RowIntervals | RowExtents) -> int:
    total = 0
    for tile in range(TILE_COUNT):
        for y in range(TILE_H):
            row = rows[tile][y]
            if row:
                total += 1
    return total


def tile_for_screen_y(y: int) -> tuple[int, int]:
    if y < TILE_H:
        return 0, y
    return 2, y - TILE_H


def add_screen_interval(rows: RowIntervals, x0: int, x1: int, y: int) -> None:
    if y < 0 or y >= SCREEN_H:
        return
    if x0 < 0:
        x0 = 0
    if x1 > SCREEN_W:
        x1 = SCREEN_W
    if x0 >= x1:
        return

    left_tile, local_y = tile_for_screen_y(y)
    if x0 < TILE_W:
        lx0 = x0
        lx1 = min(x1, TILE_W)
        if lx0 < lx1:
            rows[left_tile][local_y].append((lx0, lx1))
    if x1 > TILE_W:
        rx0 = max(x0, TILE_W) - TILE_W
        rx1 = x1 - TILE_W
        right_tile = left_tile + 1
        if rx0 < rx1:
            rows[right_tile][local_y].append((rx0, rx1))


def parse_pack(path: Path) -> tuple[bytes, Header, list[Entry]]:
    payload = path.read_bytes()
    if len(payload) < FG2_HEADER_SIZE:
        raise SystemExit(f"foreground pack too small: {path}")

    (
        magic,
        version,
        frame_count,
        display_vblanks,
        flags,
        screen_width,
        screen_height,
        union_x,
        union_y,
        union_width,
        union_height,
        table_offset,
        data_offset,
        sound_events_offset,
        sound_event_count,
        reserved1,
    ) = struct.unpack_from(FG2_HEADER, payload, 0)

    if magic not in (b"FGP2", b"FGP3"):
        raise SystemExit(f"not an FGP2/FGP3 pack: {path}")
    if version not in (1, 2):
        raise SystemExit(f"unsupported foreground pack version {version}: {path}")
    if table_offset + frame_count * FG2_ENTRY_SIZE > len(payload):
        raise SystemExit(f"entry table extends beyond pack: {path}")

    header = Header(
        magic=magic,
        version=version,
        frame_count=frame_count,
        display_vblanks=display_vblanks,
        flags=flags,
        screen_width=screen_width,
        screen_height=screen_height,
        union_x=union_x,
        union_y=union_y,
        union_width=union_width,
        union_height=union_height,
        table_offset=table_offset,
        data_offset=data_offset,
        sound_events_offset=sound_events_offset,
        sound_event_count=sound_event_count,
        reserved1=reserved1,
    )

    entries: list[Entry] = []
    for index in range(frame_count):
        (
            source_frame,
            x,
            y,
            width,
            height,
            hold_vblanks,
            data_offset_entry,
            data_size,
        ) = struct.unpack_from(FG2_ENTRY, payload, table_offset + index * FG2_ENTRY_SIZE)
        if data_offset_entry + data_size > len(payload):
            raise SystemExit(f"entry {index} payload extends beyond pack: {path}")
        entries.append(
            Entry(
                index=index,
                source_frame=source_frame,
                x=x,
                y=y,
                width=width,
                height=height,
                hold_vblanks=hold_vblanks,
                data_offset=data_offset_entry,
                data_size=data_size,
            )
        )
    return payload, header, entries


def parse_span_rows(data: bytes,
                    offset: int,
                    limit: int,
                    version: int,
                    base_x: int,
                    base_y: int,
                    with_pixel_payload: bool) -> tuple[RowIntervals, int, int, int]:
    rows = empty_intervals()
    spans = 0
    pixels = 0
    if offset + 2 > limit:
        return rows, offset, spans, pixels
    row_count = read_u16(data, offset)
    offset += 2

    for _ in range(row_count):
        if offset + 4 > limit:
            break
        rel_y = read_u16(data, offset)
        span_count = read_u16(data, offset + 2)
        offset += 4
        screen_y = base_y + rel_y
        for _span in range(span_count):
            if offset + 4 > limit:
                break
            rel_x = read_u16(data, offset)
            pixel_count = read_u16(data, offset + 2)
            offset += 4
            screen_x = base_x + rel_x
            add_screen_interval(rows, screen_x, screen_x + pixel_count, screen_y)
            spans += 1
            pixels += pixel_count
            if with_pixel_payload:
                if version == 1:
                    offset += (pixel_count + 1) // 2
                else:
                    offset += pixel_count
            if offset > limit:
                offset = limit
                break

    for tile in range(TILE_COUNT):
        for y in range(TILE_H):
            rows[tile][y] = merge_intervals(rows[tile][y])
    return rows, offset, spans, pixels


def decode_entry_rows(payload: bytes, header: Header, entry: Entry,
                      scene_dx: int, scene_dy: int) -> RowIntervals:
    if entry.data_size == 0 or entry.width == 0 or entry.height == 0:
        return empty_intervals()

    data = payload[entry.data_offset:entry.data_offset + entry.data_size]
    base_x = entry.x + scene_dx
    base_y = entry.y + scene_dy
    rows, _offset, _spans, _pixels = parse_span_rows(
        data,
        0,
        len(data),
        header.version,
        base_x,
        base_y,
        with_pixel_payload=True,
    )
    return rows


def build_frame_model(payload: bytes, header: Header, entry: Entry,
                      scene_dx: int, scene_dy: int) -> FrameModel:
    cleanup_rows = empty_intervals()
    cleanup_spans = 0
    cleanup_pixels = 0
    draw_rows = empty_intervals()
    draw_spans = 0
    draw_pixels = 0
    if entry.data_size != 0 and entry.width != 0 and entry.height != 0:
        data = payload[entry.data_offset:entry.data_offset + entry.data_size]
        base_x = entry.x + scene_dx
        base_y = entry.y + scene_dy
        if header.magic == b"FGP3":
            cleanup_rows, offset, cleanup_spans, cleanup_pixels = parse_span_rows(
                data,
                0,
                len(data),
                header.version,
                base_x,
                base_y,
                with_pixel_payload=False,
            )
            draw_rows, _offset, draw_spans, draw_pixels = parse_span_rows(
                data,
                offset,
                len(data),
                header.version,
                base_x,
                base_y,
                with_pixel_payload=True,
            )
        else:
            draw_rows, _offset, draw_spans, draw_pixels = parse_span_rows(
                data,
                0,
                len(data),
                header.version,
                base_x,
                base_y,
                with_pixel_payload=True,
            )
    return FrameModel(
        entry=entry,
        draw_rows=draw_rows,
        draw_extents=intervals_to_extents(draw_rows),
        cleanup_rows=cleanup_rows,
        cleanup_spans=cleanup_spans,
        cleanup_pixels=cleanup_pixels,
        draw_spans=draw_spans,
        draw_pixels=draw_pixels,
    )


def draw_bounds(entries: list[Entry], scene_dx: int, scene_dy: int) -> tuple[int, int, int, int] | None:
    min_x: int | None = None
    min_y: int | None = None
    max_x: int | None = None
    max_y: int | None = None
    for entry in entries:
        if entry.width == 0 or entry.height == 0 or entry.data_size == 0:
            continue
        x0 = entry.x + scene_dx
        y0 = entry.y + scene_dy
        x1 = x0 + entry.width
        y1 = y0 + entry.height
        min_x = x0 if min_x is None else min(min_x, x0)
        min_y = y0 if min_y is None else min(min_y, y0)
        max_x = x1 if max_x is None else max(max_x, x1)
        max_y = y1 if max_y is None else max(max_y, y1)
    if min_x is None or min_y is None or max_x is None or max_y is None:
        return None
    return min_x, min_y, max_x - min_x, max_y - min_y


def clean_rects_for_pack(bounds: tuple[int, int, int, int] | None) -> list[tuple[int, int, int, int]]:
    wave_min_x = 129
    wave_min_y = 303
    wave_end_x = 608
    wave_end_y = 356
    upper_split_y = 190

    if bounds is None or bounds[2] == 0 or bounds[3] == 0:
        return [(wave_min_x, wave_min_y, wave_end_x - wave_min_x, wave_end_y - wave_min_y)]

    fg_x, fg_y, fg_w, fg_h = bounds
    fg_end_x = fg_x + fg_w
    fg_end_y = fg_y + fg_h

    lower_min_x = fg_x
    lower_min_y = max(fg_y, upper_split_y)
    lower_end_x = fg_end_x
    lower_end_y = fg_end_y

    lower_min_x = min(wave_min_x, lower_min_x)
    lower_min_y = min(wave_min_y, lower_min_y)
    lower_end_x = max(wave_end_x, lower_end_x)
    lower_end_y = max(wave_end_y, lower_end_y)

    lower_min_x = max(0, lower_min_x)
    lower_min_y = max(0, lower_min_y)
    lower_end_x = min(SCREEN_W, lower_end_x)
    lower_end_y = min(SCREEN_H, lower_end_y)

    if fg_y < upper_split_y:
        upper_min_x = max(0, fg_x)
        upper_min_y = max(0, fg_y)
        upper_end_x = min(SCREEN_W, fg_end_x)
        upper_end_y = min(SCREEN_H, upper_split_y)
        if (upper_end_x <= upper_min_x or upper_end_y <= upper_min_y or
                lower_end_x <= lower_min_x or lower_end_y <= lower_min_y):
            return []
        return [
            (lower_min_x, lower_min_y, lower_end_x - lower_min_x, lower_end_y - lower_min_y),
            (upper_min_x, upper_min_y, upper_end_x - upper_min_x, upper_end_y - upper_min_y),
        ]

    if lower_end_x <= lower_min_x or lower_end_y <= lower_min_y:
        return []
    return [(lower_min_x, lower_min_y, lower_end_x - lower_min_x, lower_end_y - lower_min_y)]


def clip_extent_to_screen_rect(tile: int, y: int, extent: tuple[int, int],
                               rect: tuple[int, int, int, int]) -> tuple[int, int] | None:
    rect_x, rect_y, rect_w, rect_h = rect
    screen_y = y if tile < 2 else y + TILE_H
    if screen_y < rect_y or screen_y >= rect_y + rect_h:
        return None

    tile_screen_x = 0 if tile in (0, 2) else TILE_W
    x0 = max(extent[0] + tile_screen_x, rect_x)
    x1 = min(extent[1] + tile_screen_x, rect_x + rect_w)
    x0 = max(0, x0)
    x1 = min(SCREEN_W, x1)
    if x0 >= x1:
        return None
    return x0 - tile_screen_x, x1 - tile_screen_x


def restore_plan_for_extents(prev: RowExtents,
                             clean_rects: list[tuple[int, int, int, int]]) -> dict[str, int]:
    total = 0
    intervals = 0
    for tile in range(TILE_COUNT):
        for y in range(TILE_H):
            extent = prev[tile][y]
            if extent is None:
                continue
            for rect in clean_rects:
                clipped = clip_extent_to_screen_rect(tile, y, extent, rect)
                if clipped is not None:
                    total += (clipped[1] - clipped[0]) * 2
                    intervals += 1
    return {"bytes": total, "intervals": intervals}


def subtract_intervals(base: list[tuple[int, int]],
                       cut: list[tuple[int, int]]) -> list[tuple[int, int]]:
    pieces = merge_intervals(base)
    for cut_start, cut_end in merge_intervals(cut):
        next_pieces: list[tuple[int, int]] = []
        for start, end in pieces:
            if cut_end <= start or cut_start >= end:
                next_pieces.append((start, end))
                continue
            if start < cut_start:
                next_pieces.append((start, cut_start))
            if cut_end < end:
                next_pieces.append((cut_end, end))
        pieces = next_pieces
        if not pieces:
            break
    return pieces


def restore_rows_minus_current_plan(cleanup_rows: RowIntervals,
                                    current_exact: RowIntervals) -> dict[str, int]:
    total = 0
    intervals = 0
    for tile in range(TILE_COUNT):
        for y in range(TILE_H):
            for start, end in subtract_intervals(cleanup_rows[tile][y], current_exact[tile][y]):
                total += (end - start) * 2
                intervals += 1
    return {"bytes": total, "intervals": intervals}


def restore_minus_current_plan(prev: RowExtents, current_exact: RowIntervals,
                               clean_rects: list[tuple[int, int, int, int]]) -> dict[str, int]:
    total = 0
    intervals = 0
    for tile in range(TILE_COUNT):
        for y in range(TILE_H):
            extent = prev[tile][y]
            if extent is None:
                continue
            clipped_for_rects: list[tuple[int, int]] = []
            for rect in clean_rects:
                clipped = clip_extent_to_screen_rect(tile, y, extent, rect)
                if clipped is not None:
                    clipped_for_rects.append(clipped)
            for start, end in subtract_intervals(clipped_for_rects, current_exact[tile][y]):
                total += (end - start) * 2
                intervals += 1
    return {"bytes": total, "intervals": intervals}


def restore_threshold_plan(prev: RowExtents, current_exact: RowIntervals,
                           clean_rects: list[tuple[int, int, int, int]],
                           min_saved_pixels: int,
                           max_pieces: int) -> dict[str, int]:
    total = 0
    intervals = 0
    for tile in range(TILE_COUNT):
        for y in range(TILE_H):
            extent = prev[tile][y]
            if extent is None:
                continue
            base: list[tuple[int, int]] = []
            for rect in clean_rects:
                clipped = clip_extent_to_screen_rect(tile, y, extent, rect)
                if clipped is not None:
                    base.append(clipped)
            base = merge_intervals(base)
            if not base:
                continue

            pieces = subtract_intervals(base, current_exact[tile][y])
            base_pixels = sum(end - start for start, end in base)
            piece_pixels = sum(end - start for start, end in pieces)
            saved_pixels = base_pixels - piece_pixels
            if saved_pixels >= min_saved_pixels and len(pieces) <= max_pieces:
                chosen = pieces
            else:
                chosen = base
            intervals += len(chosen)
            total += sum((end - start) * 2 for start, end in chosen)
    return {"bytes": total, "intervals": intervals}


def row_presence_bounds(rows: RowIntervals) -> list[tuple[int, int] | None]:
    bounds: list[tuple[int, int] | None] = []
    for tile in range(TILE_COUNT):
        min_y: int | None = None
        max_y: int | None = None
        for y in range(TILE_H):
            if rows[tile][y]:
                min_y = y if min_y is None else min(min_y, y)
                max_y = y if max_y is None else max(max_y, y)
        bounds.append(None if min_y is None or max_y is None else (min_y, max_y))
    return bounds


def full_width_upload_plan(rows: RowIntervals) -> dict[str, int]:
    bounds = row_presence_bounds(rows)
    dirty_count = sum(1 for item in bounds if item is not None)
    bands: list[tuple[int, int, int]] = []
    capped = False

    for tile in range(TILE_COUNT):
        if bounds[tile] is None or capped:
            continue
        y, max_y = bounds[tile]
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
            y = last_dirty_y
            if len(bands) >= UPLOAD_MAX_RECTS:
                capped = True
                break
            bands.append((tile, start_y, y))
            y += 1

    if not capped and bands:
        rows_uploaded = sum(end_y - start_y + 1 for _tile, start_y, end_y in bands)
        return {
            "rects": len(bands),
            "rows": rows_uploaded,
            "bytes": rows_uploaded * TILE_W * 2,
            "cap_hit": 0,
            "fallback": 0,
        }

    if dirty_count == 0:
        return {"rects": 0, "rows": 0, "bytes": 0, "cap_hit": 0, "fallback": 0}

    rows_uploaded = 0
    rects = 0
    for item in bounds:
        if item is None:
            continue
        rows_uploaded += item[1] - item[0] + 1
        rects += 1
    return {
        "rects": rects,
        "rows": rows_uploaded,
        "bytes": rows_uploaded * TILE_W * 2,
        "cap_hit": 1 if capped else 0,
        "fallback": 1,
    }


def align_down(value: int, align: int) -> int:
    return (value // align) * align


def align_up(value: int, align: int) -> int:
    return ((value + align - 1) // align) * align


def xband_upload_plan(rows: RowIntervals, align: int) -> dict[str, int]:
    bounds = row_presence_bounds(rows)
    bands: list[tuple[int, int, int]] = []
    capped = False

    for tile in range(TILE_COUNT):
        if bounds[tile] is None or capped:
            continue
        y, max_y = bounds[tile]
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
            y = last_dirty_y
            if len(bands) >= UPLOAD_MAX_RECTS:
                capped = True
                break
            bands.append((tile, start_y, y))
            y += 1

    if capped:
        full = full_width_upload_plan(rows)
        full["xband_cap_hit"] = 1
        return full

    total_bytes = 0
    total_rows = 0
    for tile, start_y, end_y in bands:
        band_min_x: int | None = None
        band_max_x: int | None = None
        for y in range(start_y, end_y + 1):
            for start, end in rows[tile][y]:
                band_min_x = start if band_min_x is None else min(band_min_x, start)
                band_max_x = end if band_max_x is None else max(band_max_x, end)
        if band_min_x is None or band_max_x is None:
            continue
        band_min_x = max(0, align_down(band_min_x, align))
        band_max_x = min(TILE_W, align_up(band_max_x, align))
        h = end_y - start_y + 1
        total_rows += h
        total_bytes += (band_max_x - band_min_x) * h * 2

    return {
        "rects": len(bands),
        "rows": total_rows,
        "bytes": total_bytes,
        "cap_hit": 0,
        "fallback": 0,
        "align_px": align,
    }


def pct_saved(current: int, proposed: int) -> float:
    if current <= 0:
        return 0.0
    return round(((current - proposed) * 100.0) / current, 2)


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Analyze host-side FG2/FGP3 preprocessing upload/restore opportunities."
    )
    parser.add_argument("pack", type=Path)
    parser.add_argument("--island-x", type=int, default=0)
    parser.add_argument("--island-y", type=int, default=0)
    parser.add_argument(
        "--no-scene-relative-offset",
        action="store_true",
        help="Do not apply --island-x/--island-y even when the pack is scene-relative.",
    )
    parser.add_argument(
        "--no-initial-full-dirty",
        action="store_true",
        help="Do not model the runtime's first-frame full-screen upload.",
    )
    parser.add_argument(
        "--hotspot-count",
        type=int,
        default=12,
        help="Number of per-frame upload hotspot rows to include.",
    )
    parser.add_argument("--pretty", action="store_true", help="Pretty-print JSON output.")
    args = parser.parse_args()

    payload, header, entries = parse_pack(args.pack)
    apply_scene_relative = (
        (header.flags & FLAG_SCENE_RELATIVE) != 0 and
        not args.no_scene_relative_offset
    )
    scene_dx = args.island_x if apply_scene_relative else 0
    scene_dy = args.island_y if apply_scene_relative else 0

    active_entries = entries
    if entries and entries[0].data_size == 0:
        active_entries = entries[1:]

    frame_models = [
        build_frame_model(payload, header, entry, scene_dx, scene_dy)
        for entry in active_entries
    ]
    frame_exact = [model.draw_rows for model in frame_models]
    frame_extents = [model.draw_extents for model in frame_models]

    bounds = draw_bounds(entries, scene_dx, scene_dy)
    clean_rects = clean_rects_for_pack(bounds)

    prev_extents = empty_extents()
    prev_exact = empty_intervals()
    if not args.no_initial_full_dirty:
        prev_extents = full_tile_extents()
        prev_exact = extents_to_intervals(prev_extents)

    totals = {
        "restore_runtime_bytes": 0,
        "restore_runtime_intervals": 0,
        "restore_minus_current_exact_bytes": 0,
        "restore_minus_current_exact_intervals": 0,
        "upload_runtime_full_width_bytes": 0,
        "upload_runtime_full_width_rows": 0,
        "upload_runtime_full_width_rects": 0,
        "upload_runtime_cap_hits": 0,
        "upload_xband_extent_bytes_align1": 0,
        "upload_xband_extent_bytes_align4": 0,
        "upload_xband_extent_bytes_align16": 0,
        "upload_xband_exact_bytes_align1": 0,
        "upload_xband_exact_bytes_align4": 0,
        "upload_xband_exact_bytes_align16": 0,
        "upload_xband_exact_rects": 0,
        "upload_xband_exact_cap_hits": 0,
        "upload_exact_interval_bytes": 0,
        "upload_exact_interval_rects": 0,
        "upload_extent_row_bytes": 0,
        "cleanup_spans": 0,
        "cleanup_pixels": 0,
        "compose_spans": 0,
        "compose_pixels": 0,
    }
    maxima = {
        "restore_runtime_bytes": 0,
        "restore_runtime_intervals": 0,
        "restore_minus_current_exact_bytes": 0,
        "restore_minus_current_exact_intervals": 0,
        "upload_runtime_full_width_bytes": 0,
        "upload_runtime_full_width_rects": 0,
        "upload_xband_exact_bytes_align4": 0,
        "upload_xband_exact_rects": 0,
    }
    frame_hotspots: list[dict[str, Any]] = []

    for frame_index, model in enumerate(frame_models):
        exact_rows = model.draw_rows
        extent_rows = model.draw_extents
        first_frame_full_dirty = not args.no_initial_full_dirty and frame_index == 0

        if header.magic == b"FGP3":
            restore_bytes = interval_bytes(model.cleanup_rows)
            restore_plan = {
                "bytes": restore_bytes,
                "intervals": interval_count(model.cleanup_rows),
            }
            restore_skipped = restore_rows_minus_current_plan(model.cleanup_rows, exact_rows)
            union_exact = union_intervals(model.cleanup_rows, exact_rows)
            if first_frame_full_dirty:
                union_exact = union_intervals(extents_to_intervals(full_tile_extents()), union_exact)
            union_extent = extents_to_intervals(intervals_to_extents(union_exact))
        else:
            restore_plan = restore_plan_for_extents(prev_extents, clean_rects)
            restore_bytes = restore_plan["bytes"]
            restore_skipped = restore_minus_current_plan(prev_extents, exact_rows, clean_rects)
            union_extent = union_extent_intervals(prev_extents, extent_rows)
            union_exact = union_intervals(prev_exact, exact_rows)
        full_plan = full_width_upload_plan(union_extent)

        x_extent_1 = xband_upload_plan(union_extent, 1)
        x_extent_4 = xband_upload_plan(union_extent, 4)
        x_extent_16 = xband_upload_plan(union_extent, 16)
        x_exact_1 = xband_upload_plan(union_exact, 1)
        x_exact_4 = xband_upload_plan(union_exact, 4)
        x_exact_16 = xband_upload_plan(union_exact, 16)
        exact_interval_bytes = interval_bytes(union_exact)
        exact_interval_rects = interval_count(union_exact)
        x_exact_4_cap_hit = x_exact_4.get("xband_cap_hit", x_exact_4.get("cap_hit", 0))

        totals["restore_runtime_bytes"] += restore_bytes
        totals["restore_runtime_intervals"] += restore_plan["intervals"]
        totals["restore_minus_current_exact_bytes"] += restore_skipped["bytes"]
        totals["restore_minus_current_exact_intervals"] += restore_skipped["intervals"]
        totals["upload_runtime_full_width_bytes"] += full_plan["bytes"]
        totals["upload_runtime_full_width_rows"] += full_plan["rows"]
        totals["upload_runtime_full_width_rects"] += full_plan["rects"]
        totals["upload_runtime_cap_hits"] += full_plan["cap_hit"]
        totals["upload_xband_extent_bytes_align1"] += x_extent_1["bytes"]
        totals["upload_xband_extent_bytes_align4"] += x_extent_4["bytes"]
        totals["upload_xband_extent_bytes_align16"] += x_extent_16["bytes"]
        totals["upload_xband_exact_bytes_align1"] += x_exact_1["bytes"]
        totals["upload_xband_exact_bytes_align4"] += x_exact_4["bytes"]
        totals["upload_xband_exact_bytes_align16"] += x_exact_16["bytes"]
        totals["upload_xband_exact_rects"] += x_exact_4["rects"]
        totals["upload_xband_exact_cap_hits"] += x_exact_4_cap_hit
        totals["upload_exact_interval_bytes"] += exact_interval_bytes
        totals["upload_exact_interval_rects"] += exact_interval_rects
        totals["upload_extent_row_bytes"] += extent_bytes(intervals_to_extents(union_exact))
        totals["cleanup_spans"] += model.cleanup_spans
        totals["cleanup_pixels"] += model.cleanup_pixels
        totals["compose_spans"] += model.draw_spans
        totals["compose_pixels"] += model.draw_pixels

        maxima["restore_runtime_bytes"] = max(maxima["restore_runtime_bytes"], restore_bytes)
        maxima["restore_runtime_intervals"] = max(
            maxima["restore_runtime_intervals"],
            restore_plan["intervals"],
        )
        maxima["restore_minus_current_exact_bytes"] = max(
            maxima["restore_minus_current_exact_bytes"],
            restore_skipped["bytes"],
        )
        maxima["restore_minus_current_exact_intervals"] = max(
            maxima["restore_minus_current_exact_intervals"],
            restore_skipped["intervals"],
        )
        maxima["upload_runtime_full_width_bytes"] = max(
            maxima["upload_runtime_full_width_bytes"],
            full_plan["bytes"],
        )
        maxima["upload_runtime_full_width_rects"] = max(
            maxima["upload_runtime_full_width_rects"],
            full_plan["rects"],
        )
        maxima["upload_xband_exact_bytes_align4"] = max(
            maxima["upload_xband_exact_bytes_align4"],
            x_exact_4["bytes"],
        )
        maxima["upload_xband_exact_rects"] = max(
            maxima["upload_xband_exact_rects"],
            x_exact_4["rects"],
        )

        saved_bytes = full_plan["bytes"] - x_exact_4["bytes"]
        frame_hotspots.append({
            "frame_index": frame_index,
            "source_frame": model.entry.source_frame,
            "hold_vblanks": model.entry.hold_vblanks,
            "entry_data_size": model.entry.data_size,
            "initial_full_dirty": first_frame_full_dirty,
            "cleanup_bytes": interval_bytes(model.cleanup_rows),
            "cleanup_intervals": interval_count(model.cleanup_rows),
            "draw_exact_bytes": interval_bytes(exact_rows),
            "draw_exact_intervals": interval_count(exact_rows),
            "runtime_full_width_bytes": full_plan["bytes"],
            "runtime_full_width_rows": full_plan["rows"],
            "runtime_full_width_rects": full_plan["rects"],
            "runtime_cap_hit": full_plan["cap_hit"],
            "xband_exact_align4_bytes": x_exact_4["bytes"],
            "xband_exact_align4_rects": x_exact_4["rects"],
            "xband_exact_align4_cap_hit": x_exact_4_cap_hit,
            "xband_exact_align4_saved_bytes": saved_bytes,
            "xband_exact_align4_saved_percent": pct_saved(full_plan["bytes"], x_exact_4["bytes"]),
            "exact_interval_upload_bytes": exact_interval_bytes,
            "exact_interval_upload_rects": exact_interval_rects,
        })

        if header.magic == b"FGP2":
            prev_extents = extent_rows
            prev_exact = exact_rows

    current_upload = totals["upload_runtime_full_width_bytes"]
    current_restore = totals["restore_runtime_bytes"]
    payload_bytes = sum(entry.data_size for entry in active_entries)
    pack_metadata_bytes = header.data_offset
    restore_profiles: dict[str, dict[str, int]] = {
        "exact": {
            "bytes": totals["restore_minus_current_exact_bytes"],
            "intervals": totals["restore_minus_current_exact_intervals"],
            "max_bytes": maxima["restore_minus_current_exact_bytes"],
            "max_intervals": maxima["restore_minus_current_exact_intervals"],
        }
    }

    if header.magic == b"FGP2":
        for name, min_saved_pixels, max_pieces in (
            ("min8px_max4pieces", 8, 4),
            ("min16px_max3pieces", 16, 3),
            ("min32px_max2pieces", 32, 2),
            ("min64px_max2pieces", 64, 2),
            ("full_cover_only", 1, 0),
        ):
            total_bytes = 0
            total_intervals = 0
            max_bytes = 0
            max_intervals = 0
            prev_for_profile = empty_extents()
            if not args.no_initial_full_dirty:
                prev_for_profile = full_tile_extents()
            for exact_rows, _extent_rows in zip(frame_exact, frame_extents):
                plan = restore_threshold_plan(
                    prev_for_profile,
                    exact_rows,
                    clean_rects,
                    min_saved_pixels=min_saved_pixels,
                    max_pieces=max_pieces,
                )
                total_bytes += plan["bytes"]
                total_intervals += plan["intervals"]
                max_bytes = max(max_bytes, plan["bytes"])
                max_intervals = max(max_intervals, plan["intervals"])
                prev_for_profile = intervals_to_extents(exact_rows)
            restore_profiles[name] = {
                "bytes": total_bytes,
                "intervals": total_intervals,
                "max_bytes": max_bytes,
                "max_intervals": max_intervals,
            }

    for profile in restore_profiles.values():
        profile["saved_bytes"] = current_restore - profile["bytes"]
        profile["saved_percent"] = pct_saved(current_restore, profile["bytes"])
        profile["estimated_interval_metadata_bytes"] = profile["intervals"] * 6

    opportunities = {
        "restore_skip_under_current_exact": {
            "bytes": totals["restore_minus_current_exact_bytes"],
            "intervals": totals["restore_minus_current_exact_intervals"],
            "estimated_interval_metadata_bytes": (
                totals["restore_minus_current_exact_intervals"] * 6
            ),
            "saved_bytes": current_restore - totals["restore_minus_current_exact_bytes"],
            "saved_percent": pct_saved(current_restore, totals["restore_minus_current_exact_bytes"]),
            "note": "Exact byte win is large, but interval count must be coalesced before runtime promotion.",
        },
        "upload_xband_extent_align4": {
            "bytes": totals["upload_xband_extent_bytes_align4"],
            "estimated_payload_plus_rect_metadata_bytes": (
                totals["upload_xband_extent_bytes_align4"] +
                totals["upload_xband_exact_rects"] * 8
            ),
            "saved_bytes": current_upload - totals["upload_xband_extent_bytes_align4"],
            "saved_percent": pct_saved(current_upload, totals["upload_xband_extent_bytes_align4"]),
            "note": "Requires upload-ready contiguous band payloads or a scratch packer; current tile RAM is strided.",
        },
        "upload_xband_exact_align4": {
            "bytes": totals["upload_xband_exact_bytes_align4"],
            "estimated_payload_plus_rect_metadata_bytes": (
                totals["upload_xband_exact_bytes_align4"] +
                totals["upload_xband_exact_rects"] * 8
            ),
            "saved_bytes": current_upload - totals["upload_xband_exact_bytes_align4"],
            "saved_percent": pct_saved(current_upload, totals["upload_xband_exact_bytes_align4"]),
            "note": "Requires exact visual-change metadata; this is closer to an FGP3 direct-upload payload.",
        },
        "upload_exact_interval_ideal": {
            "bytes": totals["upload_exact_interval_bytes"],
            "rects": totals["upload_exact_interval_rects"],
            "estimated_payload_plus_rect_metadata_bytes": (
                totals["upload_exact_interval_bytes"] +
                totals["upload_exact_interval_rects"] * 8
            ),
            "saved_bytes": current_upload - totals["upload_exact_interval_bytes"],
            "saved_percent": pct_saved(current_upload, totals["upload_exact_interval_bytes"]),
            "note": "Upper-bound byte floor before rect-count and contiguous-source constraints.",
        },
    }
    hotspot_limit = max(0, args.hotspot_count)
    frame_cap_hotspots = sorted(
        frame_hotspots,
        key=lambda row: (
            row["xband_exact_align4_cap_hit"],
            row["runtime_full_width_bytes"],
            row["xband_exact_align4_saved_bytes"],
        ),
        reverse=True,
    )[:hotspot_limit]
    frame_saving_hotspots = sorted(
        frame_hotspots,
        key=lambda row: (
            row["xband_exact_align4_saved_bytes"],
            row["xband_exact_align4_saved_percent"],
            row["runtime_full_width_bytes"],
        ),
        reverse=True,
    )[:hotspot_limit]

    result: dict[str, Any] = {
        "pack": str(args.pack),
        "pack_bytes": len(payload),
        "pack_metadata_bytes": pack_metadata_bytes,
        "active_payload_bytes": payload_bytes,
        "header": {
            "magic": header.magic.decode("ascii"),
            "version": header.version,
            "frame_count": header.frame_count,
            "flags": header.flags,
            "scene_relative": bool(header.flags & FLAG_SCENE_RELATIVE),
            "screen_width": header.screen_width,
            "screen_height": header.screen_height,
            "sound_event_count": header.sound_event_count,
        },
        "analysis": {
            "scene_dx": scene_dx,
            "scene_dy": scene_dy,
            "active_frames": len(active_entries),
            "skipped_leading_empty": len(active_entries) != len(entries),
            "initial_full_dirty": not args.no_initial_full_dirty,
            "clean_rects": [
                {"x": x, "y": y, "width": w, "height": h}
                for x, y, w, h in clean_rects
            ],
        },
        "runtime_model": {
            "restore_bytes": current_restore,
            "restore_intervals": totals["restore_runtime_intervals"],
            "max_restore_bytes": maxima["restore_runtime_bytes"],
            "max_restore_intervals": maxima["restore_runtime_intervals"],
            "upload_bytes": current_upload,
            "upload_rows": totals["upload_runtime_full_width_rows"],
            "upload_rects": totals["upload_runtime_full_width_rects"],
            "max_upload_bytes": maxima["upload_runtime_full_width_bytes"],
            "max_upload_rects": maxima["upload_runtime_full_width_rects"],
            "cap_hits": totals["upload_runtime_cap_hits"],
        },
        "preprocess_totals": totals,
        "preprocess_maxima": maxima,
        "restore_skip_profiles": restore_profiles,
        "opportunities": opportunities,
        "frame_cap_hotspots": frame_cap_hotspots,
        "frame_saving_hotspots": frame_saving_hotspots,
        "frame_hotspots": frame_cap_hotspots,
        "red_team": [
            "Pack-emitted narrow multi-row uploads cannot read directly from current tile RAM; the rows are strided.",
            "An FGP3 direct-upload payload lowers runtime work but increases pack/CD bytes, so CD pressure must be gated.",
            "Restore-under-current skipping is only safe against exact opaque current spans, not row min/max dirty extents.",
            "FGP3 cleanup rows are already explicit; FGP2 coalesced restore profiles are not generated for FGP3 packs.",
            "The first active frame currently needs a full-screen upload because setup forces all background tiles dirty.",
        ],
    }

    print(json.dumps(result, indent=2 if args.pretty else None, sort_keys=args.pretty))


if __name__ == "__main__":
    main()
