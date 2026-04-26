#!/usr/bin/env python3
"""
Render a single PNG contact sheet of all 31 reviewable holidays × 5
variants. Each cell is the sprite at 4× zoom with a thin border and a
per-row label. Useful for offline review or as a PR attachment.

Run:
  python3 scripts/holidays-contact-sheet.py
  → writes scratch/holidays-contact-sheet.png
"""
import json
import sys
from pathlib import Path

try:
    import yaml
    from PIL import Image, ImageDraw, ImageFont
except ImportError as e:
    sys.stderr.write(f"error: {e}\n")
    sys.exit(2)

REPO = Path(__file__).resolve().parent.parent
YAML_PATH = REPO / "holidays.yml"
ART_DIR = REPO / "scratch" / "holidays-art"
OUT = REPO / "scratch" / "holidays-contact-sheet.png"
PICKS_FILE = REPO / "scratch" / "holidays-picks.json"
PICKS_FALLBACK = REPO / "scratch" / "holidays-picks-default.json"

ZOOM = 4
PAD = 8
ROW_LABEL_W = 220
COL_LABEL_H = 22
CELL_GAP = 12
BG = (24, 24, 36)
PANEL = (44, 44, 60)
TEXT = (232, 232, 240)
MUTED = (160, 160, 180)
ACCENT = (196, 124, 255)


def slugify(name: str) -> str:
    return name.replace(" ", "_").replace("'", "").replace(".", "")


def load_picks() -> dict:
    """Load picks JSON if present, else default-picks fallback, else {}."""
    for p in (PICKS_FILE, PICKS_FALLBACK):
        if p.exists():
            try:
                return json.loads(p.read_text(encoding="utf-8"))
            except Exception:
                pass
    return {}


def main():
    holidays = yaml.safe_load(open(YAML_PATH, "r", encoding="utf-8"))
    new_holidays = [h for h in holidays if h.get("existing_sprite") is None]
    new_holidays.sort(key=lambda h: h["id"])
    variants = [(1, "v1 LITERAL"), (2, "v2 MIN"), (3, "v3 BUSY"),
                (4, "v4 PLAYFUL"), (5, "v5 NIGHT")]
    picks = load_picks()

    # Cell size = max sprite dimensions × ZOOM
    cell_w = max(h["sprite"]["width"] for h in new_holidays) * ZOOM + PAD * 2
    cell_h = max(h["sprite"]["height"] for h in new_holidays) * ZOOM + PAD * 2

    HEADER_H = 36  # title + sub-line
    sheet_w = ROW_LABEL_W + len(variants) * (cell_w + CELL_GAP) + PAD
    sheet_h = HEADER_H + COL_LABEL_H + len(new_holidays) * (cell_h + CELL_GAP) + PAD

    sheet = Image.new("RGB", (sheet_w, sheet_h), BG)
    draw = ImageDraw.Draw(sheet)
    try:
        font = ImageFont.load_default()
    except Exception:
        font = None

    # Title and pick-summary header
    draw.text((PAD, 6), "Johnny Castaway — Holiday sprite contact sheet",
              fill=ACCENT, font=font)
    pick_count = sum(1 for h in new_holidays if str(h["id"]) in picks)
    counts = {str(v): 0 for v in (1, 2, 3, 4, 5)}
    for v in picks.values():
        sv = str(v)
        if sv in counts:
            counts[sv] += 1
    summary = (f"{len(new_holidays)} holidays × {len(variants)} variants  ·  "
               f"{pick_count}/{len(new_holidays)} picked  ·  "
               f"v1={counts['1']} v2={counts['2']} v3={counts['3']} "
               f"v4={counts['4']} v5={counts['5']}")
    draw.text((PAD, 22), summary, fill=MUTED, font=font)

    # Column headers
    for ci, (vi, label) in enumerate(variants):
        cx = ROW_LABEL_W + ci * (cell_w + CELL_GAP) + cell_w // 2
        draw.text((cx - 30, HEADER_H + 4), label, fill=ACCENT, font=font)

    # Rows
    for ri, h in enumerate(new_holidays):
        row_y = HEADER_H + COL_LABEL_H + ri * (cell_h + CELL_GAP)
        # Row label
        draw.text((PAD, row_y + 6), f"{h['id']:02d}", fill=MUTED, font=font)
        draw.text((PAD + 24, row_y + 6), h["name"][:24], fill=TEXT, font=font)
        draw.text((PAD + 24, row_y + 22), h.get("short_name", ""),
                  fill=MUTED, font=font)
        sw, shh = h["sprite"]["width"], h["sprite"]["height"]
        draw.text((PAD + 24, row_y + 38), f"{sw}×{shh} px",
                  fill=MUTED, font=font)
        slug = slugify(h["short_name"])
        picked_vi = picks.get(str(h["id"]))
        try:
            picked_vi_int = int(picked_vi) if picked_vi is not None else None
        except (TypeError, ValueError):
            picked_vi_int = None
        for ci, (vi, _label) in enumerate(variants):
            p = ART_DIR / f"{h['id']:02d}-{slug}-v{vi}.png"
            cx = ROW_LABEL_W + ci * (cell_w + CELL_GAP)
            # Cell background — green tint for the picked variant.
            cell_bg = (40, 70, 56) if vi == picked_vi_int else PANEL
            draw.rectangle(
                [cx, row_y, cx + cell_w, row_y + cell_h], fill=cell_bg)
            if vi == picked_vi_int:
                # Bright accent border for the picked cell
                draw.rectangle(
                    [cx, row_y, cx + cell_w, row_y + cell_h],
                    outline=(77, 213, 153), width=2)
            if not p.exists():
                draw.text((cx + PAD, row_y + cell_h // 2 - 6),
                          "(missing)", fill=(255, 96, 96), font=font)
                continue
            sprite = Image.open(p).convert("RGBA")
            # Treat palette index 0 (transparent) as see-through; PIL
            # handles that automatically for "P" with transparency, but
            # since our PNGs save without an alpha channel we just paste
            # solid. The black bg looks fine.
            sw_, shh_ = sprite.size
            scaled = sprite.resize((sw_ * ZOOM, shh_ * ZOOM),
                                   Image.NEAREST)
            # Center inside the cell
            ox = cx + (cell_w - scaled.width) // 2
            oy = row_y + (cell_h - scaled.height) // 2
            sheet.paste(scaled, (ox, oy))

    OUT.parent.mkdir(parents=True, exist_ok=True)
    sheet.save(OUT, optimize=True)
    print(f"Wrote {OUT.relative_to(REPO)}  "
          f"({sheet_w}×{sheet_h}, {len(new_holidays)} holidays × {len(variants)} variants)")


if __name__ == "__main__":
    main()
