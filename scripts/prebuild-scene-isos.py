#!/usr/bin/env python3
"""
Build one .bin/.cue per scene, each with BOOTMODE.TXT pinned to that
scene. Then validation = "load scratch/scene-isos/<slug>/johnnycastawayps1.cue
in DuckStation, see scene N pinned at boot, sign off, next." No
per-scene CD rebuild.

For each target scene:
  1. Stage config/ps1/BOOTMODE.TXT = "fgpilot <slug> <route>"
  2. Run scripts/build-ps1.sh (re-bakes bootmode_embedded.h, recompiles)
  3. Run scripts/make-cd-image.sh (produces johnnycastawayps1.bin/.cue at repo root)
  4. Move the .bin/.cue into scratch/scene-isos/<slug>/

Restores the original BOOTMODE.TXT + bootmode_embedded.h on exit.

Each ISO is ~144 MB; budget ~3.2 GB for all 22 unvalidated scenes.
After this script finishes, the per-scene cue files live at:
   scratch/scene-isos/<slug>/johnnycastawayps1-<slug>.cue

Usage:
  scripts/prebuild-scene-isos.py                       # all unvalidated
  scripts/prebuild-scene-isos.py --slugs visitor4 visitor5
  scripts/prebuild-scene-isos.py --filter visitor
  scripts/prebuild-scene-isos.py --route 'lowtide 1 night 0'   # custom suffix
  scripts/prebuild-scene-isos.py --dry-run
"""

import argparse
import os
import re
import shutil
import subprocess
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

DEFAULT_ROUTE = "lowtide 0 night 1 holiday 0 raft-stage 4 island-pos -154 54 loop seed 1"

HIGH_FAM_TO_SLUG = {
    "ACTIVITY": "activity", "BUILDING": "building", "FISHING": "fishing",
    "JOHNNY": "johnny", "MARY": "mary", "MISCGAG": "miscgag",
    "STAND": "stand", "SUZY": "suzy", "VISITOR": "visitor",
    "WALKSTUF": "walkstuf",
}
SLUG_TO_ADS = {v: k for k, v in HIGH_FAM_TO_SLUG.items()}


def parse_unvalidated_slugs(status_path: Path) -> list:
    slugs: list = []
    row_re = re.compile(
        r"^\|\s*([A-Z]+)\s*\|\s*(\d+)\s*\|\s*([a-z0-9]+)\s*\|\s*([⏳✅~—].*)$"
    )
    for line in status_path.read_text(encoding="utf-8").splitlines():
        m = row_re.match(line)
        if not m:
            continue
        ads, tag, slug, rest = m.group(1), m.group(2), m.group(3), m.group(4)
        visuals = rest.split("|", 1)[0].strip()
        if visuals == "⏳":
            slugs.append(slug)
    return slugs


def stage_bootmode(bootmode_path: Path, content: str) -> str:
    original = bootmode_path.read_text(encoding="utf-8") if bootmode_path.is_file() else ""
    bootmode_path.write_text(content + "\n", encoding="utf-8")
    return original


def restore_bootmode(bootmode_path: Path, original: str) -> None:
    bootmode_path.write_text(original, encoding="utf-8")


def build_one(project_root: Path, slug: str, route: str,
              out_dir: Path, dry_run: bool) -> int:
    boot_str = f"fgpilot {slug} {route}".strip()
    target_cue = out_dir / f"johnnycastawayps1-{slug}.cue"
    target_bin = out_dir / f"johnnycastawayps1-{slug}.bin"

    print(f"\n=== {slug} ===")
    print(f"  BOOTMODE.TXT = {boot_str}")
    print(f"  → {target_cue}")
    if dry_run:
        return 0

    out_dir.mkdir(parents=True, exist_ok=True)
    bootmode_path = project_root / "config" / "ps1" / "BOOTMODE.TXT"
    bootmode_header = project_root / "config" / "ps1" / "bootmode_embedded.h"
    original_txt = stage_bootmode(bootmode_path, boot_str)
    original_header = bootmode_header.read_text(encoding="utf-8") if bootmode_header.is_file() else ""

    try:
        rc = subprocess.call(["bash", str(project_root / "scripts" / "build-ps1.sh")],
                             cwd=project_root)
        if rc != 0:
            print(f"  !! build-ps1.sh exited rc={rc}")
            return rc
        rc = subprocess.call(["bash", str(project_root / "scripts" / "make-cd-image.sh")],
                             cwd=project_root)
        if rc != 0:
            print(f"  !! make-cd-image.sh exited rc={rc}")
            return rc

        repo_bin = project_root / "johnnycastawayps1.bin"
        repo_cue = project_root / "johnnycastawayps1.cue"
        if not repo_bin.is_file() or not repo_cue.is_file():
            print(f"  !! expected johnnycastawayps1.bin/.cue at repo root after make-cd-image.sh")
            return 1

        shutil.move(str(repo_bin), str(target_bin))
        # Rewrite the cue file's FILE line so it points at the renamed .bin.
        cue_text = repo_cue.read_text(encoding="utf-8")
        cue_text = re.sub(r'FILE\s+"[^"]+"', f'FILE "{target_bin.name}"', cue_text)
        target_cue.write_text(cue_text, encoding="utf-8")
        repo_cue.unlink()
        return 0
    finally:
        restore_bootmode(bootmode_path, original_txt)
        if original_header:
            bootmode_header.write_text(original_header, encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--slugs", nargs="+", default=[],
                        help="explicit slugs (overrides scene-status filter)")
    parser.add_argument("--filter", default="",
                        help="only act on slugs containing this substring")
    parser.add_argument("--skip", nargs="+", default=[],
                        help="slugs to leave alone (e.g. ones a parallel agent is handling)")
    parser.add_argument("--route", default=DEFAULT_ROUTE,
                        help=f"route suffix appended after slug (default: {DEFAULT_ROUTE!r})")
    parser.add_argument("--out-dir", default="scratch/scene-isos",
                        help="parent directory for per-scene .bin/.cue (default: scratch/scene-isos)")
    parser.add_argument("--dry-run", action="store_true")
    args = parser.parse_args()

    project_root = Path(__file__).resolve().parent.parent
    status_path = project_root / "docs" / "ps1" / "scene-status.md"

    targets = args.slugs if args.slugs else parse_unvalidated_slugs(status_path)
    if args.filter:
        targets = [s for s in targets if args.filter in s]
    if args.skip:
        skip_set = set(args.skip)
        targets = [s for s in targets if s not in skip_set]

    if not targets:
        print("no scenes to process")
        return 0

    out_root = (project_root / args.out_dir).resolve()
    print(f"plan: {len(targets)} scene(s)")
    print(f"  out_root: {out_root}")
    print(f"  route:    {args.route}")
    for s in targets:
        print(f"    - {s}")
    if args.dry_run:
        print("(dry-run; nothing executed)")
        return 0

    fails: list = []
    for i, slug in enumerate(targets, start=1):
        scene_dir = out_root / slug
        print(f"\n>>> [{i}/{len(targets)}] {slug}")
        rc = build_one(project_root, slug, args.route, scene_dir, args.dry_run)
        if rc != 0:
            fails.append((slug, rc))

    print(f"\nDone. {len(targets) - len(fails)} ISOs built, {len(fails)} failed.")
    if fails:
        for slug, rc in fails:
            print(f"  FAIL {slug} rc={rc}")
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
