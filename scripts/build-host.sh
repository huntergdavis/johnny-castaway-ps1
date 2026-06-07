#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build-host"
BIN="$BUILD_DIR/jc_reborn"
COMPAT_LINK="$PROJECT_ROOT/jc_reborn-host"

CC="${CC:-cc}"
CFLAGS="${CFLAGS:-}"
LDFLAGS="${LDFLAGS:-}"

detect_sdl() {
    if command -v sdl2-config >/dev/null 2>&1; then
        SDL_CFLAGS="$(sdl2-config --cflags | sed 's|/include/SDL2|/include|g')"
        SDL_LIBS="$(sdl2-config --libs)"
        return 0
    fi

    if command -v pkg-config >/dev/null 2>&1 && pkg-config --exists sdl2; then
        SDL_CFLAGS="$(pkg-config --cflags sdl2)"
        SDL_LIBS="$(pkg-config --libs sdl2)"
        return 0
    fi

    cat >&2 <<'EOF'
ERROR: Could not find SDL2 build flags.

Supported detection methods:
- sdl2-config
- pkg-config sdl2

Install SDL2 development headers/tools first, then retry.
Examples:
- Ubuntu/Debian: sudo apt-get install libsdl2-dev pkg-config
- Fedora: sudo dnf install SDL2-devel pkgconf-pkg-config
- macOS: brew install sdl2
EOF
    exit 2
}

detect_sdl

mkdir -p "$BUILD_DIR"

SOURCES=(
    src/jc_reborn.c
    src/core/utils.c
    src/scene/holidays.c
    src/scene/holidays_table.c
    src/core/uncompress.c
    src/resource/resource.c
    src/host/dump.c
    src/host/story.c
    src/walk/walk.c
    src/walk/calcpath.c
    src/walk/walk_render.c
    src/ads/ads.c
    src/foreground_pilot/foreground_pilot.c
    src/host/ttm.c
    src/scene/island.c
    src/host/bench.c
    src/host/graphics.c
    src/host/sound.c
    src/host/events.c
    src/host/config.c
    src/scene_freeplay/scene_freeplay.c
)

pushd "$PROJECT_ROOT" >/dev/null
"$CC" \
    -Wall -Wpedantic -std=c99 -O2 \
    -Isrc \
    -Isrc/ads \
    -Isrc/core \
    -Isrc/foreground_pilot \
    -Isrc/graphics_ps1 \
    -Isrc/host \
    -Isrc/mem_region \
    -Isrc/pause_menu \
    -Isrc/platform/ps1 \
    -Isrc/ps1_features \
    -Isrc/resource \
    -Isrc/scene \
    -Isrc/scene_freeplay \
    -Isrc/walk \
    -I. \
    $SDL_CFLAGS \
    $CFLAGS \
    "${SOURCES[@]}" \
    -o "$BIN" \
    $SDL_LIBS \
    $LDFLAGS
popd >/dev/null

echo "Built host binary: $BIN"
ln -sfn "build-host/jc_reborn" "$COMPAT_LINK"
echo "Updated compatibility link: $COMPAT_LINK"
