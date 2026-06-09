---
layout: page
title: Build infrastructure & dev environment
eyebrow: Process
subtitle: How a one-person fan port keeps a reproducible cross-compile pipeline together — Docker, CMake, mkpsxiso, and a release script that tags itself.
description: The dev environment for the Johnny Castaway PS1 port — Docker image with PSn00bSDK 0.24, CMake configuration, the mkpsxiso CD pipeline, and the release script that bumps version, tags, and publishes.
---

A labor of love by Hunter Davis. This page is the story of how the PS1 port
keeps its build pipeline reproducible across machines, what the Docker dev
image actually contains, what the wrapper scripts do, and where the
automation stops and the author starts typing things by hand. If you paid
for this, you were cheated. Open source and free.

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

## Why Docker

PSn00bSDK is sensitive to toolchain version. The SDK is a thin layer over a
specific build of `mipsel-none-elf-gcc` and a set of static archives; small
mismatches between the compiler and the SDK headers produce silent runtime
errors that look like emulator bugs rather than build bugs. The author's
first month of the port had two laptops with two slightly different [MIPS]({{ '/docs/glossary/#mips' | relative_url }})
toolchains installed natively, and one of them produced a binary that
crashed inside `SpuInit()` on real hardware. The other one didn't. That was
the start of the problem.

A native macOS toolchain attempt was abandoned earlier than that. The
precompiled binaries from `psx.arthus.net/sdk/mipsel/` (binutils 2.37 and
gcc 11.2) installed cleanly enough that `mipsel-none-elf-gcc --version`
worked, but they were missing `cc1` and `cc1plus` and were built for macOS
10.15 — incompatible with macOS 14.5. The host build couldn't reach a
working compile.

Docker won by default, not by enthusiasm. A Docker dev image pins the
compiler, the SDK, the CD authoring tool, and every Ubuntu apt package the
build chain touches. The image is `--platform linux/amd64` so it runs on
Intel Macs natively, Apple Silicon Macs through Rosetta 2, Linux x86-64
natively, and Windows via WSL2. Once the image landed, "it builds on my
machine" stopped being a question — every machine runs the same Linux
inside Docker, and the host OS only needs a working Docker daemon and a
PS1 BIOS file for testing.

This is not a high-throughput build. A clean Docker rebuild of the PS1
executable plus CD image is around 20 seconds on a recent laptop. The
overhead of running `docker run` per step (CMake configure, make, [mkpsxiso]({{ '/docs/glossary/#mkpsxiso' | relative_url }}))
is real but irrelevant for a one-person workflow. What matters is that
every build is the same build.

## The Docker dev image

The image lives in the repo at
[`config/ps1/Dockerfile.ps1`]({{ site.github_url }}/blob/main/config/ps1/Dockerfile.ps1).
A second image,
[`config/ps1/Dockerfile.regtest`]({{ site.github_url }}/blob/main/config/ps1/Dockerfile.regtest),
adds DuckStation and the headless capture harness on top — that one belongs
to [Regression testing]({{ '/docs/regtest/' | relative_url }}), not here.

The dev image is Ubuntu 22.04 plus:

| Component                       | Version | Source |
|---------------------------------|---------|--------|
| PSn00bSDK                       | 0.24    | `github.com/Lameguy64/PSn00bSDK` |
| `mipsel-none-elf-gcc`           | 12.3.0  | PSn00bSDK release downloads |
| `mkpsxiso` (CD authoring)       | bundled | `github.com/Lameguy64/mkpsxiso` |
| `elf2x` (ELF → PS-EXE)          | bundled | shipped with PSn00bSDK |
| CMake                           | 3.22+   | Ubuntu 22.04 apt |
| `build-essential`, `git`, `wget`, `unzip` | — | Ubuntu apt |

Inside the container the toolchain installs to `/opt/psn00bsdk/` and
`/opt/mipsel-none-elf/`, both prepended to `PATH`. The host source tree
mounts at `/project/`. Build the image once with:

```bash
docker build -f config/ps1/Dockerfile.ps1 \
    -t jc-reborn-ps1-dev:amd64 \
    --platform linux/amd64 .
```

It takes about five minutes the first time and is cached afterwards.

The host capture build — the SDL2-linked native binary that records
foreground frames for the PS1 to replay — does not use this image. It
builds against whatever the host's native compiler and SDL2 happen to be.
That asymmetry is intentional: the host build only has to run on the
author's actual development machine, while the PS1 build has to be
reproducible everywhere.

## The build pipeline end-to-end

