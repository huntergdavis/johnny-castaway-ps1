#!/usr/bin/env python3
"""Generate website library pages from repository documentation artifacts.

The website has authored narrative pages under site/about, site/docs, site/lab,
and site/archaeology. This script adds the exhaustive "wiki shelf": one page
for every Markdown document outside the website tree, plus per-regtest-reference
case pages and a compact resource catalog.
"""

from __future__ import annotations

import json
import re
import shutil
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
SITE = ROOT / "site"


def rel(path: Path) -> str:
    return path.relative_to(ROOT).as_posix()


def md_escape(value: str) -> str:
    return value.replace("\\", "\\\\").replace('"', '\\"')


def yaml_quote(value: str) -> str:
    return '"' + md_escape(value) + '"'


def strip_front_matter(text: str) -> str:
    if text.startswith("---\n"):
        end = text.find("\n---\n", 4)
        if end >= 0:
            return text[end + 5 :]
    return text


def clean_inline_markdown(text: str) -> str:
    text = re.sub(r"`([^`]+)`", r"\1", text)
    text = re.sub(r"\[([^\]]+)\]\([^)]+\)", r"\1", text)
    text = re.sub(r"[*_#>]+", "", text)
    text = text.replace("/home/hunter/workspace/jc_reborn/", "repo:/")
    text = text.replace("/home/hunter/workspace/jc_reborn", "repo:/")
    text = text.replace("/home/hunter/workspace/", "workspace:/")
    return " ".join(text.strip().split())


def title_from_markdown(path: Path, body: str) -> str:
    for line in body.splitlines():
        if line.startswith("# "):
            return clean_inline_markdown(line[2:])
    stem = "README" if path.name.lower() == "readme.md" else path.stem
    return stem.replace("-", " ").replace("_", " ").title()


def first_paragraph(body: str) -> str:
    chunks: list[str] = []
    in_code = False
    for raw in body.splitlines():
        line = raw.strip()
        if line.startswith("```"):
            in_code = not in_code
            continue
        if in_code:
            continue
        if not line:
            if chunks:
                break
            continue
        if line.startswith(("#", "-", "*", "|", ">", "<", "```")):
            if chunks:
                break
            continue
        chunks.append(clean_inline_markdown(line))
    para = " ".join(chunks).strip()
    if len(para) > 460:
        para = para[:457].rstrip() + "..."
    return para


def heading_outline(body: str, limit: int = 14) -> list[tuple[int, str]]:
    out: list[tuple[int, str]] = []
    in_code = False
    for raw in body.splitlines():
        if raw.strip().startswith("```"):
            in_code = not in_code
            continue
        if in_code:
            continue
        match = re.match(r"^(#{1,4})\s+(.+?)\s*$", raw)
        if match:
            out.append((len(match.group(1)), clean_inline_markdown(match.group(2))))
            if len(out) >= limit:
                break
    return out


def category_for(path: str) -> tuple[str, str]:
    if path == "README.md":
        return (
            "Project front door",
            "This is the README: the public landing document for the repository. "
            "The site links to it because it is still the shortest way to see what the project promises at the source level.",
        )
    if path.startswith("docs/ps1/research/generated/"):
        return (
            "Generated PS1 research",
            "This is machine-generated archaeology. It captures scene specs, clusters, schemas, and rollout data that would be painful to reconstruct by hand.",
        )
    if path.startswith("docs/ps1/research/archive/"):
        return (
            "Archived PS1 research",
            "This is a saved research snapshot. It records an earlier state of the investigation so later fixes can be understood instead of mythologized.",
        )
    if path.startswith("docs/ps1/research/"):
        return (
            "PS1 research worklog",
            "This is one of the working notes that drove the port: plans, validation logs, classifier notes, and the pivots that eventually shaped the runtime.",
        )
    if path.startswith("docs/ps1/archaeology/"):
        return (
            "Archaeology source",
            "This file belongs to the history layer: source maps, assumptions, memory notes, and retired artifacts that explain why the current design looks the way it does.",
        )
    if path.startswith("docs/ps1/"):
        return (
            "Active PS1 reference",
            "This is active engineering documentation for the PS1 port. It is close enough to the code that stale claims here become real bugs.",
        )
    if path.startswith("docs/general/"):
        return (
            "General project reference",
            "This is shared documentation: architecture, build setup, emulator use, memory management, and testing guidance that apply beyond one PS1 subsystem.",
        )
    if path.startswith("docs/archive/"):
        return (
            "Historical archive",
            "This is older project documentation preserved for context. It is not the current source of truth, but it explains the roads that were tried.",
        )
    if path.startswith("docs/"):
        return (
            "Repository documentation",
            "This is part of the repository's documentation corpus. The generated page exists so it has a stable place in the public site index.",
        )
    return (
        "Repository note",
        "This Markdown file sits outside the website tree and is preserved in the source library so the project can be read as a whole.",
    )


