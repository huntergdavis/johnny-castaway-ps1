#!/usr/bin/env python3
"""
Build Scene Explorer thumbnail SCR files from captured PS1 frames.

Pipeline:
1. For each scene in src/pause_menu/scene_explorer_data.h, find its capture directory
   under regtest-references/<ADS-NAME>-<TAG>/.
2. Pick the captured PNG nearest the scene's 70%-of-FG2-frame-count mark.
   Per-scene overrides live in scripts/scene-explorer-overrides.json.
3. Resize the 640x448 capture to 320x208 (matches PS1 framebuffer minus
   the 32-row chrome strip the menu paints on top).
4. Emit a 320x240 SCR file (raw 16-bit RGB555 little-endian, matching
   OCEAN00.SCR's format) with the thumbnail in the top 208 rows and a
   black 32-row chrome strip at the bottom.
5. Output goes to jc_resources/extracted/scr/SCEXPL_<SLUG>.SCR. Add
   manually to config/ps1/cd_layout.xml on first generation.

Usage:
    python3 scripts/build-scene-explorer-thumbnails.py
    python3 scripts/build-scene-explorer-thumbnails.py --slug fishing1 --debug
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

from PIL import Image

ROOT = Path(__file__).resolve().parent.parent
DATA_HEADER = ROOT / "src/pause_menu/scene_explorer_data.h"
CAPTURE_ROOT = ROOT / "regtest-references"
OVERRIDES_PATH = ROOT / "scripts/scene-explorer-overrides.json"
OUT_DIR = ROOT / "jc_resources/extracted/scr"

# PS1 framebuffer is 640x480 interlaced; thumbnails are 320x240 16-bit
# RGB555 LE (153,600 bytes), centered in the framebuffer at (160, 120)
# by the runtime loader. Menu chrome lives in the empty band below the
# centered thumbnail (logical y=360..480), well clear of any image
# pixels.
THUMB_W = 320
THUMB_H = 240


def parse_data_header():
    """Yield (slug, ads_tag_dir, frame_count) for each entry in scene_explorer_data.h."""
    text = DATA_HEADER.read_text()
    pattern = re.compile(
        r'\{\s*"([a-z0-9]+)",\s*"[^"]*",\s*"[^"]*",\s*"[^"]*",\s*"[^"]*",\s*(\d+),\s*\d+,\s*\d+\s*\}'
    )
    for slug, frame_count in pattern.findall(text):
        yield slug, int(frame_count)


def slug_to_capture_dir(slug):
    """fishing1 -> FISHING-1, miscgag2 -> MISCGAG-2, etc."""
    m = re.match(r"^([a-z]+)(\d+)$", slug)
    if not m:
        return None
    family, tag = m.groups()
    return CAPTURE_ROOT / f"{family.upper()}-{tag}"


def load_overrides():
    if not OVERRIDES_PATH.exists():
        return {}
    return json.loads(OVERRIDES_PATH.read_text())


FAMILY_ABBREV = {
    "fishing":  "FI",
    "johnny":   "JO",
    "mary":     "MA",
    "visitor":  "VI",
    "activity": "AC",
    "suzy":     "SU",
    "miscgag":  "MG",
    "stand":    "ST",
    "walkstuf": "WK",
    "building": "BL",
}


def slug_to_short_name(slug):
    """fishing1 -> SXFI1, activity12 -> SXAC12. Constrained to ISO 9660
    8.3 naming so the file fits on the PS1 disc (12 chars including .SCR).
    Must match grLoadSceneExplorerThumbnail's path-construction logic."""
    m = re.match(r"^([a-z]+)(\d+)$", slug)
    if not m:
        return None
    family, tag = m.group(1), m.group(2)
    abbrev = FAMILY_ABBREV.get(family)
    if abbrev is None:
        return None
    return f"SX{abbrev}{tag}"


