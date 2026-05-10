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

Current battle-card rollup as of 2026-05-09:

| Metric | Value |
|---|---:|
| Scene/tide variants routed through headless perf | `126 / 126` |
| Timing-bearing variants | `126 / 126` |
| Scenes with at least one active-loop timed variant | `63 / 63` |
| Scenes with both high/low variants measured | `63 / 63` |
| Blocked variants | `0 / 126` |
| Timing-bearing average over target | `+0.3%` (`0.3426%` exact, public-capped) |
| Timing-bearing average target speed | `99.7%` (`99.6651%` exact, public-capped) |
| Latest perf matrix run | `2026-05-09T22:56:32` |
| Stats version | mixed; newest optimized/code-headroom rows use `walkstuf1-high-group201-213-v277`, `visitor3-low-noop113-v249`, `visitor3-low-noop114117-v248`, `visitor3-high-f127-f130-resident-copy-v238`, `activity9-low-compact-fgp3-v174`, `johnny1-compact-fgp3-v173`, `walkstuf3-low-compact-fgp3-v171`, `activity9-high-compact-fgp3-v167`, `building6-compact-fgp3-v165`, `walkstuf3-high-compact-fgp3-v163`, `building2-low-restore-window-slack4-v160`, `visitor5-high-compact-fgp3-noautoprime-v158`, `building1-compact-fgp3-noautoprime-v157`, `mary3-preserve-window-slack8-v149`, `missing-scenes-current-v001`, `visitor3-tail-trim-stageguard-v127`, `graphics-composite-os-v111`, `building2-low-group365-381-v110`, `building2-high-group60-72-v109`, `building2-high-restore-minus-current-v108`, `visitor3-low-offscreen-exitright-v106`, `visitor3-high-offscreen-drawclip-v105`, `walkstuf1-compact-fgp3-v141`, `visitor3-low-readgroup-prune-v088`, `building4-restore-minus-current-v087`, `visitor3-restore-minus-current-v086`, `visitor3-high-readgroup-prune-v084`, `compact-u16-inline-v083`, `fgp3v4-drawcompact-all-v082`, `activity9-dead-readgroup-prune-v082`, `read-group-selector-single-assign-v082`, `visitor3-high-remove-72-84-v082`, `visitor3-high-remove-144-160-v082`, `johnny2-prefetch-relief-v081`, `activity9-low-fgp3-cleanup-compact-v081`, `building4-fgp3-cleanup-compact-window-v081`, `building2-fgp3-cleanup-compact-v081`, `visitor3-fgp3-cleanup-compact-v081`, `mary2-prefetch-relief-v081`, `mary2-fgp3-padded-v081`, `johnny2-fgp3-padded-v081`, `mary5-fgp3-padded-v081`, `activity11-fgp3-padded-v081`, `building5-fgp3-padded-v080`, and `walkstuf1-fgp2-setup-prime-v080`; full row-level versions remain in `performance-scene-matrix.csv` |
| FISHING 1 canary | `1068 / 1074 VBlanks`, `0.0% public over target`, `100.0% public target speed`, `blocking_vb=2` |

Public reporting caps faster-than-target rows at `0.0%` over target /
`100.0%` target speed so the website never presents playback faster than
native cadence. The CSV keeps the raw signed `over_target_*` values for
optimization analysis.

Latest promoted VISITOR3 note: `visitor3-low-noop113-v249` extends the
pack-only no-op residual lane one frame earlier after the v248 frame
`114..117` compaction. Low improves `1086/1035 -> 1075/1039`, overrun
`51 -> 36`, blocking `93 -> 69`, loop reads `17 -> 16`, loop-read time
`100 -> 83`, and due misses `16 -> 12`; `prefetch_overrun_vb=0`. High
remains on `visitor3-high-f127-f130-resident-copy-v238` at `1075/1037`.

Latest promoted WALKSTUF1 note: `walkstuf1-high-group201-213-v277` adds a
high-tide-only read group for sectors `201..213`. High improves
`1490/1424 -> 1488/1426`, overrun `66 -> 62`, hidden refill `31 -> 27`, and
loop reads `69 -> 68`, with broad controls flat.

Latest rejected VISITOR3 v183-v212 note: low-tide precursor motion-copy frames
`114..118`, a C-side motion fastspan copy path, terminal zero/origin trimming,
low hull-motion retries, terminal hand-authored read groups, and simple motion
row-copy runtime paths, compact motion-copy metadata, frame `117` sparse hull
in both/low-only forms, runtime/staged narrow dirty-row upload, naive compact-v4
motion-marker dispatch, both-tide frame `125` re-anchor, low frame `127`
re-anchor, frame `116`/`114` copy-only hull shapes, and low setup-prime caps
above `208 KiB` are log-only
failures.
The pack rewrites either saved no meaningful bytes or regressed active timing,
the read groups saved some reads but hurt cadence, and the runtime/data-format
copy paths either crossed the `215040` byte PS-EXE bucket, stayed exact-flat, or
hurt refill cadence or failed before `JCPERF2`. The next VISITOR3 work should
use custom data-shape, dictionary, pack-authored precomposed-strip, or generated
scheduler ideas rather than another wider precursor motion window, hand read
table, metadata shrink, or C-side upload/copy micro-optimization.

