#!/usr/bin/env python3
"""Merge VISITOR3 foreground views and synthesize the ship crash state.

VISITOR3 is not a normal per-frame sprite scene. The red ship is revealed by a
moving foreground slice, while the full host surface accumulates that slice into
a ship body and then holds stale red too long. This helper keeps the clean
foreground-only source, injects the full-host ship only during the live crash
frames, and then lets later blank frames clear through FGP3 temporal cleanup.
"""

from __future__ import annotations

import argparse
import json
import shutil
from pathlib import Path

from PIL import Image, ImageChops


KEY = (255, 0, 255)


def frame_number(path: Path) -> int:
    return int(path.stem.split("_", 1)[1])


def load_offset(capture_dir: Path, frame_name: str) -> tuple[int, int]:
    meta_path = capture_dir / "frame-meta" / Path(frame_name).with_suffix(".json").name
    if not meta_path.exists():
        raise SystemExit(f"missing VISITOR3 stitch metadata: {meta_path}")
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


def is_ship_red(rgb: tuple[int, int, int]) -> bool:
    r, g, b = rgb
    return r >= 120 and g <= 100 and b <= 100 and (r - max(g, b)) >= 35


def largest_component(points: set[tuple[int, int]]) -> set[tuple[int, int]]:
    remaining = set(points)
    best: set[tuple[int, int]] = set()

    while remaining:
        seed = remaining.pop()
        component = {seed}
        stack = [seed]

        while stack:
            x, y = stack.pop()
            for neighbor in ((x - 1, y), (x + 1, y), (x, y - 1), (x, y + 1)):
                if neighbor not in remaining:
                    continue
                remaining.remove(neighbor)
                component.add(neighbor)
                stack.append(neighbor)

        if len(component) > len(best):
            best = component

    return best


def collect_local_pixels(frame_name: str, source_fg_dirs: list[Path]) -> dict[tuple[int, int], tuple[int, int, int]]:
    local_pixels: dict[tuple[int, int], tuple[int, int, int]] = {}
    for source_dir in source_fg_dirs:
        source_frame = source_dir / "frames" / frame_name
        if not source_frame.exists():
            continue
        offset_x, offset_y = load_offset(source_dir, frame_name)
        with Image.open(source_frame) as raw:
            img = raw.convert("RGB")
        for x, y, rgb in non_key_pixels(img):
            local_key = (x - offset_x, y - offset_y)
            local_pixels.setdefault(local_key, rgb)
    return local_pixels


def near_red(red_by_y: dict[int, list[int]], x: int, y: int, expand: int) -> bool:
    for ny in range(y - expand, y + expand + 1):
        xs = red_by_y.get(ny)
        if not xs:
            continue
        for rx in xs:
            if abs(rx - x) <= expand:
                return True
    return False


def host_ship_pixels(
    reference_capture_dir: Path,
    frame_name: str,
    base_full: Image.Image | None,
    expand: int,
) -> dict[tuple[int, int], tuple[int, int, int]]:
    if base_full is None:
        return {}

    full_path = reference_capture_dir / "frames" / frame_name
    if not full_path.exists():
        return {}

    offset_x, offset_y = load_offset(reference_capture_dir, frame_name)
    with Image.open(full_path) as raw:
        full = raw.convert("RGB")

    full_pixels = full.load()
    base_pixels = base_full.load()
    red_points: set[tuple[int, int]] = set()

    for y in range(full.height):
        for x in range(full.width):
            if is_ship_red(full_pixels[x, y]):
                red_points.add((x, y))

    if len(red_points) < 100:
        return {}

    ship_points = largest_component(red_points)
    if len(ship_points) < 100:
        return {}

    red_by_y: dict[int, list[int]] = {}
    for x, y in ship_points:
        red_by_y.setdefault(y, []).append(x)

    out: dict[tuple[int, int], tuple[int, int, int]] = {}
    min_y = min(red_by_y)
    max_y = max(red_by_y)
    min_x = min(min(xs) for xs in red_by_y.values())
    max_x = max(max(xs) for xs in red_by_y.values())

    for y in range(max(0, min_y - expand), min(full.height, max_y + expand + 1)):
        for x in range(max(0, min_x - expand), min(full.width, max_x + expand + 1)):
            rgb = full_pixels[x, y]
            if (x, y) not in ship_points:
                # The base-equality and near-red gates exist to keep the
                # dither fringe without grabbing backdrop. Core component
                # pixels bypass them: a hull pixel that coincidentally
                # equals the base (e.g. the island's exact-red tree pixels
                # behind the hull in an offset reference view) is still
                # ship, and dropping it shipped a 2px vertical seam at the
                # reference-blind-edge boundary.
                if rgb == base_pixels[x, y]:
                    continue
                if y > max_y or not near_red(red_by_y, x, y, expand):
                    continue
            out[(x - offset_x, y - offset_y)] = rgb

    return out


