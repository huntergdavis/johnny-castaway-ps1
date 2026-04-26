#!/usr/bin/env python3
"""
Pack the approved holiday emblem PNGs into HOLIDAY.BMP and HOLIDAY.PSB.

The first four frames are the original Sierra sprites and are preserved from
jc_resources/extracted/bmp/HOLIDAY.BMP. New 32x32 emblems are appended in the
same order as the review sheet.
"""

from __future__ import annotations

import argparse
import re
import struct
import sys
from pathlib import Path

try:
    import yaml
    from PIL import Image
except ImportError as e:
    sys.stderr.write(f"error: {e}\n")
    sys.exit(2)

REPO = Path(__file__).resolve().parent.parent
YAML_PATH = REPO / "holidays.yml"
ICON_DIR = REPO / "scratch" / "holidays-emblems"
BMP_OUT = REPO / "jc_resources" / "extracted" / "bmp" / "HOLIDAY.BMP"
PSB_OUT = REPO / "jc_resources" / "transcoded" / "HOLIDAY.PSB"
REGISTRY = REPO / "src" / "psb_registry.h"

PSB_MAGIC = 0x31425350
PSB_VERSION = 1
HEADER_SIZE = 16
FRAME_SIZE = 12
ICON_SIZE = 32

# The review PNGs are already authored in the real Johnny palette.
DISPLAY_TO_GAME_INDEX = {
    i: i for i in range(16)
}


def slugify(name: str) -> str:
    return name.replace(" ", "_").replace("'", "").replace(".", "")


def align4(n: int) -> int:
    return (n + 3) & ~3


