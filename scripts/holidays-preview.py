#!/usr/bin/env python3
"""
Generate `scratch/holidays-preview.html` — a self-contained, single-file
HTML page that renders all 31 new holiday sprites × 3 variants in a
calendar-ordered grid for owner review.

Each cell shows:
  * Holiday name + date rule (computed via holidays-codegen rule kinds)
  * Concept description from holidays.yml
  * Three variant images side-by-side (v1 LITERAL / v2 MINIMALIST / v3 BUSY)
  * Sprite size annotation
  * A "preference" radio that captures the owner's pick (writes a JSON
    sidecar at scratch/holidays-picks.json on Save).

Each PNG is embedded as base64 so the HTML works without a webserver.

Run:
  python3 scripts/holidays-generate-art.py    # produce the PNGs
  python3 scripts/holidays-preview.py         # produce the HTML
  xdg-open scratch/holidays-preview.html
"""
import base64
import html
import json
import sys
from pathlib import Path

try:
    import yaml
except ImportError:
    sys.stderr.write("error: PyYAML not installed. pip install pyyaml\n")
    sys.exit(1)

REPO = Path(__file__).resolve().parent.parent
YAML_PATH = REPO / "holidays.yml"
ART_DIR = REPO / "scratch" / "holidays-art"
HTML_OUT = REPO / "scratch" / "holidays-preview.html"

WEEKDAY_NAMES = ["Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"]
MONTH_NAMES = ["", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
               "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"]
NTH_NAMES = {1: "1st", 2: "2nd", 3: "3rd", 4: "4th", 5: "5th", -1: "last"}


def date_rule_text(rule: dict) -> str:
    if not rule:
        return "?"
    k = rule.get("kind", "")
    if k == "fixed":
        return f"{MONTH_NAMES[rule['month']]} {rule['day']}"
    if k == "nth_weekday":
        return f"{NTH_NAMES.get(rule['n'], '?')} {WEEKDAY_NAMES[rule['weekday']]} of {MONTH_NAMES[rule['month']]}"
    if k == "easter_offset":
        off = rule.get("offset", 0)
        if off == 0:    return "Easter Sunday (movable)"
        if off < 0:     return f"{-off} days before Easter (movable)"
        return f"{off} days after Easter (movable)"
    if k in ("equinox", "equinox_vernal"):
        return "Spring Equinox (~Mar 20)"
    if k == "equinox_autumnal":
        return "Autumn Equinox (~Sep 22)"
    if k in ("solstice", "solstice_summer"):
        return "Summer Solstice (~Jun 21)"
    if k == "solstice_winter":
        return "Winter Solstice (~Dec 21)"
    if k == "election_day":
        return "1st Tue after 1st Mon of Nov"
    return k


def png_to_data_uri(path: Path) -> str | None:
    if not path.exists():
        return None
    with open(path, "rb") as f:
        b64 = base64.b64encode(f.read()).decode("ascii")
    return f"data:image/png;base64,{b64}"


def html_escape(s: str) -> str:
    return html.escape(s or "")


def slugify(name: str) -> str:
    return name.replace(" ", "_").replace("'", "").replace(".", "")


