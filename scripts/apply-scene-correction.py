#!/usr/bin/env python3
"""
Apply a single-pass scene correction across all 5 sources of truth.

Used inside the chapter-select-thumbnail loop when the on-PS1 pack
content disagrees with the prior caption-mapping guess.

Touches:
  1. site/scenes/<slug>/index.md  — frontmatter `title:` + `description:`
                                    + body "## What this scene probably is"
                                    section (also reframed to "## What this scene is").
  2. site/_data/scenes.yml         — `notes:` for the matching slug entry
                                     (prepends a one-line lead reframe so the
                                     existing engineering detail survives).
  3. docs/ps1/scene-status.md      — Notes cell for the matching row
                                     (same prepend strategy).
  4. (optional) jc_resources/extracted/scr/SX<abbrev><tag>.SCR
                                   — converted from --png if provided.
  5. scratch/chapter-select-progress.txt
                                   — flips the slug from `pending` to `done`.

Usage:
    python3 scripts/apply-scene-correction.py <slug> \
        --title "ACTIVITY 5 — Rain dance, struck by lightning" \
        --short "rain-dance-then-lightning-strike (Johnny in costume...)" \
        --body  "Johnny puts on a costume and performs a rain dance..." \
        [--png  /path/to/screenshot.png]

The --short string is the canonical one-line description (used to lead
the scenes.yml notes + scene-status.md Notes prepend); --body is the
prose paragraph that replaces the per-scene "What this scene is" body.

The script is idempotent if you re-run with the same arguments: each
surface uses an exact-string Edit, so a re-run on an already-corrected
scene fails noisily (good — it means you're about to clobber a fix).
"""
import argparse
import importlib.util
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SCENES_YML = ROOT / "site/_data/scenes.yml"
SCENE_STATUS = ROOT / "docs/ps1/scene-status.md"
PROGRESS = ROOT / "scratch/chapter-select-progress.txt"
THUMB_BUILDER = ROOT / "scripts/build-scene-explorer-thumbnails.py"


def load_thumb_helper():
    spec = importlib.util.spec_from_file_location("scene_thumb", THUMB_BUILDER)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


def find_scenes_yml_block(text, slug):
    """Find the (start, end) of the scenes.yml entry for `slug`. Each
    entry begins with `- ads:` at column 0 and runs until the next
    `- ads:` or end-of-file."""
    pattern = re.compile(r"^- ads:.*?(?=^- ads:|\Z)", re.M | re.S)
    for m in pattern.finditer(text):
        block = m.group(0)
        if re.search(rf"^  slug: {re.escape(slug)}$", block, re.M):
            return m.start(), m.end(), block
    raise SystemExit(f"slug not found in scenes.yml: {slug}")


def update_scenes_yml(slug, short_desc):
    text = SCENES_YML.read_text()
    start, end, block = find_scenes_yml_block(text, slug)
    notes_match = re.search(r"^  notes: (.*)$", block, re.M)
    if not notes_match:
        raise SystemExit(f"no notes line for {slug} in scenes.yml")
    old_notes = notes_match.group(1)
    new_notes = f"{short_desc}; {old_notes}"
    new_block = block[: notes_match.start(1)] + new_notes + block[notes_match.end(1):]
    SCENES_YML.write_text(text[:start] + new_block + text[end:])


def update_scene_page(slug, title, description, body):
    page = ROOT / f"site/scenes/{slug}/index.md"
    text = page.read_text()
    # Replace the title line in frontmatter (between two ---).
    text = re.sub(
        r"^title: .*$",
        f"title: {title}",
        text, count=1, flags=re.M,
    )
    # Replace or insert the description line.
    if re.search(r"^description: ", text, flags=re.M):
        text = re.sub(
            r'^description: "?.*?"?$',
            f'description: "{description}"',
            text, count=1, flags=re.M,
        )
    else:
        # Insert before closing ---
        text = re.sub(
            r"^---$",
            f'description: "{description}"\n---',
            text, count=1, flags=re.M,
        )
    # Replace the "What this scene probably is" section (also handles
    # already-corrected "What this scene is"). Match through the
    # "Caption mapping confidence" line so we replace the whole guessy
    # block, since the correction asserts the truth.
    pat = re.compile(
        r"## What this scene (probably )?is\s*\n\n.*?(?=\n#{2,} |\Z)",
        re.S,
    )
    # Emit a single trailing newline; the lookahead leaves the
    # newline-before-heading in place, so this produces exactly one
    # blank line before the next heading.
    new_section = f"## What this scene is\n\n{body}\n"
    if not pat.search(text):
        raise SystemExit(f"no 'What this scene is' section in {page}")
    text = pat.sub(new_section, text, count=1)
    page.write_text(text)


def update_scene_status(slug, short_desc):
    text = SCENE_STATUS.read_text()
    # Find the row by `| slug |` token. The notes column is the 8th cell —
    # after slug we must skip 4 cells (done, sfx, variants, last_verified)
    # before reaching notes. Use greedy `[^|]+` so each skipped cell consumes
    # its full content (otherwise non-greedy lands the prepend in `done`).
    pat = re.compile(
        rf"^(\| [A-Z]+ \| \d+ \| {re.escape(slug)} \| (?:[^|]+\| ){{4}})(.*?)( \|)$",
        re.M,
    )
    m = pat.search(text)
    if not m:
        raise SystemExit(f"no scene-status row for {slug}")
    old_notes = m.group(2)
    new_notes = f"{short_desc}; {old_notes}"
    text = text[: m.start()] + m.group(1) + new_notes + m.group(3) + text[m.end():]
    SCENE_STATUS.write_text(text)


def update_thumb(slug, png_path, mod):
    short = mod.slug_to_short_name(slug)
    if short is None:
        raise SystemExit(f"unknown slug for thumbnail: {slug}")
    out_dir = ROOT / "jc_resources/extracted/scr"
    out_dir.mkdir(parents=True, exist_ok=True)
    out_path = out_dir / f"{short}.SCR"
    mod.png_to_thumbnail_scr(Path(png_path), out_path)
    return out_path


def update_progress(slug):
    if not PROGRESS.is_file():
        return  # tracker is local; not fatal if missing
    lines = PROGRESS.read_text().splitlines()
    out = []
    for line in lines:
        if line.startswith(f"{slug} "):
            out.append(f"{slug} done")
        else:
            out.append(line)
    PROGRESS.write_text("\n".join(out) + "\n")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("slug")
    ap.add_argument("--title", required=True,
                    help="Scene-page H1 + frontmatter title (e.g. 'ACTIVITY 5 — Rain dance, struck by lightning')")
    ap.add_argument("--short", required=True,
                    help="One-line canonical description used as lead in scenes.yml notes + scene-status Notes")
    ap.add_argument("--description", required=True,
                    help='Scene-page frontmatter description (e.g. "ACTIVITY.ADS scene 5: Johnny ... Validated YYYY-MM-DD.")')
    ap.add_argument("--body", required=True,
                    help='Replacement prose for the "What this scene is" body paragraph')
    ap.add_argument("--png", help="Optional screenshot PNG to encode as the thumbnail SCR")
    args = ap.parse_args()

    update_scene_page(args.slug, args.title, args.description, args.body)
    update_scenes_yml(args.slug, args.short)
    update_scene_status(args.slug, args.short)

    if args.png:
        mod = load_thumb_helper()
        out = update_thumb(args.slug, args.png, mod)
        print(f"wrote {out}")

    update_progress(args.slug)
    print(f"corrected {args.slug}: title='{args.title}'")
    return 0


if __name__ == "__main__":
    sys.exit(main())
