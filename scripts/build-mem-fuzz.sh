#!/bin/bash
# Build + run the host PS1 CACHE memory fuzzer / regression on the host.
# Links the REAL allocator (src/mem_region.c) so reproductions are
# faithful to the console. See docs/ps1/mem-fuzz-sim.md.
set -e
cd "$(dirname "$0")/.."

INC="-Isrc -Isrc/mem_region -Isrc/platform/ps1"
SRC="src/mem_region.c src/generated/pack_header_metrics.c"
CC="${CC:-gcc}"
CFLAGS="-O2 -Wall"

echo "=== building mem_region_fuzz ==="
$CC $CFLAGS -o tests/mem_region_fuzz tests/mem_region_fuzz.c $SRC $INC

echo "=== building mem_region_frag_regression ==="
$CC $CFLAGS -o tests/mem_region_frag_regression tests/mem_region_frag_regression.c $SRC $INC

echo "=== building mem_region_heapmap_validate ==="
$CC $CFLAGS -o tests/mem_region_heapmap_validate tests/mem_region_heapmap_validate.c $SRC $INC

echo "=== building mem_region_945_regression ==="
$CC $CFLAGS -o tests/mem_region_945_regression tests/mem_region_945_regression.c $SRC $INC

echo "=== building mem_sim (scene-sequence simulator) ==="
$CC $CFLAGS -o tests/mem_sim tests/mem_sim.c $SRC $INC

RUNS="${1:-1000000}"
echo "=== fuzz: $RUNS layouts (disease) ==="
./tests/mem_region_fuzz "$RUNS" 2>/dev/null | grep -E 'runs=|histogram|first repro'
echo "=== fuzz: $RUNS layouts (--fixed, segregation) ==="
./tests/mem_region_fuzz "$RUNS" --fixed 2>/dev/null | grep -E 'runs=|histogram'
echo "=== heap-map validation (model == real soak layout) ==="
./tests/mem_region_heapmap_validate
echo "=== building7@945 byte-exact regression ==="
./tests/mem_region_945_regression
echo "=== scene-sim: historical config (expect BSODs at depth) ==="
./tests/mem_sim --historical --scenes 3000 --soaks 200 2>/dev/null | grep config
echo "=== scene-sim: FIXED config confidence (expect 0 BSODs) ==="
./tests/mem_sim --scenes 3000 --soaks 2000 2>/dev/null | grep config

# Red-team property: the scene simulator must catch CACHE fragmentation
# ORGANICALLY (slab-pool retention strands an unprotected region), AND
# the deployed fix (periodic rebuild) must prevent it. An unprotected
# run (16 slots, no periodic rebuild) must strand at depth; the same
# sequence under the fix must survive.
echo "=== scene-sim: organic-fragmentation capture (no-rebuild must strand) ==="
FRAG=$(./tests/mem_sim --slots 16 --no-rebuild --scenes 200000 --seed 1 2>/dev/null | grep -oE 'bsod_scene=-?[0-9]+' | grep -oE '\-?[0-9]+')
if [ "$FRAG" -gt 0 ]; then
  echo "OK: unprotected region stranded on fragmentation at scene $FRAG (organic capture)"
else
  echo "FAIL: simulator did not reproduce fragmentation organically"; exit 1
fi
echo "=== building mem_map_replay (byte-exact layout reconstruction) ==="
$CC $CFLAGS -o tests/mem_map_replay tests/mem_map_replay.c $SRC $INC
echo "=== byte-exact scene-945 BSOD replay (the real captured console layout) ==="
# Replays the forensic-captured 945 BSOD heap map through the real allocator
# and asserts the byte-exact console signature: have=88024, largest<65536.
R=$(./tests/mem_map_replay tests/fixtures/soak945_bsod.map 2>/dev/null)
echo "$R" | grep -E 'cache_used|have|STRAND'
if echo "$R" | grep -q 'have)=88024' && echo "$R" | grep -q 'STRANDS'; then
  echo "OK: scene-945 BSOD reproduced byte-exact (have=88024, 65536 strands) — no emulator"
else
  echo "FAIL: scene-945 byte-exact replay regressed"; exit 1
fi
echo "=== deterministic regression ==="
./tests/mem_region_frag_regression

echo "=== building mem_path_replay (op-path reproduction + fix gate) ==="
$CC $CFLAGS -o tests/mem_path_replay tests/mem_path_replay.c $SRC $INC
echo "=== scene-945 op-path: interleaved strands, segregation fixes it ==="
INTER=$(./tests/mem_path_replay tests/fixtures/soak945_goingin.map tests/fixtures/soak945_building7_ops.txt 2>/dev/null)
SEG=$(./tests/mem_path_replay tests/fixtures/soak945_goingin_segregated.map tests/fixtures/soak945_building7_ops.txt 2>/dev/null)
echo "  interleaved: $(echo "$INTER" | grep -oE '(STRAND|NO STRAND):.*')"
echo "  segregated : $(echo "$SEG" | grep -oE '(STRAND|NO STRAND):.*')"
if echo "$INTER" | grep -q '^STRAND' && echo "$SEG" | grep -q '^NO STRAND'; then
  echo "OK: path reproduces the BSOD and segregation clears it (fix validated)"
else
  echo "FAIL: scene-945 fix validation regressed"; exit 1
fi
