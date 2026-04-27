# PS1 Performance Next 100

Date: 2026-04-26

Current accepted fishing1 exact baseline:

| Metric | Value |
|---|---:|
| `loop_vb` | `1221` |
| `target_vb` | `1071` |
| `remaining_overrun_vb` | `150` |
| `remaining_over_target` | `14.01%` |
| `blocking_vb` | `5` |
| `prefetch_overrun_vb` | `5` |
| `loop_reads` | `67` |
| `upload_bytes` | `16281600` |
| `restore_bytes` | `2510092` |
| `prefetch_buffer` | `29712` bytes |
| `jcreborn.exe` | `143360` bytes |
| `jcreborn.elf` | `714260` bytes |

Goal: close `150` remaining loop VBlanks without changing pixels, sound event
timing, scene identity, or long-run heap stability. A 1% win at the current
baseline is about `12` VBlanks, so the practical target is roughly fourteen
1% wins, thirty 0.5% wins, or one structural CD/render breakthrough plus a
stack of flat-timing cleanup wins.

Current note: the fishing1 high-tide tail read group `396..406` is accepted as
a work-reduction checkpoint, not a VBlank speed win. It keeps the remaining
`150` VBlank gap unchanged while dropping `loop_reads 68 -> 67`,
`setloc 74 -> 73`, `loop_read_vb 284 -> 283`, and `seek_back 5 -> 4`; the
follow-up retained-capacity pass kept that saved read while reducing the
runtime prefetch buffer `31760 -> 29712` bytes. The first two narrow cold-TU
compiler probes are also accepted: `ps1_captions.c -Os` and `memcard.c -Os`
kept timing and layout flat while shrinking `jcreborn.elf 741076 -> 740196`.
The latest flat cleanup combines upload and dirty-rect perf markers, keeping
all VBlank/CD/work metrics and upload/dirty counters unchanged while shrinking
the ELF to `714260` bytes.
The latest harness pass adds host-side CD-summary comparison, so future
`blocking_reads 4 -> 5` regressions can be localized to FG2 file sectors
without adding PS1-side metrics that change the speed binary.

Acceptance rule: use the exact fishing1 headless gate first. Promote only if a
key VBlank metric improves without regressing `blocking_vb`,
`prefetch_overrun_vb`, layout identity, work identity, or correctness. Flat
timing plus meaningful code-size/work reduction is acceptable, but must not be
counted as a speed win.

## Highest-Leverage Thesis

The remaining gap is not one single bottleneck. The current loop has hidden CD
work mostly under control, but small scheduling/code-shape shifts still create
the fifth visible read. The best path is parallel pressure on five fronts:

| Front | Why It Can Still Move |
|---|---|
| CD grouping and read-cost prediction | Raw window sizes failed, but the CD log now shows zero-extra-sector group candidates. |
| Explicit render/CD slack scheduler | Several rejected variants were nominally faster but stole the slack hiding CD work. |
| Pack-emitted render/upload metadata | Runtime dirty/upload heuristics are locally exhausted; generated plans can remove branches. |
| Toolchain and layout control | Many valid cleanups regressed only because code/CD phase shifted. That is a solvable build problem. |
| Separate release/perf-log baselines | Perf logging is now part of the optimized path; release-speed measurements may expose free headroom. |

## Fresh Targets From The Latest Misses

The late 2026-04-26 wave ruled out more blind whole-TU compiler probing. Both
hot `-O3` attempts expanded executable layout, moved FG2 placement, and raised
visible CD pressure. The next useful tests should control phase first, then
retry promising source/toolchain ideas inside that controlled envelope.

The current `perf-detail` sample (`20260426-234118`) shifts priority again:
`present_wait_vb=157` against a remaining `150` VBlank overrun. Visible CD is
down to `5` VBlanks. The next major win has to reduce or hide present wait
without early display, tearing, frame drops, or weakened pause input.

