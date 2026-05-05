#!/usr/bin/env python3
"""Append a keyed foreground cleanup frame to a host capture directory.

FGP2 empty frames are normally treated as hold frames at runtime. When the pack
is converted to FGP3, appending a keyed frame gives the converter an explicit
transition from the previous foreground state to empty, producing cleanup spans
for any stale captured pixels.
"""

from __future__ import annotations

import argparse
import json
import shutil
from pathlib import Path

from PIL import Image


KEY = (255, 0, 255)


def frame_number(path: Path) -> int:
    try:
        return int(path.stem.split("_")[-1])
    except (IndexError, ValueError):
        raise SystemExit(f"frame name is not frame_NNNNN: {path}") from None


def is_keyed_frame(path: Path) -> bool:
    with Image.open(path) as raw:
        img = raw.convert("RGB")
    for rgb in img.getdata():
        if rgb != KEY:
            return False
    return True


def append_cleanup_frame(capture_dir: Path) -> None:
    frames_dir = capture_dir / "frames"
    meta_dir = capture_dir / "frame-meta"
    frame_paths = sorted(frames_dir.glob("frame_*.bmp"))
    if not frame_paths:
        raise SystemExit(f"no BMP frames found in {frames_dir}")
    if not meta_dir.is_dir():
        raise SystemExit(f"frame metadata directory not found: {meta_dir}")

    last_frame = frame_paths[-1]
    if is_keyed_frame(last_frame):
        print(f"{capture_dir}: terminal cleanup frame already present")
        return

    next_number = frame_number(last_frame) + 1
    next_name = f"frame_{next_number:05d}.bmp"
    next_frame = frames_dir / next_name

    with Image.open(last_frame) as raw:
        clean = Image.new("RGB", raw.size, KEY)
    clean.save(next_frame)

    last_meta = meta_dir / last_frame.with_suffix(".json").name
    next_meta = meta_dir / Path(next_name).with_suffix(".json").name
    if last_meta.exists():
        payload = json.loads(last_meta.read_text(encoding="utf-8"))
    else:
        payload = {}
    payload["image_path"] = f"frames/{next_name}"
    payload["update_delay_ticks"] = 1
    next_meta.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")

    review = capture_dir / "review.html"
    if review.exists():
        stale_review = capture_dir / "review.before-cleanup-frame.html"
        if not stale_review.exists():
            shutil.copy2(review, stale_review)

    print(f"{capture_dir}: appended {next_name}")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("capture_dir", type=Path)
    args = parser.parse_args()
    append_cleanup_frame(args.capture_dir)


if __name__ == "__main__":
    main()
