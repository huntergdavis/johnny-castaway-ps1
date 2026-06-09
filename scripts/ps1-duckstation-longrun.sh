#!/bin/bash
# Launch a detached visible DuckStation soak run and keep parseable diagnostics
# in scratch. This is for real emulator stability checks, not headless perf
# gates.
set -euo pipefail

if [ "$(id -u)" = "0" ]; then
    echo "ERROR: Do not run this script as root/sudo." >&2
    exit 1
fi

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$PROJECT_ROOT"

TIMEOUT_SECONDS="${PS1_LONGRUN_TIMEOUT_SECONDS:-5400}"
INITIAL_WAIT="${PS1_LONGRUN_INITIAL_WAIT:-30}"
CAPTURE_INTERVAL="${PS1_LONGRUN_CAPTURE_INTERVAL:-300}"
CAPTURE_COUNT="${PS1_LONGRUN_CAPTURE_COUNT:-18}"
MONITOR_INTERVAL="${PS1_LONGRUN_MONITOR_INTERVAL:-10}"
BUILD_MODE="noclean"
KILL_EXISTING="${PS1_LONGRUN_KILL_EXISTING:-1}"
BOOT_OVERRIDE=()

usage() {
    cat <<'USAGE'
Usage: scripts/ps1-duckstation-longrun.sh [options] [-- BOOTMODE...]

Launches a detached visible DuckStation run through rebuild-and-let-run.sh and
writes diagnostics under scratch/duckstation-longrun-<timestamp>/.

Options:
  --timeout N          Seconds before watchdog stops DuckStation (default: 5400)
  --initial-wait N     Seconds before first screenshot (default: 30)
  --capture-interval N Seconds between screenshots (default: 300)
  --capture-count N    Number of screenshots to request (default: 18)
  --monitor-interval N Seconds between monitor.tsv rows (default: 10)
  --clean              Clean PS1 build before launch
  --keep-existing      Do not kill existing DuckStation instances first
  --boot STRING        BOOTMODE override string
  --                  Remaining args are used as BOOTMODE override tokens
  -h, --help           Show this help
USAGE
}

while [ "$#" -gt 0 ]; do
    case "$1" in
        --timeout)
            TIMEOUT_SECONDS="$2"; shift 2 ;;
        --initial-wait)
            INITIAL_WAIT="$2"; shift 2 ;;
        --capture-interval)
            CAPTURE_INTERVAL="$2"; shift 2 ;;
        --capture-count)
            CAPTURE_COUNT="$2"; shift 2 ;;
        --monitor-interval)
            MONITOR_INTERVAL="$2"; shift 2 ;;
        --clean)
            BUILD_MODE="clean"; shift ;;
        --keep-existing)
            KILL_EXISTING=0; shift ;;
        --boot)
            BOOT_OVERRIDE=("$2"); shift 2 ;;
        --)
            shift
            if [ "$#" -gt 0 ]; then
                BOOT_OVERRIDE=("$*")
            fi
            break ;;
        -h|--help)
            usage; exit 0 ;;
        *)
            echo "Unknown option: $1" >&2
            usage >&2
            exit 1 ;;
    esac
done

case "$TIMEOUT_SECONDS:$INITIAL_WAIT:$CAPTURE_INTERVAL:$CAPTURE_COUNT:$MONITOR_INTERVAL" in
    *[!0-9:]*)
        echo "ERROR: numeric options must be non-negative integers." >&2
        exit 1 ;;
esac

RUN_TS="$(date +%Y%m%d-%H%M%S)"
RUN_DIR="$PROJECT_ROOT/scratch/duckstation-longrun-$RUN_TS"
mkdir -p "$RUN_DIR"

DUCK_LOG="$HOME/.var/app/org.duckstation.DuckStation/config/duckstation/duckstation.log"
SCREENSHOT_DIR="$HOME/.var/app/org.duckstation.DuckStation/config/duckstation/screenshots"
BOOTMODE="$(tr '\n' ' ' < "$PROJECT_ROOT/config/ps1/BOOTMODE.TXT" 2>/dev/null || true)"

cat > "$RUN_DIR/meta.txt" <<EOF
run_dir=$RUN_DIR
started=$(date -Is)
timeout_seconds=$TIMEOUT_SECONDS
initial_wait=$INITIAL_WAIT
capture_interval=$CAPTURE_INTERVAL
capture_count=$CAPTURE_COUNT
monitor_interval=$MONITOR_INTERVAL
build_mode=$BUILD_MODE
kill_existing=$KILL_EXISTING
bootmode_file=$BOOTMODE
boot_override=${BOOT_OVERRIDE[*]:-}
cue=$PROJECT_ROOT/jcreborn.cue
duck_log=$DUCK_LOG
screenshot_dir=$SCREENSHOT_DIR
EOF

printf 'timestamp\trunner_alive\tduck_pids\ttotal_rss_kb\tduck_log_bytes\tlatest_screenshot\n' > "$RUN_DIR/monitor.tsv"

cat > "$RUN_DIR/runner.sh" <<'EOF'
#!/bin/bash
set -u

project_root="$1"
run_dir="$2"
timeout_seconds="$3"
initial_wait="$4"
capture_interval="$5"
capture_count="$6"
monitor_interval="$7"
build_mode="$8"
kill_existing="$9"
shift 9
boot_override=("$@")

cd "$project_root" || exit 1

duck_log="$HOME/.var/app/org.duckstation.DuckStation/config/duckstation/duckstation.log"
screenshot_dir="$HOME/.var/app/org.duckstation.DuckStation/config/duckstation/screenshots"
screenshot_marker="$run_dir/.screenshot-marker"
log_marker_bytes=$(stat -c %s "$duck_log" 2>/dev/null || printf 0)
: > "$screenshot_marker"