def output_path_for_source(source: Path) -> Path:
    stem_path = source.with_suffix("")
    return SITE / "source" / stem_path.relative_to(ROOT) / "index.md"


def generate_source_library() -> None:
    target = SITE / "source"
    if target.exists():
        shutil.rmtree(target)
    target.mkdir(parents=True, exist_ok=True)

    markdown_files = sorted(
        p for p in ROOT.rglob("*.md")
        if "site" not in p.relative_to(ROOT).parts
        and ".git" not in p.relative_to(ROOT).parts
        and not any(part.startswith(".") for part in p.relative_to(ROOT).parts)
        and "vendor" not in p.relative_to(ROOT).parts
        and "build-host" not in p.relative_to(ROOT).parts
        and "build-ps1" not in p.relative_to(ROOT).parts
    )

    grouped: dict[str, list[tuple[str, str, str]]] = {}
    for path in markdown_files:
        source_rel = rel(path)
        text = path.read_text(encoding="utf-8", errors="replace")
        body = strip_front_matter(text)
        title = title_from_markdown(path, body)
        excerpt = first_paragraph(body)
        headings = heading_outline(body)
        line_count = text.count("\n") + (1 if text else 0)
        word_count = len(re.findall(r"\b[\w'-]+\b", body))
        code_fences = body.count("```") // 2
        category, why = category_for(source_rel)
        out_path = output_path_for_source(path)
        out_path.parent.mkdir(parents=True, exist_ok=True)
        depth = len(path.relative_to(ROOT).parts)
        back = "../" * (depth + 1)
        page_url = "/" + out_path.parent.relative_to(SITE).as_posix().rstrip("/") + "/"
        group_key = category
        grouped.setdefault(group_key, []).append((title, page_url, source_rel))

        outline_md = "\n".join(
            f"- {'  ' * max(level - 1, 0)}{heading}" for level, heading in headings
        ) or "- No Markdown headings detected. This is either a short note or a generated artifact."

        content = f"""---
layout: page
title: {yaml_quote(title)}
eyebrow: Source library
subtitle: {yaml_quote(source_rel)}
description: {yaml_quote('Generated source-library page for ' + source_rel)}
---

This is the website shelf page for [`{source_rel}`]({{{{ site.github_url }}}}/blob/main/{source_rel}).
It exists because the project has hundreds of Markdown files and the website
should not pretend the interesting work only happened in the polished essays.

## Why this file matters

**{category}.** {why}

{excerpt if excerpt else 'This file is mostly structured data, a terse checklist, or a generated index. The outline below is the useful part.'}

## File facts

| Field | Value |
|---|---:|
| Source path | `{source_rel}` |
| Lines | {line_count} |
| Approx. words | {word_count} |
| Code fences | {code_fences} |

## Outline

{outline_md}

## Read it in context

- [Open the source file on GitHub]({{{{ site.github_url }}}}/blob/main/{source_rel})
- [Back to the source library index]({{{{ '/source/' | relative_url }}}})
- [Main docs index]({{{{ '/docs/' | relative_url }}}})
"""
        out_path.write_text(content, encoding="utf-8")

    group_sections = []
    total = 0
    for group in sorted(grouped):
        items = sorted(grouped[group], key=lambda item: item[2].lower())
        total += len(items)
        lis = "\n".join(
            f'<li><a href="{{{{ \'{url}\' | relative_url }}}}">{title}</a><p><code>{source}</code></p></li>'
            for title, url, source in items
        )
        group_sections.append(f"## {group}\n\n<ul class=\"doc-grid\">\n{lis}\n</ul>")

    sections_md = "\n\n".join(group_sections)
    index = f"""---
layout: page
title: Source library
eyebrow: Every Markdown file gets a page
subtitle: {total} source documents, wrapped for the website instead of buried in the repository tree.
description: Generated source-library index for every Markdown documentation file in the Johnny Castaway PS1 repository.
---

This is the shelf I wanted when the project stopped being a codebase and
became an archive. Every Markdown document outside the website tree gets a
public page: active docs, generated scene specs, archaeology notes, old plans,
status reports, and things that are half fossil, half runbook.

The pages are generated. The source files remain canonical. The value here is
navigation: each page names the source path, extracts the outline, gives a
short read of why that file matters, and links back to GitHub.

Use this with the [resource catalog]({{{{ '/resources/' | relative_url }}}}) for
binary assets, the [regtest case shelf]({{{{ '/archaeology/regtest-references/cases/' | relative_url }}}})
for host references, and the [Curious Hacker's Guide]({{{{ '/hack/' | relative_url }}}})
for a guided learning path through the machinery.

{sections_md}
"""
    (target / "index.md").write_text(index, encoding="utf-8")


