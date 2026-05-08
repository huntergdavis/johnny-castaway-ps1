#!/usr/bin/env python3
"""Remove redundant FGP3/v4 cleanup spans covered by the current draw.

FGP3/v4 PAL4 residual packs store each frame as a compact cleanup prefix
followed by a compact draw payload. Cleanup spans restore the previous frame's
dirty pixels from background, but any pixel redrawn by the same current frame
does not need that restore. This tool subtracts same-row draw intervals from
the cleanup prefix, keeps draw pixels byte-identical, and can pad the result
back to the input size so CD LBAs stay fixed during perf probes.
"""

from __future__ import annotations

import argparse
import shutil
import struct
import tempfile
from dataclasses import dataclass
from pathlib import Path


HEADER = "<4sHHHHHHHHHHIIIHH"
ENTRY = "<HhhHHHII"
HEADER_SIZE = struct.calcsize(HEADER)
ENTRY_SIZE = struct.calcsize(ENTRY)


@dataclass
class Row:
    rel_y: int
    spans: list[tuple[int, int]]


@dataclass
class PayloadStats:
    cleanup_spans: int = 0
    cleanup_pixels: int = 0
    restore_bytes: int = 0


def read_u16(data: bytes, offset: int) -> int:
    return struct.unpack_from("<H", data, offset)[0]


def read_compact_u16(data: bytes, offset: int, limit: int) -> tuple[int, int]:
    if offset >= limit:
        raise ValueError("truncated compact u16")
    value = data[offset]
    offset += 1
    if value != 0xFF:
        return value, offset
    if offset + 2 > limit:
        raise ValueError("truncated compact u16 extension")
    return read_u16(data, offset), offset + 2


def write_compact_u16(out: bytearray, value: int) -> None:
    if value < 0 or value > 0xFFFF:
        raise ValueError(f"compact u16 out of range: {value}")
    if value < 0xFF:
        out.append(value)
    else:
        out.append(0xFF)
        out += struct.pack("<H", value)


def summarize_rows(rows: list[Row]) -> PayloadStats:
    stats = PayloadStats()
    for row in rows:
        stats.cleanup_spans += len(row.spans)
        for _x, count in row.spans:
            stats.cleanup_pixels += count
    stats.restore_bytes = stats.cleanup_pixels * 2
    return stats


def add_stats(total: PayloadStats, part: PayloadStats) -> None:
    total.cleanup_spans += part.cleanup_spans
    total.cleanup_pixels += part.cleanup_pixels
    total.restore_bytes += part.restore_bytes


def parse_cleanup_rows(payload: bytes, offset: int, limit: int) -> tuple[list[Row], int]:
    if offset + 2 > limit:
        raise ValueError("truncated cleanup row count")
    row_count = read_u16(payload, offset)
    offset += 2
    rows: list[Row] = []
    for _row in range(row_count):
        rel_y, offset = read_compact_u16(payload, offset, limit)
        span_count, offset = read_compact_u16(payload, offset, limit)
        spans: list[tuple[int, int]] = []
        for _span in range(span_count):
            rel_x, offset = read_compact_u16(payload, offset, limit)
            pixel_count, offset = read_compact_u16(payload, offset, limit)
            spans.append((rel_x, pixel_count))
        rows.append(Row(rel_y, spans))
    return rows, offset


def parse_draw_intervals(
    payload: bytes, offset: int, limit: int
) -> tuple[dict[int, list[tuple[int, int]]], int]:
    if offset == limit:
        return {}, offset
    if offset + 2 > limit:
        raise ValueError("truncated draw row count")
    row_count = read_u16(payload, offset)
    offset += 2
    by_y: dict[int, list[tuple[int, int]]] = {}
    for _row in range(row_count):
        rel_y, offset = read_compact_u16(payload, offset, limit)
        span_count, offset = read_compact_u16(payload, offset, limit)
        spans = by_y.setdefault(rel_y, [])
        for _span in range(span_count):
            rel_x, offset = read_compact_u16(payload, offset, limit)
            pixel_count, offset = read_compact_u16(payload, offset, limit)
            packed_bytes = (pixel_count + 1) // 2
            if offset + packed_bytes > limit:
                raise ValueError("truncated draw pixels")
            spans.append((rel_x, rel_x + pixel_count))
            offset += packed_bytes
    if offset != limit:
        raise ValueError("draw payload has trailing bytes")
    return by_y, offset


def merge_intervals(intervals: list[tuple[int, int]]) -> list[tuple[int, int]]:
    if not intervals:
        return []
    merged: list[tuple[int, int]] = []
    for start, end in sorted(intervals):
        if start >= end:
            continue
        if not merged or start > merged[-1][1]:
            merged.append((start, end))
        else:
            old_start, old_end = merged[-1]
            merged[-1] = (old_start, max(old_end, end))
    return merged


def subtract_intervals(
    cleanup_spans: list[tuple[int, int]],
    draw_intervals: list[tuple[int, int]],
) -> list[tuple[int, int]]:
    draw = merge_intervals(draw_intervals)
    if not draw:
        return list(cleanup_spans)

    out: list[tuple[int, int]] = []
    for rel_x, pixel_count in cleanup_spans:
        start = rel_x
        end = rel_x + pixel_count
        cursor = start
        for draw_start, draw_end in draw:
            if draw_end <= cursor:
                continue
            if draw_start >= end:
                break
            if draw_start > cursor:
                out.append((cursor, draw_start - cursor))
            cursor = max(cursor, draw_end)
            if cursor >= end:
                break
        if cursor < end:
            out.append((cursor, end - cursor))
    return out


