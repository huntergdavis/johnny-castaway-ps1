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
FRAMES="${PS1_PERF_FRAMES:-12000}"
INTERVAL="${PS1_PERF_INTERVAL:-999999}"
TIMEOUT="${PS1_PERF_TIMEOUT:-${REGTEST_TIMEOUT:-180}}"
LOG_LEVEL="${PS1_PERF_LOG_LEVEL:-Warning}"
MAX_LOG_BYTES="${PS1_PERF_MAX_LOG_BYTES:-536870912}"
EARLY_STOP_ON_JCPERF2="${PS1_PERF_EARLY_STOP:-1}"
EARLY_STOP_SETTLE_SECONDS="${PS1_PERF_EARLY_STOP_SETTLE_SECONDS:-2}"
HEADLESS_POLL_SECONDS="${PS1_PERF_POLL_SECONDS:-5}"
PERF_TOKEN="perf-log"
BUILD_MODE="incremental"
BASELINE_FILE=""
WRITE_BASELINE=""
COMMIT_MESSAGE=""
ROLLBACK_ON_FAIL=0
ALLOW_REGRESSION_PERCENT="${PS1_PERF_ALLOW_REGRESSION_PERCENT:-2}"
WORK_IDENTITY_MIN_PERCENT="${PS1_PERF_WORK_IDENTITY_MIN_PERCENT:-75}"
ALLOW_LAYOUT_CHANGE="${PS1_PERF_ALLOW_LAYOUT_CHANGE:-0}"
MAX_SYMBOL_ADDRESS_DELTA="${PS1_PERF_MAX_SYMBOL_ADDRESS_DELTA:-}"
REQUIRE_IMPROVEMENT=0
CHECK_ENV_ONLY=0
NO_SEED=0
SEED="${REGTEST_SEED:-1}"
CASE_LOCAL_CD=0
TRANSITIONS_SCENES="${PS1_PERF_TRANSITIONS:-0}"
GATE_SETUP_HIT="${PS1_PERF_GATE_SETUP_HIT:-0}"
GATE_SETUP_COLD="${PS1_PERF_GATE_SETUP_COLD:-0}"
FRAMES_EXPLICIT=0
TIMEOUT_EXPLICIT=0

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
  --frames N               Emulated frames per case (default: 12000).
                           Early-stop on JCPERF2 means short scenes exit as
                           soon as they emit perf data; this budget only
                           affects the longest scenes (suzy1/mary1/building3).
  --interval N             Screenshot dump interval (default: 999999).
  --timeout N              Wall-clock timeout per case (default: REGTEST_TIMEOUT or 180).
  --log LEVEL              DuckStation log level (default: Warning).
  --max-log-bytes N        Fail early if headless log exceeds N bytes
                           (default: PS1_PERF_MAX_LOG_BYTES or 536870912;
                           set 0 to disable).
  --no-early-stop          Do not stop headless DuckStation when JCPERF2
                           correctness has been emitted. By default perf runs
                           stop at that point to avoid burning the remaining
                           artificial frame budget.
  --transitions N          Transition-measurement mode: expect a loop-mode
                           boot (no explicit scene; the picker drives scene
                           rotation so next-scene stage lookahead can run).
                           Plays N scenes, parses every per-scene JCPERF2
                           block, classifies each post-boot transition as
                           staged-hit (stage_adopt metadata bit) or cold, and
                           reports per-transition setup_vb. Scales --frames
                           and --timeout with N unless given explicitly.
                           Example boot: "fgpilot perf-log loading-waves seed 1"
  --gate-setup-hit VB      With --transitions: fail if any staged-hit
                           transition has setup_vb > VB (0 = report only).
  --gate-setup-cold VB     With --transitions: fail if any cold transition
                           (including boot scene) has setup_vb > VB
                           (0 = report only).
  --output DIR             Output root (default: scratch/ps1-perf-iterate).
  --experiment-log FILE    Append one JSONL record per attempted case
                           (default: <output>/experiments.jsonl).
  --clean                  Clean PS1 build before each case.
  --skip-build             Reuse the existing PS1 executable and only remake
                           the CD image after staging BOOTMODE.TXT. This is
                           valid for perf matrix passes because the runtime
                           reads BOOTMODE.TXT from the disc before the
                           embedded fallback.
  --case-local-cd          Build the case CD image under the case output
                           directory instead of mutating root jcreborn.bin/cue
                           and config/ps1/BOOTMODE.TXT. Implies --skip-build
                           and skips the global perf lock, so parallel matrix
                           jobs can run safely.
  --baseline FILE          Compare against a prior summary JSON.
  --write-baseline FILE    Also copy this run summary to FILE.
  --allow-regression PCT   Fail if key metrics regress by more than PCT (default: 2).
  --work-identity-min PCT  Fail if baseline-sensitive render work counters fall below
                           this percent of baseline (default: 75; set 0 to disable).
  --allow-layout-change    With --baseline, allow PS-EXE sector bucket and foreground
                           pack LBA changes. Comparisons are still recorded.
  --max-symbol-address-delta N
                           With --baseline, fail if any tracked hot symbol moves by
                           more than N bytes. Use 0 for exact-address phase gates.
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
  - routed active scenes must report nonzero loop_start, loop_vb, advances,
    complete entry coverage, and final-frame coverage.
  - correctness trip/fallback/stale/frame/sound/CD counters must be zero.
  - gfx full_fallbacks must be zero.
  - with --baseline, scene_vb, loop_vb, timing overrun_vb, blocking_vb,
    and prefetch overrun_vb must not regress beyond --allow-regression.
  - with --baseline, render/restore/compose/upload call counts must not fall
    below --work-identity-min unless explicitly disabled.
  - with --baseline, PS-EXE sector bucket and foreground pack LBA must stay
    fixed unless --allow-layout-change is used.

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
    # Transition runs measure the screensaver loop itself: never inject
    # noloop, and refuse boots that carry it (one-shot runs can't produce
    # scene-to-scene transitions).
    if [ "$TRANSITIONS_SCENES" -gt 0 ]; then
        if [[ "$boot" == *"noloop"* ]]; then
            echo "ERROR: --transitions requires a looping boot; remove 'noloop' from: $boot" >&2
            exit 1
        fi
    elif [[ "$boot" != *" noloop"* && "$boot" != *" loop"* ]]; then
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
            FRAMES="$2"; FRAMES_EXPLICIT=1; shift 2 ;;
        --interval)
            INTERVAL="$2"; shift 2 ;;
        --timeout)
            TIMEOUT="$2"; TIMEOUT_EXPLICIT=1; shift 2 ;;
        --transitions)
            TRANSITIONS_SCENES="$2"; shift 2 ;;
        --gate-setup-hit)
            GATE_SETUP_HIT="$2"; shift 2 ;;
        --gate-setup-cold)
            GATE_SETUP_COLD="$2"; shift 2 ;;
        --log)
            LOG_LEVEL="$2"; shift 2 ;;
        --max-log-bytes)
            MAX_LOG_BYTES="$2"; shift 2 ;;
        --no-early-stop)
            EARLY_STOP_ON_JCPERF2=0; shift ;;
        --output)
            OUTPUT_ROOT="$2"; shift 2 ;;
        --experiment-log)
            EXPERIMENT_LOG="$2"; shift 2 ;;
        --clean)
            BUILD_MODE="clean"; shift ;;
        --skip-build)
            BUILD_MODE="skip"; shift ;;
        --case-local-cd)
            CASE_LOCAL_CD=1; BUILD_MODE="skip"; shift ;;
        --baseline)
            BASELINE_FILE="$2"; shift 2 ;;
        --write-baseline)
            WRITE_BASELINE="$2"; shift 2 ;;
        --allow-regression)
            ALLOW_REGRESSION_PERCENT="$2"; shift 2 ;;
        --work-identity-min)
            WORK_IDENTITY_MIN_PERCENT="$2"; shift 2 ;;
        --allow-layout-change)
            ALLOW_LAYOUT_CHANGE=1; shift ;;
        --max-symbol-address-delta)
            MAX_SYMBOL_ADDRESS_DELTA="$2"; shift 2 ;;
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

