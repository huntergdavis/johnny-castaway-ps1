# PS1 Scene Playback Performance Optimization Plan

> 🌐 **Rendered version:** **[/docs/performance/](https://hunterdavis.com/johnny-castaway-ps1/docs/performance/)** — this doc rendered on the project website with cross-links and prose context. The GitHub copy here is the source.


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

Post-merge status: the first performance wave and subsequent retunes are now
the normal runtime path. Held-entry no-work, one-entry staging, a sector-rounded
`16 KB` FG2 stream window, direct-stage seeding, and dirty clean-rect row
restore are active on the perf branch. The boot parameters still exist for
diagnostics, but the default FG2 playback policy is now `stage1_window`.

Latest accepted default-path fishing1 exact no-holiday night variant
(`lowtide 0`, `night 1`, raft stage `4`, island position `-154,54`), after the
pause merge, pad/SPI diagnostic gating, the post-diagnostics window retunes, the
3 VBlank refill guard, 6 VBlank fallthrough guard, row-level X dirty restore,
per-tile PAL4 row dirty marking, the tile-local PAL4 fast path, vertical
dirty-row upload bands with a 1-row gap byte trim, setup priming of a `320 KB`
first-payload FG2 window for fishing1 high tide, setup-gated threshold-`4`
catch-up, first FG2 upload scoped to the saved clean-rect Y band,
tight-slack direct staging for immediate payloads up to 8 KB,
direct-stage scratch window seeding, and exact-4 VBlank held-slack staged-frame
prep, plus leading-empty setup consume with a one-VBlank setup settle and
coalesced FG2 metadata-prefix startup reads, plus PS1 function/data section
garbage collection, foreground visual telemetry removal, legacy foreground
diagnostic scene gating, long-hold host-deadline catch-up, unused foreground
status accessor removal, dead foreground requested-mode state removal, and
base-diff foreground pack enforcement, startup pre-application of
scene-relative FG2 offsets, direct reads of those pre-applied entry offsets,
collapsed held-loop prefetch branch shape, duplicate compose active-guard
removal, simplified runtime-active accessor, and the fishing1 high-tide tail
read group `396..406` with 11-sector retained capacity, reported
`policy=stage1_window`, `buf=333656`, `hits=155`, `due_misses=0`,
`blocking_vb=0`, `prefetch.overrun_vb=0`, `loop_vb=1207`,
`overrun_vb=131`, `target_vb=1076`, `restore_bytes=251144`,
`upload_bytes=6690560`, `dirty_rows=10454`, `upload_rects=290`, `trip=0`,
`fallback=0`, `frame_mismatch=0`, `sound_late=0`, and `cd_fail=0`.
The same run also reports `setup_reads=6`, `pack_start_vb=41`,
`setup_read_vb=169`, `loop_reads=6`, `loop_read_vb=26`, and `scene_vb=1447`.
This is the current baseline for the next experiment. The accepted FGP3
temporal-residual pack intentionally changes layout (`FISHING1.FG2 LBA
396 -> 397`, PS-EXE `143360 -> 145408`) while cutting high-tide pack bytes
`829851 -> 398433`, restore bytes `2510092 -> 251144`, and upload bytes
`15888640 -> 6690560`.
The first-upload clean-rect pass removed the stale full-screen forced upload
from FG2 setup (`max_upload_bytes 614400 -> 221440`) and improved active
playback by two VBlanks without changing setup timing, restore bytes, CD
pressure, or layout identity. The later FGP3 pass is the first accepted
pack-format runtime win and supersedes the FG2 high-tide baseline for fishing1.
The FGP3 red-team pass changes the next bottleneck. `perf-detail` on the
accepted canary shows `present_wait_vb=155`, while `compose_vb=2`,
`restore_vb=0`, `upload_vb=0`, `blocking_vb=0`, and
`prefetch.overrun_vb=0`. Local retries after FGP3 did not promote:
prepared-present slack `3` and `2`, setup-prime catch-up `3`, previous-dirty
bounds discard, due-frame precompose-before-present, FGP3 helper `-Os`, and
no-holiday call-site guarding all stayed timing-flat or worsened accounting.
The next meaningful speed work should target present/VSync ownership,
release-vs-perf-log measurement, generated all-scene FGP3 rollout, or
inter-scene/setup-prime hiding instead of more fishing1 high-tide restore/CD
micro-probes.
The next FGP3 rollout step is also accepted for fishing1 low tide:
`FISH1LOW.FG2` now uses `fgp3_pal4_residual`, cuts pack bytes
`426082 -> 303083`, and improves the low-tide gate `loop_vb 1215 -> 1209`,
`overrun_vb 142 -> 135`, `blocking_vb 5 -> 4`, `prefetch.overrun_vb 5 -> 4`,
and `loop_reads 31 -> 22`. High tide stayed exact-flat afterward. This turns
FGP3 from a single-canary high-tide experiment into the preferred fishing1
high/low methodology, with remaining low-tide pressure now concentrated in
CD/refill policy rather than restore/upload volume.
Fishing1 low tide is now also covered by the existing `320 KB` setup-prime
window. Active-loop reads fall `22 -> 0`, `blocking_vb/prefetch.overrun_vb`
fall `4 -> 0`, and `loop_vb 1209 -> 1207`, matching the high-tide active-loop
gap. The trade is explicit: `setup_vb 182 -> 238` and `scene_vb 1391 -> 1445`.
This should be treated as proof that generated prime budgets can make FGP3
active playback CD-clean, not as proof that setup time is free.
FGP3 is now accepted for fishing2 high tide as well. `FISHING2.FG2` shrinks
`1595559 -> 542743` bytes, `loop_vb 1928 -> 1903`,
`overrun_vb 190 -> 139`, `blocking_vb 50 -> 8`,
`prefetch.overrun_vb 44 -> 8`, `due_misses 2 -> 0`, and
`loop_reads 134 -> 40`. This is the first non-fishing1 FGP3 runtime promotion
and confirms the format is a scene-family strategy, not a single-scene trick.
Remaining fishing2 pressure is now mostly read-window policy.
Fishing2 high tide now also has an explicit `352 KB` setup-prime budget. This
cuts active-loop CD pressure again: `loop_vb 1903 -> 1898`,
`overrun_vb 139 -> 133`, `blocking_vb/prefetch.overrun_vb 8 -> 2`,
`loop_reads 40 -> 14`, and `loop_read_vb 170 -> 63`. The heap boundary is
real: `384 KB` and `544 KB` failed before loop start, and `368 KB` hit the
headless log cap. The next fishing2 high win should be generated read groups,
segmented prime, or inter-scene preload rather than a larger contiguous setup
prime.
Red-team caveat: setup-prime wins are active-loop wins, not end-to-end
scene-time wins. Fishing2 high now moves a `352 KB` foreground read into setup
(`setup_vb 184 -> 251`, `scene_vb 2087 -> 2149`) so active playback can reduce
visible CD pressure; earlier fishing1 setup-prime wins have the same trade. The
next pass should hide these primes during inter-scene/loading time or emit
generated per-scene/tide prime windows rather than counting setup as free. The
exact-4 plus 1-row upload
plus prepared-wait prefetch
checkpoint is a work-reduction
promotion, not a claimed VBlank speed win: it kept `loop_vb`, `blocking_vb`,
and `prefetch.overrun_vb` flat while reducing `restore_calls/compose_calls`
from `193` to `155` and reducing upload bytes by `217600`. The section-GC pass
kept earlier counters flat while shrinking
`jcreborn.elf` from `709828` to `708656` bytes; `jcreborn.exe` remains in the
same `137216` byte sector bucket. Removing the now-unused foreground visual
telemetry body kept timing flat again, dropped speculative prep
`restore_calls/compose_calls` from `190` to `188`, reduced `restore_bytes` from
`3034562` to `2999408`, and shrank `jcreborn.elf` to `707916` bytes. The
legacy foreground diagnostic gate later moved the executable into the `131072`
byte bucket with flat timing; long-hold deadline catch-up then traded seven
extra speculative restore/compose calls for `5` fewer loop VBlanks. Removing
the unused foreground "ever" diagnostics kept timing flat and shrank
`jcreborn.elf`; removing the unused ADS foreground auto-start hook kept timing
flat again and shrank `jcreborn.elf` to `690932` bytes. Removing the obsolete
`FGPILOT` ADS debug dispatch kept playback flat and moved `jcreborn.exe` down
to `129024` bytes. Removing unused foreground status accessors then repeated at
`loop_vb=1221` with `blocking_vb=5` and `prefetch.overrun_vb=5`. Removing the
write-only requested-mode state kept that cadence flat and shrank
`jcreborn.elf` to `690724` bytes. Requiring base-diff FG2 packs and removing
the old non-base-diff runtime branches kept cadence flat again and shrank
`jcreborn.elf` to `689748` bytes. The pre-pause best was `loop_vb=1297`.

Latest available Detail-tier attribution on the pre-pause accepted baseline
shows the remaining
active-loop gap is not primarily due-frame CD: `render_vb=181`,
`present_wait_vb=157`, `restore_vb=43`, `compose_vb=31`, `upload_vb=0`, and
`advance_vb=1` in
`scratch/ps1-perf-iterate/20260426-084540/summary.json`. The present wait is
mostly the required frame cadence, while restore/compose crossings are real
remaining overrun candidates. A first staged next-VBlank present scheduler was
rejected because it regressed total loop time to `1306`; the next present work
needs separate render-prep and CD-prefetch slack budgets instead of stealing
the held-frame prefetch cadence. A later 4 VBlank held-slack prepared-present
pass was accepted as a small speedup (`loop_vb 1235 -> 1234`,
`blocking_vb 10 -> 8`), but it did not prove out as the full present-wait fix
because it adds duplicate RAM restore/compose work (`restore_calls 156 -> 192`,
`compose_calls 155 -> 191`). The current post-pause exact-4 variant claws back
some of that duplicate work with flat key timing, but the larger present-wait
fix still needs a scheduler with separate render-prep and CD-prefetch budgets.

Current post-cleanup Detail-tier attribution on `20260426-234118` confirms the
same priority more sharply: `loop_vb=1221`, `overrun_vb=150`,
`render_vb=179`, `present_wait_vb=157`, `restore_vb=26`, `compose_vb=32`,
`blocking_vb=5`, and `prefetch.overrun_vb=5`. The remaining gap is dominated
by present wait, not visible CD. A direct prepared-present event-poll removal
was rejected because it regressed visible CD pressure and weakens pause/input
semantics; the present fix needs a real scheduler/presentation design, not a
local poll deletion.

The first scheduler ownership pass on `20260427-105900` adds the missing
Summary-tier ownership surface without changing fishing1 timing or PS-EXE
bucket: `present=72`, `cd_stage=108`, `cd_window=54`, `visual_prepare=72`,
`wait=574`, `cd_reserved=28`, `prep_blocked_cd=13`, `prepared_ready=72`,
`prepared_used=72`, and `prepared_wasted=0`. That proves the current prepared
pipeline is not wasting prepared frames in fishing1; the next scheduler win
must spend idle/CD-reserved held slices safely instead of chasing duplicate
prepared-frame reuse.

The deeper trace surface is now split from the accepted speed binary. The first
ungated pipeline/slack-bucket counter pass changed EXE layout and timing, so it
is compile-gated behind `PS1_PERF_DEEP_TRACE=ON`. Default builds stay exact;
trace builds showed `due_frames=83`, `prepare_frames=72`,
`prepared_present_frames=72`, and `prepared_wasted=0`, confirming the scheduler
problem is ownership of idle/CD/present slots rather than discarded prepared
frames. A setup-primed `320 KB` first-payload window then made one previously
unsafe threshold-`4` catch-up shape safe for fishing1 high tide, reducing the
active-loop overrun from `147` to `140` while lowering visible CD/refill
pressure from `5` to `1` VBlank.

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
| 1 | Generated setup-prime and inter-scene preload | High | The promoted `320 KB` prime cuts active-loop overrun `147 -> 140` and visible CD/refill `5 -> 1`, but currently pays setup cost. Hiding or generating the prime can turn this into a full-scene win. |
| 2 | FG2-specific present pipeline with explicit slack budgeting | High | Detail counters show `present_wait_vb=157`; the next design must reduce or hide present latency while preserving CD lookahead and pause/input safety. |
| 3 | Pack-emitted read groups and sector layout | Medium | Current setup-primed high tide still has `43` active-loop reads and `3` backward seeks; selective generated metadata is safer than one-off group tables. |
| 4 | X-aware dirty upload and rect-pressure control | Medium | Latest default run restores `2.51 MB` after prepared-wait prefetch removes duplicate prep, and vertical bands plus a 1-row gap keep upload near `16.3 MB`; upload volume and rect pressure are still measurable dirty targets. |
| 5 | Specialized PAL4 FG2 compositor | Medium | Fishing frames are modest, but larger scenes will make span/tile split and PAL4 conversion overhead more important. |

Latest red-team note: local CD/runtime tweaks are hitting a hard determinism
wall. `384..396` did not fire, `307..317` was exact-flat with code growth,
direct stage-into-window exposed one extra visible CD VBlank, and
`foreground_pilot.c -O3` grew the executable without a speed win. Treat those
as evidence that the next CD/render pass needs generated metadata plus an
explicit cost/scheduler model, not another one-off hard-coded branch. The
successful setup-prime exception reinforces that point: preloaded coverage must
be proven by pack metadata and bounded heap policy before a shorter catch-up
threshold is safe.

Non-goals:

| Non-goal | Reason |
|---|---|
| Frame dropping | Violates pixel-perfect playback. |
| Timing compression before throughput work | The measured playback is `1.55x` over target; shorter timing files would expose the same throughput bottleneck. |
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
contiguous scratch before `LoadImage`. A later direct vertical-band upload
experiment avoided scratch packing, kept full tile width, and accepted a safe
work-volume reduction: `upload_bytes 17172480 -> 16387840` and
`dirty_rows 26832 -> 25606`, with `loop_vb`, `blocking_vb`, and
`prefetch_overrun_vb` unchanged. A 2-row gap merge then reduced command
pressure (`upload_rects 518 -> 427`, `max_upload_rects 6 -> 5`) while keeping
most of the byte win (`upload_bytes 16496000`), again with flat key timing.
After the leading-empty cadence win, an 8-row gap merge became worth accepting:
it keeps timing flat while reducing `upload_rects 424 -> 412` for only `43520`
additional bytes across the full fishing1 run.
A follow-up 10-row point was accepted: timing stayed flat,
`upload_rects` drops again to `409`, and the extra byte cost versus the 8-row
baseline is only `17,920` bytes across the loop.
A final 11-row probe was the pre-pause local knee: timing stayed flat,
`upload_rects` dropped to `401`, and the extra byte cost versus the 10-row
baseline was `56,320` bytes across the loop. After the pause/menu merge, the
current accepted point is a 1-row byte-trim gap: key timing remains flat while
`upload_bytes` drops `16499200 -> 16281600` and rects rise `401 -> 502`.
The zero-gap point is still rejected, so the current safe boundary for this
variant is between `1` and `0` clean rows.
The next upload-byte attempt should move decisions to pack-time/direct-layout
work or use scene-specific band statistics, not reintroduce runtime scratch
packing.

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
| Scheduler ownership | Implicit in branch order. | Add held-slice owner counters, CD-reserved slots, CD-blocked visual prep, prepared-ready/used/wasted, and held-slack min/max. | Existing held-loop decisions. |
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
| Scheduler ownership | `present`, `cd_stage`, `cd_window`, `visual_prepare`, `wait`, `cd_reserved`, `prep_blocked_cd`, `prepared_ready`, `prepared_used`, `prepared_wasted`, `slack_vb`, `slack_min`, `slack_max` | Present scheduler, prepared-frame scheduler, CD-first budgeting | Shows who owns each held slice and whether prepared work is useful, blocked by CD, or wasted. |
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
dirty row spans. The upload path now splits dirty tile uploads into contiguous
vertical dirty-row bands with a post-pause 1-row clean-gap merge, while keeping
full tile width and no scratch packing.
Fishing1 improved from the original `loop_vb=1426` to `1207`,
`restore_bytes=16035840` to `251144`, and `upload_bytes=17172480` to
`6690560`; next work is balancing the FGP3 executable-size/layout cost against
the remaining present/scheduler gap and extending residual packs beyond the
fishing1 high-tide canary.

| ID | Task | Rationale |
|---|---|---|
| `P2-01` | Partial: add per-tile `minX/maxX` alongside `minY/maxY`. | Preserve useful horizontal precision from FG2 spans without changing upload yet. |
| `P2-02` | Partial: keep `curr` and `prev` X/Y extents per tile. | Restore previous pixels using narrower RAM copies; upload still uses row bands. |
| `P2-03` | Done: change `grMarkRectDirty` to update X extents, not just row bands. | Existing callers get better restore precision automatically. |
| `P2-04` | Done: clean-rect restore copies only previous X extents. | Avoid full-width RAM clean copies for narrow dirty bands. |
| `P2-05` | Batch row extents into a bounded list of `LoadImage` rectangles. | Avoid one `LoadImage` per row. |
| `P2-06` | Use 8- or 16-pixel X bucket rounding for better batching. | Trade tiny extra upload for far fewer rectangles. |
| `P2-07` | Add a deterministic merge policy when rect count exceeds a cap. | Avoid command overhead spikes on dense frames without routing to an alternate render fallback. |
| `P2-08` | Record bytes actually uploaded in telemetry. | Confirm real win. |

Expected impact: current active fishing scenes often upload 216-358 KB per
VBlank. Exact row extents average 21-32 KB. A practical batching strategy
should land between those numbers and much closer to the exact side.

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
| `P3-03` | Add a 256-entry PAL4 byte-to-two-pixels LUT per FG2 palette. | Convert two pixels per byte with one lookup. |
| `P3-04` | Use aligned 32-bit stores for even destination X and even pixel count. | Halve store count on the common path. |
| `P3-05` | Keep odd-left and odd-right edge handlers simple. | Preserve exact pixels around unaligned spans. |
| `P3-06` | Add an indexed8 fast path with direct palette lookup and no transparent checks. | Future scenes include indexed8 FG2 packs. |
| `P3-07` | Consider pack-time direct16 row commands for high-cost scenes only. | Removes palette work at runtime but increases pack size. |
| `P3-08` | Keep the old compositor behind a debug token until the new path is validated. | Fast rollback. |

Expected impact: active frames average only 2.4K-4.9K visible pixels, so this
is likely behind held-frame and dirty-upload fixes for fishing. It becomes more
important for scenes like Suzy and Mary with 100K+ visible pixels per frame.

## Phase 4: FG2 Streaming, Prefetch, And CD Access

Goal: reduce synchronous CD operations without reintroducing the fragile
read-ahead behavior called out in the historical timing plan. The first target
is to move next-entry reads into already-idle held VBlanks.

Status: first wave implemented, visually signed off, and merged to `main` in
`1b457163`. Stage1 entry prefetch is default. Later retunes moved the perf
branch to the sector-rounded `16 KB` stream window plus direct-stage seeding.
The old `prefetch-stage1` token is no longer required for the normal path;
`no-prefetch`, `no-stage1`, and window-size tokens remain diagnostic controls.

Current perf-branch target: keep squeezing CD latency and upload cost without
changing pixels. After x-aware restore, PAL4 span compositing, duplicate probe
removal, guarded fallthrough, pad/SPI diagnostic gating, row-level dirty
restore, the `16 KB`/`3` VBlank post-restore retune, per-tile row dirty
marking, the `6` VBlank fallthrough guard, the base-diff OT-clear skip,
the tile-local PAL4 span fast path, vertical dirty-row upload bands with
a post-pause 1-row gap byte trim, setup priming of a `320 KB` first-payload
FG2 window on fishing1 high tide, setup-gated threshold-`4` catch-up,
first FG2 upload scoped to the saved clean-rect Y band,
tight-slack direct staging, direct-stage scratch window seeding,
prepared-wait future prefetch, and the
exact-4 VBlank held-slack prepared-present pass plus leading-empty setup consume and
coalesced FG2 metadata-prefix startup reads plus long-hold host-deadline catch-up,
the exact no-holiday fishing1 high-tide variant now uses the FGP3
zero-shift temporal-residual pack and reports
`loop_vb=1207`, `target_vb=1076`, `overrun_vb=131`, `blocking_vb=0`,
`due_misses=0`, and prefetch `overrun_vb=0`, with
`upload_bytes=8533120`, `restore_bytes=251144`, `upload_rects=436`,
`loop_reads=6`, `setup_reads=6`, and `scene_vb=1447`.
Row-level restore created enough
CPU headroom that CD blocking fell too; the latest dirty-marker cleanup
converted redundant span-side dirty work into more useful prefetch coverage,
setup-prime converted early foreground residency into a safe catch-up window,
and FGP3 residuals removed most high-tide restore/upload/read volume.
Next experiments should target the present/VSync ownership model, the setup-cost
trade, release/perf-log measurement split, generated fishing-scene FGP3 rollout,
and generated pack/read-cost metadata that can stop raw window probes from
perturbing the deterministic cadence.

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
| `P4-58` | Failed as no-op: coalesce consecutive full-width restore rows into one `memcpy()`. | Work identity and timing stayed exactly flat at `loop_vb=1240`, `restore_bytes=2510092`, `blocking_vb=20`, and `prefetch_overrun_vb=11`; future restore wins need pack-emitted bands or less restore work. |
| `P4-59` | Failed as no-op: unroll the PAL4 opaque run loop from two pixels to four pixels per iteration. | Work identity and timing stayed exactly flat at `loop_vb=1240`, `compose_calls=155`, `blocking_vb=20`, and `prefetch_overrun_vb=11`; future compositor wins need generated or pack-specialized command streams. |
| `P4-60` | Failed: lower the stream-window refill guard from `3` to `2` VBlanks after the tile-local PAL4 fast path. | The retest regressed `loop_vb 1240 -> 1245`, `blocking_vb 20 -> 26`, and `prefetch_overrun_vb 11 -> 18`; keep `3` VBlanks as the current lower slack bound. |
| `P4-61` | Failed as no-op: remove the held-loop prefetch-window would-read double-check. | Letting `fgRuntimeTryPrefetchWindow()` perform the single target/window/slack decision left all key metrics flat at `loop_vb=1240`, `blocking_vb=20`, and `prefetch_overrun_vb=11`; retry only inside broader prefetch scheduler cleanup. |
| `P4-62` | Done: split dirty tile uploads into contiguous vertical dirty-row bands. | `upload_bytes 17172480 -> 16387840` and `dirty_rows 26832 -> 25606` with flat `loop_vb=1240`, `blocking_vb=20`, and `prefetch_overrun_vb=11`; next upload work should reduce `upload_rects 351 -> 518` pressure. |
| `P4-63` | Done: merge 2-row clean gaps inside vertical upload bands. | `upload_rects 518 -> 427` with flat timing; `upload_bytes` gives back only `108160` bytes versus the accepted vertical-band split and remains `676480` below the old full-band upload baseline. |
| `P4-64` | Done: prime the first real payload during setup when frame 0 is empty. | `loop_vb 1240 -> 1237`, `blocking_vb 20 -> 13`, and `due_misses 1 -> 0`; setup pays one extra read and `prefetch_overrun_vb` rises `11 -> 13`, so the remaining CD work should target refill overrun rather than due misses. |
| `P4-65` | Done: direct-stage small payloads at the minimum accepted slack. | `loop_vb 1237 -> 1235`, `blocking_vb 13 -> 11`, and `prefetch_overrun_vb 13 -> 11` with `due_misses=0`; this validates a narrow 3-VBlank version of the old failed short-slack direct-stage idea. |
| `P4-66` | Failed: extend direct-stage small payloads to 4 VBlanks of slack. | The retest regressed `loop_vb 1235 -> 1239`, `blocking_vb 11 -> 39`, `prefetch_overrun_vb 11 -> 12`, and `due_misses 0 -> 5`; direct staging at 4 VBlanks loses too much stream-window lookahead until grouped reads or a second stage slot exist. |
| `P4-67` | Failed as no-op: shrink the post-direct-stage stream window to 15 KB. | The parameter probe matched the 16 KB default exactly at `loop_vb=1235`, `blocking_vb=11`, `prefetch_overrun_vb=11`, `due_misses=0`, and `loop_reads=68`; one-step window shrinkage is not a current lever. |
| `P4-68` | Done: seed the stream window from accepted direct-stage reads. | `blocking_vb 11 -> 10`, `prefetch_overrun_vb 11 -> 10`, `read_vb 409 -> 405`, `loop_read_vb 289 -> 285`, `seq 65 -> 66`, and `seek_back 8 -> 7` with flat `loop_vb=1235` and `due_misses=0`; direct-stage sectors are now reused instead of thrown away. |
| `P4-69` | Failed: staged next-VBlank present scheduler. | Detail showed `present_wait_vb=157`, but composing the staged next frame in the final held-loop slack regressed `loop_vb 1235 -> 1306`, `overrun_vb 158 -> 229`, `blocking_vb 10 -> 13`, and `prefetch_overrun_vb 10 -> 13`; present work must not consume the slack currently hiding CD reads. |
| `P4-70` | Done: prepare staged frames during held slack only when at least `4` VBlanks remain. | Two strict runs matched: `loop_vb 1235 -> 1234`, `overrun_vb 158 -> 157`, `blocking_vb 10 -> 8`, `prefetch_overrun_vb 10 -> 8`, and `due_misses=0`; tradeoff is extra RAM prep work (`restore_calls 156 -> 192`, `compose_calls 155 -> 191`), so the next pass should reduce speculative prepare cost. |
| `P4-71` | Failed: restrict prepared-present to exactly `4` held-slack VBlanks. | The equality guard reduced speculative work (`restore_calls 192 -> 168`, `compose_calls 191 -> 167`) but regressed `loop_vb 1234 -> 1235`, `blocking_vb 8 -> 9`, and `prefetch_overrun_vb 8 -> 9`; the accepted `>=4` shape stays as baseline. |
| `P4-72` | Failed: restrict prepared-present to `4` or `5` held-slack VBlanks. | The max-slack cap regressed the same timing (`loop_vb 1234 -> 1235`, `blocking_vb 8 -> 9`) and increased prep calls (`restore_calls 192 -> 206`); guard pruning needs better prepared-used metrics before another attempt. |
| `P4-73` | Failed: lower staged-copy fallthrough from `6` to `5` VBlanks after prepared-present. | Visible timing stayed flat and two prep calls were saved, but no key metric improved and total hidden CD read time increased (`read_vb 407 -> 416`); keep the `6` VBlank guard. |
| `P4-74` | Failed: lower tight-slack direct-stage cap from `8 KB` to `6 KB`. | Exact no-op against the current fishing1 baseline; it does not change the active direct-stage decisions. |
| `P4-75` | Failed: lower tight-slack direct-stage cap from `8 KB` to `4 KB`. | Regressed `loop_vb 1234 -> 1237`, `blocking_vb 8 -> 13`, and `prefetch_overrun_vb 8 -> 13`; the accepted `8 KB` cap remains the local knee. |
| `P4-76` | Failed: stream-window prefetch while a prepared frame waits for its present VBlank. | It removed the prepared-present duplicate prep work (`restore_calls 192 -> 156`, `compose_calls 191 -> 155`) but regressed `loop_vb 1234 -> 1235`, `blocking_vb 8 -> 9`, and `prefetch_overrun_vb 8 -> 9`; do not spend prepared-wait slack on raw reads without a cost predictor or grouped-read metadata. |
| `P4-77` | Failed: shrink the post-prepared-present stream window to `12 KB`. | `prefetch_overrun_vb 8 -> 1`, but coverage collapsed (`due_misses 0 -> 26`, `blocking_vb 8 -> 89`, `loop_vb 1234 -> 1245`); smaller raw windows are exhausted until pack groups or another stage slot preserve near-term entries. |
| `P4-78` | Failed: add a second exact-payload stage slot at the `3` VBlank lower bound. | Coverage still degraded (`due_misses 0 -> 3`, `blocking_vb 8 -> 28`, `loop_vb 1234 -> 1238`, `prefetch_overrun_vb 8 -> 11`); the second slot needs grouped coverage, not isolated direct reads. |
| `P4-79` | Failed: reuse a prepared current-frame RAM background through the normal upload path. | This removed duplicate prepared work (`restore_calls 192 -> 156`, `compose_calls 191 -> 155`) but regressed `loop_vb 1234 -> 1235`, `blocking_vb 8 -> 9`, and `prefetch_overrun_vb 8 -> 9`; the extra prep work is currently timing/pacing, not removable without replacing that pacing. |
| `P4-80` | Failed as unproven: targeted current dirty-row state clearing. | Key timing stayed exactly flat (`loop_vb=1234`, `blocking_vb=8`, `prefetch_overrun_vb=8`) and work identity shifted slightly (`restore_calls 192 -> 190`, `compose_calls 191 -> 189`); retry only with lower-level CPU counters or during a broader dirty-state refactor. |
| `P4-81` | Failed: re-sweep `14/18/20 KB` stream windows after prepared-present. | All tested sizes regressed total loop versus the accepted `16 KB` default (`1236`, `1247`, `1249` vs `1234`), confirming raw window-size tuning is exhausted until grouped/pack-aware reads improve coverage per transaction. |
| `P4-82` | Failed strict gate but promising: consume the leading empty capture artifact during setup. | `loop_vb 1234 -> 1228` and `overrun_vb 157 -> 151` with zero due misses, but `blocking_vb 8 -> 9`, `prefetch_overrun_vb 8 -> 9`, and render count dropped by one non-payload frame; retry with CD smoothing and explicit visual policy for empty artifacts. |
| `P4-83` | Done: consume the leading empty capture artifact during setup with a one-VBlank setup settle. | Strict gates passed: `loop_vb 1234 -> 1227`, `overrun_vb 157 -> 150`, `blocking_vb 8 -> 7`, `prefetch_overrun_vb 8 -> 7`, and `due_misses=0`; render count drops by one non-payload frame and the settle cost is outside active playback. |
| `P4-84` | Failed: consume the leading empty capture artifact with two setup settle VBlanks. | The second setup settle regressed the active loop (`loop_vb 1227 -> 1235`, `blocking_vb 7 -> 13`, `prefetch_overrun_vb 7 -> 13`); one setup settle is the local cadence knee. |
| `P4-85` | Failed/no-op: re-sweep adjacent `15 KB` and `17 KB` raw stream windows after leading-empty setup consume. | `15 KB` tied the accepted `16 KB` default exactly (`loop_vb=1227`, `blocking_vb=7`); `17 KB` regressed to `loop_vb=1238`, `blocking_vb=29`, and `due_misses=2`. |
| `P4-86` | Failed: re-test prepared-current RAM reuse after leading-empty setup consume, including reuse-plus-immediate-prefetch. | Both variants removed duplicate work (`restore_calls 187 -> 155`, `compose_calls 187 -> 155`) but regressed active playback to `loop_vb=1231`, `blocking_vb=12`, and `prefetch_overrun_vb=12`; the removed work still acts as CD-phase pacing. |
| `P4-87` | Failed: expand exact small-payload direct staging from exactly `3` to `3-4` VBlanks of slack. | `loop_vb 1227 -> 1228`, `blocking_vb 7 -> 16`, `prefetch_overrun_vb 7 -> 8`, and `due_misses 0 -> 2`; the accepted exact-read path must stay at the `3` VBlank knee. |
| `P4-88` | Done: widen vertical dirty-upload band clean-gap merge from `2` to `8` rows. | Timing stayed flat (`loop_vb=1227`, `blocking_vb=7`, `prefetch_overrun_vb=7`) while `upload_rects 424 -> 412`; byte cost is small (`upload_bytes 16381440 -> 16424960`) and correctness/fallback counters stayed clean. |
| `P4-89` | Failed/no-op: re-sweep tight-slack direct-stage payload caps after the 8-row upload merge. | `6 KB` and `10 KB` matched baseline exactly; `4 KB` regressed `loop_vb 1227 -> 1230`, `blocking_vb 7 -> 11`, and `prefetch_overrun_vb 7 -> 11`. Keep `8 KB` until grouped reads or another stage model changes the coverage tradeoff. |
| `P4-90` | Failed: split immediate-next and pure-lookahead window refill guards. | Keeping immediate staging at `3` VBlanks but requiring `4` VBlanks for standalone lookahead refills regressed `loop_vb 1227 -> 1230`, `blocking_vb 7 -> 10`, and `prefetch_overrun_vb 7 -> 10`; scalar slack splitting is exhausted without read-cost prediction or pack groups. |
| `P4-91` | Failed: runtime read-size predictor for tight lookahead refills. | An `8 KB` tight-read cap improved nominal `loop_vb 1227 -> 1225`, but increased `blocking_vb` and `prefetch_overrun_vb` to `8`, raised loop CD read time, and changed scheduler cadence; `12 KB` regressed to `loop_vb=1233` and `blocking_vb=14`. |
| `P4-92` | Failed/no promotion: widen dirty-upload band clean-gap merge from `8` to `12` rows. | Timing stayed flat and correctness was clean, but the extra merge only traded fewer upload rectangles (`412 -> 399`) for more uploaded bytes (`16424960 -> 16514560`); the later `11`-row midpoint is accepted, but `12` remains too wide. |
| `P4-93` | Failed: compile hot playback translation units with `-O3`. | It reduced prepared restore/compose calls (`187 -> 182`) but regressed `loop_vb 1227 -> 1229`, `blocking_vb 7 -> 10`, and `prefetch_overrun_vb 7 -> 10`; keep the SDK `-O2` default and prefer targeted hot functions/assembly. |
| `P4-94` | Failed: read tight-slack direct-stage sectors straight into the stream-window buffer. | It removed one local seed copy in theory but regressed `loop_vb 1227 -> 1231`, `loop_reads 68 -> 69`, and `seek_back 7 -> 9`; direct-stage seeding must preserve current window coverage shape. |
| `P4-95` | Failed: skip the held-loop wait when no slack and no prepared frame are available. | The apparent overshoot cleanup regressed `loop_vb 1227 -> 1231`, `blocking_vb 7 -> 10`, `prefetch_overrun_vb 7 -> 10`, and added three restore/compose calls; the wait is currently part of CD/render pacing. |
| `P4-96` | Failed: merge direct-stage scratch sectors into adjacent stream-window coverage. | Correctness stayed clean but timing regressed with the same `loop_vb=1231`, `blocking_vb=10`, `prefetch_overrun_vb=10`, and extra prep calls; direct-stage window-shape work needs per-read evidence first. |
| `P4-97` | Failed: proactively slide/append the stream window while the next future payload is still resident. | Due misses stayed zero, but eager append work regressed `loop_vb 1227 -> 1231`, `blocking_vb 7 -> 13`, and `prefetch_overrun_vb 7 -> 13`; raw lookahead needs group/read-cost metadata. |
| `P4-98` | Done: widen vertical dirty-upload band clean-gap merge from `8` to `10` rows. | Timing stayed flat (`loop_vb=1227`, `blocking_vb=7`, `prefetch_overrun_vb=7`) while `upload_rects 412 -> 409`; byte cost is bounded (`upload_bytes 16424960 -> 16442880`) and correctness/fallback counters stayed clean. |
| `P4-99` | Done: widen vertical dirty-upload band clean-gap merge from `10` to `11` rows. | Timing stayed flat (`loop_vb=1227`, `blocking_vb=7`, `prefetch_overrun_vb=7`) while `upload_rects 409 -> 401`; byte cost is bounded (`upload_bytes 16442880 -> 16499200`) and correctness/fallback counters stayed clean. |
| `P4-100` | Failed: add inline CD-read histogram metrics. | Summary-level and `perf-detail`-gated variants both regressed to `loop_vb=1231`, `blocking_vb=10`, and `prefetch_overrun_vb=10`; read-class metrics must be compile-time isolated or post-processed outside the speed baseline. |
| `P4-101` | Done: coalesce FG2 metadata startup reads. | Active loop stayed flat while `setup_reads 8 -> 6`, `pack_start_vb 55 -> 42`, `scene_vb 1419 -> 1406`, `blocking_vb 7 -> 6`, and `prefetch_overrun_vb 7 -> 6`; tradeoff is `restore_calls/compose_calls 187 -> 190`. |
| `P4-102` | Failed: move FG2 sound events into the metadata prefix. | Setup improved (`setup_reads 6 -> 5`, `pack_start_vb 42 -> 26`, `scene_vb 1406 -> 1401`), but shifting all payload offsets by `36` bytes regressed active playback (`loop_vb 1227 -> 1238`, `blocking_vb 6 -> 18`, `prefetch_overrun_vb 6 -> 18`); preserve payload/sector alignment before retrying setup coalescing. |
| `P4-103` | Failed: resolve the FG2 pack once before startup reads. | Setup improved (`pack_start_vb 42 -> 30`, `scene_vb 1406 -> 1397`), but active playback regressed (`loop_vb 1227 -> 1230`, `blocking_vb 6 -> 8`, `prefetch_overrun_vb 6 -> 8`) and speculative prep calls changed; setup code shape still affects the deterministic loop cadence. |
| `P4-104` | Failed: trim stream-window reads to complete payload entries. | Bytes/sectors dropped (`1098982 -> 1094886`, `541 -> 539`) with `hits=155` and `due_misses=0`, but elapsed CD and active timing regressed (`loop_vb 1227 -> 1240`, `loop_read_vb 286 -> 298`, `blocking_vb 6 -> 13`); trailing overread is part of the current cadence unless group metadata predicts cost. |
| `P4-105` | Failed: prepare staged frames before pure lookahead prefetch. | Prepared work dropped (`restore_calls/compose_calls 190 -> 186`, `restore_bytes 3034562 -> 2981102`) but active playback regressed (`loop_vb 1227 -> 1240`, `blocking_vb 6 -> 18`, `prefetch_overrun_vb 6 -> 18`, `seek_back 5 -> 11`); render prep and CD lookahead need separate budgets, not a simple priority inversion. |
| `P4-106` | Done: enable PS1 function/data sections plus linker garbage collection. | Two headless runs matched timing and work identity exactly while `jcreborn.elf` shrank `709828 -> 708656` bytes; this does not move VBlank metrics yet, but it makes later public-build/code-size cleanup measurable. |
| `P4-107` | Failed: gate foreground ADS-style telemetry writes behind `grPs1TelemetryEnabled`. | Timing stayed flat and correctness stayed clean, but the branch grew `jcreborn.elf` by `68` bytes and shifted speculative work (`restore_calls/compose_calls 190 -> 188`); remove legacy telemetry structurally later instead of adding hot-path conditionals. |
| `P4-108` | Done: remove foreground ADS-style visual telemetry from the hot path. | Two headless runs kept timing and correctness flat while speculative prep dropped (`restore_calls/compose_calls 190 -> 188`, `restore_bytes 3034562 -> 2999408`) and `jcreborn.elf` shrank to `707916` bytes; use printf/perf logs for diagnostics, not legacy visual telemetry writes. |
| `P4-109` | Failed: run future stream-window prefetch while a prepared frame waits for its presentation VBlank. | Duplicate speculative prep work disappeared (`restore_calls/compose_calls 188 -> 155`), but active timing regressed (`loop_vb 1227 -> 1228`, `blocking_vb 6 -> 7`, `prefetch_overrun_vb 6 -> 7`); prepared-frame slack is not safe to spend opportunistically without grouped read-cost metadata. |
| `P4-110` | Failed: raise held-slack prepared-present threshold from `4` to `5` VBlanks. | Only three speculative prep calls were saved (`restore_calls/compose_calls 188 -> 185`), while active timing regressed (`loop_vb 1227 -> 1228`, `blocking_vb 6 -> 8`, `prefetch_overrun_vb 6 -> 8`); the accepted `4` VBlank bridge remains the local knee. |
| `P4-111` | Failed: compile-gate the heavy JCPAD/JCSPI diagnostics block out of default playback. | `jcreborn.exe` shrank `137216 -> 133120` and ELF shrank `707916 -> 700304`, but timing regressed (`loop_vb 1227 -> 1232`, `blocking_vb 6 -> 12`, `prefetch_overrun_vb 6 -> 12`); code-size wins must still pass the deterministic cadence gate. |
| `P4-112` | Failed: pointer-swap current/previous dirty-row tables instead of copying them. | It saved one total loop VBlank and shrank the binary (`jcreborn.exe 137216 -> 135168`), but failed the visible-pressure gate (`blocking_vb 6 -> 9`, `prefetch_overrun_vb 6 -> 9`); render-side micro-wins can still be unsafe if they shift CD cadence. |
| `P4-113` | Failed/no-op: narrow clean-rect restore scanning to previous dirty Y ranges. | Timing and tracked work matched baseline exactly, but ELF size grew (`707916 -> 709856`); runtime row-scan pruning needs finer CPU counters or pack-emitted restore bands before another attempt. |
| `P4-114` | Failed: use a `24 KB` stream window only for lookahead prefetches with at least `8` held VBlanks of slack. | It reduced active reads (`68 -> 56`) and loop CD read time (`285 -> 268`), but visible timing regressed (`loop_vb 1227 -> 1230`, `blocking_vb 6 -> 9`, `prefetch_overrun_vb 6 -> 9`); hidden CD efficiency is not enough unless the read-cost model preserves visible cadence. |
| `P4-115` | Failed: use a smaller adaptive `20 KB` stream window for high-slack lookahead prefetches. | It reduced active reads (`68 -> 59`), loop CD read time (`285 -> 282`), and speculative restore/compose calls (`188 -> 180`), but still regressed visible timing (`loop_vb 1227 -> 1228`, `blocking_vb 6 -> 8`, `prefetch_overrun_vb 6 -> 8`); raw extended windows are exhausted until pack/read-cost metadata can schedule them safely. |
| `P4-116` | Failed: release the staged payload buffer after prepared-frame composition and advance prepared frames from saved metadata. | The window-stage variant removed duplicate restore/compose work (`188 -> 155`) but regressed `loop_vb`, `blocking_vb`, and `prefetch_overrun_vb` by one VBlank and left a suspect final frame cursor; the wait-only variant starved due-frame coverage (`due_misses 0 -> 14`, `blocking_vb 6 -> 72`). Prepared-buffer ownership needs a full scheduler model before retry. |
| `P4-117` | Done: compile out legacy foreground diagnostic scene modes from the default PS1 build. | Timing, CD, prefetch, gfx identity, and correctness stayed flat while `jcreborn.exe` shrank `137216 -> 131072` and `jcreborn.elf` shrank `707916 -> 692612`; code-size cleanup can be promoted when the headless cadence gate is exactly flat. |
| `P4-118` | Failed: mark FG2 PAL4 dirty tile rows directly instead of routing through `grMarkRectDirty()`. | It reduced speculative restore/compose calls (`188 -> 185`) but regressed visible cadence (`loop_vb 1227 -> 1229`, `blocking_vb 6 -> 10`, `prefetch_overrun_vb 6 -> 10`) and grew the ELF; dirty-marker micro-cleanups need scheduler headroom before retry. |
| `P4-119` | Failed: compile out the old visual debug screen/buffer/wait implementation by default. | It saved a full 4 KB executable bucket (`131072 -> 126976`) but regressed active cadence (`loop_vb 1227 -> 1232`, `blocking_vb 6 -> 12`, `prefetch_overrun_vb 6 -> 12`); startup/debug code-shape cleanup must wait for a stronger phase barrier or scheduler model. |
| `P4-120` | Failed: lower held-slack prepared-present threshold from `4` to `3` VBlanks. | It increased speculative prep (`restore_calls/compose_calls 188 -> 193`) and regressed active cadence (`loop_vb 1227 -> 1231`, `blocking_vb 6 -> 10`, `prefetch_overrun_vb 6 -> 10`); the accepted `4` VBlank bridge is still the local knee from both directions. |
| `P4-121` | Failed: grow retained stream-window capacity to `24 KB` while keeping `16 KB` normal reads. | It improved total loop by one VBlank (`1227 -> 1226`) but regressed visible CD pressure (`blocking_vb 6 -> 16`), reintroduced due misses (`0 -> 3`), and increased read churn (`loop_reads 68 -> 80`); raw append growth needs pack/group scheduling before retry. |
| `P4-122` | Failed: fixed `16 KB` payload group alignment in the fishing1 FG2 pack. | It reduced read count (`loop_reads 68 -> 56`) and backward seeks (`5 -> 3`) but grew the pack by `92413` bytes and regressed active playback (`loop_vb 1227 -> 1241`, `blocking_vb 6 -> 22`, `prefetch_overrun_vb 6 -> 22`); pack grouping must be selective/cost-aware. |
| `P4-123` | Failed: align the FG2 payload start to a CD sector with `904` bytes of padding. | It kept due misses at zero but regressed cadence (`loop_vb 1227 -> 1230`, `blocking_vb 6 -> 12`, `prefetch_overrun_vb 6 -> 12`); small global payload shifts are unsafe without preserving the measured offset phase. |
| `P4-124` | Failed: use a pause-poll-only event path for FG2 held/prepared waits. | It reduced speculative prep (`restore_calls/compose_calls 188 -> 183`) but regressed cadence (`loop_vb 1227 -> 1228`, `blocking_vb 6 -> 7`, `prefetch_overrun_vb 6 -> 7`); the zero-delay event tail remains scheduler ballast until FG2 owns the full present/event phase. |
| `P4-125` | Failed: compile out the old visual debug screen while padding the PS-EXE container to preserve CD layout. | `FISHING1.FG2` stayed at LBA `390` and the EXE container stayed `131072` bytes, but active cadence still regressed (`loop_vb 1227 -> 1232`, `blocking_vb 6 -> 12`, `prefetch_overrun_vb 6 -> 12`) while speculative prep dropped (`188 -> 183`); the earlier visual-debug miss was not merely a foreground-pack LBA shift. |
| `P4-126` | Failed: replace dirty-row clear loops with fixed-size `memset(..., 0xff, ...)`. | The direct run regressed with the pack shifted to LBA `389`, and the layout-padded rerun preserved LBA `390` but still failed (`loop_vb 1227 -> 1233`, `blocking_vb 6 -> 10`, `prefetch_overrun_vb 6 -> 10`, `restore_calls/compose_calls 188 -> 189`); keep the explicit row clear until dirty-state layout changes structurally. |
| `P4-127` | Failed: render the first real frame during the existing setup settle after leading-empty consume. | Active playback improved (`loop_vb 1227 -> 1220`) because one render moved before `loop_start`, but full scene time stayed flat (`scene_vb=1406`) and CD pressure regressed (`blocking_vb 6 -> 7`, `prefetch_overrun_vb 6 -> 7`); do not accept setup accounting wins that do not reduce total time or preserve CD phase. |
| `P4-128` | Failed: remove unused compose-ever foreground telemetry from the hot compose path. | Runtime metrics matched baseline exactly (`loop_vb=1227`, `blocking_vb=6`, `prefetch_overrun_vb=6`) and correctness stayed clean, but loadable text grew `124732 -> 124760`; source was reverted and this should wait for a broader telemetry/API pruning pass with map/layout review. |
| `P4-129` | Done: catch up host-deadline timing by one VBlank only after long holds. | Full actual-elapsed catch-up and uncapped one-VBlank catch-up both starved prefetch (`due_misses=18` and `8` respectively). The accepted long-hold-only guard repeated cleanly: `loop_vb 1227 -> 1222`, `scene_vb 1406 -> 1401`, `overrun_vb 150 -> 149`, `blocking_vb=6`, `prefetch_overrun_vb=6`, `due_misses=0`, with stable frame/sound correctness. |
| `P4-130` | Failed: lower long-hold catch-up threshold from `5` to `4` VBlanks. | The threshold-4 probe matched active loop time (`loop_vb=1222`) but regressed target/pressure (`target_vb 1073 -> 1069`, `overrun_vb 149 -> 153`, `blocking_vb 6 -> 9`, `prefetch_overrun_vb 6 -> 9`); keep threshold `5` until grouped prefetch or a better slack budget makes extra catch-up free. |
| `P4-131` | Failed: raise prepared-present minimum slack from `4` to `5` after catch-up. | It reduced duplicate prep (`restore_calls/compose_calls 195 -> 182`) and nominal loop by one VBlank (`1222 -> 1221`) but regressed target/pressure (`target_vb 1073 -> 1069`, `overrun_vb 149 -> 152`, `blocking_vb 6 -> 10`, `prefetch_overrun_vb 6 -> 10`); the duplicate prep still acts as scheduler ballast. |
| `P4-132` | Failed/no-op: add a second catch-up VBlank for very long holds. | Both `>=9` variants matched the accepted baseline exactly (`loop_vb=1222`, `target_vb=1073`, `blocking_vb=6`, `prefetch_overrun_vb=6`); fishing1 exposes no useful extra catch-up in that hold bucket under the current scheduler. |
| `P4-133` | Failed: lower the initial FG2 metadata prefix read from `8 KB` to `4 KB`. | Fishing1 setup bytes fell (`282104 -> 278008`) and `setup_read_vb` dropped by one, but active playback regressed (`loop_vb 1222 -> 1224`, `blocking_vb 6 -> 11`, `prefetch_overrun_vb 6 -> 11`); startup read shape still acts as deterministic cadence ballast. |
| `P4-134` | Failed: extend one-VBlank catch-up to short holds with staged/lookahead coverage. | Both `>=3` and `>=4` covered-catch variants kept `loop_vb=1222` but regressed pressure (`target_vb 1073 -> 1071`, `overrun_vb 149 -> 151`, `blocking_vb 6 -> 8`, `prefetch_overrun_vb 6 -> 8`); coverage alone is not enough to prove spare cadence. |
| `P4-135` | Failed/no-op: cache the no-holiday compose gate for fishing1. | Runtime timing and work identity matched exactly, but ELF size grew (`692704 -> 692924`); the holiday-stamp branch is not a measurable hot-path target in this form. |
| `P4-136` | Done: remove unused foreground "ever" diagnostics. | Two strict runs matched timing/work identity exactly while `jcreborn.elf` shrank `692704 -> 691584`; keep pruning old diagnostic API only when the cadence gate stays flat. |
| `P4-137` | Done: remove unused ADS foreground auto-start hook. | Two strict runs matched timing/work identity exactly while `jcreborn.elf` shrank `691584 -> 690932`; the explicit PS1 `FGPILOT` debug ADS path remains intact. |
| `P4-138` | Done: remove obsolete `FGPILOT` ADS debug dispatch. | Two strict runs matched timing/work identity exactly while `jcreborn.exe` crossed down `131072 -> 129024`; ELF file size moved upward from link-layout noise, but the shipped/loadable executable is smaller. |
| `P4-139` | Done: remove unused foreground status accessors. | Two strict runs matched exactly with a small timing/CD-pressure win: `loop_vb 1222 -> 1221`, `blocking_vb 6 -> 5`, `prefetch_overrun_vb 6 -> 5`, and `overrun_vb=149`; normal build stays in the `129024` byte PS-EXE bucket and narrows the foreground-pilot API surface. |
| `P4-140` | Done: remove dead foreground requested-mode state. | Two strict runs matched the accepted baseline exactly while `jcreborn.elf` shrank `690936 -> 690724`; this removes write-only scene-mode state left behind by the foreground status accessor cleanup. |
| `P4-141` | Done: require base-diff foreground packs. | All `126` generated FG2 packs carry the base-diff flag, so the runtime now rejects non-base-diff packs at startup and drops per-frame non-base-diff fallback checks; two strict runs matched baseline exactly while `jcreborn.elf` shrank `690724 -> 689748`. |
| `P4-142` | Done: restore the default-off JCPAD/JCSPI diagnostics gate after the pause/menu merge. | The post-menu exact no-holiday baseline was `loop_vb=1306`, `blocking_vb=19`, `prefetch_overrun_vb=14`, and `due_misses=1` because the heavy pad diagnostics path was live again. Restoring `pad-diag`/`pad-debug` as opt-in while keeping Start polling always on recovered the accepted cadence: `loop_vb 1306 -> 1221`, `blocking_vb 19 -> 5`, `prefetch_overrun_vb 14 -> 5`, `due_misses 1 -> 0`, with clean correctness. |
| `P4-143` | Failed: re-test raw `18 KB` and `14 KB` stream windows after the pause/menu merge. | The `18 KB` probe regressed `loop_vb 1221 -> 1238`, `blocking_vb 5 -> 25`, and `prefetch_overrun_vb 5 -> 25`; the `14 KB` probe regressed `loop_vb 1221 -> 1224`, `blocking_vb 5 -> 43`, and `due_misses 0 -> 12`. Keep the sector-rounded `16 KB` default until pack groups or a costed scheduler changes useful coverage per read. |
| `P4-144` | Failed: remove dirty-upload band gap merging after the pause/menu merge. | Exact zero-gap bands lowered upload bytes (`16499200 -> 16273280`) but raised upload rectangles (`401 -> 515`) and regressed `loop_vb 1221 -> 1224`, `blocking_vb 5 -> 10`, and `prefetch_overrun_vb 5 -> 10`; byte savings alone are not enough when rect pressure rises that far. |
| `P4-145` | Rejected: skip explicit `CdlSetloc` for sequential CD reads. | Setloc calls dropped (`74 -> 8`) and nominal loop improved (`1221 -> 1217`), but visual-work identity collapsed (`compose_calls 193 -> 6`, `upload_bytes 16499200 -> 1919360`) and visible CD pressure worsened (`blocking_vb 5 -> 8`). Source was reverted; retry only with a proven lower-level CD continuation API and stronger frame/work-identity gates. |
| `P4-146` | Done: combine exact-4 prepared-present gating with a 4-row upload-band gap. | This is a work-reduction checkpoint, not a VBlank speed win: `loop_vb=1221`, `blocking_vb=5`, `prefetch_overrun_vb=5`, and `due_misses=0` stayed flat, while `restore_calls/compose_calls 193 -> 166`, `restore_bytes 3085148 -> 2701496`, `upload_bytes 16499200 -> 16387840`, and `dirty_rows 25780 -> 25606`; tradeoff is `overrun_vb 149 -> 150` and `upload_rects 401 -> 421`, within the accepted flat-timing gate. |
| `P4-147` | Done: tighten the post-pause upload-band gap from `4` rows to `2` rows. | Two exact no-holiday runs kept `loop_vb=1221`, `blocking_vb=5`, `prefetch_overrun_vb=5`, and `due_misses=0` flat while reducing `upload_bytes 16387840 -> 16381440` and `dirty_rows 25606 -> 25596`; tradeoff is `upload_rects 421 -> 424`, still far below the rejected zero-gap `515` rect pressure. |
| `P4-148` | Done: narrow the post-pause upload-band gap from `2` rows to `1` row. | Two exact no-holiday runs again kept `loop_vb=1221`, `blocking_vb=5`, `prefetch_overrun_vb=5`, and `due_misses=0` flat while reducing `upload_bytes 16381440 -> 16281600` and `dirty_rows 25596 -> 25440`; tradeoff is `upload_rects 424 -> 502`, close to but still below the rejected zero-gap failure at `515` rects. |
| `P4-149` | Failed/no-op: reuse a single upload `RECT` instead of the local `RECT[16]` array. | Runtime metrics and work identity matched the accepted baseline exactly, but `jcreborn.elf` grew `713320 -> 713384`; source was reverted and only the experiment log was kept. |
| `P4-150` | Done: prefetch a future stream window while a prepared frame waits for its present VBlank. | The post-gap1 retry kept `loop_vb=1221`, `blocking_vb=5`, `prefetch_overrun_vb=5`, and `due_misses=0` flat while reducing `restore_calls/compose_calls 166 -> 155`, `restore_bytes 2701496 -> 2510092`, `loop_read_vb 289 -> 284`, and `prefetch.used_vb 292 -> 285`; this removes the remaining duplicate prepared work for fishing1 without exposing extra CD pressure. |
| `P4-151` | Failed/no-op: restore the `>=4` prepared-present threshold after prepared-wait prefetch. | Key timing and work identity matched exactly, with only loop bookkeeping movement (`advances 951 -> 957`, `held 868 -> 874`, `late 105 -> 100`); keep exact-`4` until a new scheduler budget makes threshold widening meaningful. |
| `P4-152` | Failed: lower staged-copy fallthrough from `6` to `5` VBlanks after prepared-wait prefetch. | Total loop stayed flat but visible pressure regressed (`blocking_vb 5 -> 6`, `prefetch_overrun_vb 5 -> 6`) and loop read time rose (`284 -> 300`); keep the `6` VBlank guard until grouped reads make lookahead cheaper. |
| `P4-153` | Failed: lower long-hold catch-up threshold from `5` to `4` after prepared-wait prefetch. | The retry still starved CD cadence despite lower duplicate prep: `target_vb 1071 -> 1065`, `overrun_vb 150 -> 156`, `blocking_vb 5 -> 9`, `prefetch_overrun_vb 5 -> 9`, and `blocking_reads 4 -> 8`; keep threshold `5` until grouped/predictive prefetch changes the slack budget. |
| `P4-154` | Failed: remove the prefetch would-read probe. | Deleting the redundant-looking probe reduced held-path checks but shifted CD phase into visible pressure (`loop_vb 1221 -> 1222`, `blocking_vb 5 -> 6`, `prefetch_overrun_vb 5 -> 6`, `loop_read_vb 284 -> 292`); keep small scheduler ballast unless a replacement pacing model proves the phase stays fixed. |
| `P4-155` | Failed/no promotion: preconvert host-tick deadlines during metadata load. | The helper version was runtime-flat but grew `jcreborn.elf 713176 -> 714008`; the tighter parse-loop version still grew the ELF and exited `137` before `JCPERF2`. Keep `fgEntryHoldVBlanks()` unchanged until finer CPU counters prove this conversion is worth moving. |
| `P4-156` | Failed: use an aligned 32-bit PAL4 pair store in the compositor. | The dynamic alignment branch kept visual work identity clean but regressed cadence badly (`loop_vb 1221 -> 1225`, `overrun_vb 150 -> 157`, `blocking_vb 5 -> 12`, `prefetch_overrun_vb 5 -> 12`); compositor wins need generated/assembly code that avoids extra hot-loop branching. |
| `P4-157` | Accepted integration baseline: merge main pause-menu credits/captions work into the perf branch. | The merge is correct but costs one cadence VBlank (`loop_vb 1221 -> 1222`, `blocking_vb 5 -> 6`, `prefetch_overrun_vb 5 -> 6`) and moves `FISHING1.FG2` to LBA `399` because the executable grows to `149504` bytes; future optimization tests now compare against this accepted merged state. |
| `P4-158` | Failed/no promotion: gate default-off caption rendering with `captionsGetEnabled()`. | Runtime metrics stayed exactly flat against the post-main baseline, but the executable grew (`149504 -> 151552`) and ELF grew (`743944 -> 744080`); keep the direct renderer call until captions expose an inline active flag or a release profile can compile captions differently. |
| `P4-159` | Done: compile-gate pause-menu JCPAUSE diagnostics out of the default build. | Runtime metrics stayed exactly flat against the post-main baseline while default-build diagnostic text/code was removed (`jcreborn.elf 743944 -> 743136`, PS-EXE unchanged at `149504`); keep `PAUSE_MENU_DIAG_LOGS` as the explicit opt-in for pause-menu log mining. |
| `P4-160` | Done: compile-gate `graphics_ps1.c` debug-mode diagnostics out of the default build. | Runtime metrics stayed exactly flat against the pause-diagnostics baseline while default-build graphics diagnostic text/code was removed (`jcreborn.elf 743136 -> 740984`, PS-EXE unchanged at `149504`); keep `GRAPHICS_PS1_DIAG_LOGS` as the explicit opt-in for GPU log mining. |
| `P4-161` | Done: compile-gate routine `sound_ps1.c` SPU startup diagnostics out of the default build. | Runtime metrics stayed exactly flat against the graphics-diagnostics baseline while routine sound setup text/code was removed (`jcreborn.elf 740984 -> 740664`, PS-EXE unchanged at `149504`); keep `SOUND_PS1_DIAG_LOGS` as the explicit opt-in for audio log mining. |
| `P4-162` | Done: compile-gate the unconditional `JCBOOT applyBootOverride` buffer dump out of the default build. | Runtime and correctness stayed clean with a small deterministic win (`loop_vb 1222 -> 1221`, `blocking_vb 6 -> 5`, `prefetch_overrun_vb 6 -> 5`, `loop_read_vb 292 -> 284`) while default-build boot diagnostic text/code was removed (`jcreborn.elf 740664 -> 740496`, PS-EXE unchanged at `149504`); keep `JC_BOOT_DIAG_LOGS` as the explicit opt-in for boot-string log mining. |
| `P4-163` | Failed: compile-gate the PS1 resource-count setup diagnostic. | The ELF shrank (`740496 -> 740392`) but the exact gate regressed cadence (`loop_vb 1221 -> 1222`, `blocking_vb 5 -> 6`, `prefetch_overrun_vb 5 -> 6`, `loop_read_vb 284 -> 292`); source was reverted and the resource-count print remains as layout/cadence ballast until CD phase is controlled. |
| `P4-164` | Done: compile-gate PS1 pause request-consumption diagnostics out of the default build. | Runtime metrics stayed exactly flat against the boot-diagnostics baseline while cold pause-loop text/code was removed (`jcreborn.elf 740496 -> 740336`, PS-EXE unchanged at `149504`); keep `JC_PAUSE_REQUEST_DIAG_LOGS` as the explicit opt-in for pause request log mining. |
| `P4-165` | Failed/no promotion: compile-gate the old PS1 `padtest` boot mode. | The exact gate stayed flat and correctness stayed clean, but there was no timing or file-size win (`jcreborn.exe=149504`, `jcreborn.elf=740336`); source was reverted and padtest cleanup is deferred to a future public-cleanup pass rather than the perf loop. |
| `P4-166` | Failed: compile-gate the optional `printf-test` / `logtest` `JCLOG` probe body. | The size win was real (`jcreborn.exe 149504 -> 147456`, `jcreborn.elf 740336 -> 738556`), but the exact gate regressed cadence (`loop_vb 1221 -> 1222`, `blocking_vb 5 -> 6`, `prefetch_overrun_vb 5 -> 6`, `loop_read_vb 284 -> 292`); source was reverted and this becomes a layout-padding/CD-phase-control retry candidate. |
| `P4-167` | Failed: compile-gate memory-card `JCMC` diagnostics out of the default build. | The size win crossed the same PS-EXE sector bucket (`jcreborn.exe 149504 -> 147456`, `jcreborn.elf 740336 -> 738652`) and moved `FG\\FISHING1.FG2` from LBA `399` to `398`, but the exact gate regressed cadence (`loop_vb 1221 -> 1222`, `blocking_vb 5 -> 6`, `prefetch_overrun_vb 5 -> 6`, `loop_read_vb 284 -> 292`); source was reverted and memcard logs remain layout/CD-phase ballast until foreground LBA and code phase can be controlled together. |
| `P4-168` | Failed: remove unused-looking `ISLETEMP.SCR` from the active CD layout. | The FG2 pack LBA stayed fixed at `399`, but setup/CD-layout phase still regressed the exact gate (`loop_vb 1221 -> 1222`, `blocking_vb 5 -> 6`, `prefetch_overrun_vb 5 -> 6`, `read_vb 393 -> 401`) and the emulator emitted invalid-read spam after `JCPERF2`; source was reverted and CD asset pruning needs a startup phase barrier plus exit sanity gate before promotion. |
| `P4-169` | Failed: compile-gate automatic foreground heap probes out of default playback. | The source reverted after the exact gate regressed through the same layout/CD-phase shape as other one-sector shrink misses: `FG\\FISHING1.FG2 LBA 399 -> 398`, `loop_vb 1221 -> 1222`, `blocking_vb 5 -> 6`, `prefetch_overrun_vb 5 -> 6`, `loop_read_vb 284 -> 292`; keep the automatic FGHEAP probe code as ballast until foreground LBA/cold-section control exists or CD phase is explicit. |
| `P4-170` | Done: skip disabled caption rendering through an inline-readable enabled flag. | The prior getter-gate probe grew the executable, but exposing the existing caption state as `ps1CaptionsEnabled` keeps the exact no-caption fishing1 cadence flat (`loop_vb=1221`, `blocking_vb=5`, `prefetch_overrun_vb=5`, `due_misses=0`) and leaves the shipped PS-EXE at `149504`; this is a hot-path cleanup rather than a VBlank-level speed win. |
| `P4-171` | Done: skip disabled foreground-pilot ADS caption scene lookup. | Guarding the fishing1/fishing2/fishing3 `captionsOnAdsStart()` lookup behind `ps1CaptionsEnabled` keeps the exact no-caption fishing1 cadence and work identity flat while avoiding default scene-start caption string checks; `jcreborn.exe` remains `149504` and `FG\\FISHING1.FG2` remains LBA `399`. |
| `P4-172` | Failed/no promotion: reuse the window-prefetch candidate entry in guarded held-loop paths. | The refactor kept exact no-caption fishing1 timing and work identity flat, but it did not improve any tracked counter and grew the ELF (`740568 -> 741216` after tightening, `742828` in the first draft); source was reverted and this should wait for a broader prefetch state-machine rewrite or finer CPU counters. |
| `P4-173` | Failed/no-op: lower the direct-stage payload cap from `8 KB` to `7 KB`. | The exact no-caption fishing1 gate matched baseline across timing, CD, prefetch, gfx, pack LBA, and binary size (`loop_vb=1221`, `blocking_vb=5`, `prefetch_overrun_vb=5`, `loop_reads=68`, `jcreborn.exe=149504`); source was reverted and direct-stage cap tuning remains exhausted. |
| `P4-174` | Done: lower the local dirty-upload rectangle cap from `16` to `8`. | Fishing1's measured `max_upload_rects=6`, so the smaller cap preserves exact upload behavior (`upload_rects=502`, `cap_hits=0`) while trimming stack/binary pressure slightly (`jcreborn.elf 740568 -> 740556`, PS-EXE unchanged at `149504`). |
| `P4-175` | Failed/no promotion: lower the local dirty-upload rectangle cap from `8` to `6`. | The exact fishing1 gate stayed flat and `cap_hits=0`, but there was no additional binary win over cap `8` and no headroom above fishing1's measured `max_upload_rects=6`; source was reverted to the safer cap `8` until broader scene validation proves a tighter cap. |
| `P4-176` | Failed: narrow `grDrawBackground()` upload bookkeeping locals. | The ELF shrank (`740556 -> 740516`) with identical upload work, but the exact gate regressed cadence (`loop_vb 1221 -> 1225`, `blocking_vb 5 -> 12`, `prefetch_overrun_vb 5 -> 12`, `blocking_reads 4 -> 10`); source was reverted and this should only be retried with map/register review or upload-path translation-unit isolation. |
| `P4-177` | Failed: prepare staged frames at `3` VBlanks as well as the accepted exact-`4` point. | It improved nominal deadline accounting (`target_vb 1071 -> 1073`, `overrun_vb 150 -> 148`, `late 105 -> 100`) but did not reduce actual loop time and regressed visible CD pressure (`blocking_vb 5 -> 6`, `prefetch_overrun_vb 5 -> 6`); source was reverted and present-prep changes need an explicit render-prep/CD slack budget before retry. |
| `P4-178` | Failed: read tight-slack direct-stage sectors straight into the stream window. | An `8 KB` aligned-window cap improved actual loop time (`1221 -> 1219`) but moved one more read into visible pressure (`blocking_vb/prefetch_overrun_vb 5 -> 6`, `blocking_reads 4 -> 5`, `loop_read_vb 284 -> 293`); a `6 KB` cap was an exact no-op. Source was reverted; retry only with grouped/predicted direct-stage windows that preserve the current blocking-read count. |
| `P4-179` | Done: enforce work-identity floors in the headless perf harness. | The new default `75%` floor on `timing.render`, `gfx.restore_calls`, `gfx.compose_calls`, and `gfx.upload_calls` keeps the exact fishing1 baseline passing (`155 -> 155` for all four) while rejecting future false speedups that silently skip most visual work. |
| `P4-180` | Done: pre-apply scene-relative FG2 offsets at startup. | Random island placement is preserved because offsets are applied after variant selection; the exact fishing1 gate stayed flat across timing/CD/work identity while `jcreborn.elf` shrank `740556 -> 740132` and pack flags still report `scene_relative=1`. |
| `P4-181` | Done: inline FG2 entry draw offsets after startup pre-apply. | The helper functions became trivial after `P4-180`; using `entry->x/y` directly kept the exact fishing1 gate flat across timing/CD/work identity while `jcreborn.elf` shrank `740132 -> 739684` and `jcreborn.exe` stayed in the same `149504` byte bucket. |
| `P4-182` | Done: collapse identical held-loop prefetch branches. | Prepared-frame and staged-frame states share the same window-prefetch behavior; merging the duplicate branches kept the exact fishing1 gate flat across timing/CD/work identity while `jcreborn.elf` shrank `739684 -> 739552` and `jcreborn.exe` stayed in the same `149504` byte bucket. |
| `P4-183` | Failed/no promotion: pass `NULL` for discarded next-payload frame-index outputs. | The exact fishing1 gate stayed flat, but the source change did not improve any tracked speed/work metric and grew `jcreborn.elf` `739552 -> 739568`; source was reverted and this should wait for broader prefetch helper cleanup. |
| `P4-184` | Failed/no promotion: combine adjacent no-slack perf-log guards. | The exact fishing1 gate stayed flat, but combining the two `ps1PerfEnabled` checks in each no-slack prefetch path grew `jcreborn.elf` `739552 -> 739664`; source was reverted and cosmetic hot-path guard combining should be avoided unless metrics improve. |
| `P4-185` | Done: remove duplicate compose active guard. | `grUpdateDisplay()` already guards calls to `foregroundPilotRuntimeCompose()` and the compose function still no-ops by mode; removing the duplicate active check kept the exact fishing1 gate flat across timing/CD/work identity while `jcreborn.elf` shrank `739552 -> 739544`. |
| `P4-186` | Failed/no promotion: remove the active check from `fgRuntimeCanHoldDisplayedFrame()`. | The change crossed a PS-EXE sector bucket (`149504 -> 147456`) and moved `FG\\FISHING1.FG2` to LBA `398`, but active playback regressed (`loop_vb 1221 -> 1222`, `blocking_vb/prefetch_overrun_vb 5 -> 6`); source was reverted and this becomes a layout-preservation retry candidate. |
| `P4-187` | Failed/no promotion: remove the active check from `fgRuntimeMarkFrameRendered()`. | This also crossed the PS-EXE sector bucket and moved `FG\\FISHING1.FG2` to LBA `398`, producing the same active playback regression (`loop_vb 1221 -> 1222`, `blocking_vb/prefetch_overrun_vb 5 -> 6`); source was reverted and active-guard pruning now needs explicit CD-layout preservation. |
| `P4-188` | Failed/no promotion: retry mark-rendered active-guard removal with one-sector CD padding. | The temporary padding restored `FG\\FISHING1.FG2` to LBA `399`, but the exact gate still regressed (`loop_vb 1221 -> 1222`, `blocking_vb/prefetch_overrun_vb 5 -> 6`); the miss is code-layout/scheduler phase too, not just physical FG2 LBA. |
| `P4-189` | Failed: disable stream-window append extension. | Removing the append/memmove path collapsed sequential window behavior (`seq 66 -> 0`, `seek_back 5 -> 71`) and badly regressed CD pressure (`blocking_vb/prefetch_overrun_vb 5 -> 26`); keep append preservation and target smarter grouped/appended reads instead. |
| `P4-190` | Done: simplify `foregroundPilotRuntimeActive()`. | Returning the runtime byte directly kept the exact fishing1 gate flat across timing/CD/work identity while `jcreborn.elf` shrank `739544 -> 739540` and `jcreborn.exe` stayed in the same `149504` byte bucket. |
| `P4-191` | Failed/no-op: remove the ternary from `fgRuntimeWindowSlackEligible()`. | The exact fishing1 gate and binary size matched baseline exactly (`jcreborn.elf=739540`), so the source was reverted; retry only as part of broader prefetch helper inlining. |
| `P4-192` | Failed/no-op: remove the ternary from `fgEntryHasPayload()`. | The exact fishing1 gate and binary size matched baseline exactly (`jcreborn.elf=739540`), so the source was reverted; isolated boolean-helper cleanup is exhausted unless part of a larger hot-helper rewrite. |
| `P4-193` | Failed/no promotion: cache `ps1PerfEnabled` inside `fgRuntimeTryPrefetchWindow()`. | The local cache crossed the PS-EXE sector bucket and moved `FG\\FISHING1.FG2` to LBA `398`, regressing active playback (`loop_vb 1221 -> 1222`, `blocking_vb/prefetch_overrun_vb 5 -> 6`); source was reverted and perf-flag cache refactors need layout control. |
| `P4-194` | Done: add a PS-EXE sector-bucket gate to `ps1-perf-iterate`. | The harness now records `jcreborn.exe` bytes, sector bucket, sector count, and ELF bytes in each summary/JSONL row; once a baseline contains these fields, sector-bucket changes fail by default unless `--allow-layout-change` is explicit. |
| `P4-195` | Done: add a foreground-pack LBA comparison gate. | Baseline comparisons now include `scene.pack_lba`, so `FG\\FISHING1.FG2 LBA 399 -> 398` becomes an immediate layout-identity failure instead of a manual clue found after CD metric triage. |
| `P4-196` | Done: parse build map deltas for hot functions. | `ps1-perf-iterate` now records a selected hot-symbol snapshot from `jcreborn.map` and reports address/size deltas against the baseline without failing the gate; this gives code-layout evidence before retrying helper/cache refactors. |
| `P4-197` | Queued: test deterministic executable padding independent of ISO padding. | The CD-padded retry preserved FG2 LBA but still regressed; try linker/EXE text padding to preserve code address phase, not just file placement. |
| `P4-198` | Queued: isolate foreground runtime into a layout-stable translation unit section. | Keep hot FG2 scheduler/compositor code addresses stable while allowing cold diagnostics and pause/menu code to shrink independently. |
| `P4-199` | Queued: pack-emitted append groups with current payload offsets preserved. | Fixed 16 KB group padding failed; instead emit group metadata that describes cheap append windows without moving payload bytes. |
| `P4-200` | Done: host-side group planner for fishing1 read sequence. | `ps1-perf-cdlog-summary.py --pack-file` now parses the FG2 entry table and emits zero-extra-sector group candidates; on the accepted fishing1 log it proposes `69 -> 46` reads at `12` sectors, `69 -> 29` reads at `16` sectors, and `69 -> 20` reads at `24` sectors while preserving current payload offsets. |
| `P4-201` | Partial: runtime group lookup that only changes read length, not frame identity. | Broad 12-sector grouping failed (`loop_reads 68 -> 66` but `blocking_vb 5 -> 10`). The narrow fishing1 high-tide tail group `396..406` is accepted as work reduction (`loop_reads 68 -> 67`, `setloc 74 -> 73`, flat key timing), proving selective groups are valid but must be costed boundary-by-boundary. |
| `P4-202` | Queued: append-cost predictor based on sectors plus preserved tail. | Byte-only predictors failed; model `appendBytes`, `preserveBytes`, sector count, and current slack before starting any lookahead read. |
| `P4-203` | Queued: block the fifth visible read specifically. | Current baseline has `blocking_reads=4`; rejected variants move it to `5`. Identify the exact read shape and target only that transition. |
| `P4-204` | Queued: diagnostic-only per-read trace binary. | Inline CD histograms regressed the speed baseline; create a separate non-promotable trace mode/binary for read sequence analysis. |
| `P4-205` | Done: host-side DuckStation log read-sequence extractor. | `scripts/ps1-perf-cdlog-summary.py` parses `Setloc`/`ReadN` lines and anchors them to the scene pack LBA from `summary.json`; the accepted fishing1 run shows `69` post-locate FG2 reads from LBA `400` through `801`, matching the legacy active-read count without PS1-side instrumentation. |
| `P4-206` | Queued: setup-to-loop cadence barrier experiment. | Several setup/code-shape changes perturb active cadence; test a deterministic settle/barrier that makes loop start phase independent of setup reads. |
| `P4-207` | Queued: non-`perf-log` speed baseline comparison. | The accepted loop currently measures with summary logging enabled; compare a non-logging run to decide whether perf probes themselves are now a significant scheduling actor. |
| `P4-208` | Queued: release-vs-perf dual baseline policy. | If `perf-log` materially changes cadence, maintain a diagnostic baseline and a release-speed baseline instead of optimizing the logging build only. |
| `P4-209` | Queued: retry active-guard/code-size removals under code-address padding. | The source changes are semantically valid and size-positive; they become candidates again only if text/code phase can be held constant. |
| `P4-210` | Queued: narrow `fgRuntimeWindowPrefetchWouldRead()` without removing pacing. | Prior removal shifted phase; try a layout-stable inline/read-only variant only after map gating exists. |
| `P4-211` | Queued: append-preserving direct-stage seed v2. | Direct read-into-window and seed merge both failed; retry only with exact current-window/tail metrics and no extra backward seek. |
| `P4-212` | Queued: pack-time upload bands for high-rect frames. | Upload byte volume remains large but rect-count tuning is locally exhausted; generated dirty/upload bands may reduce CPU/command work without widening bytes. |
| `P4-213` | Queued: cross-scene validation for cap `8` upload rects and append behavior. | Before wider promotion, run fishing2/fishing3 plus representative high/low tide variants to ensure fishing1-local knees are not hiding scene-specific regressions. |
| `P4-214` | Failed/no promotion: remove the duplicate prepared-present guard. | `fgRuntimePresentPreparedFrame()` is currently called only after `fgRuntimeCanPresentPreparedOnNextVBlank()`, but removing the internal guard regressed visible CD pressure despite fixed layout (`blocking_vb/prefetch_overrun_vb 5 -> 7`, `blocking_reads 4 -> 5`); keep the guard as scheduler/code-shape ballast until prepared-present is redesigned. |
| `P4-215` | Failed/no promotion: inline the window-contained check inside `fgRuntimeWindowPrefetchWouldRead()`. | The exact gate stayed flat, but the helper grew by `60` bytes and shifted downstream hot symbols; keep the shared helper call until a broader prefetch-state rewrite can reduce code size and cadence together. |
| `P4-216` | Failed: allow `24 KB` stream-window reads only at `10+` held VBlanks. | Decoupling normal `16 KB` reads from a larger high-slack capacity kept `loop_vb=1221`, but regressed `target_vb 1071 -> 1069`, `overrun_vb 150 -> 152`, `blocking_vb 5 -> 8`, and `prefetch_overrun_vb 5 -> 8`; group/read-cost metadata must be more specific than a long-hold threshold. |
| `P4-217` | Done: skip legacy CD accumulator writes for modern foreground reads. | The exact gate stayed flat while `jcreborn.elf` shrank `739540 -> 739524` and the buffered/aligned CD helper symbols each shrank by `8` bytes; `JCPERF2` remains authoritative and the legacy print now mirrors modern CD totals. |
| `P4-218` | Queued: compiler/linker/toolchain flag matrix with layout gates. | The old hot-TU `-O3` probe failed, but narrower flags remain valid targets: per-file `-Os`/`-O2`/`-O3`, hot/cold translation-unit splits, function alignment, linker section ordering, and code-address padding must each pass the same exact timing/layout gate. |
| `P4-219` | Queued: group-cost predictor for runtime append groups. | The accepted tail group and rejected broad group show that transaction count is insufficient; score each candidate by append sectors, preserved bytes, current slack, host-observed read cost, and whether it risks creating the fifth visible read. |
| `P4-220` | Queued: move read-group boundaries into generated pack metadata. | The hard-coded fishing1 tail group is a proving slice. The durable version should emit per-pack group metadata without moving payload offsets, then let runtime consume scene-authored groups with the same strict cadence gates. |
| `P4-221` | Failed/no promotion: collapse the one-entry tail read-group table to direct constants. | Timing and CD work stayed flat, but the ELF grew (`741076 -> 741404`) and `fgRuntimeFillWindowForEntry` grew by `100` bytes; keep the table form until group metadata/codegen replaces the hard-coded slice. |
| `P4-222` | Done: tighten the tail read-group retained capacity to 11 sectors. | The exact gate stayed flat (`loop_vb=1221`, `blocking_vb=5`, `prefetch_overrun_vb=5`, `loop_reads=67`) while the prefetch buffer dropped `31760 -> 29712` bytes and ELF/PS-EXE size stayed flat. |
| `P4-223` | Failed/no promotion: tighten the tail read-group retained capacity to 10 sectors. | Timing stayed flat and the buffer would have dropped to `27664` bytes, but the accepted saved read vanished (`loop_reads 67 -> 68`, `group_hits=0`); keep 11 sectors as the current safe lower bound. |
| `P4-224` | Done: compile the default-off captions translation unit with `-Os`. | The exact gate stayed flat across timing/CD/layout/work identity and hot-symbol addresses while `jcreborn.elf` shrank `741076 -> 740816`; this validates narrow cold-TU compiler probes under the layout gates. |
| `P4-225` | Done: compile the memcard translation unit with `-Os`. | The exact gate stayed flat across timing/CD/layout/work identity and hot-symbol addresses while `jcreborn.elf` shrank `740816 -> 740196`; continue cold-TU flag probes one file at a time. |
| `P4-226` | Failed/no promotion: compile the holidays translation unit with `-Os`. | The ELF shrank to `737352`, but the PS-EXE crossed `149504 -> 147456`, moved `FISHING1.FG2` LBA `399 -> 398`, and regressed visible CD pressure (`blocking_vb/prefetch_overrun_vb 5 -> 6`); defer until layout/code-phase control exists. |
| `P4-227` | Failed/no-op: compile the generated holidays table translation unit with `-Os`. | The exact gate stayed flat, but binary size did not move (`jcreborn.elf=740196`, PS-EXE bucket `149504`); the table is data-dominated under current flags. |
| `P4-228` | Failed/no promotion: prepared-stage decoupling v2. | Decoupling prepared visual state from staged payload first tripped correctness at frames `30`/`54`; after fixing ownership and blocking CD reads during prepared waits, the exact fishing1 gate was correctness-clean but flat (`loop_vb=1221`, `blocking_vb=5`, `prefetch_overrun_vb=5`). Retry only with a first-class prepared-present scheduler and explicit prepared/CD-budget metrics, not as another local state tweak. |
| `P4-229` | Queued: true double-buffered prepared visual state. | The no-promotion result shows one RAM background can only safely hold one prepared visual state; a real frame-ahead pipeline likely needs a second prepared RAM/VRAM target or pack-emitted upload-ready state so preparing frame+2 does not overwrite frame+1 or steal CD deadline slack. |
| `P4-230` | Failed/no promotion: adjacent fishing1 tail group `384..396`. | Both the table-only form and an explicit adjacent-append variant kept timing/read counts flat, did not change the runtime read shape, and raised the retained buffer `29712 -> 31760`; keep the accepted 11-sector capacity until runtime append-start evidence or generated group metadata identifies a real next group. |
| `P4-231` | Done: compile the PS1 debug translation unit with `-Os`. | The exact gate stayed flat across timing/CD/layout/work identity and tracked hot symbol addresses/sizes while `jcreborn.elf` shrank `740196 -> 739948`; continue cold/default-off TU flag probes while the PS-EXE bucket remains stable. |
| `P4-232` | Failed/no promotion: compile the pause-menu translation unit with `-Os`. | The ELF shrank `739948 -> 735944`, but the PS-EXE crossed `149504 -> 147456`, moved `FISHING1.FG2` LBA `399 -> 398`, and regressed visible CD pressure (`loop_vb 1221 -> 1222`, `blocking_vb/prefetch_overrun_vb 5 -> 6`, `blocking_reads 4 -> 5`); defer large cold-code shrink probes until layout padding/cold-section control or phase-independent CD scheduling exists. |
| `P4-233` | Failed/no promotion: compile pause-menu with `-Os` plus one-sector CD padding. | Preserving `FISHING1.FG2 LBA=399` with `<dummy sectors="1"/>` did not recover cadence: `loop_vb 1221 -> 1222` and `blocking_vb/prefetch_overrun_vb 5 -> 6` still regressed while PS-EXE bucket shrank `149504 -> 147456`; the problem is executable/load/startup phase, so prioritize a setup-to-loop cadence barrier or phase-independent CD scheduler before retrying large size wins. |
| `P4-234` | Failed/no promotion: reset the foreground scene clock at `loop_start`. | Resetting `sceneClockTick` immediately before `ps1PerfMarkLoopStart()` was flat by itself but shifted hot symbols by `+4` bytes and did not rescue the `pause_menu.c -Os` shrink; the useful barrier must stabilize CD/controller/scheduler phase, not only foreground elapsed-time accounting. |
| `P4-235` | Failed/no promotion: direct-fill the inferred `384..396` tail group. | Adding direct grouped fills and raising retained group capacity to `12` sectors kept timing/read shape flat but only increased prefetch memory (`29712 -> 31760`); inferred CD-log group candidates need runtime append/window-start traces before they are worth adding. |
| `P4-236` | Failed/no promotion: switch to PSn00bSDK Release libraries. | Release SDK libraries shrink the executable bucket and improve raw loop by one VBlank, but they raise visible CD pressure (`blocking_vb/prefetch_overrun_vb 5 -> 8`) even with pack LBA preserved; retry only after CD scheduling is phase-independent enough to accept the size/codegen win. |
| `P4-237` | Failed/no promotion: compile the SPI translation unit with `-Os`. | The exact gate stayed timing-flat, but `jcreborn.elf` grew `739948 -> 740092`; keep SPI at default `-O2` until a controller/input benchmark proves otherwise. |
| `P4-238` | Failed/no promotion: compile the PS1 stubs translation unit with `-Os`. | The ELF shrank slightly, but the PS-EXE crossed `149504 -> 147456`, moved `FISHING1.FG2` LBA `399 -> 398`, and regressed visible CD pressure (`blocking_vb/prefetch_overrun_vb 5 -> 6`); defer tail-stub size work until layout/phase control exists. |
| `P4-239` | Failed/no promotion: compile holidays with `-Os` plus one-sector CD padding. | Keeping `FISHING1.FG2 LBA=399` did not recover cadence (`loop_vb 1221 -> 1222`, `blocking_vb/prefetch_overrun_vb 5 -> 6`); one-sector executable shrink remains unsafe without phase-independent scheduling. |
| `P4-240` | Failed/no promotion: remove duplicate rendered/held perf guards. | The cleanup shifted hot symbols by `-40` bytes and regressed CD pressure even with pack LBA restored by padding (`blocking_vb/prefetch_overrun_vb 5 -> 6`); do not remove hot-path ballast until code phase is controlled. |
| `P4-241` | Failed/no promotion: compile `foreground_pilot.c` with `-O3`. | Whole-TU `-O3` grew the executable bucket, moved `FISHING1.FG2` LBA `399 -> 400`, grew key foreground functions, and regressed visible CD pressure while leaving `loop_vb` flat; avoid broad hot-TU flag probes. |
| `P4-242` | Failed/no promotion: allow 4-VBlank catch-up only when the next prepared frame and following stream-window payload are resident. | The safety-gated short-hold catch-up was correctness-clean and layout-stable but exact no-op (`loop_vb=1221`, `target_vb=1071`, `blocking_vb=5`, `prefetch_overrun_vb=5`); local threshold guards are exhausted for fishing1, so present-wait work needs a structural prepared/dual-buffer scheduler or explicit hold rebalance. |
| `P4-243` | Failed/no promotion: move `cdrom_ps1.c` beside `foreground_pilot.c` in link order. | The hot-CD link-order bucket moved tracked symbols by about `+9496` bytes but key timing/CD/layout stayed exact (`loop_vb=1221`, `blocking_vb=5`, `prefetch_overrun_vb=5`, `FISHING1.FG2 LBA=399`); simple whole-TU order changes are not enough, so use layout control to unlock specific known size/codegen wins instead. |
| `P4-244` | Failed/no promotion: hoist `grDrawBackground()` tile screen coordinates into static tables. | The upload-coordinate table probe kept key metrics flat but grew `grDrawBackground` by `24` bytes and `jcreborn.elf 739948 -> 740092`; the compiler's local constants are better than the static-table shape, so focus upload work on pack-emitted bands or fewer commands. |
| `P4-245` | Failed/no promotion: compile `sound_ps1.c` with `-Os`. | The audio TU shrink reduced `jcreborn.elf 739948 -> 739776` and crossed PS-EXE `149504 -> 147456`, but cadence regressed (`loop_vb 1221 -> 1222`, `blocking_vb/prefetch_overrun_vb 5 -> 6`) even after restoring `FISHING1.FG2 LBA=399` with a dummy CD sector; this is another executable/load-phase miss. |
| `P4-246` | Failed/no promotion: compile `resource.c` with `-Os`. | The resource TU shrink reduced `jcreborn.elf 739948 -> 738376`, but crossed PS-EXE `149504 -> 147456`, moved `FISHING1.FG2 LBA 399 -> 398`, shifted foreground hot symbols by `-240`, and regressed visible CD pressure (`blocking_vb/prefetch_overrun_vb 5 -> 6`). |
| `P4-247` | Failed/no promotion: compile `ps1_perf.c` with `-O3`. | The perf TU `-O3` probe grew `jcreborn.elf 739948 -> 742300`, expanded `ps1PerfMarkCdReadDetailed` by `912` bytes, crossed PS-EXE `149504 -> 147456`, moved `FISHING1.FG2 LBA 399 -> 398`, and regressed visible CD pressure (`blocking_vb/prefetch_overrun_vb 5 -> 6`). |
| `P4-248` | Failed/no promotion: compile `ps1_perf.c` with `-Os`. | The perf TU `-Os` probe was cadence- and layout-flat and shrank only the ELF (`739948 -> 737472`); `jcreborn.exe` stayed `149504` while hot perf functions grew, so there was no runtime-size or speed win to accept. |
| `P4-249` | Done: remove the unused `targetVBlanks` argument from `ps1PerfMarkAdvance()`. | The exact gate stayed flat (`loop_vb=1221`, `blocking_vb=5`, `prefetch_overrun_vb=5`, `FISHING1.FG2 LBA=399`) while the foreground advance helper shrank by `4` bytes and `jcreborn.elf 739948 -> 739900`; this is a flat hot-path cleanup, not a VBlank speed win. |
| `P4-250` | Failed/no promotion: remove the unused `ps1PerfMarkCdRead()` wrapper. | The wrapper deletion is semantically valid but crossed PS-EXE `149504 -> 147456`, moved `FISHING1.FG2 LBA 399 -> 398`, expanded/rephased `ps1PerfMarkCdReadDetailed` by `912` bytes, and regressed visible CD pressure (`blocking_vb/prefetch_overrun_vb 5 -> 6`); keep it as layout ballast until code-phase control exists. |
| `P4-251` | Done: add host-side fifth-visible-read comparison. | `ps1-perf-cdlog-summary.py --compare` now compares a candidate CD log summary against the accepted baseline without touching the PS1 binary, reports `JCPERF2` deltas, and ranks file-sector-normalized read timing candidates. Against the wrapper-removal miss it correctly flags `blocking_reads +1` and points at sector `106..110` / entry `38` as the largest positive timing suspect. |
| `P4-252` | Failed/no promotion: add a hard-coded fishing1 group for sectors `106..117`. | The new fifth-read locator made this region worth testing, but the runtime append path never used the group (`group_hits=0`) and all timing/CD counters stayed flat while the ELF grew `739900 -> 740444`; retry only after append-start tracing or generated group metadata proves the group can fire. |
| `P4-253` | Failed/no promotion: raise retained group capacity to `15` sectors for the `106..117` probe. | Capacity was not the blocker: timing/CD shape stayed exact no-op, `group_hits=0`, and the only measured changes were bad (`heap.prefetch 29712 -> 37904`, `jcreborn.elf 739900 -> 740444`). Do not add more early manual groups before tracing append starts. |
| `P4-254` | Failed/no promotion: compile `graphics_ps1.c` with `-O3`. | Whole-TU graphics optimization grew the PS-EXE two sectors, moved `FISHING1.FG2` to LBA `401`, expanded `grDrawBackground` and restore code, and regressed cadence (`loop_vb 1221 -> 1225`, `blocking_vb/prefetch_overrun_vb 5 -> 11`); graphics codegen needs helper-scoped or generated paths, not broad `-O3`. |
| `P4-255` | Failed/no promotion: compile only `grCompositePacked4SpansToBackground()` with `-O3`. | Function-scoped `O3` did shrink the PAL4 compositor by `28` bytes, but it regressed cadence both before and after restoring `FISHING1.FG2 LBA=399` with a dummy CD sector (`loop_vb 1221 -> 1226`, `blocking_vb/prefetch_overrun_vb 5 -> 11`). Do not retry GCC `O3` on this helper; move compositor work to generated/assembly shapes if needed. |
| `P4-256` | Done: parse delivered CD sectors host-side. | `ps1-perf-cdlog-summary.py` now counts DuckStation `DataSector` lines per `ReadN` and converts physical DataSector LBAs back to logical LBAs, so group planning uses actual delivered sector spans instead of next-read inference. Baseline fishing1 now shows `408` delivered active-loop pack sectors and a `12`-sector grouping plan of `68 -> 47` reads with `sector_delta=-3`; keep using this host-side evidence before adding more runtime groups. |
| `P4-257` | Failed/no promotion: add a hard-coded fishing1 group for sectors `384..396` with `13`-sector capacity. | Delivered-sector planning showed a real late two-read span before the accepted tail group, but the runtime probe was timing/CD exact no-op while increasing prefetch heap `29712 -> 33808` and growing `fgRuntimeFillWindowForEntry` by `72` bytes. Do not add more manual groups until append-start/slack ownership tracing proves they can save an actual read. |
| `P4-258` | Done: compile `foreground_pilot.c` with `-Os`. | Unlike the failed foreground `-O3` probe, `-Os` kept the exact fishing1 cadence and work identity flat while shrinking PS-EXE `149504 -> 145408` and ELF `739900 -> 727716`; the branch baseline now allows the deliberate `FISHING1.FG2 LBA 399 -> 397` shift because timing/CD/correctness stayed unchanged. Count this as a binary-size/code-shape win, not a VBlank speed win. |
| `P4-259` | Done: retry `holidays.c -Os` after the foreground-size baseline. | The old holidays-size failure no longer regresses cadence under the new code/CD phase: the exact no-holiday gate stays flat (`loop_vb=1221`, `blocking_vb=5`, `prefetch_overrun_vb=5`, `FISHING1.FG2 LBA=397`, PS-EXE `145408`) while ELF shrinks `727716 -> 724868`. Count as cumulative size cleanup only, not a VBlank or loaded-executable-size win. |
| `P4-260` | Done: retry `resource.c -Os` after the foreground-size baseline. | The old resource-size failure is also phase-safe now: the exact no-holiday gate stays flat (`loop_vb=1221`, `blocking_vb=5`, `prefetch_overrun_vb=5`, `FISHING1.FG2 LBA=397`, PS-EXE `145408`) while ELF shrinks `724868 -> 723284`. Count as cumulative size cleanup only, not a VBlank or loaded-executable-size win. |
| `P4-261` | Done: retry `sound_ps1.c -Os` after the foreground-size baseline. | The old sound-size failure is now phase-safe and sound-correct: the exact no-holiday gate stays flat (`loop_vb=1221`, `blocking_vb=5`, `prefetch_overrun_vb=5`, `sound_events=9`, `sound_late=0`, `sound_cursor_end=9`, `FISHING1.FG2 LBA=397`, PS-EXE `145408`) while ELF shrinks `723284 -> 723104`. Count as cumulative size cleanup only, not a VBlank or loaded-executable-size win. |
| `P4-262` | Done: retry `pause_menu.c -Os` after the foreground-size baseline. | The old pause-menu size failure is now phase-safe for the fishing1 perf path: exact no-holiday cadence stays flat (`loop_vb=1221`, `blocking_vb=5`, `prefetch_overrun_vb=5`, `FISHING1.FG2 LBA 397 -> 396`) while PS-EXE shrinks `145408 -> 143360` and ELF shrinks `723104 -> 719096`. Count as loaded-executable size cleanup only; pause UI still needs normal visual/input validation before merging. |
| `P4-263` | Done: retry `ps1_stubs.c -Os` after the pause-menu size baseline. | The old stubs-size failure is now phase-safe but tiny: exact no-holiday cadence stays flat (`loop_vb=1221`, `blocking_vb=5`, `prefetch_overrun_vb=5`, `FISHING1.FG2 LBA=396`, PS-EXE `143360`) while ELF shrinks `719096 -> 719048`. Count as cumulative size cleanup only, not a VBlank or loaded-executable-size win. |
| `P4-264` | Done: compile `events_ps1.c` with `-Os`. | Event/pause code is phase-safe under the current baseline: exact no-holiday cadence stays flat (`loop_vb=1221`, `blocking_vb=5`, `prefetch_overrun_vb=5`, `FISHING1.FG2 LBA=396`, PS-EXE `143360`) while ELF shrinks `719048 -> 717796`. Count as cumulative size cleanup only; pause/input still needs normal visual validation before merging. |
| `P4-265` | Done: compile `utils.c` with `-Os`. | Utility support code is phase-safe under the current baseline: exact no-holiday cadence stays flat (`loop_vb=1221`, `blocking_vb=5`, `prefetch_overrun_vb=5`, `FISHING1.FG2 LBA=396`, PS-EXE `143360`) while ELF shrinks `717796 -> 716584`. Count as cumulative size cleanup only, not a VBlank or loaded-executable-size win. |
| `P4-266` | Failed/no-op: compile `uncompress.c` with `-Os`. | The exact no-holiday gate stayed flat, but no tracked size or runtime metric moved (`loop_vb=1221`, `blocking_vb=5`, `prefetch_overrun_vb=5`, PS-EXE `143360`, ELF `716584`); source was reverted and only the experiment log was kept. |
| `P4-267` | Done: compile `island.c` with `-Os`. | The exact no-holiday fixed-island gate stayed flat (`loop_vb=1221`, `blocking_vb=5`, `prefetch_overrun_vb=5`, `FISHING1.FG2 LBA=396`, PS-EXE `143360`) while ELF shrinks `716584 -> 716340`; tracked hot-symbol addresses shifted by `-48` bytes without changing cadence. Count as small cumulative size cleanup only. |
| `P4-268` | Done: compile `jc_reborn.c` with `-Os`. | The exact no-holiday gate stayed flat (`loop_vb=1221`, `blocking_vb=5`, `prefetch_overrun_vb=5`, `FISHING1.FG2 LBA=396`, PS-EXE `143360`) while ELF shrinks `716340 -> 714520`; tracked hot-symbol addresses shifted by `-404` bytes without changing cadence. Count as cumulative main-TU size cleanup only, and keep UI validation separate. |
| `P4-269` | Failed/no promotion: compile `cdrom_ps1.c` with `-Os`. | The CD TU did shrink PS-EXE `143360 -> 141312`, move `FISHING1.FG2 LBA 396 -> 395`, and shrink helper symbols, but it regressed cadence (`loop_vb 1221 -> 1224`, `blocking_vb/prefetch_overrun_vb 5 -> 10`); source was reverted and only the experiment log was kept. |
| `P4-270` | Failed/no promotion: cache file LBA in `ps1_streamReadFromCdFileIntoBuffered()`. | The exact gate stayed timing/layout-flat, but the helper grew by `8` bytes and ELF grew `714520 -> 714808`; source was reverted and only the experiment log was kept. |
| `P4-271` | Done: combine upload perf guards in `grDrawBackground()`. | The exact gate stayed timing/layout/work-flat while `grDrawBackground` shrank by `64` bytes and ELF shrank `714520 -> 714440`; count as hot upload-path code cleanup only, not a VBlank speed win. |
| `P4-272` | Failed/no promotion: cache the upload perf guard in a local. | The exact gate stayed timing/layout/work-flat and `grDrawBackground` shrank another `56` bytes, but total ELF grew `714440 -> 714456` with no speed metric movement; source was reverted and only the experiment log was kept. |
| `P4-273` | Failed/no promotion: inline the perf-detail check at rendered-frame call sites. | The exact gate stayed timing/layout/work-flat and hot functions shrank (`foregroundPilotPlay -24`, `grUpdateDisplay -40`), but total ELF grew `714440 -> 714564` with no speed metric movement; source was reverted and only the experiment log was kept. |
| `P4-274` | Done: combine upload and dirty-rect perf markers. | The exact gate stayed timing/layout/work-flat while preserving upload/dirty counters and shrinking ELF `714440 -> 714260`; count as perf-log hot-path cleanup only, not a VBlank speed win. |
| `P4-275` | Done: compile only `grDrawBackground()` with `-Os`. | The exact gate stayed timing/layout/work-flat while `grDrawBackground` shrank by `32` bytes and ELF shrank `714260 -> 713672`; count as hot upload-function code-shape cleanup only, not a VBlank speed win. |
| `P4-276` | Done: compile only `grUpdateDisplay()` with `-Os`. | The exact gate stayed timing/layout/work-flat while `grUpdateDisplay` shrank by `40` bytes and ELF shrank `713672 -> 713496`; count as hot display-wrapper code-shape cleanup only, not a VBlank speed win. |
| `P4-277` | Failed/no promotion: compile only `grRestoreBgFromRects()` with `-Os`. | The exact gate stayed timing/layout/work-flat and the function shrank `980 -> 108` bytes, but total ELF grew `713496 -> 714132` with no speed metric movement; source was reverted and only the experiment log was kept. |
| `P4-278` | Failed/no promotion: compile only `grCompositePacked4SpansToBackground()` with `-Os`. | The compositor and PS-EXE shrank, but cadence regressed badly (`loop_vb 1221 -> 1225`, `blocking_vb 5 -> 26`, `prefetch_overrun_vb 5 -> 12`, `due_misses 0 -> 3`) both before and after restoring `FISHING1.FG2 LBA=396` with a temporary CD pad; source/layout were reverted and only the experiment log was kept. |
| `P4-279` | Failed/no promotion: single-band narrow upload through primitive-buffer scratch. | The exact gate stayed timing/layout-flat but upload work did not move (`upload_rects=502`, `upload_bytes=16281600`, `dirty_rows=25440`) while `grDrawBackground` grew by `200` bytes and ELF grew `713496 -> 716736`; source was reverted and only the experiment log was kept. |
| `P4-280` | Done: clear only touched current dirty rows. | The exact gate stayed timing/layout/work-flat while removing full current dirty-row-table clears from every frame and shrinking ELF `713496 -> 712692`; count as hot dirty-state cleanup only, not a VBlank speed win. |
| `P4-281` | Done: promote only touched current dirty rows into previous dirty rows. | The exact gate improved `loop_vb 1221 -> 1219`, `overrun_vb 150 -> 147`, and `scene_vb 1400 -> 1398` while keeping `blocking_vb=5`, `prefetch_overrun_vb=5`, layout, graphics work, and correctness stable; count as a real hot dirty-state speed win. |
| `P4-282` | Failed/no promotion: direct PAL4 row dirty marking. | The formal gate passed only because `target_vb 1072 -> 1073` made `overrun_vb 147 -> 146`; actual `loop_vb` stayed `1219`, graphics/CD work stayed flat, and the compositor/ELF grew materially (`jcreborn.elf 712828 -> 715112`), so source was reverted and only the experiment log was kept. |
| `P4-283` | Failed/no promotion: skip clearing previous dirty rows that overlap the new current dirty range. | The exact gate stayed timing/layout/work-flat with no speed movement, while ELF grew `712828 -> 713544`; source was reverted and only the experiment log was kept. |
| `P4-284` | Failed/no promotion: retest `20 KB` and `18 KB`/`17 KB` stream windows after dirty-row cleanup. | `20 KB` regressed active playback (`loop_vb 1219 -> 1226`, `blocking_vb 5 -> 29`, `prefetch_overrun_vb 5 -> 19`, `due_misses 0 -> 2`); `18 KB` and `17 KB` rounded into the same 9-sector read shape and failed structurally with repeated invalid reads before metrics. Keep the `16 KB` default until generated group metadata/cost prediction exists. |
| `P4-285` | Done: cache the file LBA in the aligned CD read helper. | The exact gate stayed timing/layout/work-flat while `ps1_streamReadAlignedIntoFile` shrank `540 -> 532` bytes and ELF shrank `712828 -> 712744`; count as hot CD helper code-shape cleanup only, not a VBlank speed win. |
| `P4-286` | Failed/no promotion: add a single-chunk fast path to the aligned CD read helper. | The exact gate stayed timing/layout/work-flat with no speed movement, while `ps1_streamReadAlignedIntoFile` grew `532 -> 636` bytes; source was reverted and only the experiment log was kept. |
| `P4-287` | Done: compile only the aligned CD read helper with `-Os`. | The exact gate stayed timing/layout/work-flat while `ps1_streamReadAlignedIntoFile` became a tiny wrapper (`532 -> 8` bytes) and ELF shrank `712744 -> 712556`; count as hot CD helper code-shape cleanup only, not a VBlank speed win. |
| `P4-288` | Failed/no promotion: compile only the buffered CD read helper with `-Os`. | The exact gate stayed timing/layout/work-flat with no speed movement, while ELF grew `712556 -> 712636` and the buffered helper stayed `604` bytes; source was reverted and only the experiment log was kept. |
| `P4-289` | Failed/no promotion: lower staged-copy fallthrough guard from `6` to `5` VBlanks after CD helper cleanup. | The exact gate regressed active playback (`loop_vb 1219 -> 1221`, `blocking_vb 5 -> 10`, `prefetch_overrun_vb 5 -> 10`); source was reverted and only the experiment log was kept. |
| `P4-290` | Failed/no promotion: raise staged-copy fallthrough guard from `6` to `7` VBlanks after CD helper cleanup. | The run failed structurally before metrics: headless log exceeded `512 MB` and regtest exited `137`; source was reverted and only the experiment log was kept. |
| `P4-291` | Done: compile only the unbuffered CD stream helper with `-Os`. | The exact gate stayed timing/layout/work-flat while `ps1_streamReadFromCdFile` shrank `680 -> 624` bytes and ELF shrank `712556 -> 712524`; count as CD helper code-shape cleanup only, not a VBlank speed win. |
| `P4-292` | Done: cache the file LBA in the unbuffered CD stream helper. | The exact gate stayed timing/layout/work-flat while `ps1_streamReadFromCdFile` shrank `624 -> 592` bytes and ELF shrank `712524 -> 712332`; count as CD helper code-shape cleanup only, not a VBlank speed win. |
| `P4-293` | Failed/no promotion: compile only `fgRuntimeFillWindowForEntry()` with `-Os`. | The exact gate stayed timing/layout/work-flat, but PS-EXE, ELF, and tracked hot symbol sizes also stayed exact; source was reverted and only the experiment log was kept. |
| `P4-294` | Failed/no promotion: prepared visual metadata decoupling v1. | Correctness stayed clean and key timing stayed flat, but the change only added duplicate prefetch probes (`0 -> 130`) and code growth (`ELF 712332 -> 713072`) without staging farther ahead. |
| `P4-295` | Failed/no promotion: prepared visual decoupling with stage-next branch v2. | Correctness stayed clean and key timing stayed flat; secondary CD/read metrics improved slightly (`loop_read_vb 280 -> 277`, `late 98 -> 85`) but code grew and no actual loop VBlank win appeared. |
| `P4-296` | Failed/no promotion: decoupled prepared visual with `>=4` prepare threshold v3. | The run failed structurally before metrics: headless log exceeded `512 MB` and regtest exited `137`; all prepared-decoupling source changes were reverted. |
| `P4-297` | Failed/no promotion: prepared visual stage-next branch with positive-slack guard v4. | Correctness stayed clean and key timing stayed flat; it reproduced v2's secondary read/late improvements but also kept the no-slack attempts and code growth, so all source changes were reverted. |
| `P4-298` | Failed/no promotion: add fishing1 read group `102..110`. | The candidate saved one read (`loop_reads 67 -> 66`) and three sectors, but regressed cadence (`loop_vb 1219 -> 1220`, `blocking_vb/prefetch_overrun_vb 5 -> 8`, `loop_read_vb 280 -> 293`); source was reverted and only the experiment log was kept. |
| `P4-299` | Done: reuse one stack `RECT` for immediate background uploads. | The exact gate stayed timing/layout/work-flat while `grDrawBackground` shrank `1580 -> 1572` bytes and ELF shrank `712332 -> 712272`; count as hot upload-path cleanup only, not a VBlank speed win. |
| `P4-300` | Failed/no promotion: lower upload rect cap from `8` to `6`. | Fishing1 stayed exact-flat with `cap_hits=0`, but PS-EXE, ELF, hot-symbol size, timing, and upload work did not move; source was reverted to preserve cross-scene headroom. |
| `P4-301` | Failed/no promotion: store upload band scratch indices/ranges as `uint8`. | The exact gate stayed timing/work-flat and `grDrawBackground` shrank `1572 -> 1544`, but total ELF grew `712272 -> 712372`; source was reverted and only the experiment log was kept. |
| `P4-302` | Done: add scheduler ownership counters and explicit CD/prep ownership markers. | The exact no-holiday gate stayed timing/layout/work/correctness-flat (`loop_vb=1219`, `blocking_vb=5`, `prefetch_overrun_vb=5`, `FISHING1.FG2 LBA=396`, PS-EXE `143360`) while adding `JCPERF2 sched`; this is instrumentation only and grows ELF to `715432`. |
| `P4-303` | Failed/no promotion: owned-idle 4 VBlank catch-up prototype. | The ownership-gated catch-up rule produced no useful fishing1 catch-up slots (`catchup_idle=0`), kept timing flat, and moved PS-EXE/LBA (`143360 -> 145408`, `396 -> 397`), so the behavior was reverted and only the experiment log was kept. |
| `P4-304` | Done: compile-gate deep pipeline/slack trace counters. | The ungated trace changed EXE layout and regressed timing, so the accepted form is `PS1_PERF_DEEP_TRACE=OFF` by default with optional trace builds for ownership analysis. |
| `P4-305` | Failed/no promotion: prepared bridge slack `>=4`. | It moved work into different held-slack buckets but left `loop_vb=1219`, `blocking_vb=5`, and `prefetch_overrun_vb=5`; broad prepared eligibility is exhausted without a real scheduler change. |
| `P4-306` | Failed/no promotion: global threshold-`4` host-deadline catch-up. | The global guard regressed to `loop_vb=1222` and `blocking_vb/prefetch_overrun_vb=16`; threshold `4` needs resident-payload proof. |
| `P4-307` | Failed/no promotion: setup-prime `256 KB` without catch-up. | Active reads dropped sharply, but active loop stayed flat/slower and visible pressure rose to `6`; preloading has to be spent by the scheduler to matter. |
| `P4-308` | Failed/no promotion: setup-prime `192 KB`/`256 KB` with threshold-`4` catch-up. | `256 KB` was close but still raised visible pressure to `6`, while `192 KB` lost; the coverage boundary is real. |
| `P4-309` | Done: setup-prime a `320 KB` fishing1 high-tide FG2 window and gate threshold-`4` catch-up on prime success. | Active-loop metrics improve (`loop_vb 1219 -> 1215`, `overrun_vb 147 -> 140`, `blocking_vb/prefetch_overrun_vb 5 -> 1`, `loop_reads 67 -> 43`) with stable correctness/layout, but setup rises (`setup_vb 185 -> 246`), so future work must hide or generate the prime. |
| `P4-310` | Done: scope FG2 first upload to the saved clean-rect Y band. | The static backdrop is already presented before clean-rect setup, so the first forced upload no longer dirties all four screen tiles. Active-loop metrics improve (`loop_vb 1215 -> 1213`, `overrun_vb 140 -> 138`), `max_upload_bytes` drops `614400 -> 221440`, and layout/CD/correctness stay stable. |

Prefetch variants to test in order:

| Variant | Description | Expected signal |
|---|---|---|
| One-entry synchronous staging | During held VBlanks, read the next entry into a second buffer if it is not already staged. | `cd_vb` may remain nonzero but should move out of due-frame advancement; visible speed should improve if enough hold budget exists. |
| One-entry async staging | Start `CdRead` during held time and poll completion over later held VBlanks. | Lower blocking time, but higher controller-state risk. |
| 16 KB stream window | Read a forward window from the current FG2 file and serve several entries from RAM. | Current default for fishing1 after later retunes paired with the 3 VBlank refill guard. |
| Setup-prime stream window | Fill a larger resident window before active playback, then enable shorter catch-up only if that prime succeeded. | Current fishing1 high-tide path uses `320 KB`; future variants need generated per-scene/tide sizing and inter-scene preload. |
| 24 KB/32 KB/64 KB stream windows | Larger diagnostic windows. | Useful only if later grouping/async work can hide larger refill reads. |
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
| `P5-13` | Done: add host-side FG2 preprocessing opportunity analyzer. | `scripts/analyze-fg2-preprocess-plans.py` parses an existing pack and reproduces runtime restore/upload counters before any runtime-format change. Fishing1 exact baseline matches `restore_bytes=2510092`, `restore_intervals=24300`, `upload_bytes=16281600`, and `upload_rects=502`. Exact restore skip predicts `52.41%` byte savings but `73417` intervals; coalesced `min8px_max4pieces` still saves `26.35%` with `36450` intervals. Align4 upload-ready bands save `49.61%` upload bytes but cost about `8.2 MB` of frame-band payload. |
| `P5-14` | Failed: runtime restore-under-current parser. | Parsing current PAL4 spans before restore lowered restore bytes, but every tested runtime variant regressed timing or layout (`loop_vb 1219 -> 1221/1224`, visible CD pressure `5 -> 6/11`, pack LBA `396 -> 397/395`). Continue with pack-emitted side metadata or FGP3 command streams, not hot-path reparse. |
| `P5-15` | Done: add host-side FG2 motion-comp analyzer. | `scripts/analyze-fg2-motion-comp.py` proves fishing1 is not a true translation canary: its best candidates are zero-shift temporal residuals (`151/154` pairs, `71.16%` estimated payload reduction, `0` nonzero shifts). Walking packs do have true move candidates (`WALKSTUF1` has `85`, `WALK1LOW` has `53`). Split FGP3 into zero-shift residual first, then move/residual once RAM mirror and dirty cleanup are designed. |
| `P5-16` | Done: model zero-shift temporal residual runtime work. | Fishing1's zero-shift model predicts compose payload `823277 -> 228087` (`72.30%` saved), full-width dirty upload `15667200 -> 6576000` (`58.03%` saved), and cleanup restore of `136552` bytes. Next implementation target is FGP3 zero-shift residuals with full-current dirty metadata. |
| `P5-17` | Done: promote first FGP3 zero-shift temporal-residual runtime pack for fishing1 high tide. | `scripts/build-fg3-temporal-residual-pack.py` converts the accepted PAL4 FG2 into `fgp3_pal4_residual`; runtime cleanup spans restore vanished pixels and residual PAL4 spans draw changed pixels. Exact high-tide gate improves `loop_vb 1213 -> 1207`, `overrun_vb 138 -> 131`, `blocking_vb/prefetch_overrun_vb 1 -> 0`, `loop_reads 43 -> 6`, `restore_bytes 2510092 -> 251144`, and `upload_bytes 15888640 -> 6690560`. The intentional cost is PS-EXE `143360 -> 145408` and `FISHING1.FG2 LBA 396 -> 397`; next work should fold generation into the batch builder and recover the executable-size cost. |
| `P5-18` | Done: promote FGP3 zero-shift temporal residuals for fishing1 low tide. | `FISH1LOW.FG2` now uses `fgp3_pal4_residual`; low-tide gate improves `loop_vb 1215 -> 1209`, `overrun_vb 142 -> 135`, `blocking_vb/prefetch_overrun_vb 5 -> 4`, `loop_reads 31 -> 22`, `restore_bytes 1234716 -> 182892`, and `upload_bytes 11457920 -> 5278080`. The low pack LBA stays stable at `592`, and high tide remains exact-flat after the change. |
| `P5-19` | Done: setup-prime fishing1 low tide FGP3. | The existing `320 KB` setup-prime policy now covers both fishing1 tides. Low tide improves `loop_vb 1209 -> 1207`, `overrun_vb 135 -> 131`, `blocking_vb/prefetch_overrun_vb 4 -> 0`, and `loop_reads 22 -> 0`; high tide remains exact-flat. Setup cost rises (`setup_vb 182 -> 238`), so the follow-up remains generated prime budgets or inter-scene preload. |
| `P5-20` | Done: promote FGP3 zero-shift temporal residuals for fishing2 high tide. | `FISHING2.FG2` now uses `fgp3_pal4_residual`; high-tide gate improves `loop_vb 1928 -> 1903`, `overrun_vb 190 -> 139`, `blocking_vb 50 -> 8`, `prefetch_overrun_vb 44 -> 8`, `due_misses 2 -> 0`, `loop_reads 134 -> 40`, `restore_bytes 5353808 -> 333876`, and `upload_bytes 28215040 -> 9055360`. Fishing2 low smoke still passes. |
| `P5-21` | Done: setup-prime fishing2 high tide FGP3 with an explicit scene/tide budget. | A `352 KB` budget improves `loop_vb 1903 -> 1898`, `overrun_vb 139 -> 133`, `blocking_vb/prefetch_overrun_vb 8 -> 2`, and `loop_reads 40 -> 14`. The runtime now stores `setupPrimeWindowBytes` explicitly so setup priming is policy-driven, not inferred from buffer capacity. Larger contiguous probes were rejected: `384 KB`/`544 KB` failed before playback and `368 KB` hit the log cap. |
| `P5-22` | Failed: manual fishing2 high read group for sectors `178..191`. | A local source-table group moved PS-EXE `145408 -> 147456`, shifted `FISHING2.FG2 LBA 740 -> 741`, and regressed `loop_vb 1898 -> 1899` plus `blocking_vb/prefetch_overrun_vb 2 -> 3`. Retry only with generated read metadata and layout control. |
| `P5-23` | Done: promote FGP3 zero-shift temporal residuals for fishing2 low tide. | `FISH2LOW.FG2` now uses `fgp3_pal4_residual`; low-tide gate improves `loop_vb 1912 -> 1900`, `overrun_vb 157 -> 136`, `blocking_vb/prefetch_overrun_vb 20 -> 5`, `loop_reads 58 -> 27`, `restore_bytes 5353808 -> 220390`, and `upload_bytes 28215040 -> 6727040`. Fishing2 high stays exact-flat. |
| `P5-24` | Done: setup-prime fishing2 low tide FGP3 with a `256 KB` budget. | Low tide improves `loop_vb 1900 -> 1898`, `overrun_vb 136 -> 131`, `blocking_vb/prefetch_overrun_vb 5 -> 0`, and `loop_reads 27 -> 10`. A `320 KB` low-tide prime failed structurally with a log-cap/regtest `137`; the promoted source also shortens cold foreground diagnostics to keep PS-EXE at `145408`. |
| `P5-25` | Done: promote FGP3 zero-shift temporal residuals for fishing3 high tide. | `FISHING3.FG2` now uses `fgp3_pal4_residual`; high-tide gate improves `loop_vb 2123 -> 2099`, `overrun_vb 189 -> 149`, `blocking_vb 87 -> 24`, `prefetch_overrun_vb 39 -> 21`, `due_misses 11 -> 1`, and pack bytes `1831749 -> 724829`. Fishing3 low smoke passes after the downstream LBA shift. |
| `P5-26` | Done: promote FGP3 zero-shift temporal residuals for fishing3 low tide. | `FISH3LOW.FG2` now uses `fgp3_pal4_residual`; low-tide gate improves `loop_vb 2110 -> 2098`, `overrun_vb 156 -> 138`, `blocking_vb 21 -> 8`, `prefetch_overrun_vb 21 -> 9`, `loop_reads 65 -> 42`, and pack bytes `906053 -> 549622`. Fishing3 high stays exact-flat. |
| `P5-27` | Failed: contiguous setup-prime for fishing3 high tide. | `320 KB` failed before playback and moved layout; `256 KB` completed but kept `blocking_vb=24`, worsened `due_misses 1 -> 2`, and still moved PS-EXE/LBA. Retry through segmented/generated prime coverage, not a hard-coded contiguous window. |
| `P5-28` | Done: setup-prime fishing3 low tide FGP3 with a `256 KB` budget. | Low tide improves `loop_vb 2098 -> 2091`, `overrun_vb 138 -> 134`, `blocking_vb 8 -> 7`, `prefetch_overrun_vb 9 -> 7`, and `loop_reads 42 -> 24`; fishing3 high stays exact-flat. The accepted source also shortens cold heap/raw foreground diagnostics to keep PS-EXE at `145408` and `FISH3LOW.FG2` at LBA `1549`. Larger/other fishing3 prime work should move to generated segmented coverage. |
| `P5-29` | Done: retune fishing3 low tide setup-prime to `288 KB`. | The planner showed the `256 KB` prime stopped at the next active read boundary. Raising only low tide to `288 KB` keeps `loop_vb=2091` while improving `overrun_vb 134 -> 131`, `blocking_vb 7 -> 4`, `prefetch_overrun_vb 7 -> 4`, and `loop_reads 24 -> 21`; fishing3 high stays exact-flat. Do not jump to `320 KB` without segmented coverage because the earlier probe failed structurally. |
| `P5-30` | Done: setup-prime fishing3 high tide FGP3 with a `128 KB` budget. | A smaller high-tide prime succeeds where `256 KB`/`320 KB` failed: high tide improves `loop_vb 2099 -> 2094`, `overrun_vb 149 -> 139`, `blocking_vb 24 -> 16`, `prefetch_overrun_vb 21 -> 11`, and `loop_reads 52 -> 44`; fishing3 low stays exact-flat. Setup cost rises (`setup_vb 191 -> 213`), so this remains an active-loop win until inter-scene preload exists. |
| `P5-31` | Done: setup-prime fishing3 high tide relative sectors `67..73` as a scratch segment. | A hard-coded segment read proves non-contiguous setup coverage can move high tide without a larger contiguous prime. High tide improves `loop_vb 2094 -> 2093` with stable layout, while `overrun_vb=139`, `blocking_vb=16`, and `prefetch_overrun_vb=11` stay flat; low tide validates exact-flat. Setup cost rises to `setup_reads=7` / `setup_bytes=409128`. The source also gates default-off heap-probe logging out of the speed build to keep PS-EXE at `145408`. Next step is generated segment metadata or inter-scene preload, not more one-off hot source tables. |
| `P5-32` | Done: setup-prime fishing3 low tide relative sectors `146..152` with owned storage. | The first low-segment probe proved the timing signal but failed teardown because it reused volatile streaming scratch. The promoted version keeps high tide on the accepted scratch-backed `67..73` path and gives low tide an owned segment buffer. Low tide improves `loop_vb 2091 -> 2090`, `overrun_vb 131 -> 130`, `blocking_vb/prefetch_overrun_vb 4 -> 3`, and `loop_read_vb 91 -> 85`; high tide stays exact-flat. Setup cost rises to `setup_reads=7` / `setup_bytes=572968`. |
| `P5-33` | Done: inline fishing3 setup-segment selection. | Removing the one-call selector helper keeps high/low FISHING3 timing exact-flat while shrinking the accepted low-segment build (`jcreborn.elf 726268 -> 726068`, `foregroundPilotPlay 8312 -> 8308`). This is a work-reduction/code-size promotion, not a VBlank win. |
| `P5-34` | Done: remove fixed low-segment size tracking. | The owned fishing3 low-tide setup segment has one fixed size and is released with the other scene-stream buffers, so the separate size tracker was dead state. Removing it keeps high/low FISHING3 timing exact-flat, preserves PS-EXE `145408` and pack LBAs, and shrinks `foregroundPilotPlay 8308 -> 8280`. This is a work-reduction/code-size promotion, not a VBlank win. |
| `P5-35` | Done: prune redundant setup-segment pointer checks. | `setupSegmentPrimed` is the authoritative validity bit after setup reads and runtime reset, so the extra pointer checks in the segment window/copy path were redundant. Removing them keeps high/low FISHING3 timing exact-flat, preserves PS-EXE `145408` and pack LBAs, and shrinks `jcreborn.elf 725872 -> 725828`. This is a work-reduction/code-size promotion, not a VBlank win. |
| `P5-36` | Done: prune duplicate window-cache gate from segment/window containment. | Callers already prove the window-capable path or safely fall through when no stream window or setup segment is valid, so the extra `fgRuntimeCanWindowCache()` gate in `fgRuntimeWindowContainsEntry()` was redundant. Removing it keeps high/low FISHING3 timing exact-flat, preserves PS-EXE `145408` and pack LBAs, and shrinks `jcreborn.elf 725828 -> 725820`. This is a work-reduction/code-size promotion, not a VBlank win. |
| `P5-37` | Done: prune duplicate destination-null guard from window copy. | `fgRuntimeCopyEntryFromWindow()` is internal and all callers pass prevalidated scene frame or prefetch buffers, so the `dst == NULL` guard was dead. Removing it keeps high/low FISHING3 timing exact-flat, preserves PS-EXE `145408` and pack LBAs, and shrinks `jcreborn.elf 725820 -> 725720`. This is a work-reduction/code-size promotion, not a VBlank win. |
| `P5-38` | Done: macro-expand FG2 sector alignment helpers. | Converting the tiny sector-align helpers to macros removes their callable bodies without changing stream-window behavior. High/low FISHING3 stayed exact-flat, PS-EXE stayed `145408`, pack LBAs stayed `1195/1549`, and `jcreborn.elf` shrank `725720 -> 725348`. This is a work-reduction/code-size promotion, not a VBlank win. |
| `P5-39` | Done: macro-expand FG2 window read-size helper. | The window read-size helper is a two-field selection used only by the stream-window fit/fill path. Macro expansion keeps high/low FISHING3 exact-flat, preserves PS-EXE `145408` and pack LBAs, and shrinks `jcreborn.elf 725348 -> 725164`. This is a work-reduction/code-size promotion, not a VBlank win. |
| `P5-40` | Done: macro-expand window slack eligibility helper. | The prefetch slack-eligibility helper is a single threshold compare used in two hot scheduling paths. Macro expansion keeps high/low FISHING3 exact-flat, preserves PS-EXE `145408` and pack LBAs, and shrinks `jcreborn.elf 725164 -> 725008`. This is a work-reduction/code-size promotion, not a VBlank win. |
| `P5-41` | Done: macro-expand hold-frame predicate. | `fgRuntimeCanHoldDisplayedFrame()` had one hot-loop call site and a three-field predicate. Macro expansion keeps high/low FISHING3 exact-flat, preserves PS-EXE `145408` and pack LBAs, and shrinks `jcreborn.elf 725008 -> 724876`. This is a work-reduction/code-size promotion, not a VBlank win. |
| `P5-42` | Done: remove duplicate prepared-present guard. | The foreground loop checks `fgRuntimeCanPresentPreparedOnNextVBlank()` immediately before calling `fgRuntimePresentPreparedFrame()`, and that function has no other call sites. Removing the callee guard keeps high/low FISHING3 exact-flat, preserves PS-EXE `145408` and pack LBAs, and shrinks `jcreborn.elf 724876 -> 724760`. This is a work-reduction/code-size promotion, not a VBlank win. |
| `P5-43` | Done: remove duplicate prepared-frame guard. | The foreground loop checks `fgRuntimeCanPrepareStagedFrame()` before calling `fgRuntimePrepareStagedFrameForPresent()`, and that function has no other call sites. Removing the callee guard keeps high/low FISHING3 exact-flat, preserves prepared-ready/used counts, preserves PS-EXE `145408` and pack LBAs, and shrinks `jcreborn.elf 724760 -> 724756`. This is a small work-reduction promotion, not a VBlank win. |
| `P5-44` | Done: macro-expand held-slack helper. | `fgRuntimeHeldSlackBeforeWait()` is called throughout the held-frame scheduler and was still emitted as an 84-byte callable helper. Macro expansion keeps high/low FISHING3 exact-flat, preserves PS-EXE `145408` and pack LBAs, and shrinks `jcreborn.elf 724756 -> 724544`. This is a work-reduction/code-size promotion, not a VBlank win. |
| `P5-45` | Done: macro-expand payload predicate helper. | `fgEntryHasPayload()` is small but used throughout frame selection, window containment, and load paths. Macro expansion keeps high/low FISHING3 exact-flat, preserves PS-EXE `145408` and pack LBAs, and shrinks `jcreborn.elf 724544 -> 724132`. This is a work-reduction/code-size promotion, not a VBlank win. |
| `P5-46` | Done: prune duplicate prefetch-window capability check. | `fgRuntimeTryPrefetchWindow()` checked window-cache capability before calling `fgRuntimeEntryFitsWindow()`, which checks the same capability. Removing the outer duplicate keeps high/low FISHING3 exact-flat, preserves PS-EXE `145408` and pack LBAs, and shrinks `jcreborn.elf 724132 -> 724096`. This is a work-reduction/code-size promotion, not a VBlank win. |
| `P5-47` | Done: prune duplicate prefetch-would-read capability check. | `fgRuntimeWindowPrefetchWouldRead()` had the same outer window-cache capability check before `fgRuntimeEntryFitsWindow()`. Removing that duplicate keeps high/low FISHING3 exact-flat, preserves PS-EXE `145408` and pack LBAs, and shrinks `jcreborn.elf 724096 -> 724044`. This is a work-reduction/code-size promotion, not a VBlank win. |
| `P5-48` | Done: prune duplicate prefetch null-entry checks. | The active prefetch and would-read helpers both checked `entry == NULL` immediately before `fgRuntimeEntryFitsWindow()`, whose payload predicate already handles null. Removing both duplicates keeps high/low FISHING3 exact-flat, preserves PS-EXE `145408` and pack LBAs, and shrinks `jcreborn.elf 724044 -> 724012`. This is a work-reduction/code-size promotion, not a VBlank win. |
| `P5-49` | Done: prune prechecked seed-window guards. | `fgRuntimeSeedWindowFromScratch()` has one call site after stage capability, entry-fit, and a successful direct-stage read are proven. Removing its duplicated entry/window/scratch guard keeps high/low FISHING3 exact-flat, preserves PS-EXE `145408` and pack LBAs, and shrinks `jcreborn.elf 724012 -> 723740`. This is a work-reduction/code-size promotion, not a VBlank win. |
| `P5-50` | Done: inline single-call window capability helper. | `fgRuntimeCanWindowCache()` was only used by `fgRuntimeEntryFitsWindow()`, so keeping a callable helper no longer paid for itself. Inlining the capability predicate keeps high/low FISHING3 exact-flat, preserves PS-EXE `145408` and pack LBAs, and shrinks `jcreborn.elf 723740 -> 723724`. This is a work-reduction/code-size promotion, not a VBlank win. |
| `P5-51` | Done: prune seed-window nonempty guard. | The only `fgRuntimeSeedWindowFromScratch()` call site is after entry-fit, which proves payload size is nonzero. Removing the unreachable `windowEnd <= windowStart` guard keeps high/low FISHING3 exact-flat, preserves PS-EXE `145408` and pack LBAs, and shrinks `jcreborn.elf 723724 -> 723680`. This is a work-reduction/code-size promotion, not a VBlank win. |
| `P5-52` | Done: prune seed-window capacity guards. | The direct-stage buffered read only succeeds when scratch can hold the sector-aligned span, and entry-fit proves the same span fits the stream window. Removing the duplicated capacity guard keeps high/low FISHING3 exact-flat, preserves PS-EXE `145408` and pack LBAs, and shrinks `jcreborn.elf 723680 -> 723620`. This is a work-reduction/code-size promotion, not a VBlank win. |
| `P5-53` | Done: inline single-call seed-window helper. | After the guard pruning, `fgRuntimeSeedWindowFromScratch()` had one direct-stage call site and only copied the already-read scratch span into the stream window. Inlining it keeps high/low FISHING3 exact-flat, preserves PS-EXE `145408` and pack LBAs, and shrinks `jcreborn.elf 723620 -> 723508`. This is a work-reduction/code-size promotion, not a VBlank win. |
| `P5-54` | Done: skip unused next-payload output index. | The prefetch-window helpers requested `fgRuntimeNextPayloadEntry()` to fill a frame index and immediately discarded it. Passing `NULL` removes the useless write/local while keeping high/low FISHING3 exact-flat, preserving PS-EXE `145408` and pack LBAs, and shrinking `jcreborn.elf 723508 -> 723368`. This is a work-reduction/code-size promotion, not a VBlank win. |
| `P5-55` | Done: remove next-payload output parameter. | After `P5-54`, every `fgRuntimeNextPayloadEntry()` caller passed `NULL`, so the helper no longer needed to support frame-index output. Removing that parameter and dead write keeps high/low FISHING3 exact-flat, preserves PS-EXE `145408` and pack LBAs, and shrinks `jcreborn.elf 723368 -> 723308`. This is a work-reduction/code-size promotion, not a VBlank win. |
| `P5-56` | Done: macro-expand staged-frame setter. | `fgRuntimeSetStagedFrame()` was still emitted as a 72-byte helper with five hot-path call sites. Macro expansion removes the calls while keeping high/low FISHING3 exact-flat, preserving PS-EXE `145408` and pack LBAs, and shrinking `jcreborn.elf 723308 -> 723136`. This is a work-reduction/code-size promotion, not a VBlank win. |
| `P5-57` | Done: macro-expand prepare-staged predicate. | `fgRuntimeCanPrepareStagedFrame()` was a two-call-site held-loop predicate still emitted as a 172-byte helper. Macro expansion keeps high/low FISHING3 exact-flat, preserves PS-EXE `145408` and pack LBAs, and shrinks `jcreborn.elf 723136 -> 723012`. This is a work-reduction/code-size promotion, not a VBlank win. |
| `P5-58` | Done: prune duplicate next-payload active guard. | `fgRuntimeNextPayloadEntry()` is only consumed by prefetch probes that immediately validate the returned entry through the broader entry-fit predicate. Removing the duplicate local active guard keeps high/low FISHING3 exact-flat, preserves PS-EXE `145408` and pack LBAs, and shrinks `jcreborn.elf 723012 -> 722988`. This is a work-reduction/code-size promotion, not a VBlank win. |
| `P5-59` | Done: coalesce prefetch no-slack perf gates. | `fgRuntimeTryPrefetchWindow()` had duplicate adjacent `ps1PerfEnabled` branches in the no-slack and slack-ineligible paths. Coalescing each pair keeps high/low FISHING3 exact-flat, preserves PS-EXE `145408` and pack LBAs, shrinks `fgRuntimeTryPrefetchWindow` by `20` bytes, and shrinks `jcreborn.elf 722988 -> 722952`. This is a work-reduction/code-size promotion, not a VBlank win. |
| `P5-60` | Done: prune unreachable refill zero-byte guard. | After entry-fit proves a nonzero payload and `windowStart < packCdFile.size`, the clamped refill size cannot become zero for a valid generated pack. Removing that guard keeps high/low FISHING3 exact-flat, preserves PS-EXE `145408` and pack LBAs, shrinks `fgRuntimeFillWindowForEntry` by `40` bytes, and shrinks `jcreborn.elf 722952 -> 722828`. This is a work-reduction/code-size promotion, not a VBlank win. |
| `P5-61` | Done: prune duplicate window-contains payload guard. | Current window-containment callers already pass known non-empty entries or have just passed `fgRuntimeEntryFitsWindow()`. Removing the duplicate `fgEntryHasPayload()` check keeps high/low FISHING3 exact-flat, preserves PS-EXE `145408` and pack LBAs, and shrinks `jcreborn.elf 722828 -> 722756`. This is a work-reduction/code-size promotion, not a VBlank win. |
| `P5-62` | Done: inline copy-window source selection. | `fgRuntimeCopyEntryFromWindow()` called window-containment and then repeated the setup-segment containment test to choose the source. Inlining source selection in the copy helper keeps high/low FISHING3 exact-flat, preserves PS-EXE `145408` and pack LBAs, and shrinks `jcreborn.elf 722756 -> 722752`. This is a tiny work-reduction/code-size promotion, not a VBlank win. |
| `P5-63` | Done: add FISHING3 high follow-on read group `234..246`. | The per-run read-plan artifact identified the next contiguous high-tide group after `223..234`. Adding `{234,246}` and raising retained group capacity to `13` sectors keeps FISHING3 high `loop_vb=2093` while improving `overrun_vb 139 -> 138`, `blocking_vb 16 -> 15`, `prefetch_overrun_vb 11 -> 10`, and `loop_reads 42 -> 41`; FISHING3 low and FISHING1 high stay exact-flat. This is a small active-loop pressure win and validates the new read-plan-in-the-loop method. |
| `P5-64` | Done: macro-expand prepared-present predicate. | `fgRuntimeCanPresentPreparedOnNextVBlank()` had one held-loop call site and only checked runtime state. Macro expansion keeps FISHING3 high/low and FISHING1 exact-flat, preserves PS-EXE `145408` and pack LBAs, and shrinks `jcreborn.elf 723380 -> 723228`. This is a work-reduction/code-size promotion, not a VBlank win. |
| `P5-65` | Done: inline held-vblank wait helper. | `fgRuntimeWaitHeldVBlank()` wrapped `VSync(0)` plus `eventsWaitTick(0)` and sat on the hottest held-frame wait path. Macro expansion keeps FISHING3 high/low and FISHING1 exact-flat, preserves PS-EXE `145408` and pack LBAs, and shrinks `jcreborn.elf 723228 -> 723024`. This is a work-reduction/code-size promotion, not a VBlank win. |
| `P5-66` | Done: macro-expand foreground tick counter. | `fgReadTickCounter()` only wrapped `VSync(-1)` and was used throughout the foreground timing path. Macro expansion keeps FISHING3 high/low and FISHING1 exact-flat, preserves PS-EXE `145408` and pack LBAs, and shrinks `jcreborn.elf 723024 -> 722940`. This is a work-reduction/code-size promotion, not a VBlank win. |
| `P5-67` | Done: inline frame-rendered marker. | `fgRuntimeMarkFrameRendered()` had two render-completion call sites and only marked scene-pack frames as rendered. Macro expansion keeps FISHING3 high/low and FISHING1 exact-flat, preserves PS-EXE `145408` and pack LBAs, and shrinks `jcreborn.elf 722940 -> 722792`. This is a work-reduction/code-size promotion, not a VBlank win. |
| `P5-68` | Done: inline foreground elapsed-vblank accounting. | `fgElapsedVBlanksSince()` had one caller and carried a dead null-pointer guard. Inlining the scene-clock update in `foregroundPilotRuntimeAdvance()` keeps FISHING3 high/low and FISHING1 exact-flat, preserves PS-EXE `145408` and pack LBAs, keeps `foregroundPilotPlay=9016`, and shrinks `jcreborn.elf 722792 -> 722660`. This is a work-reduction/code-size promotion, not a VBlank win. |
| `P5-69` | Done: prune prechecked compose-entry null guard. | `fgRuntimeComposeEntryToBackground()` is internal and both call sites pass runtime-owned entries/buffers after scene-pack startup or staged prep. Removing the duplicate guard keeps FISHING3 high/low and FISHING1 exact-flat, preserves PS-EXE `145408` and pack LBAs, shrinks `foregroundPilotPlay 9016 -> 8996`, shrinks `foregroundPilotRuntimeCompose 56 -> 44`, and shrinks `jcreborn.elf 722660 -> 722376`. This is a work-reduction/code-size promotion, not a VBlank win. |
| `P5-70` | Done: prune prefetch-window output clear. | `fgRuntimeTryPrefetchWindow()` callers always pass a real elapsed-output pointer and ignore it on false returns. Removing the redundant initial clear keeps FISHING3 high/low and FISHING1 exact-flat, preserves PS-EXE `145408` and pack LBAs, shrinks `fgRuntimeTryPrefetchWindow 392 -> 388`, and shrinks `jcreborn.elf 722376 -> 722364`. This is a tiny work-reduction/code-size promotion, not a VBlank win. |
| `P5-71` | Done: prune stage-next output guards. | `fgRuntimeTryStageNextFrame()` has one active-loop caller with a real elapsed-output pointer. Removing the redundant initial clear and nullable writes keeps FISHING3 high/low and FISHING1 exact-flat, preserves PS-EXE `145408` and pack LBAs, and shrinks `jcreborn.elf 722364 -> 722348`. This is a tiny work-reduction/code-size promotion, not a VBlank win. |
| `P5-72` | Done: prune prepare output guards. | `fgRuntimePrepareStagedFrameForPresent()` has one active-loop caller with a real elapsed-output pointer. Removing the redundant initial clear and nullable final write keeps FISHING3 high/low and FISHING1 exact-flat, preserves PS-EXE `145408` and pack LBAs, and shrinks `jcreborn.elf 722348 -> 722336`. This is a tiny work-reduction/code-size promotion, not a VBlank win. |
| `P5-73` | Done: prune fill-window output clear. | Non-null callers of `fgRuntimeFillWindowForEntry()` initialize their local elapsed value to zero before calling, and NULL callers do not need output normalization. Removing the redundant initial clear keeps FISHING3 high/low and FISHING1 exact-flat, preserves PS-EXE `145408` and pack LBAs, shrinks `fgRuntimeFillWindowForEntry 900 -> 888`, and shrinks `jcreborn.elf 722336 -> 722304`. This is a small work-reduction/code-size promotion, not a VBlank win. |
| `P5-74` | Done: lower dirty-upload band merge gap to zero rows. | The accepted post-pause upload path used a one-row clean-gap merge. Tightening it to zero keeps FISHING1, FISHING3 high, and FISHING3 low exact-flat on loop/blocking/refill, preserves PS-EXE `145408` and pack LBAs, and reduces uploaded bytes/dirty rows across all three gates: FISHING1 `8584960 -> 8533120`, FISHING3 high `16771200 -> 16665600`, and FISHING3 low `11838720 -> 11820800`. Rect count rises (`355 -> 436`, `693 -> 858`, `412 -> 440`), so this is an upload-byte work-reduction checkpoint, not a VBlank win. A cap-6 follow-up gave back part of the high-tide byte win, so keep the cap at `8`. |
| `P5-75` | Done: branch directly on dirty-upload cap state. | `grDrawBackground()` no longer needs a separate `useBands` local after scanning upload bands; `capped` plus `bandCount` is the authoritative decision. Removing the extra flag keeps FISHING1, FISHING3 high, and FISHING3 low exact-flat with identical upload work, preserves PS-EXE `145408` and pack LBAs, shrinks `grDrawBackground` by `56` bytes, and shrinks `jcreborn.elf 722312 -> 722044`. This is a clean upload-path code-size/work-shape promotion, not a VBlank win. |
| `P5-76` | Done: prune the single-dirty-tile upload fallback. | The band path already handles uncapped single-tile uploads, and the generic fallback emits the same whole-tile upload when banding caps. Removing the special `dirtyCount == 1` branch and dead `singleIndex` bookkeeping keeps FISHING1, FISHING3 high, and FISHING3 low exact-flat with identical upload work, preserves PS-EXE `145408` and pack LBAs, shrinks `grDrawBackground` by `160` bytes, and shrinks `jcreborn.elf 722044 -> 721456`. This is an upload-path code-size/work-shape promotion, not a VBlank win. |
| `P5-77` | Done: prune the dirty-upload scan guard. | The upload-band scan already skips every clean tile through `minYs[i] < 0`, so the outer `dirtyCount > 0` wrapper was redundant. Removing it keeps FISHING1, FISHING3 high, and FISHING3 low exact-flat with identical upload work, preserves PS-EXE `145408` and pack LBAs, shrinks `grDrawBackground` by `16` bytes, and shrinks `jcreborn.elf 721456 -> 721400`. This is an upload-path code-size/work-shape promotion, not a VBlank win. |
| `P5-78` | Done: prune the band-upload draw-sync guard. | Entry to the banded upload path already requires `bandCount > 0`, and the loop emits one `LoadImage` per band. Removing the redundant `uploadRects > 0` guard keeps FISHING1, FISHING3 high, and FISHING3 low exact-flat with identical upload work, preserves PS-EXE `145408` and pack LBAs, and shrinks `jcreborn.elf 721400 -> 721396`. This is a tiny upload-path code-size promotion, not a VBlank win. |
| `P5-79` | Failed/no promotion: make the fallback upload branch explicit on `dirtyCount > 0`. | FISHING1 stayed timing/layout/work-flat, but the branch reshuffle grew `grDrawBackground` by `72` bytes and regressed `jcreborn.elf 721396 -> 721680`; source was reverted and only the experiment log was kept. |
| `P5-80` | Done: compute upload tile coordinates from the tile index. | Replacing local `screenX`/`screenY` arrays with direct index arithmetic keeps FISHING1, FISHING3 high, and FISHING3 low exact-flat with identical upload work, preserves PS-EXE `145408` and pack LBAs, shrinks `grDrawBackground` by `8` bytes, and shrinks `jcreborn.elf 721396 -> 721380`. This is a tiny upload-path code-size promotion, not a VBlank win. |
| `P5-81` | Failed/no promotion: replace the local background-tile pointer array with index-selection macros. | FISHING1 stayed timing/layout/work-flat, but repeated ternary tile selection grew `grDrawBackground` by `248` bytes and regressed `jcreborn.elf 721380 -> 722372`; source was reverted and only the experiment log was kept. |
| `P5-82` | Done: scan active read groups directly. | After the active read-group table/count moved into runtime state, the grouped-append helper no longer needs local copies before scanning. Removing them keeps FISHING1, FISHING3 high, and FISHING3 low exact-flat, preserves PS-EXE `145408`, keeps `fgRuntimeFillWindowForEntry=844`, and shrinks `jcreborn.elf 721504 -> 721376`. This is a tiny grouped-read code-size promotion, not a VBlank win. |
| `P5-83` | Failed/no promotion: add FISHING3 high read group `246..258`. | The group saved one active-loop read (`41 -> 40`) but crossed the visible-pressure knee: `loop_vb 2093 -> 2096`, `blocking_vb 15 -> 17`, `prefetch_overrun_vb 10 -> 12`, and `due_misses 0 -> 1`; source was reverted and only the experiment log was kept. |
| `P5-84` | Done: trust generated read-group bounds. | Removing the pack-end clamp from `fgRuntimeGroupedAppendTargetEnd()` keeps FISHING1, FISHING3 high, and FISHING3 low exact-flat, preserves PS-EXE `145408`, shrinks `fgRuntimeFillWindowForEntry 844 -> 820`, and shrinks `jcreborn.elf 721376 -> 721232`. This is deterministic fallback removal and grouped-read code-size cleanup, not a VBlank win. |
| `P5-85` | Done: require initialized stream read size. | Removing the startup fallback from zero read size to stream-window capacity keeps FISHING1, FISHING3 high, and FISHING3 low exact-flat, preserves PS-EXE `145408`, shrinks `foregroundPilotPlay 9020 -> 8996`, and shrinks `jcreborn.elf 721232 -> 721200`. This is deterministic startup cleanup, not a VBlank win. |
| `P5-86` | Done: prune redundant extend-window valid store. | Successful append extension starts from an already valid window, so restating `streamWindowValid=1` is unnecessary. Removing it keeps FISHING1, FISHING3 high, and FISHING3 low exact-flat, preserves PS-EXE `145408`, and shrinks `fgRuntimeFillWindowForEntry 820 -> 816`; aggregate ELF moves `721200 -> 721228`, so this is hot-helper work-shape cleanup, not a binary-size win. |
| `P5-87` | Failed/no promotion: skip non-prefetch elapsed probes conditionally. | Avoiding the wrapper tick on non-prefetch/no-output reads was runtime-flat but grew `fgRuntimeFillWindowForEntry 816 -> 928` and aggregate ELF `721228 -> 721656`; source was reverted and only the experiment log was kept. |
| `P5-88` | Done: prune redundant extend-window capacity guards. | `targetEnd <= currentEnd` and the final capacity check already cover `appendBytes == 0` and an overlarge preserved prefix. Removing the extra guards keeps FISHING1, FISHING3 high, and FISHING3 low exact-flat, preserves PS-EXE `145408`, and shrinks `fgRuntimeFillWindowForEntry 816 -> 812`; aggregate ELF moves `721228 -> 721232`, so this is hot-helper cleanup, not a binary-size win. |
| `P5-89` | Failed/no promotion: hoist append alignment before grouped-read lookup. | The shape was runtime-flat but grew `fgRuntimeFillWindowForEntry 812 -> 820` and aggregate ELF `721232 -> 721276`; source was reverted and only the experiment log was kept. Keep the helper-local alignment guard. |
| `P5-90` | Done: fill contiguous windows through the extend path. | Allowing `fgRuntimeTryExtendWindow()` to handle `windowStart == currentEnd` removes the fallthrough to the normal fill path for contiguous aligned reads. FISHING1, FISHING3 high, and FISHING3 low stay exact-flat, PS-EXE stays `145408`, `fgRuntimeFillWindowForEntry` shrinks `812 -> 788`, and aggregate ELF shrinks `721232 -> 721144`. This is a hot-helper/size cleanup, not a VBlank win. |
| `P5-91` | Failed/no promotion: remove the grouped-read zero-count guard. | The run was timing-flat and aggregate ELF shrank, but `fgRuntimeFillWindowForEntry 788 -> 792` and scenes without generated groups would pay extra helper work. Source was reverted and only the experiment log was kept. |
| `P5-92` | Done: add host-side retry manifest and read-cost profile tooling. | `scripts/ps1-perf-retry-manifest.py` mines the durable experiment log, classifies failed/no-promotion rows into the grouped priority ladder, extracts near-miss metric signals, and attaches current read-cost profiles/read-plan candidates from existing summaries. The first run analyzed `322` failed/rejected rows, found `98` conditional retries and `67` near misses, and confirmed the current read-cost split: FISHING1 is CD-clean, FISHING3 high still has `25` visible-pressure VBlanks with one due miss, and FISHING3 low has `6` visible-pressure VBlanks. This is host-side evidence tooling only; it does not change the PS1 runtime or speed binary. |
| `P5-93` | Done: scope the retry manifest for all-scene prioritization. | The manifest now tags candidates as `global-runtime`, `generated-all-scene`, `scene-family-canary`, `one-off-scene`, or `unknown` and sorts global/generalizable work ahead of one-off probes. The refreshed canary profile includes FISHING1, FISHING2 high/low, and FISHING3 high/low: only FISHING1 is currently `12.17%` over target, while the other measured FISHING canaries are `6.63%` to `7.54%` over. This keeps FISHING3 useful as a stress canary without making hand-authored FISHING3 sector fixes the main strategy. |
| `P5-94` | Failed/no promotion: FISHING2 high setup segment `185..191`. | The read-plan's top FISHING2 high single-read candidate did not promote through either scratch-owned or persistent-owned setup-segment ownership. Scratch ownership stayed timing-flat without reducing `loop_reads`; persistent ownership regressed `loop_vb 1898 -> 1900` and `blocking_vb/prefetch_overrun_vb 2 -> 4`. The next CD/setup attempt should be generated multi-segment policy with explicit scheduler ownership/cost proof, not another hand-authored single side segment. |
| `P5-95` | Done: remove stale FISHING1 read group. | The old FISHING1 high group `{396,406}` was beyond the current FGP3 pack sector range and could not fire. Removing the table and startup branch keeps FISHING1, FISHING2 high/low, and FISHING3 high/low exact-flat, preserves PS-EXE `145408` and pack LBAs, and shrinks `foregroundPilotPlay` by `44` bytes. This is generated-policy cleanup, not a VBlank win. |
| `P5-96` | Done: wire the all-scene fgpilot performance matrix. | `scripts/ps1-foreground-scene-manifest.py` now derives the `63` scene slugs and `126` tide variants from `config/ps1/regtest-scenes.txt`, emits the full `FG/` CD layout from generated FG2 packs, prints perf-runner cases, and writes `docs/ps1/performance-scene-matrix.csv`. `scripts/ps1-perf-all-scenes.sh` runs that generated case set through the headless perf harness and refreshes the sheet from collected summaries. Runtime routing now constructs pack paths for every generated scene family while `kProvenScenes` still controls only the human-validated default random loop. |
| `P5-97` | Done: resolve FG CD aliases in the read-plan hook. | The all-scene CD layout uses short 8.3 CD aliases for several families, such as `FG\\ACTV1.FG2`, while the generated source pack is `ACTIVITY1.FG2`. `ps1-perf-iterate.sh` now falls back through `scripts/ps1-foreground-scene-manifest.py` when a CD basename is not present in `generated/ps1/foreground`, so every routed scene can emit foreground read-plan artifacts. ACTIVITY1 high now reports uncovered read candidates instead of silently skipping the plan, while timing remains unchanged at `loop_vb=4383`, `target_vb=4241`, and `blocking_vb=4`. |
| `P5-98` | Failed/no promotion: blind `128 KB` general setup-prime. | Applying a default `128 KB` setup-prime window to non-specialized scenes reduced ACTIVITY1 high loop reads but made visible pressure worse: `loop_vb 4383 -> 4384`, `blocking_vb 4 -> 5`, and `prefetch_overrun_vb 4 -> 6`. Source was reverted. Do not retry blind all-scene setup-prime sizes; future non-FISHING setup work needs generated per-scene coverage/cost metadata and full `scene_vb` accounting. |
| `P5-99` | Done: resume pending all-scene matrix rows safely. | `ps1-perf-all-scenes.sh --only-pending` now skips rows already marked `measured` in `docs/ps1/performance-scene-matrix.csv`, and the sheet importer ignores failed-gate summaries so rejected dirty experiments cannot overwrite accepted measurements. The first resumed run added ACTIVITY4 high and ACTIVITY5 high; ACTIVITY5 high is now the top measured bottleneck at `16.46%` over target with `158` blocking VBlanks and `25` due misses. |
| `P5-100` | Failed/no promotion: ACTIVITY5 scalar stream-window sweep. | Raw `20 KB`, `22 KB`, `24 KB`, and `32 KB` stream-window probes show ACTIVITY5 high is window-coverage sensitive but not promotable through a scalar size. `22 KB` improved `loop_vb 1953 -> 1921` but regressed `blocking_vb 158 -> 167`; `24 KB` improved `loop_vb 1953 -> 1946` and `blocking_vb 158 -> 127` but regressed `prefetch_overrun_vb 47 -> 75`; `32 KB` reduced due misses to `6` but regressed loop and overrun. Next attempt should combine generated group/segment coverage with the `22/24 KB` signal rather than changing the global window. |
| `P5-101` | Failed/no promotion: ACTIVITY5 read-plan range as append group. | A manual ACTIVITY5 high group for the read-plan range `465..477` left every key metric exactly flat and `group_hits=0`, then source was reverted. The host planner must model runtime append starts and preserved-window capacity before emitting group tables; observed hot sector ranges alone are not sufficient. |
| `P5-102` | Failed/no promotion: ACTIVITY5 grouped fill/direct-stage bypass. | Extending the same `465..477` ACTIVITY5 group to fresh window fills, then bypassing the 3-VBlank direct-stage shortcut for entries whose aligned start matched a group, still left ACTIVITY5 high exact-flat: `loop_vb=1953`, `blocking_vb=158`, `prefetch_overrun_vb=47`, `due_misses=25`, and the CD sector sequence stayed identical. Source was reverted. The next ACTIVITY5 attempt needs to target the blocking due-frame loader or emit scheduler-owned coverage metadata; adding group awareness to the append/window-prefetch hooks is exhausted for this range. |
| `P5-103` | Failed/no promotion: ACTIVITY5 retained group capacity `18` sectors. | Giving ACTIVITY5 high the `465..477` group plus an `18`-sector retained window finally changed read work (`loop_reads 139 -> 137`, `due_misses 25 -> 24`) but moved the cost into visible pressure (`blocking_vb 158 -> 162`, `prefetch_overrun_vb 47 -> 50`, `overrun_vb 276 -> 277`) and triggered post-run invalid-read log spam. Source was reverted. Larger retained windows need a scheduler owner/cost model before promotion; raw retained capacity repeats the scalar-window failure in a more targeted form. |
| `P5-104` | Failed/no promotion: ACTIVITY5 narrow group `465..471`. | Keeping the default `13`-sector group capacity and narrowing the generated ACTIVITY5 range to `465..471` saved one loop read but made the active loop worse: `loop_vb 1953 -> 1958`, `blocking_vb 158 -> 170`, `prefetch_overrun_vb 47 -> 50`, and `due_misses 25 -> 26`. Source was reverted. This closes manual group-boundary trimming for the first ACTIVITY5 hot cluster; the scheduler must decide when grouped reads are safe, not just how wide they are. |
| `P5-105` | Done: expand the all-scene matrix to ACTIVITY6/7 high. | The pending-row runner added ACTIVITY6 high (`1054/910`, `15.82%` over, `6` blocking VBlanks) and ACTIVITY7 high (`762/599`, `27.21%` over, `26` blocking VBlanks, zero due misses). ACTIVITY7 is now the highest measured percentage gap, and its zero-due-miss profile says the next broad win should separate CD misses from non-CD/render/target overhead instead of assuming every high gap is an ACTIVITY5-style prefetch failure. The sheet writer now preserves repo-relative `last_summary` paths when refreshed from absolute output roots. |
| `P5-106` | Failed/no promotion: ACTIVITY7 high setup-prime `384 KB`. | A scene-specific `384 KB` setup-prime for ACTIVITY7 high produced the right local shape (`loop_vb 762 -> 751`, `blocking_vb 26 -> 4`, `prefetch_overrun_vb 27 -> 4`, `loop_reads 52 -> 21`) and avoided the `448 KB` invalid-read cliff, but the added source shifted hot code enough to regress FISHING3 high (`loop_vb 2093 -> 2095`, `blocking_vb 15 -> 19`, `prefetch_overrun_vb 10 -> 14`) and trigger invalid-read log spam. Source was reverted. This is a real idea, but it needs layout-preserving/cold generated policy plumbing before it can be retried. |
| `P5-107` | Failed/no promotion: ACTIVITY7 setup-prime wrapper policy. | Retrying the same `384 KB` ACTIVITY7 high prime through a wrapper that left the old setup-prime body intact still reproduced the cross-scene failure. ACTIVITY7 again improved (`loop_vb 762 -> 751`, `blocking_vb 26 -> 4`, `prefetch_overrun_vb 27 -> 4`) and FISHING1/ACTIVITY5 stayed exact-flat, but FISHING3 high regressed to `loop_vb=2095`, `blocking_vb=19`, `prefetch_overrun_vb=14`, then emitted invalid-read log spam. Source was reverted. A wrapper is not layout-preserving enough; the next retry needs generated policy data that does not perturb hot foreground layout, or a scheduler/pack-format change that makes the phase shift irrelevant. |
| `P5-108` | Failed/no promotion: stage-next zero-slack perf-gate coalesce. | Coalescing two adjacent `ps1PerfEnabled` checks in the zero-slack stage-next branch looked like a repeat of the earlier accepted prefetch-gate cleanup, but the map preflight showed `foregroundPilotPlay` growing by `8` bytes with no helper shrink. Source was reverted without a headless run because the experiment had no speed hypothesis and failed its size/work-shape acceptance precondition. |
| `P5-109` | Failed/no promotion: setup-prime next-entry null-guard prune. | Removing the setup-time `entry == NULL` guard after the frame-count bounds check shrank aggregate ELF by `112` bytes and kept FISHING1 exact-flat, but the resulting `+4` hot-symbol phase shift reproduced the FISHING3 high failure (`loop_vb=2095`, `blocking_vb=19`, `prefetch_overrun_vb=14`) and invalid-read log spam. Source was reverted. FISHING3 high is now the phase canary for even setup-only cleanup; do not promote setup guard removals unless hot-symbol phase stays fixed or FISHING3 high passes. |
| `P5-110` | Done: add phase-risk and pressure ranking to the retry manifest. | `scripts/ps1-perf-retry-manifest.py` now tags failed/no-promotion rows with phase risk and sorts read-cost profiles by visible pressure, due misses, and overrun. The current manifest analyzed `333` failed/rejected/unknown rows, including `22` `fishing3-high-required`, `5` `invalid-read-log-risk`, and `90` `layout-sensitive` rows. Current read pressure sorts ACTIVITY5 high first (`score=2826`, `visible=205`, `due_misses=25`), then ACTIVITY7 high (`score=693`, `visible=53`), then FISHING3 high (`score=408`, `visible=25`, `due_misses=1`). This is host-side evidence tooling only; speed baseline remains FISHING1 `loop_vb=1207`, `12.17%` over target. |
| `P5-111` | Failed/no promotion: single-parse scene identity startup. | Parsing scene family/tag once and threading that identity into setup-prime/read-group selection kept FISHING1 high exact-flat, but still shifted hot foreground symbols enough to reproduce the FISHING3 high phase failure (`loop_vb=2095`, `blocking_vb=19`, `prefetch_overrun_vb=14`) and log cap. Source was reverted. Generated policy plumbing cannot be added to `foreground_pilot.c` casually; use cold/generated data, a padding harness, or a phase-independent scheduler before retrying this class. |
| `P5-112` | Failed/no promotion: hoist clean-span tile selection. | Moving `grRestoreCleanBgSpanFromRects()` tile selection out of its clean-rect loop kept FISHING1 high exact-flat, but a `+4` graphics/CD-helper address shift still reproduced the FISHING3 high phase failure and log cap. Source was reverted. The phase problem is broader than foreground source shape; add a hot-symbol/padding harness before more micro-optimizations in any hot PS1 translation unit. |
| `P5-113` | Done: add optional hot-symbol address gate to the perf harness. | `scripts/ps1-perf-iterate.sh` now accepts `--max-symbol-address-delta N` and `PS1_PERF_MAX_SYMBOL_ADDRESS_DELTA`; default behavior is unchanged. Use `--max-symbol-address-delta 0` for source-shape probes so FISHING1 canary runs fail immediately when tracked hot symbols move, before a FISHING3 high run spends time and explodes logs on a known-bad phase. |
| `P5-114` | Done: convert ACTIVITY7 high to padded FGP3. | Converting `ACTIVITY7.FG2` from FGP2 to the existing FGP3 temporal-residual format, then padding it back to its original `645455` bytes, improves ACTIVITY7 high from `loop_vb 762 -> 751`, `overrun_vb 163 -> 139`, `blocking_vb 26 -> 8`, `prefetch_overrun_vb 27 -> 8`, and `loop_reads 52 -> 10` while keeping `FG\\ACTV7.FG2` at LBA `5634`. FISHING1 high stays exact-flat. The current FISHING3 high baseline is stale under the all-FG/executable layout and fails the same way with the original ACTIVITY7 pack restored, so refresh that baseline before using it as a pack-only discriminator. This validates layout-stable FGP3 conversion as the next high-impact host-side path for large FGP2 packs. |
| `P5-115` | Done: convert ACTIVITY5 high to padded FGP3. | Converting `ACTIVITY5.FG2` from FGP2 to padded FGP3 cuts payload `1634015 -> 632152`, improves ACTIVITY5 high `loop_vb 1953 -> 1866`, `overrun_vb 276 -> 159`, `blocking_vb 158 -> 25`, `prefetch_overrun_vb 47 -> 8`, `due_misses 25 -> 4`, and `loop_reads 139 -> 48`, while preserving `FG\\ACTV5.FG2` LBA `4050` and PS-EXE `147456`. FISHING1 high and the newly accepted ACTIVITY7 high stay exact-flat. This confirms padded FGP3 as a high-impact host-preprocessing lane for large FGP2 packs and makes the next best targets ACTIVITY1/4/6 or the worst pending measured FGP2 rows. |
| `P5-116` | Done: convert ACTIVITY6 high to padded FGP3. | Converting `ACTIVITY6.FG2` from FGP2 to padded FGP3 cuts payload `385869 -> 98412`, improves ACTIVITY6 high `loop_vb 1054 -> 1043`, `overrun_vb 144 -> 132`, `blocking_vb 6 -> 0`, `prefetch_overrun_vb 6 -> 0`, and `loop_reads 29 -> 6`, while preserving `FG\\ACTV6.FG2` LBA `5254` and PS-EXE `147456`. FISHING1 high stayed exact-flat. This keeps the pack-format lane as the highest current ROI because it removes CD/render pressure without hot source movement. |
| `P5-117` | Done: convert ACTIVITY4 high to padded FGP3. | Converting `ACTIVITY4.FG2` from FGP2 to padded FGP3 cuts payload `836087 -> 291214`, improves ACTIVITY4 high `loop_vb 1215 -> 1205`, `overrun_vb 150 -> 138`, `blocking_vb 8 -> 5`, `prefetch_overrun_vb 8 -> 5`, and `loop_reads 63 -> 20`, while preserving `FG\\ACTV4.FG2` LBA `3474` and PS-EXE `147456`. FISHING1 high stayed exact-flat. Continue this host-side lane on remaining measured FGP2 rows before returning to source-shape work. |
| `P5-118` | Done: convert ACTIVITY1 high to padded FGP3. | Converting `ACTIVITY1.FG2` from FGP2 to padded FGP3 cuts payload `1683850 -> 685922`, improves ACTIVITY1 high `loop_vb 4383 -> 4373`, `overrun_vb 142 -> 129`, `blocking_vb 4 -> 0`, `prefetch_overrun_vb 4 -> 0`, and `loop_reads 137 -> 50`, while preserving `FG\\ACTV1.FG2` LBA `1822` and PS-EXE `147456`. FISHING1 high stayed exact-flat. This completes the currently measured ACTIVITY high-tide FGP2 conversion set; next work should either measure more pending rows or move to low-tide/other-family FGP2 packs with the same padded-FGP3 gate. |
| `P5-119` | Done: convert ACTIVITY7 low to padded FGP3. | Capturing a fresh ACTIVITY7 low baseline and converting `ACTV7LOW.FG2` to padded FGP3 improves `loop_vb 754 -> 747`, `overrun_vb 139 -> 133`, `blocking_vb 4 -> 1`, `prefetch_overrun_vb 4 -> 1`, and `loop_reads 27 -> 6`, while preserving `FG\\ACTV7LOW.FG2` LBA `5950` and PS-EXE `147456`. FISHING1 high stayed exact-flat. This starts the low-tide half of the ACTIVITY family conversion lane. |
| `P5-120` | Failed/no promotion: ACTIVITY6 low padded FGP3. | ACTIVITY6 low showed the same nominal conversion win as high tide (`loop_vb 1054 -> 1043`, `blocking_vb 5 -> 0`, `loop_reads 29 -> 6`), but the run emitted invalid-read log spam and exited `137`. The FGP3 pack was restored to FGP2. Retry only after the low-tide FGP3 cleanup/post-scene lifetime issue is understood; the FGP2 baseline is now recorded in the matrix. |
| `P5-121` | Done: convert ACTIVITY5 low to padded FGP3. | Capturing a fresh ACTIVITY5 low baseline and converting `ACTV5LOW.FG2` to padded FGP3 improves `loop_vb 1879 -> 1861`, `overrun_vb 183 -> 155`, `blocking_vb 47 -> 29`, `prefetch_overrun_vb 29 -> 16`, `due_misses 4 -> 3`, and `loop_reads 62 -> 36`, while preserving `FG\\ACTV5LOW.FG2` LBA `4851` and PS-EXE `147456`. FISHING1 high stayed exact-flat. Continue low-tide conversions, but keep ACTIVITY6 low blocked until the invalid-read lifetime issue is fixed. |
| `P5-122` | Done: convert ACTIVITY4 low to padded FGP3. | Capturing a fresh ACTIVITY4 low baseline and converting `ACTV4LOW.FG2` to padded FGP3 improves `loop_vb 1213 -> 1203`, `overrun_vb 144 -> 134`, `blocking_vb 2 -> 1`, `prefetch_overrun_vb 2 -> 1`, and `loop_reads 24 -> 13`, while preserving `FG\\ACTV4LOW.FG2` LBA `3885` and PS-EXE `147456`. FISHING1 high stayed exact-flat. Continue with ACTIVITY1 low as the last ACTIVITY low FGP2 candidate not already blocked. |
| `P5-123` | Done: convert ACTIVITY1 low to padded FGP3. | Capturing a fresh ACTIVITY1 low baseline and converting `ACTV1LOW.FG2` to padded FGP3 improves `loop_vb 4382 -> 4373`, `overrun_vb 141 -> 131`, `blocking_vb 3 -> 2`, `prefetch_overrun_vb 3 -> 2`, and `loop_reads 137 -> 50`, while preserving `FG\\ACTV1LOW.FG2` LBA `2648` and PS-EXE `147456`. FISHING1 high stayed exact-flat. ACTIVITY high/low FGP3 conversion is now complete except for the blocked ACTIVITY6 low structural miss. |
| `P5-124` | Failed/no promotion: ACTIVITY9 high padded FGP3. | ACTIVITY9 high is the biggest measured opportunity: FGP2 baseline `loop_vb=11819`, `blocking_vb=7301`, `due_misses=260`; padded FGP3 would improve the per-case summary to `loop_vb=2267`, `blocking_vb=92`, `due_misses=7`, and payload `29732775 -> 1746749`. It still emitted invalid-read log spam and exited `137`, so the pack was restored to FGP2. This confirms the next big unlock is fixing FGP3 post-scene/lifetime cleanup for structurally noisy scenes, not another simple conversion. |
| `P5-125` | Done: promote ACTIVITY9 high padded FGP3 via loop validation. | Re-testing the same padded FGP3 pack on the real screensaver loop completed three ACTIVITY9 scene ends with zero correctness failures and no invalid-write spam. The accepted pack keeps `FG\\ACTV9.FG2` at LBA `6532` and improves the measured row from FGP2 `loop_vb=11819`, `blocking_vb=7301`, `due_misses=260` to loop-validated FGP3 `loop_vb=2268`, `blocking_vb=97`, `due_misses=9`, with payload `29732775 -> 1746749`. FISHING1 high stayed exact-flat. Until the one-shot shutdown artifact is fixed, validate ACTIVITY9 FGP3 with `loop`, not default `noloop`. |
| `P5-126` | Done: promote ACTIVITY6 low padded FGP3 via loop validation. | Re-testing the saved ACTV6LOW FGP3 pack on the real screensaver loop passed against the prior FGP2 baseline and cross-gated FISHING1 exact-flat. The accepted pack keeps `FG\\ACTV6LOW.FG2` at LBA `5444` and improves `loop_vb 1054 -> 1043`, `blocking_vb 5 -> 0`, `prefetch_overrun_vb 5 -> 0`, `loop_reads 29 -> 6`, and payload `385869 -> 98412`. This reclassifies the earlier ACTIVITY6 low no-promotion as the same one-shot shutdown artifact class seen on ACTIVITY9. |
| `P5-127` | Done: convert ACTIVITY9 low to padded FGP3. | Capturing a fresh ACTIVITY9 low FGP2 baseline and converting `ACTV9LOW.FG2` to padded FGP3 improves `loop_vb 12101 -> 2271`, `blocking_vb 7444 -> 92`, `due_misses 250 -> 8`, `loop_reads 264 -> 134`, and payload `29424855 -> 1666401`, while preserving `FG\\ACTV9LOW.FG2` LBA `21053` and PS-EXE `147456`. FISHING1 high stayed exact-flat. ACTIVITY9 high/low are now both in the accepted padded-FGP3 lane. |

## Failed Experiment Triage After P5-90

The recent wins changed the shape of the failed backlog. FGP3 residual packs,
setup-prime and setup-segment coverage, delivered-sector host parsing,
work-identity floors, the foreground-pilot `-Os` phase shift, and the grouped
append path mean some old failures are worth re-opening. The useful failures
are the ones that already moved one metric but lacked generated metadata,
offset control, or scheduler ownership. Local threshold tweaks and source-shape
micro-edits remain low-value unless a broader change moves the underlying cost
model first.

| Failed class | Current verdict | Retest condition |
|---|---|---|
| Raw stream-window size sweeps | Do not retry as scalar sizes. | Retry only when pack metadata changes useful coverage per read. |
| Split immediate/lookahead guards | Do not retry as fixed thresholds. | Retry with per-read cost classes and due-frame coverage proof. |
| Staged-copy fallthrough thresholds | Do not retry locally. | Retry after grouped reads make same-iteration lookahead cheaper. |
| Direct-stage read into stream window | Retry as a metadata-driven experiment. | Must preserve the current stream-window tail and not add a visible read. |
| Two-entry direct-stage queue | Retry only if group-fed. | A second isolated exact-payload slot already caused due misses. |
| Manual hard-coded read groups | Retry through generated metadata. | Manual groups often saved reads but missed append-start timing or shifted layout. |
| Fixed `16 KB` payload grouping | Retry selectively. | Use host-planned hot boundaries, not every arbitrary crossing. |
| Setup-prime blind sizes | Retry through generated segmented coverage. | Contiguous probes expose heap/log-cap limits and setup/loop accounting tradeoffs. |
| Setup-settle first-frame prerender | Retry only as full-scene timing work. | Active-loop wins that leave `scene_vb` flat are not real speed wins. |
| Metadata-prefix changes | Retry only offset-stable. | Moving payload starts by bytes or sectors repeatedly damaged active CD phase. |
| Sound/event table prefixing | Retry as sidecar metadata. | Preserve payload offsets unless the group planner intentionally changes layout. |
| Runtime read-size predictors | Retry as host-generated policy. | Runtime heuristics need per-read histograms or emitted group costs. |
| Prepared-present/prepared-buffer release | Retry as a first-class scheduler. | Local thresholds moved work but stole CD cadence or duplicate-prep ballast. |
| Prepared-wait prefetch follow-ups | Retry under explicit ownership. | Further reads need proof that present, prep, and CD are not competing for the same slice. |
| Host-deadline catch-up threshold `4` | Retry only with resident-payload proof. | Setup-prime made one threshold-4 shape safe; global threshold-4 still regressed. |
| Dirty-row pointer/clear micro-edits | Retry as structural dirty-state layout. | Single rewrites are phase-sensitive and often grow code or CD pressure. |
| Upload gap and rect sweeps | Retry with generated upload-ready bands. | Runtime scratch packing and exact splitting traded bytes for too many rects. |
| PAL4 LUT, aligned stores, inline compositor | Retry as generated or assembly code. | Runtime branching and broad compiler optimization consistently hurt cadence. |
| Runtime restore-under-current parsing | Retry as pack-emitted restore metadata. | Hot-path parsing saved bytes but cost more than it saved. |
| CD helper broad `-O3` or `-Os` | Retry only with map/layout gates. | Some TUs became phase-safe after size wins; hot CD/compositor helpers still regressed. |
| Diagnostic compile gates that shifted phase | Retry with cold sections or padding harness. | Size wins are real, but active cadence must be held flat. |
| Inline runtime histograms | Do not put in speed binary. | Use separate diagnostic builds or host post-processing. |
| Pure FG2 LBA padding | Use only as a control. | LBA shifts alone were flat; code/startup phase is the stronger variable. |
| Sequential CD reads without `Setloc` | Do not retry without a new correctness gate. | It looked faster only because visual work collapsed. |
| Removing deterministic guard rails | Do not retry unless generated invariants prove them. | `P5-91` shows aggregate size can improve while hot helper and future-scene cost worsen. |

Immediate retest queue from the failed ledger:

| ID | Candidate | Acceptance bar |
|---|---|---|
| `RT-001` | Build a host-side failed-experiment classifier that tags each log entry with `retry-after`, `do-not-retry`, or `needs-metadata`. | Documentation/tool-only change, no runtime effect, produces a sorted queue from the existing log. |
| `RT-002` | Generate FISHING3 high setup segments from delivered-sector logs instead of hand-coded sectors. | Reduce high-tide `blocking_vb` or `prefetch_overrun_vb` without moving FISHING1 or FISHING3 low. |
| `RT-003` | Retry the rejected FISHING3 high `246..258` read group only with a companion setup segment or group-cost guard. | Saves the read without reintroducing `due_misses`. |
| `RT-004` | Retry direct-stage-to-window for tight `8 KB` reads only when host metadata proves the window tail is preserved. | Loop time improves and `blocking_reads` stays flat. |
| `RT-005` | Retry short-slack refill guard `2` only for generated cheap groups. | Due misses stay zero and visible CD pressure does not rise. |
| `RT-006` | Add an offset-preserving sidecar for pack metadata before any new prefix data. | Startup metadata grows without shifting payload offsets or sector crossings. |
| `RT-007` | Generate segmented setup-prime plans for FISHING2 and FISHING3 before adding more contiguous prime bytes. | Active-loop pressure improves with bounded setup memory and no log-cap failures. |
| `RT-008` | Retry diagnostic/code-pruning failures under a cold-section or padding harness. | Keeps `loop_vb`, pack LBA, and hot-symbol phase stable while shrinking shipped code. |
| `RT-009` | Generate upload-ready or restore-ready bands for one dense FGP3 scene. | Runtime upload/restore bytes fall without rect-count explosion or scratch-copy cost. |
| `RT-010` | Convert one manual successful read group into generated metadata and remove the hard-coded source table. | Same timing, smaller hot source surface, same correctness across FISHING1/FISHING3. |

Host preprocessing and multi-step idea backlog:

| ID | Idea | Why it is newly plausible |
|---|---|---|
| `HP-001` | Mine the experiment log into a condition-indexed retry manifest. | Many failures say exactly what condition would make them worth re-testing. |
| `HP-002` | Replay existing CD logs against a host simulator for stream-window state. | Lets us reject bad group/window ideas before touching PS1 code. |
| `HP-003` | Generate per-scene/tide setup-prime segment plans with heap limits. | Fishing3 proved non-contiguous setup coverage can pay. |
| `HP-004` | Build a greedy read-group planner that minimizes visible CD VBlanks, not read count. | Several failures saved reads while making the expensive read visible. |
| `HP-005` | Add a dynamic-programming pack planner with constraints for setup bytes, group bytes, and due coverage. | The remaining wins are coupled, not single-knob threshold changes. |
| `HP-006` | Preserve payload offsets while adding metadata through a sidecar file or end-of-pack table. | Prefix metadata failures mostly came from phase shifts, not bad metadata. |
| `HP-007` | Classify reads as cheap, normal, or expensive from delivered sectors and elapsed VBlanks. | Short-slack policies need read-specific cost, not a scalar guard. |
| `HP-008` | Emit per-entry next-coverage metadata: next payload offset, next group end, and safe lead VBlanks. | The runtime can decide quickly without scanning or guessing. |
| `HP-009` | Rank every failed "saved one read but regressed pressure" probe by missing companion coverage. | These are the best candidates for multi-step fixes. |
| `HP-010` | Track setup-vs-loop-vs-scene timing as a first-class report for every accepted setup-prime win. | Prevents counting active-loop shifts as true end-to-end speed. |
| `HP-011` | Generate read-group metadata for all routed FGP3 packs. | Removes one-off source tables and lets scene policy be data-driven. |
| `HP-012` | Generate retained-capacity requirements per group. | Capacity misses made some manual groups no-ops or heap regressions. |
| `HP-013` | Duplicate tiny backward-read payloads inside a pack when the byte cost is lower than the seek cost. | Backward seeks are a recurring visible-pressure source. |
| `HP-014` | Reorder FGP3 payloads into playback order for a new pack version while keeping frame table offsets explicit. | Replay order can reduce seeks more than runtime heuristics can. |
| `HP-015` | Add scene-local policy records: stream size, setup segments, group table, direct-stage cap, and catch-up guard. | One global policy is already exhausted. |
| `HP-016` | Generate setup-preload plans for scene transitions, not just scene start. | Setup-prime wins become real only if hidden during inter-scene time. |
| `HP-017` | Use the previous scene's ending hold or fade time to start the next scene's foreground prime. | Screensaver flow gives us hiding windows outside active playback. |
| `HP-018` | Build an all-scene/tide prime budget dashboard sorted by active-loop pressure per setup byte. | Chooses the next scene targets rationally. |
| `HP-019` | Convert successful manual setup segments into generated pack-side segment tables. | Keeps source professional and reduces hot-path special cases. |
| `HP-020` | Emit "group cannot be used if due frame is within N VBlanks" metadata. | Prevents groups from stealing the slice needed for presentation. |
| `HP-021` | Retry exact short-slack refills only for groups tagged cheap by the host model. | Failed scalar slack probes may pay when limited to cheap reads. |
| `HP-022` | Retry direct-stage-to-window only for reads whose aligned sector span extends the current window. | Avoids the earlier tail-replacement regression. |
| `HP-023` | Add a two-entry stage queue fed only by group reads. | The isolated second slot failed because it increased transaction churn. |
| `HP-024` | Split scheduler ownership into present, prep, window-refill, direct-stage, and event-poll budgets. | Prepared-present failures were ownership conflicts, not proof the idea is bad. |
| `HP-025` | Generate per-frame slack maps from timing files plus current render/CD measurements. | Lets us pick the frames that can safely carry more work. |
| `HP-026` | Build a release-profile timing oracle without heavy perf counters, using periodic scene-end summaries only. | Keeps speed binary stable while preserving acceptance evidence. |
| `HP-027` | Move detailed counters to a separate diagnostic ISO profile. | Inline histogram attempts already proved detail in the speed binary is dangerous. |
| `HP-028` | Add a map-delta gate that reports hot-symbol size, address, PS-EXE sectors, and pack LBA together. | Phase-sensitive failures need one combined explanation surface. |
| `HP-029` | Add cold-section isolation for default-off diagnostics and old test modes. | Retires ballast without perturbing the hot executable phase. |
| `HP-030` | Sweep function ordering with a linker-map harness after each large code-size win. | Some old `-Os` failures became safe only after the phase moved. |
| `HP-031` | Continue per-TU `-Os` retries, but require exact cadence and no hot-helper growth. | Several old size failures are now phase-safe, while CD/compositor TUs still are not. |
| `HP-032` | Try helper-scoped hand-written MIPS for PAL4 residual compose. | Compiler `O3` and branchy C variants lost; assembly can avoid runtime decision cost. |
| `HP-033` | Generate PAL4 scene-specialized compositor loops from pack statistics. | Scene-known alignment/length classes can replace hot runtime branches. |
| `HP-034` | Emit same-pair PAL4 command streams for uniform byte pairs. | Avoids runtime detection while exploiting a real pack-side pattern. |
| `HP-035` | Emit direct16 chunks only for dense frames where doubled pack bytes beat runtime CLUT work. | Direct16 should be selective, not a blanket format pivot. |
| `HP-036` | Generate per-tile command streams for FGP3 residuals. | Removes runtime cross-tile splitting and dirty marking branches. |
| `HP-037` | Emit restore-clean bands for vanished pixels. | Replaces runtime restore-under-current parsing with cheap generated metadata. |
| `HP-038` | Emit upload-ready contiguous bands for scenes with many narrow dirty rows. | Avoids scratch packing and rect explosion from runtime X-aware upload. |
| `HP-039` | Generate per-scene upload rect caps from actual max rect pressure. | Keeps FISHING1's cap small without assuming all scenes share the same bound. |
| `HP-040` | Pack dirty-row state into bitsets or byte spans so clear/promote work scales with touched rows. | Dirty-row touched-only cleanup finally produced a real VBlank win. |
| `HP-041` | Replace dirty-row `memset` retries with a custom fixed-size MIPS fill only after bitset packing. | The library-call shape failed; operation count must change first. |
| `HP-042` | Detect translation-only FGP3 frames and encode GPU `MoveImage` plus residual. | Walking scenes show real move candidates even though fishing1 did not. |
| `HP-043` | Generate static-backdrop reuse maps for scenes whose residual never touches large regions. | Reduces restore/upload ownership instead of redoing known clean areas. |
| `HP-044` | Evaluate GPU-sprite foreground compositing for a single sprite-heavy scene as an architectural branch. | It is high-risk, but it is one of the few remaining double-digit possibilities. |
| `HP-045` | Add host-side frame-hash smoke tests for every generated pack-policy change. | Prevents false speedups like the skipped-visual-work `Setloc` probe. |
| `HP-046` | Expand work-identity floors by scene class before broad optimization merges. | FISHING1 floors are not enough for Mary/Suzy/full-screen scenes. |
| `HP-047` | Add "no runtime fallback" audits for pack features after generation coverage reaches all scenes. | Deterministic generated data can replace expensive safety branches. |
| `HP-048` | Generate a pack-size and read-pressure Pareto chart for all 126 tide variants. | Shows where FGP3, direct16, grouping, or motion compensation is worth the bytes. |
| `HP-049` | Create a host replay for "what if this failed experiment ran on today's baseline" using metrics deltas. | Lets old failures be re-ranked without re-running every branch. |
| `HP-050` | Keep a "near miss" list separate from "dead end" experiments. | Ideas that saved reads or bytes but regressed pressure are useful after scheduler changes. |
| `HP-051` | Re-test old no-promotion size cleanups only after a phase-control harness exists. | Size wins matter, but not at the cost of one visible CD VBlank. |
| `HP-052` | Model pack payload sector crossings and byte offsets before changing any header/prefix layout. | Byte-level shifts repeatedly changed playback cadence. |
| `HP-053` | Try a generated metadata footer rather than prefix for FG2/FGP3 extensions. | Footer data can add tables without disturbing existing payload offsets. |
| `HP-054` | Add a host "segment plus group" combiner for FISHING3 high. | The next high-tide win likely needs a setup segment and a safe read group together. |
| `HP-055` | Build a per-scene hold-distribution report to find scenes where long-hold catch-up can actually fire. | Fishing1 exhausted some catch-up buckets; other scenes may expose them. |
| `HP-056` | Try scene-specific direct-stage caps generated from payload-size histograms. | The global `8 KB` knee may not be the knee for other scenes. |
| `HP-057` | Add an optional async-CD sandbox profile with first-class CD-state ownership counters. | Async is still plausible, but only outside the accepted speed binary first. |
| `HP-058` | Use DuckStation logs to detect emulator-side read clustering and physical-sector delivery quirks. | Delivered-sector parsing already found better evidence than inferred next reads. |
| `HP-059` | Generate blog/archeology annotations for interesting failures as the retry manifest is built. | Keeps the journey searchable without polluting the active optimization queue. |
| `HP-060` | After each accepted multi-step win, auto-promote newly eligible failed classes into the next queue. | The foreground `-Os` and holiday/resource retries proved eligibility changes over time. |

Grouped priority queue from this triage:

| Group | Priority | Included IDs | Execution rule | Expected payoff |
|---|---|---|---|---|
| Evidence and ranking foundation | 1 | `RT-001`, `HP-001`, `HP-002`, `HP-007`, `HP-009`, `HP-028`, `HP-049`, `HP-050`, `HP-058`, `HP-060` | Do first. These are host-side or documentation/tooling changes and should not perturb the PS1 speed binary. | Produces a ranked retry manifest, read-cost classes, and map/layout evidence so the next code experiments are not guesses. |
| Offset-stable metadata enablers | 2 | `RT-006`, `HP-006`, `HP-008`, `HP-011`, `HP-012`, `HP-015`, `HP-020`, `HP-052`, `HP-053` | Do before adding more runtime read heuristics. Metadata must be sidecar/footer or otherwise payload-offset stable. | Unlocks generated groups, setup segments, safe direct-stage windows, and scene-local policy without repeating prefix-phase failures. |
| Generated CD/setup pressure wins | 3 | `RT-002`, `RT-003`, `RT-007`, `RT-010`, `HP-003`, `HP-004`, `HP-005`, `HP-018`, `HP-019`, `HP-054` | Start with FISHING3 high because it still has visible pressure and several near-miss groups/segments. Promote only if FISHING1 and FISHING3 low stay clean. | Best near-term VBlank reduction path; turns one-off successful groups and setup segments into generated policy. |
| Scheduler/read-policy retries | 4 | `RT-004`, `RT-005`, `HP-021`, `HP-022`, `HP-023`, `HP-024`, `HP-025`, `HP-055`, `HP-056`, `HP-057` | Do after metadata/read-cost work. Threshold-only retries are still banned until the runtime can prove read cost and payload coverage. | Reopens old direct-stage, short-slack refill, two-entry queue, async, and prepared-present failures under a real ownership model. |
| Scene-transition and setup hiding | 5 | `HP-010`, `HP-016`, `HP-017`, `HP-026` | Run beside generated setup-prime work, but judge by `scene_vb`, not only active `loop_vb`. | Converts existing active-loop setup-prime wins into real end-to-end screensaver speed by hiding setup between scenes. |
| Pack-time graphics preprocessing | 6 | `RT-009`, `HP-032`, `HP-033`, `HP-034`, `HP-035`, `HP-036`, `HP-037`, `HP-038`, `HP-039`, `HP-040`, `HP-041`, `HP-042`, `HP-043` | Do after the CD/setup queue unless a non-fishing canary shows graphics dominates. Prefer generated data or assembly over runtime branches. | Moves restore, compose, upload, and dirty-state work off the PS1 hot path without repeating runtime parser/scratch-packing failures. |
| Layout, binary size, and diagnostic cleanup | 7 | `RT-008`, `HP-027`, `HP-029`, `HP-030`, `HP-031`, `HP-051` | Run when a phase-control harness exists or when a current baseline makes a prior failure phase-safe. Require exact cadence if it is not a speed experiment. | Recovers binary size and public-readiness cleanup without paying the recurring one-visible-VBlank phase tax. |
| Validation and expansion guardrails | 8 | `HP-045`, `HP-046`, `HP-047`, `HP-048`, `HP-059` | Run continuously around promotions, especially when expanding beyond FISHING scenes. | Prevents false wins, keeps no-fallback cleanup honest, and preserves archeology/blog traceability. |
| Architectural branch | 9 | `HP-013`, `HP-014`, `HP-044` | Branch separately after generated metadata and validation are stable. Treat as high-risk, high-upside work, not a quick loop test. | Possible larger gains through payload duplication/reorder or GPU-sprite foreground compositing if incremental wins plateau. |

All-scene targeting rule: prefer fixes whose mechanism applies to every routed
FGP3 scene, even if FISHING3 high is the first canary. One-off scene tables,
manual sector ranges, and hard-coded tide policies are acceptable only as
temporary probes or as generated-data outputs. A promotion should either improve
the shared runtime, improve the shared pack format/generator, or add
machine-generated per-scene policy that can be emitted for all `126` tide
variants. Do not spend the main optimization loop on hand-authored FISHING3
patches that cannot generalize.

Current measured canary gaps are not uniformly `12%`: FISHING1 high/low are
`12.17%` over target, FISHING2 high/low are `7.54%` and `7.41%`, and FISHING3
high/low are `7.06%` and `6.63%`. The all-scene sheet now exists at
`docs/ps1/performance-scene-matrix.csv`; rows not yet measured remain
`pending`. All-scene speed claims must come from that generated matrix rather
than extrapolation from FISHING canaries.

Near-term execution order:

| Order | Work item | Why now |
|---|---|---|
| 1 | Build the host retry manifest and read-cost simulator. | It is the cheapest way to stop repeating failed scalar probes and identify which near misses are now eligible. |
| 2 | Add offset-stable metadata sidecar/footer support for generated policies. | Most valuable retries need metadata, and prefix changes have repeatedly broken CD phase. |
| 3 | Generate FISHING3 high segment-plus-group candidates from delivered-sector logs. | FISHING3 high still has measurable visible pressure, and existing manual probes already found useful regions. |
| 4 | Promote one generated read/segment policy only if FISHING1, FISHING3 high, and FISHING3 low all pass. | This proves the grouped methodology before broadening to every scene. |
| 5 | Retry direct-stage-to-window and short-slack refill only with generated cost/coverage tags. | These old failures had useful signals but need the metadata from the previous steps. |
| 6 | Start scene-transition preload/hiding once generated setup segments are reliable. | This is how setup-prime wins become real `scene_vb` wins instead of accounting shifts. |

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
| `P7-08` | Run a toolchain flag matrix under exact layout gates. | Test `-Os`, per-file `-O2/-O3`, function alignment, section ordering, and code-address padding as first-class experiments. |
| `P7-09` | Separate hot FG2/CD code from cold menu/debug code by translation unit or section. | Prevent valid cold-code cleanup from perturbing hot scheduler/code phase. |

Detailed current experiment queue: [performance-next-100.md](performance-next-100.md).
It uses the accepted fishing1 exact baseline as of 2026-04-26 and expands the
next pass across harness, CD grouping, scheduler, graphics/upload, compositor,
and toolchain/layout tests.

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
| 11 | CD | Done: test stream-window sizes before and after the `3` VBlank slack guard. | Initial sweep made `32 KB` the first clean default; post-slack sweep promotes `24 KB` with `loop_vb=1322`, while `20 KB` and `28 KB` fail. |
| 12 | CD | Test 40 KB stream window. | Check whether the knee is between 32 KB and the original 48 KB. |
| 13 | CD | Test 56 KB stream window. | Better hit rate without extra overrun. |
| 14 | CD | Test 80 KB stream window for fishing3 only. | Determine memory/perf knee. |
| 15 | CD | Align stream window start to pack entry sector. | Lower `unaligned_start` and `overread_bytes`. |
| 16 | CD | Align stream window end to sector boundary only once. | Lower scratch-copy churn. |
| 17 | CD | Increase prefetch lead from next entry to next two entries. | Lower `due_misses`. |
| 18 | CD | Done: prefetch on holds with at least `3` VBlanks of slack; `6` VBlanks failed. | `prefetch_overrun_vb 94 -> 67` and `loop_vb 1335 -> 1325` without increasing `blocking_vb`. |
| 18a | CD | Done: retune default stream window to `24 KB` after the slack guard. | `prefetch_overrun_vb 67 -> 58`, `loop_vb 1325 -> 1322`, with `blocking_vb 106 -> 108` inside the gate. |
| 19 | CD | Split prefetch budget by remaining hold slack. | Lower visible `blocking_vb`. |
| 20 | CD | Done: stop duplicate prefetch attempts earlier. | `duplicate 887 -> 0`; timing flat, metrics cleaner. |
| 21 | CD | Cache last resolved FG2 file handle per scene. | Lower setup/loop search cost. |
| 22 | CD | Remove redundant `CdSearchFile` inside active loop. | Lower `setloc` or search logs. |
| 23 | CD | Coalesce adjacent due-frame misses into one direct read. | Lower `reads`. |
| 24 | CD | Read through current window even on partial overlap. | Increase `partial_hits`. |
| 25 | CD | Use one staged-entry buffer plus window pointer handoff. | Lower copies and misses. |
| 26 | CD | Avoid copying window-resident entries into frame buffer. | Lower `scratch_bytes` or CPU cost. |
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
| 46 | Dirty | Replace row-band dirty state with row X extents. | Lower `dirty_rounded_bytes`. |
| 47 | Dirty | Track previous row extents separately. | No stale pixels after motion. |
| 48 | Dirty | Restore previous extents, upload current/previous union. | Lower `restore_bytes`. |
| 49 | Dirty | Round X extents to 8-pixel buckets. | Balance bytes vs rect count. |
| 50 | Dirty | Round X extents to 16-pixel buckets. | Lower `upload_rects`. |
| 51 | Dirty | Merge adjacent rows with similar X extents. | Lower `upload_rects`. |
| 52 | Dirty | Cap rects by deterministic widening, not fallback. | `cap_hits` allowed, `full_fallbacks=0`. |
| 53 | Dirty | Split dense frames into planned wide bands. | Stable worst-case upload time. |
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
| 67 | Compose | Add even-X/even-count fast path. | More 32-bit stores. |
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
| 80 | Present | Upload before wait when safe. | Lower `present_wait_vb`. |
| 81 | Present | Wait only when next frame deadline requires it. | Lower idle VBlanks. |
| 82 | Present | Skip OT clear in pure software FG2 frames. | Lower per-entry CPU. |
| 83 | Present | Keep controller polling in held wait path only. | Avoid input regressions. |
| 84 | Present | Separate frame counter from rendered-entry counter. | Correct pause/input accounting. |
| 85 | Present | Avoid double display updates on empty entries. | Lower `render` or `empty` cost. |
| 86 | Present | Measure `LoadImage` rectangle count vs bytes. | Choose batching strategy. |
| 87 | Present | Try two-phase upload for very wide dirty rows. | Lower worst-case upload spikes. |
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

## Next 50 Targets From Current Timing

Current measured bottlenecks are now narrow enough that the next wins should be
small and cumulative: `157` Detail-tier present-wait VBlanks, `5` visible CD
blocking VBlanks, `5` prefetch-overrun VBlanks, `67` active-loop reads,
`16.28 MB` upload volume, `502` upload rects, and `2.51 MB` restore volume on
fishing1 high tide.

A post-holiday/menu integration probe exposed a new regression target. With the
exact pre-integration no-holiday fishing1 variant forced (`lowtide 0 night 1
holiday 0 raft-stage 4 island-pos -154 54`), the row-dirty restore path
restored and the no-holiday stamp path cached, active playback measured
`loop_vb 1221 -> 1306` (`+85`, about `+7.0%`), `overrun_vb 149 -> 256`,
`blocking_vb 5 -> 19`, and `prefetch.overrun_vb 5 -> 14`. `P4-142` resolved
this by restoring the lost default-off JCPAD/JCSPI diagnostics gate; `pad-diag`
and `pad-debug` still re-enable the deep controller probes, while normal
screensaver playback keeps only the lightweight Start poll.

| Priority | Area | Target | Expected signal |
|---:|---|---|---|
| 0 | Integration/regression | Done: recover the post-holiday/menu no-holiday fast-path cost. | Exact-variant fishing1 returned to `loop_vb=1221`, `blocking_vb=5`, `prefetch.overrun_vb=5`, and `due_misses=0`; keep diagnostics opt-in and Start polling live. |
| 1 | CD/pack | Add pack-emitted FG2 prefetch group metadata with aligned `offset/length` and covered frame range. | Lower `reads`, `blocking_vb`, and `prefetch.overrun_vb` without raising `pack_bytes` materially. |
| 2 | CD/runtime | Teach the stream window to fill from group boundaries instead of raw next-entry sector boundaries. | More `window_hits` per read and fewer backwards seeks. |
| 3 | CD/pack | Generate groups using a max-sector budget derived from observed 3-6 VBlank slack. | Preserve zero `due_misses` while lowering overrun. |
| 4 | CD/runtime | Replace fixed slack constants with a sector-count read-cost predictor. | First runtime byte-threshold attempt failed; retry only with per-read histograms/sector classes or pack-group metadata so the predictor can distinguish coverage value, not just byte count. |
| 5 | CD/runtime | Done: re-sweep direct-stage payload caps at `4 KB`, `6 KB`, `7 KB`, `8 KB`, and `10 KB`. | `8 KB` remains the local knee: `6 KB`, `7 KB`, and `10 KB` are no-ops, while `4 KB` regresses blocking/refill. |
| 6 | CD/runtime | Retry 4-VBlank direct-stage only after grouped prefetch metadata or a two-entry stage queue preserves lookahead. | Avoid repeating the observed `due_misses 0 -> 5` and `blocking_vb 11 -> 39` regression. |
| 7 | CD/runtime | Add a two-entry stage queue with bounded memory. | Convert more tight holds to stage hits without window refill. |
| 8 | CD/runtime | After a direct-stage read, prefetch the following window only if remaining slack is still above predicted cost. | Recover the one lost `window_hit` without overrun. |
| 9 | CD/metrics | Rework per-read slack, sectors, and elapsed histograms as a separate diagnostic build or host-side log post-process. | The inline `JCPERF2 cdhist` attempt regressed default timing even when detail-gated, so speed-baseline builds cannot carry the extra code shape. |
| 10 | CD/pack | Reorder payload chunks inside FG2 to eliminate current `seek_back` points. | Lower `seek_back`, `setloc` cost, and hidden/visible read time. |
| 11 | CD/pack | Duplicate tiny backward-referenced payloads when cheaper than seeking backward. | Lower `seek_back` while keeping pack growth bounded. |
| 12 | CD/runtime | Treat huge payloads as direct-read outliers that do not perturb the stream-window policy. | Reduce missed coverage after large frames. |
| 13 | CD/setup | Prime the first lookahead group during setup when group metadata proves it is cheap and deterministic. | Lower first active-loop refill cost without changing playback timing. |
| 14 | CD/runtime | Add a deterministic scene-class policy table for group/window/direct strategy. | Avoid fishing1 overfit across Mary/Suzy/large-payload scenes. |
| 15 | CD/harness | Run the accepted baseline on fishing1 low tide, fishing2, and fishing3 before each larger CD-policy change. | Catch scene-specific regressions early. |
| 16 | CD/runtime | Revisit async refill only with explicit CD ownership state and completion deadlines. | Lower `blocking_vb` without the prior async regression. |
| 17 | CD/runtime | Validate a safe continuation-read API for sequential sectors with full work-identity gates. | Potentially lower `setloc` without repeating the invalid workload-collapse result. |
| 18 | CD/pack | Align group starts to physical sector clusters used by DuckStation/real drive timing. | Lower average read VBlanks per sector group. |
| 19 | CD/runtime | Cache next group metadata in RAM during stage consume. | Reduce table walking on held-loop hot path. |
| 20 | CD/runtime | Split immediate-next coverage and lookahead coverage into separate deterministic read budgets. | Preserve zero `due_misses` while reducing lookahead overrun. |
| 21 | Upload/pack | Emit per-frame upload bands at pack time instead of scanning dirty rows at runtime. | Lower CPU work and keep `upload_rects` bounded. |
| 22 | Upload/runtime | Maintain dirty-band lists while marking rows, avoiding a 240-row scan per tile during upload. | Lower sub-VBlank CPU and future `upload_vb`. |
| 23 | Upload/runtime | Batch exact-X dirty rows into one bounded scratch arena and one final `DrawSync`. | Lower `upload_bytes` without exploding `upload_rects`. |
| 24 | Upload/runtime | Retry 16-pixel-aligned X-aware strips with a deterministic rect cap. | Lower `upload_bytes` while avoiding the previous per-strip sync failure. |
| 25 | Upload/runtime | Try wide-row partial upload only when the byte savings exceed scratch-copy cost. | Lower upload volume on frames with narrow sprites. |
| 26 | Upload/pack | Store upload-ready contiguous bands for high-cost frames only. | Trade small pack growth for lower runtime copy/upload cost. |
| 27 | Upload/runtime | Merge adjacent left/right tile bands through a 640-wide scratch row only when y-ranges match. | Lower `upload_rects` for cross-tile bands. |
| 28 | Upload/metrics | Add upload byte/rect histograms by frame index. | Identify the frames causing max upload spikes. |
| 29 | Upload/runtime | Tune `GR_MAX_UPLOAD_RECTS` with real `upload_vb`, not just rect count. | Avoid overfitting to bytes or rectangles alone. |
| 30 | Upload/runtime | Skip upload for dirty rows whose composed pixels match the current framebuffer. | Lower false-positive dirty work if comparison cost is bounded. |
| 31 | Restore/pack | Emit restore bands from capture metadata to avoid runtime row restore scanning. | Lower `restore_bytes` and restore CPU overhead. |
| 32 | Restore/runtime | Fuse clean restore and PAL4 compose for rows that are immediately overwritten by an FG2 span. | Lower `restore_bytes` without stale pixels. |
| 33 | Restore/runtime | Track exact previous-frame dirty bands rather than row min/max for restore. | Lower restore work on sparse frames. |
| 34 | Restore/runtime | Add frame-index histogram for `restore_bytes` and max restore frame. | Target the worst restore frames first. |
| 35 | Compose/pack | Split PAL4 spans at 320px tile boundaries during pack generation. | Avoid the slower runtime cross-tile split path. |
| 36 | Compose/pack | Generate tile-row command streams for PAL4 spans. | Reduce runtime branch/setup cost per span. |
| 37 | Compose/pack | Test direct16 payloads on dense scenes only. | Lower compose CPU where pack-size cost is justified. |
| 38 | Compose/runtime | Add indexed8 FG2 direct palette fast path before validating non-fishing scenes. | Avoid future regressions on indexed8 captures. |
| 39 | Compose/runtime | Specialize common PAL4 span length classes with generated helpers. | Lower compose cost without broad inlining regressions. |
| 40 | Compose/metrics | Add span class counters: tile-local, cross-tile, clipped, odd-left, odd-right. | Pick compositor optimizations from measured distribution. |
| 41 | Present/runtime | Done: run Detail-tier baseline after the latest accepted CD win. | `present_wait_vb=157`, `restore_vb=18`, `compose_vb=0`, `upload_vb=0`, and `advance_vb=1`; present serialization is the largest measured remaining bucket. |
| 42 | Present/runtime | Done as a bridge: prepare staged frames only when held slack is at least `4` VBlanks. | `loop_vb 1235 -> 1234` and `blocking_vb 10 -> 8`, but extra `restore_calls`/`compose_calls` mean this is not the final present pipeline. |
| 43 | Present/runtime | Redesign the staged present scheduler with separate render-prep and CD-prefetch slack budgets. | Recover present wait without repeating the `loop_vb 1235 -> 1306` regression or the duplicate-prep cost from the accepted bridge; use `JCPERF2 sched` to prove which slices were owned by present, CD, visual prep, or idle wait. |
| 44 | Present/runtime | Separate upload completion wait from display VSync wait in metrics and code shape. | Identify hidden serialization in `grUpdateDisplay()`. |
| 45 | Present/runtime | Avoid double update work on empty capture entries. | Lower `render`/held overhead on scenes with blank ledger frames. |
| 46 | Setup/runtime | Persist same-raft and non-holiday overlays across scene loops where heap probes prove safe. | Lower setup time without active-loop risk. |
| 47 | Setup/CD | Done: coalesce FG2 header/palette/entry-table startup reads into one metadata-prefix read. | `setup_reads 8 -> 6`, `pack_start_vb 55 -> 42`, `scene_vb 1419 -> 1406`, and active-loop CD pressure `7 -> 6`; next setup work is ISO-order/layout. |
| 48 | Binary/release | Compile-gate unused FG1/TTM/ADS paths in PS1 release builds. | Lower executable size and improve instruction cache locality. |
| 49 | Binary/release | Move hot FG2 compositor and prefetch code into a tuned translation unit. | Test file-specific optimization flags without global risk. |
| 50 | Harness | Auto-promote the accepted run to baseline only after a repeated deterministic pass. | Keep the optimization loop safe as wins get smaller. |

## Red-Team Conclusions

The safest near-term speedup is not a more aggressive timing file. The measured
runtime is still `1.139x` over the captured timing budget for fishing1 after
the latest accepted pass. We need to keep removing or hiding work, not lie
about the source timing.

Direct-stage scratch window seeding is a small but important direction signal:
the remaining CD wins are increasingly about preserving useful coverage across
small reads. It reduced one backward seek and one visible blocking VBlank
without changing total loop time, which supports the next larger group-metadata
work rather than more isolated slack or window-size probes.

Held-slack prepared-present is another small but useful bridge result. It saves
one active-loop VBlank and two visible/refill CD VBlanks, but it does so by
adding speculative RAM restore/compose work. Keep the win, but treat the next
present-pipeline target as "same scheduling benefit with less duplicate prep,"
not as solved present serialization. A simple exact-4 slack guard was tried and
rejected because it reduced prep calls but gave back one VBlank of loop,
blocking, and refill-overrun time. A `4`-or-`5` VBlank cap was also rejected
because it increased prep calls and still regressed the same timing counters,
and a `3` VBlank threshold kept total loop timing flat while regressing
blocking/refill by one VBlank and adding more duplicate prep work, so the
`>=4` guard remains the only accepted prepared-present shape.
Trying to use prepared-frame wait time for stream-window prefetch reduced the
duplicate prep work back to the non-prepared baseline, but it also regressed
loop, blocking, and refill-overrun by one VBlank. That confirms prepared wait
slack is not free CD budget unless the read is cost-predicted or pack-grouped.
A `12 KB` post-prepared-present window probe made the same point from the other
side: refill overrun nearly disappeared, but due-frame coverage collapsed. The
next meaningful CD step should preserve coverage first, then shorten reads.
A constrained two-entry direct-stage queue also failed, which narrows the
target further: the missing primitive is not another isolated frame buffer, it
is a grouped read that keeps a useful covered frame range while avoiding full
raw-window refill cost.
Prepared-current RAM background reuse removed the duplicate restore/compose
work but regressed the same one-VBlank CD shape. Treat the accepted extra prep
work as deliberate pacing until a scheduler can replace that timing explicitly.
An opportunistic post-prefetch leftover-prepare pass was also rejected: even
when a CD read appeared to leave the normal `4` VBlank prepare budget, doing
render prep in that same held slice caused five due misses and many more
backward seeks. The scheduler needs frame-level budgeting that preserves future
coverage, not local leftover-slack accounting.
A naive one-VBlank host-deadline offset was also rejected. It did not change
`loop_vb`; it only lowered `target_vb` by one because the existing
`presentedVBlanks` accumulator absorbed the offset. Present-latency work must
change scheduler state, not only the deadline conversion.
A targeted current dirty-row clear pass was also rejected as unproven. It
matched the accepted timing exactly and slightly changed prepared-work identity,
so dirty-state micro-cleanups should wait for finer CPU counters or a larger
dirty pipeline refactor.
A post-prepared-present window-size sweep rejected `14 KB`, `18 KB`, and
`20 KB` windows. The current `16 KB` window remains the local knee: smaller
windows starve near-term entries and larger windows spill too much held slack.
Consuming the leading empty capture artifact during setup was first rejected
because it saved `6` loop VBlanks but raised blocking/refill overrun by one
VBlank. The accepted retry adds a one-VBlank setup settle, dropping active
playback to `loop_vb=1227`, `blocking_vb=7`, and `prefetch.overrun_vb=7` while
keeping `due_misses=0`. The important lesson is that some CD cadence fixes can
be paid before `loop_start` if they preserve the active-loop scheduler shape.
A two-VBlank setup settle retry was rejected immediately afterward: it regressed
the active loop to `loop_vb=1235`, `blocking_vb=13`, and
`prefetch.overrun_vb=13`. Treat the one-VBlank settle as the current local knee,
not as evidence that more startup spacing is broadly useful.
A narrow post-cadence window sweep also left the default unchanged: `15 KB`
tied the accepted `16 KB` timing exactly, while `17 KB` reintroduced due misses
and jumped to `blocking_vb=29`. Raw byte-count window tuning is exhausted again.
Prepared-current RAM reuse was re-tested on the new baseline and still failed.
It cut duplicate restore/compose calls from `187` to `155`, but worsened active
playback to `loop_vb=1231` and `blocking_vb=12`; an immediate next-payload
prefetch refinement did not change the outcome. This remains a scheduler problem,
not a local reuse toggle.
Widening exact direct-stage reads to `4` VBlanks of slack also failed. It turned
some forward window coverage into exact reads, producing `due_misses=2` and
raising `blocking_vb` to `16`. Keep exact direct staging limited to the minimum
`3` VBlank slack point until pack grouping changes the coverage tradeoff.
The dirty-upload merge-gap sweep found a small safe command-pressure win. Raising
the clean-row merge gap from `2` to `8` rows reduces `LoadImage` rectangles from
`424` to `412`, with flat timing and a negligible upload-byte increase
(`+43520` bytes across the scene). This is a micro-optimization, not a speed
breakthrough, but it trims GPU command overhead without adding a fallback path.
The post-gap8 direct-stage payload-cap sweep did not produce another win:
`6 KB` and `10 KB` were exact no-ops, while `4 KB` regressed `loop_vb` to
`1230` and raised blocking/refill-overrun to `11` VBlanks. Keep the accepted
`8 KB` cap; future CD work needs grouped/layout-aware coverage, not another
single payload-size threshold.
Splitting the scalar refill guard by immediate-vs-lookahead also failed. The
`4` VBlank lookahead-only guard preserved `due_misses=0`, but regressed
`loop_vb` to `1230` and raised both `blocking_vb` and refill overrun to `10`.
That narrows the next CD path: decisions need read-size/sector cost or
pack-emitted groups, not more fixed slack thresholds.
The first runtime read-size predictor was not good enough either. Capping
3-VBlank lookahead reads at `8 KB` made the scene appear two VBlanks faster,
but increased visible CD pressure and shifted scheduler cadence; `12 KB`
regressed outright. The predictor needs more information than byte count:
sector class, seek direction, covered frame range, and per-read elapsed
histograms are the next useful data.
The next dirty-upload merge point did not clear the promotion bar. A `12`-row
clean-gap merge lowers `LoadImage` rectangles from `412` to `399`, but gives
back `89600` extra upload bytes versus the accepted `8`-row baseline and leaves
all VBlank timing flat. Follow-up midpoint probes moved the accepted local knee
to `11` rows: `upload_rects 412 -> 401` for `74240` more bytes versus the
8-row baseline. The next upload win should come from pack-emitted/upload-ready
bands that lower command count without widening DMA volume.
A hot translation-unit `-O3` pass also failed. The resulting code reduced
speculative prepared-frame restore/compose calls by five, but worsened the
actual timing and CD-pressure counters. That makes it a scheduler-shape loss,
not a CPU win. Future compiler work should be narrow enough to keep code layout
predictable: individual compositor helpers, CD copy loops, per-file `-Os`/`-O2`
hybrids, function alignment/ordering, or hand-written assembly, with
map-size/address tracking beside the perf summary.
Reading tight-slack direct-stage sectors straight into the stream-window buffer
also lost. The local copy removal changed the window state enough to add one
active-loop read and two backward seeks, regressing loop time and refill
pressure. The useful follow-up is not "read into a different buffer"; it is a
merge-preserving direct-stage seed that appends to or preserves the existing
window coverage.
A no-slack held-loop wait skip also lost. The skipped wait looked locally
redundant when no prepared frame was available, but it regressed `loop_vb` to
`1231`, raised both CD pressure counters to `10`, and added three speculative
restore/compose calls. Treat the held-wait shape as part of the current
deterministic cadence until an explicit frame-deadline scheduler replaces it.
A merge-preserving direct-stage seed also failed. Preserving adjacent or
overlapping stream-window coverage sounds like the right follow-up to the
direct-window replacement miss, but the added window-shape work produced the
same `loop_vb=1231`, `blocking_vb=10`, extra-prep cadence as the no-slack
experiment. Do not keep pushing direct-stage seed shape without better
per-read evidence.
Proactive stream-window extension while the next future payload is already
resident also failed. It preserved zero due misses, but moved more raw CD work
into scarce held slack and raised `blocking_vb`/`prefetch.overrun_vb` to `13`.
The runtime should not slide/append ahead of need until group metadata or a
read-cost predictor proves the append fits the available slack.

The tight-slack direct-stage pass proves that some previously failed ideas are
worth retrying after the baseline changes. The old direct-stage attempt failed
at 1-2 VBlanks of slack; the accepted variant only fires at the proven
3-VBlank lower bound and only for payloads up to 8 KB, cutting both
`blocking_vb` and `prefetch.overrun_vb` while keeping `due_misses=0`.

The post-setup-prime `4` VBlank refill-guard retest proved the current slack
constant family is locally exhausted. It can reduce `loop_vb` from `1237` to
`1229`, but only by returning `due_misses` and raising `blocking_vb` from `13`
to `19-20`; future CD work should preserve zero due misses through grouped or
physically adjacent reads before tightening the guard again.

The post-leading-empty upload gap sweep moved the local runtime knee upward,
but the post-pause accepted point moved back to a byte-saving 1-row dirty-band
gap. That checkpoint keeps key timing flat while reducing upload bytes and
accepting more rects. The rejected 0-row point shows the narrow-side boundary,
and the rejected 12-row point shows the wide-side boundary; the next larger
upload win should move band metadata to pack generation or emit upload-ready
layouts.

An inline CD-read histogram metrics pass was also rejected. The summary-level
variant and the supposedly safer `perf-detail`-gated variant both regressed the
default `perf-log` speed run to `loop_vb=1231`, `blocking_vb=10`, and
`prefetch.overrun_vb=10`. On this deterministic target, even extra diagnostic
code shape can move the scheduler; future high-granularity CD metrics need a
separate diagnostic binary or host-side post-processing, not baseline code.

Coalescing FG2 startup metadata reads was accepted because it amortized small
CD transactions without changing active-loop rendering logic. It reduces
`setup_reads 8 -> 6`, `pack_start_vb 55 -> 42`, and `scene_vb 1419 -> 1406`,
and it also nudges the remaining active-loop CD pressure down by one VBlank
(`blocking_vb/prefetch.overrun_vb 7 -> 6`). The tradeoff is a small metadata
overread and three additional speculative restore/compose calls, so the next
scheduler change should watch `restore_calls` and `compose_calls` closely.

The post-prime stream-window knee also stayed at sector-rounded `16 KB`. An
`18 KB` parameter probe preserved zero due misses but regressed `loop_vb`,
`blocking_vb`, and `prefetch.overrun_vb`, so larger reads are still too slow
unless a future grouped/pipelined layout makes them cheaper to hide.

The `320 KB` setup-prime pass is the first accepted proof that larger resident
coverage can safely unlock threshold-`4` catch-up, but only when the runtime
knows the prime succeeded. It reduces active-loop overrun from `147` to `140`
and visible CD/refill from `5` to `1`, while increasing setup time. Treat this
as a preload/scheduler direction: future work should generate prime sizes,
segment hot coverage, and move the read into previous-scene/menu time where
possible.
The clean-rect first-upload pass is a smaller proof that setup assumptions still
matter: once the static backdrop has been presented, full-screen first upload is
unnecessary for FG2. Keep future active-region work pack- or clean-rect-driven;
do not reintroduce blind full-screen forced redraws unless a visual regression
proves they are required.

Timing wins are only valid when work identity stays stable. The sequential
`Setloc` skip experiment proved that the current Summary gate can accept a run
where correctness counters are zero but the renderer performed far less work.
Future acceptance must compare baseline-sensitive counters such as
`compose_calls`, `upload_calls`, `restore_calls`, `upload_bytes`, and
`restore_bytes` before promoting any low-level CD or render scheduling change.
The headless harness now enforces a default `75%` minimum for `render`,
`restore_calls`, `compose_calls`, and `upload_calls` when comparing against a
baseline; override only for deliberate pack/render architecture changes.
It also caps `headless-regtest.log` at `536870912` bytes by default via
`PS1_PERF_MAX_LOG_BYTES` / `--max-log-bytes`; set the cap to `0` only for
deliberate log-mining runs.

The recent diagnostic-gating misses expose a second acceptance rule: code-size
cleanup is not safe merely because correctness and work identity remain clean.
If shrinking the executable moves foreground pack LBAs or code phase, the scene
can regress by one visible CD VBlank. Treat default-off diagnostics as
layout/cadence ballast until cold sections, explicit ISO padding, or a
phase-independent CD scheduler makes removal deterministic.

The later hot whole-TU compiler probes add the inverse rule: speed-oriented
compiler expansion is also not safe just because the target file is hot.
`foreground_pilot.c -O3` grew the scheduler and raised visible CD pressure, and
`cdrom_ps1.c -O3` grew the executable by three sectors, moved `FISHING1.FG2`
from LBA `399` to `402`, and regressed `blocking_vb/prefetch.overrun_vb` from
`5` to `13`. Future toolchain work needs function-scoped codegen, hot-symbol
address padding, or a phase sweep before retrying hot translation-unit flags.
A follow-up pure CD phase sweep moved `FISHING1.FG2` to LBAs `400`, `401`,
`402`, `403`, and `407` with exact flat timing, so pack LBA alone is not the
failing variable. The next phase-control target is executable/code/startup
phase, not more standalone ISO padding.
The held-loop prefetch pre-check removal then confirmed that point from the
opposite direction: restoring `FISHING1.FG2` to LBA `399` with a dummy sector
did not prevent the regression when hot foreground symbols moved. Hot-loop
cleanups now require hot-symbol padding or a scheduler that is insensitive to
one-sector executable/code-shape changes.
Adding `2 KB` of inert initialized executable payload was timing-flat, so raw
executable footprint is also not a standalone win. Use padding as an isolation
tool for hot-symbol address sweeps and source-cleanup salvage tests.
Aligning `fgRuntimeFillWindowForEntry()` to 16 and 32 byte boundaries was also
timing-flat, even with downstream hot symbols shifted by up to `+36` bytes.
The phase problem is narrower than "any address move"; it appears tied to
specific shrink/code-shape changes that alter read timing, not harmless
positive alignment shifts.

The first measured target is CD latency. Held-frame no-work created idle
VBlanks, but the runtime currently waits until the next frame is due before it
reads that frame. Prefetching converts those held VBlanks into useful work
without skipping frames or changing art.

The second major target is render pipeline precision and serialization. The
current clean-rect system solved memory stability, but it still restores and
uploads hundreds of KB per rendered entry. After prefetching reduces visible CD
stalls, subphase counters should guide row/X dirty uploads, compositor work,
and present scheduling.

## Next 30 Targets After Source-Shape Plateau

The late `P5-56..P5-62` pass shows that safe foreground source-shape cleanups
are now mostly code-size wins, not VBlank wins. Several return-shape and guard
prune tests became exact no-ops or code-size regressions. The next meaningful
speed work should move information out of the hot runtime and into generated
pack metadata, controlled setup/preload policy, or separate diagnostic builds.

| Priority | Area | Experiment | Acceptance signal |
|---:|---|---|---|
| 1 | Pack/CD | Generate per-scene/tide segmented setup-prime metadata instead of hard-coded FISHING3 ranges. | High-tide `blocking_vb` falls below `16` without PS-EXE/LBA movement. |
| 2 | Pack/CD | Emit candidate segment hit counts from the pack builder for FISHING3 high. | Host-side report identifies ranges with repeated active-loop reads before runtime code changes. |
| 3 | Pack/CD | Generate a no-code JSON read-plan for FISHING3 high from existing summaries and pack offsets. | Predicts which loop reads remain visible and which setup segment would cover them. |
| 4 | Pack/CD | Add a generated cold metadata table consumed through existing setup-prime code. | Same timing or better with no hot source table growth. |
| 5 | CD/runtime | Test one generated high-tide segment using existing low-owned ownership, but behind layout padding if needed. | `blocking_vb` or `prefetch_overrun_vb` improves and process exits cleanly. |
| 6 | CD/runtime | Test high-tide owned segment cleanup only as a first-class ownership patch, not local scratch replacement. | No post-scene invalid-read spam and no log-cap exit. |
| 7 | CD/runtime | Add host-side read-boundary analyzer for `setup_read_vb` vs `loop_read_vb`. | No runtime code; produces ranked next segment candidates. |
| 8 | CD/runtime | Re-score rejected setup-prime sizes using current ELF `722752` layout. | Only retry if predicted sectors avoid known bad `140/144/160 KB` phase. |
| 9 | CD/runtime | Try a smaller high-tide contiguous midpoint below `128 KB` only if the analyzer predicts fewer loop reads. | Loop reads fall without increasing blocking. |
| 10 | Scheduler | Split `cd_reserved` into setup-segment-covered vs not-covered ownership in a diagnostic build. | Explains the three `prep_blocked_cd` cases without baseline code growth. |
| 11 | Scheduler | Prototype a separate diagnostic ISO/build profile for high-detail CD traces. | Baseline speed binary remains unchanged; diagnostics no longer perturb timing. |
| 12 | Scheduler | Add host parser for existing DuckStation CD log lines instead of new runtime counters. | Per-read timing table with no PS1 code change. |
| 13 | Scheduler | Model each held slice as CD/prep/present ownership offline from JCPERF2 summary plus frame table. | Identifies candidate frames for scheduler changes. |
| 14 | Scheduler | Test a CD-first policy only for held slices with no prepared frame and generated segment coverage. | Reduces `blocking_vb` without increasing `prepared_wasted` or due misses. |
| 15 | Scheduler | Test a prepare-first policy only when next window read is setup-covered. | Lowers `prep_blocked_cd` or overrun without more CD pressure. |
| 16 | Pack/render | Generate per-entry dirty/upload cost classes for FISHING3. | Finds frames where render work, not CD, owns lateness. |
| 17 | Pack/render | Emit upload band metadata for FGP3 residual entries. | `upload_rects` or `upload_bytes` falls with stable first-frame correctness. |
| 18 | Pack/render | Emit restore band metadata for FGP3 residual entries. | `restore_bytes` falls without stale pixels. |
| 19 | Pack/render | Host-side compare full-current dirty vs previous/current union for FISHING3. | Quantifies if another row-dirty win exists before runtime changes. |
| 20 | Pack/render | Identify entries with max `upload_bytes` / `restore_bytes` in FISHING3 summaries. | Ranks render-side targets after CD stalls. |
| 21 | Format | Prototype direct16 only for the highest compose-cost FISHING3 frame range in a side pack. | Compose work falls enough to justify pack growth. |
| 22 | Format | Generate per-scene PAL4 palette specialization report. | Finds whether scene-specific compositor constants are worth codegen. |
| 23 | Format | Test FGP3 residual chunk ordering by playback order, not source order. | Lowers seeks or loop reads with identical pixels. |
| 24 | Format | Emit frame-to-sector coverage maps as build artifacts. | Makes future group/segment tests deterministic and searchable. |
| 25 | Toolchain | Build a hot-symbol padding harness for `foregroundPilotPlay` address sweeps. | Separates real source wins from code-phase accidents. |
| 26 | Toolchain | Test function-scoped `-Os` on only `fgRuntimeFillWindowForEntry()` with layout padding. | Promotes only if timing flat and ELF shrinks without CD pressure. |
| 27 | Toolchain | Test function-scoped `-Os` on only `fgRuntimeCopyEntryFromWindow()` with layout padding. | Same flat timing plus shrink requirement. |
| 28 | Cleanup | Stop trying source-only ternary/boolean rewrites in foreground hot path. | Logged no-op pattern stays documented; avoids churn. |
| 29 | Validation | Add a script to list promoted vs rejected experiments since the current baseline. | Faster status cards and no manual counting. |
| 30 | Validation | Add a host-side pack/read candidate report to the perf script output. | Every run suggests next segment/group candidates without changing PS1 code. |

Progress note: `scripts/ps1-foreground-read-plan.py` now covers targets `2`,
`3`, and `7` for ad-hoc analysis. On the accepted FISHING3 baseline it confirms
that high tide still has `43` uncovered post-locate pack-read segments after
setup coverage `[3..73)`, while low tide has `20` and only `3` blocking
VBlanks. The next runtime experiment should be a measured generated
setup/read-segment test, not another source-shape cleanup.

The first analyzer-selected runtime segment, FISHING3 high sectors `223..234`,
is accepted as a work-reduction baseline after visual signoff. It keeps
`loop_vb=2093` and `blocking_vb=16` flat while reducing high-tide
`loop_reads 44 -> 43` and `loop_read_vb 188 -> 185`. Continue stacking
measured groups; do not count this as a visible-speed win until `loop_vb` or
visible CD pressure falls.

A broad FISHING3 high tail group, `338..354`, saved transactions but failed the
clean-exit bar with post-scene invalid-read spam, so broad tail grouping is not
safe in the current runtime. The narrower `345..354` group is accepted: it keeps
high and low timing flat, exits cleanly, and reduces high-tide `loop_reads
43 -> 42` plus `loop_read_vb 185 -> 183`. Future read groups should be narrow
or generated with explicit capacity/lifetime proof.

Three follow-up FISHING3 high read groups, `149..161`, `277..289`, and
`161..174`, all saved a loop transaction but regressed visible blocking/refill.
`scripts/ps1-foreground-read-plan.py` now prints estimated saved reads, read-gap
timing, and a phase-risk hint so tight clusters are visible before runtime
patches. The remaining FISHING3 high gap should now favor generated
setup/preload ownership or scheduler work over more adjacent read merges.

The medium-risk `80..96` group then confirmed that manual FISHING3 high
adjacent grouping is exhausted for now: it saved one transaction but regressed
`loop_vb` and emitted post-scene invalid BIOS-address writes. Do not add more
one-off high-tide read groups until the scheduler can prove ownership or the
pack generator emits safe segment/group metadata.

A function-scoped `O2` probe on `fgRuntimeFillWindowForEntry()` is also
rejected for this baseline. It shrank that symbol but moved nearby hot symbols
and produced exact-flat timing/CD metrics, so future toolchain experiments need
layout padding or a clear size/phase acceptance rule before promotion.
The matching `Os` probe is retired too: it produced byte-for-byte identical
tracked hot symbols and no ELF shrink, confirming the current file-level build
already gives that function its size-optimized shape.
A one-VBlank FISHING3-high setup settle is rejected as well. Even with stable
pack LBA and PS-EXE bucket, it pushed active CD/refill pressure sharply worse,
so startup barriers are not a substitute for read-level ownership metadata.
`scripts/ps1-perf-iterate.sh` now emits foreground read-plan artifacts for each
case. This closes the validation/tooling loop: every accepted or rejected run
captures the pack/read candidate table that should drive the next preload,
grouping, or scheduler experiment.

The first follow-on run driven directly from those emitted artifacts is
accepted. FISHING3 high sectors `234..246`, with retained group capacity raised
from `11` to `13` sectors, keeps `loop_vb=2093` while improving
`overrun_vb 139 -> 138`, `blocking_vb 16 -> 15`,
`prefetch_overrun_vb 11 -> 10`, and `loop_reads 42 -> 41`. FISHING3 low and
FISHING1 high both validate exact-flat. This is still a small active-loop win,
but it proves the per-run read plan can find pressure-reducing groups after the
earlier manual adjacent-group failures.

The immediately adjacent `246..258` group is rejected even though it removes
one more read. It regresses FISHING3 high to `loop_vb=2096`,
`blocking_vb=17`, and `prefetch_overrun_vb=12`. Treat `234..246` as the safe
manual extension boundary for this phase; the next high-impact FISHING3 CD work
should be generated segmented setup/preload metadata or scheduler ownership,
not another hand-written adjacent read group.

A widened persistent side segment for FISHING3 high, `67..97`, is also
rejected. It reduced active-loop reads (`41 -> 38`) but bypassed the normal
stream-window seed pattern and made later active reads worse:
`loop_vb=2102`, `blocking_vb=32`, and `prefetch_overrun_vb=17`. The useful
lesson is structural: setup/preload bytes should feed the main window or carry
explicit scheduler ownership; a long-lived side buffer is not automatically a
win even when it removes CD transactions.

Raising FISHING3 high contiguous setup-prime coverage to `192 KB` is rejected
for the same active-cadence reason. It reduced `loop_reads 41 -> 36`, but
regressed `loop_vb` to `2102`, `blocking_vb` to `29`, and
`prefetch_overrun_vb` to `25`. The current `128 KB` high-prime plus narrow
segments/groups is a real knee; bigger setup reads are not automatically
usable unless a scheduler can prove they land outside active pressure.
The narrower `160 KB` probe confirms that knee: it still regressed
`loop_vb=2100`, `blocking_vb=26`, and `due_misses=3`. Do not continue blind
contiguous high-prime sweeps below a new scheduler/ownership model.

The read-plan analyzer now parses setup-prime constants and FISHING3
setup-segment ranges from `src/foreground_pilot.c` at run time. This matters
for future policy probes: while a source change is in flight, the candidate
table should reflect the source under test instead of the last hard-coded
baseline assumption. Historical artifacts still need reruns after source
reverts.

A stream-window-seeded side segment for FISHING3 high `67..97` is rejected,
but it is less bad than the persistent-side-buffer version: `loop_vb=2097`
instead of `2102`, with `prefetch_overrun_vb` flat. The idea has a usable
shape only if the segment is much smaller; broad setup coverage still creates
blocking pressure later in the active loop.
The smaller seeded `67..80` version is also rejected (`loop_vb=2094`,
`blocking_vb=17`, `prefetch_overrun_vb=13`). This closes the manual expansion
path for the accepted `67..73` high segment. Future wins here need generated
scheduler-owned preload, not a wider hand-authored setup segment.

Macro expansion remains selective. The single-call prepared-present predicate
is accepted as a shrink, but `fgRuntimeCanStageNextFrame()` is rejected:
duplicating its two call sites kept timing flat while growing
`foregroundPilotPlay 8936 -> 9032` and `jcreborn.elf 723228 -> 723308`.

Moving the existing FISHING3 high setup segment from `67..73` to `168..174` is
also rejected. It made the active loop worse (`loop_vb=2098`,
`blocking_vb=21`, `prefetch_overrun_vb=16`), proving the early accepted segment
is more valuable than later single-read candidates when only one segment is
available.

A FISHING3 low read-group probe for `159..171` is rejected. It removed one
loop read but did not move `loop_vb`, `blocking_vb`, or
`prefetch_overrun_vb`, and it grew the hot foreground code. Low-tide groups
need pressure-counter movement before they justify a new table/branch path.

Threading precomputed prefetch candidates through the held-loop scheduler is
also rejected in the current source shape. It avoids a duplicate next-payload
scan but grows `foregroundPilotPlay` and the ELF with no key metric movement.
Retry only with finer CPU counters or a smaller call-site shape.

Inlining the held-vblank wait helper is accepted as a small code-size/work
reduction. It keeps FISHING3 high/low and FISHING1 exact-flat, preserves the
`145408` byte PS-EXE bucket and pack LBAs, and shrinks `jcreborn.elf
723228 -> 723024` even though `foregroundPilotPlay` itself grows by `8` bytes.

Macro-expanding the foreground tick-counter wrapper is accepted under the same
rule. It keeps the same three gates exact-flat, preserves layout, and shrinks
`jcreborn.elf 723024 -> 722940`.

Inlining the frame-rendered marker and foreground elapsed-vblank accounting is
accepted under the same rule. Both keep FISHING3 high/low and FISHING1
exact-flat, preserve the `145408` byte PS-EXE bucket and pack LBAs, and shrink
`jcreborn.elf 722940 -> 722660`.

Pruning the prechecked compose-entry null guard is accepted as a larger
source-shape cleanup: it keeps the same three gates exact-flat, shrinks
`foregroundPilotPlay 9016 -> 8996`, and moves the current code-size cleanup
baseline to `jcreborn.elf=722376` bytes with no VBlank change. Pruning the
prefetch-window output clear is accepted as a follow-on hot-helper cleanup and
moves the current code-size cleanup baseline to `jcreborn.elf=722364` bytes.
Pruning the stage-next elapsed-output guards keeps the same gates exact-flat and
moves the current code-size cleanup baseline to `jcreborn.elf=722348` bytes
with no VBlank change. Pruning the prepare elapsed-output guards keeps the same
gates exact-flat and moves the current code-size cleanup baseline to
`jcreborn.elf=722336` bytes. Pruning the fill-window output clear keeps the
same gates exact-flat, shrinks `fgRuntimeFillWindowForEntry 900 -> 888`, and
moves the current code-size cleanup baseline to `jcreborn.elf=722304` bytes
with no VBlank change.
Lowering the dirty-upload band merge gap from one clean row to zero clean rows
is accepted as a render/upload work-reduction checkpoint: key timing stays
flat, uploaded bytes fall by `51.8 KB` on FISHING1, `105.6 KB` on FISHING3 high,
and `17.9 KB` on FISHING3 low, while `LoadImage` rectangle counts rise. Keep
this as a measured byte-vs-rect tradeoff until a lower-level GPU/DMA counter
proves command count dominates byte volume.
Removing the separate dirty-upload `useBands` flag is accepted immediately
afterward: upload work and timing stay exact-flat across the same three gates,
while `grDrawBackground` shrinks by `56` bytes and the current code-size
cleanup baseline moves to `jcreborn.elf=722044` bytes.
Pruning the single-dirty-tile upload fallback is accepted as the next upload
source-shape cleanup: the same three gates stay exact-flat with identical upload
work, `grDrawBackground` shrinks by another `160` bytes, and the current
code-size cleanup baseline moves to `jcreborn.elf=721456` bytes.
Pruning the outer dirty-upload scan guard is accepted after that: the tile-level
clean checks preserve behavior, the same three gates stay exact-flat, and the
current code-size cleanup baseline moves to `jcreborn.elf=721400` bytes.
Pruning the band-upload `DrawSync(0)` guard is also accepted. The band path
precondition proves at least one upload occurred, so the redundant branch is gone
and the current code-size cleanup baseline moves to `jcreborn.elf=721396` bytes.
The fallback-branch reshuffle is rejected immediately after that: it kept
FISHING1 exact-flat but grew `grDrawBackground` and the ELF, so the older
fallback shape remains the baseline.
Computing tile screen coordinates directly from the tile index is accepted as a
separate measured shape from the earlier rejected static-table probe. It keeps
the same three gates exact-flat and moves the current code-size cleanup baseline
to `jcreborn.elf=721380` bytes.
Replacing the local tile-pointer table with index-selection macros is rejected:
the table costs stack space but produces much smaller generated code in this
compiler shape.
Removing the fallback from `fgRuntimeWindowReadSize()` is accepted as a narrow
hot-helper cleanup. Active FG2 playback initializes `streamWindowReadSize`
before the window-fit/fill helpers run, so the fallback to `streamWindowSize`
is obsolete in the measured path. FISHING1, FISHING3 high, and FISHING3 low
stay exact-flat while `fgRuntimeFillWindowForEntry`,
`fgRuntimeLoadSceneFrame`, `fgRuntimeTryPrefetchWindow`, and
`fgRuntimeWindowPrefetchWouldRead` shrink by `32` bytes total. This is not a
VBlank win and not an aggregate ELF-size win; it is a smaller hot helper shape
with the PS-EXE sector bucket unchanged.
FISHING3 low tide now has its first accepted read group. Adding the generated
candidate `159..171` keeps visible timing flat but reduces active CD work:
`loop_reads 21 -> 20`, `hidden_reads 21 -> 20`, `loop_read_vb 85 -> 81`, and
`setloc 28 -> 27`. FISHING3 high and FISHING1 remain exact-flat. Count this as
a small CD transaction/read-time reduction, not as a VBlank win; the next
low-tide groups should be judged the same way and must avoid turning hidden
read reductions into visible blocking.
The grouped-append helper now caches the active read-group table pointer/count
at scene startup. This keeps the accepted FISHING3 low read reduction and all
validated timing flat while moving table selection out of the append probe:
`fgRuntimeFillWindowForEntry` shrinks `892 -> 844`, offset by setup/play growth
and a larger runtime struct. Treat this as a hot-helper cleanup only; it is not
an aggregate binary-size win.
Narrowing the cached read-group count/index to `uint8` is accepted immediately
afterward. It preserves all timing and read-work counters, keeps the
`fgRuntimeFillWindowForEntry` shrink, and claws back `60` aggregate ELF bytes
from the table-pointer refactor while staying in the same PS-EXE sector bucket.
The following source-shape pass removes the grouped-append local table/count
copies entirely. Scanning the cached runtime table directly keeps FISHING1,
FISHING3 high, and FISHING3 low exact-flat, preserves PS-EXE `145408`, and
moves the aggregate ELF baseline from `721504` to `721376` bytes. This is a
small grouped-read path cleanup, not a VBlank win.
The `246..258` FISHING3 high follow-on group is rejected despite saving one
read because it creates a due miss and raises visible CD pressure. Runtime
grouping should not count a transaction win as progress unless blocking/refill
also stays flat. The next accepted grouped-read cleanup removes the pack-end
clamp from the append helper: group bounds are generated from validated pack
sectors, so clamping them in the PS1 hot path is fallback logic. Removing it
keeps all three gates exact-flat, shrinks `fgRuntimeFillWindowForEntry 844 ->
820`, and moves the aggregate ELF baseline to `721232` bytes.
The startup read-size fallback is removed next. Active foreground playback
already initializes `streamWindowReadSize` when prefetch is enabled, and the
disabled-prefetch path rejects before using it. Removing the fallback keeps the
same three gates exact-flat, shrinks `foregroundPilotPlay 9020 -> 8996`, and
moves the aggregate ELF baseline to `721200` bytes.
The successful append-extension validity store is also removed. The function
only enters with a valid window and only the failure path clears validity, so a
successful append only needs to update the start and byte count. The measured
result is exact-flat timing with `fgRuntimeFillWindowForEntry 820 -> 816`; the
aggregate ELF moves to `721228`, so treat this as a hot-path store removal, not
as a binary-size win.
