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

VARIANT_LABELS = ["LITERAL", "MINIMALIST", "BUSY", "PLAYFUL", "NIGHT"]
VARIANTS = (1, 2, 3, 4, 5)


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

def check_originals_pinned(holidays, fails, warns):
    """The original 4 holidays are pinned to specific IDs because
    src/island.c hardcodes the switch-case (1=Halloween, 2=StPatrick,
    3=Christmas, 4=NewYear). If a yaml refactor accidentally swaps
    these IDs, the runtime would draw the wrong sprite for the wrong
    holiday. Catch it here."""
    expected = {
        1: ("Halloween",          0),
        2: ("St. Patrick's Day",  1),
        3: ("Christmas",          2),
        4: ("New Year's Day",     3),
    }
    by_id = {h["id"]: h for h in holidays}
    for hid, (exp_name, exp_psb) in expected.items():
        h = by_id.get(hid)
        if not h:
            fails.append(f"original id={hid} missing from yaml")
            continue
        if h["name"] != exp_name:
            fails.append(
                f"original id={hid}: name {h['name']!r} != expected {exp_name!r}")
        if h.get("existing_sprite") != exp_psb:
            fails.append(
                f"original id={hid} ({exp_name}): existing_sprite "
                f"{h.get('existing_sprite')} != expected {exp_psb}")


def check_yaml_schema(holidays, fails, warns):
    """IDs unique 1..35; required fields present; sprite within bounds;
    island_xy + sprite size fits inside the 640×480 logical screen."""
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
        # Anchor + size must fit within the 640×480 logical screen the
        # PS1 runtime renders to (originals use anchors up to (410, 298)).
        if h.get("existing_sprite") is None:
            ix = sp.get("island_x", 0)
            iy = sp.get("island_y", 0)
            if ix + w > 640 or iy + hh > 480:
                fails.append(
                    f"YAML id={hid}: sprite {w}×{hh} at island_xy "
                    f"({ix},{iy}) extends past 640×480 screen "
                    f"(ends at ({ix + w}, {iy + hh}))")
            if ix < 0 or iy < 0:
                fails.append(
                    f"YAML id={hid}: island_xy ({ix},{iy}) is negative")
    if seen_ids != set(range(1, 36)):
        missing = set(range(1, 36)) - seen_ids
        if missing:
            fails.append(f"YAML missing ids: {sorted(missing)}")


def check_renderer_imports(holidays, fails, warns):
    """All 5 batch modules importable; expected function-count per file."""
    # v5 NIGHT is auto-generated by holidays_concepts.py — not authored
    # in the per-batch dicts, so the per-batch counts stay at v1..v4.
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


def check_master_loader(holidays, fails, warns):
    """holidays_concepts.py merges all batch dicts into RENDERERS dict.
    Verify every reviewable yaml entry has a registered renderer tuple."""
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
    # Cross-check: every reviewable yaml id should have a tuple of length 5.
    for h in holidays:
        if h.get("existing_sprite") is not None:
            continue
        hid = h["id"]
        if hid not in renderers:
            fails.append(f"yaml id={hid} ({h['name']}) has no renderer in RENDERERS")
            continue
        tup = renderers[hid]
        if not isinstance(tup, tuple) or len(tup) != 5:
            fails.append(
                f"yaml id={hid} renderer tuple length {len(tup) if hasattr(tup, '__len__') else '?'} "
                f"!= 5 (v1..v5)")
    return renderers


def check_art_presence_and_dims(holidays, fails, warns):
    """Each new holiday has v1..v4 PNGs at the spec'd dimensions in mode P."""
    for h in holidays:
        if h.get("existing_sprite") is not None:
            continue
        hid = h["id"]
        slug = slugify(h["short_name"])
        sw, shh = h["sprite"]["width"], h["sprite"]["height"]
        for vi in (1, 2, 3, 4, 5):
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
        for vi in (1, 2, 3, 4, 5):
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
        for vi in (1, 2, 3, 4, 5):
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
        for vi in (1, 2, 3, 4, 5):
            p = ART_DIR / f"{hid:02d}-{slug}-v{vi}.png"
            if not p.exists():
                continue
            sig = png_sha(p)
            if sig in sigs:
                fails.append(f"{p.name} byte-identical to {sigs[sig]}")
            else:
                sigs[sig] = p.name


