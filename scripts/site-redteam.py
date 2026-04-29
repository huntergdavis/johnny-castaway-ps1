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
import sys
from html.parser import HTMLParser
from pathlib import Path
from urllib.parse import unquote, urlsplit
from fnmatch import fnmatch


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


class PageParser(HTMLParser):
    def __init__(self) -> None:
        super().__init__(convert_charrefs=True)
        self.links: list[tuple[str, str, str]] = []
        self.ids: set[str] = set()
        self.image_alts: list[tuple[str, str | None]] = []

    def handle_starttag(self, tag: str, attrs: list[tuple[str, str | None]]) -> None:
        attr_map = dict(attrs)
        if "id" in attr_map and attr_map["id"]:
            self.ids.add(attr_map["id"] or "")
        if tag == "a" and attr_map.get("name"):
            self.ids.add(attr_map["name"] or "")
        if tag == "img":
            self.image_alts.append((attr_map.get("src") or "", attr_map.get("alt")))
        for attr in LINK_ATTRS.get(tag, ()):
            value = attr_map.get(attr)
            if value:
                self.links.append((tag, attr, value))


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
