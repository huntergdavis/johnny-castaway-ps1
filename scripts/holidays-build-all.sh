#!/usr/bin/env bash
# Run the full holidays pipeline end-to-end. Fail-fast.
#
#   1. Regenerate src/holidays_table.c + scripts/holidays-art-spec.json
#      from holidays.yml.
#   2. Smoke-test the pure-python date algorithm mirror (28/28 spot
#      checks).
#   3. Render every variant PNG to scratch/holidays-art/.
#   4. Build the interactive HTML review page.
#   5. Build the static contact-sheet PNG.
#   6. Generate default-picks fallback.
#   7. Resolve picks → scratch/holidays-selected/.
#   8. Build the final-review HTML.
#   9. Run the red-team QA pass (17 checks) — any FAIL aborts.
#
# Run after editing holidays.yml or any renderer file. ~2.5s.
#
# --clean        Wipe scratch/holidays-art/ before rendering so deleted
#                renderers don't leave stale PNGs behind.

set -euo pipefail
cd "$(dirname "$0")/.."

PY=${PYTHON:-python3}

case "${1:-}" in
    -h|--help)
        sed -n '2,20p' "$0" | sed -e 's/^# \{0,1\}//'
        exit 0
        ;;
    --clean)
        echo "==> 0. wiping scratch/holidays-art/, pyc cache"
        rm -rf scratch/holidays-art
        rm -rf scratch/holidays-selected
        # Stale pyc cache can mask source edits (mtime preservation
        # after a restore makes Python skip recompilation). Wipe on
        # --clean.
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

echo "==> 1. codegen"
"$PY" scripts/holidays-codegen.py

echo
echo "==> 2. date-algorithm spot tests"
"$PY" scripts/holidays-test.py | tail -1

echo
echo "==> 3. render PNGs (31 holidays × 5 variants = 155 + 4 originals)"
"$PY" scripts/holidays-generate-art.py | tail -3

echo
echo "==> 4. HTML preview"
"$PY" scripts/holidays-preview.py | tail -3

echo
echo "==> 5. contact-sheet PNG"
"$PY" scripts/holidays-contact-sheet.py | tail -1

echo
echo "==> 6. default-picks fallback (variant 1 for every holiday)"
"$PY" scripts/holidays-default-picks.py | tail -1

echo
echo "==> 7. resolve picks → scratch/holidays-selected/"
"$PY" scripts/holidays-resolve-picks.py | tail -2

echo
echo "==> 8. final-review HTML"
"$PY" scripts/holidays-final-review.py | tail -1

echo
echo "==> 9. red-team QA pass"
"$PY" scripts/holidays-redteam.py | tail -10

echo
echo "==> done."
echo "    HTML preview:     scratch/holidays-preview.html"
echo "    Contact sheet:    scratch/holidays-contact-sheet.png"
echo "    Final review:     scratch/holidays-final-review.html"
echo "    Picks staged at:  scratch/holidays-selected/  (using defaults)"

# Summarize the active picks distribution.
PICKS=scratch/holidays-picks.json
[ -f "$PICKS" ] || PICKS=scratch/holidays-picks-default.json
if [ -f "$PICKS" ]; then
    echo
    echo "    Variant distribution (from $PICKS):"
    "$PY" -c "
import json, collections
picks = json.load(open('$PICKS'))
labels = {'1':'LITERAL', '2':'MINIMALIST', '3':'BUSY', '4':'PLAYFUL', '5':'NIGHT'}
counts = collections.Counter(str(v) for v in picks.values())
for k in ('1','2','3','4','5'):
    print(f'      v{k} {labels[k]:11s}  {counts.get(k, 0):>2d}')
print(f'      total                 {len(picks):>2d} / 31')
"
fi
