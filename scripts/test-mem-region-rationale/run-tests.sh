#!/bin/bash
# Tests scripts/check-mem-region-rationale.py against fixture files.
# Plan v9 step 27 (S17). Run from project root or this dir.
# Note: NO 'set -e' — we EXPECT non-zero exits from the script under
# test for the failing fixtures.
cd "$(dirname "$0")"
SCRIPT=../check-mem-region-rationale.py
fail=0

run_expect() {
    local fixture=$1
    local expected=$2
    set +e
    python3 "$SCRIPT" "$fixture" >/dev/null 2>&1
    actual=$?
    set +e
    if [ "$actual" -eq "$expected" ]; then
        echo "PASS  $fixture (exit $actual)"
    else
        echo "FAIL  $fixture (expected exit $expected, got $actual)"
        fail=1
    fi
}

run_expect valid.c           0
run_expect missing-comment.c 1
run_expect far-comment.c     1
run_expect wrapping-macro.c  2
run_expect multi-line.c      0

echo ""
if [ "$fail" -eq 0 ]; then
    echo "All 5 fixture tests pass."
    exit 0
else
    echo "Some fixture tests failed."
    exit 1
fi