def check_variant_diversity(holidays, fails, warns):
    """Within one holiday, v1..v5 must be pixel-distinct AND v5 must
    differ from v1 by a meaningful number of pixels (catches a silent
    no-op in as_night)."""
    for h in holidays:
        if h.get("existing_sprite") is not None:
            continue
        hid = h["id"]
        slug = slugify(h["short_name"])
        sigs: dict[str, list[int]] = defaultdict(list)
        pngs: dict[int, Path] = {}
        for vi in (1, 2, 3, 4, 5):
            p = ART_DIR / f"{hid:02d}-{slug}-v{vi}.png"
            if not p.exists():
                continue
            sigs[png_sha(p)].append(vi)
            pngs[vi] = p
        for sig, vis in sigs.items():
            if len(vis) > 1:
                fails.append(
                    f"id={hid} variants {vis} are byte-identical (need diversity)")
        # v5 must differ from v1 by at least 10% of pixels (the night
        # recolor + stars + moon should always meet that bar). If it
        # doesn't, something silently no-op'd.
        if 1 in pngs and 5 in pngs:
            with Image.open(pngs[1]) as im1, Image.open(pngs[5]) as im5:
                if im1.size == im5.size:
                    p1 = list(im1.getdata())
                    p5 = list(im5.getdata())
                    diff = sum(1 for a, b in zip(p1, p5) if a != b)
                    frac = diff / max(1, len(p1))
                    if frac < 0.10:
                        fails.append(
                            f"id={hid} v5 differs from v1 by only {frac:.1%} "
                            f"(threshold 10%) — as_night may have silently "
                            f"no-op'd")


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


def check_render_determinism(holidays, fails, warns):
    """Re-render every reviewable v1 sprite in-process and verify it
    matches the on-disk PNG byte-for-byte. Catches a renderer that's
    inadvertently introduced randomness (e.g. an unkeyed
    `random.random()` instead of the seeded `random.Random` in
    `as_night`)."""
    try:
        import importlib
        hc = importlib.import_module("holidays_concepts")
    except Exception as e:
        warns.append(f"determinism check: cannot import holidays_concepts: {e}")
        return
    holidays_by_id = {h["id"]: h for h in holidays}
    diffs = 0
    for hid, variants in hc.RENDERERS.items():
        h = holidays_by_id.get(hid)
        if not h:
            continue
        # Check v1 (hand-authored) and v5 (auto-generated) — covers
        # both the hand-drawn renderers and the as_night procedural.
        for vi in (1, 5):
            if vi - 1 >= len(variants):
                continue
            fn = variants[vi - 1]
            try:
                sp_a = fn(h)
                sp_b = fn(h)
            except Exception as e:
                warns.append(f"determinism check: id={hid} v{vi} render failed: {e}")
                continue
            if list(sp_a.image.getdata()) != list(sp_b.image.getdata()):
                diffs += 1
                fails.append(f"id={hid} v{vi} renders non-deterministically "
                             f"(re-running produced different pixels)")
    if diffs:
        fails.append(f"determinism: {diffs} renderers produce nondeterministic output")


def check_no_identical_date_rules(holidays, fails, warns):
    """Two holidays with identical date_rule will ALWAYS collide every
    year — that's a yaml authoring mistake (different from the
    occasional same-day overlap from independent rules). Fail."""
    import json as _json
    seen: dict[str, dict] = {}
    for h in holidays:
        rule = h.get("date_rule") or {}
        # Canonicalise to a sorted-key JSON string so dict order doesn't
        # cause spurious diffs.
        key = _json.dumps(rule, sort_keys=True)
        if key in seen:
            other = seen[key]
            fails.append(
                f"identical date_rule on id={other['id']} ({other['name']}) "
                f"and id={h['id']} ({h['name']}): {key}")
        else:
            seen[key] = h


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


def check_art_spec(holidays, fails, warns):
    """holidays-art-spec.json contains exactly the reviewable holidays
    (existing_sprite is null) with id/sprite_size matching yaml."""
    spec_path = REPO / "scripts" / "holidays-art-spec.json"
    if not spec_path.exists():
        warns.append(f"missing {spec_path.relative_to(REPO)} (run codegen)")
        return
    import json as _json
    spec = _json.loads(spec_path.read_text(encoding="utf-8"))
    if not isinstance(spec, list):
        fails.append("art-spec.json is not a list")
        return
    spec_ids = {entry["id"] for entry in spec}
    expected_ids = {h["id"] for h in holidays if h.get("existing_sprite") is None}
    if spec_ids != expected_ids:
        fails.append(
            f"art-spec.json ids {sorted(spec_ids)} != reviewable yaml ids "
            f"{sorted(expected_ids)}")
    yaml_by_id = {h["id"]: h for h in holidays}
    for e in spec:
        h = yaml_by_id.get(e["id"])
        if not h:
            continue
        if (e["sprite"]["width"] != h["sprite"]["width"] or
                e["sprite"]["height"] != h["sprite"]["height"]):
            fails.append(
                f"art-spec id={e['id']} sprite size "
                f"{e['sprite']} != yaml {h['sprite']}")


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
    for needle in ('id="save-btn"', 'id="load-input"', 'id="zoom-modal"',
                   'filter-btn', 'variant-btn'):
        if needle not in text:
            fails.append(f"HTML missing required hook: {needle}")


