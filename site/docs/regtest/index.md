---
layout: page
title: Regression testing
eyebrow: Reference
subtitle: Headless DuckStation in Docker, frame PNGs, SHA256 state hashes, and the per-scene wrapper.
description: The PS1 port's headless regtest harness — duckstation-regtest in Docker, frame capture, telemetry decode, scene-scoped runners, and how to add a scene to the manifest.
---

A labor of love by Hunter Davis. The regtest harness runs a PS1 disc image
through DuckStation's headless `duckstation-regtest` binary inside Docker,
captures frame PNGs at a configurable interval, SHA256-hashes RAM / VRAM /
SPU RAM / save state, and forwards PS1 `printf()` output to stdout. There
is no display server; it works on any Linux box with Docker. Wall-clock
speed is ~520 FPS — about 8× realtime — so 30 seconds of PS1 gameplay
captures in roughly 3.6 seconds.

The regtest harness is **secondary tooling** in the current rollout. The
primary acceptance bar is human visual + audible signoff on the
scene-playback ([fgpilot]({{ '/docs/glossary/#fgpilot' | relative_url }})) path — see
[Development workflow]({{ '/docs/dev-workflow/' | relative_url }}). Regtest
is retained for targeted diagnostics: regression hunts, deterministic
frame-timing investigations, and per-scene snapshot generation when a
human review needs frozen artifacts. It is not the gate; it is a
diagnostic.

The same headless DuckStation pipeline, run in `perf-log` mode against
a deterministic boot, drives the
[headless-perf battle card]({{ '/perf/' | relative_url }}) — the
second ledger that lives alongside [visual signoff]({{ '/scenes/' | relative_url }}).
Regtest proves *it boots and renders something*; the perf matrix
proves *it hits target rate*. Different bars; same harness. The
reference manual for the perf work is at
[/docs/performance/]({{ '/docs/performance/' | relative_url }}).

If you paid for this, you were cheated. Open source and free.

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

## What it captures

Every regtest run produces, under `regtest-results/<run-id>/`:

| Artifact            | What |
|---------------------|------|
| `regtest.log`       | Full stdout + stderr from the run, including DuckStation host messages and PS1 guest TTY. |
| `tty-output.txt`    | PS1 `printf()` lines only, extracted from the log. |
| `frames/frame_NNNNN.png` | Captured screenshot every `--dumpinterval` frames. |
| State hashes        | SHA256 of save state, main RAM (2 MB), SPU RAM, and VRAM, logged at completion. |
| `telemetry.json`    | When the on-screen telemetry overlay is enabled, decoded by `scripts/decode-ps1-bars.py`. |

The state hashes are the deterministic regression-detection surface: same
disc + same BIOS + same frame count → same hashes, every time.

## Setup

### Prerequisites

- Docker (rootless, or your user in the `docker` group).
- A real PS1 BIOS file — typically `scph1001.bin`.
- A built CD image (`jcreborn.cue` + `jcreborn.bin`). See
  [Build & toolchain]({{ '/docs/build/' | relative_url }}).

### Build the regtest Docker image

```bash
./scripts/build-regtest-image.sh
```

This is a multi-stage Docker build that compiles DuckStation from source
(Ubuntu 24.04, clang-18, `BUILD_REGTEST=ON`), then extracts the headless
binary plus its prebuilt shared libs into a slim runtime image. First
build is 15–30 minutes; rebuilds are fully cached. Image tag:
`jc-reborn-regtest:latest` (~363 MB).

To force a clean rebuild:

```bash
./scripts/build-regtest-image.sh --no-cache
```

### BIOS auto-detection

The wrapper script auto-detects BIOS files in the common locations:

- `~/.local/share/duckstation/bios/`
- `~/.config/duckstation/bios/`
- `~/.var/app/org.duckstation.DuckStation/config/duckstation/bios/`
- `~/ps1-bios/`
- `./bios/`

Or specify explicitly with `--bios /path/to/bios/`.

## Running tests

### Scripted controller input

`v0.5.x` adds an opt-in pad-script layer for headless menu and flow tests.
The canonical runbook is [Scripted input harness]({{ '/docs/scripted-input/' | relative_url }}).
The PS1 build embeds `config/ps1/PADSCRIPT.TXT` at compile time and only
uses it when [`BOOTMODE.TXT`]({{ '/docs/glossary/#bootmode' | relative_url }}) includes `pad-script` or `pad-script-log`.
Scripted buttons are merged into the same active-high pad mask as the real
controller, after analog-stick folding, so the pause menu and Freeplay code
do not know whether a human or a test script pressed Start.

The menu documentation harness is the first user:

```bash
./scripts/ps1-menu-input-harness.sh
```

