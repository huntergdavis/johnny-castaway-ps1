#!/usr/bin/env python3

import argparse
import json
import shutil
from pathlib import Path

from PIL import Image, ImageChops


KEY = (255, 0, 255)


def load_offset(capture_dir: Path, frame_name: str) -> tuple[int, int]:
    meta_path = capture_dir / "frame-meta" / Path(frame_name).with_suffix(".json").name
    if not meta_path.exists():
        raise SystemExit(f"missing stitch metadata: {meta_path}")
    payload = json.loads(meta_path.read_text(encoding="utf-8"))
    return (
        int(payload.get("scene_offset_x", 0) or 0),
        int(payload.get("scene_offset_y", 0) or 0),
    )


def frame_paths(capture_dir: Path) -> list[Path]:
    return sorted((capture_dir / "frames").glob("frame_*.bmp"))


def non_key_pixels(img: Image.Image):
    key_img = Image.new("RGB", img.size, KEY)
    bbox = ImageChops.difference(img, key_img).getbbox()
    if bbox is None:
        return

    pixels = img.load()
    left, top, right, bottom = bbox
    for y in range(top, bottom):
        for x in range(left, right):
            rgb = pixels[x, y]
            if rgb != KEY:
                yield x, y, rgb


def write_merged_foreground(
    reference_capture_dir: Path,
    source_fg_dirs: list[Path],
    output_dir: Path,
    hold_empty_frames: bool = False,
    hold_drop_threshold: float = 0.0,
    hold_drop_floor: int = 0,
    hold_johnny_in_bbox: tuple[int, int, int, int] | None = None,
    hold_johnny_frame_range: tuple[int, int] | None = None,
    hold_johnny_glitch_threshold: int = 2000,
) -> None:
    out_frames = output_dir / "frames"
    out_meta = output_dir / "frame-meta"
    out_frames.mkdir(parents=True, exist_ok=True)
    out_meta.mkdir(parents=True, exist_ok=True)

    frames = []
    global_min_x = None
    global_min_y = None
    global_max_x = None
    global_max_y = None

    last_visible_pixels: dict[tuple[int, int], tuple[int, int, int]] = {}
    last_johnny_bbox_pixels: dict[tuple[int, int], tuple[int, int, int]] = {}
    for ref_frame in frame_paths(reference_capture_dir):
        local_pixels: dict[tuple[int, int], tuple[int, int, int]] = {}
        ref_meta_path = reference_capture_dir / "frame-meta" / ref_frame.with_suffix(".json").name
        if not ref_meta_path.exists():
            raise SystemExit(f"missing reference metadata: {ref_meta_path}")
        ref_meta = json.loads(ref_meta_path.read_text(encoding="utf-8"))

        for source_dir in source_fg_dirs:
            source_frame = source_dir / "frames" / ref_frame.name
            if not source_frame.exists():
                continue
            offset_x, offset_y = load_offset(source_dir, ref_frame.name)
            with Image.open(source_frame) as raw:
                img = raw.convert("RGB")
            for x, y, rgb in non_key_pixels(img):
                local_key = (x - offset_x, y - offset_y)
                local_pixels.setdefault(local_key, rgb)

        # When --hold-empty-frames is set: a frame with no foreground in any
        # source reuses the previous visible frame's pixels. Used for scenes
        # like walkstuf1 where the host engine drops Johnny for 1-3 frame
        # bursts mid-animation; without this hold, the temporal-residual
        # pack encodes those bursts as ERASE → DRAW → ERASE, which plays
        # back as a Johnny blink. Holding instead emits no delta for the
        # gap frame, so the runtime keeps the previous Johnny visible.
        if not local_pixels and hold_empty_frames and last_visible_pixels:
            local_pixels = dict(last_visible_pixels)

        # Stronger glitch-detection hold: if the current frame's pixel
        # count drops sharply (below `hold_drop_threshold` × previous)
        # AND the previous frame was above `hold_drop_floor` (i.e. was
        # showing a healthy Johnny — not a steady-state small pose),
        # treat the current frame as a glitch and hold the previous.
        # This catches walkstuf1's "Johnny drops to a partial fragment
        # for 1-2 frames" pattern that hold-empty-frames misses
        # because the fragment isn't fully empty.
        if (
            hold_drop_threshold > 0.0
            and last_visible_pixels
            and len(last_visible_pixels) > hold_drop_floor
            and len(local_pixels) < int(len(last_visible_pixels) * hold_drop_threshold)
        ):
            local_pixels = dict(last_visible_pixels)

        # Frame-range-gated Johnny-bbox hold: only inside the active
        # frame range, replace the bbox contents with the last "real"
        # Johnny pose IF the current frame's bbox is mostly empty
        # (Johnny dropped from foreground-only diff because he stopped
        # moving). Outside the bbox, the rest of the scene (boat,
        # mermaid, etc.) animates normally.
        #
        # We track `last_johnny_bbox_pixels` on EVERY frame (whether
        # in-range or not) so the first held frame at the range start
        # has something to fall back to.
        if hold_johnny_in_bbox:
            jx0, jy0, jx1, jy1 = hold_johnny_in_bbox
            current_in_bbox = sum(
                1 for (x, y) in local_pixels
                if jx0 <= x < jx1 and jy0 <= y < jy1
            )

            in_range = False
            if hold_johnny_frame_range:
                frame_idx = int(ref_frame.stem.split("_")[-1])
                range_start, range_end = hold_johnny_frame_range
                in_range = range_start <= frame_idx <= range_end

            if (
                in_range
                and current_in_bbox < hold_johnny_glitch_threshold
                and last_johnny_bbox_pixels
            ):
                # Glitch frame inside the hold range — drop the
                # fragmented bbox pixels and replace with the last
                # full Johnny pose so we don't get a stack of
                # partial-Johnny ghosts.
                local_pixels = {
                    k: v for k, v in local_pixels.items()
                    if not (jx0 <= k[0] < jx1 and jy0 <= k[1] < jy1)
                }
                local_pixels.update(last_johnny_bbox_pixels)
            elif current_in_bbox >= hold_johnny_glitch_threshold:
                # Healthy Johnny pose — remember it for later glitch
                # replacement (whether in-range or not).
                last_johnny_bbox_pixels = {
                    k: v for k, v in local_pixels.items()
                    if jx0 <= k[0] < jx1 and jy0 <= k[1] < jy1
                }

        if local_pixels:
            last_visible_pixels = dict(local_pixels)
            frame_min_x = min(x for x, _ in local_pixels)
            frame_max_x = max(x for x, _ in local_pixels)
            frame_min_y = min(y for _, y in local_pixels)
            frame_max_y = max(y for _, y in local_pixels)
            global_min_x = frame_min_x if global_min_x is None else min(global_min_x, frame_min_x)
            global_min_y = frame_min_y if global_min_y is None else min(global_min_y, frame_min_y)
            global_max_x = frame_max_x if global_max_x is None else max(global_max_x, frame_max_x)
            global_max_y = frame_max_y if global_max_y is None else max(global_max_y, frame_max_y)

        frames.append((ref_frame, ref_meta, local_pixels))

    if global_min_x is None:
        global_min_x = 0
        global_min_y = 0
        global_max_x = 0
        global_max_y = 0

    canvas_w = global_max_x - global_min_x + 1
    canvas_h = global_max_y - global_min_y + 1
    if canvas_w <= 0 or canvas_h <= 0:
        raise SystemExit("merged foreground produced an invalid canvas")
    if canvas_h > 480:
        raise SystemExit(
            f"merged foreground canvas is too tall for runtime playback: {canvas_w}x{canvas_h}"
        )

    synth_offset_x = -global_min_x
    synth_offset_y = -global_min_y

    for ref_frame, ref_meta, local_pixels in frames:
        merged = Image.new("RGB", (canvas_w, canvas_h), KEY)
        if local_pixels:
            pixels = merged.load()
            for (local_x, local_y), rgb in local_pixels.items():
                sx = local_x + synth_offset_x
                sy = local_y + synth_offset_y
                if 0 <= sx < canvas_w and 0 <= sy < canvas_h:
                    pixels[sx, sy] = rgb

        out_path = out_frames / ref_frame.name
        merged.save(out_path)

        ref_meta["image_path"] = str(out_path)
        ref_meta["scene_offset_x"] = synth_offset_x
        ref_meta["scene_offset_y"] = synth_offset_y
        (out_meta / ref_frame.with_suffix(".json").name).write_text(
            json.dumps(ref_meta, indent=2) + "\n",
            encoding="utf-8",
        )

    sound_events = reference_capture_dir / "sound-events.jsonl"
    if sound_events.exists():
        shutil.copy2(sound_events, output_dir / "sound-events.jsonl")


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Merge multiple foreground-only host captures into one scene-local canvas."
    )
    parser.add_argument("--reference-capture", required=True)
    parser.add_argument("--source-fg-dir", action="append", required=True)
    parser.add_argument("--output", required=True)
    parser.add_argument(
        "--hold-empty-frames",
        action="store_true",
        help="If a frame has no foreground in any source, hold the "
             "previous visible frame's pixels. Use only for scenes whose "
             "source content has Johnny-disappear bursts that the "
             "temporal-residual pack would otherwise play as a blink "
             "(currently: walkstuf1).",
    )
    parser.add_argument(
        "--hold-drop-threshold",
        type=float,
        default=0.0,
        help="If a frame's foreground pixel count drops below this "
             "fraction of the previous frame's count, hold the previous "
             "frame. 0 disables (default). Recommended: 0.5 for walkstuf1.",
    )
    parser.add_argument(
        "--hold-drop-floor",
        type=int,
        default=0,
        help="Only apply --hold-drop-threshold when the previous frame "
             "had MORE than this many foreground pixels (so we don't "
             "freeze a legitimate steady-state small-pose sequence). "
             "Recommended: 5000 for walkstuf1.",
    )
    parser.add_argument(
        "--hold-johnny-in-bbox",
        default="",
        help="Scene-local bbox to hold Johnny inside: 'left,top,right,bottom'. "
             "Use with --hold-johnny-frame-range to specify when the hold "
             "applies. Pixels OUTSIDE the bbox always animate normally "
             "(boat, mermaid, etc.).",
    )
    parser.add_argument(
        "--hold-johnny-frame-range",
        default="",
        help="Frame range (inclusive) for --hold-johnny-in-bbox: 'start,end'. "
             "Outside this range the hold is inactive. Required for the "
             "bbox hold to fire — without a range, nothing is held.",
    )
    parser.add_argument(
        "--hold-johnny-glitch-threshold",
        type=int,
        default=2000,
        help="Within the hold range, replace the bbox contents with the "
             "last full Johnny pose ONLY if the current frame has fewer "
             "than this many pixels in the bbox (i.e. Johnny is missing "
             "or fragmented). Default 2000.",
    )
    args = parser.parse_args()

    bbox = None
    if args.hold_johnny_in_bbox:
        parts = [int(p) for p in args.hold_johnny_in_bbox.split(",")]
        if len(parts) != 4:
            raise SystemExit("--hold-johnny-in-bbox needs 'left,top,right,bottom'")
        bbox = tuple(parts)  # type: ignore[assignment]

    frame_range = None
    if args.hold_johnny_frame_range:
        parts = [int(p) for p in args.hold_johnny_frame_range.split(",")]
        if len(parts) != 2:
            raise SystemExit("--hold-johnny-frame-range needs 'start,end'")
        frame_range = tuple(parts)  # type: ignore[assignment]

    write_merged_foreground(
        Path(args.reference_capture),
        [Path(value) for value in args.source_fg_dir],
        Path(args.output),
        hold_empty_frames=args.hold_empty_frames,
        hold_drop_threshold=args.hold_drop_threshold,
        hold_drop_floor=args.hold_drop_floor,
        hold_johnny_in_bbox=bbox,
        hold_johnny_frame_range=frame_range,
        hold_johnny_glitch_threshold=args.hold_johnny_glitch_threshold,
    )


if __name__ == "__main__":
    main()
