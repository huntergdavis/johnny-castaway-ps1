---
layout: page
title: Build & toolchain
eyebrow: Build
subtitle: PSn00bSDK 0.24 in Docker, cmake, mkpsxiso, and the wrapper script that ties it together.
description: How to build the Johnny Castaway PS1 disc image — PSn00bSDK 0.24 toolchain, the Docker dev image, cmake configuration, and the mkpsxiso CD pipeline.
---

A labor of love by Hunter Davis. This page documents the build pipeline that
turns the source tree into `jcreborn.bin` + `jcreborn.cue`. The toolchain is
[PSn00bSDK]({{ '/docs/glossary/#psn00bsdk' | relative_url }}) 0.24 + a MIPS cross-compiler, both of which live inside a
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
Configuration always points at the PSn00bSDK toolchain file:

```bash
cmake -B build-ps1 -S . \
    -DCMAKE_TOOLCHAIN_FILE=/opt/psn00bsdk/lib/libpsn00b/cmake/toolchain.cmake
```

Sources split into two groups in `CMakeLists.txt`. The core engine list
is portable C — it compiles for both the host capture binary and the PS1.
The PS1-specific list adds the PSn00bSDK adapter modules:

```cmake
set(CORE_SOURCES
    jc_reborn.c resource.c uncompress.c ttm.c ads.c story.c
    walk.c calcpath.c island.c utils.c config.c bench.c)

set(PS1_SOURCES
    graphics_ps1.c sound_ps1.c events_ps1.c cdrom_ps1.c ps1_stubs.c)

add_executable(jcreborn ${CORE_SOURCES} ${PS1_SOURCES})
```

Linked PSn00bSDK libraries:

| Library    | Purpose                                |
|------------|----------------------------------------|
| `psxgpu`   | GPU primitives, OT, VRAM upload        |
| `psxcd`    | CD-ROM access (`CdRead`, `CdSearchFile`) |
| `psxspu`   | SPU init + voice keys for sound        |
| `psxapi`   | System / kernel calls                  |
| `psxgte`   | Geometry transformation engine         |
| `psxsio`   | Serial I/O (TTY breadcrumbs)           |
| `psxpress` | CD compression/decompression utilities |

Link order matters because they're static archives. `psxgpu` first;
`psxpress` last; everything else in the middle. See
`CMakeLists.txt` for the canonical order.

The compiler flags inherited from PSn00bSDK's toolchain file are
`-msoft-float -G0 -march=mips1 -mabi=32 -ffreestanding`. The project layers
its own `-O2 -Wall -Wpedantic` on top.

## The end-to-end build, by hand

```bash
# Configure
docker run --rm --platform linux/amd64 \
    -v "$PWD":/project jc-reborn-ps1-dev:amd64 \
    bash -c "cd /project/build-ps1 && \
             cmake -DCMAKE_TOOLCHAIN_FILE=/opt/psn00bsdk/lib/libpsn00b/cmake/toolchain.cmake .."

# Compile (produces build-ps1/jcreborn.elf and jcreborn.exe)
docker run --rm --platform linux/amd64 \
    -v "$PWD":/project jc-reborn-ps1-dev:amd64 \
    bash -c "cd /project/build-ps1 && make"

# Author the CD image
docker run --rm --platform linux/amd64 \
    -v "$PWD":/project jc-reborn-ps1-dev:amd64 \
    bash -c "cd /project && mkpsxiso cd_layout.xml"
```

The post-build step inside CMake runs `elf2x -q jcreborn jcreborn.exe`,
which is what `mkpsxiso` actually packages onto the disc. The `.elf` is
useful for symbol lookups when debugging.

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

The clean build path inside `scripts/rebuild-and-let-run.sh`:

```bash
docker run --rm --platform linux/amd64 -v "$PWD":/project \
    jc-reborn-ps1-dev:amd64 bash -c "cd /project && rm -rf build-ps1"

docker run --rm --platform linux/amd64 -v "$PWD":/project \
    jc-reborn-ps1-dev:amd64 bash -c "cd /project && mkdir build-ps1 && cd build-ps1 && \
        cmake -DCMAKE_TOOLCHAIN_FILE=/opt/psn00bsdk/lib/libpsn00b/cmake/toolchain.cmake .. && make"

docker run --rm --platform linux/amd64 -v "$PWD":/project \
    jc-reborn-ps1-dev:amd64 bash -c "cd /project && mkpsxiso cd_layout.xml"
```

`make-cd-image.sh` is the third step on its own and is fast enough to run
between scene edits.

## Common breakages

**"Could not find toolchain file"** — PSn00bSDK didn't install correctly in
the container. Confirm `/opt/psn00bsdk/lib/libpsn00b/cmake/toolchain.cmake`
exists in the image:

```bash
docker run --rm jc-reborn-ps1-dev:amd64 \
    ls /opt/psn00bsdk/lib/libpsn00b/cmake/toolchain.cmake
```

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
  · [`scripts/build-docker-image.sh`]({{ site.github_url }}/blob/main/scripts/build-docker-image.sh)
  — the wrapper scripts the body walks the reader through.
- [`docs/ps1/build-system.md`]({{ site.github_url }}/blob/main/docs/ps1/build-system.md)
- [`docs/ps1/toolchain-setup.md`]({{ site.github_url }}/blob/main/docs/ps1/toolchain-setup.md)
- [`config/ps1/Dockerfile.ps1`]({{ site.github_url }}/blob/main/config/ps1/Dockerfile.ps1)
- [`CMakeLists.txt`]({{ site.github_url }}/blob/main/CMakeLists.txt)
