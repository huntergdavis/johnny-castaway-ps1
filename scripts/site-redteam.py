#!/usr/bin/env python3
"""Red-team a generated Johnny Castaway website build.

Checks are intentionally local and boring. Each one is a preventative
gate against a regression class that has either already shipped once
or is cheap enough to lock in even with zero past hits:

- Generated HTML has no raw Liquid tags (`{{` / `{%` leaked through).
- No local filesystem paths (`/home/`, `/Users/`) leaked into output.
- Every local href / static-asset src resolves inside the build dir.
- Every fragment link points at a real `id=""` / `<a name="">` anchor.
- Every `<img>` has non-empty alt text (WCAG 1.1.1).
- Every `<img>` has both `width` and `height` attributes (CLS).
- No empty `<code></code>` elements (Liquid/kramdown content-eating).
- Heading levels never skip downward (WCAG 1.3.1; e.g. h1 → h3).
- Every `id=""` is unique within its page (WCAG 4.1.1).
- Every `<script type="application/ld+json">` block parses as valid JSON.
- Every real content page has a non-empty `<meta name="description">`
  (exempts redirect pages and explicitly-noindex'd surfaces).
- Every real content page has a `<link rel="canonical">` (same
  redirect / noindex exemption rule).
- Every real content page has a `<meta property="og:image">` (same
  redirect / noindex exemption rule) — social cards need it.
- Every `<th>` in rendered output carries a `scope=` attr (WCAG H63
  header-cell association; locked in after the kramdown styled-form
  fix in 452fe5654).
- Every page has a non-empty `<title>` (WCAG 2.4.2 Page Titled).
- Scene pages — meta description's "Validated YYYY-MM-DD" matches
  the first body mention (locked in after the f2deceef7 56-page sweep).
- `/perf/` At-a-glance rollup over_target + target_speed exact
  values match the CSV-computed aggregates (4-decimal precision).
- `/about/status/` headline rollup (public over/speed + raw signed
  over/speed) matches the CSV-computed aggregates.
- `/docs/performance/` reference-manual rollup (same four aggregates,
  different sentence shape) matches the CSV-computed aggregates.
- The /perf/ table rows match the CSV source-of-truth (no drift on
  stats_version / last_run_at / blocking / prefetch / due / vblanks /
  public-capped over_target).

Excluded subtrees (preserved research) are passed via --exclude
glob: typically `ps1/*`, `archive/*`, `general/*`, `readme/*`. Note
that `docs/ps1/performance-scene-matrix.csv` lives inside the excluded
`ps1/*` subtree but is read directly by the /perf/ drift check.
"""

from __future__ import annotations

import argparse
import csv
import json
import re
import sys
from html.parser import HTMLParser
from pathlib import Path
from urllib.parse import unquote, urlsplit
from fnmatch import fnmatch


# Matches <code></code> with optional attributes and whitespace between
# the opening and closing tags. Catches the kramdown / Liquid content-
# eating regression class (e.g. ``{{:toc}}`` silently emitted as empty
# <code></code>).
EMPTY_CODE_RE = re.compile(r"<code(?:\s[^>]*)?>\s*</code>")

# JSON-LD blocks: <script type="application/ld+json">...</script>. The
# site emits 11+ Schema.org record types across ~1300+ blocks; a regression
# that breaks the JSON body (e.g. an unescaped quote in a Liquid-interpolated
# `description` field, or an `{{ }}` mis-emit inside a code-fenced template
# that bleeds into a script block) would silently ship. Each block goes
# through json.loads() — parse failures fail the build.
JSONLD_RE = re.compile(
    r'<script\s+type="application/ld\+json">(.+?)</script>',
    re.DOTALL,
)

