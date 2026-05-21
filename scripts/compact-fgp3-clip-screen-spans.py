#!/usr/bin/env python3
"""Clip offscreen PAL4 FGP3/v4 residual spans to the 640x480 PS1 screen.

The runtime already clips spans during draw/restore. This pack-side transform
removes pixels that can never reach the visible screen, preserving frame timing
and optionally preserving payload offsets for LBA-stable perf probes.
"""

from __future__ import annotations

import argparse
import json
import shutil
import struct
import tempfile
from dataclasses import dataclass
from pathlib import Path


HEADER = "<4sHHHHHHHHHHIIIHH"
ENTRY = "<HhhHHHII"
HEADER_SIZE = struct.calcsize(HEADER)
ENTRY_SIZE = struct.calcsize(ENTRY)
SCREEN_W = 640
SCREEN_H = 480


@dataclass
class Span:
    rel_x: int
    count: int
    pixels: bytes = b""


@dataclass
class Row:
    rel_y: int
    spans: list[Span]


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


def slice_pal4(data: bytes, start: int, count: int) -> bytes:
    out = bytearray((count + 1) // 2)
    for i in range(count):
        value = pal4_index(data, start + i)
        if i & 1:
            out[i >> 1] |= value
        else:
            out[i >> 1] = value << 4
    return bytes(out)


def parse_rows(payload: bytes, offset: int, limit: int, *, has_pixels: bool) -> tuple[list[Row], int]:
    if offset + 2 > limit:
        raise ValueError("truncated row count")
    row_count = read_u16(payload, offset)
    offset += 2
    rows: list[Row] = []
    for _ in range(row_count):
        rel_y, offset = read_compact_u16(payload, offset, limit)
        span_count, offset = read_compact_u16(payload, offset, limit)
        spans: list[Span] = []
        for _span in range(span_count):
            rel_x, offset = read_compact_u16(payload, offset, limit)
            pixel_count, offset = read_compact_u16(payload, offset, limit)
            pixels = b""
            if has_pixels:
                packed_bytes = (pixel_count + 1) // 2
                if offset + packed_bytes > limit:
                    raise ValueError("truncated span pixels")
                pixels = payload[offset:offset + packed_bytes]
                offset += packed_bytes
            spans.append(Span(rel_x, pixel_count, pixels))
        rows.append(Row(rel_y, spans))
    return rows, offset


def encode_rows(rows: list[Row], *, has_pixels: bool) -> bytes:
    active_rows = [row for row in rows if row.spans]
    out = bytearray()
    out += struct.pack("<H", len(active_rows))
    for row in active_rows:
        write_compact_u16(out, row.rel_y)
        write_compact_u16(out, len(row.spans))
        for span in row.spans:
            write_compact_u16(out, span.rel_x)
            write_compact_u16(out, span.count)
            if has_pixels:
                out += span.pixels
    return bytes(out)


def clip_rows(rows: list[Row], entry_x: int, entry_y: int, *, has_pixels: bool) -> tuple[list[Row], int]:
    clipped_rows: list[Row] = []
    removed_pixels = 0
    for row in rows:
        screen_y = entry_y + row.rel_y
        if screen_y < 0 or screen_y >= SCREEN_H:
            removed_pixels += sum(span.count for span in row.spans)
            continue
        clipped_spans: list[Span] = []
        for span in row.spans:
            screen_x = entry_x + span.rel_x
            keep_start = max(0, -screen_x)
            keep_end = min(span.count, SCREEN_W - screen_x)
            if keep_start >= keep_end:
                removed_pixels += span.count
                continue
            new_count = keep_end - keep_start
            removed_pixels += span.count - new_count
            new_pixels = slice_pal4(span.pixels, keep_start, new_count) if has_pixels else b""
            clipped_spans.append(Span(span.rel_x + keep_start, new_count, new_pixels))
        if clipped_spans:
            clipped_rows.append(Row(row.rel_y, clipped_spans))
    return clipped_rows, removed_pixels


def clip_payload(payload: bytes, entry_x: int, entry_y: int) -> tuple[bytes, dict[str, int]]:
    cleanup_rows, draw_offset = parse_rows(payload, 0, len(payload), has_pixels=False)
    draw_rows: list[Row] = []
    draw_end = draw_offset
    if draw_offset < len(payload):
        draw_rows, draw_end = parse_rows(payload, draw_offset, len(payload), has_pixels=True)
    if any(payload[draw_end:]):
        raise ValueError("payload has nonzero trailing bytes")

    cleanup_rows, cleanup_removed = clip_rows(cleanup_rows, entry_x, entry_y, has_pixels=False)
    draw_rows, draw_removed = clip_rows(draw_rows, entry_x, entry_y, has_pixels=True)
    new_payload = encode_rows(cleanup_rows, has_pixels=False)
    if draw_rows:
        new_payload += encode_rows(draw_rows, has_pixels=True)
    return new_payload, {
        "cleanup_removed_pixels": cleanup_removed,
        "draw_removed_pixels": draw_removed,
    }


def convert(
    input_path: Path,
    output_path: Path,
    *,
    pad_to_input_size: bool,
    preserve_offsets: bool,
    preserve_entry_sizes: bool,
    copy_unparseable: bool,
    selected_frames: set[int] | None,
    frames_mode: str,
    summary_path: Path | None,
) -> dict:
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

    if magic != b"FGP3" or version != 4:
        raise SystemExit(f"{input_path} is not an FGP3/v4 PAL4 compact residual pack")
    if table_offset + frame_count * ENTRY_SIZE > len(data):
        raise SystemExit(f"entry table extends beyond pack: {input_path}")

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

    summary: dict[str, object] = {
        "input": str(input_path),
        "output": str(output_path),
        "preserve_offsets": preserve_offsets,
        "old_size": len(data),
        "new_size": len(data),
        "preserve_entry_sizes": preserve_entry_sizes,
        "changed_entries": 0,
        "old_payload_bytes": 0,
        "new_payload_bytes": 0,
        "saved_payload_bytes": 0,
        "cleanup_removed_pixels": 0,
        "draw_removed_pixels": 0,
        "copied_unparseable_entries": 0,
        "frames_mode": frames_mode,
        "changed_entry_details": [],
    }

    if preserve_offsets:
        out = bytearray(data)
        for index, entry in enumerate(old_entries):
            source_frame, x, y, width, height, hold_vblanks, old_offset, old_size = entry
            if not should_rewrite_entry(index, source_frame, selected_frames, frames_mode):
                continue
            if old_size == 0 or width == 0 or height == 0:
                continue
            if old_offset + old_size > len(data):
                raise SystemExit(f"entry {index} payload extends beyond pack: {input_path}")
            old_payload = data[old_offset:old_offset + old_size]
            summary["old_payload_bytes"] = int(summary["old_payload_bytes"]) + old_size
            try:
                new_payload, stats = clip_payload(old_payload, x, y)
            except ValueError:
                if not copy_unparseable:
                    raise
                summary["copied_unparseable_entries"] = int(summary["copied_unparseable_entries"]) + 1
                summary["new_payload_bytes"] = int(summary["new_payload_bytes"]) + old_size
                continue
            if len(new_payload) > old_size:
                raise SystemExit(f"entry {index} grew unexpectedly: {len(new_payload)} > {old_size}")
            summary["new_payload_bytes"] = int(summary["new_payload_bytes"]) + len(new_payload)
            if len(new_payload) == old_size and not stats["cleanup_removed_pixels"] and not stats["draw_removed_pixels"]:
                continue
            out[old_offset:old_offset + len(new_payload)] = new_payload
            out[old_offset + len(new_payload):old_offset + old_size] = b"\0" * (old_size - len(new_payload))
            table_size = old_size if preserve_entry_sizes else len(new_payload)
            struct.pack_into(ENTRY, out, table_offset + index * ENTRY_SIZE,
                             source_frame, x, y, width, height, hold_vblanks,
                             old_offset, table_size)
            summary["changed_entries"] = int(summary["changed_entries"]) + 1
            summary["cleanup_removed_pixels"] = int(summary["cleanup_removed_pixels"]) + stats["cleanup_removed_pixels"]
            summary["draw_removed_pixels"] = int(summary["draw_removed_pixels"]) + stats["draw_removed_pixels"]
            summary["changed_entry_details"].append({
                "index": index,
                "source_frame": source_frame,
                "old_size": old_size,
                "new_size": len(new_payload),
                "cleanup_removed_pixels": stats["cleanup_removed_pixels"],
                "draw_removed_pixels": stats["draw_removed_pixels"],
            })
        output_path.write_bytes(out)
        summary["saved_payload_bytes"] = int(summary["old_payload_bytes"]) - int(summary["new_payload_bytes"])
    else:
        payload_out = bytearray()
        new_entries: list[tuple[int, int, int, int, int, int, int, int]] = []
        cursor = data_offset
        for index, entry in enumerate(old_entries):
            source_frame, x, y, width, height, hold_vblanks, old_offset, old_size = entry
            if old_size == 0 or width == 0 or height == 0:
                new_entries.append((source_frame, x, y, width, height, hold_vblanks, 0, 0))
                continue
            if old_offset + old_size > len(data):
                raise SystemExit(f"entry {index} payload extends beyond pack: {input_path}")
            old_payload = data[old_offset:old_offset + old_size]
            summary["old_payload_bytes"] = int(summary["old_payload_bytes"]) + old_size
            try:
                if should_rewrite_entry(index, source_frame, selected_frames, frames_mode):
                    new_payload, stats = clip_payload(old_payload, x, y)
                else:
                    new_payload, stats = old_payload, {"cleanup_removed_pixels": 0, "draw_removed_pixels": 0}
            except ValueError:
                if not copy_unparseable:
                    raise
                new_payload, stats = old_payload, {"cleanup_removed_pixels": 0, "draw_removed_pixels": 0}
                summary["copied_unparseable_entries"] = int(summary["copied_unparseable_entries"]) + 1
            if len(new_payload) != old_size or stats["cleanup_removed_pixels"] or stats["draw_removed_pixels"]:
                summary["changed_entries"] = int(summary["changed_entries"]) + 1
                summary["cleanup_removed_pixels"] = int(summary["cleanup_removed_pixels"]) + stats["cleanup_removed_pixels"]
                summary["draw_removed_pixels"] = int(summary["draw_removed_pixels"]) + stats["draw_removed_pixels"]
            new_entries.append((source_frame, x, y, width, height, hold_vblanks, cursor, len(new_payload)))
            payload_out += new_payload
            cursor += len(new_payload)
            summary["new_payload_bytes"] = int(summary["new_payload_bytes"]) + len(new_payload)

        new_sound_events_offset = cursor if sound_events else 0
        header = struct.pack(HEADER, b"FGP3", 4, frame_count, display_vblanks, flags,
                             screen_width, screen_height, union_x, union_y, union_width,
                             union_height, table_offset, data_offset, new_sound_events_offset,
                             sound_event_count, reserved1)
        out = bytearray(header)
        out += data[HEADER_SIZE:table_offset]
        out += b"\0" * (table_offset - len(out))
        for entry in new_entries:
            out += struct.pack(ENTRY, *entry)
        out += b"\0" * (data_offset - len(out))
        out += payload_out
        out += sound_events
        if pad_to_input_size:
            if len(out) > len(data):
                raise SystemExit(f"compact pack grew unexpectedly: {len(out)} > {len(data)}")
            out += b"\0" * (len(data) - len(out))
        output_path.write_bytes(out)
        summary["new_size"] = len(out)
        summary["saved_payload_bytes"] = int(summary["old_payload_bytes"]) - int(summary["new_payload_bytes"])

    if summary_path is not None:
        summary_path.write_text(json.dumps(summary, indent=2, sort_keys=True) + "\n")
    print(
        f"{input_path} -> {output_path}: {len(data)} -> {summary['new_size']} bytes, "
        f"active_payload {summary['old_payload_bytes']} -> {summary['new_payload_bytes']}, "
        f"changed_entries={summary['changed_entries']}, "
        f"cleanup_removed_pixels={summary['cleanup_removed_pixels']}, "
        f"draw_removed_pixels={summary['draw_removed_pixels']}, "
        f"copied_unparseable={summary['copied_unparseable_entries']}"
    )
    return summary


def parse_frames(raw: str | None) -> set[int] | None:
    if not raw:
        return None
    out: set[int] = set()
    for part in raw.split(","):
        part = part.strip()
        if not part:
            continue
        out.add(int(part, 0))
    return out


def should_rewrite_entry(
    entry_index: int,
    source_frame: int,
    selected_frames: set[int] | None,
    frames_mode: str,
) -> bool:
    if selected_frames is None:
        return True
    if frames_mode == "entry":
        return entry_index in selected_frames
    if frames_mode == "source":
        return source_frame in selected_frames
    return entry_index in selected_frames or source_frame in selected_frames


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input_fgp3", type=Path)
    parser.add_argument("output_fgp3", type=Path, nargs="?")
    parser.add_argument("--in-place", action="store_true")
    parser.add_argument("--pad-to-input-size", action="store_true")
    parser.add_argument("--preserve-offsets", action="store_true")
    parser.add_argument(
        "--preserve-entry-sizes",
        action="store_true",
        help="with --preserve-offsets, keep each changed entry's dataSize unchanged",
    )
    parser.add_argument("--copy-unparseable", action="store_true")
    parser.add_argument("--frames", help="comma-separated entry or source frame numbers to rewrite")
    parser.add_argument(
        "--frames-mode",
        choices=("either", "entry", "source"),
        default="either",
        help="interpret --frames as entry indices, source frame numbers, or either match",
    )
    parser.add_argument("--summary", type=Path)
    args = parser.parse_args()

    if args.in_place == bool(args.output_fgp3):
        raise SystemExit("pass either --in-place or an output path")
    if args.preserve_entry_sizes and not args.preserve_offsets:
        raise SystemExit("--preserve-entry-sizes requires --preserve-offsets")
    selected_frames = parse_frames(args.frames)
    if args.in_place:
        with tempfile.NamedTemporaryFile(prefix=f"{args.input_fgp3.name}.",
                                         dir=args.input_fgp3.parent,
                                         delete=False) as tmp:
            tmp_path = Path(tmp.name)
        try:
            convert(args.input_fgp3, tmp_path, pad_to_input_size=args.pad_to_input_size,
                    preserve_offsets=args.preserve_offsets,
                    preserve_entry_sizes=args.preserve_entry_sizes,
                    copy_unparseable=args.copy_unparseable,
                    selected_frames=selected_frames, frames_mode=args.frames_mode,
                    summary_path=args.summary)
            shutil.move(str(tmp_path), str(args.input_fgp3))
        finally:
            tmp_path.unlink(missing_ok=True)
    else:
        convert(args.input_fgp3, args.output_fgp3, pad_to_input_size=args.pad_to_input_size,
                preserve_offsets=args.preserve_offsets,
                preserve_entry_sizes=args.preserve_entry_sizes,
                copy_unparseable=args.copy_unparseable,
                selected_frames=selected_frames, frames_mode=args.frames_mode,
                summary_path=args.summary)


if __name__ == "__main__":
    main()
