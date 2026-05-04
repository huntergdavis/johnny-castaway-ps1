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
HOST_CAPTURE_HIGH_STITCH_FGONLY_DIR="$OUTPUT_DIR/host-capture-high-stitch-fgonly"
HOST_CAPTURE_LOW_STITCH_FGONLY_DIR="$OUTPUT_DIR/host-capture-low-stitch-fgonly"
HOST_CAPTURE_HIGH_FAR_STITCH_FGONLY_DIR="$OUTPUT_DIR/host-capture-high-far-stitch-fgonly"
HOST_CAPTURE_LOW_FAR_STITCH_FGONLY_DIR="$OUTPUT_DIR/host-capture-low-far-stitch-fgonly"
HOST_CAPTURE_HIGH_MERGED_FGONLY_DIR="$OUTPUT_DIR/host-capture-high-merged-fgonly"
HOST_CAPTURE_LOW_MERGED_FGONLY_DIR="$OUTPUT_DIR/host-capture-low-merged-fgonly"
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
CAPTURE_ISLAND_X="${FG_EXPORT_ISLAND_X:-}"
CAPTURE_ISLAND_Y="${FG_EXPORT_ISLAND_Y:-54}"
if [ -z "$CAPTURE_ISLAND_X" ]; then
  if [ "$SCENE_SLUG" = "johnny5" ]; then
    # JOHNNY 5 throws the bottle far enough left that the splash is clipped in
    # the old x=-64 capture. Capture at the validated current-position X so the
    # splash is present in the source pack; do not add a production runtime pin.
    CAPTURE_ISLAND_X="80"
  elif [ "$SCENE_SLUG" = "johnny4" ]; then
    # JOHNNY 4 is a bottle/letter-message scene. Capture/test it at the same
    # right-shifted host position used by JOHNNY 2 so the message pixels are
    # not clipped; do not add a production runtime pin.
    CAPTURE_ISLAND_X="-64"
  elif [ "$SCENE_SLUG" = "fishing7" ] || [ "$SCENE_SLUG" = "fishing8" ]; then
    # FISHING 7/8 reach far to the right of Johnny. Capture them with the
    # island shifted left so all scene-relative pixels are inside the host
    # viewport; runtime playback can then follow normal island placement.
    CAPTURE_ISLAND_X="-300"
  elif [ "$SCENE_SLUG" = "mary2" ]; then
    # MARY 2 is assembled from multiple scene-relative foreground-only views
    # below. Keep the canonical full-host capture at the historical position;
    # extra foreground views provide clipped left/right action pixels without a
    # production runtime pin.
    CAPTURE_ISLAND_X="-154"
  elif [ "$SCENE_SLUG" = "mary3" ]; then
    # MARY 3's action lives left of the island. Capture with the island shifted
    # right so left-side Johnny/Mary pixels are not clipped; this is capture
    # policy, not a production runtime pin.
    CAPTURE_ISLAND_X="80"
  else
    CAPTURE_ISLAND_X="-154"
  fi
fi
CAPTURE_RAFT_STAGE="${FG_EXPORT_RAFT_STAGE:-4}"
if [ -z "${FG_EXPORT_RAFT_STAGE:-}" ] && [ "$SCENE_SLUG" = "mary2" ]; then
  # MARY 2 is a raft-stage-5 fishing scene in validation. Capture against the
  # same backdrop state so foreground pixels that cross the raft are complete.
  CAPTURE_RAFT_STAGE="5"
fi
KEYED_OVERLAY_RECT="${FG_EXPORT_KEYED_OVERLAY_RECT:-}"
KEYED_OVERLAY_INCLUDE_STATIC_BASE="${FG_EXPORT_KEYED_OVERLAY_INCLUDE_STATIC_BASE:-}"
KEYED_OVERLAY_SKIP_VISIBILITY_MASK="${FG_EXPORT_KEYED_OVERLAY_SKIP_VISIBILITY_MASK:-}"
if [ -z "$KEYED_OVERLAY_RECT" ] && [ "$SCENE_SLUG" = "johnny2" ]; then
  # JOHNNY 2's lower-left moving sprites can accumulate in full-frame
  # captures. Keep the thought-bubble lane above y=320 on full base-diff
  # pixels; foreground-only does not include those bubble pixels reliably.
  KEYED_OVERLAY_RECT="0,320,320,160"