# Real content pages must carry a non-empty <meta name="description">
# tag — Google, Bing, social-card crawlers, and AI agents all use it
# when no inline summary is available. The site has it as a frontmatter
# field on every Jekyll page, and head.html threads it through; a
# Liquid breakage or a forgotten frontmatter field would silently ship
# an empty page summary. Synthetic surfaces (redirect pages emitted
# by _layouts/redirect.html, noindex'd auto-generated wrappers under
# /source/, etc.) are exempt — they shouldn't index in the first place.
META_DESC_RE = re.compile(
    r'<meta\s+name="description"\s+content="([^"]*)"',
    re.IGNORECASE,
)
META_REFRESH_RE = re.compile(
    r'<meta\s+http-equiv="refresh"',
    re.IGNORECASE,
)
META_NOINDEX_RE = re.compile(
    r'<meta\s+name="robots"\s+content="[^"]*noindex',
    re.IGNORECASE,
)
CANONICAL_RE = re.compile(
    r'<link\s+rel="canonical"\s+href="[^"]+"',
    re.IGNORECASE,
)
OG_IMAGE_RE = re.compile(
    r'<meta\s+property="og:image"\s+content="[^"]+"',
    re.IGNORECASE,
)
TH_RE = re.compile(r'<th\b[^>]*>', re.IGNORECASE)
TH_SCOPE_RE = re.compile(r'\bscope=', re.IGNORECASE)
TITLE_RE = re.compile(r'<title>([^<]*)</title>', re.IGNORECASE)
VALIDATED_DATE_RE = re.compile(r'Validated\s+(?:on\s+)?(\d{4}-\d{2}-\d{2})')


LINK_ATTRS = {
    "a": ("href",),
    "area": ("href",),
    "audio": ("src",),
    "img": ("src",),
    "link": ("href",),
    "script": ("src",),
    "source": ("src", "srcset"),
    "track": ("src",),
    "video": ("src", "poster"),
}


HEADING_TAGS = ("h1", "h2", "h3", "h4", "h5", "h6")


class PageParser(HTMLParser):
    def __init__(self) -> None:
        super().__init__(convert_charrefs=True)
        self.links: list[tuple[str, str, str]] = []
        self.ids: set[str] = set()
        # Parallel list capturing every id occurrence in source order
        # (including duplicates the ids set silently dedupes). Used
        # downstream for the duplicate-id WCAG 4.1.1 check.
        self.id_occurrences: list[str] = []
        self.image_alts: list[tuple[str, str | None]] = []
        # CLS: every <img> in body content should carry width + height
        # attributes so the browser can reserve layout space before the
        # bytes arrive. Captures (src, has_width, has_height) per img.
        self.image_dims: list[tuple[str, bool, bool]] = []
        # Heading-level sequence captured in source order. Used downstream
        # to flag WCAG 1.3.1 hierarchy skips (e.g. h1 → h3 without h2).
        self.headings: list[int] = []
        self._in_heading: int | None = None
        self._heading_text: list[str] = []
        # Parallel list of heading text snippets so the diagnostic can
        # name the offending heading rather than just its level.
        self.heading_texts: list[str] = []

    def handle_starttag(self, tag: str, attrs: list[tuple[str, str | None]]) -> None:
        attr_map = dict(attrs)
        if "id" in attr_map and attr_map["id"]:
            self.ids.add(attr_map["id"] or "")
            self.id_occurrences.append(attr_map["id"] or "")
        if tag == "a" and attr_map.get("name"):
            self.ids.add(attr_map["name"] or "")
            self.id_occurrences.append(attr_map["name"] or "")
        if tag == "img":
            self.image_alts.append((attr_map.get("src") or "", attr_map.get("alt")))
            self.image_dims.append((
                attr_map.get("src") or "",
                bool(attr_map.get("width")),
                bool(attr_map.get("height")),
            ))
        if tag in HEADING_TAGS:
            self._in_heading = int(tag[1])
            self._heading_text = []
        for attr in LINK_ATTRS.get(tag, ()):
            value = attr_map.get(attr)
            if value:
                self.links.append((tag, attr, value))

    def handle_endtag(self, tag: str) -> None:
        if tag in HEADING_TAGS and self._in_heading is not None:
            self.headings.append(self._in_heading)
            self.heading_texts.append("".join(self._heading_text).strip())
            self._in_heading = None
            self._heading_text = []

    def handle_data(self, data: str) -> None:
        if self._in_heading is not None:
            self._heading_text.append(data)


def split_srcset(value: str) -> list[str]:
    out: list[str] = []
    for item in value.split(","):
        parts = item.strip().split()
        if parts:
            out.append(parts[0])
    return out


def is_external(raw: str) -> bool:
    parsed = urlsplit(raw)
    if parsed.scheme in {"http", "https", "mailto", "tel", "javascript", "data"}:
        return True
    if raw.startswith("//"):
        return True
    return False


