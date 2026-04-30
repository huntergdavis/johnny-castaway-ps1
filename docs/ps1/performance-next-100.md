# PS1 Performance Next 100

Date: 2026-04-27

Current accepted fishing1 exact baseline:

| Metric | Value |
|---|---:|
| `loop_vb` | `1207` |
| `target_vb` | `1076` |
| `remaining_overrun_vb` | `131` |
| `remaining_over_target` | `12.17%` |
| `blocking_vb` | `0` |
| `prefetch_overrun_vb` | `0` |
| `loop_reads` | `6` |
| `upload_bytes` | `6690560` |
| `restore_bytes` | `251144` |
| `prefetch_buffer` | `333656` bytes for fishing1 high-tide FGP3 setup-prime, `366841` bytes for fishing2 high-tide FGP3 setup-prime, smaller variants otherwise |
| `jcreborn.exe` | `145408` bytes |
| `jcreborn.elf` | `726432` bytes |

Goal: close `131` remaining loop VBlanks without changing pixels, sound event
timing, scene identity, or long-run heap stability. A 1% win at the current
baseline is about `12` VBlanks, so the practical target is roughly twelve to
fourteen 1% wins, thirty 0.5% wins, or one structural CD/render breakthrough plus a
stack of flat-timing cleanup wins.

Red-team caveat: setup-prime passes are active-loop wins, not end-to-end
scene-time wins. The latest fishing2/fishing3 budgets move `128-352 KB` of
foreground reads into setup so active playback can reduce visible CD pressure
and spend more catch-up. Future work should hide these primes during
inter-scene/loading time or generate scene/tide-specific segmented coverage,
rather than treating setup time as free.

