#!/bin/bash
# capture-scene-explorer-thumbnail.sh — produce a Scene Explorer thumbnail
# for a single scene by running the headless DuckStation regtest capture
# pipeline and then extracting the 70%-mark frame as a 320x240 RGB555 SCR.
#
# Usage:
#   ./scripts/capture-scene-explorer-thumbnail.sh <slug>
#
#   slug    Lowercase scene slug (e.g. fishing1, mary3, johnny6).
#
# Wraps:
#   1. ./scripts/capture-reference-frames.sh --scene "FAMILY N"
#   2. python3 scripts/build-scene-explorer-thumbnails.py --slug <slug>
#
# Recommended workflow: run this immediately after a scene moves from
# ⏳ to ✅ in docs/ps1/scene-status.md and commit the resulting
# jc_resources/extracted/scr/SX<FAM><TAG>.SCR alongside the validation
# commit. Subsequent release.sh runs will rebuild the CD image with
# the new thumbnail in place.
#
# The extractor's 70%-mark frame pick is automatic; per-scene overrides
# go in scripts/scene-explorer-overrides.json.

set -euo pipefail

if [ "$(id -u)" = "0" ]; then
    echo "ERROR: Do not run this script as root/sudo." >&2
    exit 1
fi

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

SLUG="${1:-}"
if [ -z "$SLUG" ]; then
    echo "Usage: $0 <slug>" >&2
    echo "  e.g. $0 mary3" >&2
    exit 2
fi

# Slug -> "FAMILY N" form expected by capture-reference-frames.sh.
# fishing1 -> "FISHING 1", activity12 -> "ACTIVITY 12", etc.
FAMILY="$(printf '%s' "$SLUG" | sed -E 's/[0-9]+$//' | tr '[:lower:]' '[:upper:]')"
TAG="$(printf '%s' "$SLUG" | sed -E 's/^[a-z]+//')"

if [ -z "$FAMILY" ] || [ -z "$TAG" ]; then
    echo "ERROR: could not parse family/tag from slug '$SLUG'" >&2
    exit 3
fi

SCENE_LABEL="${FAMILY} ${TAG}"
WORK_DIR="$PROJECT_ROOT/regtest-references/${FAMILY}-${TAG}/.regtest-work"

echo "=== Capturing $SCENE_LABEL ==="
# regtest-scene.sh wraps the headless DuckStation regtest binary and dumps
# frames into <output>/<timestamp>/filtered-frames/. We bypass the
# capture-reference-frames.sh layer because its JSON post-processing step
# breaks on certain scenes (transient empty result.json), which the
# extractor doesn't need anyway — it falls back to .regtest-work/**/.
"$SCRIPT_DIR/regtest-scene.sh" \
    --scene "$SCENE_LABEL" \
    --frames 1800 --interval 5 \
    --output "$WORK_DIR" \
    --quiet

echo ""
echo "=== Extracting 70%-mark thumbnail for $SLUG ==="
python3 "$SCRIPT_DIR/build-scene-explorer-thumbnails.py" --slug "$SLUG"

# Family abbreviation must match scripts/build-scene-explorer-thumbnails.py
# FAMILY_ABBREV and src/graphics_ps1/graphics_ps1.c grLoadSceneExplorerThumbnail.
declare -A FAMILY_ABBREV=(
    [fishing]=FI [johnny]=JO [mary]=MA [visitor]=VI [activity]=AC
    [suzy]=SU [miscgag]=MG [stand]=ST [walkstuf]=WK [building]=BL
)
FAM_LOWER="$(printf '%s' "$SLUG" | sed -E 's/[0-9]+$//')"
ABBREV="${FAMILY_ABBREV[$FAM_LOWER]:-}"
if [ -z "$ABBREV" ]; then
    echo "ERROR: unknown family '$FAM_LOWER' — update FAMILY_ABBREV table." >&2
    exit 4
fi
SCR_PATH="$PROJECT_ROOT/jc_resources/extracted/scr/SX${ABBREV}${TAG}.SCR"

echo ""
if [ -f "$SCR_PATH" ]; then
    SCR_SIZE=$(stat -c %s "$SCR_PATH")
    echo "Wrote $(realpath --relative-to="$PROJECT_ROOT" "$SCR_PATH") ($SCR_SIZE bytes)"
    echo ""
    echo "Next steps:"
    echo "  - Add a <file name=\"SX${ABBREV}${TAG}.SCR\" ...> entry under <dir name=\"SCR\"> in"
    echo "    config/ps1/cd_layout.xml so the next CD build picks it up."
    echo "  - Commit jc_resources/extracted/scr/SX${ABBREV}${TAG}.SCR alongside the"
    echo "    scene-status.md validation update."
else
    echo "ERROR: thumbnail did not land at $SCR_PATH" >&2
    echo "Check capture output under regtest-references/${FAMILY}-${TAG}/" >&2
    exit 5
fi
