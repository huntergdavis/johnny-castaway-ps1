#!/usr/bin/env python3
"""
Per-sprite quality audit. Loads every PNG in scratch/holidays-art/ and
reports:

  * total pixels
  * # distinct palette indices used
  * top-color fraction + which palette index
  * % transparent (idx 0)

Sorted by `--sort` to surface the most suspicious entries.

Run:
  python3 scripts/holidays-quality-audit.py --sort top
  python3 scripts/holidays-quality-audit.py --sort distinct
  python3 scripts/holidays-quality-audit.py --sort transparent
"""
import argparse
import sys
from collections import defaultdict
from pathlib import Path

try:
    from PIL import Image
except ImportError:
    sys.stderr.write("error: Pillow not installed\n")
    sys.exit(2)

REPO = Path(__file__).resolve().parent.parent
ART_DIR = REPO / "scratch" / "holidays-art"

PALETTE_NAMES = [
    "TRANS", "WHITE", "BLACK", "SKIN", "TRUNK", "GREEN", "DGREEN",
    "SKY", "DEEPBLU", "SAND", "RED", "YELLOW", "ORANGE", "PINK",
    "PURPLE", "GRAY",
]


def palette_name(i):
    return PALETTE_NAMES[i] if 0 <= i < 16 else f"?{i}"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--sort", choices=["name", "top", "distinct", "transparent"],
                    default="top",
                    help="sort by: name, top color frac, # distinct indices, % transparent")
    args = ap.parse_args()

    rows = []
    for p in sorted(ART_DIR.glob("*.png")):
        with Image.open(p) as im:
            if im.mode != "P":
                continue
            pixels = list(im.getdata())
        n = len(pixels)
        if not n:
            continue
        hist = defaultdict(int)
        for v in pixels:
            hist[v] += 1
        top_idx, top_count = max(hist.items(), key=lambda kv: kv[1])
        top_frac = top_count / n
        n_distinct = len(hist)
        trans_frac = hist.get(0, 0) / n
        rows.append({
            "name": p.name,
            "n": n,
            "top_idx": top_idx,
            "top_frac": top_frac,
            "n_distinct": n_distinct,
            "trans_frac": trans_frac,
        })

    if args.sort == "top":
        rows.sort(key=lambda r: -r["top_frac"])
    elif args.sort == "distinct":
        rows.sort(key=lambda r: r["n_distinct"])
    elif args.sort == "transparent":
        rows.sort(key=lambda r: -r["trans_frac"])
    else:
        rows.sort(key=lambda r: r["name"])

    print(f"{'sprite':38s} {'pixels':>7s} {'top':>10s} {'frac':>5s} "
          f"{'distinct':>8s} {'%trans':>7s}")
    print("-" * 84)
    for r in rows:
        print(f"{r['name']:38s} {r['n']:>7d} "
              f"{palette_name(r['top_idx']):>10s} "
              f"{r['top_frac']*100:>4.0f}% "
              f"{r['n_distinct']:>8d} "
              f"{r['trans_frac']*100:>6.0f}%")


if __name__ == "__main__":
    main()
