#!/usr/bin/env python3
"""Repair BUILDING7's persistent campfire lane in merged foreground captures.

The host's current foreground ledger stops emitting live FIRE draws during the
middle of BUILDING7, while the visual scene still expects the campfire to stay
alive until the fade-out sequence returns. Full-host diff injection keeps the
fire present but copies stale, ghosted framebuffer pixels. This patch instead
loops clean campfire pixels from the last animated fire rows behind the existing
scene action for the missing interval.
"""

from __future__ import annotations

import argparse
from pathlib import Path

from PIL import Image


KEY = (255, 0, 255)
FIRE_RECT = (0, 35, 82, 108)
SOURCE_FRAMES = (368, 370, 372, 375, 377, 380, 383, 385, 388, 391)


def frame_path(capture_dir: Path, frame: int) -> Path:
    return capture_dir / "frames" / f"frame_{frame:05d}.bmp"


def patch_capture(capture_dir: Path, start_frame: int, end_frame: int) -> None:
    source_images: list[Image.Image] = []
    for frame in SOURCE_FRAMES:
        path = frame_path(capture_dir, frame)
        if not path.exists():
            raise SystemExit(f"missing BUILDING7 fire source frame: {path}")
        source_images.append(Image.open(path).convert("RGB"))

    left, top, right, bottom = FIRE_RECT
    try:
        for frame in range(start_frame, end_frame + 1):
            target_path = frame_path(capture_dir, frame)
            if not target_path.exists():
                raise SystemExit(f"missing BUILDING7 fire target frame: {target_path}")

            source = source_images[(frame - start_frame) % len(source_images)]
            with Image.open(target_path) as raw_target:
                target = raw_target.convert("RGB")

            source_pixels = source.load()
            target_pixels = target.load()
            for y in range(top, min(bottom, target.height, source.height)):
                for x in range(left, min(right, target.width, source.width)):
                    rgb = source_pixels[x, y]
                    if rgb == KEY:
                        continue
                    # Preserve Johnny/props already present in this frame; the
                    # campfire belongs behind the current scene action.
                    if target_pixels[x, y] == KEY:
                        target_pixels[x, y] = rgb

            target.save(target_path)
    finally:
        for image in source_images:
            image.close()


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Patch BUILDING7 merged foreground captures with animated campfire pixels."
    )
    parser.add_argument("capture_dir", type=Path)
    parser.add_argument("--start-frame", type=int, default=392)
    parser.add_argument("--end-frame", type=int, default=528)
    args = parser.parse_args()

    patch_capture(args.capture_dir, args.start_frame, args.end_frame)


if __name__ == "__main__":
    main()