def slug_case(name: str) -> str:
    return name.lower().replace("_", "-")


def generate_regtest_cases() -> None:
    source_root = ROOT / "docs/ps1/archaeology/regtest-references"
    target = SITE / "archaeology/regtest-references/cases"
    if target.exists():
        shutil.rmtree(target)
    target.mkdir(parents=True, exist_ok=True)

    case_rows = []
    for meta_path in sorted(source_root.glob("*/metadata.json")):
        case_dir = meta_path.parent
        case_name = case_dir.name
        result_path = case_dir / "result.json"
        meta = json.loads(meta_path.read_text(encoding="utf-8"))
        result = json.loads(result_path.read_text(encoding="utf-8")) if result_path.exists() else {}
        outcome = result.get("outcome", {})
        slug = slug_case(case_name)
        out = target / slug / "index.md"
        out.parent.mkdir(parents=True, exist_ok=True)

        frames = meta.get("frame_count") or outcome.get("frames_captured") or 0
        state_hash = outcome.get("state_hash", "")
        boot = meta.get("boot_string", "")
        ads = meta.get("ads_name", "")
        tag = meta.get("tag", "")
        status = meta.get("status", "")
        date = meta.get("capture_date", "")
        source_rel = rel(case_dir)

        content = f"""---
layout: page
title: {yaml_quote(case_name + ' regression reference')}
eyebrow: Host regression reference
subtitle: {yaml_quote(str(ads) + '.ADS tag ' + str(tag))}
description: {yaml_quote('Regtest reference case for ' + case_name + '.')}
---

This is the public shelf page for the host regression reference at
[`{source_rel}`]({{{{ site.github_url }}}}/tree/main/{source_rel}).

## What this case captures

`{case_name}` is the host-side reference for `{ads}.ADS` tag `{tag}`.
The host runner booted the scene with:

```text
{boot}
```

The reference is not a finished PS1 sign-off. It is the old-fashioned
baseline: run the known desktop interpreter, capture the frames, save the
metadata, and keep the result around so later PS1 work has something to argue
with.

## Recorded facts

| Field | Value |
|---|---:|
| ADS | `{ads}` |
| Tag | `{tag}` |
| Scene index | `{meta.get('scene_index', '')}` |
| Status | `{status}` |
| Capture date | `{date}` |
| Frames captured | `{frames}` |
| Exit code | `{outcome.get('exit_code', '')}` |
| Timed out | `{outcome.get('timed_out', '')}` |
| Fatal error | `{outcome.get('has_fatal_error', '')}` |

## State hash

```text
{state_hash or '(not recorded)'}
```

## Source artifacts

- [`metadata.json`]({{{{ site.github_url }}}}/blob/main/{source_rel}/metadata.json)
- [`result.json`]({{{{ site.github_url }}}}/blob/main/{source_rel}/result.json)
- [`review.html`]({{{{ site.github_url }}}}/blob/main/{source_rel}/review.html)

## Cross-links

- [All regtest references]({{{{ '/archaeology/regtest-references/cases/' | relative_url }}}})
- [Regtest reference narrative]({{{{ '/archaeology/regtest-references/' | relative_url }}}})
- [Scene ledger]({{{{ '/scenes/' | relative_url }}}})
- [Regression testing docs]({{{{ '/docs/regtest/' | relative_url }}}})
"""
        out.write_text(content, encoding="utf-8")
        case_rows.append((case_name, slug, ads, tag, frames, status))

    rows = "\n".join(
        f'<tr><td><a href="{{{{ \'/archaeology/regtest-references/cases/{slug}/\' | relative_url }}}}">{name}</a></td><td><code>{ads}</code></td><td>{tag}</td><td>{frames}</td><td>{status}</td></tr>'
        for name, slug, ads, tag, frames, status in case_rows
    )
    index = f"""---
layout: page
title: Regtest reference cases
eyebrow: 63 host baselines
subtitle: One public shelf page for each preserved host regression reference.
description: Per-case pages for the archived host regression references used by the Johnny Castaway PS1 port.
---

These are the host baselines preserved under
[`docs/ps1/archaeology/regtest-references`]({{{{ site.github_url }}}}/tree/main/docs/ps1/archaeology/regtest-references).
They are not the final PS1 truth. They are the measurements the PS1 runtime
keeps being compared against while a scene moves from "it boots" to "it is
signed off."

<table>
<thead><tr><th>Case</th><th>ADS</th><th>Tag</th><th>Frames</th><th>Status</th></tr></thead>
<tbody>
{rows}
</tbody>
</table>

Back to [regtest reference narrative]({{{{ '/archaeology/regtest-references/' | relative_url }}}}).
"""
    (target / "index.md").write_text(index, encoding="utf-8")


