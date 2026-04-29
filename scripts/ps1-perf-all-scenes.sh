#!/bin/bash
# Run fgpilot perf cases from the generated 63-scene/126-variant manifest.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$PROJECT_ROOT"

TIDES="both"
ORDER="list"
LIMIT=""
SEED="${REGTEST_SEED:-1}"
FRAMES="${PS1_PERF_FRAMES:-7200}"
TIMEOUT="${PS1_PERF_TIMEOUT:-220}"
OUTPUT_ROOT="${PS1_PERF_OUTPUT_DIR:-$PROJECT_ROOT/scratch/ps1-perf-iterate}"
SHEET="$PROJECT_ROOT/docs/ps1/performance-scene-matrix.csv"
UPDATE_SHEET=1

PERF_ARGS=()

usage() {
    cat <<'USAGE'
Usage: ps1-perf-all-scenes.sh [options] [-- ps1-perf-iterate options...]

Generates fgpilot cases for all routed foreground scenes and feeds them to
scripts/ps1-perf-iterate.sh. This is intentionally a matrix harness, not the
legacy certification regtest.

Options:
  --tides high|low|both      Which tide variants to run (default: both).
  --order list|random        Manifest order or seeded random order (default: list).
  --random                   Shortcut for --order random.
  --limit N                  Run only the first N generated cases.
  --seed N                   Seed for random order and boot strings (default: REGTEST_SEED or 1).
  --frames N                 Emulated frames per case (default: PS1_PERF_FRAMES or 7200).
  --timeout N                Wall-clock timeout per case (default: PS1_PERF_TIMEOUT or 220).
  --output DIR               Perf output root (default: scratch/ps1-perf-iterate).
  --sheet PATH               CSV sheet to refresh after a successful run.
  --no-sheet                 Do not refresh the CSV sheet.
  -h, --help                 Show this help.

Any arguments after -- are passed through to ps1-perf-iterate.sh.
USAGE
}

while [ $# -gt 0 ]; do
    case "$1" in
        --tides)
            TIDES="$2"; shift 2 ;;
        --order)
            ORDER="$2"; shift 2 ;;
        --random)
            ORDER="random"; shift ;;
        --limit)
            LIMIT="$2"; shift 2 ;;
        --seed)
            SEED="$2"; shift 2 ;;
        --frames)
            FRAMES="$2"; shift 2 ;;
        --timeout)
            TIMEOUT="$2"; shift 2 ;;
        --output)
            OUTPUT_ROOT="$2"; shift 2 ;;
        --sheet)
            SHEET="$2"; UPDATE_SHEET=1; shift 2 ;;
        --no-sheet)
            UPDATE_SHEET=0; shift ;;
        --)
            shift
            PERF_ARGS+=("$@")
            break ;;
        -h|--help)
            usage; exit 0 ;;
        *)
            echo "Unknown option: $1" >&2
            usage >&2
            exit 1 ;;
    esac
done

if [ "$TIDES" != "high" ] && [ "$TIDES" != "low" ] && [ "$TIDES" != "both" ]; then
    echo "ERROR: --tides must be high, low, or both." >&2
    exit 1
fi
if [ "$ORDER" != "list" ] && [ "$ORDER" != "random" ]; then
    echo "ERROR: --order must be list or random." >&2
    exit 1
fi

MANIFEST_ARGS=(--print-cases --tides "$TIDES" --order "$ORDER" --seed "$SEED")
if [ -n "$LIMIT" ]; then
    MANIFEST_ARGS+=(--limit "$LIMIT")
fi

CASE_ARGS=()
while IFS=$'\t' read -r label boot; do
    [ -n "$label" ] || continue
    CASE_ARGS+=(--case "${label}::${boot}")
done < <("$SCRIPT_DIR/ps1-foreground-scene-manifest.py" "${MANIFEST_ARGS[@]}")

if [ "${#CASE_ARGS[@]}" -eq 0 ]; then
    echo "ERROR: no perf cases generated." >&2
    exit 1
fi

set +e
"$SCRIPT_DIR/ps1-perf-iterate.sh" \
    "${CASE_ARGS[@]}" \
    --frames "$FRAMES" \
    --timeout "$TIMEOUT" \
    --output "$OUTPUT_ROOT" \
    "${PERF_ARGS[@]}"
PERF_STATUS=$?
set -e

if [ "$UPDATE_SHEET" -eq 1 ]; then
    SUMMARY_ARGS=()
    while IFS= read -r summary; do
        SUMMARY_ARGS+=(--summary "$summary")
    done < <(find "$OUTPUT_ROOT" -maxdepth 2 \
        \( -name 'summary.json' -o -name 'current-*.json' \) -print | sort)
    "$SCRIPT_DIR/ps1-foreground-scene-manifest.py" \
        --write-sheet "$SHEET" \
        "${SUMMARY_ARGS[@]}"
    echo "Updated sheet: $SHEET"
fi

exit "$PERF_STATUS"
