#!/usr/bin/env python3
"""
Generate src/pause_menu/scene_explorer_data.h for the in-game Scene Explorer.

Sources:
- docs/ps1/scene-status.md          slugs + visual-validation status
- site/scenes/<slug>/index.md       display name (frontmatter `title`)
- generated/ps1/foreground/*.FG2    frame counts (FG2 header offset 6, uint16 LE)

The output is consumed by src/pause_menu/pause_menu.c at runtime to populate
the Scene Explorer sub-screen. Re-run as part of release.sh; a
CI dry-run check should fail if the committed file drifts from source.
"""

import re
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SCENE_STATUS = ROOT / "docs/ps1/scene-status.md"
SCENES_DIR = ROOT / "site/scenes"
FG2_DIR = ROOT / "generated/ps1/foreground"
OUTPUT = ROOT / "src/pause_menu/scene_explorer_data.h"

# Family display name (what the menu shows under "Family:") keyed by slug
# prefix. miscgag and suzy both surface as "Misc & Suzy" because each family
# alone is too small to justify its own row in the explorer.
FAMILY_DISPLAY = {
    "fishing":  "Fishing",
    "johnny":   "Johnny",
    "mary":     "Mary",
    "visitor":  "Visitors",
    "activity": "Activities",
    "suzy":     "Misc & Suzy",
    "miscgag":  "Misc & Suzy",
    "stand":    "Standing",
    "walkstuf": "Walking",
    "building": "Building",
}

# Family display order for L1/R1 jumps. Locked in design doc R6.
FAMILY_ORDER = [
    "Fishing",
    "Johnny",
    "Mary",
    "Visitors",
    "Activities",
    "Misc & Suzy",
    "Standing",
    "Walking",
    "Building",
]

# Common Unicode normalizations for the embedded 8x8 ASCII font.
ASCII_FIXUPS = {
    "—": " - ",
    "–": "-",
    "’": "'",
    "‘": "'",
    "“": '"',
    "”": '"',
    "…": "...",
}


def slug_family(slug):
    m = re.match(r"^([a-z]+)\d+$", slug)
    if not m:
        return "Unknown"
    return FAMILY_DISPLAY.get(m.group(1), "Unknown")


def to_ascii(text):
    for src, dst in ASCII_FIXUPS.items():
        text = text.replace(src, dst)
    return text.encode("ascii", "replace").decode("ascii")


def parse_scene_status():
    """Return [{ads, tag, slug, validated}, ...] in markdown-table order."""
    rows = []
    # Match either the validated tick or the not-yet hourglass in the visuals
    # column — these are the only two states the table currently uses.
    pattern = re.compile(
        r"^\|\s*([A-Z]+)\s*\|\s*(\d+)\s*\|\s*([a-z0-9]+)\s*\|\s*([✅⏳])\s*\|"
    )
    for line in SCENE_STATUS.read_text(encoding="utf-8").splitlines():
        m = pattern.match(line)
        if not m:
            continue
        ads, tag, slug, visuals = m.groups()
        rows.append({
            "ads": ads,
            "tag": int(tag),
            "slug": slug,
            "validated": visuals == "✅",
        })
    return rows


def read_display_name(slug):
    md_path = SCENES_DIR / slug / "index.md"
    if not md_path.exists():
        return slug.upper()
    in_frontmatter = False
    for line in md_path.read_text(encoding="utf-8").splitlines():
        if line.strip() == "---":
            in_frontmatter = not in_frontmatter
            continue
        if not in_frontmatter:
            continue
        m = re.match(r"^title:\s*(.+?)\s*$", line)
        if m:
            title = m.group(1).strip()
            if (title.startswith('"') and title.endswith('"')) or \
               (title.startswith("'") and title.endswith("'")):
                title = title[1:-1]
            return title
    return slug.upper()