def candidate_path(root: Path, page: Path, raw_path: str, baseurls: list[str]) -> Path:
    path = unquote(raw_path)
    for baseurl in sorted((b.rstrip("/") for b in baseurls if b), key=len, reverse=True):
        if path == baseurl:
            path = "/"
            break
        if path.startswith(baseurl + "/"):
            path = path[len(baseurl):] or "/"
            break

    if path.startswith("/"):
        target = root / path.lstrip("/")
    else:
        target = page.parent / path
    return target


def _public_cap_otp(otp_str: str) -> str:
    """Public-display rule on /perf/'s Over Target column: faster-than-
    target rows (negative over_target_percent) render as `0.0%`."""
    if otp_str in ("", "---"):
        return "---"
    try:
        v = float(otp_str)
    except ValueError:
        return otp_str
    return f"{max(0.0, v):.1f}%" if v >= 0 else "0.0%"


def _check_perf_doc_rollup(csv_path: Path, html_path: Path) -> list[str]:
    """Compare /docs/performance/'s reference-manual rollup against the CSV.

    The reference quotes the same four aggregates as /about/status/ but in
    a different sentence shape:
      "...(`X%` exact public over target / `Y%` exact public target
       speed); the raw signed optimization matrix is `Z%` / `W%`."
    Locking parity across the /perf/, /about/status/, and /docs/performance/
    surfaces means hand-typed rollups stay in agreement everywhere.
    """
    out: list[str] = []
    pub_over = 0.0
    pub_speed = 0.0
    raw_over = 0.0
    raw_speed = 0.0
    n = 0
    with csv_path.open(encoding="utf-8") as f:
        for r in csv.DictReader(f):
            otp = r.get("over_target_percent", "").strip()
            lvb = r.get("loop_vb", "").strip()
            tvb = r.get("target_vb", "").strip()
            if not otp or otp == "-" or not lvb or not tvb or lvb == "-" or tvb == "-":
                continue
            try:
                otp_f = float(otp)
                sp_f = float(tvb) / float(lvb) * 100.0
            except (ValueError, ZeroDivisionError):
                continue
            pub_over += max(0.0, otp_f)
            pub_speed += min(100.0, sp_f)
            raw_over += otp_f
            raw_speed += sp_f
            n += 1
    if n == 0:
        return out
    expected = {
        "public_over": f"{pub_over / n:.4f}",
        "public_speed": f"{pub_speed / n:.4f}",
        "raw_over": f"-{abs(raw_over / n):.4f}" if raw_over < 0 else f"+{raw_over / n:.4f}",
        "raw_speed": f"{raw_speed / n:.4f}",
    }
    text = html_path.read_text(encoding="utf-8", errors="replace")
    m = re.search(
        r'<code[^>]*>(\d+\.\d+)%</code>\s*exact\s*public\s*over\s*target'
        r'\s*/\s*<code[^>]*>(\d+\.\d+)%</code>\s*exact\s*public\s*target\s*speed[^<]*\);'
        r'[^<]*raw\s*signed\s*optimization\s*matrix\s*is\s*<code[^>]*>([+-]?\d+\.\d+)%</code>'
        r'\s*/\s*<code[^>]*>(\d+\.\d+)%</code>',
        text, re.DOTALL | re.IGNORECASE,
    )
    if not m:
        return out
    rendered = {
        "public_over": m.group(1),
        "public_speed": m.group(2),
        "raw_over": m.group(3),
        "raw_speed": m.group(4),
    }
    for k, exp in expected.items():
        rend = rendered[k]
        if rend.lstrip("+") != exp.lstrip("+"):
            out.append(
                f"docs/performance/index.html: rollup {k} rendered={rend!r} csv={exp!r}"
            )
    return out