fi
if [ -z "$KEYED_OVERLAY_RECT" ] && [ "$SCENE_SLUG" = "mary2" ]; then
  # MARY 2's full-host surface accumulates stale lower-water sprites, and its
  # opening cast uses thin line pixels that base-diff can under-carry against
  # the ocean. The Mary2 branch below uses this as the trigger for a multi-view
  # foreground-only scene-relative merge, then clears the overlay args before
  # pack build because the merged foreground canvas is already the pack source.
  KEYED_OVERLAY_RECT="0,235,640,245"
fi
if [ -z "$KEYED_OVERLAY_RECT" ] &&
   { [ "$SCENE_SLUG" = "johnny4" ] || [ "$SCENE_SLUG" = "johnny5" ]; }; then
  # These letter-message full host surfaces can contaminate bubbles/letters
  # and lower-left Johnny pixels with stale moving foreground. Unlike JOHNNY 2,
  # their foreground-only captures carry the message lane cleanly, so use them
  # frame-wide.
  KEYED_OVERLAY_RECT="0,0,640,480"
fi
if [ -z "$KEYED_OVERLAY_RECT" ] && [ "$SCENE_SLUG" = "fishing5" ]; then
  # FISHING 5's shark/Johnny interaction contaminates the full host surface
  # with stale moving pixels. The current foreground ledger is the source of
  # truth for this scene, so replace base-diff pixels with keyed foreground-only
  # capture across the whole frame.
  KEYED_OVERLAY_RECT="0,0,640,480"
fi
if [ -z "$KEYED_OVERLAY_RECT" ] && [ "$SCENE_SLUG" = "mary3" ]; then
  # MARY 3's old full-host base-diff pack captured a wide island/ocean plate
  # and double-painted it over the PS1 runtime island. Foreground-only replay
  # keeps the live Johnny/Mary action without baking the backdrop into FG2.
  KEYED_OVERLAY_RECT="0,0,640,480"
fi
if [ -z "$KEYED_OVERLAY_RECT" ] &&
   { [ "$SCENE_SLUG" = "fishing7" ] || [ "$SCENE_SLUG" = "fishing8" ]; }; then
  # The older full-host captures for these scenes were both full-width dirty
  # and right-edge clipped. Foreground-only captures contain the moving action
  # without the stale island/ocean pixels, so use them frame-wide.
  KEYED_OVERLAY_RECT="0,0,640,480"
fi
MULTIVIEW_STITCH="${FG_EXPORT_MULTIVIEW_STITCH:-}"
if [ -z "$MULTIVIEW_STITCH" ]; then
  if [ "$SCENE_SLUG" = "johnny2" ] || [ "$SCENE_SLUG" = "mary2" ]; then
    # These scenes have validated custom capture paths: JOHNNY2 keeps a
    # partial lower-band overlay so thought bubbles stay full-host, and MARY2
    # has a bespoke multi-view + bubble-shell injection branch below.
    MULTIVIEW_STITCH="0"
  else
    # New scene validation should start with a normal/reference capture plus
    # far-left and far-right foreground-only views. Island-relative content can
    # span more than one screen width, so single-position packs are now treated
    # as an opt-out diagnostic path rather than the default.
    MULTIVIEW_STITCH="1"
  fi
fi
if [ "$MULTIVIEW_STITCH" = "1" ]; then
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
if [ -z "$HOLD_ADJUSTMENTS" ] && [ "$SCENE_SLUG" = "johnny5" ]; then
  # The SOS thought bubble is source frame 74. The following blank rows were
  # holding too long, so preserve total duration while pausing on the note.
  HOLD_ADJUSTMENTS="74:+16 75:-12 78:-4"
fi
if [ -z "$HOLD_ADJUSTMENTS" ] && [ "$SCENE_SLUG" = "mary3" ]; then
  # The dinner thought beat only survives for source frames 347-348. Move time
  # from the following recovery rows so the gag is readable without length drift.
  HOLD_ADJUSTMENTS="347:+24 348:+28 349:-1 351:-4 353:-2 354:-2 355:-5 358:-3 359:-1 360:-5 362:-1 363:-4 365:-2 366:-2 367:-5 369:-2 370:-3 371:-5 373:-1 374:-4"