def encode_cleanup_rows(rows: list[Row]) -> bytes:
    active_rows = [row for row in rows if row.spans]
    out = bytearray()
    out += struct.pack("<H", len(active_rows))
    for row in active_rows:
        write_compact_u16(out, row.rel_y)
        write_compact_u16(out, len(row.spans))
        for rel_x, pixel_count in row.spans:
            write_compact_u16(out, rel_x)
            write_compact_u16(out, pixel_count)
    return bytes(out)


def compact_payload(payload: bytes) -> tuple[bytes, PayloadStats, PayloadStats]:
    cleanup_rows, draw_offset = parse_cleanup_rows(payload, 0, len(payload))
    draw_by_y, _end = parse_draw_intervals(payload, draw_offset, len(payload))

    old_stats = summarize_rows(cleanup_rows)
    new_rows: list[Row] = []
    for row in cleanup_rows:
        new_rows.append(
            Row(row.rel_y, subtract_intervals(row.spans, draw_by_y.get(row.rel_y, [])))
        )
    new_stats = summarize_rows(new_rows)
    return encode_cleanup_rows(new_rows) + payload[draw_offset:], old_stats, new_stats


def convert(input_path: Path, output_path: Path, pad_to_input_size: bool) -> None:
    data = input_path.read_bytes()
    if len(data) < HEADER_SIZE:
        raise SystemExit(f"pack too small: {input_path}")

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
    ) = struct.unpack_from(HEADER, data, 0)

    if magic != b"FGP3" or version != 4:
        raise SystemExit(f"{input_path} is not an FGP3/v4 PAL4 compact residual pack")
    if table_offset + frame_count * ENTRY_SIZE > len(data):
        raise SystemExit(f"entry table extends beyond pack: {input_path}")

    palette = data[HEADER_SIZE:table_offset]
    if len(palette) != 32:
        raise SystemExit(f"expected 32-byte PAL4 palette: {input_path}")

    old_entries = [
        struct.unpack_from(ENTRY, data, table_offset + index * ENTRY_SIZE)
        for index in range(frame_count)
    ]

    sound_events = b""
    if sound_events_offset and sound_event_count:
        sound_end = sound_events_offset + sound_event_count * 4
        if sound_end > len(data):
            raise SystemExit(f"sound table extends beyond pack: {input_path}")
        sound_events = data[sound_events_offset:sound_end]

    payload_out = bytearray()
    new_entries: list[tuple[int, int, int, int, int, int, int, int]] = []
    cursor = data_offset
    old_payload_bytes = 0
    old_stats = PayloadStats()
    new_stats = PayloadStats()
    for index, entry in enumerate(old_entries):
        source_frame, x, y, width, height, hold_vblanks, old_offset, old_size = entry
        if old_size == 0:
            new_entries.append((source_frame, x, y, width, height, hold_vblanks, 0, 0))
            continue
        if old_offset + old_size > len(data):
            raise SystemExit(f"entry {index} payload extends beyond pack: {input_path}")
        compact, entry_old_stats, entry_new_stats = compact_payload(
            data[old_offset:old_offset + old_size]
        )
        add_stats(old_stats, entry_old_stats)
        add_stats(new_stats, entry_new_stats)
        new_entries.append(
            (source_frame, x, y, width, height, hold_vblanks, cursor, len(compact))
        )
        old_payload_bytes += old_size
        payload_out += compact
        cursor += len(compact)

    new_sound_events_offset = cursor if sound_events else 0
    header = struct.pack(
        HEADER,
        b"FGP3",
        4,
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
        new_sound_events_offset,
        sound_event_count,
        reserved1,
    )

    out = bytearray(header)
    out += palette
    if len(out) > table_offset:
        raise SystemExit("metadata layout overflow before entry table")
    out += b"\0" * (table_offset - len(out))
    for entry in new_entries:
        out += struct.pack(ENTRY, *entry)
    if len(out) > data_offset:
        raise SystemExit("entry table overflow before payload data")
    out += b"\0" * (data_offset - len(out))
    out += payload_out
    out += sound_events

    if pad_to_input_size:
        if len(out) > len(data):
            raise SystemExit(f"compact pack grew unexpectedly: {len(out)} > {len(data)}")
        out += b"\0" * (len(data) - len(out))

    output_path.write_bytes(out)
    print(
        f"{input_path} -> {output_path}: {len(data)} -> {len(out)} bytes, "
        f"active_payload {old_payload_bytes} -> {len(payload_out)}, "
        f"cleanup_spans {old_stats.cleanup_spans} -> {new_stats.cleanup_spans}, "
        f"cleanup_pixels {old_stats.cleanup_pixels} -> {new_stats.cleanup_pixels}, "
        f"restore_bytes {old_stats.restore_bytes} -> {new_stats.restore_bytes}"
    )


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Build an FGP3/v4 pack with cleanup spans under current draw removed."
    )
    parser.add_argument("input_fgp3", type=Path)
    parser.add_argument("output_fgp3", type=Path, nargs="?")
    parser.add_argument("--in-place", action="store_true")
    parser.add_argument("--pad-to-input-size", action="store_true")
    args = parser.parse_args()

    if args.in_place == bool(args.output_fgp3):
        raise SystemExit("pass either --in-place or an output path")
    if args.in_place:
        with tempfile.NamedTemporaryFile(
            prefix=f"{args.input_fgp3.name}.", dir=args.input_fgp3.parent, delete=False
        ) as tmp:
            temp_path = Path(tmp.name)
        try:
            convert(args.input_fgp3, temp_path, args.pad_to_input_size)
            shutil.move(temp_path, args.input_fgp3)
        finally:
            if temp_path.exists():
                temp_path.unlink()
    else:
        convert(args.input_fgp3, args.output_fgp3, args.pad_to_input_size)


if __name__ == "__main__":
    main()
