#!/bin/bash
# Headless PS1 performance iteration harness for fgpilot scene playback.
#
# This intentionally does not call the legacy regtest scene-certification
# wrappers. It uses only the headless DuckStation regtest binary/image, then
# applies this project's JCPERF2 gates directly. It is built for the loop:
#   edit one thing -> run headless perf gates -> commit on pass or roll back.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$PROJECT_ROOT"

# shellcheck source=./docker-common.sh
source "$PROJECT_ROOT/scripts/docker-common.sh"

if [ -f "$PROJECT_ROOT/config/ps1/regtest-config.sh" ]; then
    # shellcheck source=../config/ps1/regtest-config.sh
    source "$PROJECT_ROOT/config/ps1/regtest-config.sh"
fi

LOCK_FILE="${REGTEST_LOCK_FILE:-$PROJECT_ROOT/.regtest-build.lock}"
BOOTMODE_FILE="$PROJECT_ROOT/config/ps1/BOOTMODE.TXT"
EMBEDDED_BOOTMODE_HEADER="$PROJECT_ROOT/config/ps1/bootmode_embedded.h"

OUTPUT_ROOT="${PS1_PERF_OUTPUT_DIR:-$PROJECT_ROOT/scratch/ps1-perf-iterate}"
EXPERIMENT_LOG="${PS1_PERF_EXPERIMENT_LOG:-$OUTPUT_ROOT/experiments.jsonl}"
FRAMES="${PS1_PERF_FRAMES:-7200}"
INTERVAL="${PS1_PERF_INTERVAL:-999999}"
TIMEOUT="${PS1_PERF_TIMEOUT:-${REGTEST_TIMEOUT:-180}}"
LOG_LEVEL="${PS1_PERF_LOG_LEVEL:-Warning}"
PERF_TOKEN="perf-log"
BUILD_MODE="incremental"
BASELINE_FILE=""
WRITE_BASELINE=""
COMMIT_MESSAGE=""
ROLLBACK_ON_FAIL=0
ALLOW_REGRESSION_PERCENT="${PS1_PERF_ALLOW_REGRESSION_PERCENT:-2}"
REQUIRE_IMPROVEMENT=0
CHECK_ENV_ONLY=0
NO_SEED=0
SEED="${REGTEST_SEED:-1}"

CASE_LABELS=()
CASE_BOOTS=()

usage() {
    cat <<'USAGE'
Usage: ps1-perf-iterate.sh [options]

Headless fgpilot performance runner. Builds the PS1 image with a staged
BOOTMODE string, runs DuckStation's headless regtest binary directly without
opening the GUI or invoking legacy regtest wrappers, extracts JCPERF2 metrics,
and enforces correctness/performance gates.

Common modes:
  ./scripts/ps1-perf-iterate.sh --matrix
  ./scripts/ps1-perf-iterate.sh --scene fishing1
  ./scripts/ps1-perf-iterate.sh --case "fishing1-low::fgpilot fishing1 lowtide 1 perf-log noloop seed 1"
  ./scripts/ps1-perf-iterate.sh --scene fishing1 --baseline scratch/baseline.json
  ./scripts/ps1-perf-iterate.sh --scene fishing1 --commit-on-pass "ps1: optimize foo"

Options:
  --scene NAME             Add fgpilot scene case using default perf args.
  --case LABEL::BOOT       Add explicit case label and BOOTMODE string.
  --boot STRING            Add one custom case named "custom".
  --matrix                 Add default fishing1 high/low, fishing2, fishing3 cases.
  --perf-log               Use perf-log token (default).
  --perf-detail            Use perf-detail token.
  --perf-debug             Use perf-debug token.
  --frames N               Emulated frames per case (default: 7200).
  --interval N             Screenshot dump interval (default: 999999).
  --timeout N              Wall-clock timeout per case (default: REGTEST_TIMEOUT or 180).
  --log LEVEL              DuckStation log level (default: Warning).
  --output DIR             Output root (default: scratch/ps1-perf-iterate).
  --experiment-log FILE    Append one JSONL record per attempted case
                           (default: <output>/experiments.jsonl).
  --clean                  Clean PS1 build before each case.
  --baseline FILE          Compare against a prior summary JSON.
  --write-baseline FILE    Also copy this run summary to FILE.
  --allow-regression PCT   Fail if key metrics regress by more than PCT (default: 2).
  --require-improvement    With --baseline, require at least one key speed metric to improve.
  --commit-on-pass MSG     git add -A and commit with MSG after all gates pass.
  --rollback-on-fail       On gate failure, restore tracked worktree files to HEAD.
                           Only use when the current dirty tree is exactly the experiment.
  --seed N                 Append seed N to default --scene/--matrix boots (default: REGTEST_SEED or 1).
  --no-seed                Do not append a seed to generated boot strings.
  --check-env              Check headless regtest prerequisites and exit.
  -h, --help               Show this help.

Gate rules:
  - JCPERF2 must be present.
  - correctness trip/fallback/stale/frame/sound/CD counters must be zero.
  - gfx full_fallbacks must be zero.
  - with --baseline, loop_vb, timing overrun_vb, blocking_vb, and prefetch
    overrun_vb must not regress beyond --allow-regression.

Outputs:
  <output>/<timestamp>/summary.json
  <output>/<timestamp>/<case>/headless-regtest.log
  <output>/<timestamp>/<case>/perf-summary.json
  <output>/experiments.jsonl
USAGE
}

