#!/usr/bin/env bash
# Build the simplified holiday emblem sheet.
#
# The old five-variant scene/concept-art pipeline was removed. This command now
# renders one small transparent 32x32 emblem for each added holiday
# and packs them into a single sprite sheet.
#
# Outputs:
#   scratch/holidays-emblems/<id>-<short>.png
#   scratch/holidays-emblems/holiday-emblems-sheet.png
#   scratch/holidays-emblems/holiday-emblems-preview.png
#   scratch/holidays-emblems/review.html
#   scratch/holidays-emblems/manifest.json
#
# --clean        Wipe scratch/holidays-emblems/ before rendering.

set -euo pipefail
cd "$(dirname "$0")/.."

PY=${PYTHON:-python3}

case "${1:-}" in
    -h|--help)
        sed -n '2,18p' "$0" | sed -e 's/^# \{0,1\}//'
        exit 0
        ;;
    --clean)
        echo "==> clean"
        rm -rf scratch/holidays-emblems
        find scripts -name "__pycache__" -type d -exec rm -rf {} + 2>/dev/null || true
        ;;
    "")
        ;;
    *)
        echo "unknown flag: $1" >&2
        echo "run with --help for usage" >&2
        exit 2
        ;;
esac

echo "==> rendering holiday emblems"
"$PY" scripts/holidays-emblem-sheet.py

echo
echo "==> regenerating holiday table"
"$PY" scripts/holidays-codegen.py

echo
echo "==> packing HOLIDAY.BMP / HOLIDAY.PSB"
"$PY" scripts/holidays-pack-psb.py --verify

echo
echo "==> done."
echo "    Sheet:    scratch/holidays-emblems/holiday-emblems-sheet.png"
echo "    Preview:  scratch/holidays-emblems/holiday-emblems-preview.png"
echo "    Review:   scratch/holidays-emblems/review.html"
echo "    Manifest: scratch/holidays-emblems/manifest.json"
echo "    BMP:      jc_resources/extracted/bmp/HOLIDAY.BMP"
echo "    PSB:      jc_resources/transcoded/HOLIDAY.PSB"
