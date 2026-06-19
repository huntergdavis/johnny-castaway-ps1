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
SUZY1_MIN_FRAMES="${PS1_PERF_SUZY1_FRAMES:-12000}"
MARY1_MIN_FRAMES="${PS1_PERF_MARY1_FRAMES:-9000}"
BUILDING3_MIN_FRAMES="${PS1_PERF_BUILDING3_FRAMES:-9600}"
TIMEOUT="${PS1_PERF_TIMEOUT:-220}"
OUTPUT_ROOT="${PS1_PERF_OUTPUT_DIR:-$PROJECT_ROOT/scratch/ps1-perf-iterate}"
SHEET="$PROJECT_ROOT/docs/ps1/performance-scene-matrix.csv"
STATS_VERSION="${PS1_PERF_STATS_VERSION:-}"
UPDATE_SHEET=1
SKIP_MEASURED=0
CONTINUE_ON_FAIL=0
CASE_RETRIES="${PS1_PERF_CASE_RETRIES:-1}"
JOBS="${PS1_PERF_JOBS:-1}"
RESUME_OUTPUT=0

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
                             suzy1 cases are raised to PS1_PERF_SUZY1_FRAMES
                             or 12000, mary1 to PS1_PERF_MARY1_FRAMES or 9000,
                             and building3 to PS1_PERF_BUILDING3_FRAMES or 9600
                             when this value is lower, because those scenes
                             reach JCPERF2 after the default budget.
  --timeout N                Wall-clock timeout per case (default: PS1_PERF_TIMEOUT or 220).
  --output DIR               Perf output root (default: scratch/ps1-perf-iterate).
  --sheet PATH               CSV sheet to refresh after a successful run.
  --stats-version VERSION    Stamp generated CSV metrics with this version.
  --no-sheet                 Do not refresh the CSV sheet.
  --skip-measured            Skip rows already marked measured in the sheet.
  --only-pending             Alias for --skip-measured.
  --skip-build               Pass --skip-build to each ps1-perf-iterate case.
                             The executable is reused; each case still stages
                             BOOTMODE.TXT and remakes the CD image.
  --retries N                Retry a failed case up to N times before marking
                             it failed (default: PS1_PERF_CASE_RETRIES or 1).
  --jobs N                   Run up to N cases in parallel. Values greater
                             than 1 use --skip-build --case-local-cd so cases
                             do not share root BOOTMODE.TXT or johnnycastawayps1.cue.
  --resume-output            Skip cases that already have a passing
                             perf-summary.json under --output.
  --continue-on-fail         Keep running later cases if one case fails.
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
        --stats-version)
            STATS_VERSION="$2"; shift 2 ;;
        --no-sheet)
            UPDATE_SHEET=0; shift ;;
        --skip-measured|--only-pending)
            SKIP_MEASURED=1; shift ;;
        --skip-build)
            PERF_ARGS+=(--skip-build); shift ;;
        --retries)
            CASE_RETRIES="$2"; shift 2 ;;
        --jobs)
            JOBS="$2"; shift 2 ;;
        --resume-output)
            RESUME_OUTPUT=1; shift ;;
        --continue-on-fail)
            CONTINUE_ON_FAIL=1; shift ;;
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
if ! [[ "$CASE_RETRIES" =~ ^[0-9]+$ ]]; then
    echo "ERROR: --retries must be a non-negative integer." >&2
    exit 1
fi
if ! [[ "$FRAMES" =~ ^[0-9]+$ ]] || [ "$FRAMES" -lt 1 ]; then
    echo "ERROR: --frames must be a positive integer." >&2
    exit 1
fi
if ! [[ "$SUZY1_MIN_FRAMES" =~ ^[0-9]+$ ]] || [ "$SUZY1_MIN_FRAMES" -lt 1 ]; then
    echo "ERROR: PS1_PERF_SUZY1_FRAMES must be a positive integer." >&2
    exit 1
fi
if ! [[ "$TIMEOUT" =~ ^[0-9]+$ ]] || [ "$TIMEOUT" -lt 1 ]; then
    echo "ERROR: --timeout must be a positive integer." >&2
    exit 1
fi
if ! [[ "$JOBS" =~ ^[0-9]+$ ]] || [ "$JOBS" -lt 1 ]; then
    echo "ERROR: --jobs must be a positive integer." >&2
    exit 1
fi
if [ "$JOBS" -gt 1 ]; then
    PERF_ARGS+=(--skip-build --case-local-cd)
fi

MANIFEST_ARGS=(--print-cases --tides "$TIDES" --order "$ORDER" --seed "$SEED")
if [ -n "$LIMIT" ]; then
    MANIFEST_ARGS+=(--limit "$LIMIT")
