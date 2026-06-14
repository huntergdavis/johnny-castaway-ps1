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
echo "=== deterministic regression ==="
./tests/mem_region_frag_regression