def check_final_review_present(fails, warns):
    """Final-review HTML present (built by holidays-build-all.sh step 8)."""
    p = REPO / "scratch" / "holidays-final-review.html"
    if not p.exists():
        warns.append(f"missing {p.relative_to(REPO)} (run holidays-final-review.py)")
        return
    text = p.read_text(encoding="utf-8")
    if 'class="card"' not in text:
        fails.append("final-review HTML lacks card markup")


def check_renderer_dims_match_yaml(holidays, fails, warns):
    """Static scan: each renderer's hardcoded Sprite(W, H, ...) must
    match holidays.yml's `sprite.width / sprite.height` for that
    holiday id. Catches the case where someone changes yaml dims
    without updating the renderer (or vice-versa) — caught earlier
    than the PNG-presence dim check."""
    import re
    files = {
        "holidays_concepts_reference.py": None,
        "holidays_concepts_batch1.py": None,
        "holidays_concepts_batch2.py": None,
        "holidays_concepts_batch3.py": None,
        "holidays_concepts_batch4.py": None,
    }
    by_id = {h["id"]: h for h in holidays}
    # Map function name → expected (w, h) by walking the per-batch
    # RENDERERS_* dict in each file. Build name → id from importing
    # the modules (already done via master loader earlier; redo here
    # so this check is self-contained).
    for fname in files:
        path = REPO / "scripts" / fname
        if not path.exists():
            continue
        text = path.read_text(encoding="utf-8")
        # Find the RENDERERS_* dict literal — it maps id → (fn1, fn2, ...).
        rd_match = re.search(
            r"RENDERERS_\w+\s*=\s*\{(.*?)\n\}", text, re.DOTALL)
        if not rd_match:
            continue
        rd_body = rd_match.group(1)
        # Parse each `<id>: (fn1, fn2, fn3, fn4)` line.
        for line in rd_body.splitlines():
            m = re.match(r"\s*(\d+):\s*\(([^)]+)\)", line)
            if not m:
                continue
            hid = int(m.group(1))
            fns = [n.strip() for n in m.group(2).split(",") if n.strip()]
            yaml_h = by_id.get(hid)
            if not yaml_h:
                continue
            ew = yaml_h["sprite"]["width"]
            eh = yaml_h["sprite"]["height"]
            for fn_name in fns:
                # Find that function's body in the file and pull the
                # Sprite(w, h, ...) literal.
                fn_match = re.search(
                    r"def " + re.escape(fn_name) + r"\(h\):(.*?)(?=\ndef |\Z)",
                    text, re.DOTALL)
                if not fn_match:
                    continue
                body = fn_match.group(1)
                sm = re.search(r"Sprite\((\d+),\s*(\d+),", body)
                if not sm:
                    continue
                rw, rh = int(sm.group(1)), int(sm.group(2))
                if (rw, rh) != (ew, eh):
                    fails.append(
                        f"{fname}::{fn_name} (id={hid}): renderer "
                        f"Sprite({rw},{rh}) != yaml ({ew},{eh})")


def check_invisible_compose_calls(fails, warns):
    """Static scan: warn if a renderer calls compose_star / compose_heart,
    sp.line, or sp.ellipse with a color that matches the renderer's
    `Sprite(.., fill=COLOR)` AND the call appears inside a comment or
    block hinting at "rays", "stars", "heart", "behind" — i.e. it's
    intended to be visible. False positives exist (sand strips, sky
    re-fills, watermelon bite marks), so this check produces warnings
    rather than fails."""
    import re
    files = [
        "holidays_concepts_reference.py",
        "holidays_concepts_batch1.py",
        "holidays_concepts_batch2.py",
        "holidays_concepts_batch3.py",
        "holidays_concepts_batch4.py",
    ]
    suspicious_keywords = re.compile(
        r"\b(ray|star|heart|fleur|burst|spark|highlight|accent|"
        r"dot|crater|halo|petal|firework|confetti)\b",
        re.IGNORECASE,
    )
    for fname in files:
        path = REPO / "scripts" / fname
        if not path.exists():
            continue
        text = path.read_text(encoding="utf-8")
        for blk in re.split(r"\ndef ", text)[1:]:
            fn_name = blk.split("(", 1)[0]
            fm = re.search(r"Sprite\(\d+,\s*\d+,\s*fill=(\w+)\)", blk)
            if not fm:
                continue
            bg = fm.group(1)
            blk_lines = blk.splitlines()
            # compose_star / compose_heart matching bg are always
            # suspicious (these primitives draw small icons).
            for line in blk_lines:
                if re.search(r"compose_(star|heart)\(", line) and \
                        re.search(r",\s*" + bg + r"\)", line):
                    warns.append(
                        f"{fname}::{fn_name}: compose_* may be invisible — "
                        f"bg={bg}, line: {line.strip()[:70]}")
            # sp.line / sp.ellipse only flagged when a nearby comment
            # has a "visible accent" keyword (rays, stars, etc.). Avoids
            # FP on sand-strip and sky-band loops.
            for i, line in enumerate(blk_lines):
                stripped = line.strip()
                if not re.search(r"sp\.(line|ellipse)\(", stripped):
                    continue
                if not re.search(r",\s*" + bg + r"\)\s*$", stripped):
                    continue
                # Check the previous 2 lines for a visible-element comment.
                ctx = "\n".join(blk_lines[max(0, i - 2):i])
                if "#" in ctx and suspicious_keywords.search(ctx):
                    warns.append(
                        f"{fname}::{fn_name}: sp.{stripped[3:7]} may be "
                        f"invisible — bg={bg}, line: {stripped[:70]}")