slugify() {
    printf '%s' "$1" | tr '[:upper:]' '[:lower:]' | sed -E 's/[^a-z0-9._-]+/-/g; s/^-+//; s/-+$//'
}

append_perf_defaults() {
    local boot="$1"
    if [[ "$boot" != *" perf-log"* && "$boot" != *" perf-detail"* && "$boot" != *" perf-debug"* ]]; then
        boot="$boot $PERF_TOKEN"
    fi
    if [[ "$boot" != *" noloop"* && "$boot" != *" loop"* ]]; then
        boot="$boot noloop"
    fi
    if [ "$NO_SEED" -eq 0 ] &&
       [[ "$boot" != *" seed "* ]] &&
       [[ "$boot" != seed\ * ]] &&
       [[ "$boot" != *" seed" ]]; then
        boot="$boot seed $SEED"
    fi
    printf '%s\n' "$boot"
}

add_case() {
    local label="$1"
    local boot="$2"
    CASE_LABELS+=("$label")
    CASE_BOOTS+=("$boot")
}

add_matrix_cases() {
    add_case "fishing1-high" "fgpilot fishing1 lowtide 0"
    add_case "fishing1-low" "fgpilot fishing1 lowtide 1"
    add_case "fishing2" "fgpilot fishing2"
    add_case "fishing3" "fgpilot fishing3"
}

check_env() {
    local ok=1
    if ! docker_maybe_init; then
        echo "ERROR: Docker is required for headless PS1 perf iteration." >&2
        ok=0
    fi
    if [ "$ok" -eq 1 ] && ! "${DOCKER_CMD[@]}" image inspect jc-reborn-regtest:latest >/dev/null 2>&1; then
        echo "ERROR: Docker image jc-reborn-regtest:latest is missing." >&2
        echo "Build it once with: ./scripts/build-regtest-image.sh" >&2
        ok=0
    fi
    if [ "$ok" -eq 1 ] && ! "${DOCKER_CMD[@]}" image inspect jc-reborn-ps1-dev:amd64 >/dev/null 2>&1; then
        echo "ERROR: Docker image jc-reborn-ps1-dev:amd64 is missing." >&2
        echo "Build/setup the PS1 toolchain image before running perf iteration." >&2
        ok=0
    fi
    if [ ! -d "$HOME/.var/app/org.duckstation.DuckStation/config/duckstation/bios" ] &&
       [ ! -d "$HOME/.local/share/duckstation/bios" ] &&
       [ ! -d "$HOME/.config/duckstation/bios" ]; then
        echo "ERROR: no DuckStation BIOS directory found." >&2
        ok=0
    fi
    if [ "$ok" -eq 1 ]; then
        echo "Headless PS1 perf environment is ready."
        return 0
    fi
    return 1
}

find_bios_dir() {
    local candidate
    for candidate in \
        "$HOME/.var/app/org.duckstation.DuckStation/config/duckstation/bios" \
        "$HOME/.local/share/duckstation/bios" \
        "$HOME/.config/duckstation/bios" \
        "$HOME/ps1-bios" \
        "$PROJECT_ROOT/bios"; do
        if [ -d "$candidate" ]; then
            realpath "$candidate"
            return 0
        fi
    done
    return 1
}

