# PS1 Port Testing Guide

> 🌐 **Rendered version:** **[/docs/dev-workflow/](https://hunterdavis.com/johnny-castaway-ps1/docs/dev-workflow/)** — this doc rendered on the project website with cross-links and prose context. The GitHub copy here is the source.


**Primary acceptance = human visual + audible signoff** on the
scene-playback (fgpilot) path. Everything else in this document is
secondary tooling: useful for targeted questions, not for certifying a
scene as done.

For the scene-by-scene bring-up loop, see
[development-workflow.md](development-workflow.md). For the per-scene
ledger, see [scene-status.md](scene-status.md).

## Primary: scene-playback validation

```bash
./scripts/rebuild-and-let-run.sh noclean
```

Builds the PS1 executable + CD image inside Docker and launches
DuckStation with the cue. `BOOTMODE.TXT` controls which scene and which
variant boots. A scene reaches `✅ / ✅` in `scene-status.md` when a
human has confirmed visuals + SFX are correct across every applicable
variant for that scene. Milestone release cadence: every 10 such scenes;
smaller stability releases may happen between scene milestones.

### Variant tokens

| Token | Range | Purpose |
|---|---|---|
| `night <0\|1>` | 0 or 1 | Night/dusk palette |
| `lowtide <0\|1>` | 0 or 1 | Tide state |
| `holiday <N>` | 0..36 | Holiday overlay variant |
| `raft-stage <N>` | 0..5 | Cumulative raft-build state |
| `island-pos <x> <y>` | — | Force island position |

## Headless Performance Matrix

All generated FG2 scene/tide variants are routable through `fgpilot`, even
before they are human-certified. Use the generated matrix runner for speed
coverage, not the legacy regtest certification scripts:

```bash
# Smoke one generated case.
./scripts/ps1-perf-all-scenes.sh --limit 1 --tides high --frames 7200

# Walk the full 126-variant matrix. This is intentionally long-running.
./scripts/ps1-perf-all-scenes.sh --tides both --frames 7200

# Current full-matrix pass: reuse the built PS-EXE, build case-local CDs,
# stop once JCPERF2 has emitted, retry transient wrapper kills, and run two
# cases at a time.
PS1_PERF_STATS_VERSION=compact-fgp3-v2-fullmatrix \
  ./scripts/ps1-perf-all-scenes.sh --tides both --order list \
  --frames 7200 --timeout 220 \
  --output scratch/ps1-perf-iterate/compact-fgp3-v2-fullmatrix \
  --stats-version compact-fgp3-v2-fullmatrix \
  --continue-on-fail --skip-build --retries 2 --resume-output --jobs 2

# Resume from rows not yet measured in the CSV sheet.
./scripts/ps1-perf-all-scenes.sh --only-pending --limit 8 --tides high
```

The durable sheet is `docs/ps1/performance-scene-matrix.csv`. Pending rows
mean no current headless perf summary has been recorded for that scene/tide.
The rendered website battle card is
[/scenes/](https://hunterdavis.com/johnny-castaway-ps1/scenes/).

Current battle-card rollup as of 2026-04-30:

| Metric | Value |
|---|---:|
| Scene/tide variants routed through headless perf | `126 / 126` |
| Timing-bearing variants | `120 / 126` |
| Scenes with at least one active-loop timed variant | `60 / 63` |
| Scenes with both high/low variants measured | `63 / 63` |
| Blocked variants | `0 / 126` |
| Timing-bearing average over target | `+15.0%` |
| Timing-bearing average target speed | `87.9%` |
| Latest perf matrix run | `2026-04-30T10:51:52` |
| Stats version | mixed: latest rows use `compact-fgp3-v41-walkstuf1low-indexed8-fgp3`; earlier follow-up rows use `compact-fgp3-v40-walkstuf1-indexed8-fgp3` through `compact-fgp3-v3-stand12low`; full-matrix baseline rows remain `compact-fgp3-v2-fullmatrix` |
| FISHING 1 canary | `1207 / 1076 VBlanks`, `+12.2%`, `89.1% target speed`, `blocking_vb=0` |

Reporting rule: after every accepted perf optimization, or every rejected
experiment worth preserving, update `performance-scene-matrix.csv`,
`performance-experiment-log.md`, the README status table, and the rendered
website scene/performance battle card. The CSV is the durable numeric source; prose
surfaces should summarize it, not invent independent numbers.
Each measured or blocked CSV row carries `last_run_at`, derived from the
headless run directory (`scratch/ps1-perf-iterate/YYYYMMDD-HHMMSS`), so stale
scene rows are visible on the rendered battle card.
Rows also carry `stats_version`; the current full matrix baseline is
`compact-fgp3-v2-fullmatrix`, and accepted follow-up rows now use
`compact-fgp3-v41-walkstuf1low-indexed8-fgp3`; earlier follow-up rows use
`compact-fgp3-v40-walkstuf1-indexed8-fgp3` through `compact-fgp3-v3-stand12low`. Six routed rows (`mary3`, `suzy1`, `suzy2`,
high/low) complete without active-loop timing and are excluded from speed
averages even though the route/gate itself passes.

## Secondary (historical): headless regtest harness

The headless regtest harness runs DuckStation's regtest binary in Docker
and captures per-frame data. It's preserved for targeted diagnostics
(regression hunts, deterministic frame-timing investigations) but is not
the certification gate.

```bash
# Single-scene run
./scripts/regtest-scene.sh --scene "STAND 2"

# Longer run with a custom interval
./scripts/regtest-scene.sh --scene "BUILDING 1" --frames 9000 --interval 120

# Raw headless run with a late capture window
./scripts/run-regtest.sh --frames 3600 --start-frame 2400 --dumpinterval 60 --dumpdir scratch/regtest-out
```

Scene manifest: `config/ps1/regtest-scenes.txt`. The `verified`,
`bringup`, `blocked`, `untested` tokens in that file reflect the legacy
regtest-route status model, **not** the current scene-playback bar. See
`ps1-branch-cleanup-plan.yaml` § `status_model_correction`.

Regtest output:
```
regtest-results/<ads>-<tag>/
  frames/*.png              # captured screenshots
  telemetry.json            # debug-panel data
  result.json               # structured outcome
  printf.log                # PS1 TTY output when TTY logging is enabled
```

## Secondary (historical): binary library

The binary-library stack builds a PS1 executable + CD image for every
code-changing commit, enabling regression bisection against any historical
build. Useful for archaeology; retired from the primary workflow.

```bash
# Full library (~15 min, ~118 GB — see cleanup plan)
./scripts/build-binary-library.sh
```

`ps1-branch-cleanup-plan.yaml` plans to archive `binary-library/` out of
the repo and retain only a manifest + sample-rebuild path. See plan
phase `phase_04_retire_binary_library_surface`.

## Boot modes

The PS1 executable reads `BOOTMODE.TXT` from the CD. Primary mode for
current development:

| Mode | Example | Description |
|---|---|---|
| `fgpilot <slug> [tokens...]` | `fgpilot fishing1 night 1` | **Primary.** Hybrid scene playback from high/low FG2 packs, with variant tokens. |

FG1/FOC packs and per-scene establishing RAWs are retired. They should
not be used to certify or bring up scenes under the current bar.

Historical routes removed from the active PS1 executable:

| Mode | Example | Description |
|---|---|---|
| `story scene N` | `story scene 38` | Host-only / historical story-loop route |
| `story direct N` | `story direct 25` | Host-only / historical direct scene route |
| `island ads X.ADS N` | `island ads BUILDING.ADS 1` | Host-only / historical ADS route |

## PS1 printf / TTY logging

PS1 `printf()` now works in DuckStation when TTY logging is enabled. The
normal live-run path handles this automatically:

```bash
./scripts/rebuild-and-let-run.sh fgpilot fishing1 printf-test noloop
```

The `printf-test` / `logtest` BOOTMODE tokens emit bounded `JCLOG` phase
breadcrumbs. Output appears in DuckStation's log file, typically:

```text
~/.var/app/org.duckstation.DuckStation/config/duckstation/duckstation.log
```

Rules:

- Use `printf()` for gated setup/teardown probes and rare failure
  breadcrumbs.
- Do not add per-frame `printf()` calls to scene playback, capture,
  compositor, sound, or perf measurement paths.
- Use `perf-log` for bounded scene-level performance summaries. It prints
  `JCPERF` lines at scene boundaries and keeps the framebuffer clean:
  `./scripts/rebuild-and-let-run.sh fgpilot fishing1 perf-log noloop`.
- Keep long-run logs bounded. `rebuild-and-let-run.sh` enables TTY logging
  for the run and truncates the DuckStation log at 2 GiB by default; set
  `DUCKSTATION_LOG_MAX_BYTES=0` to disable that guard.

## Known runtime pitfalls

- **Per-frame text logging is still unsafe for timing and log volume.**
  `printf()` is available, but debug logging in hot render/audio paths can
  alter timing and fill `duckstation.log`. Use the telemetry overlay or
  fixed counters for frame-by-frame visibility.
- **Cold-boot ADS scenes** (`FISHING 1`, `FISHING 2`, `FISHING 6`) have
  `ADD_SCENE` commands behind `IF_LASTPLAYED`. On cold boot these
  conditions are never satisfied, producing an empty scene. The
  fgpilot path sidesteps this by replaying captured frames directly.
- **`/tmp` is off-limits for scratch files.** Redirect long-running
  output (DuckStation logs, capture bundles) to `scratch/` in the repo.
  `/tmp` has filled and broken the shell before.

## Docker images

| Image | Purpose | Build |
|---|---|---|
| `jc-reborn-ps1-dev:amd64` | PS1 cross-compile (PSn00bSDK) | `./scripts/build-docker-image.sh` |
| `jc-reborn-regtest:latest` | Headless DuckStation regtest | `config/ps1/Dockerfile.regtest` |

## Do not cite

The older `Scene Status (as of 2026-04-04)` table (60 verified / 3
blocked / 63 total) from earlier versions of this file reflects the
legacy regtest-route status model and does not translate to the current
acceptance bar. It is preserved as history in
`current-status.md` § "Historical status numbers (not current)". Current
per-scene status lives in [scene-status.md](scene-status.md).
