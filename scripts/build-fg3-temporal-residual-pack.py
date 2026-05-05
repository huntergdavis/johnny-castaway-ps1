#!/usr/bin/env python3
"""Build an experimental FGP3 temporal-residual pack from an existing FG2.

FGP3 keeps the FG2 header/table/palette shape, but each payload is:

  u16 cleanup_row_count
    u16 rel_y, u16 span_count
      u16 rel_x, u16 pixel_count
  FG2 span payload for pixels that differ from the previous frame

FGP3 version 1 carries PAL4 draw spans.
FGP3 version 2 carries indexed8 draw spans.

The cleanup spans restore old foreground pixels that disappeared or changed.
The draw payload then writes only new/changed pixels. This prototype is
intended for validation before the normal pack builder owns FGP3.
"""

from __future__ import annotations

import argparse
import struct
from dataclasses import dataclass
from pathlib import Path


HEADER = "<4sHHHHHHHHHHIIIHH"
ENTRY = "<HhhHHHII"
HEADER_SIZE = struct.calcsize(HEADER)
ENTRY_SIZE = struct.calcsize(ENTRY)


@dataclass
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


@dataclass
class Entry:
    source_frame: int
    x: int
    y: int
    width: int
    height: int
    hold_vblanks: int
    data_offset: int
    data_size: int


def u16(data: bytes, offset: int) -> int:
    return struct.unpack_from("<H", data, offset)[0]


def pal4_index(data: bytes, pixel: int) -> int:
    packed = data[pixel >> 1]
    return (packed & 0x0F) if (pixel & 1) else ((packed >> 4) & 0x0F)


def parse_fg2(path: Path) -> tuple[bytes, Header, list[int], list[Entry], bytes]:
    data = path.read_bytes()
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
    if magic != b"FGP2" or version not in (1, 2):
        raise SystemExit(f"{path} is not a supported FGP2 pack")
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
    palette_count = (table_offset - HEADER_SIZE) // 2
    palette = [
        u16(data, HEADER_SIZE + i * 2)
        for i in range(palette_count)
    ]
    entries: list[Entry] = []
    for i in range(frame_count):
        values = struct.unpack_from(ENTRY, data, table_offset + i * ENTRY_SIZE)
        entries.append(Entry(*values))
    sound_events = b""
    if sound_events_offset:
        sound_events = data[sound_events_offset:sound_events_offset + sound_event_count * 4]
    return data, header, palette, entries, sound_events


def decode_pixels(pack: bytes, entry: Entry, version: int) -> dict[tuple[int, int], int]:
    pixels: dict[tuple[int, int], int] = {}
    if entry.data_size == 0 or entry.width == 0 or entry.height == 0:
        return pixels
    data = pack[entry.data_offset:entry.data_offset + entry.data_size]
    if len(data) < 2:
        return pixels
    offset = 0
    row_count = u16(data, offset)
    offset += 2
    for _row in range(row_count):
        if offset + 4 > len(data):
            break
        rel_y = u16(data, offset)
        span_count = u16(data, offset + 2)
        offset += 4
        y = entry.y + rel_y
        for _span in range(span_count):
            if offset + 4 > len(data):
                break
            rel_x = u16(data, offset)
            pixel_count = u16(data, offset + 2)
            offset += 4
            if version == 1:
                span_bytes = (pixel_count + 1) // 2
                span = data[offset:offset + span_bytes]
                for i in range(pixel_count):
                    pixels[(entry.x + rel_x + i, y)] = pal4_index(span, i)
            else:
                span_bytes = pixel_count
                span = data[offset:offset + span_bytes]
                for i, value in enumerate(span):
                    pixels[(entry.x + rel_x + i, y)] = value
            offset += span_bytes
    return pixels


def spans_from_points(points: set[tuple[int, int]], origin_x: int, origin_y: int) -> bytes:
    rows: dict[int, list[int]] = {}
    for x, y in points:
        rows.setdefault(y, []).append(x)
    out = bytearray()
    out += struct.pack("<H", len(rows))
    for y in sorted(rows):
        xs = sorted(set(rows[y]))
        spans: list[tuple[int, int]] = []
        start = xs[0]
        prev = xs[0]
        for x in xs[1:]:
            if x == prev + 1:
                prev = x
                continue
            spans.append((start, prev + 1))
            start = x
            prev = x
        spans.append((start, prev + 1))
        out += struct.pack("<HH", y - origin_y, len(spans))
        for x0, x1 in spans:
            out += struct.pack("<HH", x0 - origin_x, x1 - x0)
    return bytes(out)


