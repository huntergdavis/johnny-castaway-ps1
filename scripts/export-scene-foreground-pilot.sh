#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
OUTPUT_DIR="${1:-}"
SCENE_SLUG="${2:-fishing1}"
SCENE_NAME="${3:-FISHING 1}"
PACK_BASENAME="${4:-$(printf '%s' "$SCENE_SLUG" | tr '[:lower:]' '[:upper:]' | tr -cd 'A-Z0-9')}"
if [ "$#" -ge 9 ]; then
  # Backwards compatibility for the retired raw-frame positional slots:
  #   5 = raw frame index, 6 = raw basename.
  START_FRAME="${7:-0}"
  TIMELINE_SPEED="${8:-1.0}"
  LOWTIDE_PACK_BASENAME="${9}"
else
  START_FRAME="${5:-0}"
  TIMELINE_SPEED="${6:-1.0}"
  LOWTIDE_PACK_BASENAME="${7:-}"
fi

if [ -z "$OUTPUT_DIR" ]; then
  OUTPUT_DIR="$PROJECT_ROOT/host-results/${SCENE_SLUG}-foreground-pilot"
fi

HOST_CAPTURE_HIGH_DIR="$OUTPUT_DIR/host-capture-high"
HOST_CAPTURE_LOW_DIR="$OUTPUT_DIR/host-capture-low"
PACK_PATH="$PROJECT_ROOT/generated/ps1/foreground/${PACK_BASENAME}.FG2"
PACK_JSON="$OUTPUT_DIR/foreground-pack.json"
if [ -z "$LOWTIDE_PACK_BASENAME" ]; then
  LOWTIDE_PACK_BASENAME="${PACK_BASENAME}LOW"
  if [ "${#LOWTIDE_PACK_BASENAME}" -gt 8 ]; then
    LOWTIDE_PACK_BASENAME="${PACK_BASENAME:0:7}L"
  fi
fi
if [ "${#LOWTIDE_PACK_BASENAME}" -gt 8 ]; then
  echo "low-tide pack basename must fit 8.3 naming: $LOWTIDE_PACK_BASENAME" >&2
  exit 1
fi
LOWTIDE_PACK_PATH="$PROJECT_ROOT/generated/ps1/foreground/${LOWTIDE_PACK_BASENAME}.FG2"
LOWTIDE_PACK_JSON="$OUTPUT_DIR/foreground-pack-lowtide.json"
mkdir -p "$OUTPUT_DIR"
rm -rf "$HOST_CAPTURE_HIGH_DIR" "$HOST_CAPTURE_LOW_DIR"

"$SCRIPT_DIR/capture-host-scene.sh" \
  --scene "$SCENE_NAME" \
  --mode story-single \
  --seed 1 \
  --start-frame "$START_FRAME" \
  --interval 1 \
  --until-exit \
  --no-stamp \
  --lowtide 0 \
  --output "$HOST_CAPTURE_HIGH_DIR"

"$SCRIPT_DIR/capture-host-scene.sh" \
  --scene "$SCENE_NAME" \
  --mode story-single \
  --seed 1 \
  --start-frame "$START_FRAME" \
  --interval 1 \
  --until-exit \
  --no-stamp \
  --lowtide 1 \
  --output "$HOST_CAPTURE_LOW_DIR"

python3 "$SCRIPT_DIR/build-scene-foreground-pack.py" \
  --scene-label "$SCENE_NAME" \
  --frames-dir "$HOST_CAPTURE_HIGH_DIR/frames" \
  --frame-meta-dir "$HOST_CAPTURE_HIGH_DIR/frame-meta" \
  --sound-events "$HOST_CAPTURE_HIGH_DIR/sound-events.jsonl" \
  --timeline-speed "$TIMELINE_SPEED" \
  --pack-format fg2 \
  --base-diff \
  --scene-base-frame 0 \
  --output-pack "$PACK_PATH" \
  --output-json "$PACK_JSON"

python3 "$SCRIPT_DIR/build-scene-foreground-pack.py" \
  --scene-label "$SCENE_NAME lowtide" \
  --frames-dir "$HOST_CAPTURE_LOW_DIR/frames" \
  --frame-meta-dir "$HOST_CAPTURE_LOW_DIR/frame-meta" \
  --sound-events "$HOST_CAPTURE_LOW_DIR/sound-events.jsonl" \
  --timeline-speed "$TIMELINE_SPEED" \
  --pack-format fg2 \
  --base-diff \
  --scene-base-frame 0 \
  --output-pack "$LOWTIDE_PACK_PATH" \
  --output-json "$LOWTIDE_PACK_JSON"

if [ ! -s "$PACK_PATH" ]; then
  echo "foreground pack was not generated: $PACK_PATH" >&2
  exit 1
fi

if [ ! -s "$LOWTIDE_PACK_PATH" ]; then
  echo "low-tide foreground pack was not generated: $LOWTIDE_PACK_PATH" >&2
  exit 1
fi

echo "$PACK_JSON"
echo "$LOWTIDE_PACK_JSON"
echo "$PACK_PATH"
echo "$LOWTIDE_PACK_PATH"
