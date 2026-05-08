#!/usr/bin/env python3
"""Zero FGP3/v4 entries whose payload has no cleanup or draw pixels.

Removing VISITOR3 entries outright can shorten scene cadence and steal hidden
refill slack. This pack-side probe preserves the entry/deadline table while
turning any truly visual-empty payload into an empty hold entry, if such entries
exist in the current pack data.
"""

from __future__ import annotations

import argparse
import shutil
import struct
import tempfile
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


def compact_rows_pixel_count(payload: bytes, offset: int, limit: int, *, has_pixels: bool) -> tuple[int, int]:
    if offset + 2 > limit:
        raise ValueError("truncated row count")
    row_count = read_u16(payload, offset)
    offset += 2
    pixel_total = 0
    for _row in range(row_count):
        _rel_y, offset = read_compact_u16(payload, offset, limit)
        span_count, offset = read_compact_u16(payload, offset, limit)
        for _span in range(span_count):
            _rel_x, offset = read_compact_u16(payload, offset, limit)
            pixel_count, offset = read_compact_u16(payload, offset, limit)
            pixel_total += pixel_count
            if has_pixels:
                offset += (pixel_count + 1) // 2
            if offset > limit:
                raise ValueError("compact row payload overrun")
    return pixel_total, offset


def payload_has_no_visual_work(payload: bytes) -> bool:
    limit = len(payload)
    cleanup_pixels, draw_offset = compact_rows_pixel_count(payload, 0, limit, has_pixels=False)
    if draw_offset == limit:
        return cleanup_pixels == 0
    draw_pixels, end_offset = compact_rows_pixel_count(payload, draw_offset, limit, has_pixels=True)
    if end_offset != limit:
        raise ValueError("payload has trailing bytes")
    return cleanup_pixels == 0 and draw_pixels == 0


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
    zeroed: list[tuple[int, int, int]] = []
    old_payload_bytes = 0

    for index, entry in enumerate(old_entries):
        source_frame, x, y, width, height, hold_vblanks, old_offset, old_size = entry
        if old_size == 0 or width == 0 or height == 0:
            new_entries.append((source_frame, x, y, width, height, hold_vblanks, 0, 0))
            continue
        if old_offset + old_size > len(data):
            raise SystemExit(f"entry {index} payload extends beyond pack: {input_path}")
        payload = data[old_offset:old_offset + old_size]
        old_payload_bytes += old_size
        try:
            no_visual_work = payload_has_no_visual_work(payload)
        except ValueError as exc:
            raise SystemExit(f"entry {index} payload parse failed: {exc}") from exc
        if no_visual_work:
            new_entries.append((source_frame, x, y, 0, 0, hold_vblanks, 0, 0))
            zeroed.append((index, source_frame, old_size))
            continue
        new_entries.append((source_frame, x, y, width, height, hold_vblanks, cursor, old_size))
        payload_out += payload
        cursor += old_size

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
    zeroed_sources = ",".join(str(source) for _index, source, _size in zeroed)
    print(
        f"{input_path} -> {output_path}: {len(data)} -> {len(out)} bytes, "
        f"active_payload {old_payload_bytes} -> {len(payload_out)}, "
        f"zeroed_entries={len(zeroed)}, zeroed_bytes={sum(size for _index, _source, size in zeroed)}, "
        f"zeroed_sources={zeroed_sources}"
    )


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Build an FGP3/v4 pack with visual no-op payload entries turned into empty holds."
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
