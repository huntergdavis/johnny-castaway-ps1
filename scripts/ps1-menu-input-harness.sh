#!/usr/bin/env bash
# Build a PS1 disc with scripted controller input, run it headlessly, and
# update the website menu guide from captured menu screenshots.

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

FRAMES=12000
INTERVAL=5
TIMEOUT=420
OUTPUT_ROOT="scratch/menu-input-harness"
SKIP_BUILD=0
VERBOSE=0
RAW_CONSOLE=0
SETTLE_FRAMES=480
TAP_FRAMES=12

usage() {
    cat <<'USAGE'
Usage: ps1-menu-input-harness.sh [options]

Options:
  --frames N          Frames to run in DuckStation (default: 12000)
  --interval N        Capture every Nth frame (default: 5)
  --timeout SECS      Wall-clock timeout (default: 420)
  --settle-frames N   Frames to wait before each screenshot marker (default: 480)
                      Use 30 for targeted half-second diagnostics.
  --tap-frames N      Frames to hold each navigation button (default: 12)
  --output-root DIR   Regtest output root (default: scratch/menu-input-harness)
  --skip-build        Reuse the current CD image
  --verbose           Use pad-script-log to print parsed events
  --raw-console       Stream the full DuckStation log to stdout
  --help              Show this help
USAGE
}

while [ $# -gt 0 ]; do
    case "$1" in
        --frames) FRAMES="$2"; shift 2 ;;
        --interval|--dumpinterval) INTERVAL="$2"; shift 2 ;;
        --timeout) TIMEOUT="$2"; shift 2 ;;
        --settle-frames) SETTLE_FRAMES="$2"; shift 2 ;;
        --tap-frames) TAP_FRAMES="$2"; shift 2 ;;
        --output-root|--dumpdir) OUTPUT_ROOT="$2"; shift 2 ;;
        --skip-build) SKIP_BUILD=1; shift ;;
        --verbose) VERBOSE=1; shift ;;
        --quiet) VERBOSE=0; shift ;;
        --raw-console) RAW_CONSOLE=1; shift ;;
        --help|-h) usage; exit 0 ;;
        *) echo "Unknown option: $1" >&2; usage >&2; exit 1 ;;
    esac
done

TMP_DIR="$(mktemp -d /tmp/jc-menu-harness-XXXXXX)"
FILES_TO_RESTORE=(
    "config/ps1/BOOTMODE.TXT"
    "config/ps1/bootmode_embedded.h"
    "config/ps1/PADSCRIPT.TXT"
    "config/ps1/padscript_embedded.h"
)

backup_files() {
    for path in "${FILES_TO_RESTORE[@]}"; do
        mkdir -p "$TMP_DIR/$(dirname "$path")"
        if [ -f "$path" ]; then
            cp "$path" "$TMP_DIR/$path"
        else
            touch "$TMP_DIR/$path.missing"
        fi
    done
}

restore_files() {
    for path in "${FILES_TO_RESTORE[@]}"; do
        if [ -f "$TMP_DIR/$path.missing" ]; then
            rm -f "$path"
        elif [ -f "$TMP_DIR/$path" ]; then
            cp "$TMP_DIR/$path" "$path"
        fi
    done
    rm -rf "$TMP_DIR"
}