def host_rect_diff_pixels(
    reference_capture_dir: Path,
    frame_name: str,
    base_full: Image.Image | None,
    rect: tuple[int, int, int, int],
) -> dict[tuple[int, int], tuple[int, int, int]]:
    if base_full is None:
        return {}

    full_path = reference_capture_dir / "frames" / frame_name
    if not full_path.exists():
        return {}

    offset_x, offset_y = load_offset(reference_capture_dir, frame_name)
    with Image.open(full_path) as raw:
        full = raw.convert("RGB")

    full_pixels = full.load()
    base_pixels = base_full.load()
    x0, y0, width, height = rect
    out: dict[tuple[int, int], tuple[int, int, int]] = {}

    for y in range(max(0, y0), min(full.height, y0 + height)):
        for x in range(max(0, x0), min(full.width, x0 + width)):
            rgb = full_pixels[x, y]
            if rgb == base_pixels[x, y]:
                continue
            out[(x - offset_x, y - offset_y)] = rgb

    return out


def host_splash_pixels(
    reference_capture_dir: Path,
    frame_name: str,
    base_full: Image.Image | None,
    rect: tuple[int, int, int, int],
) -> dict[tuple[int, int], tuple[int, int, int]]:
    if base_full is None:
        return {}

    full_path = reference_capture_dir / "frames" / frame_name
    if not full_path.exists():
        return {}

    offset_x, offset_y = load_offset(reference_capture_dir, frame_name)
    with Image.open(full_path) as raw:
        full = raw.convert("RGB")

    full_pixels = full.load()
    base_pixels = base_full.load()
    x0, y0, width, height = rect
    candidates: set[tuple[int, int]] = set()

    for y in range(max(0, y0), min(full.height, y0 + height)):
        for x in range(max(0, x0), min(full.width, x0 + width)):
            rgb = full_pixels[x, y]
            if rgb == base_pixels[x, y]:
                continue
            r, g, b = rgb
            if r > 180 and g > 180 and b > 180:
                candidates.add((x, y))

    components: list[set[tuple[int, int]]] = []
    remaining = set(candidates)
    while remaining:
        seed = remaining.pop()
        component = {seed}
        stack = [seed]
        while stack:
            x, y = stack.pop()
            for neighbor in ((x - 1, y), (x + 1, y), (x, y - 1), (x, y + 1)):
                if neighbor not in remaining:
                    continue
                remaining.remove(neighbor)
                component.add(neighbor)
                stack.append(neighbor)
        components.append(component)

    out: dict[tuple[int, int], tuple[int, int, int]] = {}
    for component in components:
        if len(component) < 20:
            continue
        min_x = min(x for x, _ in component)
        if min_x < x0 + 50:
            continue
        for x, y in component:
            out[(x - offset_x, y - offset_y)] = full_pixels[x, y]

    return out