From a fresh clone, the path to `jcreborn.bin` + `jcreborn.cue` is:

```bash
# 1. Clone
git clone https://github.com/{{ site.repo }}.git
cd johnny-castaway-ps1

# 2. Build the dev Docker image (one-time, ~5 minutes)
docker build -f config/ps1/Dockerfile.ps1 \
    -t jc-reborn-ps1-dev:amd64 \
    --platform linux/amd64 .

# 3. Build the PS1 executable
./scripts/build-ps1.sh

# 4. Bundle the executable + assets into a CD image
./scripts/make-cd-image.sh

# 5. Boot it
#    Open DuckStation, File → Start File…, point at jcreborn.cue (NOT .bin).
```

The two-step split matters because the second step is fast and can be
re-run after asset edits without rebuilding the executable. The
all-in-one wrapper is `scripts/rebuild-and-let-run.sh`, which calls
both steps and then launches DuckStation.

`scripts/build-ps1.sh` produces `build-ps1/jcreborn.exe` and
`build-ps1/jcreborn.elf`. It runs two Docker invocations in sequence:
a clean of the previous build directory, then a CMake configure plus
`make jcreborn`. CMake resolves the PSn00bSDK toolchain via the
`PSN00BSDK` environment variable that the Dockerfile sets to
`/opt/psn00bsdk/PSn00bSDK-0.24-Linux`.

`scripts/make-cd-image.sh` runs a third Docker invocation: `mkpsxiso
-y /project/config/ps1/cd_layout.xml`. The output lands in the repo
root as `jcreborn.bin` (the CD image) and `jcreborn.cue` (the cue
sheet that tells DuckStation where the data track lives).

The CMake configuration lives in `CMakeLists.txt` at the repo root. The
relevant shape:

```cmake
cmake_minimum_required(VERSION 3.21)

# Find PSn00bSDK first (required before project())
list(APPEND CMAKE_MODULE_PATH "$ENV{PSN00BSDK}/lib/libpsn00b/cmake")
include(sdk)

project(JohnnyReborn LANGUAGES C VERSION 1.0.0)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wpedantic -DPS1_BUILD -ffreestanding")

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

`GPREL` enables GP-relative addressing, which makes data access faster on
the MIPS R3000A. `-ffunction-sections -fdata-sections` plus
`--gc-sections` lets the linker drop unused engine paths — the legacy ADS
and TTM runtime routes were retired from the active build, but the C
sources still reference them, and section GC removes the dead code at
link time.

### Host build vs PS1 build

The repo contains two build trees with different jobs:

- **`build-ps1/`** is the PSn00bSDK cross-compile. Output is
  `jcreborn.elf` (MIPS ELF, used for symbol lookup) and `jcreborn.exe`
  ([PS-EXE]({{ '/docs/glossary/#ps-exe' | relative_url }}) format, what gets packaged onto the disc).
- **`build-host/`** is a native SDL2-linked binary called `jc_reborn-host`.
  It runs the original Sierra engine, captures foreground frames per
  scene, and emits the `.FG2` packs that the PS1 replays. The host build
  is the authoritative renderer; the PS1 is a hybrid replay target. See
  [Method]({{ '/about/method/' | relative_url }}) for the rationale.

Both trees share most C sources (`jc_reborn.c`, `resource.c`,
`uncompress.c`, etc). The host build links against SDL2; the PS1 build
links against PSn00bSDK's `psxgpu` / `psxspu` / `psxcd` and uses the
`*_ps1.c` adapter modules instead.

### The `mkpsxiso` step

After `make` finishes, `scripts/make-cd-image.sh` runs `mkpsxiso` against
`config/ps1/cd_layout.xml`. The XML describes the ISO9660 directory tree:

```xml
<iso_project image_name="jcreborn.bin" cue_sheet="jcreborn.cue">
  <track type="data">
    <directory_tree>
      <file name="JCREBORN.EXE" source="build-ps1/jcreborn.exe"/>
      <file name="RESOURCE.MAP" source="jc_resources/RESOURCE.MAP"/>
      <file name="RESOURCE.001" source="jc_resources/RESOURCE.001"/>
      <!-- Active FG2 packs added per scene as they get wired in -->
    </directory_tree>
  </track>
