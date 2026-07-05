#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

NO_STITCH_REQUESTED=0
FORCE_STITCH_REQUESTED=0
POSITIONAL_ARGS=()
while [ "$#" -gt 0 ]; do
  case "$1" in
    --no-stitch|-nostitch|nostitch)
      NO_STITCH_REQUESTED=1
      shift
      ;;
    --stitch|stitch)
      FORCE_STITCH_REQUESTED=1
      shift
      ;;
    -h|--help)
      cat <<'EOF'
Usage: export-scene-foreground-pilot.sh [--no-stitch|--stitch] [output-dir] scene-slug scene-name pack-basename [start-frame] [timeline-speed] [lowtide-pack-basename]

  --no-stitch, -nostitch, nostitch
      Skip far-left/far-right foreground-only stitching and build from the
      single capture position. Useful for simple STAND scenes.

  --stitch, stitch
      Force generic multi-view stitching even for scenes that default to the
      fast no-stitch path.
EOF
      exit 0
      ;;
    --)
      shift
      while [ "$#" -gt 0 ]; do
        POSITIONAL_ARGS+=("$1")
        shift
      done
      ;;
    -*)
      echo "unknown option: $1" >&2
      exit 1
      ;;
    *)
      POSITIONAL_ARGS+=("$1")
      shift
      ;;
  esac
done
set -- "${POSITIONAL_ARGS[@]}"
if [ "$NO_STITCH_REQUESTED" = "1" ] && [ "$FORCE_STITCH_REQUESTED" = "1" ]; then
  echo "--no-stitch and --stitch are mutually exclusive" >&2
  exit 1
fi

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

# Static-idle scenes whose host capture draws NO foreground for the first few
# frames (scene-setup delay): the island briefly shows without Johnny, then he
# pops in (the audit's opening empty-stage / base-flash class). Skip the blank
# lead-in so the pack opens already composed. Counts measured from the host
# fgonly captures (frame N is the first with Johnny). Applied only when no
# explicit start-frame was passed (START_FRAME==0, the batch default); an
# explicit positional arg still wins. NOTE: do NOT add boat/ski edge-entry
# scenes here — their empty opening is the intended off-frame entrance.
#
# stand10 reconciliation (release-0.9.7 red-team item): stand10 ships from
# commit 30ae0e81bf (the visual-audit first-frame fix), which used exactly this
# start-frame=3 — the SAME mechanism as its stand siblings, NOT a separate one.
# So this case entry is consistent with the shipped STAND10/STND10L packs
# (FGP2, start-frame=3, Johnny present in frame 0) and a re-export through this
# path reproduces them. CAVEAT: STND10L once carried a manual compact-FGP3 perf
# pass (perf plan P5-219, 49022 B); the 30ae0e81bf re-export reverted it to FGP2
# (60288 B), so re-exporting here does NOT restore that perf optimization —
# re-apply the FGP3 compaction separately if STND10L perf matters.
if [ "$START_FRAME" = "0" ]; then
  case "$SCENE_SLUG" in
    stand1)                                START_FRAME=1 ;;
    stand15)                               START_FRAME=2 ;;
    stand3|stand4|stand5|stand8|stand9|stand10) START_FRAME=3 ;;
  esac
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
if [ -z "${FG_EXPORT_ANCHOR_Y_DELTA:-}" ] && [ "$SCENE_SLUG" = "mary5" ]; then
  # The goodbye group (shark/mermaid/Johnny/raft) anchored ~12px too high
  # on the island backdrop — shark torso visibly ON the sand. The host
  # original places the shark's waterline BELOW Johnny's feet at the lower
  # shore. Uniform 12px drop restores that relative look.
  FG_EXPORT_ANCHOR_Y_DELTA=12
fi
if [ -z "${FG_EXPORT_ANCHOR_Y_DELTA_LOW:-}" ] && [ "$SCENE_SLUG" = "mary5" ]; then
  # The low-tide island sits ~12px lower than high tide and the packs are
  # otherwise tide-identical, so one uniform anchor cannot serve both
  # backdrops (console report: farewell group floating over the sand at
  # low tide with the verified high-tide 12). Verified at the goodbye
  # beat: 24 puts the group at the low-tide waterline.
  FG_EXPORT_ANCHOR_Y_DELTA_LOW=24
