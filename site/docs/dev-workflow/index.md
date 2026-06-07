---
layout: page
title: Development workflow
eyebrow: Runbook
subtitle: The author's own per-scene loop. Capture, encode, replay, screenshot, validate.
description: The Johnny Castaway PS1 port's per-scene development loop — host capture, FG2 encoding, PS1 replay, screenshot, and the human visual + audible signoff bar.
---

A labor of love by Hunter Davis. This page is the author's own runbook —
the literal sequence of commands typed when bringing up a new scene. It is
**not** a contributor onboarding. The project does not solicit
contributions; see the [FAQ]({{ '/faq/' | relative_url }}) for the
project's stance. If you're reading this for archaeology or to understand
how the disc image is being built scene by scene, you're in the right
place.

If you paid for this, you were cheated. Open source and free.

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

## What "done" means for a scene

The current acceptance bar is **human visual + audible signoff** on the
scene-playback ([fgpilot]({{ '/docs/glossary/#fgpilot' | relative_url }})) path. A scene reaches `✅ / ✅` in
[`scene-status.md`]({{ site.github_url }}/blob/main/docs/ps1/scene-status.md)
when the author has confirmed visuals + SFX are correct across every
applicable variant for that scene.

That is the gate. Frame-hash-equal regression is a useful diagnostic but
not the gate. SHA256 state hashes are useful for "did this build change
behavior" questions but not the gate. See
[Regression testing]({{ '/docs/regtest/' | relative_url }}) for what
those tools are good for.

Performance is its own ledger, separate from the visual-signoff gate
above. Per-scene timing lives at
[/perf/]({{ '/perf/' | relative_url }}); a scene can be validated
under the [FISHING 1 bar]({{ '/docs/glossary/#fishing1-bar' | relative_url }})
without sitting in the green band on the
matrix, and a row in the green band that fails visual review
doesn't ship. The two ledgers stay separate on purpose — different
bars, different cadences, different failure modes.

Milestone release cadence was historically every **10** scenes reaching
`✅ / ✅`, with smaller stability releases between milestones when there
was something worth shipping. That cadence drove the v0.4–v0.7 line up
through the v0.7.0-ps1 release that capped the 63/63 validation grind
([retrospective]({{ '/lab/the-63-scene-grind/' | relative_url }})).
With every scene now signed off, the 0.8.x line ships against
performance, stability, and content bars instead — a per-release theme
under
[/releases/]({{ '/releases/' | relative_url }}). The visual-signoff
gate above is still the gate any new or re-validated scene would have
to clear.

## Prerequisites

- Docker, image `jc-reborn-ps1-dev:amd64` already built. See
  [Build & toolchain]({{ '/docs/build/' | relative_url }}).
- DuckStation, Flatpak install (`org.duckstation.DuckStation`).
- A real PS1 BIOS file in one of the auto-detected directories.
- The host capture binary built at least once:

  ```bash
  ./scripts/build-host.sh
  ```

## The per-scene loop

One iteration moves one scene from `⏳` to `✅ / ✅`. Six steps.

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

Outputs:

```
host-results/<slug>-foreground-pilot/
  host-capture-high/
    frames/*.bmp
    frame-meta/*.json
    sound-events.jsonl
  host-capture-low/
    frames/*.bmp
    frame-meta/*.json
    sound-events.jsonl

generated/ps1/foreground/
  <PACK_BASENAME>.FG2       # high-tide full-render base-diff FG2 pack
  <LOW_PACK_BASENAME>.FG2   # low-tide full-render base-diff FG2 pack
```

The `host-results/` tree is gitignored; only the routed `.FG2` files
needed by the PS1 disc actually get committed. The retired FG1 / FOC
output paths and per-scene establishing `.RAW` files should not be
regenerated or committed under the current bar.

### 2. Wire the scene in

Three files touch the wiring:

[`config/ps1/cd_layout.xml`]({{ site.github_url }}/blob/main/config/ps1/cd_layout.xml) —
add the packs:

```xml
<file name="<PACK_BASENAME>.FG2"     type="data" source="../../generated/ps1/foreground/<PACK_BASENAME>.FG2"/>
<file name="<LOW_PACK_BASENAME>.FG2" type="data" source="../../generated/ps1/foreground/<LOW_PACK_BASENAME>.FG2"/>
```