if ! [[ "$TRANSITIONS_SCENES" =~ ^[0-9]+$ ]]; then
    echo "ERROR: --transitions must be a non-negative integer." >&2
    exit 1
fi
if ! [[ "$GATE_SETUP_HIT" =~ ^[0-9]+$ ]] || ! [[ "$GATE_SETUP_COLD" =~ ^[0-9]+$ ]]; then
    echo "ERROR: --gate-setup-hit/--gate-setup-cold must be non-negative integers (0 = report only)." >&2
    exit 1
fi
if [ "$TRANSITIONS_SCENES" -gt 0 ]; then
    # Transition runs play (N) scenes through the loop picker. Scale the
    # frame budget and wall timeout with scene count unless given explicitly.
    if [ "$FRAMES_EXPLICIT" -eq 0 ]; then
        FRAMES=$(( TRANSITIONS_SCENES * 2200 + 4000 ))
    fi
    if [ "$TIMEOUT_EXPLICIT" -eq 0 ]; then
        # Headless software rendering paces near realtime (~1.8s wall per
        # emulated second on the dev box): ~110s wall boot + up to ~60s wall
        # per scene. Early-stop reclaims the cushion when blocks land sooner.
        TIMEOUT=$(( 240 + TRANSITIONS_SCENES * 60 ))
    fi
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
if ! [[ "$MAX_LOG_BYTES" =~ ^[0-9]+$ ]]; then
    echo "ERROR: --max-log-bytes must be a non-negative integer." >&2
    exit 1
fi
if ! [[ "$EARLY_STOP_ON_JCPERF2" =~ ^[01]$ ]]; then
    echo "ERROR: PS1_PERF_EARLY_STOP must be 0 or 1." >&2
    exit 1
fi
if ! [[ "$EARLY_STOP_SETTLE_SECONDS" =~ ^[0-9]+$ ]]; then
    echo "ERROR: PS1_PERF_EARLY_STOP_SETTLE_SECONDS must be a non-negative integer." >&2
    exit 1
fi
if ! [[ "$HEADLESS_POLL_SECONDS" =~ ^[0-9]+$ ]] || [ "$HEADLESS_POLL_SECONDS" -lt 1 ]; then
    echo "ERROR: PS1_PERF_POLL_SECONDS must be a positive integer." >&2
    exit 1
fi
if [ -n "$BASELINE_FILE" ] && [ ! -f "$BASELINE_FILE" ]; then
    echo "ERROR: baseline file not found: $BASELINE_FILE" >&2
    exit 1
fi
if ! [[ "$WORK_IDENTITY_MIN_PERCENT" =~ ^[0-9]+([.][0-9]+)?$ ]]; then
    echo "ERROR: --work-identity-min must be a non-negative number." >&2
    exit 1
fi
if [ -n "$MAX_SYMBOL_ADDRESS_DELTA" ] &&
   ! [[ "$MAX_SYMBOL_ADDRESS_DELTA" =~ ^[0-9]+$ ]]; then
    echo "ERROR: --max-symbol-address-delta must be a non-negative integer." >&2
    exit 1
fi

check_env

RUN_ID="$(date +%Y%m%d-%H%M%S)-$$"
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

BOOTMODE_BACKUP=""
BOOTMODE_WAS_PRESENT=0
HEADER_BACKUP=""
HEADER_WAS_PRESENT=0
RESTORED_BOOT_FILES=1

if [ "$CASE_LOCAL_CD" -eq 0 ]; then
    BOOTMODE_BACKUP="$(mktemp /tmp/ps1-perf-bootmode-XXXXXX.txt)"
    if [ -f "$BOOTMODE_FILE" ]; then
        cp "$BOOTMODE_FILE" "$BOOTMODE_BACKUP"
        BOOTMODE_WAS_PRESENT=1
    fi

    HEADER_BACKUP="$(mktemp /tmp/ps1-perf-bootheader-XXXXXX.h)"
    if [ -f "$EMBEDDED_BOOTMODE_HEADER" ]; then
        cp "$EMBEDDED_BOOTMODE_HEADER" "$HEADER_BACKUP"
        HEADER_WAS_PRESENT=1
    fi
    RESTORED_BOOT_FILES=0
fi

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

if [ "$CASE_LOCAL_CD" -eq 0 ]; then
    exec 9>"$LOCK_FILE"
    if command -v flock >/dev/null 2>&1; then
        echo "Waiting for PS1 perf lock: $LOCK_FILE"
        flock 9
    else
        echo "ERROR: flock is required." >&2
        exit 1
    fi
fi

