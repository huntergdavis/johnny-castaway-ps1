#!/bin/bash
# Full PS1 Rebuild and Let Run - Builds and launches, keeps running until manual kill
# Usage: ./rebuild-and-let-run.sh [noclean]

set -e  # Exit on error

if [ "$(id -u)" = "0" ]; then
    echo "ERROR: Do not run this script as root/sudo." >&2
    exit 1
fi

cd "$(dirname "$0")/.."  # Change to project root

SCRATCH_DIR="$PWD/scratch"
mkdir -p "$SCRATCH_DIR"

BOOTMODE_FILE="$PWD/config/ps1/BOOTMODE.TXT"
BOOTMODE_HEADER="$PWD/config/ps1/bootmode_embedded.h"
if [ -n "${DUCKSTATION_CONFIG_DIR:-}" ]; then
    DUCK_CONFIG_DIR="$DUCKSTATION_CONFIG_DIR"
elif command -v flatpak >/dev/null 2>&1 && flatpak info org.duckstation.DuckStation >/dev/null 2>&1; then
    DUCK_CONFIG_DIR="$HOME/.var/app/org.duckstation.DuckStation/config/duckstation"
else
    DUCK_CONFIG_DIR="$HOME/.local/share/duckstation"
fi
DUCK_SETTINGS="$DUCK_CONFIG_DIR/settings.ini"
DUCK_LOG_FILE="${DUCKSTATION_LOG_FILE:-$DUCK_CONFIG_DIR/duckstation.log}"
DUCK_LOG_MAX_BYTES="${DUCKSTATION_LOG_MAX_BYTES:-2147483648}"
DUCK_SETTINGS_BACKUP=""
BOOTMODE_BACKUP=""
BOOTMODE_HEADER_BACKUP=""
BOOT_OVERRIDE=""
LOG_WATCHDOG_PID=""

start_duckstation() {
    if [ -n "${DUCKSTATION_LAUNCHER:-}" ]; then
        read -r -a duckstation_launcher <<< "$DUCKSTATION_LAUNCHER"
        "${duckstation_launcher[@]}" "$CUE_FILE" &
        DUCK_PID=$!
        return
    fi

    if command -v flatpak >/dev/null 2>&1 && flatpak info org.duckstation.DuckStation >/dev/null 2>&1; then
        flatpak run --filesystem="$(dirname "$CUE_FILE")" org.duckstation.DuckStation "$CUE_FILE" &
        DUCK_PID=$!
        return
    fi

    if command -v duckstation >/dev/null 2>&1; then
        duckstation "$CUE_FILE" &
        DUCK_PID=$!
        return
    fi

    echo "ERROR: DuckStation launcher not found. Install the Flatpak or set DUCKSTATION_LAUNCHER." >&2
    exit 127
}

prepare_duckstation_test_settings() {
    # Patch the needed test keys IN PLACE — no backup/restore round-trip.
    # The old exit-time restore reverted EVERYTHING the user (or DuckStation
    # itself) changed during the session: controller bindings, the completed
    # setup wizard, renderer choices. The keys below are idempotent and
    # harmless to leave set between sessions.
    mkdir -p "$(dirname "$DUCK_SETTINGS")"

    python3 - "$DUCK_SETTINGS" <<'PY'
import configparser
import sys
from pathlib import Path

settings = Path(sys.argv[1])
cp = configparser.ConfigParser()
cp.optionxform = str
if settings.is_file():
    cp.read(settings, encoding="utf-8")
for section in ("Main", "BIOS", "SIO", "Logging"):
    if section not in cp:
        cp[section] = {}
# The prepare/restore round-trip previously dropped the wizard-completion
# flag, so every launch re-ran the DuckStation setup wizard (and a completion
# during the session was reverted by the exit-time settings restore).
cp["Main"]["SetupWizardIncomplete"] = "false"
cp["BIOS"]["TTYLogging"] = "true"
cp["SIO"]["RedirectToTTY"] = "true"
cp["Logging"]["LogLevel"] = "Dev"
cp["Logging"]["LogToFile"] = "true"
cp["Logging"]["LogTimestamps"] = "true"
cp["Logging"]["LogFileTimestamps"] = "true"
with settings.open("w", encoding="utf-8") as f:
    cp.write(f)
PY
}

restore_duckstation_test_settings() {
    # Intentionally a no-op (see prepare_duckstation_test_settings): restoring
    # a launch-time snapshot clobbered controller bindings and wizard
    # completion the user saved during the session.
    :
}

stop_duckstation_log_watchdog() {
    if [ -n "$LOG_WATCHDOG_PID" ]; then
        kill "$LOG_WATCHDOG_PID" 2>/dev/null || true
        wait "$LOG_WATCHDOG_PID" 2>/dev/null || true
        LOG_WATCHDOG_PID=""
    fi
}