while [ $# -gt 0 ]; do
    case "$1" in
        --scene)
            add_case "$2" "fgpilot $2"; shift 2 ;;
        --case)
            if [[ "$2" != *"::"* ]]; then
                echo "ERROR: --case must be LABEL::BOOT" >&2
                exit 1
            fi
            add_case "${2%%::*}" "${2#*::}"; shift 2 ;;
        --boot)
            add_case "custom" "$2"; shift 2 ;;
        --matrix)
            add_matrix_cases; shift ;;
        --perf-log)
            PERF_TOKEN="perf-log"; shift ;;
        --perf-detail)
            PERF_TOKEN="perf-detail"; shift ;;
        --perf-debug)
            PERF_TOKEN="perf-debug"; shift ;;
        --frames)
            FRAMES="$2"; shift 2 ;;
        --interval)
            INTERVAL="$2"; shift 2 ;;
        --timeout)
            TIMEOUT="$2"; shift 2 ;;
        --log)
            LOG_LEVEL="$2"; shift 2 ;;
        --output)
            OUTPUT_ROOT="$2"; shift 2 ;;
        --experiment-log)
            EXPERIMENT_LOG="$2"; shift 2 ;;
        --clean)
            BUILD_MODE="clean"; shift ;;
        --baseline)
            BASELINE_FILE="$2"; shift 2 ;;
        --write-baseline)
            WRITE_BASELINE="$2"; shift 2 ;;
        --allow-regression)
            ALLOW_REGRESSION_PERCENT="$2"; shift 2 ;;
        --require-improvement)
            REQUIRE_IMPROVEMENT=1; shift ;;
        --commit-on-pass)
            COMMIT_MESSAGE="$2"; shift 2 ;;
        --rollback-on-fail)
            ROLLBACK_ON_FAIL=1; shift ;;
        --seed)
            SEED="$2"; NO_SEED=0; shift 2 ;;
        --no-seed)
            NO_SEED=1; shift ;;
        --check-env)
            CHECK_ENV_ONLY=1; shift ;;
        -h|--help)
            usage; exit 0 ;;
        *)
            echo "Unknown option: $1" >&2
            usage >&2
            exit 1 ;;
    esac
done

if [ "$CHECK_ENV_ONLY" -eq 1 ]; then
    check_env
    exit $?
fi

if [ "${#CASE_LABELS[@]}" -eq 0 ]; then
    add_case "fishing1" "fgpilot fishing1"
fi

if ! [[ "$FRAMES" =~ ^[0-9]+$ ]] || [ "$FRAMES" -le 0 ]; then
    echo "ERROR: --frames must be a positive integer." >&2
    exit 1
fi
if ! [[ "$INTERVAL" =~ ^[0-9]+$ ]] || [ "$INTERVAL" -le 0 ]; then
    echo "ERROR: --interval must be a positive integer." >&2
    exit 1
fi
if ! [[ "$TIMEOUT" =~ ^[0-9]+$ ]] || [ "$TIMEOUT" -le 0 ]; then
    echo "ERROR: --timeout must be a positive integer." >&2
    exit 1
fi
if [ -n "$BASELINE_FILE" ] && [ ! -f "$BASELINE_FILE" ]; then
    echo "ERROR: baseline file not found: $BASELINE_FILE" >&2
    exit 1
fi

check_env

RUN_ID="$(date +%Y%m%d-%H%M%S)"
RUN_ROOT="$OUTPUT_ROOT/$RUN_ID"
mkdir -p "$RUN_ROOT"
mkdir -p "$(dirname "$EXPERIMENT_LOG")"
GIT_COMMIT="$(git rev-parse --short HEAD 2>/dev/null || printf 'unknown')"
GIT_BRANCH="$(git branch --show-current 2>/dev/null || printf 'unknown')"
if git diff --quiet --ignore-submodules -- 2>/dev/null &&
   git diff --cached --quiet --ignore-submodules -- 2>/dev/null; then
    GIT_DIRTY=0
else
    GIT_DIRTY=1
fi

BOOTMODE_BACKUP="$(mktemp /tmp/ps1-perf-bootmode-XXXXXX.txt)"
BOOTMODE_WAS_PRESENT=0
if [ -f "$BOOTMODE_FILE" ]; then
    cp "$BOOTMODE_FILE" "$BOOTMODE_BACKUP"
    BOOTMODE_WAS_PRESENT=1
fi

HEADER_BACKUP="$(mktemp /tmp/ps1-perf-bootheader-XXXXXX.h)"
HEADER_WAS_PRESENT=0
if [ -f "$EMBEDDED_BOOTMODE_HEADER" ]; then
    cp "$EMBEDDED_BOOTMODE_HEADER" "$HEADER_BACKUP"
    HEADER_WAS_PRESENT=1
