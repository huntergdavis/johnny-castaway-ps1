#!/bin/bash
# Build standardized visual-audit montages for one scene variant from its
# host-capture frames. Usage: audit-montage.sh <slug> <high|low>
# Writes PNGs to /tmp/audit/<slug>/<tide>_{full,edges,endpoints,dropout}.png
# Each montage is downsampled for fast model inspection.
set -u
SLUG="${1:?slug}"; TIDE="${2:?high|low}"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
case "$TIDE" in
  high) FULL="$ROOT/host-results/$SLUG-foreground-pilot/host-capture-high/frames";
        FG="$ROOT/host-results/$SLUG-foreground-pilot/host-capture-high-fgonly/frames" ;;
  low)  FULL="$ROOT/host-results/$SLUG-foreground-pilot/host-capture-low/frames";
        FG="$ROOT/host-results/$SLUG-foreground-pilot/host-capture-low-fgonly/frames" ;;
  *) echo "tide must be high|low" >&2; exit 2 ;;
esac
[ -d "$FULL" ] || { echo "NO_FRAMES $FULL"; exit 1; }
OUT="/tmp/audit/$SLUG"; mkdir -p "$OUT"; rm -f "$OUT/${TIDE}_"*.png
mapfile -t FRAMES < <(ls "$FULL"/frame_*.bmp 2>/dev/null | sort)
N=${#FRAMES[@]}
[ "$N" -eq 0 ] && { echo "NO_FRAMES $FULL"; exit 1; }
idx() { # echo evenly-spaced indices, count $1
  local c=$1 i; for ((i=0;i<c;i++)); do echo $(( i*(N-1)/(c>1?c-1:1) )); done; }

# 1) full: 8 evenly-spaced frames, labeled
args=(); for i in $(idx 8); do
  f="${FRAMES[$i]}"; convert "$f" -resize 200x -gravity South -background black -splice 0x14 \
    -annotate +0+1 "f$i" "$OUT/.f_$i.png" 2>/dev/null && args+=("$OUT/.f_$i.png"); done
montage "${args[@]}" -tile 4x2 -geometry +2+2 -title "$SLUG $TIDE FULL" "$OUT/${TIDE}_full.png" 2>/dev/null

# 2) edges: left 140px and right 140px crops of 3 frames (entering / cut-off sprites)
eargs=()
for i in $(idx 3); do
  f="${FRAMES[$i]}"
  convert "$f" -crop 140x480+0+0 +repage -resize x300 -gravity South -background navy -splice 0x12 -annotate +0+1 "f$i L" "$OUT/.el_$i.png" 2>/dev/null && eargs+=("$OUT/.el_$i.png")
  convert "$f" -crop 140x480+500+0 +repage -resize x300 -gravity South -background navy -splice 0x12 -annotate +0+1 "f$i R" "$OUT/.er_$i.png" 2>/dev/null && eargs+=("$OUT/.er_$i.png")
done
montage "${eargs[@]}" -tile 2x3 -geometry +2+2 -title "$SLUG $TIDE EDGES (L|R per row)" "$OUT/${TIDE}_edges.png" 2>/dev/null

# 3) endpoints: first 2 + last 2 frames full-res-ish (missing/blank first/last frame)
pargs=()
for i in 0 1 $((N-2)) $((N-1)); do
  [ $i -lt 0 ] && continue; f="${FRAMES[$i]}"
  [ -f "$f" ] && convert "$f" -resize 240x -gravity South -background black -splice 0x12 -annotate +0+1 "f$i" "$OUT/.p_$i.png" 2>/dev/null && pargs+=("$OUT/.p_$i.png")
done
montage "${pargs[@]}" -tile 4x1 -geometry +2+2 -title "$SLUG $TIDE ENDPOINTS" "$OUT/${TIDE}_endpoints.png" 2>/dev/null

# 4) dropout: full vs fgonly (magenta=transparent) for 2 mid frames — thin/bubble dropout
if [ -d "$FG" ]; then
  dargs=()
  for i in $(idx 4 | sed -n '2p;3p'); do
    f="${FRAMES[$i]}"; g="$FG/$(basename "$f")"
    convert "$f" -resize 220x -gravity South -background black -splice 0x12 -annotate +0+1 "full f$i" "$OUT/.df_$i.png" 2>/dev/null && dargs+=("$OUT/.df_$i.png")
    [ -f "$g" ] && convert "$g" -resize 220x -background magenta -flatten -gravity South -background black -splice 0x12 -annotate +0+1 "fgonly f$i" "$OUT/.dg_$i.png" 2>/dev/null && dargs+=("$OUT/.dg_$i.png")
  done
  montage "${dargs[@]}" -tile 2x2 -geometry +2+2 -title "$SLUG $TIDE full vs fgonly" "$OUT/${TIDE}_dropout.png" 2>/dev/null
fi
rm -f "$OUT/".[ef]*.png "$OUT/".[dp]_*.png 2>/dev/null
echo "frames=$N out=$OUT"
ls "$OUT/${TIDE}_"*.png 2>/dev/null