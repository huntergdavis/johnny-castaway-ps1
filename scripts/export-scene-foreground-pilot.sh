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
HOST_CAPTURE_HIGH_FGONLY_DIR="$OUTPUT_DIR/host-capture-high-fgonly"
HOST_CAPTURE_LOW_FGONLY_DIR="$OUTPUT_DIR/host-capture-low-fgonly"
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
CAPTURE_ISLAND_X="${FG_EXPORT_ISLAND_X:--154}"
CAPTURE_ISLAND_Y="${FG_EXPORT_ISLAND_Y:-54}"
CAPTURE_RAFT_STAGE="${FG_EXPORT_RAFT_STAGE:-4}"
KEYED_OVERLAY_RECT="${FG_EXPORT_KEYED_OVERLAY_RECT:-}"
KEYED_OVERLAY_INCLUDE_STATIC_BASE="${FG_EXPORT_KEYED_OVERLAY_INCLUDE_STATIC_BASE:-}"
KEYED_OVERLAY_SKIP_VISIBILITY_MASK="${FG_EXPORT_KEYED_OVERLAY_SKIP_VISIBILITY_MASK:-}"
if [ -z "$KEYED_OVERLAY_RECT" ] && [ "$SCENE_SLUG" = "johnny2" ]; then
  # JOHNNY 2's lower-left moving sprites can accumulate in full-frame
  # captures. Keep the thought-bubble lane above y=320 on full base-diff
  # pixels; foreground-only does not include those bubble pixels reliably.
  KEYED_OVERLAY_RECT="0,320,320,160"
fi
if [ -z "$KEYED_OVERLAY_RECT" ] && [ "$SCENE_SLUG" = "fishing5" ]; then
  # FISHING 5's shark/Johnny interaction contaminates the full host surface
  # with stale moving pixels. The current foreground ledger is the source of
  # truth for this scene, so replace base-diff pixels with keyed foreground-only
  # capture across the whole frame.
  KEYED_OVERLAY_RECT="0,0,640,480"
fi
if [ -z "$KEYED_OVERLAY_INCLUDE_STATIC_BASE" ] && [ "$SCENE_SLUG" = "fishing5" ]; then
  # The shark waterline also uses current BACKGRND.BMP ledger draws. The
  # default foreground-only capture excludes those for other scenes, but
  # FISHING 5 needs them to avoid outline-only shark frames.
  KEYED_OVERLAY_INCLUDE_STATIC_BASE="1"
fi
if [ -z "$KEYED_OVERLAY_SKIP_VISIBILITY_MASK" ] && [ "$SCENE_SLUG" = "fishing5" ]; then
  # FISHING 5's final full-host surface is contaminated, so exact final-frame
  # masking can drop current shark/water pixels. Replay the current ledger
  # directly for its keyed overlay captures.
  KEYED_OVERLAY_SKIP_VISIBILITY_MASK="1"
fi
HOLD_ADVANCE_WINDOW="${FG_EXPORT_HOLD_ADVANCE_WINDOW:-}"
if [ -z "$HOLD_ADVANCE_WINDOW" ] && [ "$SCENE_SLUG" = "johnny2" ]; then
  # The captured frame pixels are now correct, but the deduped hold rows lag
  # the visible island/SOS thought-bubble frames by a few source rows.
  HOLD_ADVANCE_WINDOW="101:120:6"
fi
HOLD_ADJUSTMENTS="${FG_EXPORT_HOLD_ADJUSTMENTS:-}"
if [ -z "$HOLD_ADJUSTMENTS" ] && [ "$SCENE_SLUG" = "johnny2" ]; then
  # Keep total duration fixed while making the repeated post-thought chain
  # read as a quick three/two/one exit, with saved time on island/SOS.
  HOLD_ADJUSTMENTS="101:+89 103:+112 104:+39 105:-3 106:-1 109:-1 113:-7 116:-7 120:-15 123:-23 127:-15 131:-23 135:-15 138:-23 142:-15 145:-23 149:-15 152:-15 156:-8 159:-16 163:-15"
fi
mkdir -p "$OUTPUT_DIR"
rm -rf "$HOST_CAPTURE_HIGH_DIR" "$HOST_CAPTURE_LOW_DIR" \
  "$HOST_CAPTURE_HIGH_FGONLY_DIR" "$HOST_CAPTURE_LOW_FGONLY_DIR"

