#!/bin/bash
# Launch DuckStation with a scene's pre-built ISO from scratch/scene-isos/.
# Pair with scripts/prebuild-scene-isos.py — that script bakes BOOTMODE.TXT
# into one ISO per scene so the runtime boots straight into that scene
# without a per-scene rebuild.
#
# Usage:
#   scripts/launch-scene-iso.sh <slug>
#   scripts/launch-scene-iso.sh visitor4
#
# Use Ctrl+C or close the DuckStation window to stop.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

SLUG="${1:?usage: launch-scene-iso.sh <slug>}"
CUE="$PROJECT_ROOT/scratch/scene-isos/$SLUG/johnnycastawayps1-$SLUG.cue"

if [ ! -f "$CUE" ]; then
    echo "no ISO at $CUE" >&2
    echo "run: python3 scripts/prebuild-scene-isos.py --slugs $SLUG" >&2
    exit 1
fi

# Match rebuild-and-let-run.sh's DuckStation invocation so logs land in
# the same place and the cue's directory is mounted into the sandbox.
echo "launching DuckStation with: $CUE"
exec flatpak run --filesystem="$(dirname "$CUE")" \
    org.duckstation.DuckStation "$CUE"
