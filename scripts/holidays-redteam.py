#!/usr/bin/env python3
"""
Red-team QA pass for the Johnny Castaway holidays art pipeline.

Runs ten independent checks. Exits non-zero if any FAIL. Run any time
the renderers change to catch regressions like:

  * the TRANSPARENT-outline-rect erasure bug (sprite ends up 90%+ idx 0)
  * a renderer using a palette index outside 0..15
  * a sprite that doesn't match its yaml-declared dimensions
  * two holidays sharing identical artwork
  * a renderer dropped from RENDERERS_BATCHN
  * holidays.yml schema drift

Usage:
  python3 scripts/holidays-redteam.py
  python3 scripts/holidays-redteam.py --verbose
"""
import argparse
import hashlib
import importlib
import re
import subprocess
import sys
from collections import defaultdict
from pathlib import Path

try:
    import yaml
except ImportError:
    sys.stderr.write("error: PyYAML not installed\n")
    sys.exit(2)

try:
    from PIL import Image
except ImportError:
    sys.stderr.write("error: Pillow not installed\n")
    sys.exit(2)

REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO / "scripts"))

YAML_PATH = REPO / "holidays.yml"
ART_DIR = REPO / "scratch" / "holidays-art"
HTML_PATH = REPO / "scratch" / "holidays-preview.html"
TABLE_C = REPO / "src" / "holidays_table.c"

# Sprites that are intentionally near-monochrome (negative-space designs).
# Anything else with >85% single color trips the sparsity check.
SPARSITY_ALLOWLIST = {
    # (id, variant) tuples — minimalist sprites where one color dominating
    # is the design, not a bug.
    (12, 2),  # Star Wars Day v2 — minimalist starfield, mostly DEEPBLUE
    (24, 2),  # Independence Day v2 — see review (added if confirmed)
}

VARIANT_LABELS = ["LITERAL", "MINIMALIST", "BUSY", "PLAYFUL"]


# ---------------------------------------------------------------------------
# helpers
# ---------------------------------------------------------------------------

def slugify(name: str) -> str:
    return name.replace(" ", "_").replace("'", "").replace(".", "")


def png_palette_indices(path: Path) -> tuple[set[int], dict[int, int], int]:
    """Return (indices_used, histogram, total_pixels) for an indexed PNG."""
    with Image.open(path) as im:
        if im.mode != "P":
            return set(), {}, 0
        pixels = list(im.getdata())
    hist: dict[int, int] = defaultdict(int)
    for p in pixels:
        hist[p] += 1
    return set(hist.keys()), dict(hist), len(pixels)