fi
mkdir -p "$OUTPUT_DIR"
rm -rf "$HOST_CAPTURE_HIGH_DIR" "$HOST_CAPTURE_LOW_DIR" \
  "$HOST_CAPTURE_HIGH_FGONLY_DIR" "$HOST_CAPTURE_LOW_FGONLY_DIR" \
  "$HOST_CAPTURE_HIGH_STITCH_FGONLY_DIR" "$HOST_CAPTURE_LOW_STITCH_FGONLY_DIR" \
  "$HOST_CAPTURE_HIGH_FAR_STITCH_FGONLY_DIR" "$HOST_CAPTURE_LOW_FAR_STITCH_FGONLY_DIR" \
  "$HOST_CAPTURE_HIGH_MERGED_FGONLY_DIR" "$HOST_CAPTURE_LOW_MERGED_FGONLY_DIR"

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
scene_base_args=(--scene-base-frame 0)
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

  if [ "$SCENE_SLUG" = "mary2" ]; then
    MARY2_STITCH_ISLAND_X="${FG_EXPORT_MARY2_STITCH_ISLAND_X:-80}"
    MARY2_FAR_STITCH_ISLAND_X="${FG_EXPORT_MARY2_FAR_STITCH_ISLAND_X:-300}"
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
      --island-x "$MARY2_STITCH_ISLAND_X" \
      --island-y "$CAPTURE_ISLAND_Y" \
      --foreground-only \
      "${foreground_capture_args[@]}" \
      --output "$HOST_CAPTURE_HIGH_STITCH_FGONLY_DIR"

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
      --island-x "$MARY2_STITCH_ISLAND_X" \
      --island-y "$CAPTURE_ISLAND_Y" \
      --foreground-only \
      "${foreground_capture_args[@]}" \
      --output "$HOST_CAPTURE_LOW_STITCH_FGONLY_DIR"

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
      --island-x "$MARY2_FAR_STITCH_ISLAND_X" \
      --island-y "$CAPTURE_ISLAND_Y" \
      --foreground-only \
      "${foreground_capture_args[@]}" \
      --output "$HOST_CAPTURE_HIGH_FAR_STITCH_FGONLY_DIR"

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
      --island-x "$MARY2_FAR_STITCH_ISLAND_X" \
      --island-y "$CAPTURE_ISLAND_Y" \
      --foreground-only \
      "${foreground_capture_args[@]}" \
      --output "$HOST_CAPTURE_LOW_FAR_STITCH_FGONLY_DIR"

    python3 - "$HOST_CAPTURE_HIGH_DIR" "$HOST_CAPTURE_HIGH_FGONLY_DIR" "$HOST_CAPTURE_HIGH_STITCH_FGONLY_DIR" "$HOST_CAPTURE_HIGH_FAR_STITCH_FGONLY_DIR" "$HOST_CAPTURE_HIGH_MERGED_FGONLY_DIR" \
              "$HOST_CAPTURE_LOW_DIR" "$HOST_CAPTURE_LOW_FGONLY_DIR" "$HOST_CAPTURE_LOW_STITCH_FGONLY_DIR" "$HOST_CAPTURE_LOW_FAR_STITCH_FGONLY_DIR" "$HOST_CAPTURE_LOW_MERGED_FGONLY_DIR" <<'PY'
import sys
import json
from pathlib import Path
from PIL import Image, ImageChops

NEUTRAL_LINE_COLORS = {
    (128, 128, 128),
    (168, 168, 168),
    (212, 212, 212),
    (252, 252, 252),
    (255, 255, 255),
}
KEY = (255, 0, 255)
MARY2_BUBBLE_LOCAL_RECT = (440, 118, 540, 220)
MARY2_BUBBLE_MIN_PIXELS = 1000


def frame_number(path):
    return int(path.stem.split("_", 1)[1])


def load_offset(capture_dir, frame_name):
    meta_path = Path(capture_dir) / "frame-meta" / Path(frame_name).with_suffix(".json").name
    if not meta_path.exists():
        raise SystemExit(f"missing Mary2 stitch metadata: {meta_path}")
    payload = json.loads(meta_path.read_text(encoding="utf-8"))
    return (
        int(payload.get("scene_offset_x", 0) or 0),
        int(payload.get("scene_offset_y", 0) or 0),
    )


def is_colored_sprite_pixel(rgb):
    r, g, b = rgb
    if rgb in (KEY, (0, 0, 0), (0, 0, 168), (0, 0, 170), (0, 0, 252), (0, 0, 255)):
        return False
    if rgb in NEUTRAL_LINE_COLORS:
        return False
    return max(r, g, b) - min(r, g, b) > 24