fi

RESTORED_BOOT_FILES=0
restore_boot_files() {
    if [ "$RESTORED_BOOT_FILES" -eq 1 ]; then
        return
    fi
    if [ "$BOOTMODE_WAS_PRESENT" -eq 1 ]; then
        cp "$BOOTMODE_BACKUP" "$BOOTMODE_FILE"
    else
        rm -f "$BOOTMODE_FILE"
    fi
    if [ "$HEADER_WAS_PRESENT" -eq 1 ]; then
        cp "$HEADER_BACKUP" "$EMBEDDED_BOOTMODE_HEADER"
    else
        rm -f "$EMBEDDED_BOOTMODE_HEADER"
    fi
    rm -f "$BOOTMODE_BACKUP" "$HEADER_BACKUP"
    RESTORED_BOOT_FILES=1
}

cleanup() {
    restore_boot_files
}
trap cleanup EXIT

exec 9>"$LOCK_FILE"
if command -v flock >/dev/null 2>&1; then
    echo "Waiting for PS1 perf lock: $LOCK_FILE"
    flock 9
else
    echo "ERROR: flock is required." >&2
    exit 1
fi

parse_case_metrics() {
    local label="$1"
    local boot="$2"
    local case_dir="$3"
    local log_file="$4"
    local out_file="$5"

    python3 - "$label" "$boot" "$case_dir" "$log_file" > "$out_file" <<'PY'
import json
import re
import sys
from pathlib import Path

label, boot, case_dir, log_file = sys.argv[1:5]
log_path = Path(log_file)
ansi_re = re.compile(r"\x1b\[[0-9;?]*[A-Za-z]")

def parse_value(value):
    value = ansi_re.sub("", value)
    if value.startswith("0x"):
        try:
            return int(value, 16)
        except ValueError:
            return value
    try:
        return int(value)
    except ValueError:
        return value

sections = {}
legacy = []
tty_lines = []
if log_path.is_file():
    for raw in log_path.read_text(encoding="utf-8", errors="ignore").splitlines():
        line = ansi_re.sub("", raw)
        if "TTY:" in line:
            line = line.split("TTY:", 1)[1].strip()
            tty_lines.append(line)
        if "JCPERF " in line:
            legacy.append(line[line.index("JCPERF "):].strip())
        marker = "JCPERF2 "
        if marker not in line:
            continue
        payload = line[line.index(marker) + len(marker):].strip()
        if not payload:
            continue
        parts = payload.split()
        section = parts[0]
        data = {}
        for token in parts[1:]:
            if "=" not in token:
                continue
            key, value = token.split("=", 1)
            data[key] = parse_value(value)
        sections[section] = data

def get(section, key, default=0):
    value = sections.get(section, {}).get(key, default)
    return value if isinstance(value, int) else default

failures = []
warnings = []
if not sections:
    failures.append("missing_jcperf2")

zero_required = [
    ("correctness", "trip"),
    ("correctness", "fallback"),
    ("correctness", "stale_guard"),
    ("correctness", "frame_mismatch"),
    ("correctness", "sound_late"),
    ("correctness", "cd_fail"),
    ("cd", "fail"),
    ("gfx", "full_fallbacks"),
]
for section, key in zero_required:
    value = get(section, key, 0)
    if value != 0:
        failures.append(f"{section}.{key}={value}")

sound_events = get("correctness", "sound_events", 0)
sound_cursor_end = get("correctness", "sound_cursor_end", sound_events)
if sound_events != sound_cursor_end:
    failures.append(f"sound_cursor_end={sound_cursor_end} sound_events={sound_events}")

expected_frames = get("correctness", "expected_frames", 0)
last_frame = get("correctness", "last_frame", -1)
if expected_frames > 0 and last_frame >= expected_frames:
    failures.append(f"last_frame={last_frame} expected_frames={expected_frames}")

loop_vb = get("timing", "loop_vb", 0)
target_vb = get("timing", "target_vb", 0)
blocking_vb = get("cd", "blocking_vb", 0)
upload_bytes = get("gfx", "upload_bytes", 0)
restore_bytes = get("gfx", "restore_bytes", 0)
compose_pixels = get("gfx", "compose_pixels", get("frame", "pixels", 0))

suggestions = []
if blocking_vb > 0:
    suggestions.append("CD/prefetch: blocking_vb remains nonzero")
if get("prefetch", "overrun_vb", 0) > 0:
    suggestions.append("CD/prefetch: refill work is overrunning held slack")
if upload_bytes > 0 and target_vb > 0:
    suggestions.append("dirty-upload: upload byte volume is measurable")
if restore_bytes > 0:
    suggestions.append("dirty-restore: clean-rect restore byte volume is measurable")
if compose_pixels > 0:
    suggestions.append("compose: compositor work is measurable")
if get("render", "present_wait_vb", 0) > 0:
    suggestions.append("present: present_wait_vb is nonzero")

summary = {
    "label": label,
    "boot": boot,
    "case_dir": str(Path(case_dir).resolve()),
    "log_file": str(log_path.resolve()),
    "sections": sections,
    "legacy_jcperf": legacy,
    "gate": {
        "pass": not failures,
        "failures": failures,
        "warnings": warnings,
    },
    "derived": {
        "loop_ratio": (loop_vb / target_vb) if target_vb else None,
        "loop_vb": loop_vb,
        "target_vb": target_vb,
        "blocking_vb": blocking_vb,
        "prefetch_policy": sections.get("prefetch", {}).get("policy"),
        "suggestions": suggestions,
    },
}
json.dump(summary, sys.stdout, indent=2)
print()
PY
}