make_case_cd_image() {
    local cd_dir="$1"
    local boot="$2"
    local build_log="$3"

    mkdir -p "$cd_dir"
    printf '%s\n' "$boot" > "$cd_dir/BOOTMODE.TXT"
    python3 - "$PROJECT_ROOT" "$cd_dir" <<'PY'
import re
import sys
from pathlib import Path

root = Path(sys.argv[1])
cd_dir = Path(sys.argv[2])
layout = (root / "config/ps1/cd_layout.xml").read_text(encoding="utf-8")
layout = layout.replace(
    'source="../../config/ps1/BOOTMODE.TXT"',
    'source="/work/BOOTMODE.TXT"',
)
layout = re.sub(
    r'source="../../([^"]+)"',
    lambda match: f'source="/project/{match.group(1)}"',
    layout,
)
(cd_dir / "cd_layout.xml").write_text(layout, encoding="utf-8")
PY

    {
        echo "=== Creating case-local PS1 CD image ==="
        echo "BOOTMODE.TXT => $boot"
        "${DOCKER_CMD[@]}" run --rm --platform linux/amd64 \
            -v "$PROJECT_ROOT:/project:ro" \
            -v "$(realpath "$cd_dir"):/work" \
            -w /work \
            jc-reborn-ps1-dev:amd64 \
            mkpsxiso -y /work/cd_layout.xml
    } >> "$build_log" 2>&1
}

