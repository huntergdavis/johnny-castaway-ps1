#!/usr/bin/env python3
"""Red-team a generated Johnny Castaway website build.

Checks are intentionally local and boring:
- generated HTML has no raw Liquid tags or workstation paths
- local links and static assets resolve inside the build directory
- fragment links point at real ids/named anchors
- images have alt text
"""

from __future__ import annotations

import argparse
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
