---
layout: page
title: Performance work
eyebrow: Reference + log
subtitle: What "performance" means on a 33 MHz machine, what got measured, what got tried, and what stuck.
description: Performance work on the Johnny Castaway PS1 fan port — frame budget, perf instrumentation, the experiment ledger of what was tried and what landed in the runtime.
---

A labor of love by Hunter Davis. This page is the running summary of perf
work on the PS1 port at {{ site.release.tag }}: where the bottleneck is, what
got measured, which experiments stayed in the build, and which got reverted.
The full per-experiment ledger lives in the source tree; the link is at the
bottom. The retrospective on how the matrix moved from the compact baseline
to the current battle card —
[*From 87 to 99.5: the post-validation performance loop*]({{ '/lab/from-87-to-99-5/' | relative_url }})
— is in the Lab. If you paid for this, you were cheated. Open source and free.

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

## The constraint

"Performance" on a PS1 means a different shape of problem than performance
on anything modern.

The [MIPS]({{ '/docs/glossary/#mips' | relative_url }}) R3000A core runs at 33.8688 MHz with no FPU. The GPU is
fixed-function — sprites, primitives, an ordering table, no shaders. Audio
is a separate processor with its own RAM. The CD is a 2x drive: 300 KB/s
sustained, 150 ms cold seek. There is no memory bandwidth budget worth
talking about for a 16-color screensaver port; the bandwidth budget is the
CD's, and it gets spent in seek latency, not transfer time.

The frame budget at 60 Hz is 16.6 ms. *Johnny Castaway* is a 1992 VGA
screensaver — at the source level, foreground content changes roughly four
times per second. The PS1 still has to draw a frame at 60 Hz, but it can
hold the same content frame after frame for many [VBlanks]({{ '/docs/glossary/#vblank' | relative_url }}) at a stretch. The
VBlank cadence is the rendering loop's heartbeat; the *interesting* timing
is which VBlanks have actual work in them and which are held idle.

That asymmetry is what shapes the runtime. A held VBlank is free CPU and
free CD bus. The whole optimization story is about scheduling work — CD
reads, RAM tile composition, dirty-row uploads — into held VBlanks before
the next "real" frame arrives. When that scheduling fails, the active
frame's VBlank gets stretched and `loop_vb` goes up.

The frame budget for a screensaver is more forgiving than a game. Nothing
the user does requires sub-frame latency. But the project's acceptance bar
is pixel-perfect playback against host-captured reference frames, which
means the runtime cannot drop frames or compress timing files to "feel
faster" — it has to render every captured entry on the captured beat.
Slack exists in the held intervals; it does not exist in the entries.

## What was measured

The perf instrumentation lives in
[`src/ps1_perf.c`]({{ site.github_url }}/blob/main/src/ps1_perf.c). It is
gated so it adds zero cost when off.

Three signal sources:

- **TTY printf** at scene-start and [scene-end]({{ '/docs/glossary/#scene-end' | relative_url }}) with structured
  [`JCPERF2`]({{ '/docs/glossary/#jcperf' | relative_url }}) records by
  default. Legacy `JCPERF` output is compile-gated behind
  `PS1_PERF_LEGACY_TRACE=1`. Levels: `OFF`, `SUMMARY`, `DETAIL`, `DEBUG`. Only the
  on-demand records cross the TTY surface; per-frame text is forbidden in
  hot paths because it perturbs timing.
- **`ps1_perf` module counters** for VBlank-level metrics: `loop_vb`,
  `target_vb`, `overrun_vb`, `blocking_vb`, `prefetch_overrun_vb`,
  `due_misses`, `restore_bytes`, `upload_bytes`, `dirty_rows`,
  `upload_rects`, `loop_reads`. Each scene-end record dumps the
  steady-state values for that run.
- **Regtest harness frame timing.** The headless DuckStation in
  [`scripts/run-regtest.sh`]({{ site.github_url }}/blob/main/scripts/run-regtest.sh)
  boots the disc image, captures PNGs, and ingests the TTY records into
  per-run summary JSON files under `scratch/ps1-perf-iterate/<runId>/`.

Every experiment goes through the same gate:
[`scripts/ps1-perf-iterate.sh`]({{ site.github_url }}/blob/main/scripts/ps1-perf-iterate.sh)
runs the case, compares it to a baseline `summary.json`, and either
promotes (if a key metric improved without a material regression in
`loop_vb` / `blocking_vb` / `prefetch_overrun_vb` / scene identity) or
rejects with a recorded failure reason.

The full experiment log is at
[`docs/ps1/performance-experiment-log.md`]({{ site.github_url }}/blob/main/docs/ps1/performance-experiment-log.md).
At the time of writing it contains 600+ experiment rows going back to
2026-04-25. Most of them failed.

The full scene/tide battle card is
[`docs/ps1/performance-scene-matrix.csv`]({{ site.github_url }}/blob/main/docs/ps1/performance-scene-matrix.csv)
and is rendered as the live, sortable, color-coded battle card at
[/perf/]({{ '/perf/' | relative_url }}). It is not the human
scene-promotion ledger at [/scenes/]({{ '/scenes/' | relative_url }});
the two ledgers stay separate on purpose — different bars,
different cadences, different failure modes.

The current compiler-flag sweep is tracked in
[`docs/ps1/performance-o2-audit.md`]({{ site.github_url }}/blob/main/docs/ps1/performance-o2-audit.md)
and its machine-readable
[`performance-o2-audit.csv`]({{ site.github_url }}/blob/main/docs/ps1/performance-o2-audit.csv).
That report is regenerated from `build-ps1/compile_commands.json` and
`build-ps1/jcreborn.map` before each `-O2` probe.

The current pack-time graphics preprocessing target sheet is
[`docs/ps1/performance-preprocess-opportunities.md`]({{ site.github_url }}/blob/main/docs/ps1/performance-preprocess-opportunities.md)
and its machine-readable
[`performance-preprocess-opportunities.csv`]({{ site.github_url }}/blob/main/docs/ps1/performance-preprocess-opportunities.csv).
It ranks today’s FG2/FGP3 packs for selective upload-ready or cleanup metadata
work without changing the runtime baseline.