def _check_status_rollup(csv_path: Path, html_path: Path) -> list[str]:
    """Compare /about/status/'s headline rollup against the CSV.

    The page emits a single sentence with four hand-typed exact
    aggregates: public over, public speed (capped at 100), raw
    signed over (no floor), raw signed speed (no ceiling). All
    four come from the same scene-matrix CSV and use the same
    public-cap / no-cap rules as elsewhere on the site.
    """
    out: list[str] = []
    pub_over = 0.0
    pub_speed = 0.0
    raw_over = 0.0
    raw_speed = 0.0
    n = 0
    with csv_path.open(encoding="utf-8") as f:
        for r in csv.DictReader(f):
            otp = r.get("over_target_percent", "").strip()
            lvb = r.get("loop_vb", "").strip()
            tvb = r.get("target_vb", "").strip()
            if not otp or otp == "-" or not lvb or not tvb or lvb == "-" or tvb == "-":
                continue
            try:
                otp_f = float(otp)
                sp_f = float(tvb) / float(lvb) * 100.0
            except (ValueError, ZeroDivisionError):
                continue
            pub_over += max(0.0, otp_f)
            pub_speed += min(100.0, sp_f)
            raw_over += otp_f
            raw_speed += sp_f
            n += 1
    if n == 0:
        return out

    expected = {
        "public_over": f"+{pub_over / n:.4f}",
        "public_speed": f"{pub_speed / n:.4f}",
        "raw_over": f"-{abs(raw_over / n):.4f}" if raw_over < 0 else f"+{raw_over / n:.4f}",
        "raw_speed": f"{raw_speed / n:.4f}",
    }
    text = html_path.read_text(encoding="utf-8", errors="replace")
    # Match the canonical sentence shape:
    # "public ... battle card is `+X%` over target / `Y%` target speed;
    #  the raw signed ... matrix is `-Z%` / `W%`"
    m = re.search(
        r'public[^<]*battle card[^<]*<code[^>]*>([+-]?\d+\.\d+)%</code>'
        r'\s*over\s*target\s*/\s*<code[^>]*>(\d+\.\d+)%</code>\s*target\s*speed[^<]*;'
        r'[^<]*raw\s*signed[^<]*<code[^>]*>([+-]?\d+\.\d+)%</code>'
        r'\s*/\s*<code[^>]*>(\d+\.\d+)%</code>',
        text, re.DOTALL | re.IGNORECASE,
    )
    if not m:
        return out
    rendered = {
        "public_over": m.group(1),
        "public_speed": m.group(2),
        "raw_over": m.group(3),
        "raw_speed": m.group(4),
    }
    for k, exp in expected.items():
        rend = rendered[k]
        if rend.lstrip("+") != exp.lstrip("+"):
            out.append(
                f"about/status/index.html: rollup {k} rendered={rend!r} csv={exp!r}"
            )
    return out