parse_case_metrics() {
    local label="$1"
    local boot="$2"
    local case_dir="$3"
    local log_file="$4"
    local ps_exe_bytes="$5"
    local ps_exe_bucket_bytes="$6"
    local ps_exe_sectors="$7"
    local elf_bytes="$8"
    local map_bytes="$9"
    local git_commit="${10}"
    local git_branch="${11}"
    local git_dirty="${12}"
    local run_id="${13}"
    local out_file="${14}"

    PS1_PERF_TRANS_EVAL="$TRANSITIONS_SCENES" \
    PS1_PERF_GATE_SETUP_HIT="$GATE_SETUP_HIT" \
    PS1_PERF_GATE_SETUP_COLD="$GATE_SETUP_COLD" \
    python3 - "$label" "$boot" "$case_dir" "$log_file" \
        "$ps_exe_bytes" "$ps_exe_bucket_bytes" "$ps_exe_sectors" "$elf_bytes" "$map_bytes" \
        "$git_commit" "$git_branch" "$git_dirty" "$run_id" > "$out_file" <<'PY'
import json
import re
import sys
from pathlib import Path

label, boot, case_dir, log_file = sys.argv[1:5]
ps_exe_bytes, ps_exe_bucket_bytes, ps_exe_sectors, elf_bytes, map_bytes = (int(value) for value in sys.argv[5:10])
git_commit, git_branch, git_dirty, run_id = sys.argv[10:14]

import os
transitions_expected = int(os.environ.get("PS1_PERF_TRANS_EVAL", "0") or 0)
gate_setup_hit = int(os.environ.get("PS1_PERF_GATE_SETUP_HIT", "0") or 0)
gate_setup_cold = int(os.environ.get("PS1_PERF_GATE_SETUP_COLD", "0") or 0)
log_path = Path(log_file)
ansi_re = re.compile(r"\x1b\[[0-9;?]*[A-Za-z]")

hot_symbol_names = {
    "fgRuntimeFillWindowForEntry",
    "fgRuntimeTryExtendWindow",
    "fgRuntimeTryPrefetchWindow",
    "fgRuntimeTryStageNextFrame",
    "fgRuntimeWindowPrefetchWouldRead",
    "fgRuntimeLoadSceneFrame",
    "fgRuntimeCanPrepareStagedFrame",
    "fgRuntimePrepareStagedFrameForPresent",
    "fgRuntimeCanPresentPreparedOnNextVBlank",
    "fgRuntimePresentPreparedFrame",
    "foregroundPilotRuntimeAdvance.part.0",
    "foregroundPilotRuntimeCompose",
    "foregroundPilotPlay",
    "grRestoreBgFromRects",
    "grCompositePacked4SpansToBackground",
    "grDrawBackground",
    "grUpdateDisplay",
    "ps1_streamReadFromCdFile",
    "ps1_streamReadFromCdFileIntoBuffered",
    "ps1_streamReadAlignedIntoFile",
    "ps1PerfMarkCdReadDetailed",
    "ps1PerfEndScene",
}

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
# Per-scene JCPERF2 blocks: each scene emits scene ... correctness in order.
# `sections` stays last-wins for the legacy single-scene gates; scene_blocks
# collects every COMPLETE block for --transitions evaluation.
scene_blocks = []
current_block = None
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
        if section == "scene":
            current_block = {}
        if current_block is not None:
            current_block[section] = data
            if section == "correctness":
                scene_blocks.append(current_block)
                current_block = None
        sections[section] = data

if transitions_expected > 0 and scene_blocks:
    # Early-stop can truncate a trailing block mid-emission; pin the legacy
    # single-scene view to the last COMPLETE scene so its gates stay coherent.
    sections = dict(scene_blocks[-1])

symbols = {}
map_path = Path("build-ps1/jcreborn.map")
if map_path.is_file():
    for raw in map_path.read_text(encoding="utf-8", errors="ignore").splitlines():
        parts = raw.split(None, 4)
        if len(parts) < 4:
            continue
        name, sym_type, address_text, size_text = parts[:4]
        if name not in hot_symbol_names:
            continue
        try:
            address = int(address_text, 16) & 0xFFFFFFFF
            size = int(size_text, 16)
        except ValueError:
            continue
        item = {
            "type": sym_type,
            "address": address,
            "size": size,
        }
        if len(parts) >= 5:
            item["source"] = parts[4]
        symbols[name] = item

def get(section, key, default=0):
    value = sections.get(section, {}).get(key, default)
    return value if isinstance(value, int) else default

# Mirror of ps1IsFgPilotOptionToken (src/jc_reborn.c) for the tokens a
# loop-mode boot is likely to carry; "fgpilot <option>" means no explicit
# scene (picker-driven loop), not a scene named like the option.
FGPILOT_OPTION_TOKENS = {
    "fgoverlay", "island-pos", "lowtide", "raft-stage", "night", "holiday",
    "noloop", "loading-waves", "load-waves", "async-load-waves",
    "loading-waves-off", "no-loading-waves", "prefetch-off", "no-prefetch",
    "prefetch-stage1", "stage1", "prefetch-stage1-off", "no-stage1",
    "prefetch-window32", "window32", "prefetch-window48", "window48",
    "prefetch-window64", "window64", "prefetch-window", "spu-cache-test",
    "spu-cache-proof", "spu-stage", "no-spu-stage", "picker-random",
    "picker-sequential", "picker-original", "perf-log", "perf",
    "perf-detail", "perf-debug", "freeplay-log", "freeplay-detail",
    "freeplay-debug", "pad-script", "pad-script-log", "seed",
}

def expected_config_from_boot(boot_text):
    parts = boot_text.split()
    expected = {}
    for idx, token in enumerate(parts):
        if (token == "fgpilot" and idx + 1 < len(parts)
                and parts[idx + 1].lower() not in FGPILOT_OPTION_TOKENS):
            expected["scene"] = parts[idx + 1].lower()
        elif token in ("lowtide", "night", "holiday", "raft-stage") and idx + 1 < len(parts):
            try:
                key = "raft" if token == "raft-stage" else token
                expected[key] = int(parts[idx + 1])
            except ValueError:
                pass
        elif token == "island-pos" and idx + 2 < len(parts):
            try:
                expected["pos"] = f"{int(parts[idx + 1])},{int(parts[idx + 2])}"
            except ValueError:
                pass
    return expected

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

expected_config = expected_config_from_boot(boot)
expected_scene_name = expected_config.get("scene")
scene_name = str(sections.get("scene", {}).get("scene", "")).lower()
if expected_scene_name and sections:
    if scene_name != expected_scene_name:
        failures.append(f"scene_mismatch expected={expected_scene_name} actual={scene_name or '?'}")
    if any(line.startswith("JCPICK ") for line in tty_lines):
        failures.append("explicit_scene_fell_through_to_picker")
    scene_section = sections.get("scene", {})
    for key in ("lowtide", "night", "holiday", "raft", "pos"):
        if key not in expected_config:
            continue
        actual = scene_section.get(key)
        expected = expected_config[key]
        if str(actual) != str(expected):
            failures.append(f"scene_option_mismatch {key}: expected={expected} actual={actual}")
scene_entries = get("scene", "entries", 0)
loop_start = get("timing", "loop_start", 0)
advances = get("timing", "advances", 0)
timing_entries = get("timing", "entries", 0)
if sections and scene_entries > 0:
    active_loop_failures = []
    if loop_start <= 0:
        active_loop_failures.append(f"timing.loop_start={loop_start}")
    if loop_vb <= 0:
        active_loop_failures.append(f"timing.loop_vb={loop_vb}")
    if advances <= 0:
        active_loop_failures.append(f"timing.advances={advances}")
    if timing_entries < scene_entries:
        active_loop_failures.append(
            f"timing.entries={timing_entries} scene.entries={scene_entries}"
        )
    if expected_frames > 0 and last_frame < expected_frames - 1:
        active_loop_failures.append(
            f"last_frame={last_frame} expected_final={expected_frames - 1}"
        )
    if active_loop_failures:
        message = "active-loop incomplete: " + ", ".join(active_loop_failures)
        failures.append(message)

transitions = []
transitions_summary = None
if transitions_expected > 0:
    if len(scene_blocks) < transitions_expected:
        failures.append(
            f"transitions_incomplete expected={transitions_expected} complete={len(scene_blocks)}"
        )
    for idx, blk in enumerate(scene_blocks):
        def bval(sec, key, default=0):
            value = blk.get(sec, {}).get(key, default)
            return value if isinstance(value, int) else default
        bscene = str(blk.get("scene", {}).get("scene", "?"))
        adopt = bval("setup", "stage_adopt", 0)
        # stage_adopt bit1 = metadata prefix served from the SPU stage; that
        # is the marker that the lookahead actually removed the cold CD read.
        kind = "boot" if idx == 0 else ("hit" if (adopt & 2) else "cold")
        rec = {
            "index": idx,
            "scene": bscene,
            "kind": kind,
            "stage_adopt": adopt,
            "setup_vb": bval("setup", "setup_vb", 0),
            "screen_vb": bval("setup", "screen_vb", 0),
            "backdrop_vb": bval("setup", "backdrop_vb", 0),
            "pack_start_vb": bval("setup", "pack_start_vb", 0),
            "clean_rect_vb": bval("setup", "clean_rect_vb", 0),
            "first_frame_vb": bval("setup", "first_frame_vb", 0),
            "setup_reads": bval("setup", "setup_reads", 0),
            "setup_bytes": bval("setup", "setup_bytes", 0),
            "gap_vb": bval("setup", "gap_vb", 0),
            "loop_vb": bval("timing", "loop_vb", 0),
            "target_vb": bval("timing", "target_vb", 0),
            "blocking_vb": bval("cd", "blocking_vb", 0),
            "lowtide": blk.get("scene", {}).get("lowtide"),
        }
        transitions.append(rec)
        for sec, key in zero_required:
            value = bval(sec, key, 0)
            if value != 0:
                failures.append(f"scene[{idx}]:{bscene} {sec}.{key}={value}")
        if idx > 0:
            if kind == "hit" and gate_setup_hit > 0 and rec["setup_vb"] > gate_setup_hit:
                failures.append(
                    f"scene[{idx}]:{bscene} staged-hit setup_vb={rec['setup_vb']} > gate {gate_setup_hit}"
                )
            if kind == "cold" and gate_setup_cold > 0 and rec["setup_vb"] > gate_setup_cold:
                failures.append(
                    f"scene[{idx}]:{bscene} cold setup_vb={rec['setup_vb']} > gate {gate_setup_cold}"
                )
    post_boot = [t for t in transitions if t["index"] > 0]
    staged_hits = [t for t in post_boot if t["kind"] == "hit"]
    cold = [t for t in post_boot if t["kind"] == "cold"]
    transitions_summary = {
        "expected_scenes": transitions_expected,
        "complete_scenes": len(scene_blocks),
        "post_boot_transitions": len(post_boot),
        "staged_hits": len(staged_hits),
        "hit_rate": (len(staged_hits) / len(post_boot)) if post_boot else None,
        "max_setup_vb_hit": max((t["setup_vb"] for t in staged_hits), default=None),
        "max_setup_vb_cold": max((t["setup_vb"] for t in cold), default=None),
        "boot_setup_vb": transitions[0]["setup_vb"] if transitions else None,
        "gates": {"setup_hit": gate_setup_hit, "setup_cold": gate_setup_cold},
        "records": transitions,
    }

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

fingerprint = {
    "case": {
        "scene": scene_name or expected_scene_name,
        "lowtide": sections.get("scene", {}).get("lowtide", expected_config.get("lowtide")),
        "night": sections.get("scene", {}).get("night", expected_config.get("night")),
        "holiday": sections.get("scene", {}).get("holiday", expected_config.get("holiday")),
        "raft": sections.get("scene", {}).get("raft", expected_config.get("raft")),
        "pos": sections.get("scene", {}).get("pos", expected_config.get("pos")),
    },
    "metrics": {
        "scene_vb": get("timing", "scene_vb", 0),
        "loop_vb": loop_vb,
        "target_vb": target_vb,
        "setup_vb": get("setup", "setup_vb", 0),
        "blocking_vb": blocking_vb,
        "prefetch_overrun_vb": get("prefetch", "overrun_vb", 0),
        "loop_reads": get("cd", "loop_reads", 0),
    },
    "layout": {
        "pack_lba": get("scene", "pack_lba", 0),
        "pack_sectors": get("scene", "pack_sectors", 0),
        "ps_exe_bytes": ps_exe_bytes,
        "ps_exe_sector_bucket_bytes": ps_exe_bucket_bytes,
        "elf_bytes": elf_bytes,
        "map_bytes": map_bytes,
    },
    "hot_symbol_sizes": {
        name: item["size"]
        for name, item in sorted(symbols.items())
        if isinstance(item.get("size"), int)
    },
}

summary = {
    "label": label,
    "boot": boot,
    "run": {
        "id": run_id,
        "git": {
            "commit": git_commit,
            "branch": git_branch,
            "dirty": bool(int(git_dirty)),
        },
    },
    "case_dir": str(Path(case_dir).resolve()),
    "log_file": str(log_path.resolve()),
    "sections": sections,
    "expected": {
        **expected_config,
    },
    "build": {
        "ps_exe": {
            "path": "build-ps1/jcreborn.exe",
            "bytes": ps_exe_bytes,
            "sector_bucket_bytes": ps_exe_bucket_bytes,
            "sectors": ps_exe_sectors,
        },
        "elf": {
            "path": "build-ps1/jcreborn.elf",
            "bytes": elf_bytes,
        },
        "map": {
            "path": "build-ps1/jcreborn.map",
            "bytes": map_bytes,
            "symbols": symbols,
        },
    },
    "fingerprint": fingerprint,
    "transitions": transitions_summary,
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

emit_foreground_read_plan() {
    local summary_file="$1"
    local case_dir="$2"
    local pack_file
    local plan_json="$case_dir/foreground-read-plan.json"
    local plan_text="$case_dir/foreground-read-plan.txt"
    local plan_err="$case_dir/foreground-read-plan.err"

    if ! pack_file="$(python3 - "$summary_file" "$PROJECT_ROOT" <<'PY'
import json
import subprocess
import sys
from pathlib import Path

summary_path = Path(sys.argv[1])
project_root = Path(sys.argv[2])
summary = json.loads(summary_path.read_text(encoding="utf-8"))
scene = summary.get("sections", {}).get("scene", {})
pack = str(scene.get("pack", "")).replace("\\", "/")
pack_name = Path(pack).name
if not pack_name:
    raise SystemExit(1)
pack_path = project_root / "generated" / "ps1" / "foreground" / pack_name
if not pack_path.is_file():
    scene_name = scene.get("scene")
    lowtide = scene.get("lowtide") == 1
    manifest_path = project_root / "scripts" / "ps1-foreground-scene-manifest.py"
    manifest = json.loads(subprocess.check_output([str(manifest_path)], text=True))
    for record in manifest:
        if record.get("slug") != scene_name:
            continue
        source_name = record.get("low_source" if lowtide else "high_source", "")
        source_path = project_root / "generated" / "ps1" / "foreground" / source_name
        if source_path.is_file():
            pack_path = source_path
            break
if not pack_path.is_file():
    raise SystemExit(1)
print(pack_path)
PY
    )"; then
        return 0
    fi

    if python3 "$PROJECT_ROOT/scripts/ps1-foreground-read-plan.py" \
        --summary "$summary_file" \
        --pack "$pack_file" \
        --top 4 \
        --output "$plan_json" > "$plan_text" 2> "$plan_err"; then
        python3 - "$summary_file" "$plan_json" "$plan_text" <<'PY'
import json
import sys
from pathlib import Path

summary_path = Path(sys.argv[1])
summary = json.loads(summary_path.read_text(encoding="utf-8"))
artifacts = summary.setdefault("artifacts", {})
artifacts["foreground_read_plan_json"] = sys.argv[2]
artifacts["foreground_read_plan_text"] = sys.argv[3]
summary_path.write_text(json.dumps(summary, indent=2) + "\n", encoding="utf-8")
PY
        echo ""
        echo "Foreground read plan:"
        sed -n '1,44p' "$plan_text"
    else
        echo "WARN: foreground read-plan generation failed for $summary_file" >&2
        if [ -s "$plan_err" ]; then
            sed -n '1,8p' "$plan_err" >&2
        fi
    fi
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
scene = sections.get("scene", {})
timing = sections.get("timing", {})
cd = sections.get("cd", {})
prefetch = sections.get("prefetch", {})
gfx = sections.get("gfx", {})
correctness = sections.get("correctness", {})
gate = summary.get("gate", {})
build = summary.get("build", {})
ps_exe = build.get("ps_exe", {})
elf = build.get("elf", {})
build_map = build.get("map", {})

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
    "gate_warnings": gate.get("warnings", []),
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
        "pack_lba": scene.get("pack_lba"),
        "pack_sectors": scene.get("pack_sectors"),
        "ps_exe_bytes": ps_exe.get("bytes"),
        "ps_exe_sector_bucket_bytes": ps_exe.get("sector_bucket_bytes"),
        "elf_bytes": elf.get("bytes"),
        "map_bytes": build_map.get("bytes"),
    },
    "fingerprint": summary.get("fingerprint"),
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

jcperf2_correctness_emitted() {
    local log_file="$1"
    [ -f "$log_file" ] && grep -q "JCPERF2 correctness" "$log_file"
}

headless_early_stopped() {
    local out_dir="$1"
    [ -f "$out_dir/early-stop.txt" ]
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
    rm -f "$cid_file" "$out_dir/early-stop.txt"
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
        echo "max_log_bytes=$MAX_LOG_BYTES"
        echo "early_stop_on_jcperf2=$EARLY_STOP_ON_JCPERF2"
        echo "early_stop_settle_seconds=$EARLY_STOP_SETTLE_SECONDS"
        echo "poll_seconds=$HEADLESS_POLL_SECONDS"
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
        sleep "$HEADLESS_POLL_SECONDS"
        if kill -0 "$pid" >/dev/null 2>&1; then
            local now elapsed size
            now="$(date +%s)"
            elapsed=$((now - start_time))
            size="$(wc -c < "$log_file" 2>/dev/null || printf '0')"
            echo "  headless still running: ${elapsed}s elapsed, ${size} log bytes"
            local needed_correctness=1
            if [ "$TRANSITIONS_SCENES" -gt 0 ]; then
                needed_correctness="$TRANSITIONS_SCENES"
            fi
            if [ "$EARLY_STOP_ON_JCPERF2" -eq 1 ] && \
               [ "$(grep -c "JCPERF2 correctness" "$log_file" 2>/dev/null; true)" -ge "$needed_correctness" ] 2>/dev/null; then
                {
                    echo "reason=jcperf2_correctness"
                    echo "needed_correctness=$needed_correctness"
                    echo "elapsed_seconds=$elapsed"
                    echo "log_bytes=$size"
                } > "$out_dir/early-stop.txt"
                echo "  headless early-stop: JCPERF2 correctness x$needed_correctness emitted; stopping $container_name"
                sleep "$EARLY_STOP_SETTLE_SECONDS"
                "${DOCKER_CMD[@]}" rm -f "$container_name" >/dev/null 2>&1 || true
                kill "$pid" >/dev/null 2>&1 || true
                break
            fi
            if [ "$MAX_LOG_BYTES" -gt 0 ] && [ "$size" -gt "$MAX_LOG_BYTES" ]; then
                echo "ERROR: headless log exceeded ${MAX_LOG_BYTES} bytes; stopping $container_name" >&2
                "${DOCKER_CMD[@]}" rm -f "$container_name" >/dev/null 2>&1 || true
                kill "$pid" >/dev/null 2>&1 || true
                break
            fi
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

    cue_for_case="$PROJECT_ROOT/jcreborn.cue"
    build_log="$case_dir/build.log"

    if [ "$CASE_LOCAL_CD" -eq 1 ]; then
        if [ ! -f "$PROJECT_ROOT/build-ps1/jcreborn.exe" ]; then
            echo "ERROR: --case-local-cd requires build-ps1/jcreborn.exe. Run ./scripts/build-ps1.sh once first." >&2
            exit 1
        fi
        : > "$build_log"
        make_case_cd_image "$case_dir/cd" "$boot" "$build_log"
        cue_for_case="$case_dir/cd/jcreborn.cue"
    else
        printf '%s\n' "$boot" > "$BOOTMODE_FILE"
    fi

    if [ "$CASE_LOCAL_CD" -eq 1 ]; then
        :
    elif [ "$BUILD_MODE" = "skip" ]; then
        if [ ! -f "$PROJECT_ROOT/build-ps1/jcreborn.exe" ]; then
            echo "ERROR: --skip-build requires build-ps1/jcreborn.exe. Run ./scripts/build-ps1.sh once first." >&2
            exit 1
        fi
        {
            echo "=== Skipping PS1 executable build ==="
            echo "BOOTMODE.TXT is refreshed on the CD image for this case."
        } > "$build_log"
    elif [ "$BUILD_MODE" = "clean" ]; then
        ./scripts/build-ps1.sh clean > "$build_log" 2>&1
    else
        ./scripts/build-ps1.sh > "$build_log" 2>&1
    fi

    if [ "$CASE_LOCAL_CD" -eq 0 ]; then
        ./scripts/make-cd-image.sh >> "$build_log" 2>&1
    fi

    ps_exe_bytes=0
    ps_exe_bucket_bytes=0
    ps_exe_sectors=0
    elf_bytes=0
    map_bytes=0
    if [ -f "$PROJECT_ROOT/build-ps1/jcreborn.exe" ]; then
        ps_exe_bytes="$(wc -c < "$PROJECT_ROOT/build-ps1/jcreborn.exe")"
        ps_exe_sectors=$(( (ps_exe_bytes + 2047) / 2048 ))
        ps_exe_bucket_bytes=$(( ps_exe_sectors * 2048 ))
    fi
    if [ -f "$PROJECT_ROOT/build-ps1/jcreborn.elf" ]; then
        elf_bytes="$(wc -c < "$PROJECT_ROOT/build-ps1/jcreborn.elf")"
    fi
    if [ -f "$PROJECT_ROOT/build-ps1/jcreborn.map" ]; then
        map_bytes="$(wc -c < "$PROJECT_ROOT/build-ps1/jcreborn.map")"
    fi

    headless_root="$case_dir/headless"
    mkdir -p "$headless_root"
    log_file="$case_dir/headless-regtest.log"

    set +e
    run_headless_regtest "$cue_for_case" "$headless_root" "$log_file"
    regtest_exit=$?
    set -e
    if [ "$CASE_LOCAL_CD" -eq 1 ]; then
        rm -rf "$case_dir/cd"
    fi

    summary_file="$case_dir/perf-summary.json"
    parse_case_metrics "$label" "$boot" "$case_dir" "$log_file" \
        "$ps_exe_bytes" "$ps_exe_bucket_bytes" "$ps_exe_sectors" "$elf_bytes" "$map_bytes" \
        "$GIT_COMMIT" "$GIT_BRANCH" "$GIT_DIRTY" "$RUN_ID" \
        "$summary_file"
    emit_foreground_read_plan "$summary_file" "$case_dir"
    SUMMARY_PATHS+=("$summary_file")

    if [ "$regtest_exit" -ne 0 ]; then
        # A nonzero wrapper exit is expected when the early-stop kills the
        # container (docker reports 137). If the JCPERF2 metrics are
        # complete, keep going: the case's own gate verdict (including
        # intentional --gate-setup-* failures) flows through the final
        # summary instead of being masked as a wrapper error.
        if headless_early_stopped "$headless_root" ||
           jcperf2_correctness_emitted "$log_file" ||
           duckstation_exited_successfully "$log_file"; then
            REGTEST_EXITS+=("$regtest_exit")
            if case_summary_passed "$summary_file"; then
                ATTEMPT_STATUSES+=("regtest_passed_after_early_stop")
            else
                ATTEMPT_STATUSES+=("regtest_gates_failed_after_early_stop")
            fi
            FAILURE_REASONS+=("wrapper_exit_${regtest_exit}_after_jcperf2_complete")
            echo "WARN: regtest wrapper exited $regtest_exit after complete JCPERF2 metrics; accepting parsed metrics." >&2
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
    "$REQUIRE_IMPROVEMENT" "$WORK_IDENTITY_MIN_PERCENT" "$ALLOW_LAYOUT_CHANGE" \
    "$MAX_SYMBOL_ADDRESS_DELTA" "${SUMMARY_PATHS[@]}" <<'PY'
import json
import shutil
import sys
from pathlib import Path

out_path = Path(sys.argv[1])
baseline_path = Path(sys.argv[2]) if sys.argv[2] else None
allow_pct = float(sys.argv[3])
require_improvement = bool(int(sys.argv[4]))
work_identity_min_pct = float(sys.argv[5])
allow_layout_change = bool(int(sys.argv[6]))
max_symbol_address_delta = int(sys.argv[7]) if sys.argv[7] else None
case_paths = [Path(p) for p in sys.argv[8:]]

cases = [json.loads(path.read_text(encoding="utf-8")) for path in case_paths]
baseline_cases = {}
if baseline_path and baseline_path.is_file():
    baseline = json.loads(baseline_path.read_text(encoding="utf-8"))
    for case in baseline.get("cases", []):
        baseline_cases[case.get("label")] = case

compare_fields = [
    ("timing", "scene_vb"),
    ("timing", "loop_vb"),
    ("timing", "overrun_vb"),
    ("cd", "blocking_vb"),
    ("prefetch", "overrun_vb"),
]

work_identity_fields = [
    ("timing", "render"),
    ("gfx", "restore_calls"),
    ("gfx", "compose_calls"),
    ("gfx", "upload_calls"),
]

layout_identity_paths = [
    ("scene.pack_lba", ("sections", "scene", "pack_lba")),
    ("build.ps_exe.sector_bucket_bytes", ("build", "ps_exe", "sector_bucket_bytes")),
]

def field(case, section, key):
    value = case.get("sections", {}).get(section, {}).get(key)
    return value if isinstance(value, int) else None

def path_value(case, path):
    value = case
    for part in path:
        if not isinstance(value, dict):
            return None
        value = value.get(part)
    return value

def hot_symbols(case):
    symbols = path_value(case, ("build", "map", "symbols"))
    return symbols if isinstance(symbols, dict) else {}

def case_identity_from_case(case):
    sections = case.get("sections", {})
    scene = sections.get("scene", {}) if isinstance(sections, dict) else {}
    expected = case.get("expected", {})
    expected = expected if isinstance(expected, dict) else {}
    identity = {
        "scene": scene.get("scene", expected.get("scene")),
        "lowtide": scene.get("lowtide", expected.get("lowtide")),
        "night": scene.get("night", expected.get("night")),
        "holiday": scene.get("holiday", expected.get("holiday")),
        "raft": scene.get("raft", expected.get("raft")),
        "pos": scene.get("pos", expected.get("pos")),
    }
    normalized = {}
    for key, value in identity.items():
        if value is None or value == "":
            continue
        if key == "scene":
            normalized[key] = str(value).lower()
        else:
            normalized[key] = value
    return normalized

def synthesize_fingerprint(case):
    existing = case.get("fingerprint")
    if isinstance(existing, dict):
        if "case" in existing:
            return existing
        merged = dict(existing)
        merged["case"] = case_identity_from_case(case)
        return merged
    sections = case.get("sections", {})
    timing = sections.get("timing", {})
    setup = sections.get("setup", {})
    cd = sections.get("cd", {})
    prefetch = sections.get("prefetch", {})
    scene = sections.get("scene", {})
    build = case.get("build", {})
    ps_exe = build.get("ps_exe", {})
    elf = build.get("elf", {})
    build_map = build.get("map", {})
    symbols = hot_symbols(case)
    return {
        "case": case_identity_from_case(case),
        "metrics": {
            "scene_vb": timing.get("scene_vb"),
            "loop_vb": timing.get("loop_vb"),
            "target_vb": timing.get("target_vb"),
            "setup_vb": setup.get("setup_vb"),
            "blocking_vb": cd.get("blocking_vb"),
            "prefetch_overrun_vb": prefetch.get("overrun_vb"),
            "loop_reads": cd.get("loop_reads"),
        },
        "layout": {
            "pack_lba": scene.get("pack_lba"),
            "pack_sectors": scene.get("pack_sectors"),
            "ps_exe_bytes": ps_exe.get("bytes"),
            "ps_exe_sector_bucket_bytes": ps_exe.get("sector_bucket_bytes"),
            "elf_bytes": elf.get("bytes"),
            "map_bytes": build_map.get("bytes"),
        },
        "hot_symbol_sizes": {
            name: item.get("size")
            for name, item in sorted(symbols.items())
            if isinstance(item, dict) and isinstance(item.get("size"), int)
        },
    }

def run_git(case):
    value = path_value(case, ("run", "git"))
    return value if isinstance(value, dict) else {}

def fingerprint_deltas(current, previous):
    deltas = {}
    for section in ("case", "metrics", "layout", "hot_symbol_sizes"):
        section_deltas = {}
        current_section = current.get(section, {})
        previous_section = previous.get(section, {})
        if not isinstance(current_section, dict) or not isinstance(previous_section, dict):
            continue
        for key in sorted(set(current_section) | set(previous_section)):
            c_value = current_section.get(key)
            p_value = previous_section.get(key)
            if c_value == p_value:
                continue
            item = {"baseline": p_value, "current": c_value}
            if isinstance(c_value, int) and isinstance(p_value, int):
                item["delta"] = c_value - p_value
            section_deltas[key] = item
        if section_deltas:
            deltas[section] = section_deltas
    return deltas

overall_failures = []
for case in cases:
    label = case["label"]
    gate = case.setdefault("gate", {"pass": True, "failures": [], "warnings": []})
    failures = gate.setdefault("failures", [])
    warnings = gate.setdefault("warnings", [])
    base = baseline_cases.get(label)
    improved = False
    if base:
        current_fingerprint = synthesize_fingerprint(case)
        previous_fingerprint = synthesize_fingerprint(base)
        fingerprint = {
            "baseline_has_stored_fingerprint": isinstance(base.get("fingerprint"), dict),
            "baseline": previous_fingerprint,
            "current": current_fingerprint,
            "deltas": fingerprint_deltas(current_fingerprint, previous_fingerprint),
        }
        base_git = run_git(base)
        current_git = run_git(case)
        if not fingerprint["baseline_has_stored_fingerprint"]:
            warnings.append("baseline missing stored fingerprint; stale-baseline detection is limited")
        if base_git:
            fingerprint["baseline_git"] = base_git
            fingerprint["current_git"] = current_git
            baseline_commit = base_git.get("commit")
            current_commit = current_git.get("commit")
            if baseline_commit and current_commit and baseline_commit != current_commit:
                warnings.append(
                    f"baseline git commit differs: baseline={baseline_commit} current={current_commit}"
                )
            if base_git.get("dirty"):
                warnings.append("baseline was generated from a dirty worktree")
        else:
            warnings.append("baseline missing run.git metadata; stale-baseline commit check is unavailable")
        case["baseline_fingerprint_comparison"] = fingerprint
        comparisons = []
        case_identity = []
        work_identity = []
        layout_identity = []
        symbol_layout = []
        current_case = current_fingerprint.get("case", {})
        previous_case = previous_fingerprint.get("case", {})
        if isinstance(current_case, dict) and isinstance(previous_case, dict):
            for key in sorted(set(current_case) | set(previous_case)):
                current = current_case.get(key)
                previous = previous_case.get(key)
                if current == previous:
                    continue
                case_identity.append({
                    "field": f"case.{key}",
                    "baseline": previous,
                    "current": current,
                })
                failures.append(
                    f"case identity {key}: baseline={previous} current={current}"
                )
        for section, key in compare_fields:
            current = field(case, section, key)
            previous = field(base, section, key)
            if current is None or previous is None:
                if section == "timing" and key == "scene_vb":
                    warnings.append("baseline comparison skipped timing.scene_vb; metric missing from current or baseline")
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
        if work_identity_min_pct > 0:
            for section, key in work_identity_fields:
                current = field(case, section, key)
                previous = field(base, section, key)
                if current is None or previous is None or previous <= 0:
                    continue
                minimum = previous * (work_identity_min_pct / 100.0)
                delta = current - previous
                work_identity.append({
                    "field": f"{section}.{key}",
                    "baseline": previous,
                    "current": current,
                    "delta": delta,
                    "minimum_percent": work_identity_min_pct,
                    "minimum_allowed": minimum,
                })
                if current < minimum:
                    failures.append(
                        f"work identity {section}.{key}: baseline={previous} current={current} minimum={minimum:.2f}"
                    )
        for name, path in layout_identity_paths:
            current = path_value(case, path)
            previous = path_value(base, path)
            if current is None or previous is None:
                continue
            layout_identity.append({
                "field": name,
                "baseline": previous,
                "current": current,
                "delta": current - previous if isinstance(current, int) and isinstance(previous, int) else None,
                "allowed": allow_layout_change,
            })
            if current != previous and not allow_layout_change:
                failures.append(f"layout identity {name}: baseline={previous} current={current}")
        case["baseline_comparison"] = comparisons
        if case_identity:
            case["case_identity_comparison"] = case_identity
        if work_identity:
            case["work_identity_comparison"] = work_identity
        if layout_identity:
            case["layout_identity_comparison"] = layout_identity
        current_symbols = hot_symbols(case)
        previous_symbols = hot_symbols(base)
        for name in sorted(set(current_symbols) & set(previous_symbols)):
            current = current_symbols[name]
            previous = previous_symbols[name]
            if not isinstance(current, dict) or not isinstance(previous, dict):
                continue
            current_address = current.get("address")
            previous_address = previous.get("address")
            current_size = current.get("size")
            previous_size = previous.get("size")
            if not all(isinstance(value, int) for value in (
                current_address,
                previous_address,
                current_size,
                previous_size,
            )):
                continue
            address_delta = current_address - previous_address
            size_delta = current_size - previous_size
            if address_delta == 0 and size_delta == 0:
                continue
            symbol_layout.append({
                "symbol": name,
                "baseline_address": previous_address,
                "current_address": current_address,
                "address_delta": address_delta,
                "baseline_size": previous_size,
                "current_size": current_size,
                "size_delta": size_delta,
                "max_address_delta_allowed": max_symbol_address_delta,
            })
            if (max_symbol_address_delta is not None and
                    abs(address_delta) > max_symbol_address_delta):
                failures.append(
                    f"symbol address {name}: baseline={previous_address} "
                    f"current={current_address} delta={address_delta} "
                    f"allowed={max_symbol_address_delta}"
                )
        if symbol_layout:
            case["symbol_layout_comparison"] = symbol_layout
        if require_improvement and not improved:
            failures.append("no key metric improved vs baseline")
    elif baseline_path:
        warnings.append(f"no matching baseline case for {label}")
        if require_improvement:
            failures.append(
                f"no matching baseline case for {label}; cannot enforce --require-improvement"
            )
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
        f"scene_vb={timing.get('scene_vb')} "
        f"loop_vb={timing.get('loop_vb')} target_vb={timing.get('target_vb')} "
        f"blocking_vb={cd.get('blocking_vb')} "
        f"policy={prefetch.get('policy')} hits={prefetch.get('hits')} "
        f"due_misses={prefetch.get('due_misses')}"
    )
    trans = case.get("transitions")
    if trans:
        hit_rate = trans.get("hit_rate")
        rate_text = f"{hit_rate:.0%}" if isinstance(hit_rate, float) else "n/a"
        print(
            f"  transitions: scenes={trans.get('complete_scenes')}/{trans.get('expected_scenes')} "
            f"staged_hits={trans.get('staged_hits')}/{trans.get('post_boot_transitions')} ({rate_text}) "
            f"max_setup_vb hit={trans.get('max_setup_vb_hit')} cold={trans.get('max_setup_vb_cold')} "
            f"boot={trans.get('boot_setup_vb')}"
        )
        for rec in trans.get("records", []):
            print(
                f"    [{rec['index']:>2}] {rec['scene']:<14} {rec['kind']:<4} "
                f"setup_vb={rec['setup_vb']:>4} gap_vb={rec.get('gap_vb', 0):>4} "
                f"adopt={rec['stage_adopt']} "
                f"screen={rec['screen_vb']} backdrop={rec['backdrop_vb']} "
                f"pack_start={rec['pack_start_vb']} clean_rect={rec['clean_rect_vb']} "
                f"reads={rec['setup_reads']} bytes={rec['setup_bytes']}"
            )
    for failure in gate.get("failures", []):
        print(f"  failure: {failure}")
    for warning in gate.get("warnings", [])[:6]:
        print(f"  warning: {warning}")
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
    layout = case.get("layout_identity_comparison", [])
    if layout:
        parts = []
        for item in layout:
            field = item.get("field")
            baseline = item.get("baseline")
            current = item.get("current")
            delta = item.get("delta")
            if isinstance(delta, int):
                sign = "+" if delta > 0 else ""
                parts.append(f"{field} {baseline}->{current} ({sign}{delta})")
            else:
                parts.append(f"{field} {baseline}->{current}")
        print("  layout: " + "; ".join(parts))
    symbol_layout = case.get("symbol_layout_comparison", [])
    if symbol_layout:
        parts = []
        for item in symbol_layout[:6]:
            symbol = item.get("symbol")
            address_delta = item.get("address_delta")
            size_delta = item.get("size_delta")
            address_sign = "+" if isinstance(address_delta, int) and address_delta > 0 else ""
            size_sign = "+" if isinstance(size_delta, int) and size_delta > 0 else ""
            parts.append(f"{symbol} addr {address_sign}{address_delta} size {size_sign}{size_delta}")
        suffix = "" if len(symbol_layout) <= 6 else f"; +{len(symbol_layout) - 6} more"
        print("  symbols: " + "; ".join(parts) + suffix)
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
