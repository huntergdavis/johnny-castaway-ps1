#!/usr/bin/env python3
"""Compact PAL4 draw metadata in FGP3/v3 residual packs.

FGP3/v3 already stores cleanup row/span coordinates as compact u16 values.
This probe keeps that cleanup encoding and also compact-encodes the following
PAL4 draw row/span coordinates, producing FGP3/v4. Packed pixel bytes are
unchanged, sound events are preserved, and optional padding keeps CD layout
fixed for canary probes.
"""

from __future__ import annotations

import argparse
import struct
from pathlib import Path


HEADER = "<4sHHHHHHHHHHIIIHH"
ENTRY = "<HhhHHHII"
HEADER_SIZE = struct.calcsize(HEADER)
ENTRY_SIZE = struct.calcsize(ENTRY)


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


def cleanup_prefix_end(payload: bytes) -> int:
    limit = len(payload)
    if limit < 2:
        return limit

    offset = 0
    cleanup_rows = read_u16(payload, offset)
    offset += 2
    for _ in range(cleanup_rows):
        _rel_y, offset = read_compact_u16(payload, offset, limit)
        span_count, offset = read_compact_u16(payload, offset, limit)
        for _span in range(span_count):
            _rel_x, offset = read_compact_u16(payload, offset, limit)
            _pixel_count, offset = read_compact_u16(payload, offset, limit)
    return offset


def compact_draw_payload(draw_payload: bytes) -> bytes:
    limit = len(draw_payload)
    if limit < 2:
        return draw_payload

    offset = 0
    row_count = read_u16(draw_payload, offset)
    offset += 2

    out = bytearray()
    out += struct.pack("<H", row_count)
    for _ in range(row_count):
        if offset + 4 > limit:
            raise ValueError("truncated draw row")
        rel_y = read_u16(draw_payload, offset)
        span_count = read_u16(draw_payload, offset + 2)
        offset += 4
        write_compact_u16(out, rel_y)
        write_compact_u16(out, span_count)
        for _span in range(span_count):
            if offset + 4 > limit:
                raise ValueError("truncated draw span")
            rel_x = read_u16(draw_payload, offset)
            pixel_count = read_u16(draw_payload, offset + 2)
            offset += 4
            packed_bytes = (pixel_count + 1) // 2
            if offset + packed_bytes > limit:
                raise ValueError("truncated draw pixels")
            write_compact_u16(out, rel_x)
            write_compact_u16(out, pixel_count)
            out += draw_payload[offset:offset + packed_bytes]
            offset += packed_bytes

    if offset != limit:
        raise ValueError("draw payload has trailing bytes")
    return bytes(out)


def compact_payload(payload: bytes) -> bytes:
    cleanup_end = cleanup_prefix_end(payload)
    if cleanup_end >= len(payload):
        return payload
    return payload[:cleanup_end] + compact_draw_payload(payload[cleanup_end:])


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

    if magic != b"FGP3" or version != 3:
        raise SystemExit(f"{input_path} is not an FGP3/v3 PAL4 compact residual pack")
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
    for index, entry in enumerate(old_entries):
        source_frame, x, y, width, height, hold_vblanks, old_offset, old_size = entry
        if old_size == 0:
            new_entries.append((source_frame, x, y, width, height, hold_vblanks, 0, 0))
            continue
        if old_offset + old_size > len(data):
            raise SystemExit(f"entry {index} payload extends beyond pack: {input_path}")
        compact = compact_payload(data[old_offset:old_offset + old_size])
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
        f"active_payload {old_payload_bytes} -> {len(payload_out)}"
    )


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Build an FGP3/v4 pack with compact PAL4 draw metadata."
    )
    parser.add_argument("--input-fgp3", type=Path, required=True)
    parser.add_argument("--output-fgp3", type=Path, required=True)
    parser.add_argument("--pad-to-input-size", action="store_true")
    args = parser.parse_args()
    convert(args.input_fgp3, args.output_fgp3, args.pad_to_input_size)


if __name__ == "__main__":
    main()
