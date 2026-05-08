#!/usr/bin/env python3
"""
Apply a hand-picked PNG screenshot as the Scene Explorer thumbnail for
one scene, bypassing the auto-pick at 70% of the FG2 frame count.

Usage:
    python3 scripts/apply-custom-scene-thumbnail.py <slug> <png-path>

Reuses build-scene-explorer-thumbnails.py's png_to_thumbnail_scr +
slug_to_short_name so the output format and filename stay identical to
the auto-built thumbnails.
"""
import argparse
import importlib.util
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
BUILDER = ROOT / "scripts/build-scene-explorer-thumbnails.py"

spec = importlib.util.spec_from_file_location("scene_thumb", BUILDER)
mod = importlib.util.module_from_spec(spec)
spec.loader.exec_module(mod)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("slug", help="Lowercase scene slug, e.g. fishing1")
    ap.add_argument("png", help="Source PNG screenshot path")
    args = ap.parse_args()

    short = mod.slug_to_short_name(args.slug)
    if short is None:
        print(f"unknown slug: {args.slug}", file=sys.stderr)
        return 2

    png = Path(args.png).expanduser()
    if not png.is_file():
        print(f"not a file: {png}", file=sys.stderr)
        return 2

    out_dir = ROOT / "jc_resources/extracted/scr"
    out_dir.mkdir(parents=True, exist_ok=True)
    out_path = out_dir / f"{short}.SCR"
    mod.png_to_thumbnail_scr(png, out_path)
    print(f"wrote {out_path} from {png}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
