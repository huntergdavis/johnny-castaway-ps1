#!/usr/bin/env python3
"""Encode selected FGP3/v4 payloads with an in-frame local LZ stream.

The PS1 runtime understands payloads beginning with:

  u16 0xfffd, bytes "L4", u32 expanded_size

Commands are COPY(u32 already_output_offset, u16 length) or
LITERAL(u16 length, bytes). This is meant for large self-repetitive residual
payloads where CD pressure is worse than a small byte-copy expansion pass.
"""

from __future__ import annotations

import argparse
import json
import struct
from dataclasses import dataclass
from pathlib import Path


HEADER = "<4sHHHHHHHHHHIIIHH"
ENTRY = "<HhhHHHII"
ENTRY_SIZE = struct.calcsize(ENTRY)
LOCAL_SENTINEL = 0xFFFD
CMD_COPY = 0
CMD_LITERAL = 1


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

    @classmethod
    def unpack(cls, data: bytes, offset: int) -> "Entry":
        return cls(*struct.unpack_from(ENTRY, data, offset))

    def pack(self) -> bytes:
        return struct.pack(
            ENTRY,
            self.source_frame,
            self.x,
            self.y,
            self.width,
            self.height,
            self.hold_vblanks,
            self.data_offset,
            self.data_size,
        )


def flush_literal(commands: list[tuple[int, int, int, bytes | None]],
                  literal: bytearray) -> None:
    if literal:
        commands.append((CMD_LITERAL, 0, len(literal), bytes(literal)))
        literal.clear()


def encode_local_lz(payload: bytes, min_match: int) -> tuple[bytes, dict]:
    if min_match < 4:
        raise ValueError("min_match must be at least 4")

    positions: dict[bytes, list[int]] = {}
    commands: list[tuple[int, int, int, bytes | None]] = []
    literal = bytearray()
    copied = 0
    i = 0
    size = len(payload)

    while i < size:
        best_pos = 0
        best_len = 0
        if i + min_match <= size:
            key = payload[i:i + 4]
            candidates = positions.get(key, [])
            for pos in reversed(candidates[-64:]):
                length = 0
                max_length = min(i - pos, size - i, 0xFFFF)
                while (length < max_length and
                       payload[pos + length] == payload[i + length] and
                       length < 0xFFFF):
                    length += 1
                if length > best_len:
                    best_pos = pos
                    best_len = length
                    if length >= 256:
                        break

        if best_len >= min_match:
            flush_literal(commands, literal)
            commands.append((CMD_COPY, best_pos, best_len, None))
            copied += best_len
            end = min(i + best_len, size - 3)
            for pos in range(i, end):
                positions.setdefault(payload[pos:pos + 4], []).append(pos)
            i += best_len
        else:
            if i + 4 <= size:
                positions.setdefault(payload[i:i + 4], []).append(i)
            literal.append(payload[i])
            i += 1
            if len(literal) == 0xFFFF:
                flush_literal(commands, literal)

    flush_literal(commands, literal)
    out = bytearray()
    out += struct.pack("<HBBI", LOCAL_SENTINEL, ord("L"), ord("4"),
                       len(payload))
    literal_bytes = 0
    for opcode, offset, length, data in commands:
        out.append(opcode)
        if opcode == CMD_COPY:
            out += struct.pack("<IH", offset, length)
        else:
            assert data is not None
            out += struct.pack("<H", length)
            out += data
            literal_bytes += length

    return bytes(out), {
        "commands": len(commands),
        "copy_bytes": copied,
        "literal_bytes": literal_bytes,
        "encoded_bytes": len(out),
        "expanded_bytes": len(payload),
    }


