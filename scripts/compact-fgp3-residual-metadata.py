#!/usr/bin/env python3
"""Compact FGP3 PAL4 temporal-residual cleanup metadata.

FGP3/v1 stores every row/span coordinate as little-endian u16. VISITOR3-like
packs have many small cleanup span coordinates, so FGP3/v3 keeps the same draw
payloads and encodes only cleanup row/span fields as compact u16:

  value < 255: one byte
  otherwise: 0xff followed by little-endian u16

Frame table offsets/sizes are rebuilt, sound events are preserved, and the
output can be padded back to the input file size to preserve CD layout.
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


def write_compact_u16(out: bytearray, value: int) -> None:
    if value < 0 or value > 0xFFFF:
        raise ValueError(f"compact u16 out of range: {value}")
    if value < 0xFF:
        out.append(value)
    else:
        out.append(0xFF)
        out += struct.pack("<H", value)


def compact_payload(payload: bytes, version: int) -> bytes:
    if len(payload) < 2:
        return payload

    out = bytearray()
    offset = 0
    cleanup_rows = read_u16(payload, offset)
    offset += 2
    out += struct.pack("<H", cleanup_rows)

    for _ in range(cleanup_rows):
        if offset + 4 > len(payload):
            raise ValueError("truncated cleanup row")
        rel_y = read_u16(payload, offset)
        span_count = read_u16(payload, offset + 2)
        offset += 4
        write_compact_u16(out, rel_y)
        write_compact_u16(out, span_count)
        for _span in range(span_count):
            if offset + 4 > len(payload):
                raise ValueError("truncated cleanup span")
            rel_x = read_u16(payload, offset)
            pixel_count = read_u16(payload, offset + 2)
            offset += 4
            write_compact_u16(out, rel_x)
            write_compact_u16(out, pixel_count)

    out += payload[offset:]
    return bytes(out)


def convert(input_path: Path, output_path: Path, pad_to_input_size: bool) -> None:
    data = input_path.read_bytes()
    if len(data) < HEADER_SIZE:
        raise SystemExit(f"pack too small: {input_path}")

    unpacked = struct.unpack_from(HEADER, data, 0)
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
    ) = unpacked

    if magic != b"FGP3" or version != 1:
        raise SystemExit(f"{input_path} is not an FGP3/v1 PAL4 residual pack")
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
    for index, entry in enumerate(old_entries):
        source_frame, x, y, width, height, hold_vblanks, old_offset, old_size = entry
        if old_size == 0:
            new_entries.append((source_frame, x, y, width, height, hold_vblanks, 0, 0))
            continue
        if old_offset + old_size > len(data):
            raise SystemExit(f"entry {index} payload extends beyond pack: {input_path}")
        compact = compact_payload(data[old_offset:old_offset + old_size], version)
        new_entries.append(
            (source_frame, x, y, width, height, hold_vblanks, cursor, len(compact))
        )
        payload_out += compact
        cursor += len(compact)

    new_sound_events_offset = cursor if sound_events else 0
    header = struct.pack(
        HEADER,
        b"FGP3",
        3,
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
            raise SystemExit(
                f"compact pack grew unexpectedly: {len(out)} > {len(data)}"
            )
        out += b"\0" * (len(data) - len(out))

    output_path.write_bytes(out)
    print(
        f"{input_path} -> {output_path}: {len(data)} -> {len(out)} bytes, "
        f"active_payload {sum(e[7] for e in old_entries)} -> {len(payload_out)}"
    )


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Build an FGP3/v3 cleanup-compact residual pack from FGP3/v1."
    )
    parser.add_argument("--input-fgp3", type=Path, required=True)
    parser.add_argument("--output-fgp3", type=Path, required=True)
    parser.add_argument("--pad-to-input-size", action="store_true")
    args = parser.parse_args()
    convert(args.input_fgp3, args.output_fgp3, args.pad_to_input_size)


if __name__ == "__main__":
    main()
