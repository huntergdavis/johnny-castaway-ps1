#!/usr/bin/env python3
"""Find translation-only motion-compensation candidates in FG2 packs.

This is host-only analysis for a future pack/runtime experiment. It does not
claim that GPU MoveImage is safe by itself; it measures whether consecutive
FG2 frames contain enough translated pixels to justify designing that format.
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
UPLOAD_BAND_MERGE_GAP = 1


@dataclass(frozen=True)
class Header:
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
class DecodedFrame:
    entry: Entry
    pixels: dict[tuple[int, int], int]


def read_u16(data: bytes, offset: int) -> int:
    return struct.unpack_from("<H", data, offset)[0]


def parse_pack(path: Path) -> tuple[bytes, Header, list[Entry]]:
    payload = path.read_bytes()
    if len(payload) < FG2_HEADER_SIZE:
        raise SystemExit(f"FG2 pack too small: {path}")

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

    if magic != b"FGP2":
        raise SystemExit(f"not an FGP2 pack: {path}")
    if version not in (1, 2):
        raise SystemExit(f"unsupported FGP2 version {version}: {path}")
    if table_offset + frame_count * FG2_ENTRY_SIZE > len(payload):
        raise SystemExit(f"entry table extends beyond pack: {path}")

    header = Header(
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


def pal4_index(data: bytes, pixel_index: int) -> int:
    packed = data[pixel_index >> 1]
    if pixel_index & 1:
        return packed & 0x0F
    return (packed >> 4) & 0x0F


def decode_entry_pixels(payload: bytes, header: Header, entry: Entry,
                        scene_dx: int, scene_dy: int) -> dict[tuple[int, int], int]:
    pixels: dict[tuple[int, int], int] = {}
    if entry.data_size == 0 or entry.width == 0 or entry.height == 0:
        return pixels

    data = payload[entry.data_offset:entry.data_offset + entry.data_size]
    if len(data) < 2:
        return pixels

    base_x = entry.x + scene_dx
    base_y = entry.y + scene_dy
    offset = 0
    row_count = read_u16(data, offset)
    offset += 2

    for _row in range(row_count):
        if offset + 4 > len(data):
            break
        rel_y = read_u16(data, offset)
        span_count = read_u16(data, offset + 2)
        offset += 4
        y = base_y + rel_y
        for _span in range(span_count):
            if offset + 4 > len(data):
                break
            rel_x = read_u16(data, offset)
            pixel_count = read_u16(data, offset + 2)
            offset += 4
            x0 = base_x + rel_x
            if header.version == 1:
                packed_bytes = (pixel_count + 1) // 2
                span = data[offset:offset + packed_bytes]
                for i in range(pixel_count):
                    x = x0 + i
                    if 0 <= x < SCREEN_W and 0 <= y < SCREEN_H:
                        pixels[(x, y)] = pal4_index(span, i)
                offset += packed_bytes
            else:
                span = data[offset:offset + pixel_count]
                for i, value in enumerate(span):
                    x = x0 + i
                    if 0 <= x < SCREEN_W and 0 <= y < SCREEN_H:
                        pixels[(x, y)] = value
                offset += pixel_count
            if offset > len(data):
                break

    return pixels


def payload_size_for_pixels(pixels: dict[tuple[int, int], int], version: int) -> int:
    if not pixels:
        return 0

    rows: dict[int, list[int]] = {}
    for x, y in pixels.keys():
        rows.setdefault(y, []).append(x)

    total = 2
    for y in sorted(rows.keys()):
        xs = sorted(set(rows[y]))
        if not xs:
            continue
        total += 4
        span_start = xs[0]
        prev_x = xs[0]
        spans: list[tuple[int, int]] = []
        for x in xs[1:]:
            if x == prev_x + 1:
                prev_x = x
                continue
            spans.append((span_start, prev_x + 1))
            span_start = x
            prev_x = x
        spans.append((span_start, prev_x + 1))
        for start, end in spans:
            count = end - start
            total += 4
            total += (count + 1) // 2 if version == 1 else count
    return total


def tile_for_pixel(x: int, y: int) -> tuple[int, int] | None:
    if x < 0 or x >= SCREEN_W or y < 0 or y >= SCREEN_H:
        return None
    tile = 0 if x < TILE_W else 1
    local_y = y
    if y >= TILE_H:
        tile += 2
        local_y -= TILE_H
    return tile, local_y


def dirty_rows_for_pixels(pixels: dict[tuple[int, int], int]) -> list[list[bool]]:
    rows = [[False for _ in range(TILE_H)] for _ in range(TILE_COUNT)]
    for x, y in pixels.keys():
        tile_row = tile_for_pixel(x, y)
        if tile_row is not None:
            tile, local_y = tile_row
            rows[tile][local_y] = True
    return rows


def full_width_upload_for_pixels(pixels: dict[tuple[int, int], int]) -> dict[str, int]:
    rows = dirty_rows_for_pixels(pixels)
    bands: list[tuple[int, int, int]] = []
    capped = False

    for tile in range(TILE_COUNT):
        y = 0
        while y < TILE_H and not capped:
            while y < TILE_H and not rows[tile][y]:
                y += 1
            if y >= TILE_H:
                break
            start_y = y
            scan_y = y + 1
            last_dirty_y = y
            clean_gap = 0
            while scan_y < TILE_H:
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

    if not capped:
        uploaded_rows = sum(end_y - start_y + 1 for _tile, start_y, end_y in bands)
        return {
            "bytes": uploaded_rows * TILE_W * 2,
            "rows": uploaded_rows,
            "rects": len(bands),
            "cap_hit": 0,
        }

    uploaded_rows = 0
    rects = 0
    for tile in range(TILE_COUNT):
        dirty = [y for y, active in enumerate(rows[tile]) if active]
        if dirty:
            uploaded_rows += max(dirty) - min(dirty) + 1
            rects += 1
    return {
        "bytes": uploaded_rows * TILE_W * 2,
        "rows": uploaded_rows,
        "rects": rects,
        "cap_hit": 1,
    }


def exact_interval_stats_for_pixels(pixels: dict[tuple[int, int], int]) -> dict[str, int]:
    rows: dict[tuple[int, int], list[int]] = {}
    for x, y in pixels.keys():
        tile_row = tile_for_pixel(x, y)
        if tile_row is None:
            continue
        tile, local_y = tile_row
        local_x = x if tile in (0, 2) else x - TILE_W
        rows.setdefault((tile, local_y), []).append(local_x)

    intervals = 0
    pixel_count = 0
    for xs in rows.values():
        ordered = sorted(set(xs))
        if not ordered:
            continue
        intervals += 1
        pixel_count += 1
        prev_x = ordered[0]
        for x in ordered[1:]:
            pixel_count += 1
            if x != prev_x + 1:
                intervals += 1
            prev_x = x

    return {
        "bytes": pixel_count * 2,
        "intervals": intervals,
        "pixels": pixel_count,
    }


def temporal_zero_shift_model(frames: list[DecodedFrame], version: int) -> dict[str, int | float]:
    totals = {
        "pairs": 0,
        "baseline_compose_payload_bytes": 0,
        "residual_compose_payload_bytes": 0,
        "baseline_upload_bytes": 0,
        "residual_upload_bytes": 0,
        "cleanup_restore_bytes": 0,
        "cleanup_restore_intervals": 0,
        "cleanup_pixels": 0,
        "draw_residual_pixels": 0,
        "residual_upload_rects": 0,
        "residual_upload_cap_hits": 0,
    }

    for prev, curr in zip(frames, frames[1:]):
        if not prev.pixels or not curr.pixels:
            continue
        cleanup = {
            point: value
            for point, value in prev.pixels.items()
            if point not in curr.pixels
        }
        draw = {
            point: value
            for point, value in curr.pixels.items()
            if prev.pixels.get(point) != value
        }
        baseline_dirty = dict(prev.pixels)
        baseline_dirty.update(curr.pixels)
        residual_dirty = dict(cleanup)
        residual_dirty.update(draw)

        cleanup_stats = exact_interval_stats_for_pixels(cleanup)
        residual_upload = full_width_upload_for_pixels(residual_dirty)
        baseline_upload = full_width_upload_for_pixels(baseline_dirty)

        totals["pairs"] += 1
        totals["baseline_compose_payload_bytes"] += curr.entry.data_size
        totals["residual_compose_payload_bytes"] += payload_size_for_pixels(draw, version)
        totals["baseline_upload_bytes"] += baseline_upload["bytes"]
        totals["residual_upload_bytes"] += residual_upload["bytes"]
        totals["cleanup_restore_bytes"] += cleanup_stats["bytes"]
        totals["cleanup_restore_intervals"] += cleanup_stats["intervals"]
        totals["cleanup_pixels"] += cleanup_stats["pixels"]
        totals["draw_residual_pixels"] += len(draw)
        totals["residual_upload_rects"] += residual_upload["rects"]
        totals["residual_upload_cap_hits"] += residual_upload["cap_hit"]

    baseline_compose = int(totals["baseline_compose_payload_bytes"])
    baseline_upload = int(totals["baseline_upload_bytes"])
    residual_compose = int(totals["residual_compose_payload_bytes"])
    residual_upload = int(totals["residual_upload_bytes"])
    totals["compose_payload_saved_bytes"] = baseline_compose - residual_compose
    totals["upload_saved_bytes"] = baseline_upload - residual_upload
    totals["compose_payload_saved_percent"] = (
        round((baseline_compose - residual_compose) * 100.0 / baseline_compose, 2)
        if baseline_compose else 0.0
    )
    totals["upload_saved_percent"] = (
        round((baseline_upload - residual_upload) * 100.0 / baseline_upload, 2)
        if baseline_upload else 0.0
    )
    return totals


def bbox_for_pixels(pixels: dict[tuple[int, int], int]) -> dict[str, int] | None:
    if not pixels:
        return None
    xs = [pt[0] for pt in pixels.keys()]
    ys = [pt[1] for pt in pixels.keys()]
    min_x = min(xs)
    min_y = min(ys)
    max_x = max(xs)
    max_y = max(ys)
    return {
        "x": min_x,
        "y": min_y,
        "width": max_x - min_x + 1,
        "height": max_y - min_y + 1,
    }


def candidate_shifts(prev: Entry, curr: Entry, local_shift: int,
                     bbox_window: int) -> list[tuple[int, int]]:
    bases = {
        (0, 0),
        (curr.x - prev.x, curr.y - prev.y),
    }
    shifts: set[tuple[int, int]] = set()
    for dx in range(-local_shift, local_shift + 1):
        for dy in range(-local_shift, local_shift + 1):
            shifts.add((dx, dy))
    for base_dx, base_dy in bases:
        for dx in range(base_dx - bbox_window, base_dx + bbox_window + 1):
            for dy in range(base_dy - bbox_window, base_dy + bbox_window + 1):
                shifts.add((dx, dy))
    return sorted(shifts)


def match_count(prev_pixels: dict[tuple[int, int], int],
                curr_pixels: dict[tuple[int, int], int],
                dx: int, dy: int) -> int:
    matches = 0
    for (x, y), value in prev_pixels.items():
        if curr_pixels.get((x + dx, y + dy)) == value:
            matches += 1
    return matches


def residual_pixels(prev_pixels: dict[tuple[int, int], int],
                    curr_pixels: dict[tuple[int, int], int],
                    dx: int, dy: int) -> dict[tuple[int, int], int]:
    residual: dict[tuple[int, int], int] = {}
    for (x, y), value in curr_pixels.items():
        if prev_pixels.get((x - dx, y - dy)) != value:
            residual[(x, y)] = value
    return residual


def evaluate_pair(prev: DecodedFrame, curr: DecodedFrame, header: Header,
                  local_shift: int, bbox_window: int,
                  move_metadata_bytes: int) -> dict[str, Any] | None:
    if not prev.pixels or not curr.pixels:
        return None

    best: tuple[int, int, int] | None = None
    for dx, dy in candidate_shifts(prev.entry, curr.entry, local_shift, bbox_window):
        matches = match_count(prev.pixels, curr.pixels, dx, dy)
        if best is None or matches > best[2]:
            best = (dx, dy, matches)

    if best is None:
        return None

    dx, dy, matches = best
    residual = residual_pixels(prev.pixels, curr.pixels, dx, dy)
    residual_bytes = payload_size_for_pixels(residual, header.version)
    estimated_motion_bytes = residual_bytes + move_metadata_bytes
    current_payload_bytes = curr.entry.data_size
    saved_bytes = current_payload_bytes - estimated_motion_bytes
    current_pixels = len(curr.pixels)
    prev_pixels = len(prev.pixels)
    current_match_pct = (matches * 100.0 / current_pixels) if current_pixels else 0.0
    prev_match_pct = (matches * 100.0 / prev_pixels) if prev_pixels else 0.0

    return {
        "entry_index": curr.entry.index,
        "prev_entry_index": prev.entry.index,
        "source_frame": curr.entry.source_frame,
        "prev_source_frame": prev.entry.source_frame,
        "dx": dx,
        "dy": dy,
        "current_payload_bytes": current_payload_bytes,
        "residual_payload_bytes": residual_bytes,
        "move_metadata_bytes": move_metadata_bytes,
        "estimated_motion_payload_bytes": estimated_motion_bytes,
        "estimated_saved_payload_bytes": saved_bytes,
        "estimated_saved_payload_percent": (
            round(saved_bytes * 100.0 / current_payload_bytes, 2)
            if current_payload_bytes > 0 else 0.0
        ),
        "matched_pixels": matches,
        "current_pixels": current_pixels,
        "prev_pixels": prev_pixels,
        "residual_pixels": len(residual),
        "cleanup_pixels": max(0, prev_pixels - matches),
        "current_match_percent": round(current_match_pct, 2),
        "prev_match_percent": round(prev_match_pct, 2),
        "bbox": bbox_for_pixels(curr.pixels),
    }


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Analyze translation-only FG2 motion-compensation opportunities."
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
        "--local-shift",
        type=int,
        default=3,
        help="Also test shifts in +/-N pixels around 0,0.",
    )
    parser.add_argument(
        "--bbox-window",
        type=int,
        default=4,
        help="Test shifts in +/-N pixels around the entry bbox delta.",
    )
    parser.add_argument(
        "--min-match-percent",
        type=float,
        default=60.0,
        help="Minimum current-frame pixel match percent for a candidate.",
    )
    parser.add_argument(
        "--min-saved-bytes",
        type=int,
        default=64,
        help="Minimum estimated payload-byte saving for a candidate.",
    )
    parser.add_argument(
        "--move-metadata-bytes",
        type=int,
        default=16,
        help="Estimated per-move metadata/header bytes for a future pack format.",
    )
    parser.add_argument("--top", type=int, default=25)
    parser.add_argument("--pretty", action="store_true", help="Pretty-print JSON output.")
    args = parser.parse_args()

    payload, header, entries = parse_pack(args.pack)
    apply_scene_relative = (
        (header.flags & FLAG_SCENE_RELATIVE) != 0 and
        not args.no_scene_relative_offset
    )
    scene_dx = args.island_x if apply_scene_relative else 0
    scene_dy = args.island_y if apply_scene_relative else 0

    decoded = [
        DecodedFrame(
            entry=entry,
            pixels=decode_entry_pixels(payload, header, entry, scene_dx, scene_dy),
        )
        for entry in entries
    ]

    pair_results: list[dict[str, Any]] = []
    candidate_results: list[dict[str, Any]] = []
    for prev, curr in zip(decoded, decoded[1:]):
        result = evaluate_pair(
            prev,
            curr,
            header,
            local_shift=args.local_shift,
            bbox_window=args.bbox_window,
            move_metadata_bytes=args.move_metadata_bytes,
        )
        if result is None:
            continue
        pair_results.append(result)
        if (
            result["current_match_percent"] >= args.min_match_percent and
            result["estimated_saved_payload_bytes"] >= args.min_saved_bytes
        ):
            candidate_results.append(result)

    total_current_payload = sum(item["current_payload_bytes"] for item in pair_results)
    total_motion_payload = 0
    total_saved_payload = 0
    candidate_indices = {item["entry_index"] for item in candidate_results}
    for item in pair_results:
        if item["entry_index"] in candidate_indices:
            total_motion_payload += item["estimated_motion_payload_bytes"]
            total_saved_payload += item["estimated_saved_payload_bytes"]
        else:
            total_motion_payload += item["current_payload_bytes"]

    top_candidates = sorted(
        candidate_results,
        key=lambda item: (
            item["estimated_saved_payload_bytes"],
            item["current_match_percent"],
        ),
        reverse=True,
    )[:args.top]
    top_nonzero_candidates = [
        item for item in sorted(
            candidate_results,
            key=lambda row: (
                row["estimated_saved_payload_bytes"],
                row["current_match_percent"],
            ),
            reverse=True,
        )
        if item["dx"] != 0 or item["dy"] != 0
    ][:args.top]

    zero_shift_candidates = [
        item for item in candidate_results
        if item["dx"] == 0 and item["dy"] == 0
    ]
    nonzero_shift_candidates = [
        item for item in candidate_results
        if item["dx"] != 0 or item["dy"] != 0
    ]
    shift_buckets: dict[str, dict[str, int]] = {}
    for item in candidate_results:
        key = f"{item['dx']},{item['dy']}"
        bucket = shift_buckets.setdefault(
            key,
            {
                "dx": item["dx"],
                "dy": item["dy"],
                "pairs": 0,
                "saved_payload_bytes": 0,
            },
        )
        bucket["pairs"] += 1
        bucket["saved_payload_bytes"] += item["estimated_saved_payload_bytes"]
    top_shift_buckets = sorted(
        shift_buckets.values(),
        key=lambda item: (item["saved_payload_bytes"], item["pairs"]),
        reverse=True,
    )[:args.top]

    result: dict[str, Any] = {
        "pack": str(args.pack),
        "pack_bytes": len(payload),
        "header": {
            "version": header.version,
            "encoding": "pal4" if header.version == 1 else "indexed8",
            "frame_count": header.frame_count,
            "flags": header.flags,
            "scene_relative": bool(header.flags & FLAG_SCENE_RELATIVE),
            "sound_event_count": header.sound_event_count,
        },
        "analysis": {
            "scene_dx": scene_dx,
            "scene_dy": scene_dy,
            "local_shift": args.local_shift,
            "bbox_window": args.bbox_window,
            "min_match_percent": args.min_match_percent,
            "min_saved_bytes": args.min_saved_bytes,
            "move_metadata_bytes": args.move_metadata_bytes,
            "decoded_frames": len(decoded),
            "evaluated_pairs": len(pair_results),
        },
        "summary": {
            "candidate_pairs": len(candidate_results),
            "candidate_pair_percent": (
                round(len(candidate_results) * 100.0 / len(pair_results), 2)
                if pair_results else 0.0
            ),
            "zero_shift_candidate_pairs": len(zero_shift_candidates),
            "nonzero_shift_candidate_pairs": len(nonzero_shift_candidates),
            "total_pair_current_payload_bytes": total_current_payload,
            "estimated_payload_bytes_with_candidates": total_motion_payload,
            "estimated_saved_payload_bytes": total_saved_payload,
            "estimated_saved_payload_percent": (
                round(total_saved_payload * 100.0 / total_current_payload, 2)
                if total_current_payload else 0.0
            ),
            "zero_shift_saved_payload_bytes": sum(
                item["estimated_saved_payload_bytes"] for item in zero_shift_candidates
            ),
            "nonzero_shift_saved_payload_bytes": sum(
                item["estimated_saved_payload_bytes"] for item in nonzero_shift_candidates
            ),
            "median_candidate_match_percent": (
                sorted(item["current_match_percent"] for item in candidate_results)[
                    len(candidate_results) // 2
                ]
                if candidate_results else 0.0
            ),
            "max_candidate_saved_bytes": (
                top_candidates[0]["estimated_saved_payload_bytes"]
                if top_candidates else 0
            ),
        },
        "temporal_zero_shift_runtime_model": temporal_zero_shift_model(
            decoded,
            header.version,
        ),
        "top_candidates": top_candidates,
        "top_nonzero_candidates": top_nonzero_candidates,
        "top_shift_buckets": top_shift_buckets,
        "red_team": [
            "This is a payload/candidate detector, not proof that GPU MoveImage is runtime-safe.",
            "A runtime implementation must keep the RAM background mirror exact, not just the displayed VRAM image.",
            "Old-position cleanup, new-position residual upload, dirty tracking, and sound/timing order still need a format design.",
            "The zero-shift runtime model assumes pack-emitted full-current dirty metadata so unchanged foreground pixels remain restorable on the next frame.",
            "Pairwise translation savings can be erased by CD reads, MoveImage synchronization, or extra metadata.",
        ],
    }

    print(json.dumps(result, indent=2 if args.pretty else None, sort_keys=args.pretty))


if __name__ == "__main__":
    main()
