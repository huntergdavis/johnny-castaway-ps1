#!/usr/bin/env python3
"""Repair ACTIVITY9's scene-local boat foreground after stitched capture.

The host can only capture the part of BOAT.BMP that falls inside the current
screen window. ACTIVITY9 intentionally moves that boat through scene-local
space wider than one viewport, so a stitched foreground can still have hard
vertical cuts on the entering stern or exiting bow. Host frame metadata already
records the real BOAT.BMP draw position for every frame; use that to fill only
missing keyed pixels from the source BOAT.PSB sprite without overwriting any
foreground pixels that the host already captured.
"""

from __future__ import annotations

import json
import struct
import sys
from pathlib import Path

from PIL import Image


KEY = (255, 0, 255)
LEGACY_SCENE_CLIP_MIN_X = 0
LEGACY_SCENE_CLIP_MAX_X = 640
# The stitched host capture can leave a one-column seam right at the legacy
# screen clip edge. Fill a small overlap band from BOAT.PSB, but only into
# keyed holes so we do not repaint host-captured island/tree pixels.
CLIP_EDGE_OVERLAP_X = 12
PALETTE = [
    (168, 0, 168),
    (0, 0, 168),
    (0, 168, 0),
    (0, 168, 168),
    (168, 0, 0),
    (0, 0, 0),
    (168, 168, 0),
    (212, 212, 212),
    (128, 128, 128),
    (0, 0, 252),
    (0, 252, 0),
    (0, 252, 252),
    (252, 0, 0),
    (252, 0, 252),
    (252, 252, 0),
    (252, 252, 252),
]


def read_meta(capture_dir: Path, frame_name: str) -> dict:
    meta_path = capture_dir / "frame-meta" / Path(frame_name).with_suffix(".json").name
    if not meta_path.exists():
        raise SystemExit(f"missing frame metadata: {meta_path}")
    return json.loads(meta_path.read_text(encoding="utf-8"))


def decode_first_psb_frame(psb_path: Path) -> Image.Image:
    data = psb_path.read_bytes()
    if len(data) < 28:
        raise SystemExit(f"PSB too small: {psb_path}")
    magic, version, frame_count, data_offset, _total_size = struct.unpack_from("<IHHII", data, 0)
    if magic != 0x31425350 or version != 1 or frame_count < 1:
        raise SystemExit(f"invalid PSB header: {psb_path}")
    width, height, frame_offset, frame_size = struct.unpack_from("<HHII", data, 16)
    pixel_count = (width * height + 1) // 2
    if pixel_count > frame_size:
        raise SystemExit(f"invalid PSB frame size: {psb_path}")
    pixels = data[data_offset + frame_offset : data_offset + frame_offset + pixel_count]

    img = Image.new("RGB", (width, height), KEY)
    out = img.load()
    pos = 0
    for y in range(height):
        for x in range(0, width, 2):
            packed = pixels[pos]
            pos += 1
            for dx, index in ((0, packed & 0x0F), (1, packed >> 4)):
                if x + dx >= width or index == 0:
                    continue
                out[x + dx, y] = PALETTE[index]
    return img


def non_key_bbox(img: Image.Image) -> tuple[int, int, int, int]:
    pix = img.load()
    min_x = img.width
    min_y = img.height
    max_x = -1
    max_y = -1
    for y in range(img.height):
        for x in range(img.width):
            if pix[x, y] == KEY:
                continue
            min_x = min(min_x, x)
            min_y = min(min_y, y)
            max_x = max(max_x, x)
            max_y = max(max_y, y)
    if max_x < min_x or max_y < min_y:
        raise SystemExit("BOAT.PSB frame has no visible pixels")
    return min_x, min_y, max_x + 1, max_y + 1


def boat_draws(reference_capture: Path, frame_name: str) -> list[tuple[int, int, int, int]]:
    meta = read_meta(reference_capture, frame_name)
    offset_x = int(meta.get("scene_offset_x", 0) or 0)
    offset_y = int(meta.get("scene_offset_y", 0) or 0)
    draws = []
    for draw in meta.get("draws", []):
        if draw.get("bmp_name") != "BOAT.BMP":
            continue
        if draw.get("flipped"):
            continue
        draws.append(
            (
                int(draw["x"]) - offset_x,
                int(draw["y"]) - offset_y,
                int(draw["width"]),
                int(draw["height"]),
            )
        )
    return draws


