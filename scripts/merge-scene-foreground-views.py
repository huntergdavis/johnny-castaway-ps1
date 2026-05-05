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


def parse_rect(value: str) -> tuple[int, int, int, int]:
    parts = [int(part.strip()) for part in value.split(",")]
    if len(parts) != 4:
        raise argparse.ArgumentTypeError("rect must be x,y,w,h")
    x, y, w, h = parts
    if w <= 0 or h <= 0:
        raise argparse.ArgumentTypeError("rect width/height must be positive")
    return x, y, w, h


def parse_frame_range(value: str) -> tuple[int, int]:
    if ":" not in value:
        frame = int(value)
        return frame, frame
    start_text, end_text = value.split(":", 1)
    start = int(start_text)
    end = int(end_text)
    if end < start:
        raise argparse.ArgumentTypeError("frame range end must be >= start")
    return start, end


def frame_number(frame_path: Path) -> int:
    try:
        return int(frame_path.stem.split("_")[-1])
    except (ValueError, IndexError):
        return -1


def in_frame_ranges(frame: int, ranges: list[tuple[int, int]]) -> bool:
    if not ranges:
        return False
    return any(start <= frame <= end for start, end in ranges)


def is_probable_backdrop_pixel(rgb: tuple[int, int, int]) -> bool:
    r, g, b = rgb
    if rgb == KEY:
        return True
    if b > 150 and r < 80 and g < 130:
        return True
    if g > 180 and b > 180 and r < 80:
        return True
    if r > 210 and g > 190 and b < 80:
        return True
    return False


def is_sand_castle_pixel(rgb: tuple[int, int, int]) -> bool:
    r, g, b = rgb
    return r > 205 and g > 165 and b < 115


def filter_sand_components(
    pixels: dict[tuple[int, int], tuple[int, int, int]],
    min_sand_pixels: int,
    right_pad_after_last_sand: int = -1,
) -> dict[tuple[int, int], tuple[int, int, int]]:
    if min_sand_pixels <= 0:
        return pixels

    remaining = set(pixels.keys())
    kept: dict[tuple[int, int], tuple[int, int, int]] = {}
    while remaining:
        start = remaining.pop()
        stack = [start]
        component = [start]
        sand_count = 1 if is_sand_castle_pixel(pixels[start]) else 0
        max_sand_x = start[0] if sand_count else None
        while stack:
            x, y = stack.pop()
            for ny in (y - 1, y, y + 1):
                for nx in (x - 1, x, x + 1):
                    if nx == x and ny == y:
                        continue
                    point = (nx, ny)
                    if point not in remaining:
                        continue
                    remaining.remove(point)
                    stack.append(point)
                    component.append(point)
                    if is_sand_castle_pixel(pixels[point]):
                        sand_count += 1
                        max_sand_x = nx if max_sand_x is None else max(max_sand_x, nx)

        if sand_count >= min_sand_pixels:
            if right_pad_after_last_sand >= 0 and max_sand_x is not None:
                keep_max_x = max_sand_x + right_pad_after_last_sand
                component = [point for point in component if point[0] <= keep_max_x]
            for point in component:
                kept[point] = pixels[point]

    return kept


def trim_sand_component_tails(
    pixels: dict[tuple[int, int], tuple[int, int, int]],
    min_sand_pixels: int,
    column_min_sand_pixels: int,
    min_y: int,
    right_pad_after_dense_sand: int,
) -> dict[tuple[int, int], tuple[int, int, int]]:
    if min_sand_pixels <= 0 or right_pad_after_dense_sand < 0:
        return pixels

    remaining = set(pixels.keys())
    kept: dict[tuple[int, int], tuple[int, int, int]] = {}
    while remaining:
        start = remaining.pop()
        stack = [start]
        component = [start]
        sand_count = 0
        sand_columns: dict[int, int] = {}
        if is_sand_castle_pixel(pixels[start]):
            sand_count = 1
            sand_columns[start[0]] = 1

        while stack:
            x, y = stack.pop()
            for ny in (y - 1, y, y + 1):
                for nx in (x - 1, x, x + 1):
                    if nx == x and ny == y:
                        continue
                    point = (nx, ny)
                    if point not in remaining:
                        continue
                    remaining.remove(point)
                    stack.append(point)
                    component.append(point)
                    if is_sand_castle_pixel(pixels[point]):
                        sand_count += 1
                        sand_columns[nx] = sand_columns.get(nx, 0) + 1

        keep_max_x = None
        if sand_count >= min_sand_pixels and sand_columns:
            dense_columns = [
                x for x, count in sand_columns.items() if count >= column_min_sand_pixels
            ]
            max_sand_x = max(dense_columns) if dense_columns else max(sand_columns)
            keep_max_x = max_sand_x + right_pad_after_dense_sand

        for point in component:
            if keep_max_x is not None and point[0] > keep_max_x and point[1] >= min_y:
                continue
            kept[point] = pixels[point]

    return kept