The per-pack detail analyzer
[`scripts/analyze-fg2-preprocess-plans.py`]({{ site.github_url }}/blob/main/scripts/analyze-fg2-preprocess-plans.py)
now parses both FGP2 and FGP3 temporal-residual payloads. Its VISITOR3 output
splits cap-hit frames from saving-heavy frames, which keeps the next
upload-ready experiment selective instead of a whole-pack conversion. The
current VISITOR3 frame sheet is
[`docs/ps1/performance-preprocess-visitor3-hotspots.csv`]({{ site.github_url }}/blob/main/docs/ps1/performance-preprocess-visitor3-hotspots.csv).
The current default VISITOR3 high-tide selective plan is still too large for a
same-footprint append: it models `5730024` selected upload bytes saved, but the
upload-ready payload plus rect metadata needs `2111224` bytes against only
`970076` bytes of padded zero-tail slack. The analyzer now emits the
same-footprint budgeted target too: `78 / 92` default-selected frames fit in
`968904` payload+rect bytes, leave `1172` bytes of slack, and retain `4232112`
modeled upload bytes saved. The analyzer now also reports whether those x-band
uploads are safe to emit from foreground data alone. For VISITOR3, `0`
selected x-band bytes are fully covered by current opaque draw spans, so a raw
pack-emitted upload payload would have to bake restored background pixels that
are dynamic at runtime. The next probe should use a different generated data
shape, explicit scheduler ownership, compression plus a safe pixel source, or a
deliberate layout-moving experiment.

A tempting VISITOR3 shortcut was rejected: pruning visually no-op FGP3 entries
reduced active payload and high-tide visible blocking, but hidden prefetch
overrun regressed from `0` to `56` high and `17` low. That confirms the next
VISITOR3 route needs explicit scheduler ownership or budgeted upload-ready data,
not isolated entry-count pruning. The safer pack-side empty-hold recast also
found `0` current VISITOR3 high/low entries whose cleanup and draw pixel counts
are both zero, so there is no cadence-preserving no-op payload to erase under
the current FGP3/v4 data.

The current post-`-O2` tooling pass also records compact baseline
fingerprints in every perf summary and classifies foreground read-plan
candidates by observed append-start ownership, current grouped-read capacity,
and visible-CD cost class. That makes stale-baseline comparisons, no-op read
groups, and tight visible-cluster candidates visible before a runtime source
edit.

Those foreground read-plan candidates are now rolled up into
[`docs/ps1/performance-read-candidate-matrix.md`]({{ site.github_url }}/blob/main/docs/ps1/performance-read-candidate-matrix.md)
and its machine-readable
[`performance-read-candidate-matrix.csv`]({{ site.github_url }}/blob/main/docs/ps1/performance-read-candidate-matrix.csv).
The current report has no standalone-safe rows, keeps VISITOR3 in the
scheduler-owned or closed lane, and ranks the remaining under-99 work after
the W1-low `238..344` setup segment, split `344..350` setup edge, and
`{91,107}` first-boundary promotion. Remaining read-timing candidates should not
be promoted as raw hand-authored table ranges without the same kind of
slack/scheduler proof. The BUILDING6 v353 `181..197` / `269..285` probe is
now the concrete counterexample for direct-stage clusters: the source table
crossed the PS-EXE bucket, never produced a `group_hit`, and left active read
counts unchanged, so BUILDING6 needs generated direct-stage ownership or a
pack-side data-shape change rather than another local read-group row.

## Experiments that didn't work

A representative slice of rejected experiments and why each one didn't
stick. The pattern is more useful than any individual line — almost every
"obvious" idea gets discarded because the PS1 runtime has counter-intuitive
cost structure.

- **Larger stream windows.** `40 KB`, `56 KB`, `64 KB`. Larger windows
  reduce CD transaction count but overrun held slack more often. The
  current default is `20 KB` after a long sweep; everything bigger lost.
- **Smaller stream windows.** `12 KB`, `14 KB`, `16 KB`. Smaller windows
  reduce per-refill overrun but starve due frames — `due_misses` rises
  and `blocking_vb` follows. The knee is sharp; one sector size in
  either direction matters.
- **Disabling stage1 isolation.** Booting with `no-stage1` to test
  whether stage-copy overhead was a real cost. The headless harness
  exited 137 before `JCPERF2` could record anything; the test was
  structurally inconclusive. Kept staging on.
- **Partial tail reads when a staged frame straddles the window end.**
  Sounded right on paper. In practice, smaller tail reads multiply CD
  transaction count and `due_misses` rises faster than the byte
  savings help. Rejected.
- **Compose-before-VSync sequencing.** Move the FG2 RAM composition
  before the VBlank wait so CPU work overlaps with previous-frame
  scanout, then upload after VBlank. Reduced `prefetch_overrun_vb` but
  stole held-prefetch time elsewhere; total `loop_vb` regressed by 12.
- **Held-loop no-slack wait skip.** Looked like a clean one-VBlank
  overshoot fix. Regressed loop, blocking, and refill metrics
  simultaneously; the skipped wait was load-bearing.
- **Async stream-window refill.** Naive async polling regressed
  `blocking_vb` badly. The CD subsystem has implicit ownership rules
  the synchronous path was respecting; the async path violated them.
  Rejected without a first-class CD-state ownership model.
- **`-O3` on hot translation units.** Less prepared RAM work in some
  scenes, but worse loop/blocking/refill timing overall. The
  optimization changed code shape enough that CD scheduling phase
  shifted unfavorably. Kept `-O2`.
- **Holiday overlap restamping.** Seed holiday decoration into the
  clean backdrop and only restamp it when the current FG2 frame
  overlaps. Logically sound, but the active fishing1 frames overlap
  the Christmas decoration enough that this didn't reduce dirty work.
  Pure no-op, rejected.
- **`vprintf` inline diagnostics.** Adding a CD-read histogram inline
  with the perf summary path regressed timing even with detail-gating. The act of
  having the code present changed binary shape enough to move
  scheduling phase. Reverted; histograms now live in post-processing.
- **FG2 sound-event table in the metadata prefix.** Setup reads
  improved, but moving the table ahead of the payload shifted every
  payload by 36 bytes and badly worsened active CD phase. The pack
  layout is more sensitive to byte offsets than is comfortable.

The recurring lesson: changes that look like clean wins on paper often
shift CD scheduling phase in ways that are not visible until the full
scene runs. The headless gate is what catches this; experiments that
regress `loop_vb` or `blocking_vb` against a baseline get rejected
even when they "obviously" should have helped.

## Experiments that did

A condensed list of changes that survived and are in the runtime today.
They cluster into a few themes.

**Foreground prefetch and stream window:**

- Stage1 staging buffer for the next FG2 entry, prefetched during
  held VBlanks.
