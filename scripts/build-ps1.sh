#!/bin/bash
# PS1 Build Script - Builds jcreborn.exe using Docker and PSn00bSDK
# Usage: ./build-ps1.sh [clean]

set -euo pipefail

cd "$(dirname "$0")/.."  # Change to project root
# shellcheck source=./docker-common.sh
source "scripts/docker-common.sh"
docker_init

PS1_PERF_DEEP_TRACE="${PS1_PERF_DEEP_TRACE:-OFF}"
PS1_PERF_VERBOSE_SCHEMA="${PS1_PERF_VERBOSE_SCHEMA:-OFF}"
PS1_VERBOSE_DIAGNOSTICS="${PS1_VERBOSE_DIAGNOSTICS:-OFF}"

python3 scripts/build-caption-data.py
python3 scripts/build-menu-text-data.py

python3 - <<'PY'
import json
import time
from pathlib import Path

root = Path.cwd()
bootmode_path = root / "config/ps1/BOOTMODE.TXT"
bootmode_header_path = root / "config/ps1/bootmode_embedded.h"
padscript_path = root / "config/ps1/PADSCRIPT.TXT"
padscript_header_path = root / "config/ps1/padscript_embedded.h"
build_date_header_path = root / "config/ps1/build_date_embedded.h"
bootmode = ""
if bootmode_path.is_file():
    bootmode = bootmode_path.read_text(encoding="utf-8").strip()
padscript = ""
if padscript_path.is_file():
    padscript_lines = []
    for raw_line in padscript_path.read_text(encoding="utf-8").splitlines():
        stripped = raw_line.strip()
        if not stripped or stripped.startswith("#"):
            continue
        padscript_lines.append(raw_line.rstrip())
    padscript = "\n".join(padscript_lines)

header = (
    "#ifndef PS1_BOOTMODE_EMBEDDED_H\n"
    "#define PS1_BOOTMODE_EMBEDDED_H\n\n"
    f"#define PS1_EMBEDDED_BOOT_OVERRIDE {json.dumps(bootmode)}\n\n"
    "#endif\n"
)
bootmode_header_path.write_text(header, encoding="utf-8")

padscript_header = (
    "#ifndef PS1_PADSCRIPT_EMBEDDED_H\n"
    "#define PS1_PADSCRIPT_EMBEDDED_H\n\n"
    f"#define PS1_EMBEDDED_PAD_SCRIPT {json.dumps(padscript)}\n\n"
    "#endif\n"
)
padscript_header_path.write_text(padscript_header, encoding="utf-8")

build_date = time.strftime("%b %d %Y")
build_date_header = (
    "#ifndef PS1_BUILD_DATE_EMBEDDED_H\n"
    "#define PS1_BUILD_DATE_EMBEDDED_H\n\n"
    f"#define PS1_EMBEDDED_BUILD_DATE {json.dumps(build_date)}\n\n"
    "#endif\n"
)
build_date_header_path.write_text(build_date_header, encoding="utf-8")
PY

if [ "${1:-}" = "clean" ]; then
    echo "=== Cleaning build directory ==="
    "${DOCKER_CMD[@]}" run --rm --platform linux/amd64 \
        -v "$PWD":/project \
        jc-reborn-ps1-dev:amd64 \
        bash -lc '
            set -e
            rm -rf /project/build-ps1
            mkdir -p /project/build-ps1
            cmake -G "Unix Makefiles" \
                -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY \
                -DPS1_PERF_DEEP_TRACE='"$PS1_PERF_DEEP_TRACE"' \
                -DPS1_PERF_VERBOSE_SCHEMA='"$PS1_PERF_VERBOSE_SCHEMA"' \
                -DPS1_VERBOSE_DIAGNOSTICS='"$PS1_VERBOSE_DIAGNOSTICS"' \
                -S /project -B /project/build-ps1
        '
fi

echo "=== Building PS1 executable ==="
"${DOCKER_CMD[@]}" run --rm --platform linux/amd64 \
    -v "$PWD":/project \
    jc-reborn-ps1-dev:amd64 \
    bash -lc '
        set -e
        rm -f /project/build-ps1/jcreborn.exe \
              /project/build-ps1/jcreborn.elf \
              /project/build-ps1/jcreborn.map
        cmake -G "Unix Makefiles" \
            -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY \
            -DPS1_PERF_DEEP_TRACE='"$PS1_PERF_DEEP_TRACE"' \
            -DPS1_PERF_VERBOSE_SCHEMA='"$PS1_PERF_VERBOSE_SCHEMA"' \
            -DPS1_VERBOSE_DIAGNOSTICS='"$PS1_VERBOSE_DIAGNOSTICS"' \
            -S /project -B /project/build-ps1
        cd /project/build-ps1
        make jcreborn
    '

echo ""
echo "=== Build complete ==="
test -f build-ps1/jcreborn.exe
ls -lh build-ps1/jcreborn.exe

# Static-image ceiling guard. The libc heap (region buffer 1440 KB +
# 2x32 KB GPU primitive buffers + boot catalog) lives between _end and
# the stack, and the margin is ~2 KB: exe growth past this ceiling
# boot-hangs with "Failed to allocate primitive buffers" (seen twice:
# verbose-schema builds at v0.9.3, release builds on transition-zero).
# If this fails, free static bytes instead of raising the ceiling:
# shave BSS (PS1_DEBUG_PRIM_BYTES history), move text/data to disc
# (see "Move PS1 ... to disc" commits), or rebalance region budgets.
END_ADDR=$(awk '$1=="_end" && $2=="B" {gsub(/^ffffffff/, "", $3); print toupper($3)}' build-ps1/jcreborn.map | head -1)
# Ceiling tracks the fattest flavor (verbose perf schema, _end
# 0x80075250 boot-validated 2026-06-09) + 1 KB slack.
END_CEILING="80075650"
if [ -n "$END_ADDR" ]; then
    if [ $((16#$END_ADDR)) -gt $((16#$END_CEILING)) ]; then
        echo "ERROR: static image _end 0x$END_ADDR exceeds ceiling 0x$END_CEILING" >&2
        echo "       (libc heap headroom cliff — see guard comment in build-ps1.sh)" >&2
        exit 1
    fi
    echo "_end 0x$END_ADDR (ceiling 0x$END_CEILING)"
else
    echo "WARN: _end symbol not found in map; ceiling guard skipped" >&2
fi

exit 0