def drop_output_rect_pixels(
    pixels: dict[tuple[int, int], tuple[int, int, int]],
    rects: list[tuple[int, int, int, int]],
) -> dict[tuple[int, int], tuple[int, int, int]]:
    if not rects:
        return pixels

    def inside_any_rect(point: tuple[int, int]) -> bool:
        x, y = point
        for rect_x, rect_y, rect_w, rect_h in rects:
            if rect_x <= x < rect_x + rect_w and rect_y <= y < rect_y + rect_h:
                return True
        return False

    return {point: rgb for point, rgb in pixels.items() if not inside_any_rect(point)}


def full_host_diff_pixels(
    reference_capture_dir: Path,
    frame_name: str,
    base_full: Image.Image | None,
    inject_rects: list[tuple[int, int, int, int]],
    keep_sand_colors: bool = False,
    sand_only: bool = False,
    sand_component_min_pixels: int = 0,
    sand_component_right_pad: int = -1,
) -> dict[tuple[int, int], tuple[int, int, int]]:
    if base_full is None or not inject_rects:
        return {}

    full_path = reference_capture_dir / "frames" / frame_name
    if not full_path.exists():
        return {}

    offset_x, offset_y = load_offset(reference_capture_dir, frame_name)
    with Image.open(full_path) as raw:
        full = raw.convert("RGB")

    full_pixels = full.load()
    base_pixels = base_full.load()
    injected: dict[tuple[int, int], tuple[int, int, int]] = {}

    for rect_x, rect_y, rect_w, rect_h in inject_rects:
        left = max(0, rect_x)
        top = max(0, rect_y)
        right = min(full.width, rect_x + rect_w)
        bottom = min(full.height, rect_y + rect_h)
        for y in range(top, bottom):
            for x in range(left, right):
                rgb = full_pixels[x, y]
                if rgb == base_pixels[x, y]:
                    continue
                if sand_only and not is_sand_castle_pixel(rgb):
                    continue
                if is_probable_backdrop_pixel(rgb):
                    r, g, b = rgb
                    if not (keep_sand_colors and r > 210 and g > 190 and b < 80):
                        continue
                injected[(x - offset_x, y - offset_y)] = rgb

    if sand_component_min_pixels > 0:
        injected = filter_sand_components(
            injected,
            sand_component_min_pixels,
            right_pad_after_last_sand=sand_component_right_pad,
        )

    return injected