def generate_resource_catalog() -> None:
    target = SITE / "resources"
    if target.exists():
        shutil.rmtree(target)
    target.mkdir(parents=True, exist_ok=True)

    groups = [
        ("Original bitmap resources", ROOT / "jc_resources/extracted/bmp", "*.BMP"),
        ("Original animation scripts", ROOT / "jc_resources/extracted/ads", "*.ADS"),
        ("Original TTM animations", ROOT / "jc_resources/extracted/ttm", "*.TTM"),
        ("Original screens", ROOT / "jc_resources/extracted/scr", "*.SCR"),
        ("Original sounds", ROOT / "jc_resources/extracted/snd", "*.VAG"),
        ("Transcoded PS1 sprite banks", ROOT / "jc_resources/transcoded", "*.PSB"),
        ("Generated foreground packs", ROOT / "generated/ps1/foreground", "*.FG2"),
    ]
    sections = []
    total = 0
    for title, directory, pattern in groups:
        files = sorted(directory.glob(pattern)) if directory.exists() else []
        total += len(files)
        rows = []
        for f in files:
            source_rel = rel(f)
            size = f.stat().st_size
            rows.append(
                f"<tr><td><code>{f.name}</code></td><td>{size:,}</td>"
                f"<td><a href=\"{{{{ site.github_url }}}}/blob/main/{source_rel}\">source</a></td></tr>"
            )
        rows_md = "\n".join(rows) or "<tr><td colspan=\"3\">No files found.</td></tr>"
        sections.append(f"""## {title}

<table>
<thead><tr><th>File</th><th>Bytes</th><th>Link</th></tr></thead>
<tbody>
{rows_md}
</tbody>
</table>
""")

    resource_sections = "\n\n".join(section.rstrip() for section in sections)
    index = f"""---
layout: page
title: Resource catalog
eyebrow: Sprites, sounds, packs
subtitle: {total} source and generated runtime assets indexed for the site.
description: Catalog of Johnny Castaway source assets and generated PS1 runtime artifacts.
---

The code is only half the port. The other half is the pile of tiny artifacts:
Sierra bitmaps, ADS scripts, TTM animations, VAG samples, PSB sprite banks, and
the generated FG2 packs the PS1 actually replays. This page is the map.

It intentionally links to source files rather than copying every binary into
the website. The GitHub tree remains the canonical archive; this page makes it
readable.

Pair this with the [source library]({{{{ '/source/' | relative_url }}}}) for
Markdown documentation, the [scene ledger]({{{{ '/scenes/' | relative_url }}}})
for runtime status, and the [Curious Hacker's Guide]({{{{ '/hack/' | relative_url }}}})
for the practical porting path.

{resource_sections}
"""
    (target / "index.md").write_text(index, encoding="utf-8")


def main() -> None:
    generate_source_library()
    generate_regtest_cases()
    generate_resource_catalog()
    print("Generated source library, regtest cases, and resource catalog.")


if __name__ == "__main__":
    main()
