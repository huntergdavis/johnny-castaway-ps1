# PS1 Scene Playback Performance Optimization Plan

Date: 2026-04-25

Original planning baseline: `f704312c ps1: keep fg2 overlays scene-relative to island`

Measured runtime baseline: `821a5745 ps1: add scene-level perf logging`

Metrics foundation checkpoint: `c0e6d95e ps1: add gated JCPERF2 metrics`

Prefetch/default-path checkpoint: `1b457163 ps1: enable FG2 prefetch by default`
merged to `main` on 2026-04-25.

Scope: active PS1 scene playback runtime only. The target is faster playback
without frame dropping, skipped art, timing lies, or regression away from the
pixel-perfect FG2 methodology.

## Executive Summary

Post-merge status: the first performance wave is now the normal runtime path.
Held-entry no-work, one-entry staging, a 16 KB FG2 stream window, guarded
fallthrough prefetch, dirty clean-rect row restore, and opt-in pad/SPI
diagnostics are active on the perf branch. The boot parameters still exist for
diagnostics, but the default FG2 playback policy is now `stage1_window` with no
JCPAD/JCSPI diagnostic sampling on the hot path.

Latest accepted default-path fishing1 high-tide run, after the pause merge,
pad/SPI diagnostic gating, the post-diagnostics window retunes, the
3 VBlank refill guard, 6 VBlank fallthrough guard, row-level X dirty restore,
and per-tile PAL4 row dirty marking, reported `policy=stage1_window`,
`buf=23568`, `hits=154`,
`due_misses=1`, `blocking_vb=21`, `prefetch.overrun_vb=12`, `loop_vb=1243`,
`overrun_vb=166`, `target_vb=1077`, `restore_bytes=2510092`,
`upload_bytes=17172480`, `trip=0`, `fallback=0`, `frame_mismatch=0`,
`sound_late=0`, and `cd_fail=0`. This is the current baseline for the next
experiment; the pre-pause best was `loop_vb=1297`.

The first real `JCPERF` sample changes the priority order. Held-entry no-work
is already implemented and working: fishing1 rendered 137 entries and held 206
VBlanks without redraw. The scene still ran about 55% too slow inside playback:
`1670` observed VBlanks against `1077` target VBlanks.

The largest measured issue is synchronous per-entry FG2 CD access. The sample
spent `562` VBlanks inside CD reads, almost the entire `593` VBlank inner
playback overrun. The current active scene reads only 423 KB of FG2 payload,
but it does that as 136 small blocking reads. That points to read amortization
and held-time prefetch as the next major bite.

The `JCPERF2` metrics foundation is now implemented and verified on an isolated
`fgpilot fishing1 perf-log noloop` run. The validated low-tide sample emitted
scene-end records with `loop_vb=1680`, `target_vb=1077`, `overrun_vb=603`,
`render=137`, `held=205`, `blocking_vb=501`, `hidden_vb=0`,
`due_misses=136`, `trip=0`, `fallback=0`, `full_fallbacks=0`, and `cd_fail=0`.
That confirms the first optimization test should attack due-frame CD blocking,
not timing-file compression.

The second issue is render pipeline serialization. A new rendered entry costs
about `10.7` VBlanks on average after subtracting held waits. CD accounts for
about `4.1` VBlanks per read, leaving about `6.6` VBlanks per rendered entry in
restore, compose, upload, the mandatory `grUpdateDisplay()` VSync, and event
work. More subphase counters are needed before changing that path aggressively.

Top likely wins, in order:

| Rank | Optimization | Expected impact | Reason |
|---|---|---|---|
| 1 | Finish CD stall hiding beyond the default 16 KB window | High | The current accepted fishing1 run is `loop_vb=1243` and still has `blocking_vb=21`, prefetch `overrun_vb=12`, and `due_misses=1`. |
| 2 | X-aware dirty upload | Medium to high | Latest default run restores only `2.5 MB` after row-level restore, but still uploads `17.2 MB` across fishing1. Upload byte volume is now the clearer dirty target. |
| 3 | Detail-tier attribution on remaining render waits | Medium | The metrics pass is active; use it to distinguish present serialization, upload, restore, compose, and event wait before changing render sequencing. |
| 4 | FG2-specific present pipeline | High | Current path still routes rendered entries through general display/update sequencing; detail counters should prove whether wait/upload ordering is serializing work. |
| 5 | Specialized PAL4 FG2 compositor | Medium | Fishing frames are modest, but larger scenes will make span/tile split and PAL4 conversion overhead more important. |

Non-goals:

| Non-goal | Reason |
|---|---|
| Frame dropping | Violates pixel-perfect playback. |
| Timing compression before throughput work | The measured playback is `1.55x` over target; shorter timing files would expose the same throughput bottleneck. |
| Default per-frame diagnostic logging or pad/SPI probes | `printf` and JCPAD/JCSPI debug sampling are useful tools, but they must be opt-in because screensaver playback is the product path. |
| Reintroducing FG1, ADS, or TTM runtime paths | Those were retired from the active public path. |
| Fixed island assumptions | The runtime must randomly place the island, so all optimizations must preserve scene-relative FG2 placement. |
| Direct framebuffer or progressive-mode experiments as first moves | Prior history says these were unstable. Exhaust stable scene playback first. |

## Current Runtime Shape

Active runtime files:

| File | Relevant role |
|---|---|
| `src/foreground_pilot.c` | FG2 selection, header/table/palette loading, frame timing, per-frame CD read, sound-event firing, scene loop. |
| `src/graphics_ps1.c` | RAM background tiles, clean-rect restore, PAL4/indexed8 FG2 span compositing, dirty-row upload, `LoadImage` calls. |
| `src/cdrom_ps1.c` | Sector reads, `CdSearchFile`, buffered read-into APIs, pack/resource loading. |
| `src/events_ps1.c` | VBlank pacing, pause input polling, and opt-in controller/SPI diagnostics via `pad-diag` / `pad-debug`. |
| `scripts/build-scene-foreground-pack.py` | FG2 pack compiler, base-diff crop, palette selection, row-span encoding, sound events. |
| `config/ps1/cd_layout.xml` | Routed FG2 files and active SCR/PSB/SND payloads. |

Current frame loop in `fgPlayOceanRuntimeScene`:

```c
while (foregroundPilotRuntimeActive()) {
    if (fgRuntimeCanHoldDisplayedFrame()) {
        fgRuntimeWaitHeldVBlank();
    } else {
        grBeginFrame();
        grRestoreBgFromRects();
        if (!fgRuntimeUsesBaseDiffBackdrop())
            fgBackdropTickBackgroundWaves();
        grUpdateDisplay(NULL, NULL, NULL);
        fgRuntimeMarkFrameRendered();
    }
    foregroundPilotRuntimeAdvance();
}
```

For base-diff FG2 packs, held VBlanks now wait without redraw. The current
stall is that the next entry is loaded only after its previous hold budget is
spent. That makes CD latency visible instead of hiding it under already-idle
held VBlanks.

Default playback diagnostics policy: keep `JCPERF2` Summary enabled only when
requested by perf boot mode, and keep controller/SPI probes behind
`pad-diag`/`pad-debug`. Pause polling stays live, but diagnostic counters,
register snapshots, and transition prints must not run by default.

## Measured Pack Evidence

These figures were computed from `generated/ps1/foreground/*.FG2` at baseline
commit `f704312c`.

Active routed packs:

| Pack set | Count | Total size | Max pack | Max frame payload | Encoding |
|---|---:|---:|---:|---:|---|
| Fishing routed high/low packs | 6 | 6.08 MB | 1.75 MB (`FISHING3.FG2`) | 9.1 KB | PAL4 |

All generated scene corpus:

| Pack set | Count | Total size | Max pack | Max frame payload | Encoding mix |
|---|---:|---:|---:|---:|---|
| All high/low FG2 packs | 126 | 342.75 MB | 32.93 MB (`MARY3LOW.FG2`) | 189.8 KB | 122 PAL4, 4 indexed8 |

Active pack details:

| Pack | Frames | Payload | Max frame | Avg visible pixels | Avg spans | Sound events |
|---|---:|---:|---:|---:|---:|---:|
| `FISHING1.FG2` | 156 | 807.2 KB | 7.0 KB | 4535 | 593 | 9 |
| `FISH1LOW.FG2` | 137 | 413.3 KB | 4.7 KB | 2445 | 336 | 9 |
| `FISHING2.FG2` | 295 | 1552.3 KB | 7.0 KB | 4708 | 597 | 9 |
| `FISH2LOW.FG2` | 247 | 760.8 KB | 4.8 KB | 2587 | 338 | 9 |
| `FISHING3.FG2` | 329 | 1782.2 KB | 9.1 KB | 4943 | 610 | 21 |
| `FISH3LOW.FG2` | 269 | 879.4 KB | 7.0 KB | 2853 | 352 | 21 |

Timing hold evidence:

| Pack | Entries | Target VBlanks | Avg hold | Max hold | Entries with hold > 1 |
|---|---:|---:|---:|---:|---:|
| `FISHING1.FG2` | 156 | 897 | 5.75 | 8 | 94.2% |
| `FISH1LOW.FG2` | 137 | 897 | 6.55 | 16 | 95.6% |
| `FISHING2.FG2` | 295 | 1473 | 4.99 | 8 | 90.8% |
| `FISH2LOW.FG2` | 247 | 1473 | 5.96 | 16 | 92.7% |
| `FISHING3.FG2` | 329 | 1636 | 4.97 | 11 | 94.2% |
| `FISH3LOW.FG2` | 269 | 1636 | 6.08 | 16 | 95.2% |

Interpretation: this hold distribution explains why the no-redraw held path
was such an important prerequisite. Those held VBlanks are now the budget that
prefetch can use to hide next-entry CD reads.

## Observed Runtime Baseline

Fishing1 low-tide/night/holiday sample from DuckStation TTY logging:

```text
JCPERF scene-start scene=fishing1 lowtide=1 night=1 holiday=4 raft=2 pos=-217,55
JCPERF scene-end scene=fishing1 scene_vb=1848 render=137 held=206 entries=137 late=137 max_elapsed=13
JCPERF timing advances=343 elapsed_vb=1670 target_vb=1077 max_hold=20 payload=423234 max_payload=4790
JCPERF cd reads=136 fail=0 bytes=423234 sectors=343 max_sectors=4 cd_vb=562 max_cd_vb=5
JCPERF gfx restore_calls=137 restore_bytes=28820416 compose_calls=136 rows=14605 spans=45705 pixels=332543 payload=423234 uploads=137 rects=548 upload_bytes=31861760 upload_vb=9 max_upload_vb=1
```

Derived read:

| Metric | Value | Meaning |
|---|---:|---|
| Inner playback ratio | `1670 / 1077 = 1.55x` | Matches the visual read that playback is about 50% too slow. |
| Full scene ratio | `1848 / 1077 = 1.72x` | Includes setup/teardown around active playback. |
| Inner overrun | `593` VBlanks | Time that must be removed or hidden to hit target. |
| CD read time | `562` VBlanks | Nearly the entire inner overrun. |
| Average CD read | `4.13` VBlanks | Blocking cost per small FG2 entry read. |
| Average rendered-entry time | `10.69` VBlanks | Render path cost after subtracting held waits. |
| Non-CD rendered-entry time | `6.58` VBlanks | Remaining restore/compose/upload/present/event budget. |
| Restore per rendered entry | `~205 KB` | RAM clean-rect copy volume. |
| Upload per rendered entry | `~227 KB` | VRAM upload volume. |

Important interpretation: the `4.13` VBlank CD read is not currently happening
during the held-frame wait. It happens after the previous frame's hold budget
has already expired, when `foregroundPilotRuntimeAdvance()` loads the next FG2
entry. Prefetch should move this work earlier so held time becomes useful.

## Dirty Restore And Upload Evidence

Current rect-mode backs up a scene-sized clean region, restores it every
VBlank, then marks the entire rect dirty. `grDrawBackground()` tracks dirty
state at tile row granularity, so a narrow dirty band can still become a full
320-pixel-wide row upload per tile.

Estimated current restore/upload cost under legal `VARPOS_OK` island positions:

| Pack | Current upload best | Current upload worst | Restore best | Restore worst |
|---|---:|---:|---:|---:|
| `FISHING1.FG2` | 216 KB | 315 KB | 195 KB | 288 KB |
| `FISH1LOW.FG2` | 216 KB | 315 KB | 205 KB | 307 KB |
| `FISHING2.FG2` | 216 KB | 315 KB | 183 KB | 276 KB |
| `FISH2LOW.FG2` | 216 KB | 315 KB | 194 KB | 290 KB |
| `FISHING3.FG2` | 263 KB | 358 KB | 239 KB | 334 KB |
| `FISH3LOW.FG2` | 263 KB | 358 KB | 239 KB | 334 KB |

Estimated exact per-frame row extent upload from the current FG2 span data:

| Pack | Avg exact row bytes | Max exact row bytes | Avg row rects |
|---|---:|---:|---:|
| `FISHING1.FG2` | 23.6 KB | 41.0 KB | 179 |
| `FISH1LOW.FG2` | 21.3 KB | 37.7 KB | 160 |
| `FISHING2.FG2` | 31.9 KB | 41.8 KB | 185 |
| `FISH2LOW.FG2` | 29.3 KB | 38.5 KB | 168 |
| `FISHING3.FG2` | 29.6 KB | 63.7 KB | 174 |
| `FISH3LOW.FG2` | 26.0 KB | 57.3 KB | 160 |

Interpretation: row/X-aware restore and upload remains a major candidate after
CD latency is hidden. A naive tile-level X-aware upload experiment that packed
partial-width rectangles through one scratch buffer failed to reach scene-end
metrics, likely because extra `DrawSync` points and unaligned transfer lengths
overwhelmed the byte savings. A later 16-pixel strip-batching variant also
failed before `JCPERF2` because it still synchronized each partial strip. A
32-pixel bucket arena variant then proved the opposite failure mode:
`upload_bytes 17172480 -> 4933312`, but `upload_rects 351 -> 11575` and
`upload_vb 5 -> 204`, regressing the loop. A rect-count-capped tile-band
variant kept rects near baseline (`351 -> 389`) and still regressed
`upload_vb 5 -> 305` because strided tile rows had to be CPU-copied into
contiguous scratch before `LoadImage`. The next viable upload-byte attempt
should be pack-time/direct-layout work, not runtime scratch packing.

## Guardrails

| Guardrail | Practical rule |
|---|---|
| Pixel-perfect means no skipped entries | Every FG2 entry must still be presented in order. |
| Faster means less work, not shorter truth | Timing can only be tightened after throughput improves. |
| One optimization at a time | Each change gets a clean build, DuckStation run, and human visual/audio check. |
| Scene-relative is mandatory | Island `xPos/yPos` must remain applied to FG2 overlays and backdrop sprites. |
| No per-frame allocation | All new buffers must be scene-persistent or static with bounded size. |
| No per-frame text I/O | `printf()` is available for gated setup/teardown probes, but not inside timing-critical playback loops. |
| Deterministic hardware means no adaptive correctness fallbacks | Every PS1 should take the same render path for the same scene/variant; non-CD fallback counters are failure tripwires. |
| Keep rollback easy | Each optimization should be a small commit with a single exit hatch. |

## Phase 0: Advanced Metrics Foundation

Goal: implement the advanced metrics layer once, before optimizing. The output
must be stable, machine-readable, scene-bounded, and rich enough to compare all
planned performance experiments without adding new log plumbing for each one.

Current status: implemented in `c0e6d95e`. `perf-log`, `perf-detail`, and
`perf-debug` are parsed; legacy `JCPERF` lines are preserved; versioned
`JCPERF2` scene-end records now cover scene identity, setup timing, loop timing,
frame complexity, CD shape, baseline prefetch placeholders, render/gfx counters,
buffer/allocation counters, and correctness tripwires.

Known intentional gaps: prefetch and async fields remain zero until those
experiments exist; heap free/min/largest are not wired in Summary; compositor
specialty counters such as `tile_splits`, `lut_pairs`, and `slow_pixels` remain
future experiment counters; `exe_bytes` stays a host-side build metric.

Metrics rules:

| Rule | Requirement |
|---|---|
| No per-frame printing | All logs are emitted at scene start/end or explicit setup checkpoints only. |
| Keep graphics clean | No visual overlays, debug pixels, or framebuffer text. |
| Gate everything | Release playback pays only cheap `if (ps1PerfEnabled)` checks. |
| Use fixed counters | No per-frame allocations, no dynamic strings, no file writes. |
| Prefer totals plus maxima | Scene totals, maxima, and buckets are enough; per-frame records are too expensive. |
| Preserve old fields initially | Keep current `JCPERF` fields until scripts/humans migrate to `JCPERF2`. |
| Version log lines | Add `schema=2` or `JCPERF2` prefix so future parsing is deterministic. |
| Separate hidden vs blocking time | Especially for prefetch: total CD work may remain, but due-frame blocking must fall. |
| Tier expensive probes | Subphase VSync probes, heap scans, and checksums must not run in normal `perf-log`. |

Metrics tiers:

| Tier | Boot token | Intended use | Allowed overhead |
|---|---|---|---|
| Off | none | Normal playback and release validation. | Only existing disabled `if (ps1PerfEnabled)` checks. |
| Summary | `perf-log` | Baseline comparisons and optimization acceptance. | Integer counters, maxima, buckets, and timing anchors already needed for scene control. |
| Detail | `perf-detail` | Short diagnostic runs when summary metrics show an unknown bottleneck. | Extra VSync tick reads around selected render/CD subphases. |
| Debug | `perf-debug` | Rare short-run investigation of correctness failures. | Optional checksums, extra assertions, or heap walks; never for speed comparison. |

Default optimization comparisons should use Summary tier. Detail tier exists so
we can diagnose the unknown `~6.6` non-CD VBlanks per rendered entry without
permanently baking that probe overhead into every benchmark.

Implemented scene-end log schema:

```text
JCPERF2 scene schema=2 scene=fishing1 pack=FG/FISH1LOW.FG2 pack_bytes=0 pack_padding=0 pack_lba=0 pack_sectors=0 fmt=fgp2_pal4 flags=base_diff,scene_relative frames=137 entries=137 sounds=9 lowtide=1 night=1 holiday=4 raft=2 pos=-217,55 seed=0
JCPERF2 timing scene_start=0 loop_start=0 loop_end=0 scene_end=0 scene_vb=1848 loop_vb=1670 target_vb=1077 overrun_vb=593 advances=343 render=137 held=206 entries=137 empty=0 late=137 max_elapsed=13 max_elapsed_idx=0
JCPERF2 setup setup_vb=0 screen_vb=0 backdrop_vb=0 pack_start_vb=0 clean_rect_vb=0 first_frame_vb=0 cleanup_vb=0 setup_reads=0 setup_bytes=0
JCPERF2 frame payload=423234 max_payload=4790 max_payload_idx=0 max_payload_src=0 rows=14605 spans=45705 pixels=332543 hold_max=20 hold_max_idx=0 hold_1=0 hold_2_4=0 hold_5_8=0 hold_9p=0 payload_0=0 payload_1k=0 payload_4k=0 payload_16k=0 payload_64k=0 payload_64kp=0
JCPERF2 cd reads=136 setup_reads=0 loop_reads=136 fail=0 setloc=136 bytes=423234 sectors=343 read_vb=562 setup_read_vb=0 loop_read_vb=562 blocking_vb=562 hidden_vb=0 max_read_vb=5 max_read_idx=0 max_read_sectors=4 unaligned_start=0 unaligned_end=0 overread_bytes=0 scratch_bytes=0 seq=0 seek_fwd=0 seek_back=0 max_gap=0 s1=0 s2=0 s3_4=0 s5_8=0 s9p=0
JCPERF2 prefetch policy=none buf=0 attempts=0 eligible=0 ineligible=0 hits=0 misses=0 due_misses=136 stage_hits=0 window_hits=0 group_hits=0 partial_hits=0 hidden_reads=0 blocking_reads=136 slack_vb=0 used_vb=0 overrun_vb=0 lead_min=0 lead_max=0 skipped_no_slack=0 skipped_busy=0 duplicate=0 wasted_bytes=0
JCPERF2 async async_start=0 async_poll=0 async_done=0 async_timeout=0 async_cancel=0 async_blocking_vb=0
JCPERF2 render render_vb=0 max_render_vb=0 max_render_idx=0 restore_vb=0 compose_vb=0 present_wait_vb=0 upload_vb=9 event_wait_vb=0 advance_vb=0 crossed_restore=0 crossed_compose=0 crossed_upload=0 crossed_advance=0
JCPERF2 gfx restore_calls=137 restore_bytes=28820416 max_restore_bytes=0 compose_calls=136 upload_calls=137 upload_rects=548 upload_bytes=31861760 max_upload_bytes=0 max_upload_rects=0 dirty_rows=0 dirty_exact_bytes=0 dirty_rounded_bytes=0 dirty_max_rows=0 dirty_max_exact=0 dirty_max_rounded=0 cap_hits=0 full_fallbacks=0
JCPERF2 heap start_free=0 end_free=0 min_free=0 largest_start=0 largest_end=0 framebuf=0 scratch=0 prefetch=0 peak_prefetch=0 alloc_fail=0 alloc_fail_bytes=0
JCPERF2 correctness trip=0 fallback=0 stale_guard=0 frame_mismatch=0 sound_events=0 sound_late=0 sound_cursor_end=0 last_frame=0 expected_frames=137 cd_fail=0
```

Zero fields now mean either "not applicable to this baseline policy" or
"reserved for the experiment that owns this counter." The schema should remain
stable while individual experiments populate their own fields.

Timing classification definitions:

| Field | Definition |
|---|---|
| `setup_reads`, `setup_read_vb` | CD reads before active scene playback loop starts. |
| `loop_reads`, `loop_read_vb` | CD reads during the active scene loop, including due-frame loads and prefetches. |
| `hidden_vb` | CD read VBlanks that occur before the entry is due and fit inside held-frame slack. |
| `blocking_vb` | CD read VBlanks that occur when an entry is due, or prefetch VBlanks that overrun available slack. |
| `slack_vb` | Held-frame VBlanks theoretically available for prefetch after preserving presentation timing. |
| `used_vb` | Held-frame slack actually spent doing prefetch work. |
| `overrun_vb` | Prefetch work that exceeded available slack and therefore became visible delay. |
| `loop_vb` | Active playback time only, excluding setup/cleanup. This is the main speed metric. |

Determinism policy:

| Condition | Policy |
|---|---|
| Non-CD fallback counter becomes nonzero | Experiment fails. Fix the implementation; do not accept a degraded alternate path. |
| Dirty cap would require full-screen fallback | Experiment fails unless the cap/merge policy is redesigned into a deterministic planned path. |
| Stale-pixel guard trips | Experiment fails. Pixel correctness is not negotiable. |
| Frame mismatch or sound cursor mismatch | Experiment fails. Timing pressure cannot alter presentation order. |
| CD read failure or unusually slow CD read | Log it as `cd_fail`, `blocking_vb`, or `overrun_vb`; do not change pixels, skip frames, or route to lower fidelity. |
| Memory allocation failure | Experiment fails. Buffers must be sized for the known deterministic scene class. |

Free precision upgrades:

| Area | Add without meaningful runtime cost | Why it helps |
|---|---|---|
| Raw timing anchors | `scene_start`, `loop_start`, `loop_end`, `scene_end` raw VSync ticks. | Allows later parsers to recompute durations and detect counter-wrap or classification mistakes. |
| Max-at-index markers | `max_*_idx`, `max_*_src`, `hold_max_idx`, `max_read_idx`, `max_render_idx`. | Turns "something spiked" into "frame/source entry N spiked" without per-frame logs. |
| Pack placement | `pack_lba`, `pack_sectors`, `pack_bytes`, `pack_padding`, entry count, sound count. | Explains seek behavior and pack-size regressions without inspecting the ISO manually. |
| Frame buckets | Hold and payload buckets. | Shows scene shape at a glance and catches large-scene class changes. |
| CD sector shape | Sequential count, forward/backward seeks, max sector gap, sector-count buckets. | Diagnoses whether latency is transaction count, seek pattern, or read size. |
| CD overread shape | Unaligned start/end counts, sector overread bytes, scratch-copy bytes. | Shows whether pack alignment/chunking can help before changing the format. |
| Prefetch eligibility | Eligible/ineligible, skipped-no-slack, skipped-busy, duplicate attempts, lead min/max. | Explains missed prefetch opportunities without expensive traces. |
| VBlank-cross flags | Count subphases that crossed at least one VBlank. | VBlank timers are coarse; crossing counts identify which phase actually burns frames. |
| Dirty maxima | Max exact/rounded dirty bytes and rows. | Locates worst frames and validates batching caps. |
| Heap high-water | Peak prefetch bytes, allocation failure size, min free, largest free. | Catches fragmentation and buffer-policy failures before long soaks. |

Do not add checksums to the normal hot path. If frame-identity debugging needs
checksums later, make them a separate debug token for short runs only. Cheap
identity markers such as frame index, source frame, payload size, and data
offset are enough for the metrics baseline.

Low-overhead implementation model:

| Source | Allowed metric style | Avoid |
|---|---|---|
| Values already in structs | Copy raw fields into counters: frame index, source frame, offsets, sizes, flags. | Re-reading files or reparsing payloads just for metrics. |
| Existing loops | Add integer totals, maxima, buckets, and reason counters. | Formatting strings or writing logs inside the loop. |
| Pack-start table scan | Compute scene-level pack shape once: max payload index, hold buckets, offset monotonicity, sector span. | Per-frame table rescans during playback. |
| CD read wrapper | Classify read phase, sector count, alignment, overread, sequential/seek direction. | Extra CD commands for measurement. |
| Render boundaries | Take raw VSync ticks at phase boundaries and count crossings. | Attempting cycle-level timing with expensive probes. |
| Allocation sites | Record requested bytes, buffer sizes, failures, min/largest free if already available. | Heap walking every frame. |

Metrics overhead red-team:

| Risk | Why it matters | Low-overhead plan |
|---|---|---|
| Subphase VSync probes perturb hot paths | Several `VSync(-1)` calls per rendered entry can alter the timing they measure. | Summary tier records bytes/calls and total loop timing; Detail tier enables subphase timers only for short diagnostic runs. |
| Heap free/largest-free queries can be expensive or unavailable | Heap walking can perturb scene setup and may hide fragmentation by changing allocation order if implemented poorly. | Record allocation request sizes and buffer sizes always; query free/largest only at scene start/end or when an existing heap probe already does it. |
| Pack-start table scan adds setup time | A metrics-only scan would inflate setup measurements. | Piggyback on the existing max-frame-size entry scan in `foregroundPilotRuntimeStart`; do not add a second scan. |
| `printf` format strings increase executable size | Many long format strings can grow the PS1 executable and data segment. | Keep all `JCPERF2` printing in `ps1_perf.c`; compile-gate advanced strings for release if binary size becomes tight. |
| Per-scene logs can grow during overnight runs | Screensavers may run indefinitely. | Keep logs to fixed scene-start/scene-end line count; use existing DuckStation log truncation; avoid per-frame lines. |
| Path/name strings can imply allocations | Copying arbitrary strings or building formatted paths would add risk. | Store compact static pack path pointers or fixed-size buffers only; never allocate strings for metrics. |
| Counters can overflow on long loops | A screensaver can run for hours, even though counters reset per scene. | Reset counters per scene and use `uint32` for totals; use saturated `uint16` only for documented maxima. |
| Metrics branches pollute hot code | Many scattered `if (ps1PerfEnabled)` checks can add code size and branch work. | Keep hot-path metric calls coarse and inline-noop when possible; prefer one guarded call per existing phase, not per pixel/span unless the loop already increments needed totals. |
| Checksums would be expensive | Payload or framebuffer checksums require reading data solely for metrics. | Exclude checksums from Summary/Detail; allow only under `perf-debug` for short correctness investigations. |
| Metrics can retain scene memory accidentally | Any allocated metric buffer would become another long-run leak risk. | Metrics module owns no heap allocations; all state is one static counter struct reset at scene start/end. |

Memory-safety rules for metrics implementation:

| Rule | Requirement |
|---|---|
| Static state only | `ps1_perf.c` owns one fixed-size counter struct and no heap allocations. |
| No retained pointers to scene buffers | Store sizes, offsets, IDs, and fixed path labels; never store pointers to frame buffers, scratch buffers, or prefetch buffers. |
| Reset on scene start and when disabling perf | `ps1PerfBeginScene()` and `ps1PerfSetEnabled(0)` must clear all counters. |
| No ownership changes | Metrics cannot allocate, free, resize, or transfer ownership of runtime buffers. |
| Failure paths still mark and clear | Allocation/read/render failures should increment tripwire counters and still run normal cleanup; they are not acceptable fallbacks. |
| Detail/debug data bounded | Any future per-frame ring buffer must be compile- or boot-token gated, fixed-size, and disabled by default. |

Performance-safety update plan:

| Step | Work | Acceptance |
|---|---|---|
| `S0-01` | Add `ps1PerfLevel` with Off/Summary/Detail/Debug. | Existing `perf-log` maps to Summary; no token maps to Off. |
| `S0-02` | Keep Summary metrics to cheap counters only: identity, loop timing anchors, CD shape, prefetch decisions, payload/hold buckets, bytes/calls, correctness counters. | Summary run changes playback no more than current `perf-log` within normal emulator variance. |
| `S0-03` | Put render subphase VSync timers behind Detail tier. | `restore_vb`, `compose_vb`, `present_wait_vb`, `event_wait_vb`, and `crossed_*` are zero or omitted in Summary; populated in Detail. |
| `S0-04` | Put heap largest-free scans and checksums behind Detail/Debug only unless an existing cheap API already exposes them. | Summary never walks heap or scans payloads solely for metrics. |
| `S0-05` | Keep `JCPERF2` schema stable across tiers by printing unavailable fields as `0` or `na`. | Parsers do not need different schemas per tier. |
| `S0-06` | Compare Off vs Summary fishing1 once after implementation. | Confirms Summary overhead is small and quantified before using it for optimization decisions. |
| `S0-07` | Compare Summary vs Detail only for short diagnostic runs. | Detail is used for attribution, not final speed acceptance. |

Metric precision audit:

| Metric group | Current/obvious version | Higher-precision no-overhead version | Overhead class |
|---|---|---|---|
| Scene identity | Scene name and variant flags. | Add pack path, pack LBA/sectors, pack byte size, format flags, entry/frame/sound counts, island position, seed. | Scene start only. |
| Scene timing | Total scene VBlanks. | Add raw start/end ticks, loop-only ticks, setup-only ticks, overrun, max elapsed with frame index. | Existing VSync reads. |
| Loop shape | Render/held counts. | Add empty entries, hold buckets, max hold index, late count and worst late index. | Existing advance path. |
| Setup timing | One setup total. | Split screen load, backdrop, pack start, clean-rect save, first-frame load, cleanup. | Existing setup boundaries. |
| Frame complexity | Total payload/spans/pixels. | Add max payload index/source frame, payload buckets, row/span/pixel totals, empty counts. | Existing entry load/compose path. |
| CD base | Read count, bytes, sectors, VBlanks. | Add setup vs loop reads, setloc count, max read index, sector buckets, sequential/forward/backward seeks, max gap. | Existing CD wrapper. |
| CD alignment | Unaligned count. | Split unaligned start/end, overread bytes, scratch-copy bytes, max read sectors. | Existing offset/size math. |
| CD blocking | One read VBlank total. | Split setup, hidden prefetch, blocking due-frame, async-blocking, and prefetch overrun VBlanks. | Existing phase/prefetch state. |
| Prefetch | Hits and misses. | Add eligibility, skipped reasons, lead min/max, stage/window/group hit types, partial hits, duplicate attempts, wasted bytes. | Existing prefetch decisions. |
| Async | Done or not done. | Add starts, polls, completions, timeouts, cancels, blocking completion VBlanks. | Existing async state machine. |
| Render timing | One render total. | Summary: render counts and max elapsed index. Detail: split restore, compose, present wait, upload, event wait, advance/load, max render index, crossing counts. | Detail tier only for subphase VSync probes. |
| Dirty precision | Upload bytes. | Add exact vs rounded dirty bytes/rows, max dirty rows/bytes, cap hits, and full-fallback tripwire counts that must remain zero. | Existing dirty marking/batching. |
| Restore/upload | Totals. | Add maxima for restore bytes, upload bytes, upload rects, and worst-frame index where available. | Existing restore/upload paths. |
| Compositor | Calls/spans/pixels. | Add tile splits, LUT pairs, slow pixels, clipped spans, odd-edge counts if paths exist. | Existing compositor branches. |
| Heap | End free memory. | Summary: buffer sizes, allocation requests/failures, peak prefetch. Detail: free/largest free if available cheaply. | Scene start/end and alloc sites only. |
| Correctness | CD fail only. | Add frame mismatch, deterministic tripwire counts, sound cursor end, last frame, expected frames. | Existing guard branches; non-CD tripwires must remain zero. |
| Binary/build | Not logged. | Record `exe_bytes` offline with build artifacts, not from PS1 runtime. | Host-side only. |

Required metrics inventory:

| Metric group | Fields | Required before | Why |
|---|---|---|---|
| Scene identity | `scene`, `pack`, `pack_lba`, `pack_sectors`, `pack_bytes`, `pack_padding`, `fmt`, `flags`, frame/entry/sound counts, variant flags, island position, seed | All experiments | Ensures comparisons are against the same scene, pack, tide, night, holiday, raft, island placement, and physical CD location. |
| Scene timing | `scene_start`, `loop_start`, `loop_end`, `scene_end`, `scene_vb`, `loop_vb`, `target_vb`, `overrun_vb`, `advances`, `late`, `max_elapsed`, `max_elapsed_idx` | All experiments | Top-level success metric: playback must move toward `1.0x` target without timing lies, and spikes must name a frame. |
| Loop shape | `render`, `held`, `entries`, `empty`, `hold_*` buckets, `hold_max_idx` | Prefetch, held-path validation | Confirms an optimization did not skip entries or change hold semantics. |
| Setup timing | `setup_vb`, `screen_vb`, `backdrop_vb`, `pack_start_vb`, `clean_rect_vb`, `first_frame_vb`, `cleanup_vb`, `setup_reads`, `setup_bytes` | Persistent resources, CD layout, startup work | Separates scene transition cost from active playback cost. |
| Frame complexity | `payload`, `max_payload`, `max_payload_idx`, `max_payload_src`, `rows`, `spans`, `pixels`, payload buckets | Pack format, compositor, large-scene strategy | Normalizes speed against actual work in each scene and identifies the worst entry. |
| CD base | `reads`, `setup_reads`, `loop_reads`, `setloc`, `fail`, `bytes`, `sectors`, `read_vb`, `setup_read_vb`, `loop_read_vb`, `max_read_vb`, `max_read_idx`, `max_read_sectors`, `unaligned_start`, `unaligned_end`, `overread_bytes`, `scratch_bytes`, seek/sector buckets | Prefetch, stream windows, pack grouping | Distinguishes setup reads, playback reads, read count, read size, sector alignment, seek pattern, and scratch-copy cost. |
| CD blocking split | `blocking_vb`, `hidden_vb`, `blocking_reads`, `hidden_reads` | Any prefetch experiment | Measures whether CD work was hidden under held VBlanks or still delayed due frames. |
| Prefetch | `policy`, `buf`, `attempts`, `eligible`, `ineligible`, `hits`, `misses`, `due_misses`, `stage_hits`, `window_hits`, `group_hits`, `partial_hits`, `slack_vb`, `used_vb`, `overrun_vb`, `lead_min`, `lead_max`, `wasted_bytes`, `skipped_*`, `duplicate` | One-entry staging, stream windows, async prefetch | Needed to debug why prefetch did or did not help. |
| Async prefetch | `async_start`, `async_poll`, `async_done`, `async_timeout`, `async_cancel`, `async_blocking_vb` | Async CD experiments | Keeps controller-state risk visible before enabling async broadly. |
| Render subphases | `render_vb`, `max_render_vb`, `max_render_idx`, `restore_vb`, `compose_vb`, `present_wait_vb`, `upload_vb`, `event_wait_vb`, `advance_vb`, `crossed_*` | Present pipeline, dirty upload, compositor work | Splits the remaining `~6.6` non-CD VBlanks per rendered entry and identifies phases that cross frame boundaries. |
| Dirty precision | `dirty_rows`, `dirty_exact_bytes`, `dirty_rounded_bytes`, `dirty_tiles`, `dirty_max_*`, `cap_hits`, `full_fallbacks` | Row/X dirty restore and upload | Proves byte reductions are not offset by too many `LoadImage` rects; `full_fallbacks` must remain zero in accepted builds. |
| Restore/upload | `restore_calls`, `restore_bytes`, `max_restore_bytes`, `upload_calls`, `upload_rects`, `upload_bytes`, `max_upload_bytes`, `max_upload_rects` | Dirty upload, present pipeline | Current first-pass metrics; keep them stable and add worst-case bounds. |
| Compositor | `compose_calls`, `compose_rows`, `compose_spans`, `compose_pixels`, `compose_payload`, `tile_splits`, `lut_pairs`, `slow_pixels` | PAL4/indexed8 compositor specialization | Shows same pixels/spans with lower time or fewer slow-path operations. |
| Heap | `start_free`, `end_free`, `min_free`, `largest_start`, `largest_end`, buffer sizes, `peak_prefetch`, `alloc_fail`, `alloc_fail_bytes` | Prefetch buffers, persistent resources, large scenes | Prevents reintroducing the fishing3 long-run memory failure and shows the failed allocation size. |
| Correctness guard | `trip`, `fallback`, `frame_mismatch`, `stale_guard`, `sound_events`, `sound_late`, `sound_cursor_end`, `last_frame`, `expected_frames`, `cd_fail` | All experiments | Tripwire counters must stay zero except `cd_fail`, which indicates media/read failure rather than an acceptable alternate render path. |
| Binary/build | `exe_bytes`, optional map hot symbols offline | Build flags, code cleanup | Tracks binary growth from metrics and optimization code. |