def list_capture_frames(scene_dir):
    """Return sorted list of frame_*.png paths for a scene, top-level first then work-dir.

    For the unfiltered .regtest-work/**/frames/jcreborn/ fallback (used when
    the capture script terminated before its post-processing step), drop
    frames before regtest-scene.sh's default scene-start-frame (1680) so the
    70%-mark math lands inside scene playback rather than inside the boot
    sequence.
    """
    if scene_dir is None or not scene_dir.exists():
        return []
    # Prefer post-processed top-level frames if the capture script finished cleanly.
    top = sorted(scene_dir.glob("frame_*.png"))
    if top:
        return top
    # Pre-filtered scene-play frames (capture script's filtered-frames dir).
    nested = sorted(scene_dir.glob(".regtest-work/**/filtered-frames/frame_*.png"))
    if nested:
        return nested
    # Raw frames from boot through scene-end. Keep only the scene-play
    # window (frame >= 1680) since the boot frames would skew the 70%-mark
    # math toward the title screen.
    raw = sorted(scene_dir.glob(".regtest-work/**/frames/jcreborn/frame_*.png"))
    out = []
    for p in raw:
        try:
            n = int(p.stem.split("_")[1])
        except (IndexError, ValueError):
            continue
        if n >= 1680:
            out.append(p)
    return out


def pick_frame(frames, fg2_frames, override_idx):
    """Pick the capture closest to the scene's 70% mark.

    The capture is keyed by emulator frame number, not scene frame number — the
    relationship between them depends on per-frame display vblanks in the FG2
    pack, so we approximate: take the captured frame at the 70th percentile
    of the captured set, which lands roughly 70% through scene playback when
    the capture window is calibrated to the scene's run.
    """
    if not frames:
        return None
    if override_idx is not None:
        return frames[max(0, min(len(frames) - 1, override_idx))]
    target_idx = int(round(0.70 * (len(frames) - 1)))
    return frames[target_idx]


def encode_rgb555(r, g, b):
    return (b >> 3) << 10 | (g >> 3) << 5 | (r >> 3)


def png_to_thumbnail_scr(png_path, out_path):
    img = Image.open(png_path).convert("RGB")
    # Source captures from DuckStation are 640x448; the PS1 framebuffer
    # is 640x480 interlaced. We render at 320x240 (half framebuffer
    # res) and center on the screen at runtime — that gives us natural
    # margins for the chrome band below.
    img = img.resize((THUMB_W, THUMB_H), Image.LANCZOS)

    pixels = img.load()
    buf = bytearray(THUMB_W * THUMB_H * 2)
    for y in range(THUMB_H):
        row_off = y * THUMB_W * 2
        for x in range(THUMB_W):
            r, g, b = pixels[x, y]
            v = encode_rgb555(r, g, b)
            buf[row_off + x * 2] = v & 0xff
            buf[row_off + x * 2 + 1] = (v >> 8) & 0xff
    out_path.write_bytes(bytes(buf))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--slug", help="Process only this slug (default: all)")
    ap.add_argument("--debug", action="store_true",
                    help="Print picked frame paths for diagnostics")
    args = ap.parse_args()

    overrides = load_overrides()
    OUT_DIR.mkdir(parents=True, exist_ok=True)

    entries = list(parse_data_header())
    if args.slug:
        entries = [(s, fc) for (s, fc) in entries if s == args.slug]
        if not entries:
            print(f"slug not found: {args.slug}", file=sys.stderr)
            return 1

    misses = []
    written = 0
    for slug, fg2_frames in entries:
        scene_dir = slug_to_capture_dir(slug)
        frames = list_capture_frames(scene_dir)
        if not frames:
            misses.append(slug)
            if args.debug:
                print(f"  [{slug}] no frames found in {scene_dir}")
            continue
        override_idx = overrides.get(slug, {}).get("frame_idx")
        picked = pick_frame(frames, fg2_frames, override_idx)
        short = slug_to_short_name(slug)
        if short is None:
            misses.append(slug)
            continue
        out_path = OUT_DIR / f"{short}.SCR"
        png_to_thumbnail_scr(picked, out_path)
        written += 1
        if args.debug:
            print(f"  [{slug}] picked={picked.name} → {out_path.name}")

    print(f"Wrote {written} thumbnails to {OUT_DIR.relative_to(ROOT)}")
    if misses:
        print(f"Missing capture data for {len(misses)} scenes: {', '.join(misses)}",
              file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