def _check_perf_drift(csv_path: Path, html_path: Path) -> list[str]:
    """Compare /perf/ rendered <tr> rows against the CSV source-of-truth.

    Five fields per row are checked: stats_version, last_run_at,
    blocking_vb, prefetch_overrun_vb, due_misses. The vblanks
    (loop_vb/target_vb) and the public-capped over-target also get
    compared but with format-tolerant whitespace handling. Notes column
    is editorial and intentionally skipped.
    """
    out: list[str] = []
    csv_rows: dict[str, dict[str, str]] = {}
    with csv_path.open(encoding="utf-8") as f:
        for row in csv.DictReader(f):
            key = f"{row['scene_slug']}-{row['tide']}"
            csv_rows[key] = row

    text = html_path.read_text(encoding="utf-8", errors="replace")
    # /perf/ has multiple <tbody> blocks (the rollup table + the main
    # perf table). Match `<tr id="perf-…">` directly across the whole
    # page — that id pattern is unique to per-scene rows and won't
    # collide with any other table on the page.
    tr_re = re.compile(r'<tr id="perf-([^"]+)">(.+?)</tr>', re.DOTALL)
    td_re = re.compile(r"<td[^>]*>(.*?)</td>", re.DOTALL)
    tag_re = re.compile(r"<[^>]+>")

    for tr_match in tr_re.finditer(text):
        rid = tr_match.group(1)
        if rid not in csv_rows:
            out.append(f"perf/index.html: row #{rid} has no matching CSV row")
            continue
        cells = [tag_re.sub("", c).strip() for c in td_re.findall(tr_match.group(2))]
        if len(cells) < 11:
            out.append(f"perf/index.html: row #{rid} has only {len(cells)} cells (<11)")
            continue
        r = csv_rows[rid]
        # Col index: 0 scene, 1 tide, 2 status, 3 last_run, 4 stats_version,
        # 5 over_target, 6 target_speed, 7 vblanks, 8 blocking, 9 prefetch,
        # 10 due, 11 notes
        expected = [
            ("status", r["status"], cells[2]),
            ("last_run_at", r["last_run_at"], cells[3]),
            ("stats_version", r["stats_version"], cells[4]),
            ("over_target_percent", _public_cap_otp(r["over_target_percent"]).replace("0.0%", "0.0%"), cells[5].replace("+", "")),
            ("vblanks", f"{r['loop_vb']}/{r['target_vb']}", cells[7]),
            ("blocking_vb", r["blocking_vb"] or "0", cells[8]),
            ("prefetch_overrun_vb", r["prefetch_overrun_vb"] or "0", cells[9]),
            ("due_misses", r["due_misses"] or "0", cells[10]),
        ]
        for field, csv_val, rendered_val in expected:
            # Skip rows whose CSV has no measured value yet
            if csv_val in ("", "---"):
                continue
            # The Over Target column tolerates +/- prefix variants
            if field == "over_target_percent":
                if rendered_val.lstrip("+-") == csv_val.lstrip("+-"):
                    continue
            if rendered_val != csv_val:
                out.append(
                    f"perf/index.html: row #{rid} {field}: rendered={rendered_val!r} csv={csv_val!r}"
                )

    # Rollup-table parity: the At-a-glance rollup carries two
    # hand-typed exact aggregates that must match what awk would
    # compute from the CSV. Caught a real regression on 2026-05-13
    # (rollup said 0.3224% over target, CSV computed 0.3225%);
    # locking that audit in. The rollup uses the same public-cap
    # rules as the per-row column: over_target floors at 0,
    # target_speed ceils at 100, each averaged across all timing-
    # bearing rows.
    over_sum = 0.0
    speed_sum = 0.0
    n = 0
    for r in csv_rows.values():
        otp = r.get("over_target_percent", "").strip()
        lvb = r.get("loop_vb", "").strip()
        tvb = r.get("target_vb", "").strip()
        if not otp or otp == "-" or not lvb or not tvb or lvb == "-" or tvb == "-":
            continue
        try:
            otp_f = max(0.0, float(otp))
            sp_f = min(100.0, float(tvb) / float(lvb) * 100.0)
        except (ValueError, ZeroDivisionError):
            continue
        over_sum += otp_f
        speed_sum += sp_f
        n += 1
    if n > 0:
        over_csv = f"{over_sum / n:.4f}"
        speed_csv = f"{speed_sum / n:.4f}"
        # Rollup cells use literal `0.3225%` / `99.6839%` strings.
        # Format: "<code>+0.3%</code> (<code>0.3225%</code> exact, public-capped)".
        # The "exact" suffix anchors the precision-4 value.
        rollup_over = re.search(
            r'over target.*?<code[^>]*>(\d+\.\d+)%</code>\s+exact',
            text, re.DOTALL | re.IGNORECASE,
        )
        rollup_speed = re.search(
            r'average target speed.*?<code[^>]*>(\d+\.\d+)%</code>\s+exact',
            text, re.DOTALL | re.IGNORECASE,
        )
        if rollup_over and rollup_over.group(1) != over_csv:
            out.append(
                f"perf/index.html: rollup over_target rendered={rollup_over.group(1)!r}"
                f" csv={over_csv!r}"
            )
        if rollup_speed and rollup_speed.group(1) != speed_csv:
            out.append(
                f"perf/index.html: rollup target_speed rendered={rollup_speed.group(1)!r}"
                f" csv={speed_csv!r}"
            )
    return out


def resolve_target(root: Path, page: Path, raw_path: str, baseurls: list[str]) -> Path | None:
    target = candidate_path(root, page, raw_path, baseurls)
    if target.is_dir():
        index = target / "index.html"
        return index if index.exists() else None
    if target.exists():
        return target
    index = target / "index.html"
    if index.exists():
        return index
    if not target.suffix:
        html = target.with_suffix(".html")
        if html.exists():
            return html
    return None


def is_excluded(path: Path, patterns: list[str]) -> bool:
    rel = path.as_posix()
    return any(fnmatch(rel, pattern) for pattern in patterns)