write_pad_script() {
    # Pause-main rows in the post-v0.8.4 menu (src/pause_menu/pause_menu.c::drawMainMenu):
    #   0 Resume
    #   1 Scene Set Options...
    #   2 Scene Explorer
    #   3 Freeplay: ON/OFF
    #   4 Freeplay Options
    #   5 World Options
    #   6 Accessibility
    #   7 System
    #
    # Each top-level visit returns to pause-main with the cursor parked on
    # the row we just opened, so subsequent Down counts are relative to the
    # previous visit.
    cat > config/ps1/PADSCRIPT.TXT <<PADSCRIPT
wait 30s
tap START 600
shot pause-main ${SETTLE_FRAMES}

# Scene Set Options (row 1) — cursor at 0
tap DOWN ${TAP_FRAMES}
tap CROSS ${TAP_FRAMES}
shot scene-set ${SETTLE_FRAMES}
tap CIRCLE ${TAP_FRAMES}
wait 30

# Scene Explorer (row 2) — cursor returned to row 1
tap DOWN ${TAP_FRAMES}
tap CROSS ${TAP_FRAMES}
shot scene-explorer ${SETTLE_FRAMES}
tap CIRCLE ${TAP_FRAMES}
wait 30

# Freeplay Options (row 4) — skip Freeplay toggle at row 3; cursor at 2
tap DOWN ${TAP_FRAMES}
tap DOWN ${TAP_FRAMES}
tap CROSS ${TAP_FRAMES}
shot freeplay-options ${SETTLE_FRAMES}

# freeplay-gags (Freeplay Options, row 0)
tap CROSS ${TAP_FRAMES}
shot freeplay-gags ${SETTLE_FRAMES}
tap CIRCLE ${TAP_FRAMES}
wait 30

# freeplay-visitors (Freeplay Options, row 1)
tap DOWN ${TAP_FRAMES}
tap CROSS ${TAP_FRAMES}
shot freeplay-visitors ${SETTLE_FRAMES}
tap CIRCLE ${TAP_FRAMES}
wait 30

# controls (Freeplay Options, row 2)
tap DOWN ${TAP_FRAMES}
tap CROSS ${TAP_FRAMES}
shot controls ${SETTLE_FRAMES}
tap CIRCLE ${TAP_FRAMES}
wait 30

# Back to pause-main; cursor on row 4 (Freeplay Options)
tap CIRCLE ${TAP_FRAMES}
wait 30

# World Options (row 5) — 1 down from row 4
tap DOWN ${TAP_FRAMES}
tap CROSS ${TAP_FRAMES}
shot world-options ${SETTLE_FRAMES}

# holidays (World Options, row 3 — Day/Night, Tide, Raft, Holidays)
tap DOWN ${TAP_FRAMES}
tap DOWN ${TAP_FRAMES}
tap DOWN ${TAP_FRAMES}
tap CROSS ${TAP_FRAMES}
shot holidays ${SETTLE_FRAMES}
tap CIRCLE ${TAP_FRAMES}
wait 30

# island-position (World Options, row 4)
tap DOWN ${TAP_FRAMES}
tap CROSS ${TAP_FRAMES}
shot island-position ${SETTLE_FRAMES}
tap CIRCLE ${TAP_FRAMES}
wait 30

# Back to pause-main; cursor on row 5 (World Options)
tap CIRCLE ${TAP_FRAMES}
wait 30

# Accessibility (row 6) — 1 down from row 5
tap DOWN ${TAP_FRAMES}
tap CROSS ${TAP_FRAMES}
shot accessibility ${SETTLE_FRAMES}

# sound-test (Accessibility, row 3)
tap DOWN ${TAP_FRAMES}
tap DOWN ${TAP_FRAMES}
tap DOWN ${TAP_FRAMES}
tap CROSS ${TAP_FRAMES}
shot sound-test ${SETTLE_FRAMES}
tap CIRCLE ${TAP_FRAMES}
wait 30

# Back to pause-main; cursor on row 6 (Accessibility)
tap CIRCLE ${TAP_FRAMES}
wait 30

# System (row 7) — 1 down from row 6
tap DOWN ${TAP_FRAMES}
tap CROSS ${TAP_FRAMES}
shot system ${SETTLE_FRAMES}

# set-time-date (System, row 1)
tap DOWN ${TAP_FRAMES}
tap CROSS ${TAP_FRAMES}
shot set-time-date ${SETTLE_FRAMES}
tap CIRCLE ${TAP_FRAMES}
wait 30

# set-rng-seed (System, row 2)
tap DOWN ${TAP_FRAMES}
tap CROSS ${TAP_FRAMES}
shot set-rng-seed ${SETTLE_FRAMES}
tap CIRCLE ${TAP_FRAMES}
wait 30

# Back to pause-main; final marker
tap CIRCLE ${TAP_FRAMES}
shot pause-main-return ${SETTLE_FRAMES}
PADSCRIPT
}

backup_files
trap restore_files EXIT

PAD_TOKEN="pad-script"
if [ "$VERBOSE" -eq 1 ]; then
    PAD_TOKEN="pad-script-log"
fi

cat > config/ps1/BOOTMODE.TXT <<BOOTMODE
fgpilot building5 lowtide 0 night 0 holiday 0 raft-stage 0 island-pos -154 54 seed 1 ${PAD_TOKEN}
BOOTMODE
write_pad_script

if [ "$SKIP_BUILD" -eq 0 ]; then
    ./scripts/build-ps1.sh
    ./scripts/make-cd-image.sh
fi

mkdir -p "$OUTPUT_ROOT"
REGTEST_ARGS=(
    --frames "$FRAMES"
    --dumpinterval "$INTERVAL"
    --dumpdir "$OUTPUT_ROOT"
    --timeout "$TIMEOUT"
)
if [ "$RAW_CONSOLE" -eq 1 ]; then
    REGTEST_ARGS+=(--raw-console)
fi

./scripts/run-regtest.sh "${REGTEST_ARGS[@]}"

RUN_DIR="$(find "$OUTPUT_ROOT" -mindepth 1 -maxdepth 1 -type d | sort | tail -n 1)"
if [ -z "$RUN_DIR" ]; then
    echo "ERROR: no regtest run directory found under $OUTPUT_ROOT" >&2
    exit 1
fi

python3 scripts/ps1-menu-harness-report.py "$RUN_DIR"

echo "Menu harness run: $RUN_DIR"