fi
if [ "$SKIP_MEASURED" -eq 1 ] && [ -f "$SHEET" ]; then
    MANIFEST_ARGS+=(--skip-measured-from "$SHEET")
fi

CASE_ARGS=()
while IFS=$'\t' read -r label boot; do
    [ -n "$label" ] || continue
    CASE_ARGS+=(--case "${label}::${boot}")
done < <("$SCRIPT_DIR/ps1-foreground-scene-manifest.py" "${MANIFEST_ARGS[@]}")

if [ "$RESUME_OUTPUT" -eq 1 ]; then
    declare -A RESUME_LABELS=()
    while IFS= read -r label; do
        [ -n "$label" ] || continue
        RESUME_LABELS["$label"]=1
    done < <(python3 - "$OUTPUT_ROOT" <<'PY'
import json
import sys
from pathlib import Path

root = Path(sys.argv[1])
if not root.exists():
    raise SystemExit(0)

labels = set()
for path in root.glob("*/**/perf-summary.json"):
    try:
        summary = json.loads(path.read_text(encoding="utf-8"))
    except Exception:
        continue
    if not summary.get("gate", {}).get("pass"):
        continue
    boot = str(summary.get("boot", "")).split()
    try:
        scene = boot[boot.index("fgpilot") + 1]
    except (ValueError, IndexError):
        continue
    tide = "high"
    if "lowtide" in boot:
        try:
            tide = "low" if boot[boot.index("lowtide") + 1] == "1" else "high"
        except IndexError:
            pass
    labels.add(f"{scene}-{tide}")

for label in sorted(labels):
    print(label)
PY
)
    FILTERED_CASE_ARGS=()
    SKIPPED_EXISTING=0
    for ((i = 0; i < ${#CASE_ARGS[@]}; i += 2)); do
        case_value="${CASE_ARGS[$((i + 1))]}"
        label="${case_value%%::*}"
        if [ "${RESUME_LABELS[$label]+x}" ]; then
            SKIPPED_EXISTING=$((SKIPPED_EXISTING + 1))
            continue
        fi
        FILTERED_CASE_ARGS+=("${CASE_ARGS[$i]}" "$case_value")
    done
    CASE_ARGS=("${FILTERED_CASE_ARGS[@]}")
    echo "Resume output: skipped $SKIPPED_EXISTING already-passing cases from $OUTPUT_ROOT"
fi

if [ "${#CASE_ARGS[@]}" -eq 0 ]; then
    echo "ERROR: no perf cases generated." >&2
    exit 1
fi

case_frame_budget() {
    local case_value="$1"
    local label="${case_value%%::*}"
    local frames="$FRAMES"

    if [[ "$label" == suzy1-* ]] && [ "$frames" -lt "$SUZY1_MIN_FRAMES" ]; then
        frames="$SUZY1_MIN_FRAMES"
    fi
    if [[ "$label" == mary1-* || "$label" == mary1 ]] && [ "$frames" -lt "$MARY1_MIN_FRAMES" ]; then
        frames="$MARY1_MIN_FRAMES"
    fi
    if [[ "$label" == building3-* || "$label" == building3 ]] && [ "$frames" -lt "$BUILDING3_MIN_FRAMES" ]; then
        frames="$BUILDING3_MIN_FRAMES"
    fi
    printf '%s\n' "$frames"
}

max_case_frame_budget() {
    local max_frames="$FRAMES"
    local case_frames

    for ((i = 0; i < ${#CASE_ARGS[@]}; i += 2)); do
        case_frames="$(case_frame_budget "${CASE_ARGS[$((i + 1))]}")"
        if [ "$case_frames" -gt "$max_frames" ]; then
            max_frames="$case_frames"
        fi
    done
    printf '%s\n' "$max_frames"
}

run_case_with_retries() {
    local case_flag="$1"
    local case_value="$2"
    local case_status=1
    local case_frames
    local attempt

    case_frames="$(case_frame_budget "$case_value")"

    for ((attempt = 0; attempt <= CASE_RETRIES; attempt += 1)); do
        if [ "$attempt" -gt 0 ]; then
            echo "WARN: retrying case attempt $((attempt + 1))/$((CASE_RETRIES + 1)): $case_value" >&2
        fi
        "$SCRIPT_DIR/ps1-perf-iterate.sh" \
            "$case_flag" "$case_value" \
            --frames "$case_frames" \
            --timeout "$TIMEOUT" \
            --output "$OUTPUT_ROOT" \
            "${PERF_ARGS[@]}"
        case_status=$?
        if [ "$case_status" -eq 0 ]; then
            break
        fi
    done
    return "$case_status"
}

set +e
if [ "$JOBS" -gt 1 ]; then
    PERF_STATUS=0
    FAILED_CASES=()
    JOB_LOG_DIR="$OUTPUT_ROOT/job-logs"
    JOB_STATUS_DIR="$OUTPUT_ROOT/job-status"
    mkdir -p "$JOB_LOG_DIR" "$JOB_STATUS_DIR"

    for ((i = 0; i < ${#CASE_ARGS[@]}; i += 2)); do
        case_value="${CASE_ARGS[$((i + 1))]}"
        label="${case_value%%::*}"
        echo "Launching perf case ($((i / 2 + 1))/$(( ${#CASE_ARGS[@]} / 2 ))): $label"
        (
            run_case_with_retries "${CASE_ARGS[$i]}" "$case_value"
            status=$?
            printf '%s\n' "$status" > "$JOB_STATUS_DIR/$label.status"
            exit "$status"
        ) > "$JOB_LOG_DIR/$label.log" 2>&1 &

        while [ "$(jobs -rp | wc -l)" -ge "$JOBS" ]; do
            wait -n || true
        done
    done
    wait || true

    for ((i = 0; i < ${#CASE_ARGS[@]}; i += 2)); do
        case_value="${CASE_ARGS[$((i + 1))]}"
        label="${case_value%%::*}"
        status_file="$JOB_STATUS_DIR/$label.status"
        if [ ! -f "$status_file" ]; then
            PERF_STATUS=1
            FAILED_CASES+=("$case_value status=missing")
            continue
        fi
        CASE_STATUS="$(cat "$status_file")"
        if [ "$CASE_STATUS" -ne 0 ]; then
            PERF_STATUS=1
            FAILED_CASES+=("$case_value status=$CASE_STATUS log=$JOB_LOG_DIR/$label.log")
            if [ "$CONTINUE_ON_FAIL" -eq 0 ]; then
                :
            fi
        fi
    done
    if [ "${#FAILED_CASES[@]}" -gt 0 ]; then
        printf 'Failed cases:\n' >&2
        printf '  %s\n' "${FAILED_CASES[@]}" >&2
    fi
elif [ "$CONTINUE_ON_FAIL" -eq 1 ]; then
    PERF_STATUS=0
    FAILED_CASES=()
    for ((i = 0; i < ${#CASE_ARGS[@]}; i += 2)); do
        run_case_with_retries "${CASE_ARGS[$i]}" "${CASE_ARGS[$((i + 1))]}"
        CASE_STATUS=$?
        if [ "$CASE_STATUS" -ne 0 ]; then
            PERF_STATUS=1
            FAILED_CASES+=("${CASE_ARGS[$((i + 1))]} status=$CASE_STATUS")
            echo "WARN: continuing after failed case: ${CASE_ARGS[$((i + 1))]} status=$CASE_STATUS" >&2
        fi
    done
    if [ "${#FAILED_CASES[@]}" -gt 0 ]; then
        printf 'Failed cases:\n' >&2
        printf '  %s\n' "${FAILED_CASES[@]}" >&2
    fi
else
    PERF_STATUS=1
    BATCH_FRAMES="$(max_case_frame_budget)"
    if [ "$BATCH_FRAMES" -gt "$FRAMES" ]; then
        echo "Using --frames $BATCH_FRAMES for this batch because at least one case needs the longer scene budget." >&2
    fi
    for ((attempt = 0; attempt <= CASE_RETRIES; attempt += 1)); do
        if [ "$attempt" -gt 0 ]; then
            echo "WARN: retrying full case batch attempt $((attempt + 1))/$((CASE_RETRIES + 1))" >&2
        fi
        "$SCRIPT_DIR/ps1-perf-iterate.sh" \
            "${CASE_ARGS[@]}" \
            --frames "$BATCH_FRAMES" \
            --timeout "$TIMEOUT" \
            --output "$OUTPUT_ROOT" \
            "${PERF_ARGS[@]}"
        PERF_STATUS=$?
        if [ "$PERF_STATUS" -eq 0 ]; then
            break
        fi
    done
fi
set -e

if [ "$UPDATE_SHEET" -eq 1 ]; then
    if [ -z "$STATS_VERSION" ]; then
        STATS_VERSION="git:$(git rev-parse --short=8 HEAD)"
    fi
    SUMMARY_ARGS=()
    while IFS= read -r summary; do
        SUMMARY_ARGS+=(--summary "$summary")
    done < <(find "$OUTPUT_ROOT" -maxdepth 3 \
        \( -name 'summary.json' -o -name 'perf-summary.json' -o -name 'current-*.json' \) -print | sort)
    "$SCRIPT_DIR/ps1-foreground-scene-manifest.py" \
        --write-sheet "$SHEET" \
        --merge-existing-sheet "$SHEET" \
        --stats-version "$STATS_VERSION" \
        "${SUMMARY_ARGS[@]}"
    echo "Updated sheet: $SHEET"
fi

exit "$PERF_STATUS"
