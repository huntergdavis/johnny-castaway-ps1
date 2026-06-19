#!/bin/bash
# Capture the standard same-commit perf canary baseline used before risky probes.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$PROJECT_ROOT"

FRAMES="${PS1_PERF_FRAMES:-12000}"
TIMEOUT="${PS1_PERF_TIMEOUT:-240}"
OUTPUT_ROOT="${PS1_PERF_OUTPUT_DIR:-$PROJECT_ROOT/scratch/ps1-perf-iterate}"
LABEL="canary-baseline-$(date +%Y%m%d-%H%M%S)"
SEED="${REGTEST_SEED:-1}"
SKIP_BUILD=0
DRY_RUN=0

usage() {
    cat <<'USAGE'
Usage: ps1-perf-canary-baseline.sh [options]

Runs the standard foreground perf canary set and records the resulting summary
path in <output>/latest-canary-baseline.txt. Use this before risky runtime
experiments so baseline comparisons are same-commit and same-layout.

Options:
  --frames N       Emulated frames per case (default: PS1_PERF_FRAMES or 12000).
  --timeout N      Wall-clock timeout per case (default: PS1_PERF_TIMEOUT or 240).
  --output DIR     Output root (default: scratch/ps1-perf-iterate).
  --label LABEL    Output subdirectory label (default: canary-baseline-<timestamp>).
  --seed N         Seed appended to boot strings (default: REGTEST_SEED or 1).
  --skip-build     Reuse build-ps1/johnnycastawayps1.exe.
  --dry-run        Print the ps1-perf-iterate command without running it.
  -h, --help       Show this help.
USAGE
}

while [ $# -gt 0 ]; do
    case "$1" in
        --frames)
            FRAMES="$2"; shift 2 ;;
        --timeout)
            TIMEOUT="$2"; shift 2 ;;
        --output)
            OUTPUT_ROOT="$2"; shift 2 ;;
        --label)
            LABEL="$2"; shift 2 ;;
        --seed)
            SEED="$2"; shift 2 ;;
        --skip-build)
            SKIP_BUILD=1; shift ;;
        --dry-run)
            DRY_RUN=1; shift ;;
        -h|--help)
            usage; exit 0 ;;
        *)
            echo "Unknown option: $1" >&2
            usage >&2
            exit 1 ;;
    esac
done

RUN_OUTPUT="$OUTPUT_ROOT/$LABEL"
mkdir -p "$RUN_OUTPUT"

CANARY_CASES=(
    "fishing1-high-canary::fgpilot fishing1 lowtide 0 night 1 holiday 0 raft-stage 4 island-pos -154 54 perf-log noloop seed $SEED"
    "visitor3-high-canary::fgpilot visitor3 lowtide 0 night 1 holiday 0 raft-stage 4 island-pos -154 54 perf-log noloop seed $SEED"
    "visitor3-low-canary::fgpilot visitor3 lowtide 1 night 1 holiday 0 raft-stage 4 island-pos -154 54 perf-log noloop seed $SEED"
    "walkstuf1-high-canary::fgpilot walkstuf1 lowtide 0 night 1 holiday 0 raft-stage 4 island-pos -154 54 perf-log noloop seed $SEED"
    "walkstuf1-low-canary::fgpilot walkstuf1 lowtide 1 night 1 holiday 0 raft-stage 4 island-pos -154 54 perf-log noloop seed $SEED"
    "building2-high-canary::fgpilot building2 lowtide 0 night 1 holiday 0 raft-stage 4 island-pos -154 54 perf-log noloop seed $SEED"
    "building2-low-canary::fgpilot building2 lowtide 1 night 1 holiday 0 raft-stage 4 island-pos -154 54 perf-log noloop seed $SEED"
    "building4-high-canary::fgpilot building4 lowtide 0 night 1 holiday 0 raft-stage 4 island-pos -154 54 perf-log noloop seed $SEED"
    "building4-low-canary::fgpilot building4 lowtide 1 night 1 holiday 0 raft-stage 4 island-pos -154 54 perf-log noloop seed $SEED"
    "building6-high-canary::fgpilot building6 lowtide 0 night 1 holiday 0 raft-stage 4 island-pos -154 54 perf-log noloop seed $SEED"
    "building6-low-canary::fgpilot building6 lowtide 1 night 1 holiday 0 raft-stage 4 island-pos -154 54 perf-log noloop seed $SEED"
    "activity9-high-canary::fgpilot activity9 lowtide 0 night 1 holiday 0 raft-stage 4 island-pos -154 54 perf-log noloop seed $SEED"
    "activity9-low-canary::fgpilot activity9 lowtide 1 night 1 holiday 0 raft-stage 4 island-pos -154 54 perf-log noloop seed $SEED"
)

CASE_ARGS=()
{
    printf '# PS1 perf canary baseline\n'
    printf 'label=%s\n' "$LABEL"
    printf 'frames=%s\n' "$FRAMES"
    printf 'timeout=%s\n' "$TIMEOUT"
    printf 'seed=%s\n' "$SEED"
    printf 'git=%s\n' "$(git rev-parse --short=12 HEAD)"
    printf '\n'
    for case_value in "${CANARY_CASES[@]}"; do
        printf '%s\n' "$case_value"
        CASE_ARGS+=(--case "$case_value")
    done
} > "$RUN_OUTPUT/cases.txt"

COMMAND=(
    "$SCRIPT_DIR/ps1-perf-iterate.sh"
    "${CASE_ARGS[@]}"
    --frames "$FRAMES"
    --timeout "$TIMEOUT"
    --case-local-cd
    --output "$RUN_OUTPUT"
)

if [ "$SKIP_BUILD" -eq 0 ]; then
    "$SCRIPT_DIR/build-ps1.sh"
fi

if [ "$DRY_RUN" -eq 1 ]; then
    printf '%q ' "${COMMAND[@]}"
    printf '\n'
    echo "Case manifest: $RUN_OUTPUT/cases.txt"
    exit 0
fi

"${COMMAND[@]}"

SUMMARY_PATH="$(find "$RUN_OUTPUT" -mindepth 2 -maxdepth 2 -name summary.json -type f | sort | tail -1)"
if [ -z "$SUMMARY_PATH" ]; then
    echo "ERROR: canary run completed without a summary.json under $RUN_OUTPUT" >&2
    exit 1
fi

printf '%s\n' "$SUMMARY_PATH" > "$OUTPUT_ROOT/latest-canary-baseline.txt"
echo "Canary baseline summary: $SUMMARY_PATH"
echo "Latest pointer: $OUTPUT_ROOT/latest-canary-baseline.txt"