def check_build(root: Path, baseurls: list[str], require_relative: bool, excludes: list[str]) -> list[str]:
    errors: list[str] = []
    pages: dict[Path, PageParser] = {}

    for html in sorted(root.rglob("*.html")):
        rel = html.relative_to(root)
        if is_excluded(rel, excludes):
            continue
        text = html.read_text(encoding="utf-8", errors="replace")
        parser = PageParser()
        parser.feed(text)
        pages[html] = parser

        if "{{" in text or "{%" in text:
            errors.append(f"{rel}: raw Liquid tag leaked into output")
        if "/home/" in text or "/Users/" in text:
            errors.append(f"{rel}: local filesystem path leaked into output")
        for src, alt in parser.image_alts:
            if alt is None or not alt.strip():
                errors.append(f"{rel}: image missing alt text ({src})")
        # CLS: every <img> in body content needs width + height attrs
        # so the browser can reserve layout space before the bytes
        # arrive. Caught a real regression in ca59c899c (the v0.8.4
        # menu-harness regen dropped width="640" height="448" from all
        # 15 /help/menu/ images) — locked in by c04a5085a.
        for src, has_w, has_h in parser.image_dims:
            if not (has_w and has_h):
                missing = []
                if not has_w: missing.append("width")
                if not has_h: missing.append("height")
                errors.append(
                    f"{rel}: image missing {' + '.join(missing)} attr (CLS): {src}"
                )
        # Empty <code></code> elements almost always indicate a content
        # bug — typically Liquid ate something between backticks (e.g.
        # `{{:toc}}` interpreted as a Liquid tag and emitted nothing) or
        # a markdown processor folded adjacent backticks. Caught a real
        # regression today on /lab/the-site-itself/ where Liquid was
        # silently eating `{{:toc}}` content. Cheap preventative guard.
        if EMPTY_CODE_RE.search(text):
            errors.append(f"{rel}: empty <code></code> element (content lost?)")
        # Every <script type="application/ld+json"> block must parse as
        # valid JSON. The site emits 11 Schema.org record types across
        # 1300+ blocks; a Liquid escaping mistake in a Schema.org field
        # (e.g. an unescaped backtick in a description, an unquoted
        # interpolation that emits a stray brace) silently breaks the
        # structured-data signal for crawlers and the page still
        # renders. Catch parse failures at build time.
        for idx, jsonld in enumerate(JSONLD_RE.findall(text), start=1):
            try:
                json.loads(jsonld)
            except json.JSONDecodeError as e:
                errors.append(
                    f"{rel}: JSON-LD block #{idx} parse error at "
                    f"line {e.lineno} col {e.colno}: {e.msg}"
                )
        # WCAG 1.3.1 (Info and Relationships) — heading levels must not
        # skip down. A page that goes from <h1> directly to <h3> hides
        # the intermediate structure from screen readers walking the
        # outline. Allowed: any descending step, or a level repeat, or
        # ascending by exactly one. Forbidden: ascending by more than
        # one. Templates emit one <h1> per page (layout-level), so a
        # skip-down from the first body heading is the typical failure
        # mode — author wrote `### Foo` where `## Foo` was intended.
        prev_level: int | None = None
        for level, htext in zip(parser.headings, parser.heading_texts):
            if prev_level is not None and level > prev_level + 1:
                preview = htext[:60] or "(empty heading)"
                errors.append(
                    f"{rel}: heading skip h{prev_level}->h{level}: '{preview}'"
                )
            prev_level = level
        # WCAG 4.1.1 (Parsing) / Schema.org URL fragments — id values
        # must be unique within a page. Two elements sharing the same
        # id silently break in-page anchor jumps (browser jumps to
        # the first occurrence; cross-refs to other instances can't
        # disambiguate). Site-wide audit on 2026-05-12 found zero
        # duplicates — this check locks that in.
        seen_ids: set[str] = set()
        for id_val in parser.id_occurrences:
            if id_val in seen_ids:
                errors.append(f"{rel}: duplicate id='{id_val}'")
            else:
                seen_ids.add(id_val)
        # SEO / discoverability — every real content page must carry a
        # non-empty <meta name="description"> tag. Google + Bing + AI
        # crawlers fall back to it when no inline summary is available;
        # the social-card OG description tag also derives from it via
        # head.html's `page.description | default: site.description`
        # template. Silent regression class: a forgotten frontmatter
        # field would ship a page with empty social cards and weaker
        # search snippets. Exempt synthetic surfaces: redirect pages
        # (have <meta http-equiv="refresh">, won't be indexed anyway)
        # and explicitly-noindex'd surfaces (have <meta name="robots"
        # content="noindex"> — the /source/* shelf, regtest case-shelf
        # entries, resources catalog). Site-wide audit on 2026-05-12
        # confirmed all real pages have a non-empty description; this
        # locks that in.
        if not META_REFRESH_RE.search(text) and not META_NOINDEX_RE.search(text):
            m = META_DESC_RE.search(text)
            if m is None or not m.group(1).strip():
                errors.append(f"{rel}: missing or empty meta name=\"description\"")
        # SEO deduplication — every real content page must carry a
        # <link rel="canonical"> with an absolute URL. Search engines
        # use it to consolidate duplicate-content surfaces (the same
        # page reachable at multiple URLs, e.g. with and without
        # trailing slash, with and without query string). The site's
        # head.html emits the canonical via `site.url + canonical_baseurl
        # + page.url`; a Liquid breakage or a future template refactor
        # could silently drop it. Same exempt-synthetic-surfaces rule
        # as the description check above (redirect pages are
        # themselves duplicates, noindex'd surfaces shouldn't claim
        # canonical authority). Site-wide audit on 2026-05-12 found
        # zero violations; this check locks that in.
        if not META_REFRESH_RE.search(text) and not META_NOINDEX_RE.search(text):
            if not CANONICAL_RE.search(text):
                errors.append(f"{rel}: missing <link rel=\"canonical\">")
        # Social-card image — every real content page must carry a
        # non-empty <meta property="og:image"> tag. Without it,
        # Facebook / LinkedIn / Slack / Discord / Twitter / Mastodon
        # cards fall back to text-only previews (or, worse, scrape a
        # random first image from the body). The site's head.html
        # emits og:image via `page.image | default: site.image`, so
        # a forgotten frontmatter field falls through to the default
        # site OG image — but a Liquid breakage in the template would
        # silently drop the tag entirely. Same redirect / noindex
        # exemption rule. Site-wide audit on 2026-05-12 confirmed
        # 612+ pages all carry og:image with valid asset URLs; this
        # check locks that in.
        if not META_REFRESH_RE.search(text) and not META_NOINDEX_RE.search(text):
            if not OG_IMAGE_RE.search(text):
                errors.append(f"{rel}: missing <meta property=\"og:image\">")
        # WCAG H63 (Using th element to identify row and column
        # headers) — every <th> in the rendered output must declare
        # `scope` so screen readers and table-summarization tools
        # correctly associate each header cell with the column / row
        # of data it describes. The build pipeline post-processes
        # kramdown's bare <th> and <th style="..."> emissions into
        # <th scope="col"> via two perl substitutions in
        # site-build-static-root.sh (commits 0aa242991 / 5796b7569
        # / 452fe5654); this check locks in that both passes ran.
        # The third site-wide audit on 2026-05-12 confirmed all 1124
        # <th> tags carry scope across the non-research tree.
        for m in TH_RE.finditer(text):
            if not TH_SCOPE_RE.search(m.group(0)):
                errors.append(f"{rel}: <th> missing scope= attr: {m.group(0)}")
        # WCAG 2.4.2 (Page Titled) — every page needs a non-empty
        # <title>. Browser tabs, history entries, search results, and
        # screen-reader page-identification announcements all derive
        # from it; an empty or missing <title> degrades all four
        # surfaces silently. head.html builds it via `{% if page.title %}
        # {{ page.title | escape }} — {{ site.title | escape }}`,
        # defaulting to the site title alone for the home page. A
        # Liquid breakage or a forgotten frontmatter `title:` would
        # silently drop it. Site-wide audit on 2026-05-12 confirmed
        # 615 / 615 pages have non-empty titles. Redirect pages get
        # "Redirecting…" via the redirect layout and don't need a
        # separate exemption — they DO have a non-empty title — but
        # the noindex / refresh skip stays in place for consistency
        # with the other meta-tag checks.
        m = TITLE_RE.search(text)
        if m is None or not m.group(1).strip():
            errors.append(f"{rel}: missing or empty <title>")
        # Scene pages — meta description's "Validated YYYY-MM-DD"
        # must match the first body mention. Both are hand-written
        # in the same source file (the description in frontmatter,
        # the body in the markdown) so they can drift if an editor
        # touches one without the other. Site-wide audit on
        # 2026-05-12 caught 56 such drifts (mostly from the v0.8.4
        # chapter-select grind regenerating descriptions without
        # touching the body validation date); locked in by f2deceef7.
        # This check makes any future re-drift fail the build.
        # Scoped to /scenes/<slug>/ pages — the scene-page layout
        # is the only surface that emits the "Validated YYYY-MM-DD"
        # phrase in both description and body.
        rel_str = rel.as_posix()
        if rel_str.startswith("scenes/") and rel_str.endswith("/index.html") and rel_str != "scenes/index.html":
            m_desc = META_DESC_RE.search(text)
            if m_desc:
                dm_desc = VALIDATED_DATE_RE.search(m_desc.group(1))
                # Find body Validated — exclude the description meta
                # tag itself by stripping it before scanning.
                body = META_DESC_RE.sub("", text, count=1)
                dm_body = VALIDATED_DATE_RE.search(body)
                if dm_desc and dm_body and dm_desc.group(1) != dm_body.group(1):
                    errors.append(
                        f"{rel}: Validated date mismatch — "
                        f"description={dm_desc.group(1)} body={dm_body.group(1)}"
                    )

    # /perf/ row freshness vs CSV. The /perf/ table's <tr> rows are
    # hand-written HTML; the CSV at docs/ps1/performance-scene-matrix.csv
    # is the durable numeric source. Drift between the two is silent —
    # caught a real regression on 2026-05-12 (building2-high stats_version
    # / last_run_at / blocking_vb / prefetch_overrun_vb stayed at the
    # pre-rg249-257 baseline after the CSV updated). Locking that audit
    # in. Skips gracefully if either file is missing.
    perf_csv = root / "ps1" / "performance-scene-matrix.csv"
    perf_html = root / "perf" / "index.html"
    if perf_csv.exists() and perf_html.exists():
        errors.extend(_check_perf_drift(perf_csv, perf_html))

    # /about/status/ headline carries the same four perf-rollup
    # aggregate values (public over, public speed, raw signed over,
    # raw signed speed) — hand-maintained and prone to the same drift
    # as the /perf/ rollup. Caught a real regression on 2026-05-13
    # (~0.0025% drift across all four values; fixed in 7ba495a50).
    # Locking that audit in. Skips gracefully if either file is
    # missing.
    status_html = root / "about" / "status" / "index.html"
    if perf_csv.exists() and status_html.exists():
        errors.extend(_check_status_rollup(perf_csv, status_html))

    # /docs/performance/ reference manual quotes the same four
    # aggregates in a different sentence shape. Caught a stale
    # `0.3224%` (twice) + `-0.4446%` on 2026-05-13; fixed in
    # 5f666f6bc. Locking that audit in across the third hand-typed
    # rollup surface on the site.
    perf_doc_html = root / "docs" / "performance" / "index.html"
    if perf_csv.exists() and perf_doc_html.exists():
        errors.extend(_check_perf_doc_rollup(perf_csv, perf_doc_html))

    for html, parser in pages.items():
        rel = html.relative_to(root)
        for tag, attr, value in parser.links:
            values = split_srcset(value) if attr == "srcset" else [value]
            for raw in values:
                raw = raw.strip()
                if not raw or is_external(raw):
                    continue
                parsed = urlsplit(raw)
                if require_relative and parsed.path.startswith("/"):
                    errors.append(f"{rel}: local URL is not relative -> {raw}")
                if parsed.path == "":
                    if parsed.fragment and parsed.fragment not in parser.ids:
                        errors.append(f"{rel}: missing local anchor #{parsed.fragment}")
                    continue
                target = resolve_target(root, html, parsed.path, baseurls)
                if target is None:
                    errors.append(f"{rel}: broken {tag}[{attr}] -> {raw}")
                    continue
                if parsed.fragment and target.suffix == ".html":
                    target_parser = pages.get(target)
                    if target_parser and parsed.fragment not in target_parser.ids:
                        errors.append(f"{rel}: missing anchor {raw}")

    return errors


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("root", type=Path, help="Generated site root, for example site/_site or www")
    ap.add_argument("--baseurl", action="append", default=[], help="Base URL prefix to strip before resolving local links")
    ap.add_argument("--exclude", action="append", default=[], help="Relative glob to skip inside the generated root")
    ap.add_argument("--require-relative", action="store_true", help="Fail root-relative local links")
    args = ap.parse_args()

    root = args.root.resolve()
    if not root.exists():
        print(f"site-redteam: missing build root {root}", file=sys.stderr)
        return 2

    errors = check_build(root, args.baseurl, args.require_relative, args.exclude)
    if errors:
        for err in errors[:200]:
            print(f"FAIL {err}")
        if len(errors) > 200:
            print(f"... {len(errors) - 200} more failures")
        return 1

    html_count = sum(1 for _ in root.rglob("*.html"))
    print(f"site-redteam: OK ({html_count} html files checked under {root})")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