def main():
    with open(YAML_PATH, "r", encoding="utf-8") as f:
        holidays = yaml.safe_load(f)

    # Sort by month/day for calendar ordering. Movable feasts approx-sorted.
    def calendar_key(h):
        rule = h.get("date_rule", {}) or {}
        k = rule.get("kind")
        if k == "fixed":
            return (rule["month"], rule["day"])
        if k == "nth_weekday":
            return (rule["month"], 15)  # rough mid-month
        if k in ("easter_offset",):
            return (4, 1)  # Apr-ish for review purposes
        if k == "equinox" and rule.get("month", 3) == 3:
            return (3, 20)
        if k == "equinox_autumnal" or (k == "equinox" and rule.get("month") == 9):
            return (9, 22)
        if k == "solstice_summer" or (k == "solstice" and rule.get("month") == 6):
            return (6, 21)
        if k == "solstice_winter":
            return (12, 21)
        if k == "election_day":
            return (11, 5)
        return (99, 99)

    holidays_sorted = sorted(holidays, key=calendar_key)

    css = """
      :root {
        --bg: #1a1a24;
        --panel: #2a2a3a;
        --accent: #c47cff;
        --text: #e8e8f0;
        --muted: #999;
        --pick: #4dd599;
      }
      body { background: var(--bg); color: var(--text); font: 14px/1.4 system-ui, -apple-system, sans-serif; margin: 0; padding: 24px; }
      h1 { color: var(--accent); margin: 0 0 8px; }
      .subtitle { color: var(--muted); margin: 0 0 24px; }
      .month-section { margin-top: 32px; }
      .month-header { color: var(--accent); border-bottom: 1px solid var(--panel); padding-bottom: 4px; margin: 24px 0 12px; }
      .holiday { background: var(--panel); border-radius: 8px; padding: 12px 16px; margin-bottom: 12px; display: grid; grid-template-columns: 240px 1fr; gap: 16px; align-items: start; }
      .holiday-meta { font-size: 13px; }
      .holiday-name { font-size: 16px; color: var(--text); font-weight: 600; }
      .holiday-id { color: var(--muted); font-size: 11px; }
      .holiday-date { color: var(--accent); margin: 4px 0; }
      .holiday-size { color: var(--muted); font-size: 11px; }
      .holiday-desc { color: var(--text); margin-top: 8px; font-size: 12px; line-height: 1.4; }
      .holiday-palette { display: flex; gap: 4px; margin-top: 6px; }
      .palette-swatch { width: 16px; height: 16px; border: 1px solid #444; border-radius: 2px; }
      .variants { display: flex; gap: 16px; align-items: flex-start; }
      .variant { display: flex; flex-direction: column; align-items: center; gap: 4px; }
      .variant-label { color: var(--muted); font-size: 11px; text-transform: uppercase; letter-spacing: 0.5px; }
      .variant-img { background: #000; padding: 4px; border-radius: 4px; image-rendering: pixelated; image-rendering: crisp-edges; }
      .variant.picked { box-shadow: 0 0 0 2px var(--pick); border-radius: 6px; padding: 4px; }
      .pick-label { color: var(--pick); font-size: 10px; text-transform: uppercase; }
      .pick-radio { margin-top: 4px; }
      .controls { position: sticky; top: 0; background: var(--bg); padding: 12px 0; border-bottom: 1px solid var(--panel); margin-bottom: 16px; z-index: 10; }
      .save-btn { background: var(--pick); color: #000; padding: 8px 16px; border: 0; border-radius: 4px; font: 600 13px sans-serif; cursor: pointer; }
      .stat { color: var(--muted); margin-left: 16px; font-size: 12px; }
      .legend { color: var(--muted); font-size: 12px; }
      a.dl-link { color: var(--accent); }
      .missing { color: #f55; padding: 8px 12px; background: #2a1a1a; border-radius: 4px; }
    """

    # Build the holiday cards
    cards_html = []
    last_month = None
    new_holidays = [h for h in holidays_sorted if h.get("existing_sprite") is None]
    for h in new_holidays:
        rule = h.get("date_rule", {}) or {}
        m = rule.get("month") if rule.get("kind") == "fixed" else (
            rule.get("month") if rule.get("kind") == "nth_weekday" else 0)
        if rule.get("kind") in ("equinox_vernal", "equinox") and rule.get("month") == 3: m = 3
        if rule.get("kind") == "equinox_autumnal" or (rule.get("kind") == "equinox" and rule.get("month") == 9): m = 9
        if rule.get("kind") in ("solstice_summer", "solstice") and rule.get("month") == 6: m = 6
        if rule.get("kind") == "election_day": m = 11
        if rule.get("kind") == "easter_offset": m = 4

        if m != last_month:
            cards_html.append(f'<div class="month-section"><h2 class="month-header">{MONTH_NAMES[m] if m else "Movable"}</h2></div>')
            last_month = m

        slug = slugify(h["short_name"])
        variants_html = []
        for vi in (1, 2, 3):
            png_path = ART_DIR / f"{h['id']:02d}-{slug}-v{vi}.png"
            data_uri = png_to_data_uri(png_path)
            if data_uri:
                w = h["sprite"]["width"] * 4
                hh = h["sprite"]["height"] * 4
                variant_label = ["LITERAL", "MINIMALIST", "BUSY"][vi - 1]
                variants_html.append(f'''
                <div class="variant" data-id="{h['id']}" data-variant="{vi}">
                  <span class="variant-label">v{vi} {variant_label}</span>
                  <img class="variant-img" src="{data_uri}" width="{w}" height="{hh}" alt="v{vi}">
                  <input class="pick-radio" type="radio" name="pick-{h['id']}" value="{vi}" id="pick-{h['id']}-{vi}">
                </div>''')
            else:
                variants_html.append(f'''
                <div class="variant"><span class="missing">v{vi} missing<br><small>{html_escape(str(png_path.relative_to(REPO)))}</small></span></div>''')

        palette_swatches = "".join(
            f'<div class="palette-swatch" style="background:{html_escape(c)}" title="{html_escape(c)}"></div>'
            for c in (h.get("palette") or [])
        )

        cards_html.append(f'''
        <div class="holiday" id="h{h['id']}">
          <div class="holiday-meta">
            <div class="holiday-id">id={h['id']}</div>
            <div class="holiday-name">{html_escape(h['name'])}</div>
            <div class="holiday-date">{html_escape(date_rule_text(rule))}</div>
            <div class="holiday-size">{h['sprite']['width']}×{h['sprite']['height']} px</div>
            <div class="holiday-desc">{html_escape(h.get('description', ''))}</div>
            <div class="holiday-palette">{palette_swatches}</div>
          </div>
          <div class="variants">{''.join(variants_html)}</div>
        </div>''')

    # Originals — show the original 4 with a placeholder note
    origs_html = []
    originals = [h for h in holidays_sorted if h.get("existing_sprite") is not None]
    for h in originals:
        rule = h.get("date_rule", {}) or {}
        origs_html.append(f'''
        <div class="holiday">
          <div class="holiday-meta">
            <div class="holiday-id">id={h['id']} • original sprite (kept)</div>
            <div class="holiday-name">{html_escape(h['name'])}</div>
            <div class="holiday-date">{html_escape(date_rule_text(rule))}</div>
            <div class="holiday-size">{h['sprite']['width']}×{h['sprite']['height']} px • existing_sprite={h.get('existing_sprite')}</div>
          </div>
          <div class="variants"><span class="legend">Existing pixel art preserved (HOLIDAY.PSB index {h.get('existing_sprite')}). No review needed.</span></div>
        </div>''')

    js = """
      // Highlight picked variants and persist to localStorage
      function pickKey() { return 'jc-holiday-picks-v1'; }
      function loadPicks() {
        try { return JSON.parse(localStorage.getItem(pickKey())) || {}; }
        catch (e) { return {}; }
      }
      function savePicks(picks) {
        localStorage.setItem(pickKey(), JSON.stringify(picks));
      }
      function applyPicks(picks) {
        document.querySelectorAll('.variant').forEach(v => v.classList.remove('picked'));
        Object.entries(picks).forEach(([id, vi]) => {
          const r = document.querySelector(`#pick-${id}-${vi}`);
          if (r) {
            r.checked = true;
            r.closest('.variant').classList.add('picked');
          }
        });
        document.getElementById('pick-stat').textContent =
          `${Object.keys(picks).length} / ${document.querySelectorAll('.holiday').length - ${len(originals)}} picks`;
      }
      document.addEventListener('change', (e) => {
        if (e.target.classList.contains('pick-radio')) {
          const picks = loadPicks();
          const id = e.target.name.replace('pick-', '');
          picks[id] = e.target.value;
          savePicks(picks);
          applyPicks(picks);
        }
      });
      document.getElementById('save-btn').addEventListener('click', () => {
        const picks = loadPicks();
        const blob = new Blob([JSON.stringify(picks, null, 2)], {type: 'application/json'});
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = 'holidays-picks.json';
        a.click();
      });
      document.getElementById('clear-btn').addEventListener('click', () => {
        if (confirm('Clear all picks?')) { savePicks({}); applyPicks({}); }
      });
      applyPicks(loadPicks());
    """.replace("${len(originals)}", str(len(originals)))

    full_html = f"""<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<title>Johnny Castaway Holiday Sprite Review</title>
<style>{css}</style>
</head>
<body>
  <div class="controls">
    <h1>🏝️ Holiday Sprite Review</h1>
    <p class="subtitle">31 new US holidays for the PS1 build. Pick one variant per holiday (LITERAL / MINIMALIST / BUSY). Picks persist in localStorage; download as JSON when done.</p>
    <button id="save-btn" class="save-btn">Download picks JSON</button>
    <button id="clear-btn" class="save-btn" style="background:#666;color:#fff;">Clear picks</button>
    <span id="pick-stat" class="stat"></span>
  </div>
  {''.join(cards_html)}
  <h2 class="month-header">Originals (preserved as-is)</h2>
  {''.join(origs_html)}
  <script>{js}</script>
</body>
</html>"""

    HTML_OUT.parent.mkdir(parents=True, exist_ok=True)
    HTML_OUT.write_text(full_html, encoding="utf-8")
    print(f"Wrote {HTML_OUT.relative_to(REPO)}")
    print(f"  {len(new_holidays)} new holidays × 3 variants")
    print(f"  {len(originals)} originals (no review)")
    print(f"Open with: xdg-open {HTML_OUT}")


if __name__ == "__main__":
    main()
