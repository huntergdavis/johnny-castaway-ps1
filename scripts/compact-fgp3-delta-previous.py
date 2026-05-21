#!/usr/bin/env python3
"""Delta-pack selected FGP3/v4 frames against the previous frame payload.

The runtime understands payloads beginning with:

  u16 0xfffe, bytes "D4", u16 expanded_size, u16 command_count

Commands are either COPY(u16 base_offset, u16 length) from the previous full
payload or LITERAL(u16 length, bytes). This tool is intentionally conservative:
it only rewrites the
requested frame table entries and payload slots, and by default keeps the input
file size unchanged so following CD files keep their LBAs.
"""

from __future__ import annotations

import argparse
import difflib
import json
import struct
from dataclasses import dataclass
from pathlib import Path


HEADER = "<4sHHHHHHHHHHIIIHH"
ENTRY = "<HhhHHHII"
HEADER_SIZE = struct.calcsize(HEADER)
ENTRY_SIZE = struct.calcsize(ENTRY)
DELTA_SENTINEL = 0xFFFE
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


def emit_literal(commands: list[tuple[int, int, bytes | None]], literal: bytearray) -> None:
    if not literal:
        return
    commands.append((CMD_LITERAL, len(literal), bytes(literal)))
    literal.clear()


def encode_delta(base: bytes, target: bytes, min_match: int) -> tuple[bytes, dict]:
    if len(target) > 0xFFFF:
        raise ValueError("delta target too large for u16 expanded_size")

    matcher = difflib.SequenceMatcher(None, base, target, autojunk=False)
    commands: list[tuple[int, int, bytes | None]] = []
    literal = bytearray()
    copied = 0
    literal_bytes = 0

    for tag, i1, i2, _j1, j2 in matcher.get_opcodes():
        match_len = i2 - i1
        if tag == "equal" and match_len >= min_match:
            emit_literal(commands, literal)
            commands.append((CMD_COPY, (i1 << 16) | match_len, None))
            copied += match_len
        else:
            literal.extend(target[_j1:j2])

    emit_literal(commands, literal)
    literal_bytes = sum(len(payload or b"") for opcode, _first, payload in commands
                        if opcode == CMD_LITERAL)

    out = bytearray()
    out += struct.pack("<HBBHH", DELTA_SENTINEL, ord("D"), ord("4"),
                       len(target), len(commands))
    for opcode, first, payload in commands:
        out.append(opcode)
        if opcode == CMD_COPY:
            base_offset, length = first >> 16, first & 0xFFFF
            out += struct.pack("<HH", base_offset, length)
        else:
            assert payload is not None
            out += struct.pack("<H", len(payload))
            out += payload

    stats = {
        "commands": len(commands),
        "copy_bytes": copied,
        "literal_bytes": literal_bytes,
        "encoded_bytes": len(out),
        "expanded_bytes": len(target),
    }
    return bytes(out), stats


def decode_delta_payload(encoded: bytes, base: bytes) -> bytes:
    if (
        len(encoded) < 8 or
        struct.unpack_from("<H", encoded, 0)[0] != DELTA_SENTINEL or
        encoded[2] != ord("D") or
        encoded[3] != ord("4")
    ):
        return encoded

    expanded_size, command_count = struct.unpack_from("<HH", encoded, 4)
    read_offset = 8
    out = bytearray()
    for _ in range(command_count):
        if read_offset >= len(encoded):
            raise ValueError("delta command extends beyond payload")
        opcode = encoded[read_offset]
        read_offset += 1
        if opcode == CMD_COPY:
            if read_offset + 4 > len(encoded):
                raise ValueError("delta copy command truncated")
            base_offset, length = struct.unpack_from("<HH", encoded, read_offset)
            read_offset += 4
            if base_offset + length > len(base):
                raise ValueError("delta copy range extends beyond base")
            out += base[base_offset:base_offset + length]
        elif opcode == CMD_LITERAL:
            if read_offset + 2 > len(encoded):
                raise ValueError("delta literal command truncated")
            (length,) = struct.unpack_from("<H", encoded, read_offset)
            read_offset += 2
            if read_offset + length > len(encoded):
                raise ValueError("delta literal range extends beyond payload")
            out += encoded[read_offset:read_offset + length]
            read_offset += length
        else:
            raise ValueError(f"unknown delta opcode: {opcode}")

    if len(out) != expanded_size:
        raise ValueError(
            f"delta expanded size mismatch: {len(out)} != {expanded_size}"
        )
    return bytes(out)