Current note: the fishing1 high-tide tail read group `396..406` is accepted as
a work-reduction checkpoint, not a VBlank speed win. It kept that era's
remaining VBlank gap unchanged while dropping `loop_reads 68 -> 67`,
`setloc 74 -> 73`, `loop_read_vb 284 -> 283`, and `seek_back 5 -> 4`; the
follow-up retained-capacity pass kept that saved read while reducing the
runtime prefetch buffer `31760 -> 29712` bytes. The first two narrow cold-TU
compiler probes are also accepted: `ps1_captions.c -Os` and `memcard.c -Os`
kept timing and layout flat while shrinking `jcreborn.elf 741076 -> 740196`.
Recent flat graphics cleanups compile only `grDrawBackground()` and
`grUpdateDisplay()` with `-Os`, keeping all VBlank/CD/work metrics and
upload/dirty counters unchanged while shrinking the ELF before the dirty-row
and CD-helper wave.
The `grRestoreBgFromRects()` function-scoped `-Os` retry is rejected despite a
local function shrink, because total ELF grew to `714132` bytes with no VBlank
movement.
The PAL4 compositor function-scoped `-Os` retry is also rejected: it shrank the
loaded executable, but still regressed to `blocking_vb=26` even when a temporary
CD pad preserved `FISHING1.FG2` at LBA `396`.
The single-band narrow-upload scratch path is rejected too: fishing1 did not
hit a useful single-band case, so upload bytes stayed flat while code grew.
The latest accepted cleanup clears only touched current dirty rows before each
frame restore, preserving all timing/work counters while shrinking the ELF to
`712692` bytes.
The latest speed win also promotes only touched dirty-row ranges from current
to previous dirty state, improving `loop_vb 1221 -> 1219` and
`overrun_vb 150 -> 147` with CD pressure and graphics work stable.
A direct PAL4 row dirty-marking probe after that baseline is rejected even
though the formal gate passed: `loop_vb` stayed flat and the apparent overrun
gain came only from a `target_vb` shift while the compositor grew.
Skipping previous dirty-row clears for rows overwritten by current dirty rows
is also rejected as an isolated change: it stayed exact-flat but grew the ELF.
Post-dirty raw stream-window retuning is rejected: `20 KB` regressed visible
CD pressure, while `18 KB`/`17 KB` hit a structural invalid-read failure before
metrics.
The aligned CD read helper now caches its file LBA once per read, keeping exact
cadence flat while shrinking the hot helper by 8 bytes and ELF by 84 bytes.
A single-chunk branch inside that helper is rejected: the common-case branch
duplicated too much error/read code and grew the hot helper by 104 bytes with
no timing movement.
Function-scoped `-Os` on the aligned CD read helper is accepted: it keeps
exact cadence flat while shrinking the public aligned-read wrapper to 8 bytes
and ELF to `712556`.
Function-scoped `-Os` on the unbuffered stream-read helper is accepted too,
shrinking that setup-facing helper by 56 bytes and ELF to `712524` with exact
playback identity.
The same unbuffered helper now also caches its file LBA once, shrinking it by
another 32 bytes and ELF to `712332` with exact playback identity.
Function-scoped `-Os` on `fgRuntimeFillWindowForEntry()` is rejected as an
exact no-op: the accepted foreground TU codegen already emits the same helper
shape.
The first prepared-visual decoupling pass is rejected but informative:
metadata-only decoupling was flat and code-heavy, stage-next decoupling reduced
`loop_read_vb` by 3 without moving `loop_vb`, and preparing earlier at `>=4`
failed structurally. The next retry needs a real scheduler budget or separate
prepared visual storage, not another local threshold tweak.
A positive-slack-only stage-next retry reproduced the same flat/key-metric
result, so local prepared-payload decoupling is exhausted for this baseline.
The `102..110` FG2 read-group probe is rejected even though it saved one
transaction: visible CD pressure rose from `5` to `8` VBlanks, confirming that
new groups need a read-duration/slack cost model before promotion.
The upload path now reuses one stack `RECT` for immediate `LoadImage()` calls,
shrinking `grDrawBackground` by 8 bytes and ELF to `712272` while keeping every
timing, CD, upload, and correctness counter exact.
Function-scoped `-Os` on the buffered CD helper is rejected: it kept timing
flat but grew the ELF and did not shrink the helper.
Retesting the staged-copy fallthrough guard at `5` held VBlanks is rejected:
it doubled visible CD pressure after the CD-helper cleanup, so the current
`6` VBlank guard remains the local knee.
Raising that guard to `7` is also rejected as a structural failure before
metrics. Local fallthrough-threshold probing is exhausted for this baseline.
The latest harness pass adds host-side CD-summary comparison, so future
`blocking_reads 4 -> 5` regressions can be localized to FG2 file sectors
without adding PS1-side metrics that change the speed binary.
The latest runtime pass adds `JCPERF2 sched` ownership counters. It keeps
fishing1 exact-flat (`loop_vb=1219`, `blocking_vb=5`, `prefetch_overrun_vb=5`,
`FISHING1.FG2 LBA=396`, PS-EXE `143360`) and reports `present=72`,
`cd_stage=108`, `cd_window=54`, `visual_prepare=72`, `wait=574`,
`cd_reserved=28`, `prep_blocked_cd=13`, `prepared_ready=72`,
`prepared_used=72`, and `prepared_wasted=0`. The first owned-idle catch-up
prototype did not fire usefully (`catchup_idle=0`) and moved layout, so it was
reverted; the next scheduler attempt needs per-frame ownership analysis, not
another threshold-only catch-up.
The first host-side preprocessing pass is now analysis-first:
`scripts/analyze-fg2-preprocess-plans.py` exactly reproduces the accepted
fishing1 runtime graphics counters (`restore_bytes=2510092`,
`upload_bytes=16281600`, `upload_rects=502`) from the pack alone. It shows
that upload-ready x-bands could cut upload bytes by about `49.61%`, but only
by carrying about `8.2 MB` of aligned frame-band payload for fishing1. Exact
interval upload is a stronger byte floor (`86.83%` reduction) but explodes to
`95259` rects. Restore-skip metadata is the safer next pack-format experiment,
but only with coalescing: exact restore skip predicts `52.41%` lower restore
bytes but raises restore intervals from `24300` to `73417`; the current
`min8px_max4pieces` profile still saves `26.35%` with `36450` intervals.
The runtime prototype that parsed current PAL4 spans before restore confirmed
the byte savings but failed as an implementation path: the best variant reduced
`restore_bytes` to `2222854`, yet regressed `loop_vb 1219 -> 1221`, visible CD
pressure `5 -> 6`, and moved `FISHING1.FG2` from LBA `396` to `397`. Treat
restore-skip as an FGP3/side-metadata problem, not a runtime reparse problem.
Two more hard-coded read-group probes are now rejected: `384..396` never fired
under the retained 11-sector capacity, and `307..317` kept every timing/read
counter exact while growing `foregroundPilotPlay` by `432` bytes. The direct
stage-into-window cache variant is also rejected: it removed the scratch-window
seed copy in theory, but exposed one extra visible CD VBlank. Finally,
single-TU `foreground_pilot.c -O3` is rejected as a no-win size/layout loss
(`jcreborn.exe 143360 -> 149504`, `FISHING1.FG2 LBA 396 -> 399`). The practical
conclusion is sharper: no more blind hard-coded groups or foreground-wide
compiler flags; the next CD win needs generated/costed group metadata or a
trace-backed scheduler budget.
The latest setup-prime wave proves a narrow exception: preloading enough FG2
coverage can make a previously unsafe threshold-`4` catch-up profitable, but
only when the catch-up is gated on a successful prime. The promoted `320 KB`
fishing1 high-tide prime improves `loop_vb 1219 -> 1215`, `overrun_vb
147 -> 140`, `blocking_vb 5 -> 1`, and `loop_reads 67 -> 43`; smaller
`192 KB`/`256 KB` versions either lost or still raised visible CD pressure.
The active-region/clean-rect follow-up found one safe narrow win: the static
backdrop has already been presented when FG2 clean rects are saved, so the
first forced upload no longer dirties all four screen tiles. Scoping that first
upload to the saved clean-rect Y band improves `loop_vb 1215 -> 1213`,
`overrun_vb 140 -> 138`, `max_upload_bytes 614400 -> 221440`, and
`upload_bytes 16281600 -> 15888640` without changing layout, restore bytes,
CD pressure, or correctness.
The I-B motion-comp analyzer changes the next pack-format order. Fishing1 has
large frame-to-frame reuse, but it is zero-shift temporal residual reuse rather
than translated motion: `151/154` candidate pairs, `71.16%` estimated payload
savings, and `0` nonzero-shift candidates. Walking scenes are the true
translation target (`WALKSTUF1` has `85` nonzero candidates; `WALK1LOW` has
`53`). Therefore the next fishing1-safe FGP3 experiment should be zero-shift
residual encoding first; GPU move/residual should wait for a walking-scene
validation path and a RAM-mirror/dirty-cleanup design.
The zero-shift runtime model is strong enough to promote to implementation
planning: fishing1 predicts compose payload `823277 -> 228087` (`72.30%`
saved), full-width dirty upload `15667200 -> 6576000` (`58.03%` saved), and
cleanup restore of only `136552` bytes. The hard invariant is that FGP3 must
carry full-current dirty metadata, because unchanged foreground pixels remain
in the RAM mirror and still need to be restorable on later frames.
The first FGP3 zero-shift temporal-residual pack is now promoted for fishing1
high tide. It converts `FISHING1.FG2` to `fgp3_pal4_residual`, improves
`loop_vb 1213 -> 1207`, `overrun_vb 138 -> 131`, and clears the last visible
high-tide CD pressure (`blocking_vb/prefetch_overrun_vb 1 -> 0`). Work volume
drops to `restore_bytes=251144`, `upload_bytes=6690560`, `upload_rects=290`,
and `loop_reads=6`. This accepted format change intentionally moves layout
(`FISHING1.FG2 LBA 396 -> 397`, PS-EXE `143360 -> 145408`), so future FGP3
work should claw back the executable cost and then fold residual generation
into the normal batch pack builder.
The follow-up red-team pass after FGP3 tested several local retries and found
the new bottleneck. Detail-tier attribution for the accepted canary reports
`present_wait_vb=155`, `compose_vb=2`, `restore_vb=0`, `upload_vb=0`,
`blocking_vb=0`, and `prefetch_overrun_vb=0`. Threshold-only prepared-present
changes, due-frame precompose, previous-dirty discard, FGP3 helper `-Os`, and
no-holiday call-site guarding all failed or stayed exact-flat. The next
high-impact path is no longer local restore/CD cleanup for fishing1 high tide;
it is a first-class present scheduler, a release/perf-log split, or a broader
pack/runtime architecture that can hide the mandatory VSync ownership without
dropping frames or weakening pause input.
The same FGP3 zero-shift format is now promoted for fishing1 low tide as well.
`FISH1LOW.FG2` converts to `fgp3_pal4_residual`, shrinks `426082 -> 303083`
bytes, and improves the low-tide gate `loop_vb 1215 -> 1209`,
`overrun_vb 142 -> 135`, `blocking_vb 5 -> 4`, `prefetch_overrun_vb 5 -> 4`,
and `loop_reads 31 -> 22`. High tide remained exact-flat after the low-tide
pack change. This makes generated all-scene FGP3 rollout a practical next
path, while low tide still has enough CD/refill pressure to justify targeted
pack-group or setup-prime policy work.
Fishing1 low tide now also uses the existing `320 KB` setup-prime policy.
Because the FGP3 low-tide pack fits inside the prime window, active-loop reads
fall `22 -> 0`, `blocking_vb 4 -> 0`, and `loop_vb 1209 -> 1207`; overrun
falls to the same `131` VBlank gap as high tide. This is intentionally logged
as an active-loop win with setup-cost trade (`setup_vb 182 -> 238`,
`scene_vb 1391 -> 1445`), so the next real global win is hiding that prime via
inter-scene preload or generated prime budgets.
FGP3 is now validated on fishing2 high tide as well. `FISHING2.FG2` shrinks
`1595559 -> 542743` bytes, clears due misses (`2 -> 0`), and improves
`loop_vb 1928 -> 1903`, `overrun_vb 190 -> 139`, `blocking_vb 50 -> 8`,
`prefetch_overrun_vb 44 -> 8`, and `loop_reads 134 -> 40`. This proves the
FGP3 residual approach scales past fishing1, but fishing2 still has enough
active-loop CD pressure to make setup-prime or generated pack-read groups the
next likely scene-specific win.
Fishing2 high tide now has a scene/tide-specific setup-prime budget. A
`352 KB` prime is the largest promoted point: it improves `loop_vb 1903 ->
1898`, `overrun_vb 139 -> 133`, `blocking_vb/prefetch_overrun_vb 8 -> 2`,
and `loop_reads 40 -> 14`. Larger contiguous primes are unsafe for this scene's
heap shape: `384 KB` and full-pack `544 KB` failed before loop start, while
`368 KB` hit the log cap/regtest `137`. The remaining two blocking VBlanks
should be attacked with generated read groups or inter-scene preload, not by
blindly growing the setup window.
A manual fishing2 high read-group probe for relative sectors `178..191` is
rejected. It moved the executable into the next sector bucket and shifted
`FISHING2.FG2 LBA 740 -> 741`, while regressing `loop_vb 1898 -> 1899` and
`blocking_vb/prefetch_overrun_vb 2 -> 3`. Any retry needs generated metadata
plus layout control, not another local source-table group.
Fishing2 low tide now also uses FGP3. `FISH2LOW.FG2` shrinks `784126 ->
385436` bytes, improves `loop_vb 1912 -> 1900`, `overrun_vb 157 -> 136`,
`blocking_vb/prefetch_overrun_vb 20 -> 5`, and `loop_reads 58 -> 27`, while
fishing2 high stays exact-flat. Remaining low-tide CD pressure is now small
enough to test generated setup-prime sizing or segmented prime coverage.
Fishing2 low tide now has a `256 KB` setup-prime budget. It improves
`loop_vb 1900 -> 1898`, `overrun_vb 136 -> 131`,
`blocking_vb/prefetch_overrun_vb 5 -> 0`, and `loop_reads 27 -> 10`, with
stable `FISH2LOW.FG2` LBA and PS-EXE bucket after cold diagnostic strings were
shortened. The rejected `320 KB` probe hit the log cap/regtest `137`, so low
tide should not grow a contiguous setup prime past `256 KB` without a new heap
or segmented-prime design.
Fishing3 high tide now uses FGP3 as the first larger next-scene proof point.
`FISHING3.FG2` shrinks `1831749 -> 724829` bytes and improves `loop_vb 2123 ->
2099`, `overrun_vb 189 -> 149`, `blocking_vb 87 -> 24`,
`prefetch_overrun_vb 39 -> 21`, and `due_misses 11 -> 1`. Its high-pack LBA and
PS-EXE bucket stay fixed; fishing3 low smoke still passes after the downstream
LBA shift. Next likely wins: FISH3LOW FGP3, then scene-specific or segmented
prime budgets.
Fishing3 low tide now also uses FGP3. `FISH3LOW.FG2` shrinks `906053 ->
549622` bytes and improves `loop_vb 2110 -> 2098`, `overrun_vb 156 -> 138`,
`blocking_vb 21 -> 8`, `prefetch_overrun_vb 21 -> 9`, and `loop_reads 65 ->
42`. Fishing3 now needs setup-prime or segmented preload work, not more format
conversion.
Contiguous fishing3 high setup-prime is rejected for now. `320 KB` failed
before playback; `256 KB` completed but kept `blocking_vb=24`, worsened
`due_misses 1 -> 2`, and moved both PS-EXE and `FISHING3.FG2` LBA. Retry this
scene only with segmented/generated prime coverage or inter-scene preload.
Fishing3 low tide now uses a `288 KB` contiguous setup-prime budget. The first
`256 KB` pass improved `loop_vb 2098 -> 2091` and `blocking_vb 8 -> 7`; the
retune keeps `loop_vb=2091` but lowers overrun/CD pressure to the fishing1-class
gap: `overrun_vb 134 -> 131`, `blocking_vb 7 -> 4`,
`prefetch_overrun_vb 7 -> 4`, and `loop_reads 24 -> 21`. Fishing3 high stays
exact-flat. The larger `320 KB` low-tide probe still remains rejected, so the
next larger target should be generated/segmented prime coverage, not another
blind contiguous read.
The `304 KB` low-tide retest confirms the knee: it kept layout stable but
regressed `blocking_vb 4 -> 5` and `overrun_vb 131 -> 132`. Keep `288 KB`
until segmented prime coverage can preload later ranges without shifting the
current CD phase.
Fishing3 high tide now has a smaller `128 KB` setup-prime budget. This is the
safe version of the earlier failed high-tide contiguous-prime idea: it improves
`loop_vb 2099 -> 2094`, `overrun_vb 149 -> 139`, `blocking_vb 24 -> 16`,
`prefetch_overrun_vb 21 -> 11`, and `loop_reads 52 -> 44` with stable layout.
Larger high-tide contiguous windows remain rejected; the next high-tide step
should be another measured small knee or segmented prime coverage, not a jump
back to `256 KB`.
The `160 KB` high-tide retest confirms the current knee: layout stayed fixed,
but active timing regressed to `loop_vb 2100`, `blocking_vb 29`, and
`due_misses 3`. Keep `128 KB` until the next test can preload later ranges
without using one larger contiguous read.
The `144 KB` midpoint is also rejected. It looked better only while moving the
executable/pack LBA; after recovering layout with cold string shrink, it
regressed `loop_vb 2094 -> 2096` and `blocking_vb 16 -> 21`. Treat high-tide
contiguous budget probing as exhausted at `128 KB`.
The planner-targeted `140 KB` point also failed with stable layout, regressing
`loop_vb 2094 -> 2095` and `blocking_vb 16 -> 19`. This confirms that the next
FISHING3 high win is not another contiguous prime size; it needs segmented
coverage or scheduler changes.
The scene-specific `2` VBlank refill guard is rejected too: it increased
visible CD pressure to `blocking_vb=23` and moved layout. Short-slack reads
remain unsafe without a real ownership budget.
Forcing fishing3 high back to catch-up threshold `5` is rejected as well:
blocking stayed flat while loop/refill/layout worsened. Keep setup-primed
threshold `4` until a scheduler can account for CD and catch-up ownership
together.
A fishing3 high read-group retry for relative sectors `223..234` is rejected:
the host CD log made it look like the safest local group, but the source-table
change moved PS-EXE/LBA and regressed `loop_vb 2099 -> 2103` plus
`blocking_vb 24 -> 28`. Future read groups must be generated and layout-held;
one-off hot source tables are exhausted for fishing3.
The first segmented setup-prime probe is accepted, but only as a narrow proof:
FISHING3 high relative sectors `67..73` read into scratch during setup moves
`loop_vb 2094 -> 2093` with stable layout and low tide exact-flat. It does not
lower `blocking_vb` yet, and it costs one setup read, so the next version needs
generated segment metadata or inter-scene preload that can target multiple
ranges without adding more hard-coded hot source logic.