| # | Target | Test Shape | Expected Signal |
|---:|---|---|---|
| 101 | Pure CD phase sweep | Insert `0..8` dummy sectors after `JCREBORN.EXE` with no source change. | Partially tested at `+1`, `+2`, `+3`, `+4`, and `+8`: exact timing-flat, so FG2 LBA alone is not the current speed lever. |
| 102 | Pure executable bucket sweep | Add inert text/data padding to keep FG2 LBA fixed while changing EXE bucket. | `+2 KB` was exact flat timing; use padding as a control tool, not a standalone speed target. |
| 103 | Hot-symbol address sweep | Pad before/after `foreground_pilot.o`, `cdrom_ps1.o`, and `graphics_ps1.o` independently. | Single-function `fgRuntimeFillWindowForEntry` positive shifts were flat; broader object/order sweeps are still untested. |
| 104 | Link-order sweep | Move `cdrom_ps1.o` before and after foreground/graphics without changing code. | Tests instruction locality and branch/cache phase as a first-class variable. |
| 105 | Function alignment sweep | Try 4/8/16/32-byte alignment for only CD/foreground hot functions. | Finds low-cost address buckets without whole-TU codegen changes. |
| 106 | Cold-section ballast | Keep cold `-Os` size wins but add deterministic padding to preserve the accepted EXE sector bucket. | Unlocks prior size wins without changing playback cadence. |
| 107 | Release-libs phase harness | Retest Release SDK libraries across CD and text phase pads. | Determines if the `1220` loop-VBlank signal can survive `blocking_vb<=5`. |
| 108 | Fifth-read locator | Host-side script maps the extra blocking read to LBA/file-sector timing candidates and covered entries. | Done for file-sector/CD-log comparison; frame/slack ownership still needs a trace binary or generated scheduler metadata. |
| 109 | Per-read slack class report | Bucket each read by held VBlanks available, sectors, preserved bytes, and overrun. | Delivered-sector parsing is done host-side; remaining gap is runtime slack ownership without perturbing the speed binary. |
| 110 | Group-fire trace build | Diagnostic binary logs why each planner group did or did not fire. | Explains why `384..396` remained a no-op. |
| 111 | Generated group metadata v2 | Emit group candidates beside FG2 entries without moving payload offsets. | Replaces hard-coded one-off group tables. |
| 112 | Selective two-group tail retry | Retry `384..396` only after group-fire tracing proves the append point. | Avoids increasing buffer capacity for groups that cannot execute. |
| 113 | Prepared-state detail counters | Add trace-only `prepared_used`, `prepared_missed`, and `prepared_blocked_cd` counters. | Separates useful precompose work from duplicate ballast. |
| 114 | CD-first scheduler prototype | Held slice owner order becomes read deadline, then precompose, then idle wait. | Prevents render prep from stealing the CD slack that hides reads. |
| 115 | Read-deadline reservation | Reserve a minimum hidden-read budget before any speculative render prep. | Retests prepared-present ideas without creating extra visible reads. |
| 116 | No-source layout canary | Nightly/headless run checks that a rebuild of unchanged source preserves cadence. | Detects toolchain/container nondeterminism before optimization tests. |
| 117 | Perf-log off baseline | Capture release-speed metrics with logging disabled or minimized. | Quantifies how much of the remaining gap is diagnostic overhead. |
| 118 | Trace-binary split | Build a separate diagnostic executable so counters never perturb accepted speed binaries. | Allows high-detail metrics without invalidating timing. |
| 119 | Pack-local upload plans | Emit dirty/upload bands at pack generation time. | Removes runtime scan/merge logic instead of tuning it further. |
| 120 | Pack-local restore plans | Emit previous-frame restore bands and full-cover row masks. | Reduces restore work without runtime intersection checks. |
| 121 | Generated compositor classes | Group spans by alignment/length class offline. | Enables branch-light PAL4 composition without whole-TU `-O3`. |
| 122 | CD helper assembly microbench | Hand-code only the sector math/copy inner helper, preserving C call shape. | Tests runtime benefit without compiler expanding the whole TU. |
| 123 | Controller-poll release probe | Measure pause/input polling cost only in release/perf-off mode. | Avoids optimizing pad paths around perf-log noise. |
| 124 | ISO ordering probe | Move inactive resource/SND trees after active FG packs in a scratch layout. | Tests whether active scene adjacency can lower seek/read variability. |
| 125 | Cross-scene phase sample | Run the phase winner against fishing2/fishing3 before promotion. | Prevents a fishing1-only CD layout win from hurting the next validated scenes. |
| 126 | Present wait map | Emit trace-only frame classes: normal render, prepared-present, crossed-restore, crossed-compose, crossed-upload. | Shows which frames actually pay the full one-VBlank present wait. |
| 127 | Prepared upload feasibility proof | Analyze VRAM layout and active display area to prove whether any offscreen partial-buffer strategy can exist. | Blocks unsafe "upload early" ideas unless VRAM memory proves them possible. |
| 128 | Dirty-band offscreen staging | Prototype staging only dirty upload bands into unused VRAM, then copy during VBlank. | Could trade CPU/VRAM for shorter visible upload work. |
| 129 | Per-band VBlank deadline ordering | Sort upload bands by scanline/display risk instead of current tile order. | May reduce visible risk if any partial pre-VBlank upload is considered. |
| 130 | Prepared frame dual-RAM background | Keep two RAM composited backgrounds for current and prepared frames if heap allows. | Removes restore/compose from due frame without reusing mutable current state. |
| 131 | Dirty-row copy-on-write prepared RAM | Store only prepared dirty rows instead of full second background. | Lower memory version of dual-RAM background. |
| 132 | Prepared dirty-band delta buffer | Encode prepared frame as dirty row deltas to apply quickly at due time. | Moves compose cost out of present path without full-frame RAM. |
| 133 | VBlank upload budget counter | Trace how many upload bytes/rects fit inside one VBlank on target. | Separates mandatory wait from upload overrun. |
| 134 | Present wait skip proof gate | Add a diagnostic-only guard that proves a VBlank was already reached before upload. | Prevents unsafe skip experiments from being promoted blindly. |
| 135 | Display-page feasibility | Audit whether any lower-resolution/dithered page flip mode can preserve pixels. | Likely no, but it must be proven before dismissing page flipping. |
| 136 | Interlaced-field split upload | Test whether top/bottom field timing can safely split uploads. | Could reduce full-frame present wait if field safety is exploitable. |
| 137 | Pre-VBlank restore scheduling | Move only RAM restore earlier under a CD-first budget. | Reduces due-frame CPU before VSync without touching display early. |
| 138 | Pre-VBlank compose scheduling | Move only PAL4 compose earlier after restore has proven safe. | Extends prepared work while watching duplicate prep and CD starvation. |
| 139 | Prepared upload no-op class | Identify frames where next upload bands are identical to current framebuffer. | Those frames might advance without an upload. |
| 140 | Host timing hold rebalance | Recompute hold distribution to absorb known one-VBlank present latency without dropping entries. | More principled version of long-hold catch-up. |
| 141 | Sound-safe timing rebalance | Verify any hold rebalance against sound event cursor and late counters. | Prevents speed wins from desyncing the now-working sound path. |
| 142 | Per-scene present budget metadata | Emit expected present cost per frame in FG2 metadata. | Lets the scheduler choose where to spend catch-up safely. |
| 143 | Frame-class-specific catch-up | Apply catch-up only after expensive rendered frames and only when next CD is resident. | More targeted than threshold-only catch-up. |
| 144 | Present/input split | Poll Start on held frames and after rendered frames through an explicit cadence table. | Allows input cleanup without removing necessary polling. |
| 145 | Prepared-present state machine v3 | Make prepared visual, staged payload, and future window ownership explicit states. | Required before retrying decoupled prepared frames. |
| 146 | VRAM copy primitive benchmark | Measure `MoveImage`/GPU copy cost versus `LoadImage` for dirty bands. | Determines whether offscreen staging can be cheaper than CPU upload. |
| 147 | Upload command prebuild | Precompute `RECT`/pointer command data for prepared dirty bands. | Removes setup overhead on the due-frame upload path. |
| 148 | Pack-emitted present bands | Emit exactly the bands needed for prepared upload at pack time. | Replaces runtime dirty scan in the present path. |
| 149 | Cross-scene present histogram | Run detail attribution on fishing2/fishing3 and later all scenes. | Confirms whether present wait dominates beyond fishing1. |
| 150 | Visual signoff harness for present experiments | Capture stills/video around any present-wait change before promotion to main. | Present optimizations can pass counters while tearing visually, so they need extra signoff. |

