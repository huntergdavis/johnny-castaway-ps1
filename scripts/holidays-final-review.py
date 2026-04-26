#!/usr/bin/env python3
"""
Generate scratch/holidays-final-review.html — a compact "this is what
you'll ship" summary showing ONLY the variants chosen in the picks
JSON, in calendar order, with the actual next-occurrence date.

Complementary to scratch/holidays-preview.html (which shows all 5
variants per holiday for review). Use this AFTER picking, as a final
sanity check before handing off to Phase D.

Run:
  python3 scripts/holidays-final-review.py
  python3 scripts/holidays-final-review.py --picks scratch/holidays-picks.json
  xdg-open scratch/holidays-final-review.html
"""
import argparse
import base64
import datetime
import html
import json
import sys
from pathlib import Path

try:
    import yaml
except ImportError:
    sys.stderr.write("error: PyYAML not installed\n")
    sys.exit(2)

REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO / "scripts"))

# Reuse the date helpers from holidays-preview.py via importlib so we
# don't duplicate the algorithm.
import importlib.util
_pv_spec = importlib.util.spec_from_file_location(
    "holidays_preview_mod", REPO / "scripts" / "holidays-preview.py")
_pv = importlib.util.module_from_spec(_pv_spec)
_pv_spec.loader.exec_module(_pv)


YAML_PATH = REPO / "holidays.yml"
ART_DIR = REPO / "scratch" / "holidays-art"
DEFAULT_PICKS = REPO / "scratch" / "holidays-picks.json"
DEFAULT_PICKS_FALLBACK = REPO / "scratch" / "holidays-picks-default.json"
OUT = REPO / "scratch" / "holidays-final-review.html"

VARIANT_LABELS = ["LITERAL", "MINIMALIST", "BUSY", "PLAYFUL", "NIGHT"]


def slugify(name: str) -> str:
    return name.replace(" ", "_").replace("'", "").replace(".", "")


def png_to_data_uri(p: Path) -> str | None:
    if not p.exists():
        return None
    return f"data:image/png;base64,{base64.b64encode(p.read_bytes()).decode('ascii')}"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--picks", type=Path, default=DEFAULT_PICKS)
    ap.add_argument("--out", type=Path, default=OUT)
    args = ap.parse_args()

    picks_path = args.picks
    if not picks_path.exists():
        if DEFAULT_PICKS_FALLBACK.exists():
            picks_path = DEFAULT_PICKS_FALLBACK
            print(f"note: falling back to {picks_path.name}")
        else:
            sys.stderr.write(f"error: no picks JSON found\n")
            sys.exit(2)

    picks = json.loads(picks_path.read_text(encoding="utf-8"))
    holidays = yaml.safe_load(open(YAML_PATH, "r", encoding="utf-8"))

    # Sort by next occurrence (so the page reads as a calendar).
    today = datetime.date.today()

    def next_date_obj(rule):
        # Parse the next occurrence text back into a date-tuple for sort.
        # If unknown, push to end.
        s = _pv.next_occurrence_text(rule, today) if rule else None
        if not s:
            return (9999, 99, 99)
        # Format "Mon D, YYYY"
        try:
            d = datetime.datetime.strptime(s, "%b %d, %Y").date()
            return (d.year, d.month, d.day)
        except Exception:
            return (9999, 99, 99)

    def sort_key(h):
        return next_date_obj(h.get("date_rule"))

    sorted_h = sorted(holidays, key=sort_key)

    cards = []
    for h in sorted_h:
        hid = h["id"]
        rule = h.get("date_rule") or {}
        next_text = _pv.next_occurrence_text(rule, today) or "—"
        rule_text = _pv.date_rule_text(rule)

        if h.get("existing_sprite") is not None:
            note = f'<span class="note">existing PSB index {h.get("existing_sprite")}</span>'
            img_html = f'<div class="placeholder">[Original sprite preserved]</div>'
        else:
            vi = picks.get(str(hid))
            if vi is None:
                img_html = '<div class="placeholder unpicked">— no pick —</div>'
                note = '<span class="note warn">unpicked</span>'
            else:
                try:
                    vi_int = int(vi)
                except (TypeError, ValueError):
                    vi_int = 0
                p = ART_DIR / f"{hid:02d}-{slugify(h['short_name'])}-v{vi_int}.png"
                data_uri = png_to_data_uri(p)
                if data_uri:
                    label = (VARIANT_LABELS[vi_int - 1]
                             if 1 <= vi_int <= len(VARIANT_LABELS)
                             else f"v{vi_int}")
                    img_html = (
                        f'<img src="{data_uri}" '
                        f'width="{h["sprite"]["width"] * 4}" '
                        f'height="{h["sprite"]["height"] * 4}" '
                        f'alt="v{vi_int}">')
                    note = f'<span class="note">picked v{vi_int} {label}</span>'
                else:
                    img_html = f'<div class="placeholder error">missing PNG: v{vi_int}</div>'
                    note = '<span class="note warn">PNG missing</span>'

        cards.append(f'''
        <div class="card">
          <div class="meta">
            <div class="when">{html.escape(next_text)}</div>
            <div class="rule">{html.escape(rule_text)}</div>
            <div class="name">{html.escape(h["name"])}</div>
            {note}
          </div>
          <div class="sprite">{img_html}</div>
        </div>''')

    css = """
      :root {
        --bg: #14141c; --panel: #24242e; --text: #e8e8f0;
        --muted: #888; --accent: #c47cff; --pick: #4dd599;
      }
      body { background: var(--bg); color: var(--text); margin: 0; padding: 24px; font: 14px/1.4 system-ui, -apple-system, sans-serif; }
      h1 { color: var(--accent); margin: 0 0 4px; }
      .subtitle { color: var(--muted); margin-bottom: 24px; }
      .grid { display: grid; grid-template-columns: repeat(auto-fill, minmax(380px, 1fr)); gap: 12px; }
      .card { background: var(--panel); border-radius: 8px; padding: 12px; display: grid; grid-template-columns: 1fr auto; gap: 12px; align-items: center; }
      .when { color: var(--accent); font-weight: 600; font-size: 14px; }
      .rule { color: var(--muted); font-size: 12px; margin: 2px 0; }
      .name { color: var(--text); font-size: 16px; font-weight: 600; margin-top: 4px; }
      .note { display: inline-block; color: var(--pick); font-size: 11px; margin-top: 4px; }
      .note.warn { color: #f55; }
      .sprite { background: #000; padding: 6px; border-radius: 4px; image-rendering: pixelated; image-rendering: crisp-edges; }
      .sprite img { display: block; }
      .placeholder { color: var(--muted); font-style: italic; padding: 24px; min-width: 100px; text-align: center; }
      .placeholder.unpicked { color: #f55; }
      .placeholder.error { color: #f55; }
    """

    full = f"""<!doctype html><html><head><meta charset="utf-8">
<title>Holidays — Final Review</title><style>{css}</style></head>
<body>
  <h1>Holiday final review</h1>
  <p class="subtitle">Each card shows the variant chosen in
  <code>{html.escape(str(picks_path.relative_to(REPO) if picks_path.is_relative_to(REPO) else picks_path))}</code>,
  sorted by next-occurrence date. Originals show a placeholder (preserved as-is).
  Total holidays: {len(sorted_h)} · picks: {len(picks)}.</p>
  <div class="grid">{''.join(cards)}</div>
</body></html>"""

    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(full, encoding="utf-8")
    print(f"Wrote {args.out.relative_to(REPO)}")


if __name__ == "__main__":
    main()