cap_duckstation_log() {
    if [ "${DUCK_LOG_MAX_BYTES:-0}" -le 0 ] 2>/dev/null; then
        return
    fi
    if [ ! -f "$DUCK_LOG_FILE" ]; then
        return
    fi

    local size
    size=$(stat -c %s "$DUCK_LOG_FILE" 2>/dev/null || printf '0')
    if [ "$size" -ge "$DUCK_LOG_MAX_BYTES" ] 2>/dev/null; then
        echo "DuckStation log reached ${size} bytes; truncating $DUCK_LOG_FILE"
        : > "$DUCK_LOG_FILE"
    fi
}

if [ "${1:-}" = "noclean" ]; then
    BUILD_MODE="noclean"
    shift
else
    BUILD_MODE=""
fi

if [ "$#" -gt 0 ]; then
    BOOT_OVERRIDE="$*"
fi

echo "======================================"
echo "PS1 Full Rebuild and Let Run"
echo "======================================"
echo ""

stage_boot_override() {
    if [ ! -f "$BOOTMODE_FILE" ]; then
        return
    fi

    if [ -z "$BOOT_OVERRIDE" ]; then
        return
    fi

    BOOTMODE_BACKUP="$SCRATCH_DIR/ps1-bootmode-$$.txt"
    cp "$BOOTMODE_FILE" "$BOOTMODE_BACKUP"
    if [ -f "$BOOTMODE_HEADER" ]; then
        BOOTMODE_HEADER_BACKUP="$SCRATCH_DIR/ps1-bootmode-embedded-$$.h"
        cp "$BOOTMODE_HEADER" "$BOOTMODE_HEADER_BACKUP"
    fi

    printf '%s\n' "$BOOT_OVERRIDE" > "$BOOTMODE_FILE"
    echo "=== Boot override ==="
    echo "$BOOT_OVERRIDE"
    echo ""
}

restore_boot_override() {
    if [ -n "$BOOTMODE_BACKUP" ] && [ -f "$BOOTMODE_BACKUP" ]; then
        cp "$BOOTMODE_BACKUP" "$BOOTMODE_FILE"
        rm -f "$BOOTMODE_BACKUP"
    fi
    if [ -n "$BOOTMODE_HEADER_BACKUP" ] && [ -f "$BOOTMODE_HEADER_BACKUP" ]; then
        cp "$BOOTMODE_HEADER_BACKUP" "$BOOTMODE_HEADER"
        rm -f "$BOOTMODE_HEADER_BACKUP"
    fi
}

trap 'stop_duckstation_log_watchdog; restore_duckstation_test_settings; restore_boot_override' EXIT

# Step 1: Stage boot override and build executable
stage_boot_override

if [ "$BUILD_MODE" = "noclean" ]; then
    echo "=== Incremental build (noclean mode) ==="
    ./scripts/build-ps1.sh
else
    echo "=== Clean build (default) ==="
    ./scripts/build-ps1.sh clean
fi

echo ""

# Step 2: Create CD image
./scripts/make-cd-image.sh

echo ""

# Step 3: Leave existing DuckStation instances alone by default. Set
# KILL_EXISTING_DUCKSTATION=1 when a deliberately clean emulator session is
# needed for a capture run.
if [ "${KILL_EXISTING_DUCKSTATION:-0}" = "1" ]; then
    echo "=== Cleaning up old DuckStation instances ==="
    pkill -9 -f "[d]uckstation" 2>/dev/null || true
    sleep 1
else
    echo "=== Leaving existing DuckStation instances running ==="
fi

# Step 4: Launch DuckStation and let it run
echo "=== Launching DuckStation (will keep running) ==="

SCREENSHOT_DIR="$DUCK_CONFIG_DIR/screenshots"
CUE_FILE="$PWD/johnnycastawayps1.cue"
CAPTURE_INTERVAL=${PS1_CAPTURE_INTERVAL:-5}
INITIAL_CAPTURE_WAIT=${PS1_INITIAL_CAPTURE_WAIT:-35}
CAPTURE_COUNT=${PS1_CAPTURE_COUNT:-4}
RUN_TIMEOUT_SECONDS="${RUN_TIMEOUT_SECONDS:-0}"

mkdir -p "$SCREENSHOT_DIR"

take_duckstation_screenshot() {
    local out_var="$1"
    local marker="$SCRATCH_DIR/.ps1_shot_marker_$$"
    : > "$marker"
    local latest=""
    local window_id=""

    if command -v xdotool >/dev/null 2>&1; then
        window_id=$(xdotool search --onlyvisible --name "DuckStation" 2>/dev/null | tail -1 || true)
        if [ -n "$window_id" ]; then
            xdotool windowactivate --sync "$window_id" 2>/dev/null || true
            sleep 0.5
            xdotool key --window "$window_id" F10 2>/dev/null || true
            sleep 1
        fi
        latest=$(find "$SCREENSHOT_DIR" -name "*.png" -newer "$marker" 2>/dev/null | sort | tail -1)
        if [ -n "$latest" ]; then
            printf -v "$out_var" '%s' "$latest"
            rm -f "$marker"
            return 0
        fi
    fi

    if command -v spectacle >/dev/null 2>&1 && { [ -n "$window_id" ] || [ "${PS1_ALLOW_FALLBACK_CAPTURE:-0}" = "1" ]; }; then
        if [ -n "$window_id" ] && command -v xdotool >/dev/null 2>&1; then
            xdotool windowactivate --sync "$window_id" 2>/dev/null || true
            sleep 0.5
        fi
        local fallback="$SCREENSHOT_DIR/ps1-test-$(date +%Y%m%d-%H%M%S).png"
        spectacle -b -n -a -e -S -d 300 -o "$fallback" >/dev/null 2>&1 || true
        if [ -f "$fallback" ]; then
            printf -v "$out_var" '%s' "$fallback"
            rm -f "$marker"
            return 0
        fi
    fi

    echo "WARNING: DuckStation-native screenshot capture failed; window fallback capture also failed." >&2
    rm -f "$marker"
    return 1
}

