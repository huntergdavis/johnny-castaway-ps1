#!/usr/bin/env python3
"""
Run the holiday-art renderers and save 3 variants per new holiday to
`scratch/holidays-art/<id>-<short_name>-v{1,2,3}.png`.

The renderer functions live in `scripts/holidays_concepts.py` (one
`RENDERERS` dict, keyed by holiday id, value is a 3-tuple of callables).
This script handles the I/O: reads holidays.yml, invokes each renderer,
saves the PNGs, prints a summary.

Run:
  python3 scripts/holidays-generate-art.py
"""
import sys
from pathlib import Path

try:
    import yaml
except ImportError:
    sys.stderr.write("error: PyYAML not installed. pip install pyyaml\n")
    sys.exit(1)

REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO / "scripts"))

import holidays_art_lib  # noqa: E402

try:
    from holidays_concepts import RENDERERS
except ImportError as e:
    sys.stderr.write(f"error: cannot import holidays_concepts: {e}\n")
    sys.exit(1)

YAML_PATH = REPO / "holidays.yml"
OUT_DIR = REPO / "scratch" / "holidays-art"


def main():
    with open(YAML_PATH, "r", encoding="utf-8") as f:
        holidays = yaml.safe_load(f)

    OUT_DIR.mkdir(parents=True, exist_ok=True)

    rendered = 0
    skipped = 0
    failed = []
    for h in holidays:
        if h.get("existing_sprite") is not None:
            skipped += 1
            continue
        hid = h["id"]
        short = h.get("short_name", f"H{hid}").replace(" ", "_").replace("'", "").replace(".", "")
        variants = RENDERERS.get(hid)
        if variants is None:
            failed.append((hid, h["name"], "no renderer in RENDERERS"))
            continue
        for vi, fn in enumerate(variants, start=1):
            try:
                sp = fn(h)
                out = OUT_DIR / f"{hid:02d}-{short}-v{vi}.png"
                sp.save(str(out))
                rendered += 1
            except Exception as ex:
                failed.append((hid, h["name"], f"v{vi}: {ex}"))

    print(f"\nRendered: {rendered}  Skipped (originals): {skipped}")
    if failed:
        print(f"Failed: {len(failed)}")
        for hid, name, reason in failed:
            print(f"  id {hid} {name!r}: {reason}")
    print(f"Output dir: {OUT_DIR.relative_to(REPO)}/")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