## Impact-Prioritized Order

| Priority | Area | Tests | Why First |
|---:|---|---|---|
| 1 | Pack-emitted/read-costed CD groups | `11-18`, `34-35` | The hard-coded tail group proves selective grouping can remove reads, while the broad 12-sector import proves a cost predictor is mandatory. |
| 2 | Pack-emitted render/upload metadata | `61-64`, `81-90` | Runtime upload/compositor heuristics are locally exhausted; generated metadata can remove branches and preserve deterministic work identity. |
| 3 | Explicit scheduler/CD budget | `41-50` | Many near-misses were nominal wins that stole CD slack. A CD-first budget is the gate for retrying them safely. |
| 4 | Toolchain/layout control | `91-100` | Valid code-size cleanups still perturb hot phase. Layout control can unlock old no-promotion wins without changing pixels. |
| 5 | Release/perf baseline separation | `1-10` | The harness should keep improving, but the immediate speed path is now runtime/pack work rather than more baseline bookkeeping. |

## Next 100 Tests

| # | Area | Test | Why Now | Promote If |
|---:|---|---|---|---|
| 1 | Baseline | Add a release-speed run without `perf-log` and compare against the perf-log baseline. | We may be optimizing the logging build instead of the shipped runtime. | Release run is faster and remains visually/sound identical, creating a separate target baseline. |
| 2 | Baseline | Add a dual-baseline policy: diagnostic baseline and release baseline. | Future changes should be judged against the right runtime mode. | Harness records both without weakening the strict perf-log gate. |
| 3 | Harness | Add a non-promotable trace binary for per-read frame/slack class logging. | Inline CD histograms perturb the speed binary. | Trace build explains the fifth visible read without changing the accepted binary. |
| 4 | Harness | Add host-side correlation from DuckStation read timestamps to frame indices. | Current CD log has LBAs but not the exact runtime frame owner. | We can name the read/frame that creates `blocking_reads=4/5`. |
| 5 | Harness | Add optional full-frame hashes to reject sequential-CD false positives. | Skipping `Setloc` once looked fast but collapsed visual work. | The gate can safely retest lower-level CD continuation ideas. |
| 6 | Harness | Gate `scene_vb` alongside `loop_vb` for setup-shift experiments. | Setup-prerender reduced loop accounting but not total time. | Future setup shifts cannot fake active-loop wins. |
| 7 | Harness | Add map-address tolerance bands for hot functions. | We now know fixed EXE/LBA can still regress from code-address phase. | We can identify which address shifts are dangerous. |
| 8 | Harness | Add automated compiler-flag matrix runner that logs no-promotion outcomes. | Toolchain work needs lots of controlled probes. | Matrix results are searchable and never dirty the accepted branch on failure. |
| 9 | Harness | Add cross-scene smoke subset after every accepted fishing1 speed win. | Fishing1 knees may not hold for fishing2/fishing3. | Fishing2/fishing3 exact cases stay within agreed pressure gates. |
| 10 | Harness | Add a "fifth visible read" summary to `ps1-perf-cdlog-summary.py`. | Current failures often move `blocking_reads 4 -> 5`. | Done: `--compare` reports `JCPERF2` deltas and file-sector-normalized timing candidates without touching the PS1 binary. |
| 11 | CD | Implement runtime lookup for 12-sector zero-extra-sector read groups. | Broad import failed, but the tail-only group `396..406` is accepted; next step is costed/selective groups, not blanket planner import. | `loop_reads`, `loop_read_vb`, or `blocking_vb` falls with no new visible read. |
| 12 | CD | Implement 16-sector group lookup behind a stricter slack guard. | Planner shows `69 -> 29`; raw 24 KB failed, but exact groups may not. | Read count falls and `blocking_vb` stays `<=5`. |
| 13 | CD | Implement 24-sector group lookup only for proven late long-hold regions. | Planner shows `69 -> 20`; broad high-slack reads failed. | Late-sequence reads drop without `prefetch_overrun_vb` growth. |
| 14 | CD | Target the remaining late groups around LBAs `748`, `755`, `762`, `769`, `801`. | The tail group already removed one late read; continue only where the cost model says the append fits held slack. | That cluster loses at least one more read or hidden VBlank. |
| 15 | CD | Target only group `file_sector 22..34` from the 12-sector plan. | Early hard-coded group `106..117` did not fire, so new groups need append-start proof first. | No timing regression and read count drops by one. |
| 16 | CD | Add append-cost predictor using `appendBytes`, `preserveBytes`, delivered sector count, and slack. | Host-side delivered-sector parsing now gives actual read spans; runtime slack ownership is the missing input. | Predictor blocks variants that would create the fifth visible read. |
| 17 | CD | Add group-cost predictor from measured host read durations. | Same sector count can have different elapsed cost. | Group selection correlates with lower `loop_read_vb`. |
| 18 | CD | Prefer group reads only when current window tail preservation is zero-copy. | `memmove` is useful but may be expensive at the wrong time. | Fewer reads without larger `used_vb`. |
| 19 | CD | Retry smaller raw windows after group metadata exists. | Old `14/15 KB` windows starved due frames because coverage was blind. | `prefetch_overrun_vb` falls without due misses. |
| 20 | CD | Retry `direct-stage` caps after group metadata exists. | `8 KB` is the current knee, but groups can change coverage cost. | A lower cap reduces visible pressure or a higher cap reduces loop time safely. |
| 21 | CD | Implement group-fed second stage slot. | Isolated second slot caused due misses; grouped coverage may fix starvation. | Stage hits increase and due misses stay zero. |
| 22 | CD | Implement group-fed prepared-wait prefetch. | Raw prepared-wait prefetch was only safe after enough scheduler cleanup. | Duplicate prep stays low and CD pressure stays flat or improves. |
| 23 | CD | Retry direct-stage read into window with group/tail preservation. | The prior `8 KB` version was two VBlanks faster but added a visible read. | Keeps `blocking_reads=4` while preserving the loop win. |
| 24 | CD | Add append-preserving direct-stage seed v2. | Scratch seeding is accepted; smarter merge may save the copy without churn. | `loop_vb` or `loop_read_vb` falls with no extra seek-back. |
| 25 | CD | Test window-only path after grouped reads. | `no-stage1` failed structurally before the current simpler pipeline. | Stage buffer can be removed or bypassed without due misses. |
| 26 | CD | Test stage-only path after grouped reads. | Window logic may now be overkill for small frames. | Smaller binary/work with exact timing or better. |
| 27 | CD | Test dual-window ping-pong refill. | Single-window append is useful but serializes preservation and read. | Hidden reads increase without heap or due-frame regressions. |
| 28 | CD | Retry true async refill with first-class state ownership. | Naive async failed because it was inline and under-instrumented. | Async reduces blocking without controller-state or correctness failures. |
| 29 | CD | Test `CdReadSync(0)` completion polling during held waits. | Could hide async completion without spin-waiting. | No visible pressure increase and no missed reads. |
| 30 | CD | Retest sequential read continuation with frame hashes and stronger work gates. | Prior result was invalid but the idea is high upside. | Setloc drops without any frame/work/hash mismatch. |
| 31 | CD | Test lower-level CD continuation only for already sequential aligned reads. | Current active sequence is mostly forward. | `setloc` falls and visual identity remains exact. |
| 32 | CD | Cache CD `CdlLOC` for next sequential sector. | Avoid repeated `CdIntToPos`/`CdPosToInt` cost in hot reads. | Helper symbol/time shrinks with flat or improved timing. |
| 33 | CD | Precompute pack file LBA once into runtime state. | Isolated helper cache was no-op, but combined CD rewrite may use it. | Smaller CD helper or lower `loop_read_vb`. |
| 34 | CD | Emit FG2 group sidecar metadata without moving payload offsets. | Sound-event prefix shift proved payload offsets are fragile. | Metadata loads in setup and active playback remains phase-identical or faster. |
| 35 | CD | Emit FGP3 grouped chunks for fishing1 only. | A one-scene experimental format can prove value before all scenes. | Read count drops without pack-size explosion or pixel change. |
| 36 | CD | Test group padding that preserves current payload sector crossings. | Global payload alignment doubled blocking. | Padding improves specific group boundaries without shifting hot payload phase. |
| 37 | CD | Test CD layout ordering around current fishing1 pack and active SCR/PSB files. | Removing an unused file regressed setup/CD phase even with same FG LBA. | Setup and active loop both stay flat or improve. |
| 38 | CD | Build a CD-phase pad searcher over 0..8 sectors after EXE. | Several size wins failed from phase. | Finds a phase bucket that lets valid size cleanups pass. |
| 39 | CD | Test exact `FISHING1.FG2` LBA shifts around `399`. | We know `398` is bad; other phases may be better. | A deliberate phase improves loop/CD metrics without source changes. |
| 40 | CD | Test sector-aligned metadata prefix sizes around 6 KB, 8 KB, 10 KB. | `4 KB` regressed active phase, but nearby sizes may improve setup safely. | Setup drops and active loop stays flat or improves. |
| 41 | Scheduler | Build explicit held-slice budget: CD first, render prep second, wait last. | Many render-prep wins stole CD slack. | Prepared work falls without `blocking_vb` growth. |
| 42 | Scheduler | Track prepared-frame "used vs wasted" counters in a trace build. | Duplicate prep currently acts as ballast. | We can delete only truly wasted prep. |
| 43 | Scheduler | Retry prepared-present threshold `3` with CD-first budget. | Prior threshold improved nominal target but hurt CD pressure. | `target_vb` improves and `blocking_vb` stays flat. |
| 44 | Scheduler | Retry prepared-present threshold `5` with group prefetch. | Threshold-only saved prep but regressed pressure. | Prep drops and groups preserve CD coverage. |
| 45 | Scheduler | Retry exact-use prepared-current RAM reuse. | It removed duplicate work but changed CD phase. | Work drops and timing stays flat or improves. |
| 46 | Scheduler | Retry prepared-buffer release with explicit frame cursor state. | Prior version had suspect final cursor and due misses. | Buffer ownership simplifies without due misses. |
| 47 | Scheduler | Retry staged-frame prep at `3` VBlanks after group predictor. | It improved nominal overrun but exposed one CD VBlank. | Actual `loop_vb` or target improves without visible CD pressure. |
| 48 | Scheduler | Retry long-hold catch-up threshold `4` after group predictor. | Threshold `4` improved some accounting but regressed pressure. | `overrun_vb` falls and `blocking_vb` stays flat. |
| 49 | Scheduler | Test short-hold catch-up only when next group is resident. | Coverage alone was insufficient before groups. | `target_vb` improves without read pressure. |
| 50 | Scheduler | Add scanline-safe present scheduler prototype. | Skipping pre-upload wait looked fast but unproven. | Reduces present wait with frame-hash visual proof. |
| 51 | Scheduler | Compose staged frame before VSync only when no CD read is eligible. | Prior compose-before-wait stole prefetch cadence. | `restore/compose` phase moves earlier without CD regression. |
| 52 | Scheduler | Split `eventsWaitTick(0)` into pause-poll-only and full event paths. | Pause-poll-only regressed as a local change; full scheduler may use it. | Held-loop wait cost falls with stable pause behavior. |
| 53 | Scheduler | Poll Start only on held waits with remaining slack above threshold. | Input polling cost is small but constant. | No pause regression and VBlank/work metrics improve. |
| 54 | Scheduler | Test controller poll cadence in release baseline, not perf-log baseline. | Pad/SPI diagnostics once hid real cost. | Release run improves without missed Start input. |
| 55 | Scheduler | Move sound-event firing to prepared/present boundary only. | Sound is correct now, but event timing may add hot-path work. | Sound counters stay exact and loop work falls. |
| 56 | Scheduler | Consume leading empty artifact with a stricter visual/hash proof. | It was accepted, but follow-up setup render only moved accounting. | Further empty handling reduces `scene_vb`, not just `loop_vb`. |
| 57 | Scheduler | Test first-frame setup render plus explicit setup/loop phase barrier. | Prior version kept `scene_vb` flat. | Full scene time improves and CD pressure stays flat. |
| 58 | Scheduler | Replace repeated `foregroundPilotRuntimeAdvance()` checks with state-specific loop bodies. | Hot loop still branches through several modes. | Code shape shrinks or `advances` cost falls. |
| 59 | Scheduler | Split FG2 scene loop from legacy/testcard loop entirely. | Legacy diagnostic scenes are compiled out but mode checks remain. | Hot loop/code shrinks without phase regression. |
| 60 | Scheduler | Generate a fishing1-specific loop policy table at startup. | Branch decisions are deterministic for a scene. | Replaces runtime conditionals with table lookups and passes exact gate. |
| 61 | Graphics | Emit pack-time dirty/upload bands per frame. | Runtime upload heuristics are locally exhausted. | `upload_rects` or CPU work falls without wider bytes. |
| 62 | Graphics | Emit pack-time restore bands per previous frame. | Restore scans still cost `2.51 MB`. | `restore_bytes` or restore work falls with no stale pixels. |
| 63 | Graphics | Emit full-cover row masks to skip restore for rows overwritten by current frame. | Some rows may be fully replaced by FG2 spans. | Restore calls/bytes drop and visual hash stays exact. |
| 64 | Graphics | Retry clean-rect intersection skip using pack-time masks. | Runtime intersection screen cost more than it saved. | Pack-driven skip avoids hot runtime math. |
| 65 | Graphics | Replace dirty row min/max clear loops with packed bitset generations. | `memset` regressed because layout changed, not necessarily because clearing is optimal. | Clear work shrinks and CD phase stays flat. |
| 66 | Graphics | Write a custom MIPS dirty-row clear fill. | libc `memset` changed code shape and regressed. | Same layout or better timing with smaller clear cost. |
| 67 | Graphics | Isolate `grDrawBackground()` upload path into its own TU. | Narrow locals shrank ELF but regressed scheduler shape. | Upload changes become testable without disturbing FG2/CD code. |
| 68 | Graphics | Retry narrow upload locals after TU isolation. | Prior miss may be register/layout coupling. | ELF/stack shrinks and timing stays flat. |
| 69 | Graphics | Test 16-pixel X-aware upload with one final `DrawSync`. | Per-strip sync failed structurally. | Upload bytes fall and scene reaches `JCPERF2`. |
| 70 | Graphics | Test bounded scratch arena for X-aware upload strips. | Previous scratch attempts exploded rects or syncs. | Bytes fall with bounded rect count and no heap leak. |
| 71 | Graphics | Test exact-width multi-row bands only above a width-savings threshold. | Single-row exact-width was no-op in fishing1. | Upload bytes fall without rect pressure crossing the zero-gap failure. |
| 72 | Graphics | Sweep upload gap `0/1/2/3` after each accepted scheduler/CD change. | The current 1-row knee may move. | Either bytes or rects improve with flat timing. |
| 73 | Graphics | Add deterministic rect-widening cap instead of full fallback. | Pixel-perfect requires no fallback, but widening is deterministic. | Cap hits are explainable and `full_fallbacks=0`. |
| 74 | Graphics | Sort upload bands to reduce GPU command overhead. | Current order is likely row order, not necessarily command-optimal. | `upload_vb`/loop falls without pixel difference. |
| 75 | Graphics | Batch `LoadImage` rect setup data in a persistent small array. | Rect count is high at `502`. | Stack/code shrinks or upload work falls. |
| 76 | Graphics | Test max upload rect cap `7` after cross-scene proof. | Fishing1 max is `6`; cap `6` had no win but no headroom. | Cap `7` shrinks or remains safe across fishing scenes. |
| 77 | Graphics | Cache clean-rect row source pointers. | Restore bytes are stable but pointer math may be hot. | Restore helper shrinks or `restore_vb` drops in detail runs. |
| 78 | Graphics | Assembly unroll restore row copy for aligned spans. | Local C 32-bit helper regressed. | Detail restore time falls without CD phase loss. |
| 79 | Graphics | Test halfword-edge plus word-body restore copy. | Most rows may be aligned enough for word copies. | Restore detail counters improve. |
| 80 | Graphics | Test persistent clean-rect buffer reuse across same tide/background. | Memory leak fixes release per scene, but some buffers may be safely persistent. | Setup/restore improves without long-run heap drift. |
| 81 | Compose | Generate pack-time tile-split spans. | Runtime cross-tile splitting regressed; pack-time can remove hot branches. | Compose work falls and spans remain exact. |
| 82 | Compose | Generate per-row destination offsets in FG2 payload. | Runtime computes tile/row address repeatedly. | Compositor code/time shrinks. |
| 83 | Compose | Generate span command classes by alignment and length. | Dynamic aligned pair store regressed from branching. | Fast path uses branch-free classes. |
| 84 | Compose | Test MIPS assembly PAL4 even-run compositor. | C aligned-store variants added too much branch cost. | Compose detail improves without CD pressure. |
| 85 | Compose | Test MIPS assembly PAL4 odd-edge handler. | Edge cost may dominate short spans. | Compose detail improves. |
| 86 | Compose | Test PAL4 direct16 pack option for fishing1. | Pack size may grow, but CPU could fall sharply. | Loop improves enough to justify pack-size budget. |
| 87 | Compose | Test direct16 only for hot/large frames. | Avoid full pack-size explosion. | Worst frames get faster with bounded pack growth. |
| 88 | Compose | Test per-scene generated fishing1 compositor. | One validated scene can justify bespoke codegen. | Fishing1 loop improves and generated output remains auditable. |
| 89 | Compose | Test LUT-per-palette direct two-pixel writes in generated code. | Runtime LUT attempt was not enough. | Compose detail improves with no branch growth. |
| 90 | Compose | Test row-level span coalescing at pack time. | PAL4 four-pixel unroll was no-op alone. | Fewer commands/spans or lower compose detail. |
| 91 | Toolchain | Run per-file `-Os` on cold files: pause, captions, memcard, holidays, debug. | Holidays now passes under the foreground-size baseline; pause/resource/sound/stubs remain phase-sensitive retries. | EXE/ELF shrinks and exact cadence passes. |
| 92 | Toolchain | Run per-file `-O3` only on `graphics_ps1.c`. | Tested and failed; it grew graphics helpers and regressed visible CD pressure. | Do not retry whole-TU graphics `-O3`; the follow-up PAL4 helper-scoped `O3` also failed, so use assembly/generated code. |
| 93 | Toolchain | Run per-file `-O3` only on `foreground_pilot.c`. | Scheduler code may benefit or reveal layout limits. | Loop improves without CD pressure. |
| 94 | Toolchain | Run per-file `-O3` only on `cdrom_ps1.c`. | CD helpers are hot and recently shrank safely. | `loop_read_vb` or helper size improves. |
| 95 | Toolchain | Run per-file `-Os` only on `foreground_pilot.c`. | Done: exact-flat timing/work with PS-EXE `149504 -> 145408` and ELF `739900 -> 727716`. | Keep as a size/code-shape win; do not count as VBlank speed. |
| 96 | Toolchain | Sweep `-G0`, `-G4`, `-G8`, `-G16`/GP-relative small-data thresholds. | Current `GPREL` likely implies a default small-data tradeoff. | Loop or binary size improves without heap/data regressions. |
| 97 | Toolchain | Sweep hot-function alignment: default, 4, 8, 16, 32 bytes. | Code-address phase is proven important. | A phase bucket improves loop or CD pressure. |
| 98 | Toolchain | Link hot FG2/CD sections first. | Keep scheduler/CD code contiguous and stable while cold code changes. | Hot symbol addresses stabilize and timing improves or becomes less fragile. |
| 99 | Toolchain | Split cold menu/debug/caption code into a cold archive or section. | Valid cold-code cleanup currently perturbs hot phase. | Cold shrink passes exact gate. |
| 100 | Toolchain | Add deterministic text padding around hot functions, then retry failed valid cleanups. | Active-guard removals and diagnostic gates were semantically valid but phase-sensitive. | At least one old size win becomes timing-flat or faster. |

