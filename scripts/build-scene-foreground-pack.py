#!/usr/bin/env python3

import argparse
import json
import struct
from pathlib import Path

from PIL import Image


def parse_rgb(value: str) -> tuple[int, int, int]:
    raw = value.strip().lower().replace("#", "")
    if len(raw) != 6:
        raise argparse.ArgumentTypeError("expected RRGGBB")
    return tuple(int(raw[i:i + 2], 16) for i in (0, 2, 4))


def rgb888_to_ps1(rgb: tuple[int, int, int]) -> int:
    r, g, b = rgb
    value = (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10)
    if value == 0:
        value = 0x8000
    return value


def image_to_ps1_values(img: Image.Image) -> tuple[list[int], int, int]:
    rgb = img.convert("RGB")
    pixels = rgb.load()
    width, height = rgb.size
    values: list[int] = []

    for y in range(height):
        for x in range(width):
            values.append(rgb888_to_ps1(pixels[x, y]))
    return values, width, height


def encode_base_diff_crop(base_values: list[int], img: Image.Image) -> tuple[dict | None, bytes]:
    cur_values, width, height = image_to_ps1_values(img)
    if len(base_values) != len(cur_values):
        raise SystemExit(
            f"base-diff size mismatch: base has {len(base_values)} pixels, "
            f"frame has {len(cur_values)} pixels"
        )

    min_x = width
    min_y = height
    max_x = -1
    max_y = -1

    for y in range(height):
        row_base = y * width
        for x in range(width):
            i = row_base + x
            if cur_values[i] != base_values[i]:
                if x < min_x:
                    min_x = x
                if y < min_y:
                    min_y = y
                if x > max_x:
                    max_x = x
                if y > max_y:
                    max_y = y

    if max_x < min_x or max_y < min_y:
        return None, b""

    bbox = {
        "x": min_x,
        "y": min_y,
        "width": max_x - min_x + 1,
        "height": max_y - min_y + 1,
    }
    out = bytearray(bbox["width"] * bbox["height"] * 2)
    out_i = 0
    for y in range(bbox["y"], bbox["y"] + bbox["height"]):
        row_base = y * width
        for x in range(bbox["x"], bbox["x"] + bbox["width"]):
            i = row_base + x
            value = cur_values[i] if cur_values[i] != base_values[i] else 0
            out[out_i:out_i + 2] = struct.pack("<H", value)
            out_i += 2
    return bbox, bytes(out)


def iter_u16_payload(payload: bytes):
    for offset in range(0, len(payload), 2):
        yield struct.unpack_from("<H", payload, offset)[0]


def collect_visible_colors(data_chunks: list[bytes]) -> list[int]:
    colors: set[int] = set()
    for chunk in data_chunks:
        for value in iter_u16_payload(chunk):
            if value != 0:
                colors.add(value)
    return sorted(colors)


def build_fg2_palette(data_chunks: list[bytes]) -> tuple[list[int], str]:
    colors = collect_visible_colors(data_chunks)
    if len(colors) > 15:
        if len(colors) > 255:
            raise SystemExit(
                f"FG2 indexed8 supports at most 255 visible colors per scene; found {len(colors)}"
            )
        palette = [0] + colors
        while len(palette) < 256:
            palette.append(0)
        return palette, "indexed8"

    palette = [0] + colors
    while len(palette) < 16:
        palette.append(0)
    return palette, "pal4"