- Stream window default of `20 KB`, reduced from earlier `32 KB` after
  the post-pause-merge sweep showed it as the local minimum.
- 3 VBlank refill guard, raised from earlier 2/1 thresholds after
  smaller guards caused due-frame starvation.
- Forward-extend stream window when a straddling entry is detected:
  preserve the resident suffix and append-read only the missing
  aligned tail. Replaces overlapping full-window refills.
- Stage-copy fallthrough at 5 VBlanks: after a zero-VBlank stage copy
  from the resident window, immediately prefetch the following
  window if at least 5 held VBlanks remain. Converts idle held time
  into hidden CD work.
- Tight-slack direct staging up to `8 KB` for immediate payloads when
  the window refill would otherwise be skipped.

**Compositor:**

- PAL4 opaque-span compositor — FG2 PAL4 spans contain only visible
  pixels, so the per-pixel transparent-index branch was removable.
- Tile-local PAL4 fast path — split each span by destination tile
  once instead of per-pixel.
- Per-tile PAL4 row dirty marking — track which rows of which tiles
  changed, not just which tiles.
- Base-diff FG2 pack format — the active path requires base-diff
  packs, which makes RAM tile compositing the only render path and
  lets `grBeginFrame()` / `ClearOTagR()` skip when nothing's queued.

**Dirty-rect bookkeeping:**

- X-aware clean-rect restore — track dirty X extents per tile so
  RAM clean-background restore only touches the changed region.
- Vertical dirty-row upload bands with an 11-row gap merge —
  collapses adjacent uploads into wider rectangles.
- Long-hold host-deadline catch-up — a small render bookkeeping
  adjustment that traded seven extra speculative restore/compose
  calls for five fewer loop VBlanks.

**Code shape and link:**

- `-ffunction-sections -fdata-sections` plus `--gc-sections` for the
  PS1 link. The legacy ADS / TTM / FG1 / FOC runtime paths are still
  in the source tree but get stripped at link time.
- Removal of the foreground visual telemetry hot-path body, the
  legacy foreground diagnostic gate, the unused foreground "ever"
  diagnostics, the unused ADS foreground start hook, the obsolete
  FGPILOT ADS dispatch, the unused foreground status accessors, the
  dead foreground requested-mode state.

**Diagnostic gating:**

- Pad / SPI diagnostics gated default-off. The pause-menu work
  introduced always-on `JCPAD` / `JCSPI` sampling; a strict-gate
  red-team pass showed the diagnostics were costing 52 VBlanks of
  loop time. Default-off recovered that; `pad-diag` / `pad-debug`
  boot tokens still enable them on demand.

The cumulative effect is visible in the current accepted baseline:
fishing1 high-tide playback at `loop_vb=1068` against a target of
`target_vb=1074`. The original headless perf-loop baseline was
`loop_vb=1426`, so the FISHING 1 canary is down `358` VBlanks
(`25.11%` loop reduction).

## Where it sits at {{ site.release.tag }}

The current accepted fishing1 high-tide run, captured in the perf log:

```text
policy = stage1_window
buf    = 137048
hits   = 155
due_misses = 0
blocking_vb = 2
prefetch.overrun_vb = 2
loop_vb = 1068
overrun_vb = 0
target_vb = 1074
restore_bytes = 251,144
upload_bytes  = 10,646,400
dirty_rows    = 16,635
upload_rects  = 456
trip = 0   fallback = 0   frame_mismatch = 0
sound_late = 0   cd_fail = 0
```

That is **0.0% public over target**, or **[100.0% public target speed]({{ '/docs/glossary/#target-speed' | relative_url }})**. The raw signed
CSV row is `-0.4%` / `100.4%`. Across the 126 timing-bearing battle-card rows,
the public average is **+0.2% over target / 99.8% target speed** (`0.1956%`
exact public over target / `99.8062%` exact public target speed); the raw
signed optimization matrix is `-0.5213%` / `100.5366%`.

The latest WALKSTUF1 allocator-era baseline uses targeted setup segments
instead of the old full-scene resident setup buffers. High keeps relative
sectors `198..244` resident and retargets the second slice from `411..435` to
`286..344`, then adds `{149,165}` and frame92 D4, improving the current row
`1475/1433 -> 1471/1440`, blocking/refill `76/15 -> 56/13`,
reads/read time `55/229 -> 42/205`, and due `15 -> 10`. The current
prepare-first scheduler row moves high to `1472/1441`, keeps overrun/refill
flat at `31/13`, cuts blocking/due to `43/7`, and the same-speed
`{411..423}` replacement lowers loop reads/read time `42/201 -> 41/198`.
Low now replaces the old `197..243` plus `410..434` split with one
retained `238..344` CACHE setup segment after low-only 48 KiB clean-rect
chunking, then adds `{91,107}` as the first post-boundary read group and a
split TRANSIENT `344..350` setup edge, improving the current row
`1479/1435 -> 1470/1445`, blocking/refill `65/18 -> 35/7`,
reads/read time `50/230 -> 31/163`, and due `10 -> 4`; the later
`{378..390}`, `244..350`/`179..185` plus `{113..129}`, and `{355..371}`
passes keep the row at `1470/1446`, improve blocking/refill to `33/5`, and
lower reads/read time to `24/146`. The newest fresh-owner `160..176` pocket
keeps speed and reads flat while lowering W1-low blocking/refill again to
`32/4`. Both W1 rows stay yellow while staying
inside the new CACHE allocator budget.

The latest BUILDING2 high allocator baseline keeps targeted CACHE slices
at relative sectors `3..35` and `202..242`, then replaces the tail read group
with `83..95`, adds `{158..174}`, guarded `271..287`, `315..327`, and
`{185..197}`, and trims entries `92`/`94`/`95` as a same-speed payload
baseline. The current previous-visible cleanup promotion moves the row to
`1343/1311`, overrun `32`, blocking/refill `51/18`, reads/read time `44/196`,
and due `7`; active payload drops `669408 -> 574094`, runtime restore bytes
drop `438988 -> 116648`, and the row avoids the allocator-era clean-rect
failure seen with full setup buffers.

The latest BUILDING2 low allocator baseline adds setup-resident `112..128`
and `226..262` slices, low-only `80 KiB` clean-strip shaping, a slack-5 low
window, and `{141,153}`. It improves active loop/target `1336/1316 -> 1327/1318`,
cuts overrun `20 -> 9`, blocking `48 -> 47`, reads `35 -> 27`, and
due `10 -> 9`, with setup cost paid before the active loop.