def packed_size(width: int, height: int) -> int:
    return ((width + 1) // 2) * height


def read_holidays() -> list[dict]:
    with open(YAML_PATH, "r", encoding="utf-8") as f:
        holidays = yaml.safe_load(f)
    if not isinstance(holidays, list):
        raise SystemExit("holidays.yml must contain a list")
    return holidays


def frame_dims(h: dict) -> tuple[int, int]:
    sprite = h.get("sprite") or {}
    return int(sprite.get("width", 0)), int(sprite.get("height", 0))


def original_frame_defs(holidays: list[dict]) -> list[dict]:
    originals = [h for h in holidays if h.get("existing_sprite") is not None]
    originals.sort(key=lambda h: int(h["existing_sprite"]))
    for idx, h in enumerate(originals):
        if int(h["existing_sprite"]) != idx:
            raise SystemExit("original existing_sprite values must be contiguous from 0")
    return originals


def new_frame_defs(holidays: list[dict]) -> list[dict]:
    return [h for h in holidays if h.get("existing_sprite") is None]


def read_original_chunks(holidays: list[dict]) -> list[tuple[int, int, bytes]]:
    raw = BMP_OUT.read_bytes()
    chunks: list[tuple[int, int, bytes]] = []
    offset = 0
    for h in original_frame_defs(holidays):
        width, height = frame_dims(h)
        size = packed_size(width, height)
        if len(raw) < offset + size:
            raise SystemExit(f"{BMP_OUT} is too small to contain original holiday frames")
        chunks.append((width, height, raw[offset:offset + size]))
        offset += size
    return chunks


def icon_png_to_sierra_chunk(path: Path) -> bytes:
    if not path.exists():
        raise SystemExit(f"missing emblem PNG: {path.relative_to(REPO)}")
    with Image.open(path) as im:
        if im.mode != "P":
            raise SystemExit(f"{path.relative_to(REPO)} must be indexed PNG mode P")
        if im.size != (ICON_SIZE, ICON_SIZE):
            raise SystemExit(
                f"{path.relative_to(REPO)} must be {ICON_SIZE}x{ICON_SIZE}, got {im.size}"
            )
        px = im.load()
        out = bytearray()
        for y in range(ICON_SIZE):
            for x in range(0, ICON_SIZE, 2):
                p0 = int(px[x, y])
                p1 = int(px[x + 1, y])
                if p0 > 15 or p1 > 15:
                    raise SystemExit(f"{path.relative_to(REPO)} uses palette index > 15")
                g0 = DISPLAY_TO_GAME_INDEX[p0]
                g1 = DISPLAY_TO_GAME_INDEX[p1]
                out.append((g0 << 4) | g1)  # Sierra order: high nibble = even pixel
        return bytes(out)


def build_sierra_frames(holidays: list[dict]) -> list[tuple[int, int, bytes]]:
    frames = read_original_chunks(holidays)
    for h in new_frame_defs(holidays):
        slug = slugify(h["short_name"])
        path = ICON_DIR / f"{int(h['id']):02d}-{slug}.png"
        frames.append((ICON_SIZE, ICON_SIZE, icon_png_to_sierra_chunk(path)))
    return frames


def sierra_to_psb_chunk(chunk: bytes) -> bytes:
    return bytes(((b & 0x0F) << 4) | (b >> 4) for b in chunk)


def write_raw_bmp(frames: list[tuple[int, int, bytes]]) -> int:
    data = b"".join(chunk for _, _, chunk in frames)
    BMP_OUT.write_bytes(data)
    return len(data)


def write_psb(frames: list[tuple[int, int, bytes]]) -> int:
    data_offset = align4(HEADER_SIZE + FRAME_SIZE * len(frames))
    frame_records = []
    chunks = []
    data_pos = 0

    for width, height, sierra_chunk in frames:
        psb_chunk = sierra_to_psb_chunk(sierra_chunk)
        padded_size = align4(len(psb_chunk))
        chunks.append(psb_chunk + (b"\0" * (padded_size - len(psb_chunk))))
        frame_records.append((width, height, data_pos, padded_size))
        data_pos += padded_size

    total_size = data_offset + data_pos
    header = struct.pack("<IHHII", PSB_MAGIC, PSB_VERSION, len(frames),
                         data_offset, total_size)
    table = b"".join(
        struct.pack("<HHII", width, height, offset, size)
        for width, height, offset, size in frame_records
    )
    padding = b"\0" * (data_offset - len(header) - len(table))
    PSB_OUT.write_bytes(header + table + padding + b"".join(chunks))
    return total_size


def update_psb_registry(psb_size: int) -> None:
    text = REGISTRY.read_text(encoding="utf-8")
    new_text, count = re.subn(
        r'\{ "HOLIDAY\.BMP", [0-9]+ \}',
        f'{{ "HOLIDAY.BMP", {psb_size} }}',
        text,
        count=1,
    )
    if count != 1:
        raise SystemExit("could not update HOLIDAY.BMP entry in psb_registry.h")
    REGISTRY.write_text(new_text, encoding="utf-8")


def verify_psb(expected_frames: int, expected_size: int) -> None:
    data = PSB_OUT.read_bytes()
    if len(data) != expected_size:
        raise SystemExit(f"PSB size mismatch: got {len(data)}, expected {expected_size}")
    magic, version, num_frames, data_offset, total_size = struct.unpack(
        "<IHHII", data[:HEADER_SIZE]
    )
    if magic != PSB_MAGIC or version != PSB_VERSION:
        raise SystemExit("invalid PSB header")
    if num_frames != expected_frames:
        raise SystemExit(f"PSB frame count mismatch: {num_frames} != {expected_frames}")
    if total_size != expected_size or data_offset > len(data):
        raise SystemExit("invalid PSB size fields")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--verify", action="store_true",
                        help="verify outputs after writing them")
    args = parser.parse_args()

    holidays = read_holidays()
    frames = build_sierra_frames(holidays)
    raw_size = write_raw_bmp(frames)
    psb_size = write_psb(frames)
    update_psb_registry(psb_size)
    if args.verify:
        verify_psb(len(frames), psb_size)

    print(f"wrote {BMP_OUT.relative_to(REPO)} ({raw_size} bytes, {len(frames)} frames)")
    print(f"wrote {PSB_OUT.relative_to(REPO)} ({psb_size} bytes, {len(frames)} frames)")
    print(f"updated {REGISTRY.relative_to(REPO)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