def isolated_from_sprite(full, x, y):
    width, height = full.size
    colored = 0
    for ny in range(max(0, y - 2), min(height, y + 3)):
        for nx in range(max(0, x - 2), min(width, x + 3)):
            if is_colored_sprite_pixel(full.getpixel((nx, ny))):
                colored += 1
                if colored > 2:
                    return False
    return True


def patch_pair(full_dir, fg_dir):
    for fg_path in sorted(Path(fg_dir).glob("frame_*.bmp")):
        if frame_number(fg_path) > 149:
            continue
        full_path = Path(full_dir) / fg_path.name
        if not full_path.exists():
            raise SystemExit(f"missing full host frame for Mary2 overlay patch: {full_path}")
        with Image.open(full_path) as full_raw, Image.open(fg_path) as fg_raw:
            full = full_raw.convert("RGB")
            fg = fg_raw.convert("RGB")
            changed = False
            # MARY2's lower-band line loss is confined to the upper footer.
            for y in range(350, 380):
                for x in range(0, 640):
                    if fg.getpixel((x, y)) != KEY:
                        continue
                    rgb = full.getpixel((x, y))
                    if rgb not in NEUTRAL_LINE_COLORS:
                        continue
                    if not isolated_from_sprite(full, x, y):
                        continue
                    fg.putpixel((x, y), rgb)
                    changed = True
            if changed:
                fg.save(fg_path)


def frame_paths(capture_dir):
    return sorted((Path(capture_dir) / "frames").glob("frame_*.bmp"))


def non_key_pixels(img):
    key_img = Image.new("RGB", img.size, KEY)
    bbox = ImageChops.difference(img, key_img).getbbox()
    if bbox is None:
        return
    pix = img.load()
    left, top, right, bottom = bbox
    for y in range(top, bottom):
        for x in range(left, right):
            rgb = pix[x, y]
            if rgb != KEY:
                yield x, y, rgb


def is_mary2_bubble_pixel(rgb):
    r, g, b = rgb
    if rgb in (KEY, (0, 0, 0)):
        return False
    if r > 190 and g > 190 and b > 190:
        return True
    if max(r, g, b) - min(r, g, b) <= 35:
        return False
    if b > 150 and r < 80 and g < 120:
        return False
    if g > 180 and b > 180 and r < 80:
        return False
    return True


def mary2_full_host_bubble_pixels(reference_capture_dir, frame_name, base_full):
    full_path = Path(reference_capture_dir) / "frames" / frame_name
    if not full_path.exists() or base_full is None:
        return {}

    offset_x, offset_y = load_offset(reference_capture_dir, frame_name)
    with Image.open(full_path) as raw:
        full = raw.convert("RGB")

    full_pixels = full.load()
    base_pixels = base_full.load()
    left, top, right, bottom = MARY2_BUBBLE_LOCAL_RECT
    candidates = {}

    for local_y in range(top, bottom):
        screen_y = local_y + offset_y
        if screen_y < 0 or screen_y >= full.height:
            continue
        for local_x in range(left, right):
            screen_x = local_x + offset_x
            if screen_x < 0 or screen_x >= full.width:
                continue
            rgb = full_pixels[screen_x, screen_y]
            if rgb == base_pixels[screen_x, screen_y]:
                continue
            if not is_mary2_bubble_pixel(rgb):
                continue
            candidates[(local_x, local_y)] = rgb

    return candidates if len(candidates) >= MARY2_BUBBLE_MIN_PIXELS else {}


