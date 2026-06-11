#!/bin/bash
# Watch a ps1-perf-iterate headless case dir for a JCBSOD-HALT in the
# regtest log and kill the docker container when one appears. Without
# this, a halted game keeps emulating (PerfMon noise only) until the
# -frames budget runs out — hours of wasted wall clock — while holding
# .regtest-build.lock and starving every queued run.
#
# Usage: ps1-soak-watchdog.sh <case-dir>
#   <case-dir> is the per-case dir, e.g.
#   scratch/ps1-perf-iterate/<stamp>/<case>  (contains headless/ and
#   headless-regtest.log)

set -u

CASE_DIR="${1:?usage: ps1-soak-watchdog.sh <case-dir>}"
LOG="$CASE_DIR/headless-regtest.log"
CID_FILE="$CASE_DIR/headless/container.cid"

echo "watchdog: watching $LOG"
while true; do
    if [ -f "$LOG" ] && grep -q 'JCBSOD-HALT' "$LOG"; then
        echo "watchdog: JCBSOD-HALT detected at $(date)"
        grep -m 20 'JCBSOD' "$LOG"
        if [ -f "$CID_FILE" ]; then
            CID="$(cat "$CID_FILE")"
            echo "watchdog: killing container $CID"
            sudo -n docker kill "$CID" || sudo docker kill "$CID"
        else
            echo "watchdog: no container.cid at $CID_FILE; nothing to kill"
        fi
        exit 1
    fi
    # The case is over when the cid file's container is gone AND the log
    # has stopped growing; simplest robust signal is the parent harness
    # removing the container — grep for it cheaply via docker inspect.
    if [ -f "$CID_FILE" ]; then
        CID="$(cat "$CID_FILE")"
        if ! sudo -n docker inspect "$CID" >/dev/null 2>&1; then
            echo "watchdog: container gone, run ended without BSOD halt"
            exit 0
        fi
    fi
    sleep 30
done
