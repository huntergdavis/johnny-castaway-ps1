#!/bin/bash
# Batch-capture every scene in config/ps1/regtest-scenes.txt, producing
# high-tide and low-tide full-render base-diff FG2 packs plus an establishing
# .RAW in generated/ps1/foreground/. Skips scenes whose two FG2 packs already
# exist.
#
# Naming convention, matching fishing3:
#   SCENE_SLUG      = lowercase ADS + tag  (activity1, building7, mary3, ...)
#   SCENE_NAME      = "ADS N"              (pass-through to host binary)
#   PACK_BASENAME   = uppercase SCENE_SLUG (ACTIVITY1, BUILDING7, MARY3, ...)
#   RAW_BASENAME    = first 4 letters of ADS uppercase + tag
#                      (ACTV1, BUIL7, MARY3, JOHN1, VIST6, WALK1, ...)
#   LOW_PACK_NAME   = RAW_BASENAME + LOW, shortened to RAW_BASENAME + L
#                      if needed to keep the basename 8.3-safe
#   RAW_FRAME_INDEX = 0
#   START_FRAME     = 0
#   TIMELINE_SPEED  = 1.0
#
# Runs captures sequentially (the host binary is single-process).
# Each scene takes roughly 5-15 minutes of host capture time.

set -uo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
SCENE_LIST="$PROJECT_ROOT/config/ps1/regtest-scenes.txt"
GEN_DIR="$PROJECT_ROOT/generated/ps1/foreground"
LOG_DIR="$PROJECT_ROOT/scratch/batch-capture-logs"
STATUS_FILE="$PROJECT_ROOT/scratch/batch-capture-status.txt"

mkdir -p "$LOG_DIR"
: > "$STATUS_FILE"

"$SCRIPT_DIR/build-host.sh" >> "$STATUS_FILE" 2>&1

# ADS name → 4-letter abbreviation used for the establishing-frame RAW file.
ads_abbrev() {
  case "$1" in
    ACTIVITY) echo "ACTV" ;;
    BUILDING) echo "BUIL" ;;
    FISHING)  echo "FISH" ;;
    JOHNNY)   echo "JOHN" ;;
    MARY)     echo "MARY" ;;
    MISCGAG)  echo "MISC" ;;
    STAND)    echo "STND" ;;
    SUZY)     echo "SUZY" ;;
    VISITOR)  echo "VIST" ;;
    WALKSTUF) echo "WALK" ;;
    *)        echo "$1" | cut -c1-4 ;;
  esac
}

log_status() {
  printf '[%s] %s\n' "$(date -u +%H:%M:%S)" "$*" >> "$STATUS_FILE"
  printf '[%s] %s\n' "$(date -u +%H:%M:%S)" "$*"
}

total=0
captured=0
skipped=0
failed=0

# Parse the scene list, skip blank/comment lines.
while IFS= read -r raw_line; do
  line="${raw_line%%#*}"
  line="$(echo "$line" | awk '{$1=$1; print}')"
  [ -z "$line" ] && continue

  ads="$(echo "$line" | awk '{print $1}')"
  tag="$(echo "$line" | awk '{print $2}')"
  # scene_index = column 3, status = column 4 (unused here)

  [ -z "$ads" ] && continue
  [ -z "$tag" ] && continue

  total=$((total + 1))

  slug_lower="$(echo "${ads}${tag}" | tr '[:upper:]' '[:lower:]')"
  scene_name="${ads} ${tag}"
  pack_basename="$(echo "${ads}${tag}" | tr '[:lower:]' '[:upper:]')"
  raw_basename="$(ads_abbrev "$ads")${tag}"
  low_pack_basename="${raw_basename}LOW"
  if [ "${#low_pack_basename}" -gt 8 ]; then
    low_pack_basename="${raw_basename}L"
  fi

  pack_path="$GEN_DIR/${pack_basename}.FG2"
  low_pack_path="$GEN_DIR/${low_pack_basename}.FG2"

  if [ -s "$pack_path" ] && [ -s "$low_pack_path" ]; then
    log_status "SKIP  $slug_lower  (packs already exist: ${pack_path##*/}, ${low_pack_path##*/})"
    skipped=$((skipped + 1))
    continue
  fi

  log_status "START $slug_lower  name='$scene_name'  high=$pack_basename  low=$low_pack_basename  raw=${raw_basename}0"
  capture_log="$LOG_DIR/${slug_lower}.log"

  if "$SCRIPT_DIR/export-scene-foreground-pilot.sh" \
      "" "$slug_lower" "$scene_name" "$pack_basename" 0 "$raw_basename" 0 1.0 "$low_pack_basename" \
      > "$capture_log" 2>&1; then
    if [ -s "$pack_path" ] && [ -s "$low_pack_path" ]; then
      high_size="$(stat -c%s "$pack_path" 2>/dev/null || echo ?)"
      low_size="$(stat -c%s "$low_pack_path" 2>/dev/null || echo ?)"
      log_status "OK    $slug_lower  high=${pack_path##*/} ($high_size bytes) low=${low_pack_path##*/} ($low_size bytes)"
      captured=$((captured + 1))
    else
      log_status "FAIL  $slug_lower  export returned 0 but one or both packs are empty/missing (see $capture_log)"
      failed=$((failed + 1))
    fi
  else
    rc=$?
    log_status "FAIL  $slug_lower  export exited rc=$rc (see $capture_log)"
    failed=$((failed + 1))
  fi
done < "$SCENE_LIST"

log_status "DONE  total=$total captured=$captured skipped=$skipped failed=$failed"
