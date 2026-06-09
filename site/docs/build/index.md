---
layout: page
title: Build & toolchain
eyebrow: Build
subtitle: PSn00bSDK 0.24 in Docker, cmake, mkpsxiso, and the wrapper script that ties it together.
description: How to build the Johnny Castaway PS1 disc image — PSn00bSDK 0.24 toolchain, the Docker dev image, cmake configuration, and the mkpsxiso CD pipeline.
---

A labor of love by Hunter Davis. This page documents the build pipeline that
turns the source tree into `jcreborn.bin` + `jcreborn.cue`. The toolchain is
[PSn00bSDK]({{ '/docs/glossary/#psn00bsdk' | relative_url }}) 0.24 + a [MIPS]({{ '/docs/glossary/#mips' | relative_url }}) cross-compiler, both of which live inside a
reproducible Docker image so the build is identical on Linux, Intel macOS,
Apple Silicon (via Rosetta 2), and Windows (via WSL2). The native macOS
toolchain attempt was abandoned because the precompiled binaries from
psx.arthus.net were missing `cc1` / `cc1plus` and were built for macOS 10.15.
Docker won by default.

If you paid for this, you were cheated. Open source and free.

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

## What you need

- **Docker Desktop** (or rootless Docker on Linux). The build never runs
  natively on the host.
- **Git**, to clone the repository.
- **[DuckStation]({{ '/docs/glossary/#duckstation' | relative_url }})** for testing — the regtest image carries its own copy, but
  for live development you launch the desktop emulator yourself. Get a real
  PS1 BIOS file separately; the project does not redistribute one.

## The fast path

```bash
# 1. Clone
git clone https://github.com/huntergdavis/johnny-castaway-ps1.git
cd johnny-castaway-ps1

# 2. Build the dev Docker image (one-time, ~5 min)
./scripts/build-docker-image.sh

# 3. Build the PS1 executable + CD image + launch DuckStation
./scripts/rebuild-and-let-run.sh

# 4. Boot it
#    Open DuckStation, File → Start File…, point at jcreborn.cue (NOT .bin).
```

Output is `jcreborn.bin` + `jcreborn.cue` in the repo root. At
`{{ site.release.tag }}` the CD image is about **76 MB** — that
weight is almost all foreground (FG2) packs routed onto the disc;
the [PS-EXE]({{ '/docs/glossary/#ps-exe' | relative_url }}) itself is **208 KiB** (104 × 2 KiB CD-ROM sectors). If
you want the per-scene loop instead
of a one-shot build, read
[Development workflow]({{ '/docs/dev-workflow/' | relative_url }}).

## The Docker image

`config/ps1/Dockerfile.ps1` is built `--platform linux/amd64` so it works on every host
architecture the author owns. It's based on Ubuntu 22.04 and installs:

| Component                       | Version     | Source |
|---------------------------------|-------------|--------|
| PSn00bSDK                       | 0.24        | github.com/Lameguy64/PSn00bSDK |
| `mipsel-none-elf-gcc`           | 12.3.0      | github.com/Lameguy64/PSn00bSDK releases |
| `mkpsxiso` (CD authoring)       | bundled     | github.com/Lameguy64/mkpsxiso |
| `elf2x` (ELF → PS-EXE)          | bundled     | shipped with PSn00bSDK |
| CMake                           | 3.22+       | Ubuntu 22.04 |

Inside the container the toolchain lives at `/opt/psn00bsdk/` with a
`PATH=/opt/psn00bsdk/bin:/opt/mipsel-none-elf/bin:$PATH`. The host source
tree mounts at `/project/`.

## CMake configuration

The PS1 build is a separate CMake invocation from the host capture build.
`CMakeLists.txt` resolves the PSn00bSDK toolchain via the `PSN00BSDK`
environment variable that the Dockerfile sets to
`/opt/psn00bsdk/PSn00bSDK-0.24-Linux`, so the configure command does not
need a `-DCMAKE_TOOLCHAIN_FILE` flag:

```bash
cmake -G "Unix Makefiles" \
    -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY \
    -S /project -B /project/build-ps1
```

Sources are a single `set(SOURCES ...)` list in `CMakeLists.txt` covering
26 translation units — engine core, PS1 adapter modules, the pause-menu /
captions / SPI / memcard / scene-picker / freeplay UI subsystems, the
walk family, and the holiday code-generated tables:

```cmake
set(SOURCES
    src/jc_reborn.c src/core/utils.c src/core/uncompress.c src/resource/resource.c
    src/foreground_pilot/foreground_pilot.c src/platform/ps1/ps1_perf.c src/scene/island.c
    src/graphics_ps1/graphics_ps1.c src/platform/ps1/sound_ps1.c src/platform/ps1/events_ps1.c src/cdrom_ps1.c
    src/platform/ps1/ps1_pad_script.c
    src/platform/ps1/ps1_debug.c src/pause_menu/pause_menu.c src/platform/ps1/ps1_captions.c
    src/platform/ps1/spi.c src/platform/ps1/memcard.c
    src/scene/scene_picker.c src/scene_freeplay/scene_freeplay.c
    src/walk/walk.c src/walk_pilot.c src/walk/walk_render.c src/walk/calcpath.c
    src/scene/holidays.c src/scene/holidays_table.c
    src/platform/ps1/ps1_stubs.c)

psn00bsdk_add_executable(jcreborn GPREL ${SOURCES})

target_compile_options(jcreborn PRIVATE
    -ffunction-sections -fdata-sections)
target_link_options(jcreborn PRIVATE -Wl,--gc-sections)

target_link_libraries(jcreborn PRIVATE
    psxgpu psxgte psxspu psxcd c)
```

The legacy `src/ads/ads.c`, `src/host/ttm.c`, `src/host/story.c`, `src/host/config.c`, and
`src/host/bench.c` source files still exist on disk but are not in the
SOURCES list. The host build references them; the PS1 build doesn't.
Section GC (`--gc-sections`) drops the dead code from cross-references
the active modules still hold.

`GPREL` enables GP-relative addressing, which makes data access faster
on the MIPS R3000A. `-ffunction-sections -fdata-sections` plus
`--gc-sections` lets the linker drop unused engine paths.

Linked PSn00bSDK libraries (link order is the order they appear in the
`target_link_libraries` call above):

| Library  | Purpose                                  |
|----------|------------------------------------------|
| `psxgpu` | GPU primitives, OT, VRAM upload          |
| `psxgte` | Geometry transformation engine           |
| `psxspu` | SPU init + voice keys for sound          |
| `psxcd`  | CD-ROM access (`CdRead`, `CdSearchFile`) |
| `c`      | C runtime (libc + PSn00bSDK glue)        |

The compiler flags inherited from the PSn00bSDK `sdk` cmake module are
`-msoft-float -G0 -march=mips1 -mabi=32 -ffreestanding`. The project
layers `-Wall -Wpedantic -DPS1_BUILD -ffreestanding` on top via
`CMAKE_C_FLAGS`.

## The end-to-end build, by hand

```bash
# Configure (PSN00BSDK env var set in the Docker image; no -DCMAKE_TOOLCHAIN_FILE needed)
docker run --rm --platform linux/amd64 \
    -v "$PWD":/project jc-reborn-ps1-dev:amd64 \
    bash -c "cmake -G 'Unix Makefiles' \
                  -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY \
                  -S /project -B /project/build-ps1"

# Compile (produces build-ps1/jcreborn.elf and jcreborn.exe)
docker run --rm --platform linux/amd64 \
    -v "$PWD":/project jc-reborn-ps1-dev:amd64 \
    bash -c "cmake --build /project/build-ps1 --target jcreborn"

# Author the CD image
docker run --rm --platform linux/amd64 \
    -v "$PWD":/project jc-reborn-ps1-dev:amd64 \
    bash -c "mkpsxiso -y /project/config/ps1/cd_layout.xml"
```