Implementation status for the metrics pass:

| Step | Status | Work | Acceptance |
|---|---|---|---|
| `M0-01` | Done | Introduce `ps1PerfLevel` and `JCPERF2` scene-end schema alongside existing `JCPERF`. | Existing fishing1 numbers still appear; new lines parse as key/value pairs; no token remains Off. |
| `M0-02` | Done | Add Summary-tier setup/loop/timing counters and explicit `loop_vb` measurement. | Can separate scene setup from playback without deriving it manually. |
| `M0-03` | Done | Add Summary-tier CD blocking/hidden split and prefetch fields initialized to zero. | Baseline says `policy=none`, `hidden_vb=0`, `due_misses=entries_with_payload`. |
| `M0-04` | Done | Add Summary-tier CD shape precision: alignment, overread, sector buckets, sequential/seek direction, max read index. | Can tell whether runtime prefetch or pack layout is the right CD fix. |
| `M0-05` | Done | Add Summary-tier dirty/compositor complexity counters that piggyback existing loops. | Row/X and compositor experiments have before/after data without extra scans. |
| `M0-06` | Partial | Add Summary-tier heap/buffer counters at scene start/end and allocation sites only. | Buffer sizes and allocation failures are logged; free/min/largest remain reserved until a cheap heap probe is chosen. |
| `M0-07` | Done | Add Summary-tier deterministic tripwire counters. | Future speedups cannot silently enter fallback behavior; any nonzero non-CD tripwire fails the experiment. |
| `M0-08` | Done | Add Detail-tier render subphase timers around restore, compose, present wait, upload, event wait, and advance/load. | The current non-CD `~6.6` VBlank/render budget can be attributed when needed. |
| `M0-09` | Pending | Run Off vs Summary fishing1 comparison. | Summary overhead is documented before using it for optimization decisions. |
| `M0-10` | Pending | Run Summary baseline matrix: fishing1 default, fishing1 low/night/holiday, fishing2, fishing3. | Produces comparable `JCPERF2` records before the first optimization. |

Metrics pass non-goals:

| Non-goal | Reason |
|---|---|
| Cycle-accurate profiling | VBlank-resolution plus byte/work counters are enough for these scene-level decisions. |
| Per-frame log records | Would explode logs and perturb timing. Use buckets and maxima. |
| Visual debug UI | We now have `printf`; graphics must stay pixel-clean. |
| Hardware CD benchmarking claims | DuckStation logs guide optimization; hardware validation remains separate. |

## Phase 1: Held-Entry No-Work Path

Goal: render each FG2 entry once, then hold the framebuffer unchanged for the
remaining VBlanks.

| ID | Task | Rationale |
|---|---|---|
| `P1-01` | Add a base-diff-only no-redraw hold path in `fgPlayOceanRuntimeScene`. | Base-diff packs already include visible background changes, and runtime waves are disabled. |
| `P1-02` | Track whether the current FG2 entry has already been rendered. | Prevent repeated restore/compose/upload for held VBlanks. |
| `P1-03` | Add a display-only wait path that handles `VSync` and controller polling without `LoadImage`. | Preserve timing and input while doing no visual work. |
| `P1-04` | Compare frame index, source frame, and sound-event timing before/after. | Avoid subtle SFX drift. |
| `P1-05` | Validate with `fishing1` high, low, holiday, night, raft, and forced island positions. | First acceptance scene must stay perfect. |
| `P1-06` | Validate with `fishing2` and `fishing3`. | Ensure longer scenes and larger payloads still advance correctly. |

Status: implemented before the measured `JCPERF` sample. Fishing1 now shows
`render=137` and `held=206`, confirming the runtime no longer redraws every
held VBlank. It does not skip any FG2 entry.

Red-team risks:

| Risk | Mitigation |
|---|---|
| Sound fires one hold too early or late | Keep `fgRuntimeLoadSceneFrame` timing unchanged; verify splash/catch cues. |
| Pause/debug input feels less responsive | Call the input wait path on every hold VBlank. |
| Non-base-diff scenes need animated runtime background | Gate the no-work path to base-diff FG2 only. |
| Last-frame hold/end behavior changes | Test scene completion and screensaver loop repeat. |

## Phase 2: Row/X-Aware Dirty Restore And Upload

Goal: stop restoring and uploading full scene-sized clean rects when the actual
FG2 spans touch a much smaller row/X set.

Status: second restore-only slice implemented on the perf branch, with a later
PAL4 compositor cleanup reducing dirty-marker calls without changing the dirty
region. Dirty tracking now carries per-row X extents for current and previous
dirty state, so RAM clean-background restore copies only the exact previous
dirty row spans. The upload path still uses the proven full-row batching.
Fishing1 improved from the original `loop_vb=1426` to `1243` and
`restore_bytes=16035840` to `2510092`; next work is X-aware upload batching.

| ID | Task | Rationale |
|---|---|---|
| `P2-01` | Done: add per-tile `minX/maxX` alongside `minY/maxY`. | Preserve useful horizontal precision from FG2 spans without changing upload yet. |
| `P2-02` | Done: keep `curr` and `prev` X/Y extents per tile. | Restore previous pixels using narrower RAM copies; upload still uses row bands. |
| `P2-03` | Done: change `grMarkRectDirty` to update X extents, not just row bands. | Existing callers get better restore precision automatically. |
| `P2-04` | Done: clean-rect restore copies only previous X extents. | Avoid full-width RAM clean copies for narrow dirty bands. |
| `P2-04a` | Done: track previous/current X extents per tile row and restore exact previous row spans. | `restore_bytes 9520664 -> 2510092`, `loop_vb 1296 -> 1266`; upload remains unchanged and conservative. |
| `P2-04b` | Done: aggregate PAL4 dirty marking once per encoded row and per tile. | `loop_vb 1254 -> 1248` and `blocking_vb 35 -> 21` while `restore_bytes` and `upload_bytes` stayed unchanged. |
| `P2-05` | Failed naive per-partial-sync, byte-minimal arena, and rect-capped tile-band runtime upload. | Runtime scratch packing loses to full-width direct tile uploads even when bytes fall. |
| `P2-06` | Move future X-aware upload work to pack-time/direct-layout design. | Lower upload bytes only if the source is already contiguous enough for `LoadImage`; do not repack rows during playback. |
| `P2-07` | Keep deterministic merge/cap policy for any future upload-ready format. | Avoid command overhead spikes on dense frames without routing to an alternate render fallback. |
| `P2-08` | Record bytes actually uploaded in telemetry. | Confirm real win. |

Expected impact: restore byte volume is no longer the primary dirty bottleneck
for fishing1; upload still moves full dirty rows (`17172480` bytes in the
accepted run). A practical upload batching strategy should land between the
full-row upload number and exact row-span bytes without exploding `LoadImage`
rect count.

Red-team risks:

| Risk | Mitigation |
|---|---|
| Stale pixels from under-restoring | Restore the union of previous frame row extents, not just current frame. |
| `LoadImage` command overhead beats byte savings | Use bucketed row merging and a rectangle cap. |
| Clean source rect does not cover a dirty row | Fail the experiment and fix bounds/capture; do not accept a full-rect runtime fallback. |
| Random island placement moves bounds negative or across tile splits | Clamp after applying `islandState.xPos/yPos`, and test extreme legal positions. |

## Phase 3: Specialized FG2 Compositors

Goal: make the compositor match the FG2 contract instead of treating every
span like an arbitrary transparent sprite.

| ID | Task | Rationale |
|---|---|---|
| `P3-01` | Split PAL4 span compositing by tile once per span, not once per pixel. | Remove thousands of repeated tile tests. |
| `P3-02` | Remove transparent-index checks inside FG2 spans. | Pack compiler only emits nonzero contiguous spans. |
| `P3-03` | Failed in runtime form: add a 256-entry PAL4 byte-to-two-pixels LUT per FG2 palette. | The runtime cache/table path regressed `loop_vb 1266 -> 1279`; revisit only as pack-time direct16 or cheaper scene specialization. |
| `P3-04` | Failed with `P3-03`: use aligned 32-bit stores for even destination X and even pixel count. | The combined LUT/store path was slower than direct halfword writes on fishing1. |
| `P3-05` | Keep odd-left and odd-right edge handlers simple. | Preserve exact pixels around unaligned spans. |
| `P3-06` | Add an indexed8 fast path with direct palette lookup and no transparent checks. | Future scenes include indexed8 FG2 packs. |
| `P3-07` | Consider pack-time direct16 row commands for high-cost scenes only. | Removes palette work at runtime but increases pack size. |
| `P3-08` | Keep the old compositor behind a debug token until the new path is validated. | Fast rollback. |
| `P3-09` | Done: keep PAL4 dirty marking outside the per-span compositor and split it per tile row. | Removes redundant `grMarkRectDirty` calls without widening restore extents across the 320px tile boundary. |
| `P3-10` | Failed: inline the PAL4 span compositor into the decoded row loop. | Work identity stayed stable, but `loop_vb 1243 -> 1249`, `blocking_vb 21 -> 23`, and `prefetch_overrun_vb 12 -> 14`; local helper inlining lost to code-shape effects. |
| `P3-11` | Done: fast-path PAL4 spans wholly inside one 320px tile. | Reproduced `loop_vb 1242 -> 1240` with stable visual-work counters; this keeps the generic path for cross-tile/clipped spans but avoids repeated tile setup for the common tile-local case. |

Expected impact: active frames average only 2.4K-4.9K visible pixels, so this
is likely behind held-frame and dirty-upload fixes for fishing. It becomes more
important for scenes like Suzy and Mary with 100K+ visible pixels per frame.

## Phase 4: FG2 Streaming, Prefetch, And CD Access

Goal: reduce synchronous CD operations without reintroducing the fragile
read-ahead behavior called out in the historical timing plan. The first target
is to move next-entry reads into already-idle held VBlanks.

Status: first wave implemented, visually signed off, and merged to `main` in
`1b457163`. Stage1 entry prefetch is default. The perf branch now uses a
16 KB stream window after the post-row-restore slack/window combination test. The old
`prefetch-stage1` token is no longer required for the normal path;
`no-prefetch`, `no-stage1`, and window-size tokens remain diagnostic controls.

Current perf-branch target: keep squeezing CD latency and upload cost without
changing pixels. After x-aware restore, PAL4 span compositing, duplicate probe
removal, guarded fallthrough, pad/SPI diagnostic gating, row-level dirty
restore, the `16 KB`/`3` VBlank post-restore retune, per-tile row dirty
marking, the `6` VBlank fallthrough guard, the base-diff OT-clear skip, and
the tile-local PAL4 span fast path, fishing1 high-tide reports
`loop_vb=1240`, `blocking_vb=20`, `due_misses=1`, and prefetch
`overrun_vb=11`. Row-level restore created enough
CPU headroom that CD blocking fell too; the latest dirty-marker cleanup
converted redundant span-side dirty work into more useful prefetch coverage.
Next experiments should target the remaining blocking, bounded refill overrun,
and upload byte volume.