fi
export FG_EXPORT_ANCHOR_Y_DELTA
export FG_EXPORT_ANCHOR_Y_DELTA_LOW
if [ -z "${FG_EXPORT_RAFT_STAGE:-}" ] && [ "$SCENE_SLUG" = "mary5" ]; then
  # MARY 5 is flagged NORAFT in story_data.h and carries its own raft art.
  # Capture with the generic raft off so the pack/backdrop match runtime.
  CAPTURE_RAFT_STAGE="0"
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
if [ -z "$KEYED_OVERLAY_RECT" ] && [ "$SCENE_SLUG" = "activity5" ]; then
  # Same split as JOHNNY 2: keep the upper thought-bubble lane on full
  # base-diff so the storm-cloud bubble + connector dots survive, and apply
  # keyed overlay only to the lower third so the post-dive splash/ocean
  # band cleans up without leaving ghost trails.
  KEYED_OVERLAY_RECT="0,320,640,160"
fi
if [ -z "$KEYED_OVERLAY_RECT" ] && [ "$SCENE_SLUG" = "mary2" ]; then
  # MARY 2's full-host surface accumulates stale lower-water sprites, and its
  # opening cast uses thin line pixels that base-diff can under-carry against
  # the ocean. The Mary2 branch below uses this as the trigger for a multi-view
  # foreground-only scene-relative merge, then clears the overlay args before
  # pack build because the merged foreground canvas is already the pack source.
  KEYED_OVERLAY_RECT="0,235,640,245"
fi
if [ -z "$KEYED_OVERLAY_RECT" ] && [ "$SCENE_SLUG" = "johnny3" ]; then
  # JOHNNY 3 thought-bubble (clock/embrace daydream). The bubble shell +
  # contents survive frame-wide keyed, but the thin connector-dot trail linking
  # Johnny to the bubble fails the keyed visibility mask's byte-exact match and
  # drops out. Mirror johnny2/activity5: keep the lower actor band (y>=320) on
  # keyed foreground-only and let the upper band (bubble + dots + Johnny's
  # near-static head) ride base-diff. Measured: bubble+dots above y~310, moving
  # body below y320. johnny3's upper band is mostly static sky/ocean so the
  # base-diff cost is small (~303 KB pack).
  #
  # NOTE: suzy1 was tried here too but REVERTED — its big island-cutaway thought
  # bubble fills the upper band with high-entropy changing pixels, so base-diff
  # ballooned the pack 830 KB -> 14 MB (would not fit PS1 RAM). suzy1's dots are
  # left dropped (cosmetic); its bubble shell ships fine via keyed.
  KEYED_OVERLAY_RECT="0,320,640,160"
fi
if [ -z "$KEYED_OVERLAY_RECT" ] &&
   { [ "$SCENE_SLUG" = "johnny4" ] || [ "$SCENE_SLUG" = "johnny5" ]; }; then
  # These letter-message full host surfaces can contaminate bubbles/letters
  # and lower-left Johnny pixels with stale moving foreground. Unlike JOHNNY 2,
  # their foreground-only captures carry the message lane cleanly, so use them
  # frame-wide.
  KEYED_OVERLAY_RECT="0,0,640,480"
fi
if [ -z "$KEYED_OVERLAY_RECT" ] && [ "$SCENE_SLUG" = "activity10" ]; then
  # ACTIVITY 10 (reads-seagull-steals-book): the static-Johnny base-diff
  # leaves a ghosted standing-Johnny pose on the RIGHT side of the island
  # (visible as a dark Johnny silhouette while the real Johnny sits reading).
  # Same residue activity7/activity11 hit; replace base-diff with keyed
  # foreground-only across the whole frame so no stale pose survives.
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
if [ -z "$KEYED_OVERLAY_RECT" ] && [ "$SCENE_SLUG" = "suzy2" ]; then
  # SUZY 2 needs MRAFT.BMP in the foreground overlay. The static-base include
  # below keeps the raft body while this rect avoids a huge full-host pack.
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
if [ "$NO_STITCH_REQUESTED" = "1" ]; then
  MULTIVIEW_STITCH="0"