## First Execution Order

Run these first because they unlock many later tests or target known high-upside
near misses:

| Order | Test # | Reason |
|---:|---:|---|
| 1 | 1 | Establish whether the remaining 14.01% is partly perf-log overhead. |
| 2 | 10 | Done; use the host-side comparison output to target sector-specific CD/read-cost work. |
| 3 | 16 | Cost predictor needed before any more grouped-window or raw window-size probes. |
| 4 | 11 | Continue grouped-read runtime only through selective/costed boundaries; broad 12-sector import already failed, tail `396..406` is accepted. |
| 5 | 23 | Revisit a near miss that already showed a two-VBlank loop win. |
| 6 | 38 | Find a safe CD/code phase bucket for valid size cleanups. |
| 7 | 91 | Start compiler/toolchain matrix with cold `-Os`, not hot `-O3`. |
| 8 | 126 | Build present-wait frame-class tracing in a diagnostic binary. |
| 9 | 127 | Prove or reject offscreen/dual-buffer feasibility before upload-early tests. |
| 10 | 145 | Redesign prepared-present ownership as an explicit state machine. |

## Retest Rules For Old Failures

| Old Failure Class | Retry Only After |
|---|---|
| Raw larger windows | Group metadata plus cost predictor exists. |
| Smaller windows | Group metadata preserves due-frame coverage. |
| Prepared-frame cleanup | Explicit render/CD budget exists. |
| Direct-stage read-into-window | Group/tail-preserving merge keeps `blocking_reads=4`. |
| Debug/code-size compile gates | Text/CD phase padding or hot/cold section isolation exists. |
| Audio TU `-Os` | Done under the foreground/resource-size baseline; keep accepted unless cross-scene sound validation regresses. |
| Resource TU `-Os` | Done under the foreground-size baseline; keep accepted unless a cross-scene setup regression appears. |
| Pause-menu TU `-Os` | Done under the foreground/resource/sound-size baseline; keep accepted if normal pause visual/input validation passes. |
| PS1 stubs TU `-Os` | Done under the pause-menu-size baseline; keep as cumulative ELF pressure only. |
| Events TU `-Os` | Done under the pause/stub-size baseline; keep accepted if normal pause/input validation passes. |
| Utils TU `-Os` | Done under the events-size baseline; keep as cumulative ELF pressure only. |
| Uncompress TU `-Os` | Do not retry as an isolated flag; it was exact no-op on runtime and size. |
| Island TU `-Os` | Done under the utils-size baseline; keep accepted if random-island visual validation stays clean. |
| Main TU `-Os` | Done under the island-size baseline; keep accepted if menu/pause visual validation stays clean. |
| CDROM TU `-Os` | Do not retry as a whole-TU flag; it crossed a smaller PS-EXE bucket and regressed `blocking_vb` to `10`. |
| Buffered read file-LBA cache | Do not retry alone; it was timing-flat but grew the helper and ELF. |
| Upload perf guard combine | Done; keep because it shrank `grDrawBackground` with exact timing/work identity. |
| Upload perf guard local cache | Do not retry alone; it shrank the function but grew total ELF with no speed movement. |
| Inline perf-detail check | Do not retry alone; it shrank two hot functions but grew total ELF with no speed movement. |
| Upload perf marker combine | Done; keep because it removed one rendered-frame marker call and preserved all upload/dirty counters. |
| Hot whole-TU `-O3` | Function-scoped codegen or address padding preserves hot layout first. |
| Graphics whole-TU `-O3` | Do not retry; `grDrawBackground`/restore code grew and cadence regressed to `blocking_vb=11`. |
| PAL4 compositor function-scoped `O3` | Do not retry; it shrank `grCompositePacked4SpansToBackground` by `28` bytes but still regressed cadence with `FISHING1.FG2` LBA restored. |
| Perf TU `-O3` | Do not retry as a whole-TU flag; it bloated `ps1PerfMarkCdReadDetailed` and regressed the exact gate. |
| Perf TU `-Os` | Do not promote just for ELF shrink; it left `jcreborn.exe` flat and grew hot perf functions. |
| Unused perf wrapper removals | Do not remove `ps1PerfMarkCdRead()` without padding/control; it is dead code but currently stabilizes perf/CD layout. |
| Pure FG2 LBA shifts | Do not retry as a standalone speed test; tested offsets up to `+8` sectors were timing-flat. |
| Hot-loop source cleanups | Preserve or deliberately sweep hot-symbol addresses first; the redundant prefetch pre-check removal failed even with FG2 LBA restored. |
| Whole-TU link-order moves | Do not retry the simple `cdrom_ps1.c`-next-to-foreground order; it was timing-flat despite large symbol movement. Use targeted hot-section padding or cold-section isolation instead. |
| Runtime dirty/upload heuristics | Pack-emitted masks or upload plans replace hot runtime checks. |
| Hard-coded read groups | Append-start trace or generated group metadata proves the group can fire; sectors `106..117` and `384..396` were exact no-ops even with extra retained capacity. |
| Upload coordinate static tables | Do not retry; static tables grew `grDrawBackground` and did not move timing. |
| Async CD | Async state ownership and polling metrics exist in a trace build. |
| `Setloc` skipping | Full frame hashes and work-identity gates prove every frame rendered. |
| Upload rect cap `6` | Cross-scene matrix proves `max_upload_rects <= 6`; current fishing1-only retest has no measured win. |
| Present-path polling cleanup | Replace only as part of a scheduler/input design; direct removal regressed pressure and weakens pause responsiveness. |
| 4-VBlank catch-up guards | Do not retry as another local threshold guard; the prepared-plus-window-resident form was exact no-op. Retry only as structural hold rebalance or a first-class prepared/dual-buffer scheduler. |