It stages a temporary boot mode, waits 30 seconds, presses Start, walks the
major pause-menu screens, emits delayed
[`JCPADSHOT label=<name> frame=<n> tick=<n>`]({{ '/docs/glossary/#jcpadshot' | relative_url }}) markers, runs DuckStation
regtest headlessly, copies the first captured PNG at or after each marker
into `site/assets/img/help/menu/`, and rewrites
[Menu help guide]({{ '/help/menu/' | relative_url }}). The staged boot files
are restored before the script exits.

Pad-script commands are deliberately small:

```text
wait 30s
tap START
tap DOWN
hold R1+RIGHT 12
shot pause-main 30
```

Durations are frames by default; a trailing `s` means seconds at 60 Hz.
The optional number after `shot` is the settle delay; `shot pause-main 30`
marks the screenshot point about half a second after the preceding input.
The menu-guide harness uses a more conservative default settle window
because this pause-menu path has real framebuffer and polling latency, but
`--settle-frames 30` is available for targeted timing diagnostics.
`pad-script-log` prints parsed events for debugging. Plain `pad-script`
keeps the extra logs off while still printing screenshot markers; the menu
harness defaults to the quiet path and accepts `--verbose` when the route
itself needs debugging.

Use this when the question is input-driven rather than scene-driven: Start
opening the pause menu, Circle backing out, Freeplay enter/exit recovery,
world-option menu changes, or a bug report that can be written as button
presses.

### Single scene

```bash
./scripts/regtest-scene.sh --scene "STAND 2"
./scripts/regtest-scene.sh --scene "BUILDING 1" --frames 9000 --interval 120
```

`regtest-scene.sh` is the high-level wrapper. It:

1. Looks up the scene in
   [`config/ps1/regtest-scenes.txt`]({{ site.github_url }}/blob/main/config/ps1/regtest-scenes.txt).
2. Rebuilds the CD image with the canonical boot route for that scene
   set in `BOOTMODE.TXT`.
3. Runs the headless harness with reviewed scene window defaults.
4. Decodes the telemetry overlay if it's present in the captured frames.
5. Writes a structured `result.json` describing the outcome.

### Raw headless run

```bash
./scripts/run-regtest.sh \
    --frames 3600 \
    --start-frame 2400 \
    --dumpinterval 60 \
    --dumpdir scratch/regtest-out
```

`--start-frame N` filters the dumped frames so only frames at or after
frame `N` are kept — useful for skipping the BIOS chime and title screen
and only reviewing the actual scene window. The wrapper materializes a
`filtered-frames/` directory with the kept set.

### Direct Docker invocation

```bash
docker run --rm \
    -v "$PWD":/game:ro \
    -v ~/.var/app/org.duckstation.DuckStation/config/duckstation/bios:/root/.local/share/duckstation/bios:ro \
    -v $HOME/scratch/regtest-out:/output \
    --entrypoint duckstation-regtest \
    jc-reborn-regtest:latest \
    -renderer Software -console -frames 3600 -dumpdir /output -dumpinterval 60 \
    -- /game/jcreborn.cue
```

## Command-line options

`run-regtest.sh` flags:

| Option              | Default           | Description |
|---------------------|-------------------|-------------|
| `--frames N`        | 1800              | Total frames to execute (60 fps). |
| `--start-frame N`   | 0                 | Keep only dumped frames at or after frame N. |
| `--dumpinterval N`  | 60                | Capture a frame every N frames. |
| `--dumpdir DIR`     | `regtest-results/` | Output root directory. |
| `--cue FILE`        | auto-detect       | Path to `.cue` file. |
| `--bios DIR`        | auto-detect       | Directory containing the PS1 BIOS. |
| `--renderer NAME`   | Software          | Software / Vulkan / OpenGL. |
| `--log LEVEL`       | Info              | Error / Warning / Info / Verbose / Debug. |
| `--timeout SECS`    | 120               | Wall-clock timeout. Kills runaway runs. |
| `--upscale N`       | (native)          | Resolution multiplier. |
| `--cpu MODE`        | (default)         | Interpreter / CachedInterpreter / Recompiler. |

Defaults are sourced from
[`config/ps1/regtest-config.sh`]({{ site.github_url }}/blob/main/config/ps1/regtest-config.sh):

```bash
REGTEST_FRAMES=1800
REGTEST_INTERVAL=60
REGTEST_OUTPUT_DIR=regtest-results
REGTEST_TIMEOUT=120
REGTEST_PARALLEL=4
```

## Scene routing

The scene-routing model is "exact story-scene entry preferred":

```
story scene <index>
```

Raw `island ads <ADS> <tag>` boots are no longer the primary truth path —
they can reach bootstrap or ocean states that are not valid certification
routes. For fgpilot scenes, the harness sets `BOOTMODE.TXT` to:

```
fgpilot <slug> [tokens...]
```

