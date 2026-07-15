#!/usr/bin/env bash
set -euo pipefail

here="$(cd "$(dirname "$0")" && pwd)"
repo_root="$(cd "$here/../../.." && pwd)"
core_dir="$repo_root/vendor/emulatorjs/data/cores"
work="${EJS_CORE_BUILD_DIR:-$repo_root/.build/pcsx-rearmed}"
metadata="$work/metadata"
pcsx="$work/pcsx_rearmed"
retroarch="$work/RetroArch"
package_root="$work/EmulatorJS/data/cores"

pcsx_commit="588e1338f85a5867e46245f861e3d5958e7a4592"
retroarch_commit="d907a125ee76e02a21d89003c140fd0b72886e90"

for tool in emcc emmake embuilder git make 7z sha256sum; do
    command -v "$tool" >/dev/null || {
        echo "missing required tool: $tool" >&2
        exit 1
    }
done

rm -rf "$work"
mkdir -p "$metadata" "$package_root"

7z e -y -o"$metadata" "$core_dir/pcsx_rearmed-wasm.data" \
    build.json core.json license.txt >/dev/null

git clone https://github.com/EmulatorJS/pcsx_rearmed.git "$pcsx"
git -C "$pcsx" checkout "$pcsx_commit"
git -C "$pcsx" apply "$here/pcsx-rearmed.patch"

embuilder build zlib
emmake make -C "$pcsx" -f Makefile.libretro clean
emmake make -C "$pcsx" -j"$(nproc)" -f Makefile.libretro \
    platform=emscripten HAVE_CHD=0 WANT_ZLIB=0

git clone https://github.com/EmulatorJS/RetroArch.git "$retroarch"
git -C "$retroarch" checkout "$retroarch_commit"
git -C "$retroarch" apply "$here/retroarch.patch"
cp "$pcsx/pcsx_rearmed_libretro_emscripten.bc" \
    "$retroarch/emulatorjs/pcsx_rearmed_libretro_emscripten.bc"

(
    cd "$retroarch/emulatorjs"
    emmake ./build-emulatorjs.sh --clean
    emmake ./build-emulatorjs.sh --clean --legacy
)

for package in pcsx_rearmed-wasm.data pcsx_rearmed-legacy-wasm.data; do
    built="$package_root/$package"
    (
        cd "$metadata"
        7z a -t7z "$built" build.json core.json license.txt >/dev/null
    )
    7z t "$built" >/dev/null
    cp "$built" "$core_dir/$package"
done

sha256sum \
    "$core_dir/pcsx_rearmed-wasm.data" \
    "$core_dir/pcsx_rearmed-legacy-wasm.data"
echo "Bump data/cores/reports/pcsx_rearmed.json before serving this build."