append_experiment_log() {
    local summary_file="$1"
    local regtest_exit="$2"
    local attempt_status="$3"
    local failure_reason="$4"

    python3 - "$EXPERIMENT_LOG" "$summary_file" "$RUN_ID" "$GIT_COMMIT" "$GIT_BRANCH" \
        "$GIT_DIRTY" "$FRAMES" "$INTERVAL" "$TIMEOUT" "$LOG_LEVEL" "$BUILD_MODE" \
        "$PERF_TOKEN" "$BASELINE_FILE" "$WRITE_BASELINE" "$regtest_exit" \
        "$attempt_status" "$failure_reason" <<'PY'
import json
import sys
from datetime import datetime, timezone
from pathlib import Path

(
    log_path,
    summary_path,
    run_id,
    git_commit,
    git_branch,
    git_dirty,
    frames,
    interval,
    timeout,
    log_level,
    build_mode,
    perf_token,
    baseline_file,
    write_baseline,
    regtest_exit,
    attempt_status,
    failure_reason,
) = sys.argv[1:18]

summary_file = Path(summary_path)
summary = {}
if summary_file.is_file():
    summary = json.loads(summary_file.read_text(encoding="utf-8"))

sections = summary.get("sections", {})
timing = sections.get("timing", {})
cd = sections.get("cd", {})
prefetch = sections.get("prefetch", {})
gfx = sections.get("gfx", {})
correctness = sections.get("correctness", {})
gate = summary.get("gate", {})

record = {
    "schema": "ps1-perf-experiment-log/v1",
    "timestamp_utc": datetime.now(timezone.utc).isoformat(),
    "run_id": run_id,
    "label": summary.get("label"),
    "boot": summary.get("boot"),
    "attempt_status": attempt_status,
    "failure_reason": failure_reason or None,
    "regtest_exit": int(regtest_exit),
    "gate_pass": gate.get("pass"),
    "gate_failures": gate.get("failures", []),
    "git": {
        "branch": git_branch,
        "commit": git_commit,
        "dirty": bool(int(git_dirty)),
    },
    "config": {
        "frames": int(frames),
        "interval": int(interval),
        "timeout": int(timeout),
        "log_level": log_level,
        "build_mode": build_mode,
        "perf_token": perf_token,
        "baseline_file": baseline_file or None,
        "write_baseline": write_baseline or None,
    },
    "paths": {
        "case_dir": summary.get("case_dir"),
        "summary_file": str(summary_file.resolve()),
        "log_file": summary.get("log_file"),
    },
    "metrics": {
        "loop_vb": timing.get("loop_vb"),
        "target_vb": timing.get("target_vb"),
        "overrun_vb": timing.get("overrun_vb"),
        "blocking_vb": cd.get("blocking_vb"),
        "loop_reads": cd.get("loop_reads"),
        "prefetch_policy": prefetch.get("policy"),
        "prefetch_hits": prefetch.get("hits"),
        "prefetch_due_misses": prefetch.get("due_misses"),
        "prefetch_overrun_vb": prefetch.get("overrun_vb"),
        "upload_bytes": gfx.get("upload_bytes"),
        "restore_bytes": gfx.get("restore_bytes"),
        "full_fallbacks": gfx.get("full_fallbacks"),
        "trip": correctness.get("trip"),
        "fallback": correctness.get("fallback"),
        "frame_mismatch": correctness.get("frame_mismatch"),
        "sound_late": correctness.get("sound_late"),
        "cd_fail": correctness.get("cd_fail"),
    },
}

Path(log_path).parent.mkdir(parents=True, exist_ok=True)
with Path(log_path).open("a", encoding="utf-8") as fh:
    fh.write(json.dumps(record, sort_keys=True) + "\n")
PY
}