[`config/ps1/regtest-scenes.txt`]({{ site.github_url }}/blob/main/config/ps1/regtest-scenes.txt)
records every scene as:

```
ADS_NAME TAG SCENE_INDEX STATUS BOOTMODE...
```

Status tokens: `verified`, `bringup`, `blocked`, `untested`. These
reflect the **legacy regtest-route** status model and are not the same
thing as the current scene-playback acceptance bar — see
[`scene-status.md`]({{ site.github_url }}/blob/main/docs/ps1/scene-status.md)
for the current per-scene ledger.

### Adding a scene to the harness

1. Append a row to `config/ps1/regtest-scenes.txt` in the canonical form
   above. The boot route should match the scene's actual playback path
   under the current rollout — typically `fgpilot <slug>` for scenes that
   ship as `.FG2` packs.
2. If the scene needs specific variant tokens for its truth window
   (e.g. `night 1`, `lowtide 1`), include them after the slug.
3. Run `./scripts/regtest-scene.sh --scene "<ADS> <tag>"` once and confirm
   the captured frames + TTY look right.

## Overlay-backed character checks

For PS1 bug fixing, the preferred screenshot harness path is:

1. Run a headless regtest with `capture-overlay`.
2. Take one dumped PNG from the run.
3. Decode the embedded overlay into character truth.
4. Compare against expected truth.
5. Open the generated HTML diff report.

One-command path:

```bash
./scripts/capture-and-check-ps1.sh \
    --expected-root host-script-review/fishing1 \
    --scene "FISHING 1" \
    --frame-number 80 \
    --actual-frame 1200
```

`--frame-number` chooses the expected truth frame; `--actual-frame`
chooses the dumped `frame_NNNNN.png` to compare against it.
`check-character-screenshot.py` prefers the frame number embedded in the
overlay packet itself, so DuckStation timestamped filenames don't need
manual frame numbering.

Headless manual capture:

```bash
./scripts/regtest-scene.sh \
    --scene "FISHING 1" \
    --overlay \
    --overlay-mask
```

Single-screenshot check against a captured screenshot:

```bash
python3 scripts/check-character-screenshot.py \
    --image ~/.var/app/org.duckstation.DuckStation/config/duckstation/screenshots/<shot>.png \
    --expected-root host-script-review/fishing1 \
    --out-dir scratch/ps1-character-check
```

## Frame-hash regression detection

To compare two builds:

```bash
md5sum regtest-results/baseline/frames/*.png > scratch/baseline.md5
cd regtest-results/candidate/frames/
md5sum -c scratch/baseline.md5
```

DuckStation also ships `scripts/check_regression_tests.py` for HTML diff
reports — adapted for this project as:

```bash
python3 path/to/check_regression_tests.py \
    regtest-results/baseline \
    regtest-results/candidate \
    -o regtest-results/diff-report.html
```

The HTML report shows side-by-side comparisons with an interactive viewer.

## Parallel runs

```bash
for scene in STAND JOHNNY WALKSTUF; do
    ./scripts/run-regtest.sh \
        --dumpdir "regtest-results/${scene}" \
        --frames 1800 &
done
wait
```

`REGTEST_PARALLEL=4` is the intended cap for batch scripts. Each Docker
container is isolated, so parallel runs do not interfere with each other.

## Common breakages

**"Docker image not found"** — Run `./scripts/build-regtest-image.sh`
first.

**"No .cue file found"** — Build the CD image:
`./scripts/build-ps1.sh && ./scripts/make-cd-image.sh`.

**Test hangs / timeout** — `--timeout` (default 120s) kills hung tests.
Check `regtest.log` for the last activity.

**No frames captured** — Frame count must exceed dump interval. With
`--frames 60 --dumpinterval 60` only one frame is captured.

**BIOS errors** — DuckStation requires a real PS1 BIOS. With `-fastboot`
it sometimes boots without one but results may differ.

**"Software renderer only"** — The Docker container has no GPU access
by default. Use `--renderer Software` (the default).

**Exit code 124** — Hit the wall-clock timeout. Either the run genuinely
needs more frames or the game looped forever.

## File reference

```
config/ps1/Dockerfile.regtest      Image build (multi-stage, builds DuckStation)
config/ps1/regtest-config.sh       Default frame counts, timeouts, etc.
config/ps1/regtest-scenes.txt      63-scene manifest with status + boot route
scripts/build-regtest-image.sh     One-time image build wrapper
scripts/run-regtest.sh             Docker wrapper, full option set
scripts/regtest-scene.sh           Single-scene runner (uses manifest)
scripts/regtest-all-scenes.sh      Parallel orchestrator
scripts/analyze-regtest.py         Post-run analysis + HTML report
scripts/regtest-compare.sh         Diff two test runs
scripts/decode-ps1-bars.py         Telemetry overlay decoder
scripts/check-character-screenshot.py  Overlay-backed character check
```