[`src/foreground_pilot/foreground_pilot.c`]({{ site.github_url }}/blob/main/src/foreground_pilot/foreground_pilot.c) —
add the scene to the active routing function:

- `fgCompactOverlayPackPathForScene(sceneName)` returns the high or low
  `.FG2` based on `islandState.lowTide`.

[`src/jc_reborn.c`]({{ site.github_url }}/blob/main/src/jc_reborn.c) —
add the slug to `kAllScenes[]` so the screensaver picker can draw it.
There used to be a separate `kProvenScenes` array gated on human
visual + audible signoff; that gate was retired once the full
63-scene set ran cleanly post-v0.7.0-ps1, so the picker now draws
from everything in `kAllScenes`. New scenes land here as soon as
their FG2 pack ships on the CD layout. Pending scenes can still be
launched explicitly with `fgpilot <slug>` for bring-up work.

### 3. Build + launch

```bash
./scripts/rebuild-and-let-run.sh noclean
```

That builds the PS1 executable inside Docker, regenerates the CD image,
and launches DuckStation with the cue. The boot script no longer passes
`-fastboot`, so the BIOS chime plays first — handy for volume calibration
before the scene loads. The game then boots straight into whatever scene
[`BOOTMODE.TXT`]({{ '/docs/glossary/#bootmode' | relative_url }}) named.

### 4. Validate variants

Edit [`config/ps1/BOOTMODE.TXT`]({{ site.github_url }}/blob/main/config/ps1/BOOTMODE.TXT)
(or pass tokens on the `rebuild-and-let-run` line) to exercise each
applicable variant:

```text
fgpilot <slug>                          # default
fgpilot <slug> night 1                  # dusk / night palette
fgpilot <slug> lowtide 1                # tide state
fgpilot <slug> holiday <N>              # holiday overlay 1..36
fgpilot <slug> raft-stage <N>           # raft build stage 0..5
fgpilot <slug> island-pos <x> <y>       # forced island position
```

Strike through any variant that does not apply to the scene (the legend
in [`scene-status.md`]({{ site.github_url }}/blob/main/docs/ps1/scene-status.md)
lists which variants apply to which scenes). Sign off each variant by
human visual + audible review.

### 5. Tick the row and commit

In `docs/ps1/scene-status.md`:

- Update the scene's row: `⏳` → `✅` for visuals and SFX.
- List the validated variants (or strike through N/A ones).
- Fill in "last verified" with the release tag, or the current commit
  SHA in the interim.

Commit with a scene-scoped message:

```
<slug>: pixel-perfect playback with synced SFX
```

### 6. Release cadence

Every ten scenes reaching `✅ / ✅`:

```bash
./scripts/release.sh "<milestone message>"
```

That bumps `VERSION`, copies the ISO to `release/`, updates the website
release metadata, rebuilds `www/`, creates an annotated tag `vX.Y.Z-ps1`,
and pushes.

## The shorter shortcuts

| Command                                       | What it does |
|-----------------------------------------------|--------------|
| `./scripts/build-ps1.sh`                      | Build PS1 executable only (incremental). |
| `./scripts/build-ps1.sh clean`                | Clean PS1 rebuild. |
| `./scripts/make-cd-image.sh`                  | Regenerate CD image only (after build). |
| `./scripts/rebuild-and-let-run.sh noclean`    | Build + CD image + launch DuckStation. |
| `./scripts/release.sh "<message>"`            | Full release (build + bump + tag + push). |

`rebuild-and-let-run.sh` temporarily enables DuckStation TTY logging
(`BIOS.TTYLogging`, `SIO.RedirectToTTY`, file logging) for the run, then
restores the user's settings on exit. It truncates `duckstation.log` at
2 GiB by default; override with `DUCKSTATION_LOG_MAX_BYTES`. Long-run
logs are bounded.

## Honest about the rate

A scene takes anywhere from a couple of hours to a couple of days to
bring up. The capture step is fast — minutes. The encode is fast —
seconds. The PS1 replay is fast. The hard part is everything that breaks
between "host renders this correctly" and "PS1 renders this correctly":