| ID | Task | Rationale |
|---|---|---|
| `P4-01` | Done: add a next-entry prefetch state machine to the held-frame path. | Current blocking read happens after hold time expires; start it while the previous frame is still being held. |
| `P4-02` | Done: stage exactly one next FG2 entry into a second scene-persistent buffer. | Lowest-risk prefetch: preserves current entry identity and avoids window complexity. |
| `P4-03` | Done: swap staged next-entry data into `currentFrameData` when `foregroundPilotRuntimeAdvance()` reaches that entry. | Due-frame path becomes a RAM pointer swap instead of a CD read. |
| `P4-04` | Done: add a synchronous one-entry prefetch experiment during held waits before attempting async CD reads. | Proves frame identity and memory behavior with minimal controller-state risk. |
| `P4-05` | Done: add a scene-local stream window over the current FG2 file. | Entries are sequential; one 32-64 KB read can cover many small fishing frames. |
| `P4-06` | Done: serve entries directly from the stream window when fully resident. | Avoid per-entry `CdControl`, `CdRead`, sector scratch copy, and `CdReadSync`. |
| `P4-07` | Done: refill the stream window only during held VBlanks when enough hold budget remains. | Hides CD latency instead of shifting it to the due frame. |
| `P4-08` | Use the original blocking direct read when a frame is due and not resident. | Preserves correctness under short holds, large frames, or missed prefetch; this is timing pressure, not a lower-fidelity fallback. |
| `P4-09` | Next: add deterministic scene-class policy: one-entry staging for small scenes, stream window for sequential small entries, direct read for huge outliers. | Fishing and Mary/Suzy likely need different memory/latency tradeoffs, but the chosen path must be deterministic for a given scene class. |
| `P4-10` | Done: track prefetch hit/miss and hidden/blocking VBlank counters in `JCPERF2`. | Confirm whether CD VBlanks were hidden, not merely moved. |
| `P4-11` | Test true async CD prefetch only after synchronous staging/windowing is correct. | Prior naive read-ahead failed; isolate correctness before adding concurrency. |
| `P4-12` | Align FG2 data chunks or pack-window boundaries as an optional pack mode. | Helps only after runtime windowing proves physical sector layout is limiting. |
| `P4-13` | Keep all prefetch buffers scene-persistent and bounded. | Preserve the fishing3 memory-leak fix and avoid fragmentation. |
| `P4-14` | Avoid cross-file prefetch as a first pass. | Current measured stall is inside one FG2 file, not between scene files. |
| `P4-15` | Done: require at least `3` held VBlanks before starting a stream-window refill. | Avoids short-slack reads that become visible delay; `6` VBlanks was too strict and raised due misses. |
| `P4-16` | Done: change the default stream window from `32 KB` to `24 KB` after the post-slack sweep. | `24 KB` improved `loop_vb 1325 -> 1322` and `prefetch_overrun_vb 67 -> 58`; `20 KB` and `28 KB` lost. |
| `P4-17` | Done: retune the default stream window from `24 KB` to `20 KB` after pad/SPI diagnostics were gated off. | `20 KB` improved `loop_vb 1317 -> 1312`, `blocking_vb 93 -> 91`, and `prefetch_overrun_vb 41 -> 37`; due misses rose `8 -> 11`. |
| `P4-18` | Done: lower the stream-window refill guard from `3` to `2` VBlanks after the 20 KB retune. | `loop_vb 1312 -> 1300`, `blocking_vb 91 -> 66`, `due_misses 11 -> 4`, with bounded `prefetch_overrun_vb 37 -> 45`. |
| `P4-19` | Failed: lower the stream-window refill guard from `2` to `1` VBlank. | `due_misses 4 -> 2` did not compensate for `loop_vb 1300 -> 1312` and `prefetch_overrun_vb 45 -> 62`; keep the `2` VBlank default. |
| `P4-20` | Failed as a no-op: lower staged-copy fallthrough from `5` to `4` VBlanks under the post-merge baseline. | Key metrics stayed identical; keep the stricter `5` VBlank default until another timing change makes this guard matter again. |
| `P4-21` | Done: retune the default stream window from `20 KB` to `18 KB` after the 2 VBlank guard. | `loop_vb 1300 -> 1296` and `prefetch_overrun_vb 45 -> 33`; `blocking_vb 66 -> 75` and `due_misses 4 -> 8` are the next CD target. |
| `P4-22` | Failed: lower SPI pad polling from `250 Hz` to `125 Hz` or `65 Hz`. | CD submetrics improved slightly, but total `loop_vb` regressed `1296 -> 1297`; keep input timing at the known-good rate unless a dedicated IRQ/input harness proves a better tradeoff. |
| `P4-23` | Done via dirty pipeline: restore exact per-row X extents before the next CD attempt. | `loop_vb 1296 -> 1266`, `blocking_vb 75 -> 36`, `due_misses 8 -> 1`; reduced RAM restore work created more usable held-frame slack for existing prefetch policy. |
| `P4-24` | Failed: re-sweep `16/20/24/32 KB` stream windows after row-level restore under the `2` VBlank guard. | All tested sizes regressed total loop (`1270`, `1276`, `1281`, `1285`) versus the then-accepted `18 KB` baseline (`1266`); the `16 KB` point only became useful after the later `3` VBlank guard. |
| `P4-25` | Failed as a no-op: targeted clearing for row-level dirty state. | Key metrics stayed identical at VBlank resolution; retry only with finer CPU counters or during a broader dirty-state refactor. |
| `P4-26` | Failed gate: raise the post-row-restore refill guard from `2` to `3` VBlanks. | `loop_vb 1266 -> 1257` and `prefetch_overrun_vb 26 -> 10`, but `blocking_vb 36 -> 56` and `due_misses 1 -> 9`; this is a useful starvation signal, not an accepted default. |
| `P4-27` | Done: pair the post-row-restore `16 KB` window with the `3` VBlank refill guard. | `loop_vb 1266 -> 1254`, `blocking_vb 36 -> 35`, and `prefetch_overrun_vb 26 -> 6`; extra `due_misses 1 -> 6` are bounded and now the next CD target. |
| `P4-28` | Failed: lower staged-copy fallthrough from `5` to `4` VBlanks under the `16 KB`/`3` VBlank baseline. | `blocking_vb 35 -> 31` and `due_misses 6 -> 5`, but total `loop_vb 1254 -> 1264`; keep the `5` VBlank guard because playback speed is the primary gate. |
| `P4-29` | Failed: use a `2` VBlank guard only for immediate next-frame window staging while keeping lookahead at `3` VBlanks. | `due_misses 6 -> 1` and `blocking_vb 35 -> 27`, but `loop_vb 1254 -> 1268` and `prefetch_overrun_vb 6 -> 22`; immediate short reads still need cheaper grouping. |
| `P4-30` | Done via compositor/dirty pipeline: aggregate PAL4 dirty marks per tile row. | `loop_vb 1254 -> 1248`, `blocking_vb 35 -> 21`, and `due_misses 6 -> 1`; `prefetch_overrun_vb 6 -> 15` is the next CD smoothing target. |
| `P4-31` | Failed: raise the post-row-dirty refill guard from `3` to `4` VBlanks. | `loop_vb 1248 -> 1244` and `prefetch_overrun_vb 15 -> 12`, but `blocking_vb 21 -> 36` and `due_misses 1 -> 5`; keep visible blocking as a hard gate. |
| `P4-32` | Done: raise the staged-copy fallthrough guard from `5` to `6` VBlanks after per-tile row dirty marking. | `loop_vb 1248 -> 1243`, `prefetch_overrun_vb 15 -> 12`, and `blocking_vb` stayed `21`. |
| `P4-33` | Failed: re-sweep `15 KB` and `14 KB` stream windows after the `6` VBlank fallthrough guard. | `15 KB` rounded to the current behavior; `14 KB` reduced `prefetch_overrun_vb 12 -> 9` but regressed `loop_vb 1243 -> 1244` and `blocking_vb 21 -> 36`. |
| `P4-34` | Failed as no-op: hoist the PAL4 dirty visible-row check out of the per-span loop. | Key metrics stayed exactly flat at VBlank resolution; retry only with finer CPU counters or combined compositor branch cleanup. |
| `P4-35` | Failed: call `markTileDirtyRect()` directly from PAL4 row dirty aggregation. | The direct path regressed `loop_vb 1243 -> 1248`, `blocking_vb 21 -> 26`, and `prefetch_overrun_vb 12 -> 17`; generic `grMarkRectDirty()` stays faster in this build. |
| `P4-36` | Failed via present pipeline: compose FG2 RAM tiles before `VSync(0)`. | Correctness stayed clean, but `loop_vb 1243 -> 1248`, `blocking_vb 21 -> 39`, `due_misses 1 -> 4`, and `prefetch_overrun_vb 12 -> 15`; render sequencing is coupled to the current prefetch cadence. |
| `P4-37` | Failed: raise the staged-copy fallthrough guard from `6` to `7` VBlanks. | `blocking_vb` stayed `21`, but `loop_vb 1243 -> 1246` and `prefetch_overrun_vb 12 -> 13`; keep the local optimum at `6` VBlanks. |
| `P4-38` | Failed: point due window hits directly at stream-window bytes instead of copying to `frameBuffer`. | Correctness stayed clean, but `loop_vb 1243 -> 1250`, `blocking_vb 21 -> 26`, and `prefetch_overrun_vb 12 -> 19`; the explicit copy remains faster in this code shape. |
| `P4-39` | Failed as no-op: use a circular FG2 stream-window head to avoid most refill `memmove()` calls. | Correctness stayed clean, but `loop_vb=1243`, `blocking_vb=21`, `prefetch_overrun_vb=12`, `hits=154`, and `due_misses=1` matched baseline exactly; the next CD target needs cheaper/grouped reads, not RAM compaction. |
| `P4-40` | Rejected: skip `Setloc` for sequential aligned CD reads. | The old gate passed (`loop_vb 1243 -> 1233`, `blocking_vb 21 -> 16`, `setloc 76 -> 9`), but the visual workload collapsed (`compose_calls 155 -> 4`, `upload_calls 156 -> 14`), so this is an invalid speedup and the gate needs work-identity checks. |
| `P4-41` | Failed: shrink stream-window reads only under tight held-frame slack. | `loop_vb 1243 -> 1242` and `prefetch_overrun_vb 12 -> 6`, but `blocking_vb 21 -> 36` and `due_misses 1 -> 5`; reading less proves the overrun source but sacrifices coverage. |
| `P4-42` | Failed as no-op: fast-return from zero-delay event waits after Start polling. | Key metrics matched baseline exactly; retry only with finer CPU counters or when cleaning up event/pause code shape. |
| `P4-43` | Rejected after red-team review: skip the pre-upload present wait after a held-loop VBlank. | Headless timing improved (`loop_vb 1243 -> 1239`, `blocking_vb 21 -> 20`), but the safety proof is insufficient because frame load/compose can consume the earlier VBlank before `LoadImage`; retry only with an explicit scanline-safe present scheduler. |
| `P4-44` | Done: skip `grBeginFrame()` OT reset on base-diff FG2 frames. | Reproduced `loop_vb 1243 -> 1242` and `blocking_vb 21 -> 20` with unchanged visual-work counters; keep OT reset for non-base-diff wave/legacy primitive paths. |
| `P4-45` | Failed: re-test an `18 KB` stream window after the OT-clear cleanup. | `loop_vb 1242 -> 1251`, `blocking_vb 20 -> 42`, and `due_misses 1 -> 4`; keep `16 KB` as the current local window knee. The harness now fails baseline-label mismatches so parameter probes cannot silently skip comparisons. |
| `P4-46` | Failed as no-op: call `foregroundPilotRuntimeCompose()` unconditionally from `grUpdateDisplay()`. | The compose function already guards inactive runtime state, but removing the outer active check left all key VBlank metrics unchanged at the current baseline. |
| `P4-47` | Failed as no-op: guard holiday stamping at the compose call site for non-holiday scenes. | Fishing1 has `holiday=0`, but avoiding the no-op helper call did not move any VBlank-level metric. |
| `P4-48` | Failed: replace restore-row `memcpy()` with a local aligned 32-bit copy helper. | `loop_vb` stayed flat, but `blocking_vb 20 -> 22` and `prefetch_overrun_vb 12 -> 14`; keep libc `memcpy` for restore rows. |
| `P4-49` | Failed as no-op: cache the base-diff header flag in runtime state. | Replacing repeated `header.reserved0` masks with a cached byte left all key timing and work-identity metrics unchanged at `loop_vb=1242`; retry only with finer CPU counters or as cleanup. |
| `P4-50` | Failed as no-op: exact-width upload for single dirty tile rows. | Fishing1 produced no measurable single-row upload bands; timing, `upload_bytes`, `upload_rects`, and `dirty_rows` all matched the accepted baseline exactly. |
| `P4-51` | Failed: lower the staged-copy fallthrough guard from `6` to `5` VBlanks after the OT-clear skip. | The retest regressed `loop_vb 1242 -> 1245` and `prefetch_overrun_vb 12 -> 13`; keep `6` VBlanks as the local optimum. |
| `P4-52` | Failed hard: naive async stream-window refill. | Starting `CdRead` asynchronously and polling during held frames regressed `loop_vb 1242 -> 1267`, `blocking_vb 20 -> 71`, and `prefetch_overrun_vb 12 -> 64`; async needs first-class scheduling/metrics before another retry. |
| `P4-53` | Failed: skip clean-rect restore scans that do not intersect previous dirty tile bounds. | `loop_vb` stayed flat at `1242`, but `blocking_vb 20 -> 23` and `prefetch_overrun_vb 12 -> 14`; the runtime intersection screen costs more than the one skipped restore call saves. |
| `P4-54` | Failed as no-op: cache stream-read file-start LBA inside CD helpers. | Avoiding repeated `CdPosToInt(cdfile->pos)` calls left all key metrics exactly flat at `loop_vb=1242`, `blocking_vb=20`, and `prefetch_overrun_vb=12`; not worth promoting as a perf change. |
| `P4-55` | Failed: split cross-tile PAL4 spans directly at runtime after the tile-local fast path. | Work identity stayed stable, but `loop_vb 1240 -> 1246`, `blocking_vb 20 -> 27`, and `prefetch_overrun_vb 11 -> 18`; boundary-span splitting should be pack-time if retried. |
| `P4-56` | Failed as no-op: add a dedicated PAL4 zero-offset opaque-run helper. | The tile-local path already calls the helper with constant `srcPixel=0`; a separate helper left all key metrics flat at `loop_vb=1240`, `blocking_vb=20`, and `prefetch_overrun_vb=11`. |
| `P4-57` | Failed: lower the staged-copy fallthrough guard from `6` to `5` VBlanks after the tile-local PAL4 fast path. | The retest regressed `loop_vb 1240 -> 1244`, `blocking_vb 20 -> 22`, and `prefetch_overrun_vb 11 -> 15`; keep `6` VBlanks as the current slack knee. |

