#!/usr/bin/env python3
"""
Generate a default `holidays-picks.json` choosing variant 1 (LITERAL) for
every reviewable holiday. Used as a fallback so Phase D (PSB packaging)
can run without owner input — but the real picks should override this
once the owner has reviewed `scratch/holidays-preview.html`.

Run:
  python3 scripts/holidays-default-picks.py
  → writes scratch/holidays-picks-default.json
"""
import json
import sys
from pathlib import Path

try:
    import yaml
except ImportError:
    sys.stderr.write("error: PyYAML not installed\n")
    sys.exit(1)

REPO = Path(__file__).resolve().parent.parent
YAML_PATH = REPO / "holidays.yml"
OUT_PATH = REPO / "scratch" / "holidays-picks-default.json"


def main():
    holidays = yaml.safe_load(open(YAML_PATH, "r", encoding="utf-8"))
    picks = {}
    for h in holidays:
        if h.get("existing_sprite") is None:
            picks[str(h["id"])] = "1"  # default to LITERAL variant
    OUT_PATH.parent.mkdir(parents=True, exist_ok=True)
    OUT_PATH.write_text(json.dumps(picks, indent=2) + "\n", encoding="utf-8")
    print(f"Wrote {OUT_PATH.relative_to(REPO)} ({len(picks)} default picks)")


if __name__ == "__main__":
    main()