- Cold-boot ADS scenes (`FISHING 1`, `FISHING 2`, `FISHING 6`) have
  `ADD_SCENE` commands gated by `IF_LASTPLAYED`. On cold boot those
  conditions are never satisfied — the scene comes up empty. The fgpilot
  path sidesteps this by replaying captured frames directly.
- A scene's wave overlay timing can be one frame off from what the host
  captured if the clean-rect ordering changed.
- Holiday stamping order can paint over the wrong layer if the new
  variant doesn't re-snapshot.
- A scene that worked in the screensaver loop can break when re-entered
  cold because some other scene left state behind. The fix is almost
  always in `fgBackdropRelease(keepBackgrnd)` semantics.

The runbook above is the *happy path*. The actual time per scene is
dominated by debugging mismatches between host capture and PS1 replay,
which is why the [regtest harness]({{ '/docs/regtest/' | relative_url }})
exists as a diagnostic — not as the acceptance gate.

## Diagnostics

- **Telemetry overlay**:
  [`ps1_debug.c`]({{ site.github_url }}/blob/main/src/platform/ps1/ps1_debug.c) provides
  a 5-panel on-screen overlay. Use this for frame-by-frame counters and
  hot-path diagnostics.
- **PS1 `printf()` / TTY logging** works in DuckStation for gated probes.
  Use `printf-test` or `logtest` in `BOOTMODE.TXT` to emit `JCLOG`
  breadcrumbs:
  ```bash
  ./scripts/rebuild-and-let-run.sh fgpilot fishing1 printf-test noloop
  ```
- **DuckStation log file** receives TTY output, typically at
  `~/.var/app/org.duckstation.DuckStation/config/duckstation/duckstation.log`.
- **Do not log per frame** from render, sound, capture, or perf paths.
  Text I/O is too noisy for timing-sensitive screensaver playback.
- **Per-scene capture diffs**: the host-capture frames + frame-meta JSONs
  are useful ground truth when a replay mismatches. Point
  `compare-scene-reference.py` at them.
- **Scripted controller repros**: use
  [`ps1-menu-input-harness.sh`]({{ site.github_url }}/blob/main/scripts/ps1-menu-input-harness.sh)
  or a custom `PADSCRIPT.TXT` route when a bug depends on menu navigation or
  [Freeplay]({{ '/docs/glossary/#freeplay' | relative_url }}) input. The script drives the actual PS1 pad path and emits
  `JCPADSHOT` markers for screenshot alignment; see
  [Scripted input harness]({{ '/docs/scripted-input/' | relative_url }}).
- **Scratch files go in `scratch/`**, never `/tmp`. DuckStation logs
  grow fast and `/tmp` has filled and broken the shell before.

## Known runtime pitfalls

- **Per-frame text logging is unsafe for timing and log volume.**
  `printf()` is available but debug logging in hot paths alters timing
  and fills `duckstation.log`.
- **Cold-boot ADS scenes** (above): use the fgpilot replay path.
- **`/tmp` is off-limits.** Redirect long-running output to `scratch/`.

## Boot mode reference

The PS1 executable reads `BOOTMODE.TXT` from the CD. The active mode for
current development:

| Mode                     | Example                       | Description |
|--------------------------|-------------------------------|-------------|
| `fgpilot <slug> [tokens]`| `fgpilot fishing1 night 1`    | Hybrid scene playback from high/low FG2 packs, with variant tokens. |

FG1 / FOC packs and per-scene establishing RAWs are retired. The
historical routes — `story scene N`, `story direct N`, and
`island ads X.ADS N` — are removed from the active PS1 executable. They
should not be used to certify or bring up scenes under the current bar.

## Existing all-scene pack corpus

`./scripts/batch-capture-all-scenes.sh` has previously generated
high-tide and low-tide FG2 packs for all 63 scenes in
[`generated/ps1/foreground/`]({{ site.github_url }}/tree/main/generated/ps1/foreground)
(126 packs total). The corpus exists so new scenes can be routed and
validated one at a time; the CD image should include only the packs
currently needed by routed scene-playback entries.

The legacy `--pack-format fg1`, `fgOverlayPackPathForScene`, and
`fgDirectPackPathForScene` paths are removed from the active runtime.
Don't restore them while bringing up new scenes; the old details live
in the archaeology and research docs.

## Docker images in this workflow