case_summary_passed() {
    local summary_file="$1"
    python3 - "$summary_file" <<'PY'
import json
import sys
from pathlib import Path

path = Path(sys.argv[1])
if not path.is_file():
    raise SystemExit(1)

summary = json.loads(path.read_text(encoding="utf-8"))
sections = summary.get("sections", {})
required_sections = (
    "scene",
    "timing",
    "setup",
    "frame",
    "cd",
    "prefetch",
    "render",
    "gfx",
    "heap",
    "correctness",
)
if not all(name in sections for name in required_sections):
    raise SystemExit(1)
if not summary.get("gate", {}).get("pass"):
    raise SystemExit(1)
raise SystemExit(0)
PY
}

duckstation_exited_successfully() {
    local log_file="$1"
    [ -f "$log_file" ] && grep -q "Exiting with success" "$log_file"
}

run_headless_regtest() {
    local cue_file="$1"
    local out_dir="$2"
    local log_file="$3"
    local bios_dir=""
    local cue_dir
    local cue_name
    local case_slug
    local container_name
    local cid_file

    cue_file="$(realpath "$cue_file")"
    cue_dir="$(dirname "$cue_file")"
    cue_name="$(basename "$cue_file")"
    case_slug="$(basename "$(dirname "$out_dir")")"
    container_name="$(printf 'jc-ps1-perf-%s-%s-%s' "$RUN_ID" "$case_slug" "$$" | tr -c 'A-Za-z0-9_.-' '-')"
    cid_file="$out_dir/container.cid"
    mkdir -p "$out_dir/frames"
    rm -f "$cid_file"
    "${DOCKER_CMD[@]}" rm -f "$container_name" >/dev/null 2>&1 || true

    if bios_dir="$(find_bios_dir)"; then
        :
    else
        bios_dir=""
    fi

    {
        echo "schema=ps1-perf-headless/v1"
        echo "cue=$cue_file"
        echo "frames=$FRAMES"
        echo "interval=$INTERVAL"
        echo "timeout=$TIMEOUT"
        echo "log_level=$LOG_LEVEL"
        echo "bios_dir=${bios_dir:-<not-found>}"
        echo "renderer=Software"
        echo "container_name=$container_name"
    } > "$out_dir/headless-run.txt"

    local docker_args=(
        "${DOCKER_CMD[@]}" run --rm
        --platform linux/amd64
        --name "$container_name"
        --cidfile "$cid_file"
        -v "${cue_dir}:/game:ro"
        -v "$(realpath "$out_dir"):/output"
    )
    if [ -n "$bios_dir" ]; then
        docker_args+=(-v "${bios_dir}:/bios:ro")
        docker_args+=(-v "${bios_dir}:/root/.local/share/duckstation/bios:ro")
    fi
    docker_args+=(jc-reborn-regtest:latest)

    local regtest_args=(
        -log "$LOG_LEVEL"
        -console
        -renderer Software
        -frames "$FRAMES"
        -dumpdir /output/frames
        -dumpinterval "$INTERVAL"
        -- "/game/${cue_name}"
    )

    local cmd=()
    if command -v timeout >/dev/null 2>&1; then
        cmd=(timeout "${TIMEOUT}s" "${docker_args[@]}" "${regtest_args[@]}")
    else
        cmd=("${docker_args[@]}" "${regtest_args[@]}")
    fi

    "${cmd[@]}" > "$log_file" 2>&1 &
    local pid=$!
    local start_time
    start_time="$(date +%s)"

    while kill -0 "$pid" >/dev/null 2>&1; do
        sleep 15
        if kill -0 "$pid" >/dev/null 2>&1; then
            local now elapsed size
            now="$(date +%s)"
            elapsed=$((now - start_time))
            size="$(wc -c < "$log_file" 2>/dev/null || printf '0')"
            echo "  headless still running: ${elapsed}s elapsed, ${size} log bytes"
        fi
    done

    wait "$pid"
    local run_status=$?
    if [ "$run_status" -ne 0 ]; then
        "${DOCKER_CMD[@]}" rm -f "$container_name" >/dev/null 2>&1 || true
    fi
    rm -f "$cid_file"
    return "$run_status"
}