</iso_project>
```

The boot file is `SYSTEM.CNF`:

```text
BOOT  = cdrom:\JCREBORN.EXE;1
TCB   = 4
EVENT = 10
STACK = 801FFF00
```

Per-scene `.FG2` foreground packs are added as scenes get wired in.
Routing a scene means appending its high/low pack pair to the
`directory_tree` and rebuilding the CD image — `make-cd-image.sh` is
faster than a full executable rebuild and is the day-to-day inner loop.

## The wrapper scripts

The `scripts/` directory contains roughly 155 shell and Python files at
the time of writing. The ones that matter for daily build work:

| Script | Purpose |
|--------|---------|
| `scripts/build-ps1.sh` | Clean rebuild of the PS1 executable (`jcreborn.elf` + `jcreborn.exe`). Does not produce the CD image — `make-cd-image.sh` is the next step. |
| `scripts/make-cd-image.sh` | Re-run `mkpsxiso` against the current `build-ps1/jcreborn.exe`. Faster than a full rebuild when only the layout XML changed. |
| `scripts/rebuild-and-let-run.sh` | Rebuild + CD + launch DuckStation with a temporary TTY-logging config. Day-to-day scene work. |
| `scripts/build-host.sh` | Build the SDL2 capture binary. Independent of the PS1 image. |
| `scripts/release.sh` | Bump VERSION, tag, push, attach assets to a GitHub release. |
| `scripts/build-docker-image.sh` | Wrapper around `docker build` for the dev image. |
| `scripts/build-regtest-image.sh` | Same, for the regtest image. |
| `scripts/run-regtest.sh` | Run the headless DuckStation harness against a built CD image. |

The rest of the directory is scene capture, FG2 pack compilation, vision
artifact pipelines, and host-side analysis scripts — those belong to other
pages.

## The release script

`scripts/release.sh` is what cuts a tagged build. The flow:

1. Refuses to run as root. Sudo + Docker corrupts file ownership.
2. Reads the current version from the `VERSION` file at the repo root.
3. Increments the patch component (e.g. `0.3.9` → `0.3.10`).
4. Constructs the tag name as `v<version>-ps1`.
5. Aborts if the tag already exists.
6. Runs `scripts/build-ps1.sh clean` and then `scripts/make-cd-image.sh`.
   No emulator launch — release builds are headless.
7. Copies `jcreborn.bin` and `jcreborn.cue` into `release/`.
8. Updates the `VERSION` file plus `site/_config.yml` release metadata.
9. Rebuilds the portable website into `www/` and runs the relative-link
   red-team.
10. Commits the bump, release artifacts, website source metadata, and
   generated website output together.
11. Creates an *orphan-tree tag commit* whose tree contains only the two
   release files. This is so GitHub's auto-generated "Source code (zip)"
   download for the tag is the disc image, not the entire repo. The tag
   commit's parent is the main release commit, so provenance is
   preserved through `git log --all`.
12. Pushes the branch and the tag.
13. If `gh` is installed and authenticated, also publishes a GitHub
    Release with `jcreborn.bin` and `jcreborn.cue` as direct-download
    assets.

The website does not need a second hand-edit step. Cutting a release with
this script updates the release tag in the Jekyll config, rebuilds the
checked-in `www/` archive, and verifies that the generated site still uses
relative links for project-page deployment.

The current release is **{{ site.release.tag }}**, with
{{ site.release.scenes_validated }} of {{ site.release.scenes_total }}
scenes validated under the human-signoff bar. See the
[scene ledger]({{ '/scenes/' | relative_url }}) for which scenes those are.

## Where this gets tested

The build pipeline produces a CD image. Testing that image happens in two
places: live in DuckStation on the developer's machine, and headlessly via
the Docker-packaged regtest harness. The regtest harness boots the same
`jcreborn.cue`, runs scenes for a configurable number of frames, captures
PNGs every *N* frames, and reads the on-screen telemetry overlay to
identify which scene the runtime believes it is rendering. That's the
surface the perf experiments and vision-classifier work both run against.

This page does not reproduce the regtest content. See
[Regression testing]({{ '/docs/regtest/' | relative_url }}) for the
harness, and [Vision-classifier work]({{ '/docs/vision/' | relative_url }})
for the pixel-vs-reference comparison layer that runs on top of it.

## What's not automated yet

The release flow is automated end-to-end, but several scene-bring-up steps
the author still does by hand on each release:

- **Routing new FG2 packs into `cd_layout.xml`.** When a scene gets
  promoted from bring-up to validated, its high/low FG2 pair gets added
  to the layout XML and the project structure manually. There is no
  scene-promotion script that does this in one step.
- **Visual sign-off.** The acceptance bar for a scene is human visual +
  audible review on the playback path. The vision classifier and the
  pixel-diff regtest both produce useful signal, but neither one signs
  off a scene. The author watches it.
- **DuckStation TTY config.** `rebuild-and-let-run.sh` enables TTY
  logging for the run by editing the DuckStation config in place and
  reverting it after. This is fine but fragile; if DuckStation's config
  schema changes, the script breaks silently.
- **CD image asset pruning.** The 9.9 MB CD image carries only the FG2
  packs for currently-validated scenes plus minimal SCR/PSB/SND assets.
  When scenes get retired or restructured, deciding what stays on the
  disc is a manual diff against `cd_layout.xml`.
- **Hardware burn-in.** Real-hardware testing happens on the author's
  PS1 when something looks wrong on emulator and the question is "is
  this DuckStation HLE diverging?" There is no automated hardware
  test rig.

The pipeline does what it needs to. It does not need to do more.

## Common breakages

**"Could not find toolchain file"** — PSn00bSDK didn't install correctly
in the container. Confirm
`/opt/psn00bsdk/lib/libpsn00b/cmake/toolchain.cmake` exists in the image:

```bash
docker run --rm jc-reborn-ps1-dev:amd64 \
    ls /opt/psn00bsdk/lib/libpsn00b/cmake/toolchain.cmake