"$SCRIPT_DIR/capture-host-scene.sh" \
  --scene "$SCENE_NAME" \
  --mode story-single \
  --seed 1 \
  --start-frame "$START_FRAME" \
  --interval 1 \
  --until-exit \
  --no-stamp \
  --lowtide 0 \
  --raft-stage "$CAPTURE_RAFT_STAGE" \
  --island-x "$CAPTURE_ISLAND_X" \
  --island-y "$CAPTURE_ISLAND_Y" \
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
  --raft-stage "$CAPTURE_RAFT_STAGE" \
  --island-x "$CAPTURE_ISLAND_X" \
  --island-y "$CAPTURE_ISLAND_Y" \
  --output "$HOST_CAPTURE_LOW_DIR"

high_overlay_args=()
low_overlay_args=()
foreground_capture_args=()
if [ "$KEYED_OVERLAY_INCLUDE_STATIC_BASE" = "1" ]; then
  foreground_capture_args=(--foreground-include-static-base)
fi
if [ "$KEYED_OVERLAY_SKIP_VISIBILITY_MASK" = "1" ]; then
  foreground_capture_args+=(--foreground-skip-visibility-mask)
fi
hold_advance_args=()
if [ -n "$HOLD_ADVANCE_WINDOW" ]; then
  hold_advance_args=(--hold-advance-window "$HOLD_ADVANCE_WINDOW")
fi
hold_adjust_args=()
if [ -n "$HOLD_ADJUSTMENTS" ]; then
  read -r -a hold_adjust_values <<< "$HOLD_ADJUSTMENTS"
  for hold_adjust_value in "${hold_adjust_values[@]}"; do
    hold_adjust_args+=(--hold-adjust "$hold_adjust_value")
  done
fi
if [ -n "$KEYED_OVERLAY_RECT" ]; then
  "$SCRIPT_DIR/capture-host-scene.sh" \
    --scene "$SCENE_NAME" \
    --mode story-single \
    --seed 1 \
    --start-frame "$START_FRAME" \
    --interval 1 \
    --until-exit \
    --no-stamp \
    --lowtide 0 \
    --raft-stage "$CAPTURE_RAFT_STAGE" \
    --island-x "$CAPTURE_ISLAND_X" \
    --island-y "$CAPTURE_ISLAND_Y" \
    --foreground-only \
    "${foreground_capture_args[@]}" \
    --output "$HOST_CAPTURE_HIGH_FGONLY_DIR"

  "$SCRIPT_DIR/capture-host-scene.sh" \
    --scene "$SCENE_NAME" \
    --mode story-single \
    --seed 1 \
    --start-frame "$START_FRAME" \
    --interval 1 \
    --until-exit \
    --no-stamp \
    --lowtide 1 \
    --raft-stage "$CAPTURE_RAFT_STAGE" \
    --island-x "$CAPTURE_ISLAND_X" \
    --island-y "$CAPTURE_ISLAND_Y" \
    --foreground-only \
    "${foreground_capture_args[@]}" \
    --output "$HOST_CAPTURE_LOW_FGONLY_DIR"

  high_overlay_args=(
    --keyed-overlay-frames-dir "$HOST_CAPTURE_HIGH_FGONLY_DIR/frames"
    --keyed-overlay-rect "$KEYED_OVERLAY_RECT"
  )
  low_overlay_args=(
    --keyed-overlay-frames-dir "$HOST_CAPTURE_LOW_FGONLY_DIR/frames"
    --keyed-overlay-rect "$KEYED_OVERLAY_RECT"
  )
fi

python3 "$SCRIPT_DIR/build-scene-foreground-pack.py" \
  --scene-label "$SCENE_NAME" \
  --frames-dir "$HOST_CAPTURE_HIGH_DIR/frames" \
  --frame-meta-dir "$HOST_CAPTURE_HIGH_DIR/frame-meta" \
  --sound-events "$HOST_CAPTURE_HIGH_DIR/sound-events.jsonl" \
  --timeline-speed "$TIMELINE_SPEED" \
  --pack-format fg2 \
  --base-diff \
  --scene-base-frame 0 \
  "${high_overlay_args[@]}" \
  "${hold_advance_args[@]}" \
  "${hold_adjust_args[@]}" \
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
  "${low_overlay_args[@]}" \
  "${hold_advance_args[@]}" \
  "${hold_adjust_args[@]}" \
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