elif [ "$FORCE_STITCH_REQUESTED" = "1" ]; then
  MULTIVIEW_STITCH="1"
elif [ -z "$MULTIVIEW_STITCH" ]; then
  if [ "$SCENE_SLUG" = "johnny2" ] || [ "$SCENE_SLUG" = "mary2" ] || [ "$SCENE_SLUG" = "activity5" ]; then
    # These scenes have validated custom capture paths: JOHNNY2 keeps a
    # partial lower-band overlay so thought bubbles stay full-host, MARY2
    # has a bespoke multi-view + bubble-shell injection branch below, and
    # ACTIVITY 5's storm-cloud thought bubble + connector dots only survive
    # in pure base-diff (frame-wide keyed overlay drops the bubble shell).
    MULTIVIEW_STITCH="0"
  elif [ "$SCENE_SLUG" = "activity9" ]; then
    # ACTIVITY 9's boat spans wider than the host viewport during entry/exit.
    # It needs extra-wide far-left/far-right foreground views so the stitched
    # scene-local canvas has both the incoming stern and outgoing bow.
    MULTIVIEW_STITCH="1"
  elif [ "$SCENE_SLUG" = "suzy2" ]; then
    # SUZY 2's raft body is drawn by MRAFT.BMP as scene-local static art.
    # Keep a single capture position, but include static base draws in the
    # foreground-only overlay below so the raft is not dropped.
    MULTIVIEW_STITCH="0"
  elif [[ "$SCENE_SLUG" == stand* ]]; then
    # STAND scenes are static/island-local by construction. Start them on the
    # fast single-position capture path; use --stitch if validation exposes
    # clipped off-screen action later.
    MULTIVIEW_STITCH="0"
  elif [ "$SCENE_SLUG" = "building1" ] ||
       [ "$SCENE_SLUG" = "building2" ] ||
       [ "$SCENE_SLUG" = "building3" ] ||
       [ "$SCENE_SLUG" = "building4" ] ||
       [ "$SCENE_SLUG" = "building5" ] ||
       [ "$SCENE_SLUG" = "building6" ] ||
       [ "$SCENE_SLUG" = "building7" ] ||
       [ "$SCENE_SLUG" = "mary4" ] ||
       [ "$SCENE_SLUG" = "mary5" ] ||
       [ "$SCENE_SLUG" = "mary6" ] ||
       [ "$SCENE_SLUG" = "miscgag1" ] ||
       [ "$SCENE_SLUG" = "miscgag2" ] ||
       [ "$SCENE_SLUG" = "visitor1" ] ||
       [ "$SCENE_SLUG" = "visitor3" ] ||
       [ "$SCENE_SLUG" = "visitor4" ] ||
       [ "$SCENE_SLUG" = "visitor5" ] ||
       [ "$SCENE_SLUG" = "visitor6" ] ||
       [ "$SCENE_SLUG" = "visitor7" ] ||
       [ "$SCENE_SLUG" = "walkstuf1" ]; then
    # These scenes have already proven they need the normal/far-left/far-right
    # stitch, a scene-local persistent prop, or a helper that depends on the
    # stitched foreground canvas. Keep them on the safe validated path.
    MULTIVIEW_STITCH="1"
  else
    # New scene validation starts on the fast single-position path. Use
    # --stitch, or add the scene above, only after host review or visual
    # validation proves off-screen clipping or scene-local persistent state.
    MULTIVIEW_STITCH="0"
  fi
fi
if [ "$MULTIVIEW_STITCH" = "1" ]; then
  KEYED_OVERLAY_RECT="0,0,640,480"