def read_frame_count(slug):
    fg2_path = FG2_DIR / f"{slug.upper()}.FG2"
    if not fg2_path.exists():
        return 0
    with fg2_path.open("rb") as f:
        header = f.read(8)
    if len(header) < 8:
        return 0
    return struct.unpack_from("<H", header, 6)[0]


def pack_path_for(slug):
    return f"FG/{slug.upper()}.FG2"


def thumb_psb_for(slug):
    return f"BMP/SCEXPL_{slug.upper()}.PSB"


def main():
    rows = parse_scene_status()
    if len(rows) != 63:
        print(
            f"WARN: expected 63 scenes in {SCENE_STATUS.name}, got {len(rows)}",
            file=sys.stderr,
        )

    entries = []
    for row in rows:
        slug = row["slug"]
        family = slug_family(slug)
        entries.append({
            "slug": slug,
            "display_name": to_ascii(read_display_name(slug)),
            "family": family,
            "pack": pack_path_for(slug),
            "thumb_psb": thumb_psb_for(slug),
            "frame_count": read_frame_count(slug),
            "validated": 1 if row["validated"] else 0,
        })

    def family_rank(name):
        return FAMILY_ORDER.index(name) if name in FAMILY_ORDER else 99

    entries.sort(key=lambda e: (family_rank(e["family"]), e["slug"]))

    # Family-start index table for L1/R1 jumps.
    family_starts = []
    seen = set()
    for i, e in enumerate(entries):
        if e["family"] not in seen:
            family_starts.append(i)
            seen.add(e["family"])

    out = []
    out.append("/* Generated by scripts/build-scene-explorer-data.py.")
    out.append(" * Sources: docs/ps1/scene-status.md, site/scenes/<slug>/index.md,")
    out.append(" *          generated/ps1/foreground/<SLUG>.FG2.")
    out.append(" * Re-run as part of release.sh; do not edit by hand. */")
    out.append("")
    out.append("#ifndef SCENE_EXPLORER_DATA_H")
    out.append("#define SCENE_EXPLORER_DATA_H")
    out.append("")
    out.append('#include "mytypes.h"')
    out.append("")
    out.append("struct TSceneExplorerEntry {")
    out.append("    const char *slug;          /* internal scene identifier (e.g. \"fishing1\") */")
    out.append("    const char *display_name;  /* shown on the menu line */")
    out.append("    const char *family;        /* display family group */")
    out.append("    const char *pack;          /* CD-relative FG2 path */")
    out.append("    const char *thumb_psb;     /* CD-relative thumbnail PSB path */")
    out.append("    uint16      frame_count;   /* from FG2 header */")
    out.append("    uint8       validated;     /* 1 if scene-status visuals are checked */")
    out.append("    uint8       _pad;")
    out.append("};")
    out.append("")
    out.append("static const struct TSceneExplorerEntry gSceneExplorer[] = {")
    for e in entries:
        display = e["display_name"].replace('"', '\\"')
        out.append(
            f'    {{ "{e["slug"]}", "{display}", "{e["family"]}", '
            f'"{e["pack"]}", "{e["thumb_psb"]}", {e["frame_count"]}, '
            f'{e["validated"]}, 0 }},'
        )
    out.append("};")
    out.append("#define gSceneExplorerCount "
               "((int)(sizeof(gSceneExplorer) / sizeof(gSceneExplorer[0])))")
    out.append("")
    out.append("static const int gSceneExplorerFamilyStart[] = {")
    for s in family_starts:
        out.append(f"    {s},")
    out.append("};")
    out.append("#define gSceneExplorerFamilyCount "
               "((int)(sizeof(gSceneExplorerFamilyStart) / "
               "sizeof(gSceneExplorerFamilyStart[0])))")
    out.append("")
    out.append("#endif")
    out.append("")

    OUTPUT.write_text("\n".join(out), encoding="utf-8")
    print(
        f"Wrote {OUTPUT.relative_to(ROOT)} "
        f"({len(entries)} entries, {len(family_starts)} families)"
    )


if __name__ == "__main__":
    main()