The latest BUILDING4 low renderer retune widens dirty-upload band merging to
gap `8`, improving the public row to `2849/2816`, overrun `33`,
blocking/refill `38/31`, read time `222`, and due `1`. The follow-up B4-low
stream-window retune narrows the scene-local low-tide window to `24 KiB`,
moving the row green at `2847/2820`, overrun `27`, blocking/refill `32/27`,
read time `252`, and due `1`.

The recent VISITOR3 high promotion merges the terminal retained setup coverage
into relative sectors `203..262`, keeps frames `56` and `57` raw inside that
paid gap with a `56 KiB` tight-refill cap, widens the clean-relief stream
window to `80 KiB`, and pays the early retained setup edge `40..47`. It
improves the current allocator-era high row from `1096/1041` to `1070/1046`,
overrun `55 -> 24`, blocking `67 -> 35`, due `4 -> 2`, and cuts hidden refill
`5 -> 0`.

The latest allocator-era VISITOR3 checkpoint keeps clean-memory relief enabled,
preserves the stage1 prefetch buffer for both tides, and restores only bounded
stream windows under clean pressure. High uses the accepted `80 KiB` knee plus
terminal read trimming, merged setup coverage `203..262`, the frame139
raw-gap relocation, the frame56/57 tight56 raw-gap pass, the high-only
`64 KiB` clean-strip cap, the 80 KiB clean-relief window retune, and the
early `40..47` retained setup edge at `1070/1046`, while
low now uses a `16 KiB` slack-5 window plus a third retained setup segment
extended to `206..232`, with frame `138` raw relocated into that paid gap,
the later frame135 gap-D4 data-shape pass, the newest `16..32`, `72..88`, and
`88..104` retained read groups, the four-VBlank dual-segment slack-knee
guard, and a one-VBlank low-tide phase offset. The current canonical row is
`1065/1041`, overrun `24`, blocking/read/due `55/28/10`, without hidden
prefetch debt.

## Scene Battle Card

As of 2026-05-14, all 126 scene/tide variants have current headless
perf measurements. The latest updated rows are stamped
`building2-low-trimtails-v739`,
`visitor3-high-tail-pack-v629`,
`visitor5-high-rg30-46-v496`,
`visitor3-low-frame137-primegap-v510`,
`walkstuf1-low-rg78-91-v474`,
`walkstuf1-high-current-v458-refresh`,
`building2-low-rg218-229-slack8-v626`,
`building2-low-delta-v454`,
`visitor5-low-compact-rg23-47-v451`,
`walkstuf1-high-shared-dual-tail-v428`,
`walkstuf1-low-shared-dual-tail-v428`,
`building2-high-rg206-230-cap24-v441`,
`building6-window-slack4-v364`,
`johnny6-compact-fgp3-v354`,
`visitor3-low-tail-pack-only-v338`,
`visitor3-low-f128-resident-seg27-v302`,
`visitor3-high-f131-resident-alias121123-v299`,
`visitor3-low-alias-noop114117-v292`,
`visitor3-high-f140-segment-copy-v291`,
`visitor3-low-noop113-v249`,
`visitor3-low-noop114117-v248`,
`visitor3-high-f127-f130-resident-copy-v238`,
`visitor3-drop-unused-motion-dispatch-v197`,
`activity9-low-compact-fgp3-v174`,
`johnny1-compact-fgp3-v173`,
`walkstuf3-low-compact-fgp3-v171`,
`activity9-high-compact-fgp3-v167`,
`building6-compact-fgp3-v165`,
`walkstuf3-high-compact-fgp3-v163`,
`building2-low-restore-window-slack4-v160`,
`visitor5-high-current-v401`,
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
`walkstuf1-high-primecap144-v089`,
`visitor3-low-readgroup-prune-v088`,
`building4-restore-minus-current-v087`,
`visitor3-restore-minus-current-v086`,
`visitor3-high-readgroup-prune-v084`,
`compact-u16-inline-v083`,
`fgp3v4-drawcompact-all-v082`,
`activity9-dead-readgroup-prune-v082`,
`read-group-selector-single-assign-v082`,
`visitor3-high-group138-162-slack4-v081`,
`walkstuf1-low-primecap160-v081`,
`johnny2-prefetch-relief-v081`,
`activity9-low-fgp3-cleanup-compact-v081`,
`activity9-current-v081-refresh`,
`building4-fgp3-cleanup-compact-window-v081`,
`building2-fgp3-cleanup-compact-v081`,
`visitor3-fgp3-cleanup-compact-v081`,
`mary2-prefetch-relief-v081`,
`mary2-fgp3-padded-v081`,
`johnny2-fgp3-padded-v081`,
`mary5-fgp3-padded-v081`,
`activity11-fgp3-padded-v081`,
`building5-fgp3-padded-v080`,
`walkstuf1-fgp2-setup-prime-v080`,
`visitor3-setup-prime-192k-v080`,
`visitor3-high-group170-186-v080-current`,
`activity9-lowgroup-v072c`,
`activity9-fgp3-v072c`,
`activity9-window-v072c`,
`activity4-fishing4-v072c-prefetch-relief`,
`activity1-v072c-current-refresh`,
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
`visitor3-v072-prefetch-relief`,
`fishing5-v065-current-ledger-overlay`,
`compact-fgp3-v66-final-frame-hold`,
`compact-fgp3-v64-building2-group318-330`,
`compact-fgp3-v63-building2low-prime`, and
`indexed8-row-local-dirty-v1`; other refreshed rows include
`compact-fgp3-v62-fishing3low-group253-265`,
`compact-fgp3-v61-fishing3low-group163-175`,
`compact-fgp3-v60-visitor3high-group230-242`,
`compact-fgp3-v59-visitor3high-group72-84`, `indexed8-tile-local-compose-v1`,
`compact-fgp3-v58-activity9high-window20-table`, `compact-fgp3-v57-policy-table-refactor`, and `compact-fgp3-v49-walkstuf2-auto-prime` through `compact-fgp3-v29-smallprime`, and the full-matrix baseline rows are stamped
`compact-fgp3-v2-fullmatrix`. 63 of 63 scenes have at least one routed
variant, and 63 scenes have both high- and low-tide variants routed. All 126
rows now carry active-loop timing; `suzy1` needs the longer `12000`-frame
matrix budget because its valid scene-end lands after the default `7200`-frame
window. The latest matrix run is `2026-05-13T21:31:34`; per-row freshness and stats version are shown on
the [battle card]({{ '/perf/' | relative_url }}). The values below are
public-capped `over target / target speed (loop_vb/target_vb)`, with `blk`
and `due` called out when nonzero. Faster-than-target rows display
`0.0% / 100.0%`; their raw signed values remain in
`docs/ps1/performance-scene-matrix.csv`.

