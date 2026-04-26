#!/usr/bin/env python3
"""
Generate `scratch/holidays-preview.html` — a self-contained, single-file
HTML page that renders all 31 new holiday sprites × 5 variants in a
calendar-ordered grid for owner review.

Each cell shows:
  * Holiday name + date rule (computed via holidays-codegen rule kinds)
  * Concept description from holidays.yml
  * Five variant images side-by-side
    (v1 LITERAL / v2 MINIMALIST / v3 BUSY / v4 PLAYFUL / v5 NIGHT)
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


def _easter_sunday(year):
    a, b, c = year % 19, year // 100, year % 100
    d, e = b // 4, b % 4
    f = (b + 8) // 25
    g = (b - f + 1) // 3
    hh = (19 * a + b - d - g + 15) % 30
    i, k = c // 4, c % 4
    ll = (32 + 2 * e + 2 * i - hh - k) % 7
    mm = (a + 11 * hh + 22 * ll) // 451
    month = (hh + ll - 7 * mm + 114) // 31
    day = ((hh + ll - 7 * mm + 114) % 31) + 1
    return month, day


def _nth_weekday(year, month, n, yaml_dow):
    # yaml dow 0=Sun..6=Sat → python calendar 0=Mon..6=Sun
    cal_dow = (yaml_dow - 1) % 7
    import calendar
    cal = calendar.Calendar()
    days = [d for d, w in cal.itermonthdays2(year, month) if d != 0 and w == cal_dow]
    return days[n - 1] if n > 0 else days[n]


def _equinox(year, month):
    return (3, 20) if month == 3 else (9, 22)


def _solstice(year, month):
    return (6, 21) if month == 6 else (12, 21)


def _election(year):
    import calendar
    cal = calendar.Calendar()
    mons = [d for d, w in cal.itermonthdays2(year, 11) if d != 0 and w == 0]
    return (11, mons[0] + 1)


def next_occurrence_text(rule: dict, today=None) -> str | None:
    """Compute the next calendar date (month, day, year) this holiday
    fires on or after `today`. Returns a short string like "Apr 5, 2026"
    or None if the rule is unrecognized."""
    if not rule:
        return None
    import datetime
    today = today or datetime.date.today()
    k = rule.get("kind", "")
    for year in (today.year, today.year + 1):
        try:
            if k == "fixed":
                m, d = rule["month"], rule["day"]
            elif k == "nth_weekday":
                d = _nth_weekday(year, rule["month"], rule["n"], rule["weekday"])
                m = rule["month"]
            elif k == "easter_offset":
                em, ed = _easter_sunday(year)
                tgt = datetime.date(year, em, ed) + datetime.timedelta(days=rule["offset"])
                m, d = tgt.month, tgt.day
                if tgt.year != year:
                    continue
            elif k == "equinox":
                m, d = _equinox(year, rule.get("month", 3))
            elif k == "equinox_vernal":
                m, d = _equinox(year, 3)
            elif k == "equinox_autumnal":
                m, d = _equinox(year, 9)
            elif k == "solstice":
                m, d = _solstice(year, rule.get("month", 6))
            elif k == "solstice_summer":
                m, d = _solstice(year, 6)
            elif k == "solstice_winter":
                m, d = _solstice(year, 12)
            elif k == "election_day":
                m, d = _election(year)
            else:
                return None
        except Exception:
            continue
        try:
            occ = datetime.date(year, m, d)
        except ValueError:
            continue
        if occ >= today:
            return f"{MONTH_NAMES[m]} {d}, {year}"
    return None


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
      h1 { color: var(--accent); margin: 0 0 8px; font-size: 22px; }
      .subtitle { color: var(--muted); margin: 0 0 12px; max-width: 800px; }
      .month-section { margin-top: 32px; }
      .month-header { color: var(--accent); border-bottom: 1px solid var(--panel); padding-bottom: 4px; margin: 24px 0 12px; font-size: 18px; }
      .holiday { background: var(--panel); border-radius: 8px; padding: 12px 16px; margin-bottom: 12px; display: grid; grid-template-columns: 260px 1fr; gap: 16px; align-items: start; transition: opacity 0.2s; }
      .holiday.hidden { display: none; }
      .holiday.has-pick { border-left: 3px solid var(--pick); }
      .holiday-meta { font-size: 13px; }
      .holiday-name { font-size: 16px; color: var(--text); font-weight: 600; }
      .holiday-id { color: var(--muted); font-size: 11px; }
      .holiday-date { color: var(--accent); margin: 4px 0; }
      .holiday-size { color: var(--muted); font-size: 11px; }
      .holiday-desc { color: var(--text); margin-top: 8px; font-size: 12px; line-height: 1.4; font-style: italic; opacity: 0.9; }
      .holiday-palette { display: flex; gap: 4px; margin-top: 6px; }
      .palette-swatch { width: 16px; height: 16px; border: 1px solid #444; border-radius: 2px; }
      .variants { display: flex; gap: 16px; align-items: flex-start; flex-wrap: wrap; }
      .variant { display: flex; flex-direction: column; align-items: center; gap: 4px; cursor: pointer; padding: 6px; border-radius: 6px; transition: background 0.15s; }
      .variant:hover { background: rgba(196, 124, 255, 0.1); }
      .variant.picked { box-shadow: 0 0 0 2px var(--pick); background: rgba(77, 213, 153, 0.15); }
      .variant-label { color: var(--muted); font-size: 11px; text-transform: uppercase; letter-spacing: 0.5px; }
      .variant.picked .variant-label { color: var(--pick); font-weight: 600; }
      .variant.dim { opacity: 0.25; }
      .variant.dim:hover { opacity: 0.85; }
      .variant[data-variant="5"] .variant-label::after { content: " (auto)"; color: #888; font-style: italic; font-size: 9px; }
      .variant-img { background: #000; padding: 4px; border-radius: 4px; image-rendering: pixelated; image-rendering: crisp-edges; max-width: 100%; }
      .pick-radio { margin-top: 4px; cursor: pointer; }
      .kbd { display: inline-block; background: #444; color: #ddd; padding: 1px 6px; border-radius: 3px; font: 11px/1 'SF Mono', Menlo, monospace; border: 1px solid #555; box-shadow: 0 1px 0 #222; margin: 0 1px; }
      .legend-row { color: var(--muted); font-size: 11px; margin-top: 4px; }
      .controls { position: sticky; top: 0; background: var(--bg); padding: 12px 0; border-bottom: 1px solid var(--panel); margin-bottom: 16px; z-index: 10; }
      .controls-row { display: flex; gap: 12px; align-items: center; flex-wrap: wrap; }
      .btn { background: var(--pick); color: #000; padding: 8px 14px; border: 0; border-radius: 4px; font: 600 13px sans-serif; cursor: pointer; transition: filter 0.15s; }
      .btn:hover { filter: brightness(1.1); }
      .btn-ghost { background: transparent; color: var(--text); border: 1px solid var(--panel); }
      .btn-ghost.active { background: var(--accent); color: #000; border-color: var(--accent); }
      .stat { color: var(--muted); margin-left: 8px; font-size: 13px; }
      .stat-value { color: var(--pick); font-weight: 600; }
      .legend { color: var(--muted); font-size: 12px; }
      .missing { color: #f55; padding: 8px 12px; background: #2a1a1a; border-radius: 4px; }
      /* Modal for image zoom */
      .modal { position: fixed; inset: 0; background: rgba(0,0,0,0.85); display: none; align-items: center; justify-content: center; z-index: 100; cursor: zoom-out; }
      .modal.show { display: flex; }
      .modal img { max-width: 80vw; max-height: 80vh; image-rendering: pixelated; image-rendering: crisp-edges; background: #000; padding: 8px; border-radius: 8px; }
      .modal-caption { position: absolute; bottom: 24px; color: white; font-size: 14px; }
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
        VARIANT_LABELS = ["LITERAL", "MINIMALIST", "BUSY", "PLAYFUL", "NIGHT"]
        for vi in (1, 2, 3, 4, 5):
            png_path = ART_DIR / f"{h['id']:02d}-{slug}-v{vi}.png"
            data_uri = png_to_data_uri(png_path)
            if data_uri:
                w = h["sprite"]["width"] * 4
                hh = h["sprite"]["height"] * 4
                variant_label = VARIANT_LABELS[vi - 1]
                variants_html.append(f'''
                <div class="variant" data-id="{h['id']}" data-variant="{vi}">
                  <span class="variant-label">v{vi} {variant_label}</span>
                  <img class="variant-img" src="{data_uri}" width="{w}" height="{hh}" alt="v{vi}" title="Double-click to zoom">
                  <input class="pick-radio" type="radio" name="pick-{h['id']}" value="{vi}" id="pick-{h['id']}-{vi}">
                </div>''')
            elif vi <= 3:
                # Only flag missing for v1-v3 (v4/v5 are alternates).
                variants_html.append(f'''
                <div class="variant"><span class="missing">v{vi} missing<br><small>{html_escape(str(png_path.relative_to(REPO)))}</small></span></div>''')

        palette_swatches = "".join(
            f'<div class="palette-swatch" style="background:{html_escape(c)}" title="{html_escape(c)}"></div>'
            for c in (h.get("palette") or [])
        )

        next_occ = next_occurrence_text(rule)
        date_line = html_escape(date_rule_text(rule))
        if next_occ:
            date_line += f' <span style="color:var(--muted);font-size:11px;">· next: {html_escape(next_occ)}</span>'
        cards_html.append(f'''
        <div class="holiday" id="h{h['id']}">
          <div class="holiday-meta">
            <div class="holiday-id">id={h['id']}</div>
            <div class="holiday-name">{html_escape(h['name'])}</div>
            <div class="holiday-date">{date_line}</div>
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
        next_occ = next_occurrence_text(rule)
        date_line = html_escape(date_rule_text(rule))
        if next_occ:
            date_line += f' <span style="color:var(--muted);font-size:11px;">· next: {html_escape(next_occ)}</span>'
        origs_html.append(f'''
        <div class="holiday">
          <div class="holiday-meta">
            <div class="holiday-id">id={h['id']} • original sprite (kept)</div>
            <div class="holiday-name">{html_escape(h['name'])}</div>
            <div class="holiday-date">{date_line}</div>
            <div class="holiday-size">{h['sprite']['width']}×{h['sprite']['height']} px • existing_sprite={h.get('existing_sprite')}</div>
          </div>
          <div class="variants"><span class="legend">Existing pixel art preserved (HOLIDAY.PSB index {h.get('existing_sprite')}). No review needed.</span></div>
        </div>''')

    js = """
      // Highlight picked variants, persist to localStorage, support filter + modal zoom
      const TOTAL_REVIEWABLE = ${len(new_holidays)};
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
        document.querySelectorAll('.holiday').forEach(c => c.classList.remove('has-pick'));
        Object.entries(picks).forEach(([id, vi]) => {
          const r = document.querySelector(`#pick-${id}-${vi}`);
          if (r) {
            r.checked = true;
            r.closest('.variant').classList.add('picked');
            const card = document.getElementById('h' + id);
            if (card) card.classList.add('has-pick');
          }
        });
        const n = Object.keys(picks).length;
        document.getElementById('pick-stat').innerHTML =
          `<span class="stat-value">${n}</span> / ${TOTAL_REVIEWABLE} picks`;
        applyFilter();
      }
      let filterMode = 'all';
      function applyFilter() {
        document.querySelectorAll('.holiday').forEach(card => {
          if (filterMode === 'all') { card.classList.remove('hidden'); return; }
          const hasPick = card.classList.contains('has-pick');
          const isOriginal = card.querySelector('.holiday-id') &&
                              card.querySelector('.holiday-id').textContent.includes('original');
          if (filterMode === 'unpicked') {
            card.classList.toggle('hidden', hasPick || isOriginal);
          } else if (filterMode === 'picked') {
            card.classList.toggle('hidden', !hasPick || isOriginal);
          } else if (filterMode === 'originals') {
            card.classList.toggle('hidden', !isOriginal);
          }
        });
      }
      // Click handlers
      document.addEventListener('change', (e) => {
        if (e.target.classList.contains('pick-radio')) {
          const picks = loadPicks();
          const id = e.target.name.replace('pick-', '');
          picks[id] = e.target.value;
          savePicks(picks);
          applyPicks(picks);
        }
      });
      // Click variant cell to pick + zoom
      document.addEventListener('click', (e) => {
        const variant = e.target.closest('.variant');
        if (variant && !e.target.classList.contains('pick-radio')) {
          // Toggle the radio
          const radio = variant.querySelector('.pick-radio');
          if (radio) {
            radio.checked = true;
            radio.dispatchEvent(new Event('change', { bubbles: true }));
          }
        }
        // Zoom on image dblclick
        if (e.target.classList.contains('variant-img') && e.detail >= 2) {
          const modal = document.getElementById('zoom-modal');
          modal.querySelector('img').src = e.target.src;
          modal.querySelector('.modal-caption').textContent =
            e.target.closest('.holiday').querySelector('.holiday-name').textContent +
            ' · ' + e.target.closest('.variant').querySelector('.variant-label').textContent;
          modal.classList.add('show');
        }
      });
      // Modal close
      document.getElementById('zoom-modal').addEventListener('click', () => {
        document.getElementById('zoom-modal').classList.remove('show');
      });
      // Save / clear / filter buttons
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
      document.getElementById('load-input').addEventListener('change', (e) => {
        const file = e.target.files && e.target.files[0];
        if (!file) return;
        const reader = new FileReader();
        reader.onload = (ev) => {
          try {
            const parsed = JSON.parse(ev.target.result);
            if (typeof parsed !== 'object' || Array.isArray(parsed)) {
              throw new Error('expected an object');
            }
            // Coerce values to strings for consistency with the radio values.
            const picks = {};
            Object.entries(parsed).forEach(([k, v]) => { picks[String(k)] = String(v); });
            savePicks(picks);
            applyPicks(picks);
          } catch (err) {
            alert('Failed to load picks JSON: ' + err.message);
          }
          // Reset the input so the same file can be re-loaded.
          e.target.value = '';
        };
        reader.readAsText(file);
      });
      document.querySelectorAll('.filter-btn').forEach(btn => {
        btn.addEventListener('click', () => {
          filterMode = btn.dataset.filter;
          document.querySelectorAll('.filter-btn').forEach(b =>
            b.classList.toggle('active', b === btn));
          applyFilter();
        });
      });

      // Variant focus: dim variants except the chosen one (0 = all).
      let focusVariant = 0;
      function applyFocus() {
        document.querySelectorAll('.variant').forEach(v => {
          v.classList.remove('dim');
          if (focusVariant !== 0 && v.dataset.variant &&
              parseInt(v.dataset.variant, 10) !== focusVariant) {
            v.classList.add('dim');
          }
        });
      }
      document.querySelectorAll('.variant-btn').forEach(btn => {
        btn.addEventListener('click', () => {
          focusVariant = parseInt(btn.dataset.variant, 10);
          document.querySelectorAll('.variant-btn').forEach(b =>
            b.classList.toggle('active', b === btn));
          applyFocus();
        });
      });

      // Keyboard navigation. Track an "active card" — the topmost holiday
      // whose top edge is within the viewport.
      function activeCardId() {
        const cards = [...document.querySelectorAll('.holiday[id^="h"]')]
          .filter(c => !c.classList.contains('hidden'));
        const sticky = 120;
        for (const c of cards) {
          const r = c.getBoundingClientRect();
          if (r.bottom > sticky + 80) {
            return c.id.slice(1);
          }
        }
        return cards.length ? cards[0].id.slice(1) : null;
      }
      function scrollToHoliday(id) {
        const c = document.getElementById('h' + id);
        if (c) c.scrollIntoView({behavior:'smooth', block:'start'});
      }
      function nextCard(dir) {
        const cards = [...document.querySelectorAll('.holiday[id^="h"]')]
          .filter(c => !c.classList.contains('hidden'));
        const cur = activeCardId();
        const idx = cards.findIndex(c => c.id === 'h' + cur);
        if (idx < 0) { if (cards.length) scrollToHoliday(cards[0].id.slice(1)); return; }
        const next = cards[(idx + dir + cards.length) % cards.length];
        scrollToHoliday(next.id.slice(1));
      }
      function nextUnpicked() {
        const cards = [...document.querySelectorAll('.holiday[id^="h"]')]
          .filter(c => !c.classList.contains('hidden') && !c.classList.contains('has-pick'));
        if (!cards.length) { return; }
        const cur = activeCardId();
        const idx = cards.findIndex(c => c.id === 'h' + cur);
        const next = cards[(idx + 1) % cards.length] || cards[0];
        scrollToHoliday(next.id.slice(1));
      }
      function setPick(id, vi) {
        const r = document.querySelector(`#pick-${id}-${vi}`);
        if (!r) return;
        r.checked = true;
        r.dispatchEvent(new Event('change', { bubbles: true }));
      }
      const focusOrder = [0, 1, 2, 3, 4, 5];
      function cycleFocus() {
        const idx = focusOrder.indexOf(focusVariant);
        focusVariant = focusOrder[(idx + 1) % focusOrder.length];
        document.querySelectorAll('.variant-btn').forEach(b =>
          b.classList.toggle('active', parseInt(b.dataset.variant, 10) === focusVariant));
        applyFocus();
      }
      document.addEventListener('keydown', (e) => {
        if (e.target.tagName === 'INPUT' || e.target.tagName === 'TEXTAREA') return;
        if (e.metaKey || e.ctrlKey || e.altKey) return;
        const k = e.key;
        if (k === 'j' || k === 'ArrowDown') { e.preventDefault(); nextCard(+1); }
        else if (k === 'k' || k === 'ArrowUp') { e.preventDefault(); nextCard(-1); }
        else if (k === 'u') { e.preventDefault(); nextUnpicked(); }
        else if (k === 'f') { e.preventDefault(); cycleFocus(); }
        else if (k >= '1' && k <= '5') {
          const id = activeCardId();
          if (id) setPick(id, k);
        } else if (k === '?') {
          alert(
            'Keyboard shortcuts:\\n' +
            '  1-5  pick that variant for the holiday in view\\n' +
            '  j / ArrowDown   next holiday\\n' +
            '  k / ArrowUp     previous holiday\\n' +
            '  u    jump to next unpicked\\n' +
            '  f    cycle variant focus (dim others)\\n' +
            '  ?    this help'
          );
        }
      });

      applyPicks(loadPicks());
      applyFocus();
    """.replace("${len(new_holidays)}", str(len(new_holidays)))

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
    <p class="subtitle">31 new US holidays for the PS1 build, 5 variants each (v1 LITERAL · v2 MINIMALIST · v3 BUSY · v4 PLAYFUL · v5 NIGHT). Click a variant to pick it. Double-click a sprite to zoom. Picks persist in localStorage — download as JSON when done.</p>
    <div class="controls-row">
      <button id="save-btn" class="btn">Download picks JSON</button>
      <label class="btn" style="background:#446;color:#fff;cursor:pointer;">
        Load picks JSON
        <input id="load-input" type="file" accept="application/json,.json" style="display:none;">
      </label>
      <button id="clear-btn" class="btn" style="background:#666;color:#fff;">Clear picks</button>
      <span style="margin-left:8px;color:var(--muted);font-size:12px;">Card filter:</span>
      <button class="btn btn-ghost active filter-btn" data-filter="all">All</button>
      <button class="btn btn-ghost filter-btn" data-filter="unpicked">Unpicked</button>
      <button class="btn btn-ghost filter-btn" data-filter="picked">Picked</button>
      <button class="btn btn-ghost filter-btn" data-filter="originals">Originals</button>
      <span id="pick-stat" class="stat"></span>
    </div>
    <div class="controls-row" style="margin-top:6px;">
      <span style="color:var(--muted);font-size:12px;">Variant focus:</span>
      <button class="btn btn-ghost active variant-btn" data-variant="0">All</button>
      <button class="btn btn-ghost variant-btn" data-variant="1">v1 LITERAL</button>
      <button class="btn btn-ghost variant-btn" data-variant="2">v2 MINIMALIST</button>
      <button class="btn btn-ghost variant-btn" data-variant="3">v3 BUSY</button>
      <button class="btn btn-ghost variant-btn" data-variant="4">v4 PLAYFUL</button>
      <button class="btn btn-ghost variant-btn" data-variant="5">v5 NIGHT</button>
    </div>
    <div class="legend-row">
      Keys: <span class="kbd">1</span>–<span class="kbd">5</span> pick variant for the holiday in view ·
      <span class="kbd">j</span>/<span class="kbd">k</span> next/prev holiday ·
      <span class="kbd">f</span> cycle variant focus ·
      <span class="kbd">u</span> jump to next unpicked ·
      <span class="kbd">?</span> help
    </div>
  </div>
  <div id="zoom-modal" class="modal">
    <img alt="">
    <div class="modal-caption"></div>
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
    print(f"  {len(new_holidays)} new holidays × 5 variants")
    print(f"  {len(originals)} originals (no review)")
    print(f"Open with: xdg-open {HTML_OUT}")


if __name__ == "__main__":
    main()