sleep_while_duckstation_alive() {
    local remaining="$1"
    local step

    while [ "$remaining" -gt 0 ]; do
        if ! kill -0 "$DUCK_PID" 2>/dev/null; then
            return 1
        fi
        step=1
        if [ "$remaining" -gt 5 ]; then
            step=5
        fi
        sleep "$step"
        remaining=$((remaining - step))
    done

    kill -0 "$DUCK_PID" 2>/dev/null
}

terminate_duckstation_for_cue() {
    kill -TERM "$DUCK_PID" 2>/dev/null || true
    pkill -TERM -f "([d]uckstation-qt|[D]uckStation[^[:space:]]*|org[.]duckstation[.]DuckStation).*${CUE_FILE}" 2>/dev/null || true
    sleep 5
    kill -KILL "$DUCK_PID" 2>/dev/null || true
    pkill -KILL -f "([d]uckstation-qt|[D]uckStation[^[:space:]]*|org[.]duckstation[.]DuckStation).*${CUE_FILE}" 2>/dev/null || true
}

# Launch DuckStation with fast boot
cap_duckstation_log
prepare_duckstation_test_settings
start_duckstation

if [ "${DUCK_LOG_MAX_BYTES:-0}" -gt 0 ] 2>/dev/null; then
    (
        while kill -0 "$DUCK_PID" 2>/dev/null; do
            sleep 60
            cap_duckstation_log
        done
    ) &
    LOG_WATCHDOG_PID=$!
fi

# Background watchdog: if DuckStation is still alive after the deadline,
# TERM it (then KILL if that doesn't take). The deadline starts when the
# emulator launches, not after screenshot capture completes.
if [ "$RUN_TIMEOUT_SECONDS" -gt 0 ]; then
    (
        sleep "$RUN_TIMEOUT_SECONDS"
        if kill -0 "$DUCK_PID" 2>/dev/null; then
            echo "" >&2
            echo "rebuild-and-let-run.sh: emergency timeout reached, killing DuckStation (pid $DUCK_PID)." >&2
            terminate_duckstation_for_cue
        fi
    ) &
    WATCHDOG_PID=$!
fi

echo "DuckStation PID: $DUCK_PID"
if [ "$CAPTURE_COUNT" -gt 0 ]; then
    echo "Waiting ${INITIAL_CAPTURE_WAIT} seconds for initial screenshot..."

    if ! sleep_while_duckstation_alive "$INITIAL_CAPTURE_WAIT"; then
        echo "DuckStation exited before initial screenshot."
    fi

    if kill -0 "$DUCK_PID" 2>/dev/null; then
        if take_duckstation_screenshot SCREENSHOT_FILE; then
            echo "Screenshot 1/${CAPTURE_COUNT} saved to: $SCREENSHOT_FILE"
        fi
    fi
fi

for ((capture_index=2; capture_index<=CAPTURE_COUNT; capture_index++)); do
    echo "Waiting ${CAPTURE_INTERVAL} more seconds for screenshot ${capture_index}..."
    if ! sleep_while_duckstation_alive "$CAPTURE_INTERVAL"; then
        echo "DuckStation exited before screenshot ${capture_index}."
        break
    fi
    if kill -0 "$DUCK_PID" 2>/dev/null; then
        if take_duckstation_screenshot SCREENSHOT_FILE2; then
            echo "Screenshot ${capture_index}/${CAPTURE_COUNT} saved to: $SCREENSHOT_FILE2"
        fi
    fi
done

echo ""
echo "======================================"
echo "Build complete! DuckStation running..."
echo "Press Escape in emulator or Ctrl+C here to stop"
if [ "$RUN_TIMEOUT_SECONDS" -gt 0 ]; then
    echo "Emergency timeout: ${RUN_TIMEOUT_SECONDS}s (override with RUN_TIMEOUT_SECONDS=0)"
fi
echo "Screenshots in: $SCREENSHOT_DIR"
echo "======================================"

# Wait for DuckStation to exit (user closes it, or watchdog fires)
wait $DUCK_PID 2>/dev/null || true

# If Duck exited naturally before the deadline, cancel the sleeping watchdog
if [ -n "${WATCHDOG_PID:-}" ]; then
    kill "$WATCHDOG_PID" 2>/dev/null || true
    wait "$WATCHDOG_PID" 2>/dev/null || true
fi

stop_duckstation_log_watchdog

echo "DuckStation closed."
exit 0
