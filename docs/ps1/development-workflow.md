# PS1 Development Workflow

> 🌐 **Rendered version:** **[/docs/dev-workflow/](https://hunterdavis.com/johnny-castaway-ps1/docs/dev-workflow/)** — this doc rendered on the project website with cross-links and prose context. The GitHub copy here is the source.


Operator loop for bringing up a new scene to the current acceptance bar
(pixel-perfect visuals + synced SFX across applicable variants). See
[scene-status.md](scene-status.md) for the per-scene ledger.

## Prerequisites

- Docker (or `sudo docker`) with image `jc-reborn-ps1-dev:amd64`
- DuckStation (Flatpak: `org.duckstation.DuckStation`)
- The project's `jc_reborn` host binary built at least once:
  ```bash
  ./scripts/build-host.sh
  ```

## The per-scene loop

One iteration = one scene promoted from `⏳` to `✅ / ✅` in
[scene-status.md](scene-status.md).

### 1. Capture the scene on the desktop host

```bash
./scripts/export-scene-foreground-pilot.sh \
    ''                   \  # output dir (default: host-results/<slug>-foreground-pilot)
    <slug>               \  # e.g. fishing2
    '<ADS TAG>'          \  # e.g. 'FISHING 2'
    <PACK_BASENAME>      \  # e.g. FISHING2
    0                    \  # start frame
    1.0                  \  # timeline speed
    <LOW_PACK_BASENAME>     # e.g. FISH2LOW
```

Produces:
- `host-results/<slug>-foreground-pilot/host-capture-high/frames/*.bmp`
- `host-results/<slug>-foreground-pilot/host-capture-high/frame-meta/*.json`
- `host-results/<slug>-foreground-pilot/host-capture-high/sound-events.jsonl`
- `host-results/<slug>-foreground-pilot/host-capture-low/frames/*.bmp`
- `host-results/<slug>-foreground-pilot/host-capture-low/frame-meta/*.json`
- `host-results/<slug>-foreground-pilot/host-capture-low/sound-events.jsonl`
- `generated/ps1/foreground/<PACK_BASENAME>.FG2` (high-tide full-render base-diff FG2 pack)
- `generated/ps1/foreground/<LOW_PACK_BASENAME>.FG2` (low-tide full-render base-diff FG2 pack)

The `host-results/` tree is gitignored; only routed `.FG2` files needed
by the PS1 CD are committed. FG1/FOC outputs and per-scene establishing
`.RAW` files are retired and should not be regenerated or committed.

### 2. Wire the scene in

`scripts/ps1-foreground-scene-manifest.py` — regenerate the active
foreground inventory from `config/ps1/regtest-scenes.txt`:
```bash
./scripts/ps1-foreground-scene-manifest.py \
  --write-cd-layout config/ps1/cd_layout.xml \
  --write-sheet docs/ps1/performance-scene-matrix.csv
```

`foreground_pilot.c` — routes every generated scene family dynamically.
Do not add one-off scene branches for normal FG2 playback; add a family only
if the rollout manifest introduces a new ADS prefix/naming rule.

`jc_reborn.c` — add the slug to `kProvenScenes` only after full human
visual + audible signoff. Pending scenes can still be launched explicitly with
`fgpilot <slug>` because all generated FG2 scene slugs are routable.

### 3. Build + launch

```bash
./scripts/rebuild-and-let-run.sh noclean
```

Builds the PS1 executable inside Docker, regenerates the CD image, and
launches DuckStation with the cue. BIOS plays first (boot script no
longer passes `-fastboot`, so you get the chime for volume calibration).
The game then boots straight into the scene named in `BOOTMODE.TXT`.

### 4. Validate variants

Edit `config/ps1/BOOTMODE.TXT` (or pass tokens on the `rebuild-and-let-run`
line) to exercise each applicable variant:

```
fgpilot <slug>                          # default
fgpilot <slug> night 1                  # dusk / night palette
fgpilot <slug> lowtide 1                # tide state
fgpilot <slug> holiday <N>              # holiday overlay 1..36
fgpilot <slug> raft-stage <N>           # raft build stage 0..5
fgpilot <slug> island-pos <x> <y>       # forced island position
```

Strike through any variant that does not apply to the scene (see
[scene-status.md](scene-status.md) legend). Sign off each by human
visual + audible review.

`island-pos` is also a useful diagnostic probe. Use it to prove whether
source pixels exist at a different placement, but do not turn that into
a runtime hard pin unless the original scene or host capture genuinely
requires one. Current hard pins should stay rare; as of 2026-05-03 there
is no validated reason to pin `FISHING 7` or `FISHING 8` at runtime.
They were captured and stress-tested at `x=-300,y=54`, but production
runtime placement remains variable. `JOHNNY 4` is the same rule in a
different scene: it was captured and tested at `x=-64,y=54` to keep
bottle-message pixels in frame, but production runtime placement remains
variable.

### 5. Tick the row and commit

In `docs/ps1/scene-status.md`:
- Update the scene's row: `⏳` → `✅` for visuals and SFX.
- List the validated variants (or strike through N/A ones).
- Fill in "last verified" with the release tag once the next release
  lands, or the current commit SHA in the interim.

Commit with a scene-scoped message:
```
<slug>: pixel-perfect playback with synced SFX
```

### 6. Release cadence

Every **10** scenes reaching `✅ / ✅` under this bar:
```bash
./scripts/release.sh "<milestone message>"
```
which bumps `VERSION`, copies the ISO to `release/`, creates an annotated
tag `vX.Y.Z-ps1`, and pushes.

## Rebuild / launch shortcuts

```bash
# Build PS1 executable only (incremental)
./scripts/build-ps1.sh

# Clean PS1 rebuild
./scripts/build-ps1.sh clean

# Regenerate CD image only (after build)
./scripts/make-cd-image.sh

# Build + CD image + launch DuckStation
./scripts/rebuild-and-let-run.sh noclean

# Full release (build + bump + tag + push)
./scripts/release.sh "<message>"
```

## Debug + diagnostics

- **Telemetry overlay**: `ps1_debug.c` provides a 5-panel on-screen
  overlay. Use this for frame-by-frame counters and hot-path diagnostics.
- **PS1 `printf()` / TTY logging** works in DuckStation for gated probes.
  Use `printf-test` or `logtest` in BOOTMODE to emit `JCLOG` breadcrumbs:
  `./scripts/rebuild-and-let-run.sh fgpilot fishing1 printf-test noloop`.
- **DuckStation log file** receives TTY output. The live-run script
  temporarily enables `BIOS.TTYLogging`, `SIO.RedirectToTTY`, and file
  logging, then restores the user's settings. It truncates
  `duckstation.log` at 2 GiB by default; override with
  `DUCKSTATION_LOG_MAX_BYTES`.
- **Do not log per frame** from render, sound, capture, or perf paths.
  Text I/O is too noisy for timing-sensitive screensaver playback.
- **Per-scene capture diffs**: the host-capture frames + frame-meta
  JSONs are a useful ground truth when a replay mismatches. Point
  `compare-scene-reference.py` at them.
- **Scripted controller repros**: use
  `./scripts/ps1-menu-input-harness.sh` or a custom `PADSCRIPT.TXT` route
  when a bug depends on menu navigation or Freeplay input. The script drives
  the actual PS1 pad path and can emit `JCPADSHOT` markers for screenshot
  alignment; see [scripted-input-harness.md](scripted-input-harness.md).
- **Keep scratch files in `scratch/`**, never `/tmp`. DuckStation
  logs grow fast and `/tmp` has filled and broken the shell before.

## Historical / secondary tooling

The headless regtest harness (`scripts/regtest-scene.sh`,
`scripts/run-regtest.sh`, `config/ps1/regtest-scenes.txt`) and the
binary-library regression stack are retained as secondary tools for
targeted questions. They are **not** the primary acceptance gate;
refer to [TESTING.md](TESTING.md) for when each is still useful and
[ps1-branch-cleanup-plan.yaml](ps1-branch-cleanup-plan.yaml) for
pending archival decisions.

## Existing all-scene pack corpus

`./scripts/batch-capture-all-scenes.sh` has generated high-tide and
low-tide FG2 packs for all 63 scenes in `generated/ps1/foreground/`
(126 packs total). The corpus exists so new scenes can be routed and
validated one at a time; the CD image should include only the packs
currently needed by routed scene-playback entries.

Legacy `--pack-format fg1`, `fgOverlayPackPathForScene`, and
`fgDirectPackPathForScene` paths have been removed from the active PS1
runtime/generation path. Do not restore them while bringing up new
scenes; old details live in the archaeology and research docs.

## See also

- [scene-status.md](scene-status.md) — per-scene ledger
- [current-status.md](current-status.md) — project narrative + history
- [TESTING.md](TESTING.md) — validation strategy
- [build-system.md](build-system.md) — CMake / Docker / CD layout
- [toolchain-setup.md](toolchain-setup.md) — dev environment