Acceptance rule: use the exact fishing1 headless gate first. Promote only if a
key VBlank metric improves without regressing `blocking_vb`,
`prefetch_overrun_vb`, work identity, or correctness. Layout identity remains
mandatory for code-only experiments; deliberate pack-format experiments may use
`--allow-layout-change` only when the layout movement is documented and the
speed/work win is otherwise clean. Flat timing plus meaningful code-size/work
reduction is acceptable, but must not be counted as a speed win.

## Highest-Leverage Thesis

The remaining gap is not one single bottleneck. The current loop has visible
CD mostly contained to one VBlank on the setup-primed high-tide path, but that
came by moving read work into setup. The best path is parallel pressure on six
fronts:

| Front | Why It Can Still Move |
|---|---|
| Setup-prime/inter-scene preloading | `320 KB` priming proves residency can unlock catch-up; the setup cost must be hidden or generated per scene/tide. |
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

The current detail/trace samples shift priority again: `present_wait_vb=155`
against a remaining `131` VBlank active-loop overrun, while the FGP3 canary
has driven high-tide visible CD pressure to `0` VBlanks. The next major win
has to reduce or hide present wait and move setup-prime cost out of visible
scene startup without early display, tearing, frame drops, or weakened pause
input.

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
| 113 | Prepared-state detail counters | Done in Summary as `JCPERF2 sched`: prepared-ready/used/wasted plus CD-blocked prep and held-slice owners. | Fishing1 shows `72 ready / 72 used / 0 wasted`, so duplicate prepared-frame waste is not the current big win. |
| 114 | CD-first scheduler prototype | Refine from the first no-win ownership pass into a per-frame budget: read deadline, then precompose, then idle wait. | Prevents render prep from stealing CD slack while identifying which of the `574` wait slots and `28` CD-reserved slots can become useful work. |
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
| 151 | Generated setup-prime planner | Analyze every scene/tide pack for the smallest setup-prime byte window that covers useful early payloads. | Generalizes the fishing1 `320 KB` win without hard-coding one scene. |
| 152 | Inter-scene prime handoff | Start reading the next scene's prime window during the previous scene's dead/held tail when scene selection is known. | Converts the current setup-cost trade into a real end-to-end speed win. |
| 153 | Segmented prime window | Prime only the hot early and late FG2 spans instead of one contiguous first `N` bytes. | May reduce setup bytes while preserving the catch-up-safe coverage boundary. |
| 154 | Setup-prime low-tide sweep | Test low-tide fishing1 with generated prime sizes and keep threshold `5` unless coverage proves threshold `4` safe. | Prevents high-tide assumptions from leaking into the low-tide path. |
| 155 | Prime-size heap budget table | Emit per-scene largest-safe prime size after clean-rect allocation and runtime buffers. | Avoids memory regressions before trying all 63 scenes. |
| 156 | Prime-aware catch-up table | Emit frame ranges where threshold `4` is safe because all needed payload bytes are already resident. | Replaces the current scene-global setup-primed catch-up with frame-level proof. |
| 157 | Setup-cost gate | Add a harness mode that reports active-loop win, setup cost, and net scene cost separately. | Stops future preloading wins from accidentally hiding startup regressions. |
| 158 | Prime prefetch during title/menu | Investigate whether menu/transition time can warm the first scene's FG2 window before playback starts. | Converts cold-start setup reads into user-invisible work. |
| 159 | Cross-scene setup-prime matrix | Run fishing1/fishing2/fishing3 high/low with generated prime settings before main promotion. | Ensures the policy is not a fishing1-only trick. |
| 160 | Prime-plus-present scheduler | Use primed coverage to retry present/pipeline scheduling only inside proven-resident frame ranges. | Combines the current CD residency win with the remaining present-wait target. |
| 161 | FGP3 zero-shift residual pack | Encode fishing1 frames as previous-frame residuals where `dx=0,dy=0`. | Analyzer predicts this is the canary-safe temporal reuse path, unlike true translation. |
| 162 | FGP3 move/residual pack | Encode nonzero translation candidates only for walking scenes. | Analyzer proves walking packs, not fishing1, are the first real MoveImage targets. |
| 163 | Motion cleanup masks | Emit old-position cleanup bands for move/residual frames. | GPU move is unsafe unless old pixels are restored and dirty state remains exact. |
| 164 | RAM mirror motion proof | Prototype host-side replay that keeps RAM mirror and displayed image identical after move/residual frames. | Blocks runtime MoveImage until the mirror invariant is solved. |