def draw_payload_from_pixels(pixels: dict[tuple[int, int], int],
                             origin_x: int, origin_y: int, version: int) -> bytes:
    rows: dict[int, list[tuple[int, int]]] = {}
    for (x, y), value in pixels.items():
        rows.setdefault(y, []).append((x, value))
    out = bytearray()
    out += struct.pack("<H", len(rows))
    for y in sorted(rows):
        items = sorted(rows[y])
        spans: list[list[tuple[int, int]]] = []
        current: list[tuple[int, int]] = [items[0]]
        for item in items[1:]:
            if item[0] == current[-1][0] + 1:
                current.append(item)
            else:
                spans.append(current)
                current = [item]
        spans.append(current)
        out += struct.pack("<HH", y - origin_y, len(spans))
        for span in spans:
            if version == 1:
                encoded = bytearray((len(span) + 1) // 2)
                for i, (_x, value) in enumerate(span):
                    if i & 1:
                        encoded[i >> 1] |= value & 0x0F
                    else:
                        encoded[i >> 1] = (value & 0x0F) << 4
            else:
                encoded = bytearray(len(span))
                for i, (_x, value) in enumerate(span):
                    encoded[i] = value & 0xFF
            out += struct.pack("<HH", span[0][0] - origin_x, len(span))
            out += encoded
    return bytes(out)


def bbox(points: set[tuple[int, int]]) -> tuple[int, int, int, int] | None:
    if not points:
        return None
    xs = [p[0] for p in points]
    ys = [p[1] for p in points]
    min_x = min(xs)
    min_y = min(ys)
    return min_x, min_y, max(xs) - min_x + 1, max(ys) - min_y + 1


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--input-fg2", required=True, type=Path)
    parser.add_argument("--output-fg3", required=True, type=Path)
    parser.add_argument(
        "--keyframe-source",
        action="append",
        type=int,
        default=[],
        help=(
            "Source-frame number to encode as a self-contained residual keyframe. "
            "The frame first restores all prior foreground pixels, then redraws "
            "the full current foreground state."
        ),
    )
    args = parser.parse_args()

    pack, header, palette, entries, sound_events = parse_fg2(args.input_fg2)
    out_entries: list[Entry] = []
    chunks: list[bytes] = []
    prev_pixels: dict[tuple[int, int], int] = {}
    keyframe_sources = set(args.keyframe_source)

    for entry in entries:
        current = decode_pixels(pack, entry, header.version)
        if not current and not prev_pixels:
            out_entries.append(Entry(entry.source_frame, 0, 0, 0, 0,
                                     entry.hold_vblanks, 0, 0))
            chunks.append(b"")
            prev_pixels = current
            continue

        if entry.source_frame in keyframe_sources:
            cleanup = set(prev_pixels.keys())
            draw = dict(current)
        else:
            cleanup = {
                point for point, value in prev_pixels.items()
                if current.get(point) != value
            }
            draw = {
                point: value for point, value in current.items()
                if prev_pixels.get(point) != value
            }
        dirty_points = set(cleanup) | set(draw.keys())
        dirty_bbox = bbox(dirty_points)
        if dirty_bbox is None:
            out_entries.append(Entry(entry.source_frame, 0, 0, 0, 0,
                                     entry.hold_vblanks, 0, 0))
            chunks.append(b"")
            prev_pixels = current
            continue

        x, y, width, height = dirty_bbox
        cleanup_payload = spans_from_points(cleanup, x, y)
        draw_payload = draw_payload_from_pixels(draw, x, y, header.version)
        chunk = cleanup_payload + draw_payload
        out_entries.append(Entry(entry.source_frame, x, y, width, height,
                                 entry.hold_vblanks, 0, len(chunk)))
        chunks.append(chunk)
        prev_pixels = current

    header_size = HEADER_SIZE
    palette_size = len(palette) * 2
    table_offset = header_size + palette_size
    data_offset = table_offset + len(out_entries) * ENTRY_SIZE
    next_offset = data_offset
    for entry, chunk in zip(out_entries, chunks):
        entry.data_offset = next_offset
        entry.data_size = len(chunk)
        next_offset += len(chunk)
    sound_offset = next_offset if sound_events else 0

    args.output_fg3.parent.mkdir(parents=True, exist_ok=True)
    with args.output_fg3.open("wb") as f:
        f.write(struct.pack(
            HEADER,
            b"FGP3",
            header.version,
            len(out_entries),
            header.display_vblanks,
            header.flags,
            header.screen_width,
            header.screen_height,
            header.union_x,
            header.union_y,
            header.union_width,
            header.union_height,
            table_offset,
            data_offset,
            sound_offset,
            header.sound_event_count,
            header.reserved1,
        ))
        for value in palette:
            f.write(struct.pack("<H", value))
        for entry in out_entries:
            f.write(struct.pack(
                ENTRY,
                entry.source_frame,
                entry.x,
                entry.y,
                entry.width,
                entry.height,
                entry.hold_vblanks,
                entry.data_offset,
                entry.data_size,
            ))
        for chunk in chunks:
            f.write(chunk)
        f.write(sound_events)

    original_payload = sum(entry.data_size for entry in entries)
    residual_payload = sum(entry.data_size for entry in out_entries)
    if original_payload:
        saved_percent = (original_payload - residual_payload) * 100.0 / original_payload
    else:
        saved_percent = 0.0
    print(
        f"{args.output_fg3}: FGP3/v{header.version} payload "
        f"{original_payload} -> {residual_payload} ({saved_percent:.2f}% saved)"
    )


if __name__ == "__main__":
    main()