echo "long-run wrapper started at $(date -Is)"
echo "run dir: $run_dir"

if [ "$build_mode" = "clean" ]; then
    rebuild_args=()
else
    rebuild_args=(noclean)
fi
if [ "${#boot_override[@]}" -gt 0 ]; then
    rebuild_args+=("${boot_override[@]}")
fi

if [ "$kill_existing" = "1" ]; then
    echo "=== Cleaning up old DuckStation instances ==="
    pkill -9 -f '([d]uckstation-qt|org[.]duckstation[.]DuckStation)' 2>/dev/null || true
    sleep 1
fi

KILL_EXISTING_DUCKSTATION=0 \
RUN_TIMEOUT_SECONDS="$timeout_seconds" \
PS1_INITIAL_CAPTURE_WAIT="$initial_wait" \
PS1_CAPTURE_INTERVAL="$capture_interval" \
PS1_CAPTURE_COUNT="$capture_count" \
DUCKSTATION_LOG_MAX_BYTES=1073741824 \
    ./scripts/rebuild-and-let-run.sh "${rebuild_args[@]}" &
rebuild_pid=$!
echo "$rebuild_pid" > "$run_dir/rebuild-and-let-run.pid"
echo "rebuild-and-let-run pid: $rebuild_pid"

monitor_stop="$run_dir/.monitor-stop"
rm -f "$monitor_stop"

monitor_loop() {
    while [ ! -f "$monitor_stop" ]; do
        pids=$(pgrep -f '([d]uckstation-qt|org[.]duckstation[.]DuckStation)' | tr '\n' ' ' | sed 's/[[:space:]]*$//')
        rss=0
        if [ -n "$pids" ]; then
            for p in $pids; do
                r=$(ps -o rss= -p "$p" 2>/dev/null | awk '{print $1}')
                if [ -n "$r" ]; then
                    rss=$((rss + r))
                fi
            done
        fi
        log_bytes=0
        if [ -f "$duck_log" ]; then
            log_bytes=$(stat -c %s "$duck_log" 2>/dev/null || printf 0)
            if [ "$log_bytes" -ge "$log_marker_bytes" ]; then
                tail -c +"$((log_marker_bytes + 1))" "$duck_log" 2>/dev/null \
                    | tail -n 4000 > "$run_dir/duckstation-tail.log" 2>/dev/null || true
                tail -c +"$((log_marker_bytes + 1))" "$duck_log" 2>/dev/null \
                    | tail -n 4000 \
                    | grep -aEi 'JCPERF|JCPICK|JCWALK|JCSPU|JCMEM|JCBOOT|fgpilot|foreground|scene|BSOD|panic|fatal|error|warn|heap|memory|alloc|assert|UnknownReadHandler|Invalid .*read' \
                    > "$run_dir/events-tail.log" 2>/dev/null || true
            else
                tail -n 4000 "$duck_log" > "$run_dir/duckstation-tail.log" 2>/dev/null || true
                tail -n 4000 "$duck_log" \
                    | grep -aEi 'JCPERF|JCPICK|JCWALK|JCSPU|JCMEM|JCBOOT|fgpilot|foreground|scene|BSOD|panic|fatal|error|warn|heap|memory|alloc|assert|UnknownReadHandler|Invalid .*read' \
                    > "$run_dir/events-tail.log" 2>/dev/null || true
            fi
        fi
        latest=$(find "$screenshot_dir" -name '*.png' -newer "$screenshot_marker" -printf '%T@ %p\n' 2>/dev/null \
            | sort -n | tail -1 | cut -d' ' -f2-)
        printf '%s\t1\t%s\t%s\t%s\t%s\n' "$(date -Is)" "$pids" "$rss" "$log_bytes" "$latest" >> "$run_dir/monitor.tsv"
        sleep "$monitor_interval"
    done
}

monitor_loop &
monitor_pid=$!
echo "$monitor_pid" > "$run_dir/monitor.pid"

wait "$rebuild_pid"
rc=$?
touch "$monitor_stop"
kill "$monitor_pid" 2>/dev/null || true
wait "$monitor_pid" 2>/dev/null || true

echo "rebuild-and-let-run exited rc=$rc at $(date -Is)"
if [ -f "$duck_log" ]; then
    cp "$duck_log" "$run_dir/duckstation-final.log" 2>/dev/null || true
fi
ps aux > "$run_dir/ps-final.txt" 2>/dev/null || true
printf 'finished=%s\nexit_code=%s\n' "$(date -Is)" "$rc" >> "$run_dir/meta.txt"
pids=$(pgrep -f '([d]uckstation-qt|org[.]duckstation[.]DuckStation)' | tr '\n' ' ' | sed 's/[[:space:]]*$//')
printf '%s\t0\t%s\t0\t0\t\n' "$(date -Is)" "$pids" >> "$run_dir/monitor.tsv"
exit "$rc"
EOF
chmod +x "$RUN_DIR/runner.sh"

nohup setsid "$RUN_DIR/runner.sh" \
    "$PROJECT_ROOT" "$RUN_DIR" "$TIMEOUT_SECONDS" "$INITIAL_WAIT" \
    "$CAPTURE_INTERVAL" "$CAPTURE_COUNT" "$MONITOR_INTERVAL" \
    "$BUILD_MODE" "$KILL_EXISTING" "${BOOT_OVERRIDE[@]}" \
    > "$RUN_DIR/session.log" 2>&1 < /dev/null &
LAUNCHER_PID=$!
printf '%s\n' "$LAUNCHER_PID" > "$RUN_DIR/launcher.pid"

echo "$RUN_DIR"
echo "launcher_pid=$LAUNCHER_PID"
echo "session_log=$RUN_DIR/session.log"
echo "monitor=$RUN_DIR/monitor.tsv"