Prefetch variants to test in order:

| Variant | Description | Expected signal |
|---|---|---|
| One-entry synchronous staging | During held VBlanks, read the next entry into a second buffer if it is not already staged. | `cd_vb` may remain nonzero but should move out of due-frame advancement; visible speed should improve if enough hold budget exists. |
| One-entry async staging | Start `CdRead` during held time and poll completion over later held VBlanks. | Lower blocking time, but higher controller-state risk. |
| 16 KB stream window | Read a forward window from the current FG2 file and serve several entries from RAM. | Current default for fishing1 after the post-row-restore slack/window retune. |
| 20 KB stream window | Larger diagnostic window. | Former default; useful to re-test if later due-miss hiding makes coverage more valuable again. |
| 24 KB stream window | Larger diagnostic window. | Former default; useful to re-test if later due-miss hiding makes coverage more valuable again. |
| 32 KB/64 KB stream windows | Larger diagnostic windows. | Useful only if later grouping/async work can hide larger refill reads. |
| Dual-window ping-pong | Render from one window while filling the next during holds. | Best latency hiding, but only after single-window correctness. |
| Sector-aligned FGP3 chunks | Pack frames into prefetch-friendly sector groups. | Only useful if runtime windowing exposes sector-copy overhead. |

Physical CD layout guidance:

| Idea | Use when |
|---|---|
| Keep routed FG2 files grouped in `/FG`. | Already sensible for scene startup and should remain the default. |
| Do not split one scene across many CD files. | Per-frame file switching would add seeks/searches and defeat stream-window reads. |
| Add optional pack-internal prefetch groups before changing ISO ordering. | The measured stall is inside one active FG2 file. |
| Consider ordering scene variants by likely transition order later. | Useful for screensaver scene-to-scene startup, not the current per-frame bottleneck. |
| Keep sound files separate unless a measured scene startup bottleneck requires bundling. | Scene playback needs predictable FG2 streaming without accidental sound/resource coupling. |

Prefetch acceptance metrics:

| Metric | Target |
|---|---|
| `hits` | Majority of rendered entries for fishing1. |
| `blocking_vb` | Drops substantially from `562` on the same variant. |
| `reads` | Drops below `136` for window variants. |
| `loop_vb / target_vb` | Moves materially toward `1.0x` without shorter timing files. |
| `cd_fail` | Stays `0`. |
| Heap after scene loop | Stable over fishing3 overnight loops. |

Red-team risks:

| Risk | Mitigation |
|---|---|
| Prefetch read blocks long enough to consume visible held time | Start with one-entry staging and only prefetch when the current hold has slack. |
| Window refill causes visible hitch | Measure hidden vs blocking VBlanks first; then consider async only for refill. |
| Large scenes exceed memory budget | Bound window size and fall back to direct buffered read. |
| Off-by-one frame data after refill | Validate with checksums or entry index telemetry in debug builds. |
| CD controller state regressions | Keep `CdSearchFile` amortized, avoid new per-frame searches, and test title boot every time. |
| Sound events fire from a prefetched-but-not-presented entry | Prefetch bytes only; do not advance `frameIndex`, `sourceFrame`, or sound cursor until presentation. |
| Async read collides with sound/CD resource activity | Keep async disabled until the synchronous prefetch path is proven. |

## Phase 5: Pack Format Improvements

Goal: move repeatable parsing and clipping work out of the PS1 runtime.

| ID | Task | Rationale |
|---|---|---|
| `P5-01` | Emit per-frame row extents in the entry table or a side table. | Dirty restore/upload can avoid parsing spans twice. |
| `P5-02` | Emit per-tile command streams. | Runtime no longer splits spans around x=320/y=240. |
| `P5-03` | Emit row offsets for each encoded frame. | Runtime can jump rows without scanning from the beginning. |
| `P5-04` | Add a versioned `FGP3` format for optimized commands. | Keep `FGP2` available during migration. |
| `P5-05` | Use `uint8` relative row deltas where legal. | Shrink command overhead for small bboxes. |
| `P5-06` | Use `uint16` or varint payload sizes per row. | Reduce overhead without complex decompression. |
| `P5-07` | Pre-bucket X extents to the upload batching granularity. | Runtime can directly merge rows. |
| `P5-08` | Add optional prefetch groups: byte ranges containing several sequential entries. | Lets the runtime read one group and satisfy multiple frame deadlines. |
| `P5-09` | Emit sector-aligned group offsets only for scenes that benefit. | Avoid bloating every pack with sector padding. |
| `P5-10` | Add per-scene capability flags: pal4-pair-lut, row-extents, tile-split, prefetch-groups, sector-aligned. | Runtime can select fast paths safely. |
| `P5-11` | Emit pack-stat JSON alongside every FG2/FGP3. | Make routing and prefetch policy decisions data-driven. |
| `P5-12` | Add a corpus scanner that flags scenes with full-screen diffs. | Plan special handling for Suzy, Mary3, Activity9. |

## Phase 6: Scene Startup And Backdrop Cost

Goal: keep per-scene setup from stealing heap and causing long transitions.

| ID | Task | Rationale |
|---|---|---|
| `P6-01` | Keep `BACKGRND.PSB` persistent across scene loops. | Already done; preserve it. |
| `P6-02` | Keep `MRAFT.PSB` persistent when raft stage remains active. | Avoid repeat CD load on screensaver loops. |
| `P6-03` | Consider keeping `HOLIDAY.PSB` persistent across loops. | Avoid reload when random holiday repeats. |
| `P6-04` | Remove `ISLETEMP.SCR` from active CD layout if no active path uses it. | Smaller CD and fewer accidental fixed-island regressions. |
| `P6-05` | Build a minimal palette/metadata table to replace `RESOURCE.001` dependence. | Potential CD size and startup simplification. |
| `P6-06` | Track scene-start heap largest allocation in bounded `JCPERF` setup summaries. | Catch fragmentation before long soaks without visual debug overlays. |
| `P6-07` | Validate all scene cleanup paths with overnight loop. | Screensaver stability remains non-negotiable. |

## Phase 7: Build And Binary-Level Optimizations

Goal: keep the executable small and hot code friendly.

| ID | Task | Rationale |
|---|---|---|
| `P7-01` | Keep `-O2`, `-G8`, and function/data sections as the baseline. | Current flags are already sensible. |
| `P7-02` | Test `-O3` only on hot compositor files. | Whole-program `-O3` may grow code and hurt I-cache. |
| `P7-03` | Split hot compositor code into a small translation unit if needed. | Easier to test flags per file. |
| `P7-04` | Consider MIPS assembly only after C fast paths are measured. | Assembly should target proven hot loops. |
| `P7-05` | Remove or compile-gate unused debug/text formatting in release builds. | `vsnprintf` and debug paths are visible in the map. |
| `P7-06` | Skip `ClearOTagR` in pure FG2 software-background frames if safe. | Active playback often does not emit GPU primitives. |
| `P7-07` | Keep the primitive path available for debug/other scenes. | Avoid breaking future sandbox work. |

## Phase 7b: Remove Runtime Fallback Code

Goal: after deterministic optimized paths are validated, remove alternate
runtime fallback paths from the PS1 release build. The PS1 is a fixed hardware
target; if a path is correct for one console, it should be correct for all
consoles. Runtime fallbacks hide bugs, increase binary size, and create
untestable behavior branches.

| ID | Task | Rationale |
|---|---|---|
| `P7b-01` | Inventory every PS1 runtime fallback path: dirty full-rect fallback, old compositor fallback, direct alternate render modes, debug overlays, retired TTM/ADS/FG1 hooks. | Make fallback removal searchable and reviewable. |
| `P7b-02` | Classify each fallback as `remove`, `debug-only`, or `CD timing path`. | Only CD timing paths may remain in release, and they must not change pixels/sound/order. |
| `P7b-03` | Replace correctness fallbacks with fail-fast tripwires in `perf-log`/debug builds. | Bugs become visible instead of silently degrading output. |
| `P7b-04` | Keep original blocking CD reads as the deterministic prefetch-miss path. | A prefetch miss is timing pressure, not a lower-fidelity fallback. |
| `P7b-05` | Compile-gate debug-only fallback code out of release builds. | Reduces executable size and branch surface. |
| `P7b-06` | Require `trip=0`, `fallback=0`, `full_fallbacks=0`, `frame_mismatch=0`, and `sound_late=0` for every accepted optimization commit. | Prevents fallback behavior from creeping back in. |

## Phase 8: Large-Scene Strategy

Goal: avoid optimizing only fishing and then failing on Mary/Suzy/Activity.

| ID | Task | Rationale |
|---|---|---|
| `P8-01` | Classify packs by max frame payload and average visible pixels. | Large scenes need different policies. |
| `P8-02` | Special-case full-screen packs for direct tile memcpy paths. | Span overhead is poor when almost every pixel changes. |
| `P8-03` | Consider scene-specific base selection for full-screen outliers. | Better base frames can reduce false diffs. |
| `P8-04` | Revisit capture masks for scenes that diff the entire screen. | Some full-screen diffs may be capture artifact, not real animation. |
| `P8-05` | Add a "must benchmark before routing" flag for packs over a threshold. | Prevent routing a 30 MB pack blindly. |
| `P8-06` | Keep CD image route scene-by-scene, not all 126 packs. | Disk budget and validation discipline. |
| `P8-07` | Classify scenes by prefetchability: many small sequential entries, few huge entries, or mixed. | Determines staging, stream-window, or direct-read policy. |
| `P8-08` | For huge-entry scenes, test prefetch groups only if hold budgets can hide the read. | A 150 KB frame may not fit the same strategy as fishing1. |

Large-scene outliers from the current corpus:

| Pack | Pack size | Max frame payload | Note |
|---|---:|---:|---|
| `MARY3LOW.FG2` | 32.93 MB | 189.8 KB | Indexed8, very large diffs. |
| `MARY3.FG2` | 32.35 MB | 186.7 KB | Indexed8, very large diffs. |
| `ACTIVITY9.FG2` | 28.36 MB | 111.8 KB | PAL4 but broad screen diffs. |
| `ACTV9LOW.FG2` | 28.07 MB | 114.7 KB | PAL4 but broad screen diffs. |
| `SUZY1.FG2` | 19.68 MB | 153.8 KB | Full-screen style diffs. |
| `SUZY2.FG2` | 13.07 MB | 153.8 KB | Full-screen style diffs. |

Prefetch classification to add to the corpus scanner:

| Class | Runtime policy |
|---|---|
| Many small sequential entries | Stream window or dual-window prefetch. |
| Small entries with long holds | One-entry staging may be enough. |
| Large entries with long holds | Async or grouped prefetch may be needed. |
| Large entries with short holds | Renderer/format changes may matter more than prefetch. |
| Sparse empty entries | Preserve hold semantics and do not advance sound early. |

## Phase 9: Validation Protocol

Every performance change should use this checklist:

| Step | Check |
|---|---|
| 1 | `./scripts/build-ps1.sh clean` succeeds. |
| 2 | `./scripts/make-cd-image.sh` succeeds. |
| 3 | Run `fgpilot fishing1` default. |
| 4 | Run `fgpilot fishing1 lowtide 1`. |
| 5 | Run `fgpilot fishing1 night 1`. |
| 6 | Run `fgpilot fishing1 holiday 4`. |
| 7 | Run `fgpilot fishing1 raft-stage 5`. |
| 8 | Run forced island extremes from `VARPOS_OK`. |
| 9 | Run `fgpilot fishing2` and `fgpilot fishing3`. |
| 10 | Confirm sound timing, especially splash/catch events. |
| 11 | Let a loop run long enough to catch memory regressions. |
| 12 | Commit only after human visual/audio signoff. |
| 13 | Confirm the change added no per-frame `printf()`/console-output dependency. |
| 14 | For prefetch changes, compare `hits`, `blocking_vb`, `reads`, and `loop_vb / target_vb` against the saved fishing1 baseline. |

Forced island positions to keep using:

| Position | Why |
|---|---|
| `island-pos -222 -44` | Upper-left edge of first `VARPOS_OK` branch. |
| `island-pos -113 84` | Lower-right edge of first branch. |
| `island-pos -114 -14` | Second-branch boundary. |
| `island-pos 20 85` | Right/lower extreme. |
| `island-pos -114 -73` | Highest island case. |
| `island-pos 5 -13` | Third-branch right edge. |

## Metric Expectations By Experiment

Use this matrix after `JCPERF2` exists. If an experiment changes metrics outside
its expected blast radius, treat that as a regression or a sign that the mental
model is incomplete.

