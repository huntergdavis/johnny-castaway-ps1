#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
OUTPUT_DIR="${1:-}"
SCENE_SLUG="${2:-fishing1}"
SCENE_NAME="${3:-FISHING 1}"
PACK_BASENAME="${4:-$(printf '%s' "$SCENE_SLUG" | tr '[:lower:]' '[:upper:]' | tr -cd 'A-Z0-9')}"
RAW_FRAME_INDEX="${5:-24}"
RAW_BASENAME="${6:-FISH}"
START_FRAME="${7:-0}"
TIMELINE_SPEED="${8:-1.0}"

if [ -z "$OUTPUT_DIR" ]; then
  OUTPUT_DIR="$PROJECT_ROOT/host-results/${SCENE_SLUG}-foreground-pilot"
fi

HOST_CAPTURE_DIR="$OUTPUT_DIR/host-capture"
HOST_CAPTURE_HIGH_DIR="$OUTPUT_DIR/host-capture-high"
HOST_CAPTURE_LOW_DIR="$OUTPUT_DIR/host-capture-low"
PACK_PATH="$PROJECT_ROOT/generated/ps1/foreground/${PACK_BASENAME}.FG2"
PACK_JSON="$OUTPUT_DIR/foreground-pack.json"
LOWTIDE_PACK_BASENAME="${9:-${RAW_BASENAME}LOW}"
if [ "${#LOWTIDE_PACK_BASENAME}" -gt 8 ]; then
  LOWTIDE_PACK_BASENAME="${RAW_BASENAME}L"
fi
LOWTIDE_PACK_PATH="$PROJECT_ROOT/generated/ps1/foreground/${LOWTIDE_PACK_BASENAME}.FG2"
LOWTIDE_PACK_JSON="$OUTPUT_DIR/foreground-pack-lowtide.json"
RAW_FRAME_NAME="$(printf 'frame_%05d.bmp' "$RAW_FRAME_INDEX")"
RAW_FRAME_SOURCE="$HOST_CAPTURE_HIGH_DIR/frames/$RAW_FRAME_NAME"
RAW_FRAME_PATH="$PROJECT_ROOT/generated/ps1/foreground/${RAW_BASENAME}${RAW_FRAME_INDEX}.RAW"
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

python3 "$SCRIPT_DIR/build-ps1-rawframe.py" \
  --input "$RAW_FRAME_SOURCE" \
  --output "$RAW_FRAME_PATH"

if [ ! -s "$PACK_PATH" ]; then
  echo "foreground pack was not generated: $PACK_PATH" >&2
  exit 1
fi

if [ ! -s "$LOWTIDE_PACK_PATH" ]; then
  echo "low-tide foreground pack was not generated: $LOWTIDE_PACK_PATH" >&2
  exit 1
fi

if [ ! -s "$RAW_FRAME_PATH" ]; then
  echo "foreground raw frame was not generated: $RAW_FRAME_PATH" >&2
  exit 1
fi

echo "$PACK_JSON"
echo "$LOWTIDE_PACK_JSON"
echo "$PACK_PATH"
echo "$LOWTIDE_PACK_PATH"
echo "$RAW_FRAME_PATH"