Reporting rule: after every accepted perf optimization, or every rejected
experiment worth preserving, update `performance-scene-matrix.csv`,
`performance-experiment-log.md`, the README status table, and the rendered
website scene/performance battle card. Pack-format/preprocessing planning
passes also refresh `performance-preprocess-opportunities.csv` and its
markdown summary. The CSVs are the durable numeric sources; prose surfaces
should summarize them, not invent independent numbers.
Each measured or blocked CSV row carries `last_run_at`, derived from the
headless run directory (`scratch/ps1-perf-iterate/YYYYMMDD-HHMMSS`), so stale
scene rows are visible on the rendered battle card.
Rows also carry `stats_version`; the current full matrix baseline is
`compact-fgp3-v2-fullmatrix`, and the latest refreshed follow-up rows now use
`visitor3-high-f127-f130-resident-copy-v238`,
`visitor3-low-noop114117-v248`,
`activity9-low-compact-fgp3-v174`,
`johnny1-compact-fgp3-v173`,
`walkstuf3-low-compact-fgp3-v171`,
`activity9-high-compact-fgp3-v167`,
`building6-compact-fgp3-v165`,
`walkstuf3-high-compact-fgp3-v163`,
`building2-low-restore-window-slack4-v160`,
`visitor5-high-compact-fgp3-noautoprime-v158`,
`building1-compact-fgp3-noautoprime-v157`,
`mary3-preserve-window-slack8-v149`,
`missing-scenes-current-v001`,
`visitor3-tail-trim-stageguard-v127`,
`graphics-composite-os-v111`,
`building2-low-group365-381-v110`,
`building2-high-group60-72-v109`,
`building2-high-restore-minus-current-v108`,
`visitor3-low-offscreen-exitright-v106`,
`visitor3-high-offscreen-drawclip-v105`,
`walkstuf1-compact-fgp3-v141`,
`visitor3-low-readgroup-prune-v088`,
`building4-restore-minus-current-v087`,
`visitor3-restore-minus-current-v086`,
`visitor3-high-readgroup-prune-v084`,
`compact-u16-inline-v083`,
`fgp3v4-drawcompact-all-v082`,
`visitor3-high-remove-144-160-v082`,
`building2-fgp3-cleanup-compact-v081`,
`visitor3-fgp3-cleanup-compact-v081`,
`johnny2-prefetch-relief-v081`,
`johnny2-fgp3-padded-v081`,
`mary2-prefetch-relief-v081`,
`mary2-fgp3-padded-v081`,
`mary5-fgp3-padded-v081`,
`activity11-fgp3-padded-v081`,
`activity9-low-fgp3-cleanup-compact-v081`,
`activity9-current-v081-refresh`,
`activity9-lowgroup-v072c`,
`activity9-fgp3-v072c`,
`activity9-window-v072c`,
`activity11-12-v072c-prefetch-relief`,
`stale-next-v072c-current-refresh`,
`mary1-v072c-prefetch-relief`,
`stale-layout-v072c-current-refresh`,
`activity9-v072c-prefetch-relief`,
`stale-pressure2-v072c-current-refresh`,
`johnny1-v072c-prefetch-relief`,
`stale-pressure-v072c-current-refresh`,
`activity10-johnny3-v072-prefetch-relief`,
`stale-zero2-v072b-current-refresh`,
`stale-zero-v072b-current-refresh`,
`stale-top-v072b-current-refresh`,
`visitor5-v072-prefetch-relief`,
`mismatch-top-v072-current-refresh`,
`stand-family-v072-current-refresh`,
`visitor4-v072-current-refresh`,
`stand1-v072-current-refresh`,
`building5-fgp3-padded-v080`,
`visitor3-setup-prime-192k-v080`,
`visitor3-v072-prefetch-relief`,
`walkstuf1-fgp2-setup-prime-v080`,
`fishing5-v065-current-ledger-overlay`,
`compact-fgp3-v66-final-frame-hold`,
`compact-fgp3-v64-building2-group318-330`,
`compact-fgp3-v63-building2low-prime`, and `indexed8-row-local-dirty-v1`; other refreshed rows include
`visitor3-high-group170-186-v080-current`,
`compact-fgp3-v62-fishing3low-group253-265`,
`compact-fgp3-v61-fishing3low-group163-175`,
`compact-fgp3-v60-visitor3high-group230-242`,
`compact-fgp3-v59-visitor3high-group72-84`, `indexed8-tile-local-compose-v1`,
`compact-fgp3-v58-activity9high-window20-table`, `compact-fgp3-v57-policy-table-refactor`, and `compact-fgp3-v49-walkstuf2-auto-prime` through `compact-fgp3-v29-smallprime`. All 126 routed rows now carry active-loop timing. `suzy1` needs the longer
`12000`-frame matrix budget because its valid scene end lands after the default
`7200`-frame window.

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

## Secondary: scripted controller input

The pad-script harness is the active way to test menu and controller flows
headlessly. It embeds `config/ps1/PADSCRIPT.TXT` into the PS1 executable,
enables it with `pad-script` or `pad-script-log` in `BOOTMODE.TXT`, and merges
scripted button masks into the same active-high pad value used by real input.

```bash
# Rebuild, run DuckStation regtest, capture every major pause-menu page,
# and regenerate the website menu guide from real PS1 screenshots.
./scripts/ps1-menu-input-harness.sh

# Tune route timing while debugging a menu page.
./scripts/ps1-menu-input-harness.sh --settle-frames 30 --verbose
```

The harness emits sparse `JCPADSHOT` markers instead of per-frame logs, so it
can drive screenshots without flooding DuckStation's TTY output. Full details:
[scripted-input-harness.md](scripted-input-harness.md).

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