The complete matrix pass is `compact-fgp3-v2-fullmatrix`; accepted follow-up
rows now use `visitor3-high-tail-pack-v629`,
`visitor5-high-rg30-46-v496`,
`visitor3-low-frame137-primegap-v510`,
`walkstuf1-low-rg78-91-v474`,
`walkstuf1-high-current-v458-refresh`,
`building2-low-trimtails-v739`,
`building2-low-rg218-229-slack8-v626`,
`building2-low-delta-v454`,
`visitor5-low-compact-rg23-47-v451`,
`walkstuf1-high-shared-dual-tail-v428`,
`walkstuf1-low-shared-dual-tail-v428`,
`building2-low-rg238-250-v445`,
`building2-high-rg206-230-cap24-v441`,
`building6-window-slack4-v364`,
`visitor3-high-f131-resident-alias121123-v299`,
`visitor3-low-tail-pack-only-v338`,
`visitor3-low-f128-resident-seg27-v302`,
`visitor3-low-alias-noop114117-v292`,
`visitor3-high-f140-segment-copy-v291`,
`visitor3-low-noop113-v249`,
`visitor3-low-noop114117-v248`,
`visitor3-high-f127-f130-resident-copy-v238`,
`visitor3-drop-unused-motion-dispatch-v197`,
`johnny1-compact-fgp3-v173`,
`walkstuf3-low-compact-fgp3-v171`,
`activity9-high-compact-fgp3-v167`,
`building6-compact-fgp3-v165`,
`walkstuf3-high-compact-fgp3-v163`,
`building2-low-restore-window-slack4-v160`,
`visitor5-high-current-v401`,
`building1-compact-fgp3-noautoprime-v157`,
`mary3-preserve-window-slack8-v149`,
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
`fgp3v4-drawcompact-all-v082`,
`compact-u16-inline-v083`,
`visitor3-fgp3-cleanup-compact-v081`,
`walkstuf1-low-primecap160-v081`,
`johnny2-prefetch-relief-v081`,
`mary2-prefetch-relief-v081`,
`mary2-fgp3-padded-v081`,
`johnny2-fgp3-padded-v081`,
`mary5-fgp3-padded-v081`,
`activity11-fgp3-padded-v081`,
`building5-fgp3-padded-v080`,
`walkstuf1-fgp2-setup-prime-v080`,
`visitor3-setup-prime-192k-v080`,
`visitor3-high-group170-186-v080-current`,
`activity9-lowgroup-v072c`,
`activity9-fgp3-v072c`,
`activity9-window-v072c`,
`johnny6-compact-fgp3-v354`,
`activity4-fishing4-v072c-prefetch-relief`,
`activity1-v072c-current-refresh`,
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
`visitor3-v072-prefetch-relief`,
`compact-fgp3-v66-final-frame-hold`,
`fishing5-v065-current-ledger-overlay`,
`compact-fgp3-v64-building2-group318-330`,
`compact-fgp3-v63-building2low-prime`, and
`indexed8-row-local-dirty-v1`; other refreshed rows include
`compact-fgp3-v62-fishing3low-group253-265`,
`compact-fgp3-v61-fishing3low-group163-175`,
`compact-fgp3-v60-visitor3high-group230-242`,
`compact-fgp3-v59-visitor3high-group72-84`, `indexed8-tile-local-compose-v1`,
`compact-fgp3-v58-activity9high-window20-table`, `compact-fgp3-v57-policy-table-refactor`, and `compact-fgp3-v49-walkstuf2-auto-prime` through `compact-fgp3-v29-smallprime`. Older `padded-fgp3-v1` / `compact-fgp3-v1`
rows are historical only.