SUMMARY_PATHS=()
REGTEST_EXITS=()
ATTEMPT_STATUSES=()
FAILURE_REASONS=()

for i in "${!CASE_LABELS[@]}"; do
    label="${CASE_LABELS[$i]}"
    boot="$(append_perf_defaults "${CASE_BOOTS[$i]}")"
    slug="$(slugify "$label")"
    case_dir="$RUN_ROOT/$slug"
    mkdir -p "$case_dir"

    echo ""
    echo "======================================"
    echo "PS1 perf case: $label"
    echo "BOOTMODE: $boot"
    echo "======================================"

    printf '%s\n' "$boot" > "$BOOTMODE_FILE"

    if [ "$BUILD_MODE" = "clean" ]; then
        ./scripts/build-ps1.sh clean > "$case_dir/build.log" 2>&1
    else
        ./scripts/build-ps1.sh > "$case_dir/build.log" 2>&1
    fi
    ./scripts/make-cd-image.sh >> "$case_dir/build.log" 2>&1

    headless_root="$case_dir/headless"
    mkdir -p "$headless_root"
    log_file="$case_dir/headless-regtest.log"

    set +e
    run_headless_regtest "$PROJECT_ROOT/jcreborn.cue" "$headless_root" "$log_file"
    regtest_exit=$?
    set -e

    summary_file="$case_dir/perf-summary.json"
    parse_case_metrics "$label" "$boot" "$case_dir" "$log_file" "$summary_file"
    SUMMARY_PATHS+=("$summary_file")

    if [ "$regtest_exit" -ne 0 ]; then
        if case_summary_passed "$summary_file" && duckstation_exited_successfully "$log_file"; then
            REGTEST_EXITS+=("$regtest_exit")
            ATTEMPT_STATUSES+=("regtest_passed_after_wrapper_exit")
            FAILURE_REASONS+=("wrapper_exit_${regtest_exit}_after_duckstation_success")
            echo "WARN: regtest wrapper exited $regtest_exit after DuckStation success; accepting parsed JCPERF2 case metrics." >&2
            continue
        fi
        append_experiment_log "$summary_file" "$regtest_exit" "regtest_failed" "regtest_exit_$regtest_exit"
        echo "ERROR: regtest exited $regtest_exit for case $label" >&2
        echo "See: $log_file" >&2
        exit 1
    fi

    REGTEST_EXITS+=("$regtest_exit")
    ATTEMPT_STATUSES+=("regtest_passed")
    FAILURE_REASONS+=("")
done

FINAL_SUMMARY="$RUN_ROOT/summary.json"
set +e
python3 - "$FINAL_SUMMARY" "$BASELINE_FILE" "$ALLOW_REGRESSION_PERCENT" \
    "$REQUIRE_IMPROVEMENT" "${SUMMARY_PATHS[@]}" <<'PY'
import json
import shutil
import sys
from pathlib import Path

out_path = Path(sys.argv[1])
baseline_path = Path(sys.argv[2]) if sys.argv[2] else None
allow_pct = float(sys.argv[3])
require_improvement = bool(int(sys.argv[4]))
case_paths = [Path(p) for p in sys.argv[5:]]

cases = [json.loads(path.read_text(encoding="utf-8")) for path in case_paths]
baseline_cases = {}
if baseline_path and baseline_path.is_file():
    baseline = json.loads(baseline_path.read_text(encoding="utf-8"))
    for case in baseline.get("cases", []):
        baseline_cases[case.get("label")] = case

compare_fields = [
    ("timing", "loop_vb"),
    ("timing", "overrun_vb"),
    ("cd", "blocking_vb"),
    ("prefetch", "overrun_vb"),
]

def field(case, section, key):
    value = case.get("sections", {}).get(section, {}).get(key)
    return value if isinstance(value, int) else None