The post-build step inside CMake runs `elf2x -q jcreborn.elf
jcreborn.exe`, which is what `mkpsxiso` actually packages onto the disc.
The `.elf` is useful for symbol lookups when debugging.

## CD layout

`cd_layout.xml` describes the ISO9660 tree that `mkpsxiso` writes:

```xml
<iso_project image_name="jcreborn.bin" cue_sheet="jcreborn.cue">
  <track type="data">
    <directory_tree>
      <file name="JCREBORN.EXE" source="build-ps1/jcreborn.exe"/>
      <file name="RESOURCE.MAP" source="jc_resources/RESOURCE.MAP"/>
      <file name="RESOURCE.001" source="jc_resources/RESOURCE.001"/>
      <!-- per-scene .FG2 packs added here as scenes get wired -->
    </directory_tree>
  </track>
</iso_project>
```

Boot configuration lives in `SYSTEM.CNF`:

```text
BOOT  = cdrom:\JCREBORN.EXE;1
TCB   = 4
EVENT = 10
STACK = 801FFF00
```

Per-scene `.FG2` packs are added under `config/ps1/cd_layout.xml` as scenes
get wired in — see [Development workflow]({{ '/docs/dev-workflow/' | relative_url }})
for the routing step.

## The wrapper scripts

| Script                           | What it does |
|----------------------------------|--------------|
| `scripts/rebuild-and-let-run.sh` | Rebuild executable + CD + launch DuckStation with a temporary TTY-logging config. The "make me a fresh disc and run it" button; the day-to-day scene-work entry. |
| `scripts/build-ps1.sh`           | Incremental executable build only. No CD authoring. |
| `scripts/make-cd-image.sh`       | Re-run `mkpsxiso` against the current `build-ps1/jcreborn.exe`. Faster than a full rebuild when only the layout XML changed. |
| `scripts/build-docker-image.sh`  | Build the dev Docker image from `config/ps1/Dockerfile.ps1`. Run once after clone, then again when the Dockerfile changes. |

What `scripts/rebuild-and-let-run.sh` ultimately invokes is two
container shells. The first is `scripts/build-ps1.sh` (executable +
ELF), the second is `scripts/make-cd-image.sh` (CD image). Stripped of
the wrapper plumbing, the commands inside each container are:

```bash
# scripts/build-ps1.sh — clean rebuild of jcreborn.exe
docker run --rm --platform linux/amd64 -v "$PWD":/project \
    jc-reborn-ps1-dev:amd64 bash -c "rm -rf /project/build-ps1"

docker run --rm --platform linux/amd64 -v "$PWD":/project \
    jc-reborn-ps1-dev:amd64 bash -c "
        cmake -G 'Unix Makefiles' \
            -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY \
            -S /project -B /project/build-ps1 && \
        cmake --build /project/build-ps1"

# scripts/make-cd-image.sh — bundle the build + assets into jcreborn.bin/.cue
docker run --rm --platform linux/amd64 -v "$PWD":/project \
    jc-reborn-ps1-dev:amd64 bash -c "
        cd /project && \
        mkpsxiso -y /project/config/ps1/cd_layout.xml"
```

The cmake invocation does not pass `-DCMAKE_TOOLCHAIN_FILE` because
`CMakeLists.txt` resolves the PSn00bSDK toolchain via the
`PSN00BSDK` environment variable that the Dockerfile sets to
`/opt/psn00bsdk/PSn00bSDK-0.24-Linux`. The `mkpsxiso` invocation needs
the `-y` flag (overwrite) and the full path to `cd_layout.xml`
because the layout file lives under `config/ps1/`, not the project
root.

`make-cd-image.sh` is the third step on its own and is fast enough to
run between scene edits.