```

**"undefined reference to `SpuInit`"** — A new audio path needs `psxspu`
in the link list. Audio code that does not link will surface as missing
`SpuSetKey`, `SpuSetVoiceAttr`, etc.

**Permission errors on build outputs** — Never run docker with `sudo`. If
ownership is already wrong, `sudo chown -R $USER:$USER .` once and never
again. Sudo + Docker corrupts permissions in surprising ways and breaks
DuckStation's access to `jcreborn.cue`.

## Related pages

- [Build & toolchain]({{ '/docs/build/' | relative_url }}) — the
  pipeline as a quick-reference.
- [Hardware]({{ '/docs/hardware/' | relative_url }}) — what the build
  output is targeting.
- [Performance work]({{ '/docs/performance/' | relative_url }}) — the
  experiment ledger that runs against this build.
- [Regression testing]({{ '/docs/regtest/' | relative_url }}) — the
  headless harness that boots the disc image.
- [Lab: the 24/7 build farm]({{ '/lab/build-farm/' | relative_url }})
  — the same machinery framed as a methodology essay.
- [Lab: the dunking bird]({{ '/lab/dunking-bird/' | relative_url }})
  — the parallel-agent infrastructure that drives the build farm
  between human review passes.
- [Method]({{ '/about/method/' | relative_url }}) — how the project
  decides what's worth automating.
- [Devlog]({{ '/devlog/' | relative_url }}) — day-by-day worklog.

## View source on GitHub

- [`docs/ps1/build-system.md`]({{ site.github_url }}/blob/main/docs/ps1/build-system.md)
- [`docs/ps1/toolchain-setup.md`]({{ site.github_url }}/blob/main/docs/ps1/toolchain-setup.md)
- [`config/ps1/Dockerfile.ps1`]({{ site.github_url }}/blob/main/config/ps1/Dockerfile.ps1)
- [`config/ps1/Dockerfile.regtest`]({{ site.github_url }}/blob/main/config/ps1/Dockerfile.regtest)
- [`CMakeLists.txt`]({{ site.github_url }}/blob/main/CMakeLists.txt)
- [`scripts/build-ps1.sh`]({{ site.github_url }}/blob/main/scripts/build-ps1.sh)
  · [`scripts/make-cd-image.sh`]({{ site.github_url }}/blob/main/scripts/make-cd-image.sh)
  · [`scripts/rebuild-and-let-run.sh`]({{ site.github_url }}/blob/main/scripts/rebuild-and-let-run.sh)
  — the day-to-day build pipeline (executable, CD image, all-in-one
  wrapper).
- [`scripts/build-host.sh`]({{ site.github_url }}/blob/main/scripts/build-host.sh)
  — separate SDL2 host capture binary.
- [`scripts/build-docker-image.sh`]({{ site.github_url }}/blob/main/scripts/build-docker-image.sh)
  · [`scripts/build-regtest-image.sh`]({{ site.github_url }}/blob/main/scripts/build-regtest-image.sh)
  — the two `docker build` wrappers (dev image + regtest image).
- [`scripts/run-regtest.sh`]({{ site.github_url }}/blob/main/scripts/run-regtest.sh)
  — the headless DuckStation harness runner.
- [`scripts/release.sh`]({{ site.github_url }}/blob/main/scripts/release.sh)
  — full release flow (bump VERSION, tag, push, attach assets).