overall_failures = []
for case in cases:
    label = case["label"]
    gate = case.setdefault("gate", {"pass": True, "failures": [], "warnings": []})
    failures = gate.setdefault("failures", [])
    warnings = gate.setdefault("warnings", [])
    base = baseline_cases.get(label)
    improved = False
    if base:
        comparisons = []
        for section, key in compare_fields:
            current = field(case, section, key)
            previous = field(base, section, key)
            if current is None or previous is None:
                continue
            limit = previous * (1.0 + allow_pct / 100.0)
            delta = current - previous
            comparisons.append({
                "field": f"{section}.{key}",
                "baseline": previous,
                "current": current,
                "delta": delta,
                "allow_regression_percent": allow_pct,
            })
            if current < previous:
                improved = True
            if current > limit:
                failures.append(
                    f"regression {section}.{key}: baseline={previous} current={current} allowed={limit:.2f}"
                )
        case["baseline_comparison"] = comparisons
        if require_improvement and not improved:
            failures.append("no key metric improved vs baseline")
    elif baseline_path:
        warnings.append(f"no matching baseline case for {label}")
    gate["pass"] = not failures
    if failures:
        overall_failures.append({"label": label, "failures": failures})

payload = {
    "schema": "ps1-perf-iterate/v1",
    "overall_pass": not overall_failures,
    "overall_failures": overall_failures,
    "cases": cases,
}
out_path.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
for case, path in zip(cases, case_paths):
    path.write_text(json.dumps(case, indent=2) + "\n", encoding="utf-8")

print("")
print("======================================")
print("PS1 perf summary")
print("======================================")
for case in cases:
    derived = case.get("derived", {})
    sections = case.get("sections", {})
    timing = sections.get("timing", {})
    cd = sections.get("cd", {})
    prefetch = sections.get("prefetch", {})
    gate = case.get("gate", {})
    status = "PASS" if gate.get("pass") else "FAIL"
    print(
        f"{status} {case['label']}: "
        f"loop_vb={timing.get('loop_vb')} target_vb={timing.get('target_vb')} "
        f"blocking_vb={cd.get('blocking_vb')} "
        f"policy={prefetch.get('policy')} hits={prefetch.get('hits')} "
        f"due_misses={prefetch.get('due_misses')}"
    )
    for failure in gate.get("failures", []):
        print(f"  failure: {failure}")
    comparisons = case.get("baseline_comparison", [])
    if comparisons:
        parts = []
        for item in comparisons:
            field = item.get("field")
            baseline = item.get("baseline")
            current = item.get("current")
            delta = item.get("delta")
            sign = "+" if isinstance(delta, int) and delta > 0 else ""
            parts.append(f"{field} {baseline}->{current} ({sign}{delta})")
        print("  vs baseline: " + "; ".join(parts))
    for suggestion in derived.get("suggestions", [])[:4]:
        print(f"  next: {suggestion}")
print(f"Summary JSON: {out_path}")

raise SystemExit(0 if not overall_failures else 1)
PY
GATE_EXIT=$?
set -e

for i in "${!SUMMARY_PATHS[@]}"; do
    append_experiment_log "${SUMMARY_PATHS[$i]}" \
        "${REGTEST_EXITS[$i]}" \
        "${ATTEMPT_STATUSES[$i]}" \
        "${FAILURE_REASONS[$i]}"
done

restore_boot_files
trap - EXIT

if [ -n "$WRITE_BASELINE" ]; then
    mkdir -p "$(dirname "$WRITE_BASELINE")"
    cp "$FINAL_SUMMARY" "$WRITE_BASELINE"
    echo "Wrote baseline: $WRITE_BASELINE"
fi

if [ "$GATE_EXIT" -ne 0 ]; then
    echo "PS1 perf gates failed. Summary: $FINAL_SUMMARY" >&2
    if [ "$ROLLBACK_ON_FAIL" -eq 1 ]; then
        echo "Rolling back tracked worktree changes to HEAD (--rollback-on-fail)." >&2
        git restore --staged --worktree -- .
        untracked="$(git ls-files --others --exclude-standard)"
        if [ -n "$untracked" ]; then
            echo "Untracked files were not removed:" >&2
            printf '%s\n' "$untracked" >&2
        fi
    fi
    exit "$GATE_EXIT"
fi

if [ -n "$COMMIT_MESSAGE" ]; then
    git add -A
    if git diff --cached --quiet; then
        echo "No staged changes to commit."
    else
        git commit -m "$COMMIT_MESSAGE"
    fi
fi

echo "PS1 perf gates passed. Summary: $FINAL_SUMMARY"
