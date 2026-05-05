#!/usr/bin/env python3
"""
Pre-generate FG2 high/low packs for every scene still marked
unvalidated in docs/ps1/scene-status.md, so the per-scene
visual-validation loop becomes "launch + signoff" instead of
"regenerate (5 min) + launch + signoff."

Cache: if generated/ps1/foreground/<HIGH>.FG2 and <LOW>.FG2 both
exist + are non-empty, the scene is skipped. Pass --force to
re-export anyway.

Usage:
  scripts/pregenerate-unvalidated-packs.py            # all unvalidated
  scripts/pregenerate-unvalidated-packs.py --force    # re-export everything
  scripts/pregenerate-unvalidated-packs.py --filter visitor   # subset
  scripts/pregenerate-unvalidated-packs.py --dry-run  # show plan, no run
  scripts/pregenerate-unvalidated-packs.py --slugs visitor4 visitor5
"""

import argparse
import os
import re
import shutil
import subprocess
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

# Source-pack family → slug prefix. Mirrors the cd_layout source-name
# convention: HIGH packs use the long family name, LOW packs use the
# 8.3-friendly short prefix. Both reduce to the same slug family.
HIGH_FAM_TO_SLUG = {
    "ACTIVITY": "activity",
    "BUILDING": "building",
    "FISHING":  "fishing",
    "JOHNNY":   "johnny",
    "MARY":     "mary",
    "MISCGAG":  "miscgag",
    "STAND":    "stand",
    "SUZY":     "suzy",
    "VISITOR":  "visitor",
    "WALKSTUF": "walkstuf",
}
LOW_FAM_TO_SLUG = {
    "ACTV": "activity",
    "BUIL": "building",
    "FISH": "fishing",
    "JOHN": "johnny",
    "MARY": "mary",
    "MISC": "miscgag",
    "STND": "stand",
    "SUZY": "suzy",
    "VIST": "visitor",
    "WALK": "walkstuf",
}

# Slug prefix → human ADS name (matches scene-status.md ADS column).
SLUG_TO_ADS = {v: k for k, v in HIGH_FAM_TO_SLUG.items()}


def parse_pack_mapping(layout_path: Path) -> dict:
    """Returns slug → {'high': basename, 'low': basename} from cd_layout.xml."""
    tree = ET.parse(layout_path)
    mapping: dict = {}
    for f in tree.iter("file"):
        name = f.get("name", "")
        src = f.get("source", "")
        if not name.endswith(".FG2") or "foreground/" not in src:
            continue
        src_base = src.rsplit("/", 1)[1].replace(".FG2", "")
        m = re.match(r"^([A-Z]+?)([0-9]+)(LOW|L)?$", src_base)
        if not m:
            continue
        fam_raw, tag, low_suffix = m.group(1), m.group(2), m.group(3)
        slug_fam = HIGH_FAM_TO_SLUG.get(fam_raw) or LOW_FAM_TO_SLUG.get(fam_raw)
        if slug_fam is None:
            continue
        slug = f"{slug_fam}{tag}"
        kind = "low" if low_suffix else "high"
        mapping.setdefault(slug, {})[kind] = src_base
    return mapping


def parse_unvalidated_slugs(status_path: Path) -> list:
    """Returns slugs whose visuals column in scene-status.md is ⏳ (in row order)."""
    slugs: list = []
    row_re = re.compile(
        r"^\|\s*([A-Z]+)\s*\|\s*(\d+)\s*\|\s*([a-z0-9]+)\s*\|\s*([⏳✅~—].*)$"
    )
    for line in status_path.read_text(encoding="utf-8").splitlines():
        m = row_re.match(line)
        if not m:
            continue
        ads, tag, slug, rest = m.group(1), m.group(2), m.group(3), m.group(4)
        # Visuals column is the first cell after the slug.
        visuals = rest.split("|", 1)[0].strip()
        if visuals == "⏳":
            slugs.append((slug, ads, int(tag)))
    return slugs