def patch_activity9_boat(reference_capture: Path, merged_capture: Path, psb_path: Path) -> None:
    frames_dir = merged_capture / "frames"
    meta_dir = merged_capture / "frame-meta"
    frame_paths = sorted(frames_dir.glob("frame_*.bmp"))
    if not frame_paths:
        raise SystemExit(f"no merged frames found: {frames_dir}")

    boat = decode_first_psb_frame(psb_path)
    boat_bbox = non_key_bbox(boat)
    boat_pixels = [
        (x, y, boat.getpixel((x, y)))
        for y in range(boat_bbox[1], boat_bbox[3])
        for x in range(boat_bbox[0], boat_bbox[2])
        if boat.getpixel((x, y)) != KEY
    ]

    old_meta = read_meta(merged_capture, frame_paths[0].name)
    old_offset_x = int(old_meta.get("scene_offset_x", 0) or 0)
    old_offset_y = int(old_meta.get("scene_offset_y", 0) or 0)
    with Image.open(frame_paths[0]) as first_raw:
        old_width, old_height = first_raw.size

    min_local_x = -old_offset_x
    min_local_y = -old_offset_y
    max_local_x = old_width - old_offset_x - 1
    max_local_y = old_height - old_offset_y - 1
    raw_frame_draws: dict[str, list[tuple[int, int, int, int]]] = {}
    for frame_path in frame_paths:
        raw_frame_draws[frame_path.name] = boat_draws(reference_capture, frame_path.name)

    first_boat_index = None
    last_boat_index = None
    for index, frame_path in enumerate(frame_paths):
        if raw_frame_draws[frame_path.name]:
            if first_boat_index is None:
                first_boat_index = index
            last_boat_index = index

    per_frame_draws: dict[str, list[tuple[int, int, int, int]]] = {}
    last_draws: list[tuple[int, int, int, int]] = []
    for index, frame_path in enumerate(frame_paths):
        draws = raw_frame_draws[frame_path.name]
        if draws:
            last_draws = draws
        elif (
            first_boat_index is not None
            and last_boat_index is not None
            and first_boat_index <= index <= last_boat_index
        ):
            # ACTIVITY9 only records BOAT.BMP draws on update frames, but the
            # host holds the boat on intervening frames. Carry the last boat
            # draw forward so off-screen bow/stern repairs do not flicker.
            draws = last_draws
        per_frame_draws[frame_path.name] = draws
        for local_x, local_y, _width, _height in draws:
            min_local_x = min(min_local_x, local_x + boat_bbox[0])
            min_local_y = min(min_local_y, local_y + boat_bbox[1])
            max_local_x = max(max_local_x, local_x + boat_bbox[2] - 1)
            max_local_y = max(max_local_y, local_y + boat_bbox[3] - 1)

    new_offset_x = -min_local_x
    new_offset_y = -min_local_y
    new_width = max_local_x - min_local_x + 1
    new_height = max_local_y - min_local_y + 1
    if new_width <= 0 or new_height <= 0 or new_height > 480:
        raise SystemExit(f"invalid Activity9 boat patch canvas: {new_width}x{new_height}")

    paste_x = new_offset_x - old_offset_x
    paste_y = new_offset_y - old_offset_y

    for frame_path in frame_paths:
        with Image.open(frame_path) as raw:
            old_img = raw.convert("RGB")
        patched = Image.new("RGB", (new_width, new_height), KEY)
        patched.paste(old_img, (paste_x, paste_y))
        pix = patched.load()

        for local_x, local_y, _width, _height in per_frame_draws[frame_path.name]:
            dest_x = local_x + new_offset_x
            dest_y = local_y + new_offset_y
            for sx, sy, rgb in boat_pixels:
                pixel_local_x = local_x + sx
                in_clip_edge_band = (
                    pixel_local_x < LEGACY_SCENE_CLIP_MIN_X + CLIP_EDGE_OVERLAP_X
                    or pixel_local_x >= LEGACY_SCENE_CLIP_MAX_X - CLIP_EDGE_OVERLAP_X
                )
                if (
                    LEGACY_SCENE_CLIP_MIN_X <= pixel_local_x < LEGACY_SCENE_CLIP_MAX_X
                    and not in_clip_edge_band
                ):
                    continue
                dx = dest_x + sx
                dy = dest_y + sy
                if 0 <= dx < new_width and 0 <= dy < new_height and pix[dx, dy] == KEY:
                    pix[dx, dy] = rgb

        patched.save(frame_path)

        meta_path = meta_dir / frame_path.with_suffix(".json").name
        meta = json.loads(meta_path.read_text(encoding="utf-8"))
        meta["scene_offset_x"] = new_offset_x
        meta["scene_offset_y"] = new_offset_y
        meta["activity9_boat_source_patch"] = {
            "source": str(psb_path),
            "old_offset": [old_offset_x, old_offset_y],
            "new_offset": [new_offset_x, new_offset_y],
            "canvas": [new_width, new_height],
        }
        meta_path.write_text(json.dumps(meta, indent=2) + "\n", encoding="utf-8")


def main() -> None:
    if len(sys.argv) != 3:
        raise SystemExit(
            "usage: patch-activity9-boat-foreground.py <reference-capture> <merged-capture>"
        )
    repo_root = Path(__file__).resolve().parent.parent
    patch_activity9_boat(
        Path(sys.argv[1]),
        Path(sys.argv[2]),
        repo_root / "jc_resources" / "transcoded" / "BOAT.PSB",
    )


if __name__ == "__main__":
    main()
