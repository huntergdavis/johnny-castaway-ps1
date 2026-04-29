#!/usr/bin/env python3
"""Trim zero padding from generated FGP3 foreground packs.

The padded-FGP3 rollout kept each converted pack at its original FGP2 file
size so CD layout stayed fixed during per-scene validation. This script is the
follow-up compaction pass: for every FGP3 pack, compute the last byte referenced
by the entry table or sound table and truncate only if all bytes after that
point are zero.
"""

from __future__ import annotations

import argparse
import struct
from pathlib import Path


HEADER = "<4sHHHHHHHHHHIIIHH"
ENTRY = "<HhhHHHII"
HEADER_SIZE = struct.calcsize(HEADER)
ENTRY_SIZE = struct.calcsize(ENTRY)


def valid_pack_size(data: bytes, path: Path) -> int | None:
    if len(data) < HEADER_SIZE:
        return None

    (
        magic,
        _version,
        frame_count,
        _display_vblanks,
        _flags,
        _screen_width,
        _screen_height,
        _union_x,
        _union_y,
        _union_width,
        _union_height,
        table_offset,
        data_offset,
        sound_events_offset,
        sound_event_count,
        _reserved1,
    ) = struct.unpack_from(HEADER, data, 0)

    if magic != b"FGP3":
        return None

    table_end = table_offset + frame_count * ENTRY_SIZE
    if table_offset < HEADER_SIZE or table_end > len(data):
        raise SystemExit(f"invalid FGP3 entry table in {path}")
    if data_offset < table_end or data_offset > len(data):
        raise SystemExit(f"invalid FGP3 data offset in {path}")

    valid_end = data_offset
    for index in range(frame_count):
        entry_offset = table_offset + index * ENTRY_SIZE
        (_source_frame, _x, _y, width, height, _hold_vblanks,
         payload_offset, payload_size) = struct.unpack_from(ENTRY, data, entry_offset)
        if payload_size == 0 and (width == 0 or height == 0):
            continue
        payload_end = payload_offset + payload_size
        if payload_offset < data_offset or payload_end > len(data):
            raise SystemExit(f"invalid FGP3 payload range in {path} entry {index}")
        valid_end = max(valid_end, payload_end)

    if sound_events_offset:
        sound_end = sound_events_offset + sound_event_count * 4
        if sound_events_offset < valid_end or sound_end > len(data):
            raise SystemExit(f"invalid FGP3 sound table range in {path}")
        valid_end = max(valid_end, sound_end)

    return valid_end


def compact(path: Path, dry_run: bool) -> tuple[int, int, bool]:
    data = path.read_bytes()
    valid_size = valid_pack_size(data, path)
    if valid_size is None:
        return len(data), len(data), False
    if valid_size > len(data):
        raise SystemExit(f"computed valid size exceeds file size for {path}")
    if valid_size == len(data):
        return len(data), valid_size, False

    padding = data[valid_size:]
    if any(padding):
        raise SystemExit(f"non-zero trailing bytes after valid FGP3 data in {path}")

    if not dry_run:
        path.write_bytes(data[:valid_size])
    return len(data), valid_size, True


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("paths", nargs="*", type=Path)
    parser.add_argument(
        "--pack-dir",
        type=Path,
        default=Path("generated/ps1/foreground"),
        help="Directory to scan when no explicit paths are supplied.",
    )
    parser.add_argument("--dry-run", action="store_true")
    args = parser.parse_args()

    paths = args.paths or sorted(args.pack_dir.glob("*.FG2"))
    trimmed = 0
    saved = 0
    scanned = 0
    for path in paths:
        original, compacted, changed = compact(path, args.dry_run)
        if original == compacted and not changed:
            if path.read_bytes()[:4] == b"FGP3":
                scanned += 1
            continue
        scanned += 1
        trimmed += 1
        saved += original - compacted
        print(f"{path}: {original} -> {compacted} (-{original - compacted})")

    mode = "would trim" if args.dry_run else "trimmed"
    print(f"{mode} {trimmed} / {scanned} FGP3 packs, saved {saved} bytes")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