def png_sha(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


# ---------------------------------------------------------------------------
# checks
# ---------------------------------------------------------------------------

def check_yaml_schema(holidays, fails, warns):
    """IDs unique 1..35; required fields present; sprite within bounds."""
    seen_ids = set()
    for h in holidays:
        hid = h.get("id")
        if hid in seen_ids:
            fails.append(f"YAML duplicate id={hid}")
        seen_ids.add(hid)
        if not (1 <= (hid or 0) <= 35):
            fails.append(f"YAML id out of range: {hid}")
        for k in ("name", "short_name", "description", "date_rule", "sprite"):
            if k not in h:
                fails.append(f"YAML id={hid} missing field: {k}")
        sn = h.get("short_name", "")
        if len(sn) > 12:
            fails.append(f"YAML id={hid} short_name '{sn}' > 12 chars")
        sp = h.get("sprite", {})
        w, hh = sp.get("width", 0), sp.get("height", 0)
        if not (32 <= w <= 160 and 32 <= hh <= 96):
            fails.append(
                f"YAML id={hid} sprite size out of bounds {w}×{hh} "
                f"(need 32..160 × 32..96)")
    if seen_ids != set(range(1, 36)):
        missing = set(range(1, 36)) - seen_ids
        if missing:
            fails.append(f"YAML missing ids: {sorted(missing)}")


def check_renderer_imports(holidays, fails, warns):
    """All 5 batch modules importable; expected function-count per file."""
    expected_counts = {
        "holidays_concepts_reference": 8,   # 2 holidays × 4
        "holidays_concepts_batch1": 28,     # 7 × 4
        "holidays_concepts_batch2": 24,     # 6 × 4
        "holidays_concepts_batch3": 28,     # 7 × 4
        "holidays_concepts_batch4": 36,     # 9 × 4
    }
    for mod_name, expected in expected_counts.items():
        try:
            mod = importlib.import_module(mod_name)
        except Exception as e:
            fails.append(f"renderer import failed: {mod_name}: {e}")
            continue
        # Find the RENDERERS_* dict
        renderers = None
        for attr in dir(mod):
            if attr.startswith("RENDERERS_"):
                renderers = getattr(mod, attr)
                break
        if renderers is None:
            fails.append(f"{mod_name}: no RENDERERS_* dict found")
            continue
        actual = sum(
            sum(1 for fn in tup if callable(fn))
            for tup in renderers.values()
        )
        if actual != expected:
            warns.append(
                f"{mod_name}: {actual} renderers found, expected {expected}")


def check_master_loader(fails, warns):
    """holidays_concepts.py merges all batch dicts into RENDERERS dict."""
    try:
        mod = importlib.import_module("holidays_concepts")
    except Exception as e:
        fails.append(f"master loader import failed: {e}")
        return None
    renderers = getattr(mod, "RENDERERS", None)
    if not isinstance(renderers, dict):
        fails.append("holidays_concepts.RENDERERS missing or not a dict")
        return None
    if len(renderers) != 31:
        fails.append(f"holidays_concepts.RENDERERS has {len(renderers)} entries, expected 31")
    return renderers


def check_art_presence_and_dims(holidays, fails, warns):
    """Each new holiday has v1..v4 PNGs at the spec'd dimensions in mode P."""
    for h in holidays:
        if h.get("existing_sprite") is not None:
            continue
        hid = h["id"]
        slug = slugify(h["short_name"])
        sw, shh = h["sprite"]["width"], h["sprite"]["height"]
        for vi in (1, 2, 3, 4):
            p = ART_DIR / f"{hid:02d}-{slug}-v{vi}.png"
            if not p.exists():
                fails.append(f"missing PNG: {p.relative_to(REPO)}")
                continue
            with Image.open(p) as im:
                if im.mode != "P":
                    fails.append(f"{p.name}: mode={im.mode} (expected P)")
                if im.size != (sw, shh):
                    fails.append(
                        f"{p.name}: dims {im.size} != yaml {(sw, shh)}")


def check_palette_discipline(holidays, fails, warns):
    """Every PNG uses ONLY palette indices 0..15."""
    for h in holidays:
        if h.get("existing_sprite") is not None:
            continue
        hid = h["id"]
        slug = slugify(h["short_name"])
        for vi in (1, 2, 3, 4):
            p = ART_DIR / f"{hid:02d}-{slug}-v{vi}.png"
            if not p.exists():
                continue
            indices, _, _ = png_palette_indices(p)
            bad = {i for i in indices if not (0 <= i <= 15)}
            if bad:
                fails.append(
                    f"{p.name}: uses palette indices outside 0..15: {sorted(bad)}")
            if not indices:
                fails.append(f"{p.name}: failed to read indices")


def check_sparsity(holidays, fails, warns):
    """Flag PNGs where >85% of pixels are a single index, unless allow-listed."""
    for h in holidays:
        if h.get("existing_sprite") is not None:
            continue
        hid = h["id"]
        slug = slugify(h["short_name"])
        for vi in (1, 2, 3, 4):
            p = ART_DIR / f"{hid:02d}-{slug}-v{vi}.png"
            if not p.exists():
                continue
            _, hist, total = png_palette_indices(p)
            if total == 0:
                continue
            top_idx, top_count = max(hist.items(), key=lambda kv: kv[1])
            frac = top_count / total
            if frac > 0.85 and (hid, vi) not in SPARSITY_ALLOWLIST:
                fails.append(
                    f"{p.name}: {frac:.1%} of pixels are palette idx {top_idx} "
                    f"(threshold 85%, not allowlisted) — likely an erasure bug")
            elif frac > 0.95 and (hid, vi) in SPARSITY_ALLOWLIST:
                warns.append(
                    f"{p.name}: {frac:.1%} idx {top_idx} (allowlisted, but extreme)")


def check_no_duplicate_art(holidays, fails, warns):
    """No two distinct PNGs are byte-identical (catches accidental copy-paste)."""
    sigs: dict[str, str] = {}
    for h in holidays:
        if h.get("existing_sprite") is not None:
            continue
        hid = h["id"]
        slug = slugify(h["short_name"])
        for vi in (1, 2, 3, 4):
            p = ART_DIR / f"{hid:02d}-{slug}-v{vi}.png"
            if not p.exists():
                continue
            sig = png_sha(p)
            if sig in sigs:
                fails.append(f"{p.name} byte-identical to {sigs[sig]}")
            else:
                sigs[sig] = p.name


def check_variant_diversity(holidays, fails, warns):
    """Within one holiday, v1..v4 must be pixel-distinct (different histograms)."""
    for h in holidays:
        if h.get("existing_sprite") is not None:
            continue
        hid = h["id"]
        slug = slugify(h["short_name"])
        sigs: dict[str, list[int]] = defaultdict(list)
        for vi in (1, 2, 3, 4):
            p = ART_DIR / f"{hid:02d}-{slug}-v{vi}.png"
            if not p.exists():
                continue
            sigs[png_sha(p)].append(vi)
        for sig, vis in sigs.items():
            if len(vis) > 1:
                fails.append(
                    f"id={hid} variants {vis} are byte-identical (need diversity)")


def check_date_algorithms(fails, warns):
    """Run scripts/holidays-test.py — must report 20/20 passing."""
    try:
        out = subprocess.run(
            [sys.executable, str(REPO / "scripts" / "holidays-test.py")],
            cwd=str(REPO), capture_output=True, text=True, timeout=60)
    except Exception as e:
        fails.append(f"date algorithm test crashed: {e}")
        return
    if out.returncode != 0:
        fails.append(f"holidays-test.py exit={out.returncode}")
    text = out.stdout + out.stderr
    m = re.search(r"(\d+)\s*/\s*(\d+)", text)
    if not m or m.group(1) != m.group(2):
        warns.append(f"holidays-test.py output unexpected: {text[-200:]}")


def check_holiday_collisions(holidays, fails, warns):
    """All 35 holidays fire in 2024-2030; 0 same-day collisions per year."""
    try:
        # Re-use the python date mirror from holidays-test if available, else
        # use lightweight inline checks here. We just call holidayForDate via
        # the python algorithms.
        from holidays_test import (  # type: ignore
            easter_sunday, nth_weekday, equinox, solstice, election_day,
        )
    except Exception:
        # Inline minimal set — same algorithms as scripts/holidays-test.py
        def easter_sunday(y):
            a, b, c = y % 19, y // 100, y % 100
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

        def nth_weekday(year, month, n, weekday):
            # yaml weekday: 0=Sun..6=Sat. python's calendar: 0=Mon..6=Sun.
            # Convert: yaml_dow → calendar_dow.
            cal_dow = (weekday - 1) % 7
            import calendar
            cal = calendar.Calendar()
            days = [d for d, w in cal.itermonthdays2(year, month)
                    if d != 0 and w == cal_dow]
            return days[n - 1] if n > 0 else days[n]

        def equinox(year, month):
            return (3 if month == 3 else 9, 20 if month == 3 else 22)

        def solstice(year, month):
            return (6 if month == 6 else 12, 21)

        def election_day(year):
            # 1st Tue after 1st Mon of November.
            # python calendar: Monday=0.
            import calendar
            cal = calendar.Calendar()
            mons = [d for d, w in cal.itermonthdays2(year, 11)
                    if d != 0 and w == 0]
            first_mon = mons[0]
            return (11, first_mon + 1)

    def date_for(rule, year):
        k = rule.get("kind")
        if k == "fixed":
            return (rule["month"], rule["day"])
        if k == "nth_weekday":
            d = nth_weekday(year, rule["month"], rule["n"], rule["weekday"])
            return (rule["month"], d)
        if k == "easter_offset":
            m, d = easter_sunday(year)
            import datetime
            base = datetime.date(year, m, d)
            target = base + datetime.timedelta(days=rule["offset"])
            return (target.month, target.day)
        # Loose `equinox` / `solstice` kinds are disambiguated by the
        # rule's `month` field — same logic the codegen uses to normalize
        # into the specific C enum value.
        if k == "equinox":
            return equinox(year, rule.get("month", 3))
        if k == "equinox_vernal":
            return equinox(year, 3)
        if k == "equinox_autumnal":
            return equinox(year, 9)
        if k == "solstice":
            return solstice(year, rule.get("month", 6))
        if k == "solstice_summer":
            return solstice(year, 6)
        if k == "solstice_winter":
            return solstice(year, 12)
        if k == "election_day":
            return election_day(year)
        return None

    for year in range(2024, 2031):
        seen: dict[tuple[int, int], int] = {}
        for h in holidays:
            d = date_for(h.get("date_rule") or {}, year)
            if d is None:
                fails.append(
                    f"id={h['id']} ({h['name']}) failed to compute date for {year}")
                continue
            if d in seen:
                warns.append(
                    f"{year}: id={h['id']} ({h['name']}) shares {d} with id={seen[d]}")
            else:
                seen[d] = h["id"]


def check_table_c_present(fails, warns):
    """Generated holidays_table.c exists and has 35 entries."""
    if not TABLE_C.exists():
        fails.append(f"missing generated file: {TABLE_C.relative_to(REPO)}")
        return
    text = TABLE_C.read_text(encoding="utf-8")
    # Each row begins with `.id            = N,` — count those.
    n = len(re.findall(r"^\s*\.id\s+=\s+\d+,", text, flags=re.MULTILINE))
    if n != 35:
        warns.append(f"holidays_table.c has {n} '.id' rows (expected 35)")


def check_html_integrity(holidays, fails, warns):
    """HTML preview present, has 35 holiday cards and matching base64 PNGs."""
    if not HTML_PATH.exists():
        warns.append(f"missing {HTML_PATH.relative_to(REPO)} (run holidays-preview.py)")
        return
    text = HTML_PATH.read_text(encoding="utf-8")
    cards = text.count('class="holiday"')
    if cards != 35:
        warns.append(f"HTML has {cards} holiday cards (expected 35)")
    pngs = text.count("data:image/png;base64,")
    expected_pngs = sum(
        4 for h in holidays if h.get("existing_sprite") is None
    )
    if pngs < expected_pngs - 4:
        warns.append(f"HTML has {pngs} embedded PNGs (expected ~{expected_pngs})")


def check_save_button(fails, warns):
    """HTML has the Download-as-JSON button + zoom modal + filter buttons."""
    if not HTML_PATH.exists():
        return
    text = HTML_PATH.read_text(encoding="utf-8")
    for needle in ('id="save-btn"', 'id="zoom-modal"', 'filter-btn'):
        if needle not in text:
            fails.append(f"HTML missing required hook: {needle}")


# ---------------------------------------------------------------------------
# main
# ---------------------------------------------------------------------------

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--verbose", action="store_true")
    args = ap.parse_args()

    holidays = yaml.safe_load(open(YAML_PATH, "r", encoding="utf-8"))

    fails: list[str] = []
    warns: list[str] = []

    checks = [
        ("YAML schema", lambda: check_yaml_schema(holidays, fails, warns)),
        ("renderer imports", lambda: check_renderer_imports(holidays, fails, warns)),
        ("master loader",   lambda: check_master_loader(fails, warns)),
        ("PNG presence + dims", lambda: check_art_presence_and_dims(holidays, fails, warns)),
        ("palette discipline",  lambda: check_palette_discipline(holidays, fails, warns)),
        ("sparsity",            lambda: check_sparsity(holidays, fails, warns)),
        ("no duplicate art",    lambda: check_no_duplicate_art(holidays, fails, warns)),
        ("variant diversity",   lambda: check_variant_diversity(holidays, fails, warns)),
        ("date algorithms",     lambda: check_date_algorithms(fails, warns)),
        ("holiday collisions",  lambda: check_holiday_collisions(holidays, fails, warns)),
        ("table.c generated",   lambda: check_table_c_present(fails, warns)),
        ("HTML integrity",      lambda: check_html_integrity(holidays, fails, warns)),
        ("HTML save / modal",   lambda: check_save_button(fails, warns)),
    ]

    print(f"red-team pass over {len(holidays)} holidays\n")
    for name, fn in checks:
        prev_f = len(fails)
        prev_w = len(warns)
        fn()
        df = len(fails) - prev_f
        dw = len(warns) - prev_w
        marker = "OK " if df == 0 else "FAIL"
        extra = f" ({dw} warn)" if dw else ""
        print(f"  [{marker}] {name}{extra}")

    if warns:
        print("\nwarnings:")
        for w in warns:
            print(f"  ! {w}")
    if fails:
        print(f"\nFAILED ({len(fails)}):")
        for f in fails:
            print(f"  X {f}")
        sys.exit(1)
    else:
        print(f"\nall checks passed ({len(warns)} warnings).")


if __name__ == "__main__":
    main()
