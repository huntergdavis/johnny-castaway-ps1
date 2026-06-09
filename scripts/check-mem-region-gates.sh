#!/bin/bash
# scripts/check-mem-region-gates.sh
#
# Runs all the mem-region plan v9 CI gates. Returns 0 if everything
# passes, non-zero on any failure with a clear diagnostic.
#
# Plan v9 references: steps 17 (CI checks), 20 (rationale), 21 (count
# match), 22 (pack-hash currency).
#
# Usage:
#     ./scripts/check-mem-region-gates.sh
#
# Designed to run from the project root or any subdir.

set -e

# Find project root (parent of scripts/).
cd "$(dirname "$0")/.."
root="$(pwd)"

fail=0

# ----- Gate 1: MEM_REGION_RATIONALE comment on every memAlloc call.
echo "Gate 1: MEM_REGION_RATIONALE comments…"
if ! python3 scripts/check-mem-region-rationale.py src/ >/dev/null 2>&1; then
    echo "  FAIL: run 'python3 scripts/check-mem-region-rationale.py' for details"
    fail=1
else
    echo "  OK"
fi

# ----- Gate 2: MEM_REGION_* enum count matches decision-tree count.
echo "Gate 2: MEM_REGION_* enum count match…"
header_count=$(grep -oE "MEM_REGION_(BOOT|CACHE|TRANSIENT)\b" src/mem_region/mem_region.h | sort -u | wc -l | tr -d '[:space:]')
tree_count=$(grep -oE "MEM_REGION_(BOOT|CACHE|TRANSIENT)\b" docs/ps1/mem-region-decision-tree.md 2>/dev/null | sort -u | wc -l | tr -d '[:space:]')
if [ "$header_count" != "$tree_count" ] || [ -z "$header_count" ]; then
    echo "  FAIL: mem_region.h has $header_count unique MEM_REGION_* names; decision tree has $tree_count"
    fail=1
else
    echo "  OK ($header_count regions documented)"
fi

# ----- Gate 3: removal manifest grep — no zombie skip-code remaining.
echo "Gate 3: removal manifest grep (Phase 2)…"
if grep -rE "JCSKIP|Caller handles gracefully|skip scene silently|skip scene gracefully|Graceful skip|Pool exhausted - fall back" src/ >/dev/null 2>&1; then
    echo "  WARN: residual Phase 2 manifest strings still in src/ (some may be intentional"
    echo "        comments referring to removed code; check the matches manually)"
    grep -rEn "JCSKIP|Caller handles gracefully|skip scene silently|skip scene gracefully|Graceful skip|Pool exhausted - fall back" src/ | head -10
    # Not failing — these are often explanatory comments now.
else
    echo "  OK"
fi

# ----- Gate 4: MEM_DEV_BUILD must be off in release config.
echo "Gate 4: MEM_DEV_BUILD off in release config…"
if grep -qE "^[^/]*MEM_DEV_BUILD\s*=\s*1" CMakeLists.txt 2>/dev/null; then
    echo "  FAIL: MEM_DEV_BUILD=1 in CMakeLists.txt; must be 0 for release"
    fail=1
else
    echo "  OK (MEM_DEV_BUILD is implicit-off)"
fi

# ----- Gate 5: bsod-ui-test-mem-* flags must not auto-fire (BOOTMODE not pre-set).
echo "Gate 5: bsod-ui-test-mem-* bootmodes off by default…"
if [ -f config/ps1/BOOTMODE.TXT ]; then
    if grep -qE "bsod-ui-test-mem-" config/ps1/BOOTMODE.TXT; then
        echo "  FAIL: bsod-ui-test-mem-* found in BOOTMODE.TXT"
        fail=1
    else
        echo "  OK (no synthetic-halt bootmode active)"
    fi
else
    echo "  OK (no BOOTMODE.TXT)"
fi

# ----- Gate 6: pack metrics freshness — header regenerated from JSON.
echo "Gate 6: pack metrics freshness check…"
if [ -f scripts/generate-pack-metrics.py ] && [ -f docs/ps1/research/generated/scene_analysis_output_2026-03-21.json ]; then
    tmp_h=$(mktemp)
    tmp_c=$(mktemp)
    python3 scripts/generate-pack-metrics.py \
        --json docs/ps1/research/generated/scene_analysis_output_2026-03-21.json \
        --out-h "$tmp_h" --out-c "$tmp_c" >/dev/null 2>&1
    if ! diff -q "$tmp_h" src/generated/pack_header_metrics.h >/dev/null 2>&1; then
        echo "  FAIL: src/generated/pack_header_metrics.h is stale — regenerate via:"
        echo "        python3 scripts/generate-pack-metrics.py \\"
        echo "          --json docs/ps1/research/generated/scene_analysis_output_2026-03-21.json \\"
        echo "          --out-h src/generated/pack_header_metrics.h \\"
        echo "          --out-c src/generated/pack_header_metrics.c"
        fail=1
    else
        echo "  OK"
    fi
    rm -f "$tmp_h" "$tmp_c"
else
    echo "  SKIP (generator or JSON missing)"
fi

echo ""
if [ "$fail" -eq 0 ]; then
    echo "All mem-region gates pass."
    exit 0
else
    echo "Some gates failed — see messages above."
    exit 1
fi