def encode_pal4_span_payload(payload: bytes, width: int, height: int,
                             palette_index: dict[int, int]) -> bytes:
    if not payload or width <= 0 or height <= 0:
        return b""
    expected = width * height * 2
    if len(payload) != expected:
        raise SystemExit(
            f"payload size mismatch for FG2 row spans: got {len(payload)}, expected {expected}"
        )

    rows: list[tuple[int, list[tuple[int, bytes, int]]]] = []
    values = list(iter_u16_payload(payload))
    for y in range(height):
        row_base = y * width
        x = 0
        row_spans: list[tuple[int, bytes, int]] = []
        while x < width:
            while x < width and values[row_base + x] == 0:
                x += 1
            if x >= width:
                break

            start_x = x
            indices: list[int] = []
            while x < width and values[row_base + x] != 0:
                value = values[row_base + x]
                try:
                    indices.append(palette_index[value])
                except KeyError as exc:
                    raise SystemExit(f"FG2 palette missing color 0x{value:04x}") from exc
                x += 1

            packed = bytearray((len(indices) + 1) // 2)
            for i, index in enumerate(indices):
                if index <= 0 or index > 15:
                    raise SystemExit(f"FG2 palette index out of range: {index}")
                if (i & 1) == 0:
                    packed[i >> 1] = (index & 0x0F) << 4
                else:
                    packed[i >> 1] |= index & 0x0F
            row_spans.append((start_x, bytes(packed), len(indices)))

        if row_spans:
            rows.append((y, row_spans))

    out = bytearray()
    out += struct.pack("<H", len(rows))
    for y, spans in rows:
        out += struct.pack("<HH", y, len(spans))
        for x, packed, pixel_count in spans:
            out += struct.pack("<HH", x, pixel_count)
            out += packed
    return bytes(out)


def encode_indexed8_span_payload(payload: bytes, width: int, height: int,
                                 palette_index: dict[int, int]) -> bytes:
    if not payload or width <= 0 or height <= 0:
        return b""
    expected = width * height * 2
    if len(payload) != expected:
        raise SystemExit(
            f"payload size mismatch for FG2 row spans: got {len(payload)}, expected {expected}"
        )

    rows: list[tuple[int, list[tuple[int, bytes, int]]]] = []
    values = list(iter_u16_payload(payload))
    for y in range(height):
        row_base = y * width
        x = 0
        row_spans: list[tuple[int, bytes, int]] = []
        while x < width:
            while x < width and values[row_base + x] == 0:
                x += 1
            if x >= width:
                break

            start_x = x
            indices: list[int] = []
            while x < width and values[row_base + x] != 0:
                value = values[row_base + x]
                try:
                    index = palette_index[value]
                except KeyError as exc:
                    raise SystemExit(f"FG2 palette missing color 0x{value:04x}") from exc
                if index <= 0 or index > 255:
                    raise SystemExit(f"FG2 palette index out of range: {index}")
                indices.append(index)
                x += 1

            row_spans.append((start_x, bytes(indices), len(indices)))

        if row_spans:
            rows.append((y, row_spans))

    out = bytearray()
    out += struct.pack("<H", len(rows))
    for y, spans in rows:
        out += struct.pack("<HH", y, len(spans))
        for x, indexed, pixel_count in spans:
            out += struct.pack("<HH", x, pixel_count)
            out += indexed
    return bytes(out)


def load_frame_delays(frame_meta_dir: Path | None) -> dict[str, int]:
    if frame_meta_dir is None:
        return {}
    if not frame_meta_dir.is_dir():
        raise SystemExit(f"frame metadata directory not found: {frame_meta_dir}")

    delays: dict[str, int] = {}
    for meta_path in sorted(frame_meta_dir.glob("frame_*.json")):
        payload = json.loads(meta_path.read_text(encoding="utf-8"))
        image_path = payload.get("image_path", "")
        frame_name = Path(image_path).name if image_path else ""
        if not frame_name:
            frame_name = meta_path.with_suffix(".bmp").name
        delay_ticks = int(payload.get("update_delay_ticks", 1) or 1)
        if delay_ticks <= 0:
            delay_ticks = 1
        delays[frame_name] = delay_ticks
    return delays


def load_sound_events(path: Path | None) -> list[tuple[int, int]]:
    """Parse a JSONL file of {"frame": N, "sample": M} entries.

    Returns a list of (source_frame, sample_id) pairs sorted by source_frame.
    A missing file is treated as "no events".
    """
    if path is None:
        return []
    if not path.is_file():
        return []

    events: list[tuple[int, int]] = []
    for line in path.read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if not line:
            continue
        try:
            payload = json.loads(line)
        except json.JSONDecodeError:
            continue
        frame = payload.get("frame")
        sample = payload.get("sample")
        if frame is None or sample is None:
            continue
        frame = int(frame)
        sample = int(sample)
        if frame < 0 or sample < 0 or sample > 0xFFFF or frame > 0xFFFF:
            continue
        events.append((frame, sample))
    events.sort(key=lambda e: (e[0], e[1]))
    return events


def load_frame_offsets(frame_meta_dir: Path | None) -> dict[str, tuple[int, int]]:
    if frame_meta_dir is None:
        return {}
    if not frame_meta_dir.is_dir():
        raise SystemExit(f"frame metadata directory not found: {frame_meta_dir}")

    offsets: dict[str, tuple[int, int]] = {}
    for meta_path in sorted(frame_meta_dir.glob("frame_*.json")):
        payload = json.loads(meta_path.read_text(encoding="utf-8"))
        image_path = payload.get("image_path", "")
        frame_name = Path(image_path).name if image_path else ""
        if not frame_name:
            frame_name = meta_path.with_suffix(".bmp").name
        scene_offset_x = int(payload.get("scene_offset_x", 0) or 0)
        scene_offset_y = int(payload.get("scene_offset_y", 0) or 0)
        offsets[frame_name] = (scene_offset_x, scene_offset_y)
    return offsets


def main():
    parser = argparse.ArgumentParser(
        description="Build a PS1 FG2 full-render base-diff playback pack."
    )
    parser.add_argument("--frames-dir", required=True)
    parser.add_argument("--output-pack", required=True)
    parser.add_argument("--output-json")
    parser.add_argument("--scene-label", default="")
    parser.add_argument("--key-rgb", default="ff00ff", type=parse_rgb)
    parser.add_argument("--frame-step", type=int, default=1)
    parser.add_argument(
        "--pack-format",
        choices=("fg2",),
        default="fg2",
        help="On-disc pack format. FG2 is the only active PS1 scene-pack format.",
    )
    parser.add_argument("--frame-meta-dir")
    parser.add_argument(
        "--timeline-speed",
        type=float,
        default=1.0,
        help="Scale host-tick deadlines by this factor. 2.0 plays the captured timeline twice as fast.",
    )
    parser.add_argument(
        "--sound-events",
        help="JSONL of captured sound events (one {\"frame\": N, \"sample\": M} per line).",
    )
    parser.add_argument(
        "--base-diff",
        action="store_true",
        help="Pack all pixels that differ from --scene-base-frame after PS1 15-bit color quantization.",
    )
    parser.add_argument(
        "--scene-base-frame",
        type=int,
        default=0,
        help="Index in frames-dir to treat as the pristine scene base (default 0).",
    )
    args = parser.parse_args()

    frames_dir = Path(args.frames_dir)
    frame_paths = sorted(
        path for path in frames_dir.iterdir()
        if path.is_file() and path.suffix.lower() in {".bmp", ".png"}
    )
    if not frame_paths:
        raise SystemExit(f"no frame images found in {frames_dir}")
    if args.frame_step <= 0:
        raise SystemExit("--frame-step must be > 0")
    if args.timeline_speed <= 0:
        raise SystemExit("--timeline-speed must be > 0")
    if not args.base_diff:
        raise SystemExit("FG2 scene packs must use --base-diff")

    selected_indices = list(range(0, len(frame_paths), args.frame_step))
    if selected_indices[-1] != (len(frame_paths) - 1):
        selected_indices.append(len(frame_paths) - 1)

    rows = []
    data_chunks: list[bytes] = []
    frame_delays = load_frame_delays(Path(args.frame_meta_dir) if args.frame_meta_dir else None)
    frame_offsets = load_frame_offsets(Path(args.frame_meta_dir) if args.frame_meta_dir else None)
    union_min_x = None
    union_min_y = None
    union_max_x = None
    union_max_y = None

    _raw_sound_events = load_sound_events(
        Path(args.sound_events) if args.sound_events else None
    )
    _first_frame_stem = frame_paths[0].stem
    try:
        _first_abs_frame = int(_first_frame_stem.split("_")[-1])
    except ValueError:
        _first_abs_frame = 0

    scene_base_ps1_values: list[int] | None = None
    base_index = args.scene_base_frame
    if base_index < 0 or base_index >= len(frame_paths):
        raise SystemExit(
            f"--scene-base-frame {base_index} out of range (0..{len(frame_paths) - 1})"
        )
    with Image.open(frame_paths[base_index]) as raw:
        scene_base_ps1_values, _, _ = image_to_ps1_values(raw)

    for source_index in selected_indices:
        frame_path = frame_paths[source_index]
        with Image.open(frame_path) as raw:
            rgb = raw.convert("RGB")

        if scene_base_ps1_values is None:
            raise SystemExit("--base-diff missing scene base data")
        bbox, payload = encode_base_diff_crop(scene_base_ps1_values, rgb)

        if bbox is not None:
            x2 = bbox["x"] + bbox["width"] - 1
            y2 = bbox["y"] + bbox["height"] - 1
            if union_min_x is None or bbox["x"] < union_min_x:
                union_min_x = bbox["x"]
            if union_min_y is None or bbox["y"] < union_min_y:
                union_min_y = bbox["y"]
            if union_max_x is None or x2 > union_max_x:
                union_max_x = x2
            if union_max_y is None or y2 > union_max_y:
                union_max_y = y2

        scene_offset_x, scene_offset_y = frame_offsets.get(frame_path.name, (0, 0))
        local_x = 0 if bbox is None else bbox["x"] - scene_offset_x
        local_y = 0 if bbox is None else bbox["y"] - scene_offset_y

        row = {
            "source_frame": source_index,
            "frame": frame_path.name,
            "x": local_x,
            "y": local_y,
            "width": 0 if bbox is None else bbox["width"],
            "height": 0 if bbox is None else bbox["height"],
            "scene_offset_x": scene_offset_x,
            "scene_offset_y": scene_offset_y,
            "hold_ticks": frame_delays.get(frame_path.name, 1),
            "hold_frames": 1,
            "hold_vblanks": 1,
            "data_offset": 0,
            "data_size": len(payload),
        }

        if rows:
            prev = rows[-1]
            prev_payload = data_chunks[-1]
            same_frame = (
                prev["x"] == row["x"] and
                prev["y"] == row["y"] and
                prev["width"] == row["width"] and
                prev["height"] == row["height"] and
                prev_payload == payload
            )
            if same_frame:
                prev["hold_ticks"] += row["hold_ticks"]
                prev["hold_frames"] += 1
                continue

        rows.append(row)
        data_chunks.append(payload)

    header_flags = 0
    if args.frame_meta_dir:
        header_flags |= 0x0004
        header_flags |= 0x0008
    if args.base_diff:
        header_flags |= 0x0010

    cumulative_ticks = 0
    for row in rows:
        cumulative_ticks += row["hold_ticks"]
        if args.timeline_speed == 1.0:
            row["deadline_ticks"] = cumulative_ticks
        else:
            row["source_deadline_ticks"] = cumulative_ticks
            row["deadline_ticks"] = max(1, int((cumulative_ticks / args.timeline_speed) + 0.5))
        row["hold_vblanks"] = row["deadline_ticks"] if (header_flags & 0x0004) else row["hold_frames"]

    sound_events = list(_raw_sound_events)
    first_abs_frame = _first_abs_frame
    if first_abs_frame:
        sound_events = [(ev[0] - first_abs_frame, ev[1]) for ev in sound_events if ev[0] >= first_abs_frame]
    max_source_frame = selected_indices[-1]
    sound_events = [ev for ev in sound_events if ev[0] <= max_source_frame]
    if len(sound_events) > 0xFFFF:
        sound_events = sound_events[:0xFFFF]

    palette_values: list[int] | None = None
    fg2_encoding: str | None = None
    palette_values, fg2_encoding = build_fg2_palette(data_chunks)
    palette_index = {value: index for index, value in enumerate(palette_values) if index > 0}
    encoded_chunks: list[bytes] = []
    for row, chunk in zip(rows, data_chunks):
        if fg2_encoding == "indexed8":
            encoded = encode_indexed8_span_payload(
                chunk,
                int(row["width"]),
                int(row["height"]),
                palette_index,
            )
        else:
            encoded = encode_pal4_span_payload(
                chunk,
                int(row["width"]),
                int(row["height"]),
                palette_index,
            )
        row["data_size_raw16"] = row["data_size"]
        row["data_size"] = len(encoded)
        encoded_chunks.append(encoded)
    data_chunks = encoded_chunks

    HEADER_SIZE = 40
    PALETTE_SIZE = (len(palette_values) * 2) if palette_values is not None else 0
    ENTRY_SIZE = 20
    EVENT_SIZE = 4

    table_offset = HEADER_SIZE + PALETTE_SIZE
    data_offset = table_offset + (len(rows) * ENTRY_SIZE)
    next_offset = data_offset
    for row, chunk in zip(rows, data_chunks):
        row["data_offset"] = next_offset
        next_offset += len(chunk)

    sound_events_offset = next_offset if sound_events else 0

    header = struct.pack(
        "<4sHHHHHHHHHHIIIHH",
        b"FGP2",
        2 if fg2_encoding == "indexed8" else 1,
        len(rows),
        1,
        header_flags,
        640,
        480,
        0 if union_min_x is None else union_min_x,
        0 if union_min_y is None else union_min_y,
        0 if union_min_x is None else (union_max_x - union_min_x + 1),
        0 if union_min_y is None else (union_max_y - union_min_y + 1),
        table_offset,
        data_offset,
        sound_events_offset,
        len(sound_events),
        0 if palette_values is None else sum(1 for value in palette_values if value != 0),
    )

    output_pack = Path(args.output_pack)
    output_pack.parent.mkdir(parents=True, exist_ok=True)

    with output_pack.open("wb") as f:
        f.write(header)
        if palette_values is not None:
            for value in palette_values:
                f.write(struct.pack("<H", value))
        for row in rows:
            f.write(struct.pack(
                "<HhhHHHII",
                row["source_frame"],
                row["x"],
                row["y"],
                row["width"],
                row["height"],
                row["hold_vblanks"],
                row["data_offset"],
                row["data_size"],
            ))
        for chunk in data_chunks:
            f.write(chunk)
        for src_frame, sample_id in sound_events:
            f.write(struct.pack("<HH", src_frame, sample_id))

    summary = {
        "scene_label": args.scene_label,
        "frames_dir": str(frames_dir),
        "scene_base_frame": args.scene_base_frame,
        "output_pack": str(output_pack),
        "pack_format": args.pack_format,
        "pack_magic": "FGP2",
        "fg2_encoding": fg2_encoding,
        "base_diff": args.base_diff,
        "fg2_palette": None if palette_values is None else [
            f"0x{value:04x}" for value in palette_values
        ],
        "fg2_visible_color_count": None if palette_values is None else sum(
            1 for value in palette_values if value != 0
        ),
        "frame_step": args.frame_step,
        "timeline_speed": args.timeline_speed,
        "pack_frame_count": len(rows),
        "source_frame_count": len(frame_paths),
        "present_tick_count": sum(row["hold_ticks"] for row in rows),
        "scaled_present_tick_count": rows[-1]["deadline_ticks"] if rows else 0,
        "present_frame_count": sum(row["hold_vblanks"] for row in rows),
        "union_bbox": None if union_min_x is None else {
            "x": union_min_x,
            "y": union_min_y,
            "width": union_max_x - union_min_x + 1,
            "height": union_max_y - union_min_y + 1,
        },
        "total_payload_bytes": sum(row["data_size"] for row in rows),
        "pack_size_bytes": output_pack.stat().st_size,
        "sound_event_count": len(sound_events),
        "sound_events": [
            {"source_frame": src, "sample_id": samp}
            for src, samp in sound_events
        ],
        "rows": rows,
    }

    payload = json.dumps(summary, indent=2) + "\n"
    if args.output_json:
        Path(args.output_json).write_text(payload, encoding="utf-8")
    else:
        print(payload, end="")


if __name__ == "__main__":
    main()