## Impact-Prioritized Order

| Priority | Area | Tests | Why First |
|---:|---|---|---|
| 1 | Setup-prime and inter-scene preload | `151-160` | The latest accepted win proves residency unlocks threshold-`4` catch-up; the next step is making the prime free or generated, not hard-coded. |
| 2 | Temporal/motion FGP3 | `161-164` | The I-B analyzer shows fishing1 has a large zero-shift residual path, while true nonzero motion belongs to walking scenes. |
| 3 | Pack-emitted/read-costed CD groups | `11-18`, `34-35` | The hard-coded tail group proves selective grouping can remove reads, while the broad 12-sector import proves a cost predictor is mandatory. |
| 4 | Pack-emitted render/upload metadata | `61-64`, `81-90` | Runtime upload/compositor heuristics are locally exhausted; generated metadata can remove branches and preserve deterministic work identity. |
| 5 | Explicit scheduler/CD budget | `41-50` | Many near-misses were nominal wins that stole CD slack. A CD-first budget is the gate for retrying them safely. |
| 6 | Toolchain/layout control | `91-100` | Valid code-size cleanups still perturb hot phase. Layout control can unlock old no-promotion wins without changing pixels. |

## Next 100 Tests

| # | Area | Test | Why Now | Promote If |
|---:|---|---|---|---|
| 1 | Baseline | Add a release-speed run without `perf-log` and compare against the perf-log baseline. | We may be optimizing the logging build instead of the shipped runtime. | Release run is faster and remains visually/sound identical, creating a separate target baseline. |
| 2 | Baseline | Add a dual-baseline policy: diagnostic baseline and release baseline. | Future changes should be judged against the right runtime mode. | Harness records both without weakening the strict perf-log gate. |
| 3 | Harness | Add a non-promotable trace binary for per-read frame/slack class logging. | Inline CD histograms perturb the speed binary. | Trace build explains the remaining visible read and setup-prime ownership without changing the accepted binary. |
| 4 | Harness | Add host-side correlation from DuckStation read timestamps to frame indices. | Current CD log has LBAs but not the exact runtime frame owner. | We can prove the FGP3 zero-pressure path stays zero and name any setup-prime or cross-scene read that reintroduces pressure. |
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
| 1 | 1 | Establish whether the remaining `12.17%` is partly perf-log overhead. |
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
| VISITOR3 raw stream windows | Do not retry scalar window sizes; fresh-baseline high/low sweeps failed. Use generated grouping, direct16/selective preprocessing, or scheduler ownership. |
| BUILDING2 raw stream windows | Do not retry scalar window sizes. High regressed all tested sizes, and low's parameter-only `32 KiB` win failed as compiled default source. Use generated grouping or preprocessing instead. |
| BUILDING5 raw stream windows | Do not retry scalar window sizes. High and low both regressed total loop despite lower read counts; use generated grouping or preprocessing instead. |
| BUILDING-family raw stream windows | BUILDING4 and BUILDING6 high/low are accepted; retry only scene-locally with fresh baselines and bounded CD tradeoff rules, starting with remaining high-pressure building rows. |
| Smaller windows | Group metadata preserves due-frame coverage. |
| Prepared-frame cleanup | Explicit render/CD budget exists. |
| Direct-stage read-into-window | Group/tail-preserving merge keeps `blocking_reads=4`. |
| ACTIVITY10 low contiguous setup-prime | Generated segmented coverage or inter-scene preload exists; `304 KiB`/`288 KiB` zero-loop and `256 KiB` regresses loop/canary timing. |
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
| `grDrawBackground()` function `-Os` | Done; keep because scoped upload-function codegen stayed exact-flat and shrank ELF. |
| `grUpdateDisplay()` function `-Os` | Done; keep because scoped display-wrapper codegen stayed exact-flat and shrank ELF. |
| Reusable upload `RECT` stack storage | Done; keep because it removes unused per-call stack array storage with exact timing/upload identity. |
| Upload rect cap `6` after stack reuse | Do not promote from fishing1 alone; it was exact no-op on all tracked metrics and narrows cross-scene headroom. |
| Upload band byte arrays | Do not promote alone; it shrank `grDrawBackground` but grew the final ELF with no timing or work movement. |
| `grRestoreBgFromRects()` function `-Os` | Do not retry alone; it shrank the function but grew total ELF with no timing movement. |
| PAL4 compositor function-scoped `Os` | Do not retry; it shrank the compositor and loaded executable but regressed blocking even with foreground LBA restored. |
| Single-band narrow upload scratch | Do not retry as a special case; fishing1 upload bytes did not move and code grew. |
| Touched-only current dirty-row clearing | Done; keep because it removes per-frame row-table stores with exact timing/work identity and no new memory. |
| Touched-only dirty-row promotion | Done; keep because it removes full dirty-row table copies and produced a repeatable `2` VBlank loop win. |
| Direct PAL4 row dirty marking | Do not retry as written; it produced only target-accounting movement with flat loop speed and large compositor growth. |
| Dirty-row promotion overlap clear skip | Do not retry as an isolated branchy cleanup; exact-flat timing with ELF growth. |
| Post-dirty raw window retune | Do not retry raw `18-20 KB` windows blindly; `20 KB` regressed and the 9-sector rounded window shape crashed before metrics. Use generated group metadata/cost prediction first. |
| Aligned CD file-LBA cache | Done; keep because it shrank the active aligned read helper and ELF with exact timing/layout identity. |
| Aligned CD single-chunk fast path | Do not retry as a duplicated branch; it grew the helper by 104 bytes without moving timing. |
| Aligned CD helper function-scoped `Os` | Done; keep because it shrank the aligned-read path and ELF with exact timing/layout identity. |
| Buffered CD helper function-scoped `Os` | Do not retry alone; it grew the ELF and did not shrink the helper. |
| Fallthrough slack `5` after CD helper cleanup | Do not retry as a local guard change; it regressed `blocking_vb` and `prefetch_overrun_vb` to `10`. |
| Fallthrough slack `7` after CD helper cleanup | Do not retry as a local guard change; it failed before metrics with log overflow/regtest `137`. |
| Unbuffered CD helper function-scoped `Os` | Done; keep because it shrank the setup-facing stream helper and ELF with exact playback identity. |
| Unbuffered CD file-LBA cache | Done; keep because it shrank the setup-facing stream helper and ELF with exact playback identity. |
| `fgRuntimeFillWindowForEntry()` function-scoped `Os` | Do not retry alone; it was exact no-op on timing, size, and tracked symbols. |
| Prepared visual metadata decoupling | Do not retry as metadata-only; it adds duplicate probes and code growth without staging farther ahead. |
| Prepared visual stage-next branch | Retry only with an explicit no-slack guard and scheduler budget; v2 was correctness-clean and lowered read/late counters but left `loop_vb` flat. |
| Prepared visual `>=4` threshold | Do not retry as a threshold-only tweak; it failed structurally before metrics. |
| Prepared visual positive-slack stage-next branch | Do not retry as a local guard; it reproduced v2's flat timing and code growth. |
| FG2 read group `102..110` | Do not retry as a raw hard-coded group; it saved one read but regressed visible CD pressure to `8` VBlanks. Retry only with group-cost prediction or CD-first slack ownership. |
| FG2 read groups `384..396` and `307..317` | Do not retry as raw hard-coded groups. `384..396` did not fit/fire; `307..317` was exact-flat with code growth. Generated metadata must prove append fit and read-count movement first. |
| Direct stage into stream window | Do not retry as a local helper swap; it preserved reads but raised visible CD pressure by one VBlank. Retry only when group/tail metadata proves the direct-stage window stays hidden. |
| Hot whole-TU `-O3` | Function-scoped codegen or address padding preserves hot layout first. |
| Foreground pilot TU `-O3` | Do not retry as a whole-TU flag; it grew `foregroundPilotPlay` by about `5 KB`, moved the foreground pack three LBAs, and produced no key speed gain. |
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
