#!/usr/bin/env python3
"""Trim trailing transparent pixels from FGP3/v4 PAL4 draw spans.

This is a pack-side transform for already validated compact residual packs. It
preserves the entry/deadline table semantics, rewrites only draw span tails that
end in palette index 0, and can pad back to the original file size so CD LBAs
stay fixed for perf probes.
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
class DrawSpan:
    rel_x: int
    pixel_count: int
    pixels: bytes


@dataclass
class DrawRow:
    rel_y: int
    spans: list[DrawSpan]


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


def pal4_index(data: bytes, pixel_index: int) -> int:
    packed = data[pixel_index >> 1]
    return (packed & 0x0F) if (pixel_index & 1) else ((packed >> 4) & 0x0F)


def skip_compact_rows(payload: bytes, offset: int, limit: int, *, has_pixels: bool) -> int:
    if offset + 2 > limit:
        raise ValueError("truncated row count")
    row_count = read_u16(payload, offset)
    offset += 2
    for _row in range(row_count):
        _rel_y, offset = read_compact_u16(payload, offset, limit)
        span_count, offset = read_compact_u16(payload, offset, limit)
        for _span in range(span_count):
            _rel_x, offset = read_compact_u16(payload, offset, limit)
            pixel_count, offset = read_compact_u16(payload, offset, limit)
            if has_pixels:
                offset += (pixel_count + 1) // 2
            if offset > limit:
                raise ValueError("compact row payload overrun")
    return offset


def parse_draw_rows(payload: bytes, offset: int, limit: int) -> tuple[list[DrawRow], int]:
    if offset + 2 > limit:
        raise ValueError("truncated draw row count")
    row_count = read_u16(payload, offset)
    offset += 2
    rows: list[DrawRow] = []
    for _row in range(row_count):
        rel_y, offset = read_compact_u16(payload, offset, limit)
        span_count, offset = read_compact_u16(payload, offset, limit)
        spans: list[DrawSpan] = []
        for _span in range(span_count):
            rel_x, offset = read_compact_u16(payload, offset, limit)
            pixel_count, offset = read_compact_u16(payload, offset, limit)
            packed_bytes = (pixel_count + 1) // 2
            if offset + packed_bytes > limit:
                raise ValueError("truncated draw pixels")
            spans.append(DrawSpan(rel_x, pixel_count, payload[offset:offset + packed_bytes]))
            offset += packed_bytes
        rows.append(DrawRow(rel_y, spans))
    return rows, offset


def encode_draw_rows(rows: list[DrawRow]) -> bytes:
    active_rows = [row for row in rows if row.spans]
    if not active_rows:
        return b""
    out = bytearray()
    out += struct.pack("<H", len(active_rows))
    for row in active_rows:
        write_compact_u16(out, row.rel_y)
        write_compact_u16(out, len(row.spans))
        for span in row.spans:
            write_compact_u16(out, span.rel_x)
            write_compact_u16(out, span.pixel_count)
            out += span.pixels
    return bytes(out)


def trim_payload(payload: bytes) -> tuple[bytes, int]:
    limit = len(payload)
    draw_offset = skip_compact_rows(payload, 0, limit, has_pixels=False)
    cleanup = payload[:draw_offset]
    rows, draw_end = parse_draw_rows(payload, draw_offset, limit)
    trailing = payload[draw_end:]
    if any(trailing):
        raise ValueError("draw payload has nonzero trailing bytes")
    trimmed_pixels = 0

    for row in rows:
        kept_spans: list[DrawSpan] = []
        for span in row.spans:
            new_count = span.pixel_count
            while new_count > 0 and pal4_index(span.pixels, new_count - 1) == 0:
                new_count -= 1
            if new_count == span.pixel_count:
                kept_spans.append(span)
                continue
            old_bytes = len(span.pixels)
            new_bytes = (new_count + 1) // 2
            trimmed_pixels += span.pixel_count - new_count
            if new_count:
                kept_spans.append(DrawSpan(span.rel_x, new_count, span.pixels[:new_bytes]))
        row.spans = kept_spans

    return cleanup + encode_draw_rows(rows), trimmed_pixels


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
    trimmed_entries = 0
    trimmed_pixels = 0
    trimmed_bytes = 0

    for index, entry in enumerate(old_entries):
        source_frame, x, y, width, height, hold_vblanks, old_offset, old_size = entry
        if old_size == 0 or width == 0 or height == 0:
            new_entries.append((source_frame, x, y, width, height, hold_vblanks, 0, 0))
            continue
        if old_offset + old_size > len(data):
            raise SystemExit(f"entry {index} payload extends beyond pack: {input_path}")
        old_payload = data[old_offset:old_offset + old_size]
        old_payload_bytes += old_size
        try:
            new_payload, entry_trimmed_pixels = trim_payload(old_payload)
        except ValueError as exc:
            raise SystemExit(f"entry {index} payload parse failed: {exc}") from exc
        entry_trimmed_bytes = old_size - len(new_payload)
        if entry_trimmed_bytes:
            trimmed_entries += 1
            trimmed_pixels += entry_trimmed_pixels
            trimmed_bytes += entry_trimmed_bytes
        new_entries.append((source_frame, x, y, width, height, hold_vblanks, cursor, len(new_payload)))
        payload_out += new_payload
        cursor += len(new_payload)

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
            raise SystemExit(f"trimmed pack grew unexpectedly: {len(out)} > {len(data)}")
        out += b"\0" * (len(data) - len(out))

    output_path.write_bytes(out)
    print(
        f"{input_path} -> {output_path}: {len(data)} -> {len(out)} bytes, "
        f"active_payload {old_payload_bytes} -> {len(payload_out)}, "
        f"trimmed_entries={trimmed_entries}, trimmed_pixels={trimmed_pixels}, "
        f"trimmed_bytes={trimmed_bytes}"
    )


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Trim trailing transparent PAL4 draw pixels from an FGP3/v4 pack."
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
        assert args.output_fgp3 is not None
        convert(args.input_fgp3, args.output_fgp3, args.pad_to_input_size)


if __name__ == "__main__":
    main()