def validate_ranges(entries: list[Entry], payloads: dict[int, bytes],
                    file_size: int) -> None:
    ranges: list[tuple[int, int, int]] = []
    for index, entry in enumerate(entries):
        if entry.data_size == 0:
            continue
        end = entry.data_offset + entry.data_size
        if entry.data_offset < 0 or end > file_size:
            raise SystemExit(f"entry {index} payload range outside file")
        ranges.append((entry.data_offset, end, index))

    ranges.sort()
    prev_start = prev_end = prev_index = -1
    for start, end, index in ranges:
        if prev_end > start:
            prev_entry = entries[prev_index]
            cur_entry = entries[index]
            same_alias = (
                start == prev_start and
                end == prev_end and
                cur_entry.data_size == prev_entry.data_size and
                payloads[index] == payloads[prev_index]
            )
            if not same_alias:
                raise SystemExit(
                    f"payload overlap: entry {prev_index} {prev_start}..{prev_end} "
                    f"with entry {index} {start}..{end}"
                )
        prev_start, prev_end, prev_index = start, end, index


def convert(args: argparse.Namespace) -> dict:
    data = args.input_fgp3.read_bytes()
    header = struct.unpack_from(HEADER, data, 0)
    (
        magic,
        version,
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
        _data_offset,
        _sound_events_offset,
        _sound_event_count,
        _reserved1,
    ) = header

    if magic != b"FGP3" or version != 4:
        raise SystemExit(f"{args.input_fgp3} is not an FGP3/v4 pack")
    if table_offset + frame_count * ENTRY_SIZE > len(data):
        raise SystemExit(f"entry table extends beyond pack: {args.input_fgp3}")

    entries = [
        Entry.unpack(data, table_offset + index * ENTRY_SIZE)
        for index in range(frame_count)
    ]
    payloads = {
        index: data[entry.data_offset:entry.data_offset + entry.data_size]
        for index, entry in enumerate(entries)
    }

    stats: dict[str, dict] = {}
    for index in args.local_lz_frame:
        if index < 0 or index >= frame_count:
            raise SystemExit(f"local-LZ frame out of range: {index}")
        target = payloads[index]
        encoded, frame_stats = encode_local_lz(target, args.min_match)
        if len(encoded) >= len(target) and not args.allow_larger:
            raise SystemExit(
                f"local-LZ frame {index} is not smaller: {len(encoded)} >= {len(target)}"
            )
        payloads[index] = encoded
        entries[index].data_size = len(encoded)
        stats[str(index)] = frame_stats | {
            "old_bytes": len(target),
            "saved_bytes": len(target) - len(encoded),
            "old_sectors": (len(target) + 2047) // 2048,
            "new_sectors": (len(encoded) + 2047) // 2048,
        }

    validate_ranges(entries, payloads, len(data))

    out = bytearray(data)
    for index, entry in enumerate(entries):
        table_entry_offset = table_offset + index * ENTRY_SIZE
        out[table_entry_offset:table_entry_offset + ENTRY_SIZE] = entry.pack()
        if entry.data_size:
            payload = payloads[index]
            out[entry.data_offset:entry.data_offset + entry.data_size] = payload

    args.output_fgp3.parent.mkdir(parents=True, exist_ok=True)
    args.output_fgp3.write_bytes(out)

    summary = {
        "input": str(args.input_fgp3),
        "output": str(args.output_fgp3),
        "file_bytes": len(out),
        "local_lz_frames": stats,
    }
    if args.summary_json:
        args.summary_json.parent.mkdir(parents=True, exist_ok=True)
        args.summary_json.write_text(json.dumps(summary, indent=2) + "\n", encoding="utf-8")
    return summary


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--input-fgp3", type=Path, required=True)
    parser.add_argument("--output-fgp3", type=Path, required=True)
    parser.add_argument("--summary-json", type=Path)
    parser.add_argument("--local-lz-frame", type=int, action="append", default=[])
    parser.add_argument("--min-match", type=int, default=8)
    parser.add_argument("--allow-larger", action="store_true")
    args = parser.parse_args()
    print(json.dumps(convert(args), indent=2))


if __name__ == "__main__":
    main()