## Common breakages

**"Could not find toolchain file"** or **"PSN00BSDK environment
variable not set"** — PSn00bSDK didn't install correctly in the
container. Confirm both the env var and the SDK cmake module exist
in the image:

```bash
docker run --rm jc-reborn-ps1-dev:amd64 \
    bash -c 'echo "$PSN00BSDK" && ls "$PSN00BSDK/lib/libpsn00b/cmake/sdk.cmake"'
```

Expected output: `/opt/psn00bsdk/PSn00bSDK-0.24-Linux` followed by a
listing of `sdk.cmake`.

**"undefined reference to `SpuInit`"** — A new audio path needs `psxspu` in
the link list. Audio code that doesn't link will surface as missing
`SpuSetKey`, `SpuSetVoiceAttr`, etc.

**"File not found: RESOURCE.MAP"** — The CD step is run from the wrong
directory or the resource files are missing. `ls jc_resources/RESOURCE.MAP
jc_resources/RESOURCE.001` should both succeed before `mkpsxiso` runs.

**Permission errors on build outputs** — Never run docker with `sudo`. If
ownership is already wrong, `sudo chown -R $USER:$USER .` once and never
again. Sudo + Docker breaks DuckStation's access to the cue file in
unrelated and surprising ways.

## Verifying a fresh setup

```bash
# Toolchain version check
docker run --rm jc-reborn-ps1-dev:amd64 mipsel-none-elf-gcc --version
# expected: mipsel-none-elf-gcc (GCC) 12.3.0

# Full build smoke test
./scripts/rebuild-and-let-run.sh
ls -lh jcreborn.bin jcreborn.cue
# expected: jcreborn.bin around 76 MB (mostly FG2 pack payload),
#           jcreborn.cue a few hundred bytes
```

If both succeed, the next step is loading `jcreborn.cue` in DuckStation —
or running it through the [headless regtest harness]({{ '/docs/regtest/' | relative_url }}).

## Related pages

- [Development workflow]({{ '/docs/dev-workflow/' | relative_url }}) — what to do
  *with* a fresh build, scene by scene.
- [Build infrastructure]({{ '/docs/infrastructure/' | relative_url }}) —
  the bigger picture: Docker images, regtest container, release
  script, what's still done by hand on each release.
- [Hardware]({{ '/docs/hardware/' | relative_url }}) — the
  PS1 envelope the build is targeting.
- [Regression testing]({{ '/docs/regtest/' | relative_url }}) — booting the disc
  image headlessly for screenshots and TTY capture.
- [API mapping]({{ '/docs/api/' | relative_url }}) — what each PSn00bSDK
  library is replacing.
- [Lab: the 24/7 build farm]({{ '/lab/build-farm/' | relative_url }})
  — the same machinery framed as methodology.

## View source on GitHub

- [`scripts/rebuild-and-let-run.sh`]({{ site.github_url }}/blob/main/scripts/rebuild-and-let-run.sh)
  · [`scripts/build-ps1.sh`]({{ site.github_url }}/blob/main/scripts/build-ps1.sh)
  · [`scripts/make-cd-image.sh`]({{ site.github_url }}/blob/main/scripts/make-cd-image.sh)
  · [`scripts/build-docker-image.sh`]({{ site.github_url }}/blob/main/scripts/build-docker-image.sh)
  — the wrapper scripts the body walks the reader through.
- [`docs/ps1/build-system.md`]({{ site.github_url }}/blob/main/docs/ps1/build-system.md)
- [`docs/ps1/toolchain-setup.md`]({{ site.github_url }}/blob/main/docs/ps1/toolchain-setup.md)
- [`config/ps1/Dockerfile.ps1`]({{ site.github_url }}/blob/main/config/ps1/Dockerfile.ps1)
- [`CMakeLists.txt`]({{ site.github_url }}/blob/main/CMakeLists.txt)