def write_merged_foreground(reference_capture_dir, source_fg_dirs, out_dir):
    out = Path(out_dir)
    out_frames = out / "frames"
    out_meta = out / "frame-meta"
    out_frames.mkdir(parents=True, exist_ok=True)
    out_meta.mkdir(parents=True, exist_ok=True)

    frames = []
    base_full_path = Path(reference_capture_dir) / "frames" / "frame_00000.bmp"
    base_full = Image.open(base_full_path).convert("RGB") if base_full_path.exists() else None
    global_min_x = None
    global_min_y = None
    global_max_x = None
    global_max_y = None

    for ref_frame in frame_paths(reference_capture_dir):
        local_pixels = {}
        ref_meta_path = Path(reference_capture_dir) / "frame-meta" / ref_frame.with_suffix(".json").name
        if not ref_meta_path.exists():
            raise SystemExit(f"missing Mary2 reference metadata: {ref_meta_path}")
        ref_meta = json.loads(ref_meta_path.read_text(encoding="utf-8"))

        for source_dir in source_fg_dirs:
            source_frame = Path(source_dir) / "frames" / ref_frame.name
            if not source_frame.exists():
                continue
            offset_x, offset_y = load_offset(source_dir, ref_frame.name)
            with Image.open(source_frame) as raw:
                img = raw.convert("RGB")
            for x, y, rgb in non_key_pixels(img):
                local_key = (x - offset_x, y - offset_y)
                local_pixels.setdefault(local_key, rgb)

        local_pixels.update(
            mary2_full_host_bubble_pixels(reference_capture_dir, ref_frame.name, base_full)
        )

        if local_pixels:
            frame_min_x = min(x for x, _ in local_pixels)
            frame_max_x = max(x for x, _ in local_pixels)
            frame_min_y = min(y for _, y in local_pixels)
            frame_max_y = max(y for _, y in local_pixels)
            global_min_x = frame_min_x if global_min_x is None else min(global_min_x, frame_min_x)
            global_min_y = frame_min_y if global_min_y is None else min(global_min_y, frame_min_y)
            global_max_x = frame_max_x if global_max_x is None else max(global_max_x, frame_max_x)
            global_max_y = frame_max_y if global_max_y is None else max(global_max_y, frame_max_y)

        frames.append((ref_frame, ref_meta, local_pixels))

    if global_min_x is None:
        global_min_x = 0
        global_min_y = 0
        global_max_x = 0
        global_max_y = 0

    canvas_w = global_max_x - global_min_x + 1
    canvas_h = global_max_y - global_min_y + 1
    if canvas_w <= 0 or canvas_h <= 0:
        raise SystemExit("Mary2 merged foreground produced an invalid canvas")
    if canvas_h > 480:
        raise SystemExit(
            f"Mary2 merged foreground canvas is too tall for the runtime: {canvas_w}x{canvas_h}"
        )

    synth_offset_x = -global_min_x
    synth_offset_y = -global_min_y

    for ref_frame, ref_meta, local_pixels in frames:
        merged = Image.new("RGB", (canvas_w, canvas_h), KEY)
        if local_pixels:
            pix = merged.load()
            for (local_x, local_y), rgb in local_pixels.items():
                sx = local_x + synth_offset_x
                sy = local_y + synth_offset_y
                if 0 <= sx < canvas_w and 0 <= sy < canvas_h:
                    pix[sx, sy] = rgb

        out_path = out_frames / ref_frame.name
        merged.save(out_path)

        ref_meta["image_path"] = str(out_path)
        ref_meta["scene_offset_x"] = synth_offset_x
        ref_meta["scene_offset_y"] = synth_offset_y
        (out_meta / ref_frame.with_suffix(".json").name).write_text(
            json.dumps(ref_meta, indent=2) + "\n",
            encoding="utf-8",
        )

    if base_full is not None:
        base_full.close()