| Experiment | Metrics expected to improve or move | Metrics expected to stay stable | Success definition |
|---|---|---|---|
| Metrics foundation only | New `JCPERF2` fields become populated; existing `JCPERF` values remain comparable. | Pixels, sound, `target_vb`, `entries`, `payload`, `trip=0`, `fallback=0`, `fail=0`, `loop_vb / target_vb` within measured overhead. | Adds observability without changing playback behavior. |
| One-entry synchronous staging | `policy=stage1`, `attempts`, `stage_hits`, `hits`, `hidden_reads`, `hidden_vb`, `slack_vb`, `used_vb` increase; `due_misses`, `blocking_reads`, `blocking_vb`, `advance_vb`, `loop_vb / target_vb` decrease; prefetch `overrun_vb` stays near zero. | `reads` may stay near baseline; `bytes`, `payload`, `entries`, `rows`, `spans`, `pixels`, `sound_events` stable. | CD time moves out of due-frame advance and playback ratio improves. |
| One-entry async staging | `async_start`, `async_done`, `async_poll`, `hidden_vb`, `hits` increase; `async_timeout=0`; `blocking_vb` decreases. | `fail=0`, `frame_mismatch=0`, `sound_late=0`, `payload` stable. | Same visual/audio result with less blocking than sync staging. |
| 32 KB stream window | `policy=window32`, `window_hits`, `reads`, `setloc`, `blocking_reads`, `due_misses`, `blocking_vb` decrease; derived `sectors / reads` increases. | `bytes` may increase modestly due window rounding; `payload`, `entries`, `sound_events` stable. | Fewer CD transactions and lower visible CD stall without heap regression. |
| 64 KB stream window | Same as 32 KB, plus `buf=65536`; `reads` and `due_misses` should fall further. | `min_free`, `largest_end`, `alloc_fail=0`; no scene-start failure. | Better hit rate than 32 KB without destabilizing fishing3 heap. |
| Dual-window ping-pong | `hidden_reads`, `hidden_vb`, `window_hits` increase; `blocking_vb` and refill stalls decrease. | `wasted_bytes` bounded; heap stable. | Refills happen behind held time with no visible hitch. |
| Sector-aligned FGP3 chunks | `unaligned`, `scratch_bytes`, `max_read_vb` decrease; `pack_padding` and `pack_bytes` may increase. | `payload` and visible complexity stable; CD failures zero. | Sector alignment reduces copy/read overhead enough to justify pack growth. |
| Row/X dirty restore | `restore_bytes`, `restore_vb`, `dirty_exact_bytes`, `dirty_rounded_bytes` become meaningful; `full_fallbacks=0`, `trip=0`. | `compose_pixels`, `payload`, `entries`, `frame_mismatch=0`. | Restore byte volume drops without stale pixels and without fallback paths. |
| Row/X dirty upload batching | `upload_bytes`, `upload_vb`, `dirty_rounded_bytes` decrease; `upload_rects` may increase; `cap_hits` should be low. | `dirty_exact_bytes`, `compose_pixels`, `entries` stable. | Lower upload cost without rectangle-command overhead erasing the win. |
| Dirty rect cap/merge policy | `cap_hits` may increase on dense frames; `full_fallbacks=0`, `trip=0`, `frame_mismatch=0`. | No sound/timing changes beyond expected upload cost. | Dense frames remain correct through a planned deterministic merge path, not fallback rendering. |
| PAL4 pair-LUT compositor | `compose_vb`, `slow_pixels`, `max_render_vb` decrease; `lut_pairs` increases. | `compose_pixels`, `compose_spans`, `payload`, `dirty_exact_bytes` stable. | Same pixels and spans with lower compose time. |
| Span-level tile split | `tile_splits` becomes explicit; `compose_vb` and branch-heavy slow path counts decrease. | `compose_rows`, `compose_spans`, `compose_pixels` stable. | Less compositor time without changing dirty/upload work. |
| Indexed8 fast path | `compose_vb` decreases for indexed8 packs; indexed8 scene classification logs active. | PAL4 scenes unchanged; pixel counts stable. | Future Mary/Suzy indexed8 scenes become measurable and faster. |
| FG2-specific present pipeline | `present_wait_vb`, `render_vb`, `loop_vb / target_vb` decrease or shift; `upload_vb` may stay similar. | `restore_bytes`, `compose_pixels`, `upload_bytes` stable. | Less serialization without changing amount of work. |
| Event/input wait tuning | `event_wait_vb` decreases; controller responsiveness remains acceptable. | `target_vb`, `entries`, sound timing stable. | Removes idle waits that are not part of captured scene timing. |
| Pack row extents | Runtime `dirty_exact_bytes` comes from pack metadata; span re-scan counters decrease. | Pixel output and pack payload identity stable, aside from metadata growth. | Runtime stops recomputing data the pack can provide. |
| Pack per-tile command streams | `tile_splits`, clipping slow-path counts, and `compose_vb` decrease; pack metadata size increases. | `compose_pixels`, visible output, sound stable. | Pack-time work replaces runtime clipping. |
| Pack prefetch groups | `policy=groups`, `group_hits` increase; `reads`, `due_misses`, `blocking_vb` decrease. | `entries`, `payload`, `sound_events` stable; `pack_bytes` and `pack_padding` record size growth. | Group layout improves streaming beyond generic windows. |
| Persistent `BACKGRND`/raft/holiday resources | `setup_vb`, setup `reads`, `screen_vb`, `backdrop_vb` decrease; persistent buffer bytes increase. | Active `loop_vb`, `payload`, `entries` stable. | Scene transitions improve without long-run heap loss. |
| Remove unused CD assets | ISO size decreases; scene setup search/path metrics may improve. | Active playback metrics stable. | Smaller image without runtime regression. |
| Build flag or hot-TU optimization | `exe_bytes`, `compose_vb`, `render_vb` may move. | Scene identity, payload, CD, heap, correctness stable. | Faster hot code without binary bloat that hurts cache. |
| Remove unused runtime code paths | `exe_bytes` decreases; possibly `setup_vb` or cache-sensitive render metrics improve. | Playback work/correctness metrics stable. | Smaller executable and no behavior change. |
| Large-scene routing/classification | New pack stats classify scenes; no runtime metric change unless routed. | Current routed scenes stable. | Prevents blindly routing packs whose size/timing class needs a different policy. |

Metrics needed before each experiment:

| Experiment family | Required first |
|---|---|
| Prefetch/staging/windowing | Scene timing, CD base, CD blocking split, prefetch counters, heap counters, correctness guard. |
| Dirty restore/upload | Render subphases, dirty precision, restore/upload totals, correctness guard. |
| Compositor fast paths | Render subphases, compositor complexity counters, dirty/upload totals, correctness guard. |
| Present pipeline | Render subphases with `present_wait_vb`, `upload_vb`, `event_wait_vb`, and top-level timing. |
| Pack format changes | Frame complexity, CD base, dirty/compositor counters, pack identity/flags. |
| Scene startup/resource persistence | Setup timing, CD base for setup reads, heap counters. |
| Build/binary cleanup | Binary size, top-level timing, render subphases, correctness guard. |

## Next 100 Testable Optimization Ideas

This backlog is intentionally granular. Each row should be a small experiment
with one primary metric and one correctness gate. Do not batch unrelated rows
into one commit.