<table class="scene-table perf-summary-table">
  <thead>
    <tr>
      <th scope="col">Scene</th>
      <th scope="col">High tide</th>
      <th scope="col">Low tide</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td><code>activity1</code></td>
      <td>0.0% / 100.0% (2754/2764); blk 1</td>
      <td>0.0% / 100.0% (2754/2765)</td>
    </tr>
    <tr>
      <td><code>activity4</code></td>
      <td>0.0% / 100.0% (1065/1066); blk 4</td>
      <td>0.0% / 100.0% (1064/1068); blk 1</td>
    </tr>
    <tr>
      <td><code>activity5</code></td>
      <td>0.0% / 100.0% (1730/1749); blk 2</td>
      <td>0.0% / 100.0% (1731/1749); blk 2</td>
    </tr>
    <tr>
      <td><code>activity6</code></td>
      <td>+0.1% / 99.9% (912/911)</td>
      <td>+0.1% / 99.9% (912/911)</td>
    </tr>
    <tr>
      <td><code>activity7</code></td>
      <td>0.0% / 100.0% (593/596)</td>
      <td>0.0% / 100.0% (594/596)</td>
    </tr>
    <tr>
      <td><code>activity8</code></td>
      <td>0.0% / 100.0% (898/904); blk 1</td>
      <td>0.0% / 100.0% (899/904); blk 2</td>
    </tr>
    <tr>
      <td><code>activity9</code></td>
      <td>+1.0% / 99.0% (2082/2062); due 1; blk 24</td>
      <td>+0.7% / 99.3% (2075/2061); due 1; blk 17</td>
    </tr>
    <tr>
      <td><code>activity10</code></td>
      <td>0.0% / 100.0% (1259/1259); due 1; blk 7</td>
      <td>0.0% / 100.0% (1255/1256); due 2; blk 17</td>
    </tr>
    <tr>
      <td><code>activity11</code></td>
      <td>0.0% / 100.0% (1715/1722); blk 2</td>
      <td>0.0% / 100.0% (1717/1722); blk 4</td>
    </tr>
    <tr>
      <td><code>activity12</code></td>
      <td>0.0% / 100.0% (1411/1412); blk 7</td>
      <td>0.0% / 100.0% (1409/1411); due 1; blk 10</td>
    </tr>
    <tr>
      <td><code>building1</code></td>
      <td>+2.1% / 98.0% (794/778); blk 21</td>
      <td>+1.9% / 98.1% (794/779); blk 21</td>
    </tr>
    <tr>
      <td><code>building2</code></td>
      <td>+2.4% / 97.6% (1343/1311); due 7; blk 51</td>
      <td>+0.7% / 99.3% (1327/1318); due 9; blk 47</td>
    </tr>
    <tr>
      <td><code>building3</code></td>
      <td>0.0% / 100.0% (5460/5465)</td>
      <td>0.0% / 100.0% (5460/5465)</td>
    </tr>
    <tr>
      <td><code>building4</code></td>
      <td>+1.0% / 99.1% (2843/2816); due 1; blk 34</td>
      <td>+1.0% / 99.1% (2847/2820); due 1; blk 32</td>
    </tr>
    <tr>
      <td><code>building5</code></td>
      <td>0.0% / 100.0% (3343/3348); blk 5</td>
      <td>0.0% / 100.0% (3345/3347); blk 8</td>
    </tr>
    <tr>
      <td><code>building6</code></td>
      <td>+1.0% / 99.0% (2482/2457); blk 25</td>
      <td>+1.2% / 98.8% (2485/2456); blk 28</td>
    </tr>
    <tr>
      <td><code>building7</code></td>
      <td>0.0% / 100.0% (3132/3133); blk 9</td>
      <td>0.0% / 100.0% (3130/3133); blk 7</td>
    </tr>
    <tr>
      <td><code>fishing1</code></td>
      <td>0.0% / 100.0% (1068/1074); blk 2</td>
      <td>0.0% / 100.0% (1067/1074); blk 1</td>
    </tr>
    <tr>
      <td><code>fishing2</code></td>
      <td>0.0% / 100.0% (1761/1763); blk 6</td>
      <td>0.0% / 100.0% (1759/1765); blk 3</td>
    </tr>
    <tr>
      <td><code>fishing3</code></td>
      <td>+0.6% / 99.4% (1962/1950); due 1; blk 17</td>
      <td>+0.1% / 99.9% (1957/1955); blk 9</td>
    </tr>
    <tr>
      <td><code>fishing4</code></td>
      <td>0.0% / 100.0% (835/842); blk 2</td>
      <td>0.0% / 100.0% (834/843)</td>
    </tr>
    <tr>
      <td><code>fishing5</code></td>
      <td>0.0% / 100.0% (885/890)</td>
      <td>0.0% / 100.0% (885/890)</td>
    </tr>
    <tr>
      <td><code>fishing6</code></td>
      <td>0.0% / 100.0% (744/753)</td>
      <td>0.0% / 100.0% (744/753)</td>
    </tr>
    <tr>
      <td><code>fishing7</code></td>
      <td>0.0% / 100.0% (715/725)</td>
      <td>0.0% / 100.0% (715/725)</td>
    </tr>
    <tr>
      <td><code>fishing8</code></td>
      <td>0.0% / 100.0% (1243/1253)</td>
      <td>0.0% / 100.0% (1243/1253)</td>
    </tr>
    <tr>
      <td><code>johnny1</code></td>
      <td>+1.4% / 98.6% (1973/1945); blk 25</td>
      <td>+1.4% / 98.6% (1973/1945); blk 25</td>
    </tr>
    <tr>
      <td><code>johnny2</code></td>
      <td>0.0% / 100.0% (1741/1751)</td>
      <td>0.0% / 100.0% (1741/1751)</td>
    </tr>
    <tr>
      <td><code>johnny3</code></td>
      <td>0.0% / 100.0% (1158/1161); due 1; blk 10</td>
      <td>0.0% / 100.0% (1157/1166)</td>
    </tr>
    <tr>
      <td><code>johnny4</code></td>
      <td>0.0% / 100.0% (1204/1214)</td>
      <td>0.0% / 100.0% (1204/1214)</td>
    </tr>
    <tr>
      <td><code>johnny5</code></td>
      <td>0.0% / 100.0% (811/820)</td>
      <td>0.0% / 100.0% (810/820)</td>
    </tr>
    <tr>
      <td><code>johnny6</code></td>
      <td>+1.0% / 99.0% (2829/2802); blk 24</td>
      <td>+1.0% / 99.0% (2830/2802); blk 25</td>
    </tr>
    <tr>
      <td><code>mary1</code></td>
      <td>+0.8% / 99.2% (4867/4830); due 2; blk 47</td>
      <td>+0.4% / 99.6% (4860/4840); due 1; blk 31</td>
    </tr>
    <tr>
      <td><code>mary2</code></td>
      <td>0.0% / 100.0% (2241/2248); blk 2</td>
      <td>0.0% / 100.0% (2242/2250); blk 2</td>
    </tr>
    <tr>
      <td><code>mary3</code></td>
      <td>+0.1% / 99.9% (2296/2294); due 13; blk 53</td>
      <td>+0.1% / 99.9% (2297/2295); due 13; blk 51</td>
    </tr>
    <tr>
      <td><code>mary4</code></td>
      <td>0.0% / 100.0% (1968/2016); due 3; blk 28</td>
      <td>0.0% / 100.0% (1966/2019); due 3; blk 24</td>
    </tr>
    <tr>
      <td><code>mary5</code></td>
      <td>0.0% / 100.0% (1581/1586); due 1; blk 5</td>
      <td>0.0% / 100.0% (1581/1584); due 1; blk 6</td>
    </tr>
    <tr>
      <td><code>miscgag1</code></td>
      <td>0.0% / 100.0% (953/961)</td>
      <td>0.0% / 100.0% (953/961)</td>
    </tr>
    <tr>
      <td><code>miscgag2</code></td>
      <td>0.0% / 100.0% (1352/1356)</td>
      <td>0.0% / 100.0% (1352/1356)</td>
    </tr>
    <tr>
      <td><code>stand1</code></td>
      <td>0.0% / 100.0% (194/202)</td>
      <td>0.0% / 100.0% (194/202)</td>
    </tr>
    <tr>
      <td><code>stand2</code></td>
      <td>0.0% / 100.0% (480/490)</td>
      <td>0.0% / 100.0% (480/490)</td>
    </tr>
    <tr>
      <td><code>stand3</code></td>
      <td>0.0% / 100.0% (547/557)</td>
      <td>0.0% / 100.0% (547/557)</td>
    </tr>
    <tr>
      <td><code>stand4</code></td>
      <td>0.0% / 100.0% (1202/1220)</td>
      <td>0.0% / 100.0% (1203/1218); blk 3</td>
    </tr>
    <tr>
      <td><code>stand5</code></td>
      <td>0.0% / 100.0% (1442/1460)</td>
      <td>0.0% / 100.0% (1442/1460)</td>
    </tr>
    <tr>
      <td><code>stand6</code></td>
      <td>0.0% / 100.0% (1346/1364)</td>
      <td>0.0% / 100.0% (1346/1364)</td>
    </tr>
    <tr>
      <td><code>stand7</code></td>
      <td>0.0% / 100.0% (520/538)</td>
      <td>0.0% / 100.0% (520/538)</td>
    </tr>
    <tr>
      <td><code>stand8</code></td>
      <td>0.0% / 100.0% (483/499); blk 2</td>
      <td>0.0% / 100.0% (483/499); blk 2</td>
    </tr>
    <tr>
      <td><code>stand9</code></td>
      <td>0.0% / 100.0% (520/538)</td>
      <td>0.0% / 100.0% (522/538)</td>
    </tr>
    <tr>
      <td><code>stand10</code></td>
      <td>0.0% / 100.0% (528/538)</td>
      <td>0.0% / 100.0% (528/538)</td>
    </tr>
    <tr>
      <td><code>stand11</code></td>
      <td>0.0% / 100.0% (528/538)</td>
      <td>0.0% / 100.0% (528/538)</td>
    </tr>
    <tr>
      <td><code>stand12</code></td>
      <td>0.0% / 100.0% (1450/1459); blk 1</td>
      <td>0.0% / 100.0% (1450/1460)</td>
    </tr>
    <tr>
      <td><code>stand15</code></td>
      <td>0.0% / 100.0% (444/452)</td>
      <td>0.0% / 100.0% (444/452)</td>
    </tr>
    <tr>
      <td><code>stand16</code></td>
      <td>+0.2% / 99.8% (473/472)</td>
      <td>+0.2% / 99.8% (473/472)</td>
    </tr>
    <tr>
      <td><code>suzy1</code></td>
      <td>no active loop</td>
      <td>no active loop</td>
    </tr>
    <tr>
      <td><code>suzy2</code></td>
      <td>no active loop</td>
      <td>no active loop</td>
    </tr>
    <tr>
      <td><code>visitor1</code></td>
      <td>0.0% / 100.0% (672/677)</td>
      <td>0.0% / 100.0% (672/677)</td>
    </tr>
    <tr>
      <td><code>visitor3</code></td>
      <td>+2.2% / 97.8% (1063/1040); due 6; blk 35</td>
      <td>+2.1% / 97.9% (1062/1040); due 7; blk 42</td>
    </tr>
    <tr>
      <td><code>visitor4</code></td>
      <td>0.0% / 100.0% (424/428)</td>
      <td>0.0% / 100.0% (424/428)</td>
    </tr>
    <tr>
      <td><code>visitor5</code></td>
      <td>+1.1% / 98.9% (1104/1092); blk 11</td>
      <td>+2.0% / 98.0% (1112/1090); blk 12</td>
    </tr>
    <tr>
      <td><code>visitor6</code></td>
      <td>0.0% / 100.0% (2043/2047); blk 1</td>
      <td>0.0% / 100.0% (2043/2047); blk 1</td>
    </tr>
    <tr>
      <td><code>visitor7</code></td>
      <td>0.0% / 100.0% (1619/1625)</td>
      <td>0.0% / 100.0% (1619/1625)</td>
    </tr>
    <tr>
      <td><code>walkstuf1</code></td>
      <td>+2.2% / 97.9% (1472/1441); due 7; blk 43</td>
      <td>+1.7% / 98.3% (1470/1445); due 4; blk 35</td>
    </tr>
    <tr>
      <td><code>walkstuf2</code></td>
      <td>0.0% / 100.0% (451/461)</td>
      <td>0.0% / 100.0% (451/461)</td>
    </tr>
    <tr>
      <td><code>walkstuf3</code></td>
      <td>+0.9% / 99.1% (2310/2290); due 6; blk 47</td>
      <td>+1.2% / 98.8% (2321/2293); due 5; blk 41</td>
    </tr>
  </tbody>