def parse_index_offset(raw: str) -> tuple[int, int]:
    try:
        index_raw, offset_raw = raw.split(":", 1)
        return int(index_raw, 0), int(offset_raw, 0)
    except ValueError as exc:
        raise argparse.ArgumentTypeError(f"expected INDEX:OFFSET, got {raw!r}") from exc


def parse_index_after(raw: str) -> tuple[int, int]:
    try:
        index_raw, after_raw = raw.split(":", 1)
        return int(index_raw, 0), int(after_raw, 0)
    except ValueError as exc:
        raise argparse.ArgumentTypeError(f"expected INDEX:AFTER_INDEX, got {raw!r}") from exc


def validate_ranges(entries: list[Entry], payloads: dict[int, bytes], file_size: int) -> None:
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
    if len(data) < HEADER_SIZE:
        raise SystemExit(f"pack too small: {args.input_fgp3}")

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
        raise SystemExit(f"{args.input_fgp3} is not an FGP3/v4 PAL4 compact residual pack")
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
    decoded_payloads: dict[int, bytes] = {}
    for index in range(frame_count):
        if index == 0:
            decoded_payloads[index] = payloads[index]
        else:
            try:
                decoded_payloads[index] = decode_delta_payload(
                    payloads[index], decoded_payloads[index - 1]
                )
            except ValueError as exc:
                raise SystemExit(f"could not decode frame {index}: {exc}") from exc

    delta_stats: dict[str, dict] = {}
    for index in args.delta_frame:
        if index <= 0 or index >= frame_count:
            raise SystemExit(f"delta frame out of range: {index}")
        base = decoded_payloads[index - 1]
        target = decoded_payloads[index]
        old_payload_size = len(payloads[index])
        encoded, stats = encode_delta(base, target, args.min_match)
        if len(encoded) >= old_payload_size and not args.allow_larger:
            raise SystemExit(
                f"delta frame {index} is not smaller: {len(encoded)} >= {old_payload_size}"
            )
        payloads[index] = encoded
        entries[index].data_size = len(encoded)
        decoded_payloads[index] = target
        delta_stats[str(index)] = stats | {
            "old_bytes": old_payload_size,
            "saved_bytes": old_payload_size - len(encoded),
            "expanded_old_bytes": len(target),
            "base_frame": index - 1,
        }

    for index, offset in args.move_frame:
        if index < 0 or index >= frame_count:
            raise SystemExit(f"move frame out of range: {index}")
        entries[index].data_offset = offset

    for index, after_index in args.place_after:
        if index < 0 or index >= frame_count or after_index < 0 or after_index >= frame_count:
            raise SystemExit(f"place-after frame out of range: {index}:{after_index}")
        entries[index].data_offset = entries[after_index].data_offset + entries[after_index].data_size

    validate_ranges(entries, payloads, len(data))

    out = bytearray(data)
    for index, entry in enumerate(entries):
        table_entry_offset = table_offset + index * ENTRY_SIZE
        out[table_entry_offset:table_entry_offset + ENTRY_SIZE] = entry.pack()
        if entry.data_size:
            payload = payloads[index]
            if len(payload) != entry.data_size:
                raise SystemExit(f"entry {index} payload size mismatch")
            out[entry.data_offset:entry.data_offset + entry.data_size] = payload

    args.output_fgp3.parent.mkdir(parents=True, exist_ok=True)
    args.output_fgp3.write_bytes(out)

    summary = {
        "input": str(args.input_fgp3),
        "output": str(args.output_fgp3),
        "file_bytes": len(out),
        "delta_frames": delta_stats,
        "moved_frames": [
            {"frame": index, "offset": entries[index].data_offset, "size": entries[index].data_size}
            for index, _offset in args.move_frame
        ],
        "placed_frames": [
            {"frame": index, "after": after_index, "offset": entries[index].data_offset,
             "size": entries[index].data_size}
            for index, after_index in args.place_after
        ],
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
    parser.add_argument("--delta-frame", type=int, action="append", default=[])
    parser.add_argument("--move-frame", type=parse_index_offset, action="append", default=[])
    parser.add_argument("--place-after", type=parse_index_after, action="append", default=[])
    parser.add_argument("--min-match", type=int, default=8)
    parser.add_argument("--allow-larger", action="store_true")
    args = parser.parse_args()
    summary = convert(args)
    print(json.dumps(summary, indent=2))


if __name__ == "__main__":
    main()