| Image                       | Purpose                            |
|-----------------------------|------------------------------------|
| `jc-reborn-ps1-dev:amd64`   | PS1 cross-compile (PSn00bSDK).     |
| `jc-reborn-regtest:latest`  | Headless DuckStation regtest.      |

## Related pages

- [Build & toolchain]({{ '/docs/build/' | relative_url }}) — what
  `build-ps1.sh` actually does inside the dev container.
- [Regression testing]({{ '/docs/regtest/' | relative_url }}) — secondary
  diagnostics path. Useful for frozen-artifact comparisons.
- [Pause menu]({{ '/docs/pause-menu/' | relative_url }}) — the editors
  for Set Time / Island Pos / Seed cover variant testing without
  rebooting.
- [Scene ledger]({{ '/scenes/' | relative_url }}) — current per-scene
  status (visual signoff bar).
- [Performance battle card]({{ '/perf/' | relative_url }}) — second
  ledger; per-scene timing against the target frame budget.
- [Performance reference]({{ '/docs/performance/' | relative_url }}) —
  what each `loop_vb` / `target_vb` / `blocking_vb` column means
  and the experiment-log discipline behind the matrix.
- [Lab: the 63-scene grind]({{ '/lab/the-63-scene-grind/' | relative_url }})
  — the magazine treatment of the per-scene validation loop this
  page is the reference manual for. From "first scene signed off"
  to "every scene signed off," same loop repeated 63 times: capture,
  pack, route, replay, sign off.
- [Lab: the chapter-select grind]({{ '/lab/chapter-select-grind/' | relative_url }})
  — the v0.8.4-ps1 sequel to the 63-scene grind: walking every
  validated pack again on hardware to ship custom Scene Explorer
  thumbnails plus reconcile [caption-mismaps]({{ '/docs/glossary/#caption-mismap' | relative_url }})
  in the per-scene metadata.

## View source on GitHub

- [`scripts/rebuild-and-let-run.sh`]({{ site.github_url }}/blob/main/scripts/rebuild-and-let-run.sh)
  — the inner-loop wrapper this page references throughout
  the per-scene runbook.
- [`scripts/build-host.sh`]({{ site.github_url }}/blob/main/scripts/build-host.sh)
  — host capture binary build (Prerequisites step).
- [`scripts/export-scene-foreground-pilot.sh`]({{ site.github_url }}/blob/main/scripts/export-scene-foreground-pilot.sh)
  — host-side capture-and-pack script that produces per-scene
  FG2 packs (Step 1).
- [`scripts/make-cd-image.sh`]({{ site.github_url }}/blob/main/scripts/make-cd-image.sh)
  — CD image regenerator (the "Shorter shortcuts" entry).
- [`scripts/release.sh`]({{ site.github_url }}/blob/main/scripts/release.sh)
  — full release wrapper (Step 6: build + bump + tag + push).
- [`scripts/batch-capture-all-scenes.sh`]({{ site.github_url }}/blob/main/scripts/batch-capture-all-scenes.sh)
  — generates the all-63-scenes pack corpus referenced in the
  "Existing all-scene pack corpus" section.
- [`src/foreground_pilot/foreground_pilot.c`]({{ site.github_url }}/blob/main/src/foreground_pilot/foreground_pilot.c)
  — the routing function `fgCompactOverlayPackPathForScene` that Step 2
  asks the contributor to extend.
- [`src/jc_reborn.c`]({{ site.github_url }}/blob/main/src/jc_reborn.c)
  — `kAllScenes[]` (the scene-picker pool Step 2 lands new slugs in,
  post-`v0.7.0-ps1` retirement of the gated `kProvenScenes`).
- [`src/platform/ps1/ps1_debug.c`]({{ site.github_url }}/blob/main/src/platform/ps1/ps1_debug.c)
  — the 5-panel telemetry overlay the Diagnostics section names.
- [`docs/ps1/development-workflow.md`]({{ site.github_url }}/blob/main/docs/ps1/development-workflow.md) — original.
- [`docs/ps1/TESTING.md`]({{ site.github_url }}/blob/main/docs/ps1/TESTING.md) — the active strategy.
- [`docs/ps1/scene-status.md`]({{ site.github_url }}/blob/main/docs/ps1/scene-status.md) — per-scene ledger.