def cache_hit(project_root: Path, high: str, low: str) -> tuple:
    """(hit, high_path, low_path) — hit only if both packs exist and are non-empty."""
    fg_dir = project_root / "generated" / "ps1" / "foreground"
    high_path = fg_dir / f"{high}.FG2"
    low_path = fg_dir / f"{low}.FG2"
    have_high = high_path.is_file() and high_path.stat().st_size > 0
    have_low = low_path.is_file() and low_path.stat().st_size > 0
    return (have_high and have_low, high_path, low_path)


def export_one(project_root: Path, slug: str, ads_name: str, high: str, low: str) -> int:
    output_dir = project_root / "host-results" / f"{slug}-foreground-pilot"
    cmd = [
        str(project_root / "scripts" / "export-scene-foreground-pilot.sh"),
        str(output_dir),
        slug,
        ads_name,
        high,
        "0",
        "1.0",
        low,
    ]
    print(f"  $ {' '.join(cmd)}", flush=True)
    result = subprocess.run(cmd, cwd=project_root)
    return result.returncode


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--force", action="store_true",
                        help="re-export even if packs already exist")
    parser.add_argument("--dry-run", action="store_true",
                        help="show plan without running anything")
    parser.add_argument("--filter", default="",
                        help="only act on slugs containing this substring (e.g. 'visitor', 'activity')")
    parser.add_argument("--slugs", nargs="+", default=[],
                        help="explicit slugs to act on (overrides scene-status filter)")
    parser.add_argument("--skip", nargs="+", default=[],
                        help="slugs to leave alone (e.g. ones a parallel agent is already running)")
    args = parser.parse_args()

    project_root = Path(__file__).resolve().parent.parent
    layout_path = project_root / "config" / "ps1" / "cd_layout.xml"
    status_path = project_root / "docs" / "ps1" / "scene-status.md"

    if not layout_path.is_file():
        print(f"missing {layout_path}", file=sys.stderr)
        return 1
    if not status_path.is_file():
        print(f"missing {status_path}", file=sys.stderr)
        return 1

    mapping = parse_pack_mapping(layout_path)

    if args.slugs:
        targets = [(slug, SLUG_TO_ADS.get(re.sub(r"\d+", "", slug), "?"),
                    int(re.sub(r"\D", "", slug) or "0"))
                   for slug in args.slugs]
    else:
        targets = parse_unvalidated_slugs(status_path)

    if args.filter:
        targets = [t for t in targets if args.filter in t[0]]
    if args.skip:
        skip_set = set(args.skip)
        targets = [t for t in targets if t[0] not in skip_set]

    if not targets:
        print("no scenes to process")
        return 0

    print(f"plan: {len(targets)} scene(s)")
    cached = []
    to_run = []
    for slug, ads, tag in targets:
        packs = mapping.get(slug)
        if not packs or "high" not in packs or "low" not in packs:
            print(f"  ! {slug}: no HIGH/LOW pack mapping in cd_layout.xml — skipping")
            continue
        high, low = packs["high"], packs["low"]
        hit, hpath, lpath = cache_hit(project_root, high, low)
        if hit and not args.force:
            cached.append((slug, hpath.stat().st_size, lpath.stat().st_size))
        else:
            to_run.append((slug, ads, tag, high, low))

    print(f"  cached: {len(cached)}")
    for slug, hsz, lsz in cached:
        print(f"    [cache] {slug:14s} ({hsz:>9d}B / {lsz:>9d}B)")
    print(f"  to regenerate: {len(to_run)}")
    for slug, ads, tag, high, low in to_run:
        print(f"    [regen] {slug:14s} ADS='{ads} {tag}' HIGH={high} LOW={low}")

    if args.dry_run:
        print("(dry-run; nothing executed)")
        return 0

    fails: list = []
    for i, (slug, ads, tag, high, low) in enumerate(to_run, start=1):
        print(f"\n=== [{i}/{len(to_run)}] {slug} ({ads} {tag}) ===")
        rc = export_one(project_root, slug, f"{ads} {tag}", high, low)
        if rc != 0:
            print(f"  !! {slug} exited rc={rc}")
            fails.append((slug, rc))

    print(f"\nDone. {len(to_run) - len(fails)} regenerated, {len(fails)} failed, {len(cached)} cached.")
    if fails:
        for slug, rc in fails:
            print(f"  FAIL {slug} rc={rc}")
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