</table>
Detail-tier attribution for the canary currently points at render and
restore pressure rather than CD stalls:

```text
sched.wait       = 722
sched.present    = 99
sched.cd_stage   = 137
sched.cd_window  = 19
gfx.restore_bytes = 251,144
gfx.upload_bytes  = 8,643,840
```

The FISHING1 canary remains at the public `100.0%` cap with raw signed
headroom, but the full battle card still has CD-heavy scenes (`visitor3`,
`building2`, `walkstuf1`, and `building4`). The clean-pressure relief rows
prove scene-local CD policy can recover large due-miss collapses, and the
allocator-era VISITOR3 stage1-only promotion proves the same path can keep a
small prefetch buffer live when full setup-prime/window buffers no longer fit.

Next plausible wins, in priority order:

1. **Generated deadline/refill owner metadata.** The remaining yellow rows are
   BUILDING2 high, VISITOR3 high/low, and WALKSTUF1 high/low. Hand-authored
   read tables now repeatedly save reads while shifting cost into visible
   blocking or hidden refill, so the next CD swing is a generated sidecar that
   owns append-start, frame deadline, and refill budget before any grouped read
   fires. W1-low `160..176` is the first narrow fresh-owner pocket to promote;
   broader neighboring W1-low ranges still need real generated ownership.
2. **VISITOR3 terminal data shape.** VISITOR3 high/low still need a different
   data representation, not another scalar range. The next candidate is a
   pixel-perfect row-reference or setup-dictionary terminal-frame codec after
   simple alignment and early read groups closed.
3. **WALKSTUF1 no-decode pack canonicalization.** W1-high D4 byte wins moved
   work into visible blocking, and W1-low isolated trims are exact-flat. The
   next W1 swing should shrink or canonicalize pack rows without adding runtime
   decode cost or changing sector cadence unless the canary proves it.
4. **BUILDING2 frame/deadline-owned data-shape.** B2-high duplicate aliasing,
   isolated entry trims, broad slack gates, and prefetch-only ownership all
   failed. Future B2 work needs generated per-frame ownership or a selective
   no-decode relocation that preserves the accepted cadence.
5. **Render/restore and source-headroom compounding.** Exact-flat code shrink
   remains promotable when it keeps pack LBAs fixed. The latest dirty upload
   band merge retune keeps the five-yellow canary exact-flat while shrinking
   `grDrawBackground` by `36` bytes, and W1-low `160..176` cuts blocking/refill
   `33/5 -> 32/4` without changing speed, giving future generated-owner and
   data-shape work a cleaner baseline.

