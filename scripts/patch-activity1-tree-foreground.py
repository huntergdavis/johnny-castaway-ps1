#!/usr/bin/env python3
"""Remove ACTIVITY1 palm/tree occlusion contamination from fg-only captures."""

from __future__ import annotations

import re
import sys
from pathlib import Path

from PIL import Image


MAGENTA = (255, 0, 255)
TREE_STALE_RECT = (220, 145, 120, 95)

# Signed-off two-beat capture layout. These windows cover both climbs where
# foreground-only replay exposes Johnny's hat/head before the full composite
# shows him above the tree.
CAPPED_PATCH_WINDOWS = ((79, 117), (284, 321), (303, 314))
CAPPED_CLEAR_WINDOWS = ((288, 300),)


def frame_number(path: Path) -> int:
    match = re.search(r"frame_(\d+)", path.stem)
    if not match:
        raise ValueError(f"cannot parse frame number from {path}")
    return int(match.group(1))


def in_windows(value: int, windows: tuple[tuple[int, int], ...]) -> bool:
    return any(start <= value < end for start, end in windows)


def should_patch_frame(frame_no: int, max_frame_no: int) -> tuple[bool, bool]:
    if max_frame_no <= 450:
        return (
            in_windows(frame_no, CAPPED_PATCH_WINDOWS),
            in_windows(frame_no, CAPPED_CLEAR_WINDOWS),
        )

    # Legacy until-exit captures can contain repeated copies of the same gag.
    # Keep this helper safe for older artifacts by applying the same local
    # windows every 200 frames.
    local = frame_no % 200
    return (
        in_windows(local, CAPPED_PATCH_WINDOWS),
        in_windows(local, CAPPED_CLEAR_WINDOWS),
    )


def patch_capture(full_host_dir: Path, foreground_dir: Path) -> int:
    full_frames = full_host_dir / "frames"
    fg_frames = foreground_dir / "frames"
    base_path = full_frames / "frame_00000.bmp"
    if not base_path.is_file():
        raise SystemExit(f"missing full-host base frame: {base_path}")
    if not fg_frames.is_dir():
        raise SystemExit(f"missing foreground frames dir: {fg_frames}")

    frame_paths = sorted(fg_frames.glob("frame_*.bmp"))
    if not frame_paths:
        raise SystemExit(f"no foreground frames found in {fg_frames}")
    max_frame_no = max(frame_number(path) for path in frame_paths)

    with Image.open(base_path) as img:
        base = img.convert("RGB")

    x0, y0, width, height = TREE_STALE_RECT
    changed_frames = 0

    for fg_path in frame_paths:
        frame_no = frame_number(fg_path)
        should_patch, clear_all = should_patch_frame(frame_no, max_frame_no)
        if not should_patch and not clear_all:
            continue

        full_path = full_frames / fg_path.name
        if not full_path.is_file():
            raise SystemExit(f"missing matching full-host frame: {full_path}")

        with Image.open(full_path) as full_img, Image.open(fg_path) as fg_img:
            full = full_img.convert("RGB")
            fg = fg_img.convert("RGB")
            changed = False

            for y in range(y0, min(y0 + height, fg.height)):
                for x in range(x0, min(x0 + width, fg.width)):
                    pixel = fg.getpixel((x, y))
                    if pixel == MAGENTA:
                        continue
                    if clear_all or full.getpixel((x, y)) == base.getpixel((x, y)):
                        fg.putpixel((x, y), MAGENTA)
                        changed = True

            if changed:
                fg.save(fg_path)
                changed_frames += 1

    return changed_frames


def main() -> int:
    if len(sys.argv) != 3:
        print(
            "usage: patch-activity1-tree-foreground.py FULL_HOST_CAPTURE_DIR "
            "FOREGROUND_CAPTURE_DIR",
            file=sys.stderr,
        )
        return 2

    changed = patch_capture(Path(sys.argv[1]), Path(sys.argv[2]))
    print(f"patched ACTIVITY1 tree foreground frames: {changed}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