elif [ -z "$KEYED_OVERLAY_RECT" ]; then
  # Fast no-stitch captures still need a foreground-only source for static
  # actor pixels. A pure base-diff pack treats frame-0 Johnny as background and
  # can drag stale full-host composites when the actor moves over water/tree
  # backdrops.
  KEYED_OVERLAY_RECT="0,0,640,480"
fi
if [ -z "$KEYED_OVERLAY_INCLUDE_STATIC_BASE" ] && [ "$SCENE_SLUG" = "fishing5" ]; then
  # The shark waterline also uses current BACKGRND.BMP ledger draws. The
  # default foreground-only capture excludes those for other scenes, but
  # FISHING 5 needs them to avoid outline-only shark frames.
  KEYED_OVERLAY_INCLUDE_STATIC_BASE="1"
fi
if [ -z "$KEYED_OVERLAY_INCLUDE_STATIC_BASE" ] && [ "$SCENE_SLUG" = "suzy2" ]; then
  # MRAFT.BMP is a scene-local static prop. Without static-base foreground
  # replay, the raft body is treated as background and Johnny appears to float.
  KEYED_OVERLAY_INCLUDE_STATIC_BASE="1"
fi
if [ -z "$KEYED_OVERLAY_SKIP_VISIBILITY_MASK" ] && [ "$SCENE_SLUG" = "suzy1" ]; then
  # SUZY 1's thought-bubble connector dots live in the keyed lower band
  # (they trail up from sleeping Johnny's head) and are thin enough that
  # the byte-exact visibility mask drops them — the "middle bubbles
  # missing" console report. Same remedy as FISHING 5: replay the
  # current ledger directly for keyed captures.
  KEYED_OVERLAY_SKIP_VISIBILITY_MASK="1"
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
if [ -z "$HOLD_ADJUSTMENTS" ] && [ "$SCENE_SLUG" = "activity5" ]; then
  # The storm-cloud thought bubble (source frame 46) only holds for 4 vblanks,
  # too fast to read on the climb/look/dive gag. Pause on it long enough to
  # read the weather concern.
  HOLD_ADJUSTMENTS="46:+30"
fi
if [ -z "$HOLD_ADJUSTMENTS" ] && [ "$SCENE_SLUG" = "visitor3" ]; then
  # The clean splash exists only on source frame 158. Earlier rescue attempts
  # copied stale full-host splash pixels into later ship rows; keep the real
  # splash readable by moving a few ticks from the first ship row instead.
  HOLD_ADJUSTMENTS="158:+4 159:-4"
fi
if [ -z "$HOLD_ADJUSTMENTS" ] && [ "$SCENE_SLUG" = "visitor5" ]; then
  # The coconut hit/downed-plane motion has several one-tick rows between long
  # static holds. Move time into those action rows so the gag reads clearly.
  HOLD_ADJUSTMENTS="91:-2 93:-2 95:-2 96:-1 98:-2 100:-2 102:-2 104:-1 105:+2 108:+2 109:+1 112:+2 113:+3 116:+2 119:+2 147:-2 149:+2 150:-2 152:+2 153:-2 155:+2 156:-2 158:+2 159:-2 161:+2 162:-2 164:+2 165:-2 167:+2 168:-2 170:+2 171:-2 173:+2 174:-2 176:+2 177:-1 179:+2 180:-1"
fi
if [ -z "$HOLD_ADJUSTMENTS" ] && [ "$SCENE_SLUG" = "visitor7" ]; then
  # The coconut impact star frames exist in the capture but dedupe leaves them
  # at four ticks between long static poses, so they read as missing in replay.
  HOLD_ADJUSTMENTS="32:+8 39:-8 62:+8 65:-8 71:+8 74:-8 80:+8 85:-8"
fi
if [ -z "$HOLD_ADJUSTMENTS" ] && [ "$SCENE_SLUG" = "activity1" ]; then
  # ACTIVITY 1 has two score-card beats in the capped story capture. Hold both
  # animal score-card poses long enough for the gag to read.
  HOLD_ADJUSTMENTS="148:+208 348:+208"
