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

RUNS="${1:-1000000}"
echo "=== fuzz: $RUNS layouts (disease) ==="
./tests/mem_region_fuzz "$RUNS" 2>/dev/null | grep -E 'runs=|histogram|first repro'
echo "=== fuzz: $RUNS layouts (--fixed, segregation) ==="
./tests/mem_region_fuzz "$RUNS" --fixed 2>/dev/null | grep -E 'runs=|histogram'
echo "=== deterministic regression ==="
./tests/mem_region_frag_regression
