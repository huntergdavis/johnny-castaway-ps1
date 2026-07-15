#!/usr/bin/env bash
# Serve the Johnny Castaway browser player against this checkout's current
# BIN/CUE. Mirrors the Zoomies web prototype (pspsps-engine/scripts/run-web.sh).
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

if [[ ! -f "$PROJECT_DIR/johnnycastawayps1.bin" || ! -f "$PROJECT_DIR/johnnycastawayps1.cue" ]]; then
    echo "ERROR: build johnnycastawayps1.bin/.cue first (./scripts/make-cd-image.sh)." >&2
    exit 1
fi

exec "$SCRIPT_DIR/serve-web-local.py" --disc "$PROJECT_DIR/johnnycastawayps1.bin" "$@"