| # | Area | Experiment | Primary signal |
|---:|---|---|---|
| 1 | Baseline | Capture default fishing1 high-tide Summary after `1b457163`. | Stable `loop_vb`, `blocking_vb`, correctness zeroes. |
| 2 | Baseline | Capture default fishing1 low-tide Summary. | Compare tide-dependent pack size and CD behavior. |
| 3 | Baseline | Capture default fishing2 Summary. | Longer-scene prefetch hit rate. |
| 4 | Baseline | Capture default fishing3 Summary. | Memory-stable larger payload behavior. |
| 5 | Baseline | Capture fishing1 high-tide Detail once. | Populate render subphase split. |
| 6 | Baseline | Capture fishing3 Detail once. | Attribute larger-scene non-CD cost. |
| 7 | Baseline | Run Off vs Summary fishing1. | Quantify perf-log overhead. |
| 8 | Baseline | Run Summary vs Detail fishing1. | Quantify detail-probe overhead. |
| 9 | Baseline | Save a machine-readable metrics comparison script. | Same-field before/after diffs. |
| 10 | Baseline | Add a benchmark manifest for required variants. | Repeatable scene matrix. |
| 11 | CD | Done: test stream-window sizes before and after the `3` VBlank slack guard and diagnostics cleanup. | Initial sweep made `32 KB` the first clean default; post-slack sweep promoted `24 KB`; post-diagnostics sweep promotes `20 KB` with `loop_vb=1312`. |
| 12 | CD | Test 40 KB stream window. | Check whether the knee is between 32 KB and the original 48 KB. |
| 13 | CD | Test 56 KB stream window. | Better hit rate without extra overrun. |
| 14 | CD | Test 80 KB stream window for fishing3 only. | Determine memory/perf knee. |
| 15 | CD | Align stream window start to pack entry sector. | Lower `unaligned_start` and `overread_bytes`. |
| 16 | CD | Align stream window end to sector boundary only once. | Lower scratch-copy churn. |
| 17 | CD | Increase prefetch lead from next entry to next two entries. | Lower `due_misses`. |
| 18 | CD | Done: prefetch on holds with at least `3` VBlanks of slack; `6` VBlanks failed. | `prefetch_overrun_vb 94 -> 67` and `loop_vb 1335 -> 1325` without increasing `blocking_vb`. |
| 18a | CD | Done: retune default stream window to `24 KB` after the slack guard. | `prefetch_overrun_vb 67 -> 58`, `loop_vb 1325 -> 1322`, with `blocking_vb 106 -> 108` inside the gate. |
| 18b | CD | Done: retune default stream window to `20 KB` after diagnostics gating. | `loop_vb 1317 -> 1312`, `blocking_vb 93 -> 91`, and `prefetch_overrun_vb 41 -> 37`; due misses rose `8 -> 11`. |
| 18c | CD | Done: lower the post-20 KB refill guard to `2` VBlanks. | `loop_vb 1312 -> 1300`, `blocking_vb 91 -> 66`, and `due_misses 11 -> 4`; `prefetch_overrun_vb` rose `37 -> 45`. |
| 18d | CD | Failed: lower the post-20 KB refill guard to `1` VBlank. | `due_misses 4 -> 2`, but `loop_vb 1300 -> 1312` and `prefetch_overrun_vb 45 -> 62`; retry only after refill cost changes. |
| 18e | CD | Failed as a no-op: lower staged-copy fallthrough from `5` to `4` VBlanks under the current baseline. | `loop_vb`, `blocking_vb`, `prefetch_overrun_vb`, `hits`, and `due_misses` all matched baseline exactly. |
| 18f | CD | Done: retune default stream window to the sector-rounded `18 KB` bucket. | `loop_vb 1300 -> 1296` and `prefetch_overrun_vb 45 -> 33`; `blocking_vb 66 -> 75` and `due_misses 4 -> 8`. |
| 18g | CD | Failed: raise the post-row-restore refill guard from `2` to `3` VBlanks. | `loop_vb 1266 -> 1257` and `prefetch_overrun_vb 26 -> 10`, but `blocking_vb 36 -> 56`; retry only with grouped/pipelined coverage. |
| 18h | CD | Done: pair the post-row-restore `16 KB` window with the `3` VBlank refill guard. | `loop_vb 1266 -> 1254`, `blocking_vb 36 -> 35`, and `prefetch_overrun_vb 26 -> 6`; due misses rose `1 -> 6`. |
| 18i | CD | Failed: lower staged-copy fallthrough from `5` to `4` VBlanks after the `16 KB`/`3` VBlank retune. | `blocking_vb 35 -> 31`, but `loop_vb 1254 -> 1264`; submetric wins are not enough. |
| 18j | CD | Failed: split immediate and lookahead refill guards. | `due_misses 6 -> 1`, but `loop_vb 1254 -> 1268` and `prefetch_overrun_vb 6 -> 22`; short immediate reads are still too costly. |
| 18k | CD/Dirty | Done: aggregate PAL4 dirty marking per tile row. | `loop_vb 1254 -> 1248`, `blocking_vb 35 -> 21`, and `due_misses 6 -> 1`; `prefetch_overrun_vb` rose to `15` and should be smoothed next. |
| 18l | CD | Failed: raise the post-row-dirty refill guard to `4` VBlanks. | `loop_vb 1248 -> 1244`, but `blocking_vb 21 -> 36`; stricter gating starves due-frame residency. |
| 18m | CD | Done: raise the post-row-dirty staged-copy fallthrough guard to `6` VBlanks. | `loop_vb 1248 -> 1243`, `prefetch_overrun_vb 15 -> 12`, and `blocking_vb` stayed `21`. |
| 18n | CD | Failed: re-sweep `15 KB` and `14 KB` windows after the fallthrough guard. | `15 KB` no-op; `14 KB` lowered overrun but starved coverage and raised blocking. |
| 18o | Dirty/Compose | Failed as no-op: hoist the PAL4 dirty visible-row branch. | Metrics were unchanged; needs finer CPU counters or batching with adjacent branch cleanup. |
| 18p | Dirty/Compose | Failed: call direct tile dirty marker from PAL4 row aggregation. | Slower than the generic wrapper despite equivalent dirty metrics. |
| 18q | CD | Failed: raise the post-row-dirty staged-copy fallthrough guard to `7` VBlanks. | Slower total loop with no blocking reduction; keep `6` until refill costs change. |
| 19 | CD | Split prefetch budget by remaining hold slack. | Lower visible `blocking_vb`. |
| 20 | CD | Done: stop duplicate prefetch attempts earlier. | `duplicate 887 -> 0`; timing flat, metrics cleaner. |
| 21 | CD | Cache last resolved FG2 file handle per scene. | Lower setup/loop search cost. |
| 22 | CD | Remove redundant `CdSearchFile` inside active loop. | Lower `setloc` or search logs. |
| 23 | CD | Coalesce adjacent due-frame misses into one direct read. | Lower `reads`. |
| 24 | CD | Read through current window even on partial overlap. | Increase `partial_hits`. |
| 25 | CD | Use one staged-entry buffer plus window pointer handoff. | Lower copies and misses. |
| 26 | CD | Failed: avoid copying due window-resident entries into `frameBuffer`. | Correctness clean, but slower than the explicit copy; retry only with broader stream-window/stage ownership changes. |
| 27 | CD | Track and reuse previous window if frame offset stays inside. | Increase `window_hits`. |
| 28 | CD | Add scene-class window policy table generated from pack stats. | Deterministic buffer choice. |
| 29 | CD | Try synchronous dual-window ping-pong during long holds. | Lower refill `blocking_vb`. |
| 30 | CD | Prototype async read only for window refill, not due frames. | Increase `async_done`, keep `timeout=0`. |
| 31 | Pack | Emit per-frame payload sector span in FG2 metadata. | Remove runtime offset math. |
| 32 | Pack | Emit per-frame row extents. | Lower dirty extent computation. |
| 33 | Pack | Emit previous/current dirty union extents. | Lower restore planning work. |
| 34 | Pack | Emit per-tile span command streams. | Lower `tile_splits`. |
| 35 | Pack | Emit span rows sorted by tile. | Fewer tile-boundary branches. |
| 36 | Pack | Add optional sector-aligned prefetch groups. | Lower `reads` and `unaligned_*`. |
| 37 | Pack | Add no-payload held-entry records. | Avoid empty entry reads. |
| 38 | Pack | Pack sound-event table into startup metadata only. | No per-frame sound scan. |
| 39 | Pack | Add payload checksum only for debug builds. | Faster frame identity debugging. |
| 40 | Pack | Emit frame max X/Y bounds after scene-relative offset. | Faster clipping. |
| 41 | Pack | Quantize dirty extents to upload buckets at pack time. | Lower runtime batching. |
| 42 | Pack | Add pack-stat JSON for every generated FG2. | Better route decisions. |
| 43 | Pack | Flag huge frames before routing to CD. | Avoid blind large-scene regressions. |
| 44 | Pack | Re-evaluate base frames for full-screen outliers. | Lower pack bytes and payload. |
| 45 | Pack | Prototype `FGP3` sidecar metadata without changing pixels. | Runtime speed with easy rollback. |
| 46 | Dirty | Done: replace restore-side row-band dirty state with row X extents. | `restore_bytes 9520664 -> 2510092`. |
| 47 | Dirty | Done: track previous row extents separately. | Correct previous-frame cleanup with `trip=0` and `frame_mismatch=0`. |
| 48 | Dirty | Partial: restore previous extents exactly; upload remains current/previous row-band union. | Restore is now precise; upload batching is the remaining work. |
| 48a | Dirty/Compose | Done: mark PAL4 FG2 dirty rows once per tile row instead of once per span. | `loop_vb 1254 -> 1248`; `restore_bytes` and `upload_bytes` unchanged. |
| 49 | Dirty | Round X extents to 8-pixel buckets. | Balance bytes vs rect count. |
| 50 | Dirty | Round X extents to 16/32-pixel buckets. | Already insufficient alone; retry only under a rect-count cap. |
| 51 | Dirty | Merge adjacent rows with similar X extents. | Lower `upload_rects` without per-strip `DrawSync`. |
| 52 | Dirty | Cap rects by deterministic widening, not fallback. | Runtime cap alone failed because scratch copies dominated; keep for future upload-ready formats. |
| 53 | Dirty | Split dense frames into planned wide bands at pack time. | Stable worst-case upload time without runtime row repacking. |
| 54 | Dirty | Avoid marking unchanged clean rows dirty. | Lower `dirty_rows`. |
| 55 | Dirty | Cache clean-rect row source pointers. | Lower restore CPU. |
| 56 | Dirty | Use word copies for aligned restore spans. | Lower `restore_vb`. |
| 57 | Dirty | Use halfword edge handlers around word restore. | Preserve exact pixels. |
| 58 | Dirty | Track max dirty frame index in Summary. | Faster targeted debugging. |
| 59 | Dirty | Detail-test restore before upload batching. | Separate RAM vs VRAM wins. |
| 60 | Dirty | Detail-test upload batching before restore narrowing. | Separate VRAM command cost. |
| 61 | Dirty | Add stale-guard debug checksum for dirty bounds only. | Catch under-restore. |
| 62 | Dirty | Remove old full-rect dirty fallback after validation. | Lower branch/code size. |
| 63 | Compose | Use PAL4 byte-to-two-pixel LUT. | Lower `compose_vb`. |
| 64 | Compose | Prebuild LUT per scene palette at pack start. | No per-frame palette setup. |
| 65 | Compose | Remove transparent checks for FG2 spans. | Lower branch count. |
| 66 | Compose | Split spans at tile boundaries once. | Lower per-pixel tile tests. |
| 67 | Compose | Failed: runtime PAL4 pair LUT plus aligned 32-bit stores. | `loop_vb 1266 -> 1279`; retry only as pack-time direct16 or lower-overhead scene specialization. |
| 68 | Compose | Add odd-left edge handler. | Preserve edge pixels. |
| 69 | Compose | Add odd-right edge handler. | Preserve edge pixels. |
| 70 | Compose | Use row-local destination pointer increments. | Less address math. |
| 71 | Compose | Precompute tile base pointer per row band. | Less tile lookup work. |
| 72 | Compose | Specialized high-tide fishing compositor path. | Test if scene-class specialization pays. |
| 73 | Compose | Specialized low-tide fishing compositor path. | Test smaller visible-pixel class. |
| 74 | Compose | Indexed8 direct palette fast path. | Prepare Mary/Suzy classes. |
| 75 | Compose | Measure direct16 pack option on one dense scene. | Trade pack size for CPU. |
| 76 | Compose | Batch spans by contiguous destination rows. | Better cache locality. |
| 77 | Compose | Add clipped-span counters. | Find capture/offset waste. |
| 78 | Compose | Remove generic sprite compositor branch from FG2 path. | Lower hot-path code/branches. |
| 79 | Present | Detail-split `grUpdateDisplay()` wait vs upload. | Identify present serialization. |
| 80 | Present | Failed first attempt: compose FG2 RAM tiles before `VSync(0)`. | Correctness clean, but `loop_vb`, `blocking_vb`, and `due_misses` regressed; retry only with Detail-tier attribution or a broader render scheduler. |
| 81 | Present | Wait only when next frame deadline requires it. | Lower idle VBlanks; first held-VBlank skip attempt was rejected as not scanline-safe despite a small headless speed win. |
| 82 | Present | Done: skip OT clear in pure software base-diff FG2 frames. | `loop_vb 1243 -> 1242`; non-base-diff wave/legacy primitive paths still reset OT state. |
| 83 | Present | Failed: lower SPI pad polling from `250 Hz` to `125 Hz` or `65 Hz`. | Total loop regressed by one VBlank despite slightly better CD submetrics; retry only with finer CPU/IRQ counters and input-latency validation. |
| 84 | Present | Separate frame counter from rendered-entry counter. | Correct pause/input accounting. |
| 85 | Present | Avoid double display updates on empty entries. | Lower `render` or `empty` cost. |
| 86 | Present | Measure `LoadImage` rectangle count vs bytes. | Choose batching strategy. |
| 87 | Present | Try two-phase upload for very wide dirty rows only from contiguous sources. | Lower worst-case upload spikes without scratch-packing rows. |
| 88 | Setup | Keep holiday art persistent when heap allows. | Lower `setup_reads`. |
| 89 | Setup | Keep raft art persistent across same raft stage. | Lower `backdrop_vb`. |
| 90 | Setup | Preload next scene metadata only, not payload. | Faster transition with low heap. |
| 91 | Setup | Remove unused active CD assets from layout. | Smaller ISO/search surface. |
| 92 | Setup | Replace `RESOURCE.001` dependence for active FG2 route. | Lower setup reads. |
| 93 | Setup | Sort CD layout by active startup order. | Lower setup seek gaps. |
| 94 | Setup | Add scene transition setup metrics matrix. | Find non-playback delays. |
| 95 | Binary | Compile-gate unused debug strings in release. | Lower `exe_bytes`. |
| 96 | Binary | Move hot compositor to its own translation unit. | Test file-specific `-O2/-O3`. |
| 97 | Binary | Generate a map-size diff after each perf commit. | Catch bloat. |
| 98 | Binary | Remove retired FG1/TTM/ADS runtime hooks from PS1 release. | Lower code size. |
| 99 | Binary | Replace runtime fallback branches with debug tripwires. | Lower branch surface. |
| 100 | Validation | Automate metric acceptance checks for `trip=0` and speed deltas. | Prevent silent regressions. |

## Recommended Next Experiments

| Order | Experiment | Commit only if |
|---:|---|---|
| 1 | Implement the `JCPERF2` metrics foundation. | Done in `c0e6d95e`; isolated fishing1 Summary run emitted valid scene-end records. |
| 2 | Done: implement held-entry no-work plus stage1/window prefetch and make it default. | Merged in `1b457163`; default run reports `policy=stage1_window`, clean correctness counters, and visual signoff. |
| 3 | Capture the post-merge baseline matrix with `JCPERF2` Summary and selected Detail runs. | fishing1 high/low, fishing2, and fishing3 have comparable records under the new default path. |
| 4 | Done: gate controller/SPI diagnostics off by default. | fishing1 improved `loop_vb 1369 -> 1317` with clean correctness; `pad-diag`/`pad-debug` preserve the deeper controller probe path. |
| 5 | Reduce remaining prefetch blocking and refill overrun. | `blocking_vb`, `blocking_reads`, `due_misses`, and `overrun_vb` fall without increasing heap risk or changing sound/pixels. |
| 6 | Prototype pack-emitted FG2/FGP3 prefetch groups or sector-aligned group sidecar metadata. | `reads`, `due_misses`, and `blocking_vb` fall without growing `pack_bytes` enough to threaten the CD budget. |
| 7 | Add X-aware upload batching on top of accepted row-level restore. | `upload_bytes`, `upload_rects`, or `upload_vb` fall with no stale pixels and no runtime full fallback. |
| 8 | Test FG2-specific present/update sequencing beyond the rejected compose-before-VSync attempt. | `present_wait_vb`, `upload_vb`, or `loop_vb / target_vb` improve without changing work identity or worsening CD prefetch coverage. |
| 9 | Specialize PAL4 FG2 compositor with span-level tile split and pair LUT. | Same pixels, lower compose counters. |

## Red-Team Conclusions

The safest near-term speedup is not a more aggressive timing file. The measured
runtime is still `1.16x` over the captured timing budget for fishing1 after the
latest accepted pass. We need to keep removing or hiding work, not lie about
the source timing.

Timing wins are only valid when work identity stays stable. The sequential
`Setloc` skip experiment proved that the current Summary gate can accept a run
where correctness counters are zero but the renderer performed far less work.
Future acceptance must compare baseline-sensitive counters such as
`compose_calls`, `upload_calls`, `restore_calls`, `upload_bytes`, and
`restore_bytes` before promoting any low-level CD or render scheduling change.
The headless harness now enforces a default `75%` minimum for `render`,
`restore_calls`, `compose_calls`, and `upload_calls` when comparing against a
baseline; override only for deliberate pack/render architecture changes.

The first measured target is CD latency. Held-frame no-work created idle
VBlanks, but the runtime currently waits until the next frame is due before it
reads that frame. Prefetching converts those held VBlanks into useful work
without skipping frames or changing art.

The second major target is render pipeline precision and serialization.
Row-level restore removed most RAM restore waste, but VRAM upload still moves
full dirty row bands. Subphase counters should guide X-aware dirty uploads,
compositor work, and present scheduling.