The author considers the current build comfortable for the validated scenes,
not yet headroom-clean. The canary bottleneck is no longer raw CD stall; the
matrix bottleneck is uneven per-scene payload/read shape plus render/restore
pressure.

## Non-goals

A few things the perf work explicitly does not chase, with reasons:

- **Frame dropping.** Violates pixel-perfect playback. The acceptance
  bar requires every captured entry to render on its captured beat.
- **Timing compression before throughput work.** The timing-bearing matrix
  public average is now +0.1956% over target / 99.8062% target speed, with three
  remaining CD-/data-shape-bound outliers; compressing the timing files would expose the same
  throughput bottleneck without fixing it.
- **Reintroducing FG1 / ADS / TTM runtime paths.** Those are retired
  from the active public path. The PS1 executable links only the
  scene-playback runtime plus the minimal background / audio / input
  / CD layers it needs.
- **Fixed island assumptions.** The runtime must randomly place the
  island per scene, so all optimizations must preserve scene-relative
  FG2 placement.
- **Direct framebuffer or progressive-mode experiments as first
  moves.** Prior history says these were unstable. Exhaust stable
  scene playback first.

## Related pages

- [Performance battle card]({{ '/perf/' | relative_url }}) — the
  live timing matrix this reference manual describes the columns
  of. 126 scene/tide variants, sortable, color-coded.
- [From 87 to 99.5: the post-validation performance loop]({{ '/lab/from-87-to-99-5/' | relative_url }})
  — the retrospective on the optimization arc, including which
  experiments landed and which got rejected.
- [v0.8.1: what the soak found that the matrix didn't]({{ '/lab/v081-mary4-freeze/' | relative_url }})
  — the soak-loop war story; matrix and soak are not redundant.
- [The 24/7 build farm]({{ '/lab/build-farm/' | relative_url }})
  — the magazine treatment of the parallel Docker machinery that
  iterates the perf experiments this reference describes the
  output of. Same `JCPERF2` records, but framed as
  methodology for keeping a 126-row matrix moving.
- [Hardware]({{ '/docs/hardware/' | relative_url }}) — what the
  optimizations are running against.
- [Build & toolchain]({{ '/docs/build/' | relative_url }}) — how the
  PS1 binary is produced.
- [Build infrastructure]({{ '/docs/infrastructure/' | relative_url }}) —
  the wrapper around the perf iterate script.
- [Audio pipeline]({{ '/docs/audio/' | relative_url }}) — the SPU side,
  which has its own scheduling concerns.
- [Story-loop walks]({{ '/docs/walks/' | relative_url }}) — the
  walk subsystem's persistent clean buffer is part of the same
  pressure-accounting envelope the matrix above measures; the v0.8.0
  clean-rect retry path and v0.8.1 wave-band/split-rect pressure
  changes are documented there.
- [Vision-classifier work]({{ '/docs/vision/' | relative_url }}) — the
  validation layer that runs against perf-experiment outputs.
- [Devlog]({{ '/devlog/' | relative_url }}) — perf work shows up
  day-by-day there.

## View source on GitHub

The body cites a dozen files; this section collects them. Grouped
by purpose — plan and ledgers, runtime, iterate gate, the scene
matrix, the compiler-flag and preprocessing sweeps, the read-plan
rollup, and the regtest runner.

- [`docs/ps1/performance-optimization-plan.md`]({{ site.github_url }}/blob/main/docs/ps1/performance-optimization-plan.md)
  · [`docs/ps1/performance-experiment-log.md`]({{ site.github_url }}/blob/main/docs/ps1/performance-experiment-log.md)
  — the optimization plan and the 600+ experiment ledger.
- [`src/ps1_perf.c`]({{ site.github_url }}/blob/main/src/ps1_perf.c)
  · [`src/foreground_pilot.c`]({{ site.github_url }}/blob/main/src/foreground_pilot.c)
  — runtime: the `JCPERF2` instrumentation and the FG2 dispatcher
  whose per-frame budget the matrix measures.
- [`scripts/ps1-perf-iterate.sh`]({{ site.github_url }}/blob/main/scripts/ps1-perf-iterate.sh)
  — the experiment gate every probe goes through (run → compare →
  promote-or-reject).
- [`docs/ps1/performance-scene-matrix.csv`]({{ site.github_url }}/blob/main/docs/ps1/performance-scene-matrix.csv)
  — the full scene/tide battle card; rendered as the live sortable
  matrix at [/perf/]({{ '/perf/' | relative_url }}).
- [`docs/ps1/performance-o2-audit.md`]({{ site.github_url }}/blob/main/docs/ps1/performance-o2-audit.md)
  · [`docs/ps1/performance-o2-audit.csv`]({{ site.github_url }}/blob/main/docs/ps1/performance-o2-audit.csv)
  — current compiler-flag sweep, regenerated from
  `build-ps1/compile_commands.json` + `build-ps1/jcreborn.map`
  before each `-O2` probe.
- [`docs/ps1/performance-preprocess-opportunities.md`]({{ site.github_url }}/blob/main/docs/ps1/performance-preprocess-opportunities.md)
  · [`docs/ps1/performance-preprocess-opportunities.csv`]({{ site.github_url }}/blob/main/docs/ps1/performance-preprocess-opportunities.csv)
  · [`scripts/analyze-fg2-preprocess-plans.py`]({{ site.github_url }}/blob/main/scripts/analyze-fg2-preprocess-plans.py)
  · [`docs/ps1/performance-preprocess-visitor3-hotspots.csv`]({{ site.github_url }}/blob/main/docs/ps1/performance-preprocess-visitor3-hotspots.csv)
  — pack-time graphics preprocessing target sheet, the FGP2/FGP3
  per-pack analyzer, and the VISITOR3 cap-hit / saving-heavy frame
  sheet.
- [`docs/ps1/performance-read-candidate-matrix.md`]({{ site.github_url }}/blob/main/docs/ps1/performance-read-candidate-matrix.md)
  · [`docs/ps1/performance-read-candidate-matrix.csv`]({{ site.github_url }}/blob/main/docs/ps1/performance-read-candidate-matrix.csv)
  — foreground read-plan candidates classified by append-start
  ownership, grouped-read capacity, and visible-CD cost class.
- [`scripts/run-regtest.sh`]({{ site.github_url }}/blob/main/scripts/run-regtest.sh)
  — headless DuckStation runner that captures PNGs and ingests
  TTY records into per-run summary JSON files.