def write_merged_foreground(
    reference_capture_dir: Path,
    source_fg_dirs: list[Path],
    output_dir: Path,
    inject_full_host_diff_rects: list[tuple[int, int, int, int]] | None = None,
    inject_full_host_diff_frame_ranges: list[tuple[int, int]] | None = None,
    inject_full_host_diff_keep_sand: bool = False,
    inject_full_host_diff_sand_only: bool = False,
    inject_full_host_diff_sand_component_min_pixels: int = 0,
    inject_full_host_diff_sand_component_right_pad: int = -1,
    trim_sand_tail_frame_ranges: list[tuple[int, int]] | None = None,
    trim_sand_tail_min_y: int = 0,
    trim_sand_tail_component_min_pixels: int = 0,
    trim_sand_tail_column_min_sand: int = 1,
    trim_sand_tail_right_pad: int = -1,
    drop_output_rects: list[tuple[int, int, int, int]] | None = None,
    drop_output_frame_ranges: list[tuple[int, int]] | None = None,
) -> None:
    out_frames = output_dir / "frames"
    out_meta = output_dir / "frame-meta"
    out_frames.mkdir(parents=True, exist_ok=True)
    out_meta.mkdir(parents=True, exist_ok=True)

    inject_full_host_diff_rects = inject_full_host_diff_rects or []
    inject_full_host_diff_frame_ranges = inject_full_host_diff_frame_ranges or []
    trim_sand_tail_frame_ranges = trim_sand_tail_frame_ranges or []
    drop_output_rects = drop_output_rects or []
    drop_output_frame_ranges = drop_output_frame_ranges or []
    base_full_path = reference_capture_dir / "frames" / "frame_00000.bmp"
    base_full = Image.open(base_full_path).convert("RGB") if base_full_path.exists() else None

    frames = []
    global_min_x = None
    global_min_y = None
    global_max_x = None
    global_max_y = None

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

        if in_frame_ranges(frame_number(ref_frame), inject_full_host_diff_frame_ranges):
            local_pixels.update(
                full_host_diff_pixels(
                    reference_capture_dir,
                    ref_frame.name,
                    base_full,
                    inject_full_host_diff_rects,
                    keep_sand_colors=inject_full_host_diff_keep_sand,
                    sand_only=inject_full_host_diff_sand_only,
                    sand_component_min_pixels=(
                        inject_full_host_diff_sand_component_min_pixels
                    ),
                    sand_component_right_pad=(
                        inject_full_host_diff_sand_component_right_pad
                    ),
                )
            )

        if local_pixels:
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
        frame_no = frame_number(ref_frame)
        if local_pixels:
            canvas_pixels: dict[tuple[int, int], tuple[int, int, int]] = {}
            pixels = merged.load()
            for (local_x, local_y), rgb in local_pixels.items():
                sx = local_x + synth_offset_x
                sy = local_y + synth_offset_y
                if 0 <= sx < canvas_w and 0 <= sy < canvas_h:
                    canvas_pixels[(sx, sy)] = rgb

            if in_frame_ranges(frame_no, trim_sand_tail_frame_ranges):
                canvas_pixels = trim_sand_component_tails(
                    canvas_pixels,
                    trim_sand_tail_component_min_pixels,
                    trim_sand_tail_column_min_sand,
                    trim_sand_tail_min_y,
                    trim_sand_tail_right_pad,
                )

            if in_frame_ranges(frame_no, drop_output_frame_ranges):
                canvas_pixels = drop_output_rect_pixels(canvas_pixels, drop_output_rects)

            for (sx, sy), rgb in canvas_pixels.items():
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

    if base_full is not None:
        base_full.close()


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Merge multiple foreground-only host captures into one scene-local canvas."
    )
    parser.add_argument("--reference-capture", required=True)
    parser.add_argument("--source-fg-dir", action="append", required=True)
    parser.add_argument("--output", required=True)
    parser.add_argument(
        "--inject-full-host-diff-rect",
        action="append",
        type=parse_rect,
        default=[],
        help="Inject non-backdrop full-host pixels that differ from frame 0 inside x,y,w,h screen rects.",
    )
    parser.add_argument(
        "--inject-full-host-diff-frames",
        action="append",
        type=parse_frame_range,
        default=[],
        help="Source frame or inclusive start:end range where full-host diff injection applies.",
    )
    parser.add_argument(
        "--inject-full-host-diff-keep-sand",
        action="store_true",
        help="Allow yellow sand-colored diff pixels through full-host injection.",
    )
    parser.add_argument(
        "--inject-full-host-diff-sand-only",
        action="store_true",
        help="Only inject sand/castle-colored full-host diff pixels.",
    )
    parser.add_argument(
        "--inject-full-host-diff-sand-component-min-pixels",
        type=int,
        default=0,
        help=(
            "Keep only full-host diff components containing at least this many "
            "sand/castle-colored pixels. Used when a mostly-static sand object "
            "needs its outlines, but unrelated stale host components must stay out."
        ),
    )
    parser.add_argument(
        "--inject-full-host-diff-sand-component-right-pad",
        type=int,
        default=-1,
        help=(
            "When using sand-component filtering, discard component pixels to "
            "the right of the last sand/castle-colored column plus this pad."
        ),
    )
    parser.add_argument(
        "--trim-sand-tail-frames",
        action="append",
        type=parse_frame_range,
        default=[],
        help="Source frame or inclusive start:end range where sand-component tail trimming applies.",
    )
    parser.add_argument(
        "--trim-sand-tail-min-y",
        type=int,
        default=0,
        help="Only trim sand-component tail pixels at or below this merged output y coordinate.",
    )
    parser.add_argument(
        "--trim-sand-tail-component-min-pixels",
        type=int,
        default=0,
        help="Only trim components containing at least this many sand/castle-colored pixels.",
    )
    parser.add_argument(
        "--trim-sand-tail-column-min-sand",
        type=int,
        default=1,
        help="Use the last sand/castle column with at least this many pixels as the trim anchor.",
    )
    parser.add_argument(
        "--trim-sand-tail-right-pad",
        type=int,
        default=-1,
        help="Trim lower sand-component pixels to the right of the dense sand anchor plus this pad.",
    )
    parser.add_argument(
        "--drop-output-rect",
        action="append",
        type=parse_rect,
        default=[],
        help="Drop merged output pixels inside x,y,w,h after stitching and cleanup.",
    )
    parser.add_argument(
        "--drop-output-frames",
        action="append",
        type=parse_frame_range,
        default=[],
        help="Source frame or inclusive start:end range where output rect drops apply.",
    )
    args = parser.parse_args()

    write_merged_foreground(
        Path(args.reference_capture),
        [Path(value) for value in args.source_fg_dir],
        Path(args.output),
        args.inject_full_host_diff_rect,
        args.inject_full_host_diff_frames,
        args.inject_full_host_diff_keep_sand,
        args.inject_full_host_diff_sand_only,
        args.inject_full_host_diff_sand_component_min_pixels,
        args.inject_full_host_diff_sand_component_right_pad,
        args.trim_sand_tail_frames,
        args.trim_sand_tail_min_y,
        args.trim_sand_tail_component_min_pixels,
        args.trim_sand_tail_column_min_sand,
        args.trim_sand_tail_right_pad,
        args.drop_output_rect,
        args.drop_output_frames,
    )


if __name__ == "__main__":
    main()