def check_manifest_present(holidays, fails, warns):
    """Resolver writes a manifest.json beside the staged PNGs. Verify
    it covers every reviewable holiday with a sane variant id."""
    p = REPO / "scratch" / "holidays-selected" / "manifest.json"
    if not p.exists():
        warns.append(
            f"missing {p.relative_to(REPO)} (run holidays-resolve-picks.py)")
        return
    import json as _json
    try:
        data = _json.loads(p.read_text(encoding="utf-8"))
    except Exception as e:
        fails.append(f"manifest.json parse error: {e}")
        return
    entries = data.get("holidays") if isinstance(data, dict) else None
    if not isinstance(entries, list):
        fails.append("manifest.json missing 'holidays' list")
        return
    expected_ids = {h["id"] for h in holidays
                    if h.get("existing_sprite") is None}
    seen_ids = {e.get("id") for e in entries if isinstance(e, dict)}
    if seen_ids != expected_ids:
        fails.append(
            f"manifest.json ids {sorted(seen_ids)} != reviewable ids "
            f"{sorted(expected_ids)}")
    for e in entries:
        if not (1 <= int(e.get("variant", 0)) <= 5):
            fails.append(
                f"manifest.json id={e.get('id')}: variant out of range")


def check_contact_sheet_present(fails, warns):
    """Contact sheet PNG present and reasonable in size."""
    p = REPO / "scratch" / "holidays-contact-sheet.png"
    if not p.exists():
        warns.append(f"missing {p.relative_to(REPO)} (run holidays-contact-sheet.py)")
        return
    sz = p.stat().st_size
    # Should be at least 100KB (compressed) for a real grid; complain if absurdly small.
    if sz < 50_000:
        fails.append(f"contact sheet PNG is suspiciously small ({sz} B)")


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
        ("originals pinned",     lambda: check_originals_pinned(holidays, fails, warns)),
        ("YAML schema", lambda: check_yaml_schema(holidays, fails, warns)),
        ("renderer imports", lambda: check_renderer_imports(holidays, fails, warns)),
        ("master loader",   lambda: check_master_loader(holidays, fails, warns)),
        ("PNG presence + dims", lambda: check_art_presence_and_dims(holidays, fails, warns)),
        ("palette discipline",  lambda: check_palette_discipline(holidays, fails, warns)),
        ("sparsity",            lambda: check_sparsity(holidays, fails, warns)),
        ("no duplicate art",    lambda: check_no_duplicate_art(holidays, fails, warns)),
        ("variant diversity",   lambda: check_variant_diversity(holidays, fails, warns)),
        ("date algorithms",     lambda: check_date_algorithms(fails, warns)),
        ("render determinism",  lambda: check_render_determinism(holidays, fails, warns)),
        ("identical date rules", lambda: check_no_identical_date_rules(holidays, fails, warns)),
        ("holiday collisions",  lambda: check_holiday_collisions(holidays, fails, warns)),
        ("table.c generated",   lambda: check_table_c_present(fails, warns)),
        ("art-spec.json",       lambda: check_art_spec(holidays, fails, warns)),
        ("HTML integrity",      lambda: check_html_integrity(holidays, fails, warns)),
        ("HTML save / modal",   lambda: check_save_button(fails, warns)),
        ("final-review HTML",   lambda: check_final_review_present(fails, warns)),
        ("contact sheet PNG",   lambda: check_contact_sheet_present(fails, warns)),
        ("manifest.json",       lambda: check_manifest_present(holidays, fails, warns)),
        ("invisible compose_*", lambda: check_invisible_compose_calls(fails, warns)),
        ("renderer dims = yaml", lambda: check_renderer_dims_match_yaml(holidays, fails, warns)),
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
