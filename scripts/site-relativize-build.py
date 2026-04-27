#!/usr/bin/env python3
"""Rewrite a generated static site so local URLs are relative.

GitHub Pages branch-root publishing may serve this repository at
/johnny-castaway-ps1/, /Johnny-Castaway-PS1/, or another prefix later. Jekyll's
relative_url filter is prefix-relative, not file-relative, so the committed
static build gets a final portability pass.
"""

from __future__ import annotations

import argparse
import os
import re
from pathlib import Path
from urllib.parse import quote, unquote, urlsplit, urlunsplit


ATTR_RE = re.compile(r"""(?P<prefix>\b(?:href|src|poster|content)=["'])(?P<url>[^"']+)(?P<suffix>["'])""")
URL_RE = re.compile(r"""url\((?P<quote>["']?)(?P<url>/[^)"']+)(?P=quote)\)""")


def is_external(url: str) -> bool:
    parsed = urlsplit(url)
    return bool(parsed.scheme) or url.startswith("//") or url.startswith("data:")


def normalize_local_path(path: str, baseurls: list[str]) -> str | None:
    if not path.startswith("/"):
        return None
    for base in sorted((b.rstrip("/") for b in baseurls if b), key=len, reverse=True):
        if path == base:
            return "/"
        if path.startswith(base + "/"):
            return path[len(base):] or "/"
    return path


def rel_url(root: Path, current: Path, url: str, baseurls: list[str]) -> str:
    if is_external(url):
        return url
    parsed = urlsplit(url)
    path = normalize_local_path(unquote(parsed.path), baseurls)
    if path is None:
        return url

    target = root / path.lstrip("/")
    rel = os.path.relpath(target, current.parent)
    if rel == ".":
        rel = "."
    if path.endswith("/") and not rel.endswith("/"):
        rel += "/"
    if not rel.startswith("."):
        rel = "./" + rel
    rel = quote(rel, safe="/.#?=&:%+-_~")
    return urlunsplit(("", "", rel, parsed.query, parsed.fragment))


def rewrite_srcset(root: Path, current: Path, value: str, baseurls: list[str]) -> str:
    items = []
    for raw in value.split(","):
        parts = raw.strip().split()
        if not parts:
            continue
        parts[0] = rel_url(root, current, parts[0], baseurls)
        items.append(" ".join(parts))
    return ", ".join(items)


def rewrite_text(root: Path, current: Path, text: str, baseurls: list[str]) -> str:
    def attr_sub(match: re.Match[str]) -> str:
        url = match.group("url")
        if "," in url and (" " in url or "\n" in url):
            rewritten = rewrite_srcset(root, current, url, baseurls)
        else:
            rewritten = rel_url(root, current, url, baseurls)
        return f"{match.group('prefix')}{rewritten}{match.group('suffix')}"

    def css_sub(match: re.Match[str]) -> str:
        rewritten = rel_url(root, current, match.group("url"), baseurls)
        quote_char = match.group("quote")
        return f"url({quote_char}{rewritten}{quote_char})"

    text = ATTR_RE.sub(attr_sub, text)
    text = URL_RE.sub(css_sub, text)
    return text


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("root", type=Path)
    ap.add_argument("--baseurl", action="append", default=[])
    args = ap.parse_args()

    root = args.root.resolve()
    for path in list(root.rglob("*.html")) + list(root.rglob("*.css")) + list(root.rglob("*.xml")):
        text = path.read_text(encoding="utf-8", errors="replace")
        rewritten = rewrite_text(root, path, text, args.baseurl)
        if rewritten != text:
            path.write_text(rewritten, encoding="utf-8")

    print(f"Relativized static URLs under {root}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