patch_pair(Path(sys.argv[1]) / "frames", Path(sys.argv[2]) / "frames")
patch_pair(Path(sys.argv[6]) / "frames", Path(sys.argv[7]) / "frames")
write_merged_foreground(sys.argv[1], [sys.argv[2], sys.argv[3], sys.argv[4]], sys.argv[5])
write_merged_foreground(sys.argv[6], [sys.argv[7], sys.argv[8], sys.argv[9]], sys.argv[10])
PY

    if [ -f "$HOST_CAPTURE_HIGH_DIR/sound-events.jsonl" ]; then
      cp "$HOST_CAPTURE_HIGH_DIR/sound-events.jsonl" "$HOST_CAPTURE_HIGH_MERGED_FGONLY_DIR/sound-events.jsonl"
    fi
    if [ -f "$HOST_CAPTURE_LOW_DIR/sound-events.jsonl" ]; then
      cp "$HOST_CAPTURE_LOW_DIR/sound-events.jsonl" "$HOST_CAPTURE_LOW_MERGED_FGONLY_DIR/sound-events.jsonl"
    fi

    high_overlay_args=()
    low_overlay_args=()
    HOST_CAPTURE_HIGH_DIR="$HOST_CAPTURE_HIGH_MERGED_FGONLY_DIR"
    HOST_CAPTURE_LOW_DIR="$HOST_CAPTURE_LOW_MERGED_FGONLY_DIR"
    KEYED_OVERLAY_RECT=""
    scene_base_args=(--scene-base-color ff00ff)
  fi

  if [ "$SCENE_SLUG" != "mary2" ] && [ "$MULTIVIEW_STITCH" = "1" ]; then
    STITCH_LEFT_ISLAND_X="${FG_EXPORT_STITCH_LEFT_ISLAND_X:--300}"
    STITCH_RIGHT_ISLAND_X="${FG_EXPORT_STITCH_RIGHT_ISLAND_X:-300}"

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
      --island-x "$STITCH_LEFT_ISLAND_X" \
      --island-y "$CAPTURE_ISLAND_Y" \
      --foreground-only \
      "${foreground_capture_args[@]}" \
      --output "$HOST_CAPTURE_HIGH_STITCH_FGONLY_DIR"

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
      --island-x "$STITCH_LEFT_ISLAND_X" \
      --island-y "$CAPTURE_ISLAND_Y" \
      --foreground-only \
      "${foreground_capture_args[@]}" \
      --output "$HOST_CAPTURE_LOW_STITCH_FGONLY_DIR"

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
      --island-x "$STITCH_RIGHT_ISLAND_X" \
      --island-y "$CAPTURE_ISLAND_Y" \
      --foreground-only \
      "${foreground_capture_args[@]}" \
      --output "$HOST_CAPTURE_HIGH_FAR_STITCH_FGONLY_DIR"

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
      --island-x "$STITCH_RIGHT_ISLAND_X" \
      --island-y "$CAPTURE_ISLAND_Y" \
      --foreground-only \
      "${foreground_capture_args[@]}" \
      --output "$HOST_CAPTURE_LOW_FAR_STITCH_FGONLY_DIR"

    python3 "$SCRIPT_DIR/merge-scene-foreground-views.py" \
      --reference-capture "$HOST_CAPTURE_HIGH_DIR" \
      --source-fg-dir "$HOST_CAPTURE_HIGH_FGONLY_DIR" \
      --source-fg-dir "$HOST_CAPTURE_HIGH_STITCH_FGONLY_DIR" \
      --source-fg-dir "$HOST_CAPTURE_HIGH_FAR_STITCH_FGONLY_DIR" \
      --output "$HOST_CAPTURE_HIGH_MERGED_FGONLY_DIR"

    python3 "$SCRIPT_DIR/merge-scene-foreground-views.py" \
      --reference-capture "$HOST_CAPTURE_LOW_DIR" \
      --source-fg-dir "$HOST_CAPTURE_LOW_FGONLY_DIR" \
      --source-fg-dir "$HOST_CAPTURE_LOW_STITCH_FGONLY_DIR" \
      --source-fg-dir "$HOST_CAPTURE_LOW_FAR_STITCH_FGONLY_DIR" \
      --output "$HOST_CAPTURE_LOW_MERGED_FGONLY_DIR"

    high_overlay_args=()
    low_overlay_args=()
    HOST_CAPTURE_HIGH_DIR="$HOST_CAPTURE_HIGH_MERGED_FGONLY_DIR"
    HOST_CAPTURE_LOW_DIR="$HOST_CAPTURE_LOW_MERGED_FGONLY_DIR"
    KEYED_OVERLAY_RECT=""
    scene_base_args=(--scene-base-color ff00ff)
  elif [ "$SCENE_SLUG" != "mary2" ]; then
    high_overlay_args=(
      --keyed-overlay-frames-dir "$HOST_CAPTURE_HIGH_FGONLY_DIR/frames"
      --keyed-overlay-rect "$KEYED_OVERLAY_RECT"
    )
    low_overlay_args=(
      --keyed-overlay-frames-dir "$HOST_CAPTURE_LOW_FGONLY_DIR/frames"
      --keyed-overlay-rect "$KEYED_OVERLAY_RECT"
    )
  fi
fi

python3 "$SCRIPT_DIR/build-scene-foreground-pack.py" \
  --scene-label "$SCENE_NAME" \
  --frames-dir "$HOST_CAPTURE_HIGH_DIR/frames" \
  --frame-meta-dir "$HOST_CAPTURE_HIGH_DIR/frame-meta" \
  --sound-events "$HOST_CAPTURE_HIGH_DIR/sound-events.jsonl" \
  --timeline-speed "$TIMELINE_SPEED" \
  --pack-format fg2 \
  --base-diff \
  "${scene_base_args[@]}" \
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
  "${scene_base_args[@]}" \
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