## Related pages

- [Devices — what it runs on]({{ '/docs/devices/' | relative_url }})
  — the device matrix this harness exercises: DuckStation as the
  every-commit reference (and what the BIOS-error symptoms look
  like when the harness can't find one), plus the should-work-
  unverified and real-PS1 paths the regtest doesn't currently cover.
- [Build & toolchain]({{ '/docs/build/' | relative_url }}) — how the disc
  image is produced before regtest can run.
- [Development workflow]({{ '/docs/dev-workflow/' | relative_url }}) — the
  primary acceptance loop. Regtest is secondary tooling.
- [Performance battle card]({{ '/perf/' | relative_url }}) — the same
  headless DuckStation pipeline run in `perf-log` mode against
  every scene/tide variant.
- [Performance reference]({{ '/docs/performance/' | relative_url }}) —
  what the `loop_vb` / `target_vb` / `blocking_vb` columns the
  harness emits actually mean.
- [Scripted input harness]({{ '/docs/scripted-input/' | relative_url }})
  — pad-script routes the regtest binary consumes via
  `BOOTMODE.TXT pad-script`.
- [Glossary: soak-test]({{ '/docs/glossary/#soak-test' | relative_url }}) —
  the long-run randomized DuckStation pass that catches
  state-coupling bugs the per-commit regtest gate doesn't. The
  v0.8.1 [MARY 4 freeze retrospective]({{ '/lab/v081-mary4-freeze/' | relative_url }})
  is the canonical example.
- [API mapping]({{ '/docs/api/' | relative_url }}) — the SDL2 → PSn00bSDK
  surface the regtest binary is exercising.
- [Lab: regression as a lifestyle]({{ '/lab/regression-as-lifestyle/' | relative_url }})
  — the magazine treatment of why regression testing isn't a CI
  feature on this project but a way of working. Reciprocal of
  the link from there to here.
- [Lab: the 24/7 build farm]({{ '/lab/build-farm/' | relative_url }}) —
  the infrastructure-side companion to *regression as a lifestyle*:
  the Dockerized PSn00bSDK build, the parallel headless DuckStation
  runs, the `JCPERF2` log parsing, and the rule that every
  accepted performance change becomes the next baseline. This page is
  the reference manual for the orchestration the essay walks through.

## View source on GitHub

- [`scripts/run-regtest.sh`]({{ site.github_url }}/blob/main/scripts/run-regtest.sh)
  — the Docker wrapper this page documents; full option set
  in the body's Command-line options section.
- [`scripts/regtest-scene.sh`]({{ site.github_url }}/blob/main/scripts/regtest-scene.sh)
  · [`scripts/regtest-all-scenes.sh`]({{ site.github_url }}/blob/main/scripts/regtest-all-scenes.sh)
  — single-scene runner and parallel orchestrator.
- [`scripts/build-regtest-image.sh`]({{ site.github_url }}/blob/main/scripts/build-regtest-image.sh)
  — one-time Docker image build wrapper (the page's first command).
- [`scripts/analyze-regtest.py`]({{ site.github_url }}/blob/main/scripts/analyze-regtest.py)
  · [`scripts/regtest-compare.sh`]({{ site.github_url }}/blob/main/scripts/regtest-compare.sh)
  — post-run analysis + HTML report and run-vs-run diff.
- [`scripts/decode-ps1-bars.py`]({{ site.github_url }}/blob/main/scripts/decode-ps1-bars.py)
  — telemetry overlay decoder (used when frames carry the on-screen
  perf bars from `src/platform/ps1/ps1_debug.c`).
- [`scripts/check-character-screenshot.py`]({{ site.github_url }}/blob/main/scripts/check-character-screenshot.py)
  — overlay-backed character check the body's "Overlay-backed
  character checks" section walks through.
- [`config/ps1/Dockerfile.regtest`]({{ site.github_url }}/blob/main/config/ps1/Dockerfile.regtest)
  · [`config/ps1/regtest-config.sh`]({{ site.github_url }}/blob/main/config/ps1/regtest-config.sh)
  · [`config/ps1/regtest-scenes.txt`]({{ site.github_url }}/blob/main/config/ps1/regtest-scenes.txt)
  — the regtest Docker image build, default frame-counts/timeouts
  config, and 63-scene manifest with status + boot route.
- [`docs/ps1/regtest-harness.md`]({{ site.github_url }}/blob/main/docs/ps1/regtest-harness.md)
- [`docs/ps1/regtest-quickstart.md`]({{ site.github_url }}/blob/main/docs/ps1/regtest-quickstart.md)
- [`docs/ps1/TESTING.md`]({{ site.github_url }}/blob/main/docs/ps1/TESTING.md)