def write_visitor3_foreground(
    reference_capture_dir: Path,
    source_fg_dirs: list[Path],
    output_dir: Path,
    ship_start: int,
    ship_end: int,
    ship_keep_until: int,
    ship_expand: int,
    splash_start: int,
    splash_end: int,
    splash_rect: tuple[int, int, int, int],
    extra_reference_capture_dirs: list[Path] | None = None,
) -> None:
    out_frames = output_dir / "frames"
    out_meta = output_dir / "frame-meta"
    if output_dir.exists():
        shutil.rmtree(output_dir)
    out_frames.mkdir(parents=True, exist_ok=True)
    out_meta.mkdir(parents=True, exist_ok=True)

    frames: list[tuple[Path, dict, dict[tuple[int, int], tuple[int, int, int]]]] = []
    base_full_path = reference_capture_dir / "frames" / "frame_00000.bmp"
    base_full = Image.open(base_full_path).convert("RGB") if base_full_path.exists() else None
    # Extra full-host references at other island positions widen the
    # accumulated-hull coverage: the primary reference (island-x -154) is
    # blind to scene-local x < 154, which shipped as a hard vertical hull
    # truncation on PS1. Each extra maps through its own frame-meta offsets,
    # so overlapping regions land at the same scene-local coordinates; the
    # primary reference and foreground-only views take precedence.
    extra_refs: list[tuple[Path, Image.Image | None]] = []
    for extra_dir in extra_reference_capture_dirs or []:
        extra_base_path = extra_dir / "frames" / "frame_00000.bmp"
        extra_base = (
            Image.open(extra_base_path).convert("RGB")
            if extra_base_path.exists() else None
        )
        extra_refs.append((extra_dir, extra_base))
    global_min_x: int | None = None
    global_min_y: int | None = None
    global_max_x: int | None = None
    global_max_y: int | None = None

    for ref_frame in frame_paths(reference_capture_dir):
        number = frame_number(ref_frame)
        ref_meta_path = reference_capture_dir / "frame-meta" / ref_frame.with_suffix(".json").name
        if not ref_meta_path.exists():
            raise SystemExit(f"missing VISITOR3 reference metadata: {ref_meta_path}")
        ref_meta = json.loads(ref_meta_path.read_text(encoding="utf-8"))
        local_pixels = collect_local_pixels(ref_frame.name, source_fg_dirs)

        if ship_start <= number <= ship_end or ship_start <= number <= ship_keep_until:
            output_pixels = {}
            for extra_dir, extra_base in extra_refs:
                output_pixels.update(
                    host_ship_pixels(
                        extra_dir,
                        ref_frame.name,
                        extra_base,
                        ship_expand,
                    )
                )
            output_pixels.update(
                host_ship_pixels(
                    reference_capture_dir,
                    ref_frame.name,
                    base_full,
                    ship_expand,
                )
            )
            output_pixels.update(local_pixels)
        else:
            output_pixels = dict(local_pixels)

        if splash_start <= number <= splash_end:
            output_pixels.update(
                host_splash_pixels(
                    reference_capture_dir,
                    ref_frame.name,
                    base_full,
                    splash_rect,
                )
            )

        if output_pixels:
            frame_min_x = min(x for x, _ in output_pixels)
            frame_max_x = max(x for x, _ in output_pixels)
            frame_min_y = min(y for _, y in output_pixels)
            frame_max_y = max(y for _, y in output_pixels)
            global_min_x = frame_min_x if global_min_x is None else min(global_min_x, frame_min_x)
            global_min_y = frame_min_y if global_min_y is None else min(global_min_y, frame_min_y)
            global_max_x = frame_max_x if global_max_x is None else max(global_max_x, frame_max_x)
            global_max_y = frame_max_y if global_max_y is None else max(global_max_y, frame_max_y)

        frames.append((ref_frame, ref_meta, output_pixels))

    if global_min_x is None:
        global_min_x = 0
        global_min_y = 0
        global_max_x = 0
        global_max_y = 0

    canvas_w = global_max_x - global_min_x + 1
    canvas_h = global_max_y - global_min_y + 1
    if canvas_w <= 0 or canvas_h <= 0:
        raise SystemExit("VISITOR3 foreground produced an invalid canvas")
    if canvas_h > 480:
        raise SystemExit(
            f"VISITOR3 foreground canvas is too tall for runtime playback: {canvas_w}x{canvas_h}"
        )

    synth_offset_x = -global_min_x
    synth_offset_y = -global_min_y

    for ref_frame, ref_meta, output_pixels in frames:
        merged = Image.new("RGB", (canvas_w, canvas_h), KEY)
        if output_pixels:
            pixels = merged.load()
            for (local_x, local_y), rgb in output_pixels.items():
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

    if base_full is not None:
        base_full.close()


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Merge VISITOR3 foreground-only views and accumulate the red ship crash."
    )
    parser.add_argument("--reference-capture", required=True, type=Path)
    parser.add_argument(
        "--extra-reference-capture",
        action="append",
        type=Path,
        default=[],
        help=(
            "Additional full-host capture at another island position; its "
            "accumulated ship pixels fill regions the primary reference "
            "cannot see (e.g. scene-local x < 154 at the -154 capture position)."
        ),
    )
    parser.add_argument("--source-fg-dir", action="append", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--ship-start", type=int, default=160)
    parser.add_argument("--ship-end", type=int, default=189)
    parser.add_argument("--ship-keep-until", type=int, default=189)
    parser.add_argument("--ship-expand", type=int, default=8)
    parser.add_argument("--splash-start", type=int, default=158)
    parser.add_argument("--splash-end", type=int, default=158)
    parser.add_argument("--splash-rect", default="400,345,90,45")
    args = parser.parse_args()
    try:
        splash_rect = tuple(int(part) for part in args.splash_rect.split(","))
    except ValueError as exc:
        raise SystemExit(f"invalid --splash-rect {args.splash_rect!r}") from exc
    if len(splash_rect) != 4:
        raise SystemExit(f"invalid --splash-rect {args.splash_rect!r}")

    write_visitor3_foreground(
        args.reference_capture,
        args.source_fg_dir,
        args.output,
        args.ship_start,
        args.ship_end,
        args.ship_keep_until,
        args.ship_expand,
        args.splash_start,
        args.splash_end,
        splash_rect,  # type: ignore[arg-type]
        extra_reference_capture_dirs=args.extra_reference_capture,
    )


if __name__ == "__main__":
    main()