fi
capture_stop_args=(--until-exit)
if [ "$SCENE_SLUG" = "activity1" ]; then
  # The host story-single route repeats this gag if left to run until process
  # exit. Capture the two signed-off beats and stop at a clean loop boundary.
  ACTIVITY1_CAPTURE_FRAMES="${FG_EXPORT_ACTIVITY1_CAPTURE_FRAMES:-400}"
  capture_stop_args=(--frames "$ACTIVITY1_CAPTURE_FRAMES")
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
  "${capture_stop_args[@]}" \
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
  "${capture_stop_args[@]}" \
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
convert_pack_to_fgp3=0
if [ -n "$KEYED_OVERLAY_RECT" ]; then
  "$SCRIPT_DIR/capture-host-scene.sh" \
    --scene "$SCENE_NAME" \
    --mode story-single \
    --seed 1 \
    --start-frame "$START_FRAME" \
    --interval 1 \
    "${capture_stop_args[@]}" \
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
    "${capture_stop_args[@]}" \
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
      "${capture_stop_args[@]}" \
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
      "${capture_stop_args[@]}" \
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
      "${capture_stop_args[@]}" \
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
      "${capture_stop_args[@]}" \
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
import os
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


def write_merged_foreground(reference_capture_dir, source_fg_dirs, out_dir,
                            anchor_env="FG_EXPORT_ANCHOR_Y_DELTA"):
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
    # FG_EXPORT_ANCHOR_Y_DELTA: shift the merged content DOWN by N px
    # within the canvas (pads the canvas height to fit). The pack spans
    # carry canvas coordinates, so this is the lever that actually moves
    # actors at runtime — a metadata-only offset tweak leaves the pack
    # bytes untouched.
    # Per-tide override: the LOW canvas may need a deeper drop than HIGH
    # (mary5: low island sits lower -> 24 vs 12). anchor_env selects which
    # env var this invocation honors; LOW falls back to the shared value.
    anchor_raw = os.environ.get(anchor_env)
    if anchor_raw is None and anchor_env != "FG_EXPORT_ANCHOR_Y_DELTA":
        anchor_raw = os.environ.get("FG_EXPORT_ANCHOR_Y_DELTA")
    anchor_dy = int(anchor_raw or 0)
    canvas_h += anchor_dy

    for ref_frame, ref_meta, local_pixels in frames:
        merged = Image.new("RGB", (canvas_w, canvas_h), KEY)
        if local_pixels:
            pix = merged.load()
            for (local_x, local_y), rgb in local_pixels.items():
                sx = local_x + synth_offset_x
                sy = local_y + synth_offset_y + anchor_dy
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
write_merged_foreground(sys.argv[6], [sys.argv[7], sys.argv[8], sys.argv[9]], sys.argv[10],
                        anchor_env="FG_EXPORT_ANCHOR_Y_DELTA_LOW")
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
    if [ "$SCENE_SLUG" = "activity9" ]; then
      # The boat still clips with the generic +/-300 stitch anchors. Use wider
      # capture anchors; runtime island placement remains variable.
      STITCH_LEFT_ISLAND_X="${FG_EXPORT_STITCH_LEFT_ISLAND_X:--500}"
      STITCH_RIGHT_ISLAND_X="${FG_EXPORT_STITCH_RIGHT_ISLAND_X:-500}"
    fi

    "$SCRIPT_DIR/capture-host-scene.sh" \
      --scene "$SCENE_NAME" \
      --mode story-single \
      --seed 1 \
      --start-frame "$START_FRAME" \
      --interval 1 \
      "${capture_stop_args[@]}" \
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
      "${capture_stop_args[@]}" \
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
      "${capture_stop_args[@]}" \
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
      "${capture_stop_args[@]}" \
      --no-stamp \
      --lowtide 1 \
      --raft-stage "$CAPTURE_RAFT_STAGE" \
      --island-x "$STITCH_RIGHT_ISLAND_X" \
      --island-y "$CAPTURE_ISLAND_Y" \
      --foreground-only \
      "${foreground_capture_args[@]}" \
      --output "$HOST_CAPTURE_LOW_FAR_STITCH_FGONLY_DIR"

    if [ "$SCENE_SLUG" = "visitor3" ]; then
      # VISITOR 3's red ship is revealed as a moving foreground slice. The
      # full host surface accumulates that slice but then keeps stale red too
      # long; foreground-only by itself drops the already-revealed hull. Build
      # a clean synthesized source that accumulates only the ship slice, then
      # convert to FGP3 so the post-crash blank frame explicitly restores the
      # red hull instead of being treated as a hold frame.
      #
      # The primary reference (island-x -154) is blind to scene-local
      # x < 154, which shipped as a hard vertical truncation of the hull's
      # left side on PS1 (2026-07-02). Capture an extra FULL-host view at the
      # far-right stitch position so the merge can fill the accumulated hull
      # across the whole scene-local sweep.
      "$SCRIPT_DIR/capture-host-scene.sh" \
        --scene "$SCENE_NAME" \
        --mode story-single \
        --seed 1 \
        --start-frame "$START_FRAME" \
        --interval 1 \
        "${capture_stop_args[@]}" \
        --no-stamp \
        --lowtide 0 \
        --raft-stage "$CAPTURE_RAFT_STAGE" \
        --island-x "$STITCH_RIGHT_ISLAND_X" \
        --island-y "$CAPTURE_ISLAND_Y" \
        --output "$OUTPUT_DIR/host-capture-high-far-stitch-full"

      "$SCRIPT_DIR/capture-host-scene.sh" \
        --scene "$SCENE_NAME" \
        --mode story-single \
        --seed 1 \
        --start-frame "$START_FRAME" \
        --interval 1 \
        "${capture_stop_args[@]}" \
        --no-stamp \
        --lowtide 1 \
        --raft-stage "$CAPTURE_RAFT_STAGE" \
        --island-x "$STITCH_RIGHT_ISLAND_X" \
        --island-y "$CAPTURE_ISLAND_Y" \
        --output "$OUTPUT_DIR/host-capture-low-far-stitch-full"

      python3 "$SCRIPT_DIR/merge-visitor3-ship-foreground.py" \
        --reference-capture "$HOST_CAPTURE_HIGH_DIR" \
        --extra-reference-capture "$OUTPUT_DIR/host-capture-high-far-stitch-full" \
        --source-fg-dir "$HOST_CAPTURE_HIGH_FGONLY_DIR" \
        --source-fg-dir "$HOST_CAPTURE_HIGH_STITCH_FGONLY_DIR" \
        --source-fg-dir "$HOST_CAPTURE_HIGH_FAR_STITCH_FGONLY_DIR" \
        --output "$HOST_CAPTURE_HIGH_MERGED_FGONLY_DIR"

      python3 "$SCRIPT_DIR/merge-visitor3-ship-foreground.py" \
        --reference-capture "$HOST_CAPTURE_LOW_DIR" \
        --extra-reference-capture "$OUTPUT_DIR/host-capture-low-far-stitch-full" \
        --source-fg-dir "$HOST_CAPTURE_LOW_FGONLY_DIR" \
        --source-fg-dir "$HOST_CAPTURE_LOW_STITCH_FGONLY_DIR" \
        --source-fg-dir "$HOST_CAPTURE_LOW_FAR_STITCH_FGONLY_DIR" \
        --output "$HOST_CAPTURE_LOW_MERGED_FGONLY_DIR"

      convert_pack_to_fgp3=1
    else
      stitch_inject_args=()
      if [ "$SCENE_SLUG" = "visitor6" ]; then
        # VISITOR 6 has scene-owned coconut/tree impact pixels in the full
        # host layer. Foreground-only captures keep Johnny/coconuts but omit
        # the tree strike deltas, so inject only the non-backdrop full-host
        # differences during the impact window.
        stitch_inject_args=(
          --inject-full-host-diff-rect "330,55,260,275"
          --inject-full-host-diff-frames "120:141"
        )
      fi
      if [ "$SCENE_SLUG" = "building2" ] ||
         [ "$SCENE_SLUG" = "building4" ] ||
         [ "$SCENE_SLUG" = "building7" ]; then
        # BUILDING 2/4/7 need temporal-residual cleanup for scene-local state.
        # BUILDING 7's campfire is also a persistent prop that is cheaper and
        # safer as explicit residual spans after the injected full-host lane.
        convert_pack_to_fgp3=1
      fi
      if [ "$SCENE_SLUG" = "mary5" ]; then
        # MARY 5 is residual-compressed, but live shoreline waves tick between
        # frames. Restamp the lower actor band below so shark/Mary pixels stay
        # visually above the wave animation without full-frame residual draws.
        convert_pack_to_fgp3=1
      fi
      if [ "$SCENE_SLUG" = "building2" ]; then
        # The Lilliputian sandcastle is scene-local state: it is built once,
        # then the red flag and planes originate from it. Foreground-only
        # captures can omit the mostly-static yellow castle after reveal, so
        # inject the full-host diff lane and allow sand-colored pixels through
        # the backdrop filter for this rect only.
        stitch_inject_args=(
          --inject-full-host-diff-rect "70,220,310,220"
          --inject-full-host-diff-frames "20:408"
          --inject-full-host-diff-keep-sand
          --inject-full-host-diff-sand-component-min-pixels 80
          --inject-full-host-diff-sand-component-right-pad 4
          --trim-sand-tail-frames "20:408"
          --trim-sand-tail-min-y 245
          --trim-sand-tail-component-min-pixels 80
          --trim-sand-tail-column-min-sand 5
          --trim-sand-tail-right-pad 4
          --drop-output-rect "668,179,75,25"
          --drop-output-frames "360:400"
        )
      fi

      python3 "$SCRIPT_DIR/merge-scene-foreground-views.py" \
        --reference-capture "$HOST_CAPTURE_HIGH_DIR" \
        --source-fg-dir "$HOST_CAPTURE_HIGH_FGONLY_DIR" \
        --source-fg-dir "$HOST_CAPTURE_HIGH_STITCH_FGONLY_DIR" \
        --source-fg-dir "$HOST_CAPTURE_HIGH_FAR_STITCH_FGONLY_DIR" \
        "${stitch_inject_args[@]}" \
        --output "$HOST_CAPTURE_HIGH_MERGED_FGONLY_DIR"

      python3 "$SCRIPT_DIR/merge-scene-foreground-views.py" \
        --reference-capture "$HOST_CAPTURE_LOW_DIR" \
        --source-fg-dir "$HOST_CAPTURE_LOW_FGONLY_DIR" \
        --source-fg-dir "$HOST_CAPTURE_LOW_STITCH_FGONLY_DIR" \
        --source-fg-dir "$HOST_CAPTURE_LOW_FAR_STITCH_FGONLY_DIR" \
        "${stitch_inject_args[@]}" \
        --output "$HOST_CAPTURE_LOW_MERGED_FGONLY_DIR"
    fi

    if [ "$SCENE_SLUG" = "activity1" ]; then
      # Foreground-only views draw Johnny over the palm during the post-coconut
      # jump. The full host composite correctly hides those pixels behind the
      # tree, so patch that lane from full-host differences after stitching.
      python3 "$SCRIPT_DIR/patch-activity1-tree-foreground.py" \
        "$HOST_CAPTURE_HIGH_DIR" \
        "$HOST_CAPTURE_HIGH_MERGED_FGONLY_DIR"
      python3 "$SCRIPT_DIR/patch-activity1-tree-foreground.py" \
        "$HOST_CAPTURE_LOW_DIR" \
        "$HOST_CAPTURE_LOW_MERGED_FGONLY_DIR"
    fi

    if [ "$SCENE_SLUG" = "activity9" ]; then
      # ACTIVITY 9 moves BOAT.BMP wider than any single host viewport. The
      # normal/far-left/far-right captures tell us the authored per-frame
      # position, but screen-window clipping can still cut the entering stern
      # or exiting bow. Fill only keyed gaps from the decoded BOAT source.
      python3 "$SCRIPT_DIR/patch-activity9-boat-foreground.py" \
        "$HOST_CAPTURE_HIGH_DIR" \
        "$HOST_CAPTURE_HIGH_MERGED_FGONLY_DIR"
      python3 "$SCRIPT_DIR/patch-activity9-boat-foreground.py" \
        "$HOST_CAPTURE_LOW_DIR" \
        "$HOST_CAPTURE_LOW_MERGED_FGONLY_DIR"
    fi

    if [ "$SCENE_SLUG" = "building7" ]; then
      # Full-host diff copies keep this campfire present but ghosted. Use the
      # clean animated foreground rows instead and layer them behind the live
      # action for the long missing middle interval.
      python3 "$SCRIPT_DIR/patch-building7-fire-foreground.py" \
        "$HOST_CAPTURE_HIGH_MERGED_FGONLY_DIR"
      python3 "$SCRIPT_DIR/patch-building7-fire-foreground.py" \
        "$HOST_CAPTURE_LOW_MERGED_FGONLY_DIR"
    fi

    high_overlay_args=()
    low_overlay_args=()
    HOST_CAPTURE_HIGH_DIR="$HOST_CAPTURE_HIGH_MERGED_FGONLY_DIR"
    HOST_CAPTURE_LOW_DIR="$HOST_CAPTURE_LOW_MERGED_FGONLY_DIR"
    KEYED_OVERLAY_RECT=""
    scene_base_args=(--scene-base-color ff00ff)
    if [ "$SCENE_SLUG" = "building2" ] ||
       [ "$SCENE_SLUG" = "building4" ] ||
       [ "$SCENE_SLUG" = "building7" ]; then
      python3 "$SCRIPT_DIR/append-foreground-cleanup-frame.py" \
        "$HOST_CAPTURE_HIGH_DIR"
      python3 "$SCRIPT_DIR/append-foreground-cleanup-frame.py" \
        "$HOST_CAPTURE_LOW_DIR"
    fi
  elif [ "$SCENE_SLUG" != "mary2" ]; then
    if [ "$SCENE_SLUG" = "activity1" ]; then
      # Foreground-only replay draws Johnny over the palm during the post-
      # coconut jump. Patch that tree lane from the full-host composite before
      # the pack builder consumes the foreground-only overlay.
      python3 "$SCRIPT_DIR/patch-activity1-tree-foreground.py" \
        "$HOST_CAPTURE_HIGH_DIR" \
        "$HOST_CAPTURE_HIGH_FGONLY_DIR"
      python3 "$SCRIPT_DIR/patch-activity1-tree-foreground.py" \
        "$HOST_CAPTURE_LOW_DIR" \
        "$HOST_CAPTURE_LOW_FGONLY_DIR"
    fi

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

if [ "$convert_pack_to_fgp3" = "1" ]; then
  high_fgp3_tmp="${PACK_PATH}.fgp3tmp"
  low_fgp3_tmp="${LOWTIDE_PACK_PATH}.fgp3tmp"
  fgp3_extra_args=()
  if [ "$SCENE_SLUG" = "mary5" ]; then
    fgp3_extra_args+=(--restamp-rect "80,315,300,70")
  fi
  python3 "$SCRIPT_DIR/build-fg3-temporal-residual-pack.py" \
    --input-fg2 "$PACK_PATH" \
    "${fgp3_extra_args[@]}" \
    --output-fg3 "$high_fgp3_tmp"
  python3 "$SCRIPT_DIR/build-fg3-temporal-residual-pack.py" \
    --input-fg2 "$LOWTIDE_PACK_PATH" \
    "${fgp3_extra_args[@]}" \
    --output-fg3 "$low_fgp3_tmp"
  mv "$high_fgp3_tmp" "$PACK_PATH"
  mv "$low_fgp3_tmp" "$LOWTIDE_PACK_PATH"
fi

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
