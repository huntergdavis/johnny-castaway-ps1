---
layout: page
title: History
eyebrow: 2025-10 to v0.8.9-ps1
subtitle: Pre-port era, first PS1 attempts, the hybrid pivot, the 63-scene grind, the post-validation performance loop. Quote dates where they exist.
description: Project history of the Johnny Castaway PS1 fan port — from the upstream jc_reborn engine decode through the hybrid host-and-replay pivot to the post-validation performance baseline, the v0.8.4-ps1 chapter-select-thumbnail reconciliation, the v0.8.5-ps1 full headless matrix release, the v0.8.6-ps1 WALKSTUF1/VISITOR3 setup-segment compaction follow-through, the v0.8.7-ps1 deterministic boot and Scene Explorer preview stability release, the v0.8.8-ps1 VISITOR5 high performance release, and the v0.8.9-ps1 WALKSTUF1 low payload-reduction release.
---

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

## The pre-port era

Before this project there was an engine to decode. *Johnny Castaway*
shipped in 1992 as a Win16 screensaver: a small interpreter for two
custom bytecodes (ADS, scene selector; TTM, per-scene script) backed
by a single packed resource file (`RESOURCE.MAP` / `RESOURCE.001`).
The decode work that made any later port possible is upstream
[jno6809/jc_reborn](https://github.com/jno6809/jc_reborn), a clean
SDL2 reimplementation of the engine. Other lineage that fed in:
[Hans Milling's JCOS](https://github.com/nivs1978/Johnny-Castaway-Open-Source),
which extracted the WAV sound effects into a usable form, and
[Alexandre Fontoura's Castaway](https://github.com/xesf/castaway).
The Sierra Chest archive page kept the original screensaver and
documentation findable through 30 years of link-rot.

This project began as a branch of `jc_reborn` aimed at PlayStation 1.
A fuller version of the pre-port archaeology -- jno6809's commits, the
JCOS sound extraction, the abandoned Castaway port lineage -- lives at
[/archaeology/]({{ '/archaeology/' | relative_url }}). The short
version is: without that decode, this port would not exist, and the
license here (GPL-3.0) is inherited from there.

## First PS1 prototype attempts

The initial session was **2025-10-18**. The starting point was the
`4mb2025` branch of upstream `jc_reborn`, an aggressively memory-
optimized variant that had brought peak runtime usage down to about
**350&nbsp;KB** -- a number that was load-bearing for the PS1, since
the console's 2&nbsp;MB main RAM left ~1.65&nbsp;MB of headroom for
resource caching above the engine's working set.

The first phase was infrastructure. [PSn00bSDK]({{ '/docs/glossary/#psn00bsdk' | relative_url }}) 0.24 was selected over
the official Sony SDK -- modern, open source, CMake-friendly. A
precompiled macOS toolchain was attempted and abandoned (missing
`cc1` / `cc1plus`); building from source needed Linux. The fix was
Docker: `config/ps1/Dockerfile.ps1` on `linux/amd64`, which works on Intel Mac,
Apple Silicon (Rosetta), Linux x86-64, and WSL2. Build configured
through CMake with PSn00bSDK's toolchain file; CD packaging via
[mkpsxiso]({{ '/docs/glossary/#mkpsxiso' | relative_url }}); one shell script ([`build-ps1.sh`]({{ site.github_url }}/blob/main/scripts/build-ps1.sh), later
[`rebuild-and-let-run.sh`]({{ site.github_url }}/blob/main/scripts/rebuild-and-let-run.sh)) on top.

Core implementation came next. The plan was: port only the platform
layer, leave the engine alone. That principle held: only three files
needed real porting -- `graphics_ps1.c`, `events_ps1.c`,
`sound_ps1.c` -- plus a CD I/O reimplementation in `cdrom_ps1.c`
(~2,570 lines, replacing `fopen` / `fread` against CD sectors).
Roughly 4,000+ lines of upstream engine code went unchanged. By the
end of Phase 4 the game booted on DuckStation and loaded resources
from the CD image.

What didn't work, in this era:

- **`vprintf` in hot paths was destabilizing.** Unbounded format
  buffers plus per-frame text I/O changed timing in ways that
  showed up as scene playback corruption. The interim answer was
  "visual debugging" -- colored pixels via `LoadImage`, the
  [five-panel telemetry overlay]({{ '/docs/glossary/#telemetry-overlay' | relative_url }}), and `debugMode=0` to disable
  noisy text paths. Reliable [TTY]({{ '/docs/glossary/#tty' | relative_url }}) printf would not return until
  **2026-04-25**.
- **A faithful TTM/ADS interpreter on the PS1 produced
  "Johnny disappears" bugs that moved as fast as they were
  fixed.** The runtime was trusting cross-scene state that the
  desktop engine had built up across many scenes; CD latency
  (150ms cold seeks) plus the original engine's "replay prior
  scenes to establish state" model did not survive the port
  context. The bug class was real and recurrent; the fix class
  -- recovery heuristics, state injection, replay merge --
  produced incrementally smarter heuristics and incrementally
  shifted bugs.

The next two phases were performance and offline-restore. Phase 6
(2026-03) brought the rendering hot-path optimizations that still
ship: 4-bit indexed sprite format (`indexedPixels`, ~4x RAM savings
over 15-bit direct color), palette LUT compositing, 4-pixel unrolled
inner loop, opaque sprite fast-path, hash-based O(1) resource lookup
replacing O(N) `strcmp` scanning, and a dirty-rect system that cut
per-frame data movement by **80-95%**. Binary size came down from
~124&nbsp;KB to ~120&nbsp;KB; compositing was ~15-25% faster.

Phase 7 (2026-03, historical) tried to fix the disappearing-Johnny
class structurally rather than heuristically. The idea was offline-
authored *scene restore contracts*: analyze every scene offline,
declare which BMP/SCR resources it needs and which dirty regions it
expects to touch, compile that into a C header (`ps1_restore_pilots.h`,
~1,000 auto-generated lines), and have the runtime consume those
contracts instead of replaying prior scenes. This worked, partly. The
restore-pilot pipeline was real; 63 scene-scoped restore specs were
generated, compressed into 34 cluster contracts, and the live header
carried 26 active pilots. **25 of 63** scenes were verified under
that model on **2026-03-21**.

That number does not appear in current status. It was a different
definition of "verified" -- bring-up under the harness with no
disappearing Johnny on the canonical route -- and the harness itself
was later demoted as a primary acceptance gate. The
[`current-status.md`](https://github.com/{{ site.repo }}/blob/main/docs/ps1/current-status.md)
document calls out the older counts explicitly:

```
25 / 63   2026-03-21   Restore-rollout verified scenes
27 / 63   2026-03-21   Research-snapshot verified
63 / 63   2026-03-29   Harness-level validation claim
                       (retroactively demoted as a false summit)
57 / 63   2026-04-04   Scenes rendering with island content
60 / 63   2026-04-04..07  Bringup in the headless regtest surface
```

The `63/63` row is the relevant one: a harness pass declared all 63
scenes validated on **2026-03-29**, and that claim later collapsed
when human visual + audible signoff was applied to specific scenes
and revealed differences the harness had not been measuring. It is
preserved in the doc as a *false summit*. Each older count belongs
to a different definition of "verified"; none of them carry
forward.

## The hybrid pipeline pivot

The pivot happened in **2026-04**, in two beats. The first beat was
that a fully working scene needed a real acceptance bar -- not a
harness count, not a "renders something on the island" check, but
**pixel-perfect visuals plus synced SFX, signed off across every
applicable variant by human visual + audible review**. The second
beat was that meeting that bar through a runtime TTM/ADS
interpreter, even with offline restore contracts, was not the
straightest path.

The straighter path was to stop interpreting on the PS1 at all. Run
the desktop engine -- the same engine, on a host that has gigabytes
of RAM and a real OS -- as the authoritative renderer. Capture every
visible foreground draw. Capture every `0xC051 PLAY_SAMPLE` opcode
the TTM interpreter fired. Encode the result into a small per-scene
binary (an **[FG2 pack]({{ '/docs/glossary/#fg2-pack' | relative_url }})**: full-render base frames plus per-frame
indexed-pixel diff spans plus a sound-event table). Ship the packs
on the CD. On the PS1, replay the packs and own only the narrow
runtime surface: background, wave animation, holiday overlay,
controller input, SPU playback.

Internally this was called [`fgpilot`]({{ '/docs/glossary/#fgpilot' | relative_url }}) (after the [`foreground_pilot.c`]({{ site.github_url }}/blob/main/src/foreground_pilot/foreground_pilot.c)
runtime); externally it is "PS1 scene playback." The decisive
property is that the PS1 stops carrying state that the desktop
engine built up across scenes. The disappearing-Johnny class doesn't
need a heuristic fix because the runtime no longer has the state in
which it lived.

## First scenes validated

`FISHING 1` was the first reference scene. **2026-04-22** was the
first human-signed pass under the full visual + SFX bar. The
release that captured it was **`v0.3.6-ps1`** (commit `f2737253`):
fishing1 pixel-perfect with full SFX across every applicable variant
-- night, low-tide, holiday, raft-stage. A prior visual-only release
**`v0.3.5-ps1`** (commit `9448d49f`) was superseded by it.

That milestone also drove the SPU bring-up. The VAG encoder
(`scripts/wav2vag.py`) and the SPU upload/playback path
(`sound_ps1.c`) had a long bug list that had to be cleared before
captured `PLAY_SAMPLE` events would fire correctly: shift-exponent
inversion, ADPCM nibble-pair order swapped, SPU DMA missing 64-byte
alignment, ADSR1 attack-rate orientation flipped, mute requiring
direct SPU register writes because `SpuSetCommonMasterVolume` is
not honored by DuckStation HLE. Commit `355227fa` is the canonical
reference for that work.

`FISHING 2` followed on **2026-04-23**. Same bar, same variants.
That was the second scene; the project's count of "fully validated"
became **2 / 63**.

On **2026-05-01**, `FISHING 3`, `FISHING 4`, and `FISHING 6` joined the
validated ledger. `FISHING 4` needed the original `LEFT_ISLAND` draw
offset restored in the fgpilot path; `FISHING 6` needed a terminal FGP3
cleanup repair for the final splash and pole residue. `FISHING 5`
remained blocked on shark cleanup residue. The current count became
**5 / 63**.

Later on **2026-05-01**, `FISHING 7` joined the validated ledger with a
single-position FG2 replay pinned to the host-captured island position
`x=3,y=9`. That was later superseded: on **2026-05-03** the pack was
rebuilt from a far-left full-frame foreground-only capture, stress-tested
at the far-left runtime position, and released back to normal random
island placement. The current count became **6 / 63**.

`FISHING 8` followed with the same initial captured-position replay rule,
then got the same far-left recapture and runtime-unpin treatment. The
current count became **7 / 63**.

## The 63-scene grind

`JOHNNY 1` then joined the ledger on **2026-05-02** after its
full-screen black-backdrop playback path fixed placement, scaling, and
clean-rect memory pressure. `JOHNNY 2` followed the same day after its
first-SOS-bottle capture was rebuilt at a pinned island position
(`x=-64,y=54`), lower-band keyed overlay cleanup removed bottle/feet
residue, and the island/SOS thought-bubble holds were redistributed.
The current count became **9 / 63**.

`FISHING 5` then cleared the shark-residue blocker on **2026-05-02**.
The fix was host-capture side: a full-frame keyed current-ledger overlay
removed stale full-host shark overpaint without masking current shark
pixels into outline-only frames. The current count became **10 / 63**.

`JOHNNY 3` followed on **2026-05-02**. A right-shift island-position
probe confirmed the full source pixels are present, so the scene stays
variable-position rather than joining the rare runtime hard-pin list.
The current count became **11 / 63**.

`JOHNNY 4` followed on **2026-05-03**. It used the same right-shift
host/test position as the first SOS-bottle scene so the bottle-message
pixels stayed in frame, but production placement remains variable. The
fix was capture-side: full-frame keyed foreground-only overlay removed
stale bottle overpaint and the blue-line artifact through the SOS bubble.
The current count became **12 / 63**.

`JOHNNY 5` followed the same day. Its host/test capture moved to
`x=80,y=54`, because the earlier bottle-message capture exposed the note
but clipped the thrown-bottle splash back in the water. The scene kept
normal production island placement: x=80 is capture evidence, not a
runtime pin. The same full-frame keyed foreground-only overlay removed
stale lower-band overpaint, and hold timing was moved onto the SOS note
bubble instead of the blank post-bubble rows. The current count became
**13 / 63**.

`JOHNNY 6` followed as another black-backdrop scene. It now uses the
same runtime classification as `JOHNNY 1`, so the PS1 does not paint
ocean/island behind the office daydream. The current count became
**14 / 63**.

`MARY 1` followed on the historical validation route
`x=-124,y=37`, raft-stage `5`. It passed visual and audible review
without a pack or runtime change. The current count became **15 / 63**.

`MARY 2` followed after a more interesting capture fix. The scene can
expose more than one screen width of island-relative action depending
on where the island sits: line, mermaid splash, boot throw, and lower
water cleanup all needed different host sightlines. The validated packs
stitch multiple foreground-only views into one scene-relative foreground
canvas, then inject the fish thought-bubble shell from full-host frames.
Far-right and true far-left stress runs passed. The count became
**16 / 63**.

`MARY 3` followed with a different failure shape: a large foreground pack
that needed far-right host capture, stale host capture surface invalidation,
and low-memory clean-snapshot relief on PS1.

`MARY 4` promoted the multi-view stitch from scene-specific fix to default
workflow. The exporter now captures normal, far-left, and far-right
foreground-only host views for new scene bring-up, then merges them into a
scene-relative canvas so action on both sides of the island survives random
runtime placement. The count became **18 / 63**.

`MARY 5` followed as a story-flag policy fix. The scene contains its own raft
art and begins behind a [frog-clock]({{ '/docs/glossary/#frog-clock' | relative_url }}) full wipe, so the PS1 route now clamps
generic raft state off for `NORAFT` scenes and skips the walk prelude for
`FIRST` full-wipe scenes. The count became **19 / 63**.

`MISCGAG 1` then validated cleanly after regenerating high and low tide packs
with the generic normal/far-left/far-right foreground-only multi-view stitch.

`MISCGAG 2` followed the same path. Its story flags do not randomize low tide,
but both high and low packs were regenerated for parity and the normal
high-tide/night route passed human visual + audible review. The count became
**21 / 63**.

`STAND 1` followed with a much shorter idle-loop validation. Its high and low
packs were regenerated through the same stitch, and the resulting 35-frame /
169-vblank no-SFX loop matched the user's visual review. `STAND 2` through
`STAND 12`, plus `STAND 15` and `STAND 16`, then passed the normal
high-tide/night route on **2026-05-04**.

`SUZY 1` followed after a backdrop-classification fix. The source scene
uses a separate beach screen, so the PS1 runtime now loads `SUZBEACH.SCR`
for SUZY scenes instead of painting the standard island/ocean background
behind the foreground pack.

`SUZY 2` followed after the raft was classified as scene-local static art.
The foreground pack now includes `MRAFT.BMP` static-base pixels so Johnny
rides the raft instead of floating, and the SFX mixer leaves headroom for
the overlapping raft samples.

`VISITOR 1` followed through the now-standard multi-view stitch. The
Lilliputian arrival validated with a wide scene-relative foreground union
and one captured SFX event. The current count is **38 / 63**.

The same day, `FISHING 7` and `FISHING 8` were revalidated under the new
capture-position rule: controlled host/test placement can prove pack
completeness, but production runtime should stay random-position safe
unless a scene proves otherwise.

By `v0.7.0-ps1` (2026-05-05) every routed scene was signed off:
**{{ site.release.scenes_validated }} of {{ site.release.scenes_total }}**.
The per-scene workflow stayed the same loop, repeated 63 times:

```
1. capture-host-scene.sh          (host capture, high tide + low tide)
2. export-scene-foreground-pilot  (build the FG2 pack from the capture)
3. wire it into cd_layout.xml     (route the pack onto the CD image)
4. wire it into foreground_pilot.c (route the scene at runtime)
5. make-cd-image + rebuild-and-let-run
6. human visual + audible signoff
7. update scene-status.md, commit
```

There is no shortcut and no batch promotion. Each scene needs one
high-tide capture and one low-tide capture, each capture needs
to produce a pack the PS1 can replay, and each replay needs a human
review that signs off pixels and audio. Milestone releases are cut
every 10 newly validated scenes; smaller stability releases happen
between milestones.

The phases that lived alongside the grind, recently, are the
component-completeness phases:

- **Closed captions.** Pause -> Accessibility -> Captions: ON. A dark
  band at the bottom of the frame for ~5 seconds at scene start
  with descriptive subtitle text. Corpus from the upstream
  `closed_captions` branch of `jc_reborn`. The original sequential
  ADS-tag map had ~20 mismatches; a content-driven re-audit
  (`docs/ps1/caption-audit-2026-04-26.yaml`) replaced it.
- **Holidays expansion.** From the original four (Christmas, New
  Year, Halloween, St. Patrick's) to **36 holidays**, code-generated,
  with an emblem sprite sheet packed into the holiday overlay.
  Selectable from the pause menu and from `BOOTMODE.TXT`. Design
  notes in `docs/ps1/holidays-expansion-design.md`.
- **Pause menu locked design.** Start opens an overlay mid-scene,
  custom embedded 8x8 ASCII font (because PSn00bSDK's `FntFlush`
  is empirically broken in the scene-runtime context),
  `POLY_F4` dim quad. By `v0.5.0`, the menu is split into compact
  sub-screens: Freeplay Options, World Options, Accessibility,
  Sound Test, System, and the date/island/seed editors.
- **Memcard persistence.** Pause-menu choices save to `bu00:`
  block 0; restore on boot. Edge cases on a fresh / formatted
  card are still under iteration.
- **TTY printf restored.** **2026-04-25**: bounded `vprintf`
  plus DuckStation TTY/file logging restored gated `printf()`
  for setup/teardown probes. The `JCSPI`, `JCPAD`, `JCPERF`
  prefixes downstream tools key off were introduced here.
- **SPI pad driver.** Lifted from spicyjpeg's `pads` example
  because the BIOS pad path (`InitPAD` / `StartPAD`) was
  unusable in PSn00bSDK 0.24 + DuckStation. One change from the
  reference: poll `tx_len` is **5**, not 4. DuckStation only
  delivers button bytes when the full 5-byte sequence comes
  from the TX buffer.
- **[Story-loop walking]({{ '/docs/walks/' | relative_url }}).** `v0.4.20-ps1` made Johnny walk between
  scene endpoints instead of teleporting, with palm-tree occlusion,
  wave motion, holiday restamping, and a persistent walk-erase buffer
  that survived a long DuckStation soak.
- **[Freeplay/debug mode]({{ '/docs/glossary/#freeplay' | relative_url }}).** `v0.5.0-ps1` made Johnny controllable.
  The mode launches from the pause menu, shows the original meanwhile
  frog during teardown/rebuild, lets the player walk with D-pad or
  analog, fish, clear the screen, and change world state immediately.
  The rest of the debug surface moved into menu catalogs: gags,
  visitors, sound effects, controls, world options, accessibility,
  and system pages. The important engineering rule was the same as
  the walking release: the steady-state frame loop does not allocate.

## Where it stands at {{ site.release.tag }}

- Build: **`{{ site.release.tag }}`**.
- Validated scenes:
  **{{ site.release.scenes_validated }} / {{ site.release.scenes_total }}** —
  every routed scene the original game had, signed off across every
  applicable variant (night palette, low tide, holiday overlay,
  raft-stage progression). The live ledger is at
  [/scenes/]({{ '/scenes/' | relative_url }}).
- Performance baseline:
  **{{ site.release.perf_target_speed_pct }}%** target speed across
  the 126 timing-bearing scene/tide rows on the headless-perf
  battle card. The retrospective is at
  [/lab/from-87-to-99-5/]({{ '/lab/from-87-to-99-5/' | relative_url }}).
- Current mainline performance: the post-v0.8.7 optimization arc now includes
  VISITOR3 high's frame137 sector-203 setup relocation, VISITOR5 high's
  `30..46` retained-read group, VISITOR5 low's matching `30..46` retained-read
  group, VISITOR3 low's frame132 setup-prime
  relocation, WALKSTUF1 low's `78..91` retained-read boundary, BUILDING2
  low's previous-frame D4 deltas, BUILDING2 low's `218..229` slack-8 row and
  v739 draw-tail trim,
  VISITOR3 high's `277..293` tail-pack repack, BUILDING4 low's offscreen
  draw-span clipping, and WALKSTUF1 high/low late-tail work-volume clips. The
  public battle card is `+0.2708%` over target /
  `99.7337%` target speed; raw signed rollup is `-0.4963%` / `100.5160%`.
  VISITOR3 high measures `1063/1040` with `blocking_vb=35`; BUILDING4 low is
  `2853/2816` with `blocking_vb=40`;
  WALKSTUF1 low/high, BUILDING2 high/low, VISITOR3 high/low, JOHNNY1 low/high,
  and BUILDING4 low remain the under-99 tactical queue.
- Performance-baseline release: **`v0.8.0-ps1`** — promoted the
  headless optimization methodology to a release baseline; routed
  all 126 high/low scene variants through the perf matrix; clean-
  memory-relief drop-prefetch turned the post-validation perf
  arc from `+17.4%` over target to `+0.9%` over target.
- Latest performance release: **`v0.8.9-ps1`** — VISITOR5 low `30..46`
  retained-read promotion, BUILDING2 low `218..229` slack8 plus v739 draw-tail
  trim, VISITOR3 high `277..293` tail-pack repack, BUILDING4 low offscreen
  draw-span clipping plus frame291 in-place shrink, WALKSTUF1 high/low
  same-speed work-volume clips, and WALKSTUF1 low frame
  `51`/`49`/`47`/`61`/`62`/`58`/`45`/`37`/`35` in-place payload reductions.
  Public rollup is `+0.2708%` over target / `99.7337%` target speed; raw
  signed rollup is `-0.4963%` / `100.5160%`.
- Previous performance release: **`v0.8.8-ps1`** — VISITOR5 high `30..46`
  retained-read promotion. Public rollup is `+0.2867%` over target /
  `99.7183%` target speed; raw signed rollup is `-0.4805%` /
  `100.5006%`.
- Latest stability release: **`v0.8.7-ps1`** — deterministic BOOTMODE
  scene selection, expected-scene gates in the headless perf harness,
  Suzy backdrop cleanup hardening, and heapless Scene Explorer thumbnail
  streaming. Public rollup remains `+0.3156%` over target /
  `99.6902%` target speed.
- Earlier performance release: **`v0.8.6-ps1`** — WALKSTUF1 low
  gap6-prefix + slack-guard promotion, WALKSTUF1 high window-prefetch /
  slack4 guard, and VISITOR3 high/low setup-segment resident copies for
  frames `131` / `128`. Public rollup `+0.3157%` over target /
  `99.6902%` target speed.
- Previous performance baseline: **`v0.8.5-ps1`** — promoted the full
  126-row timing-bearing headless matrix as the public release baseline.
- Latest content release: **`v0.8.4-ps1`** — custom on-PS1 thumbnails
  for all 63 [chapter-select]({{ '/docs/glossary/#scene-explorer' | relative_url }}) grid slots, with scene titles and bodies
  reconciled against the on-PS1 packs. The earlier caption-mapping
  audit got several scenes mismapped (boot / octopus / coconut-plane /
  jog) — corrected from direct on-PS1 observation. New 5-surface
  helper at [`scripts/apply-scene-correction.py`]({{ site.github_url }}/blob/main/scripts/apply-scene-correction.py). No perf or pack
  content changed.
- Previous performance release: **`v0.8.3-ps1`** — promoted the
  WALKSTUF1 compact FGP3/v4 foreground packs; the 120 timing-
  bearing rows now average slightly under target at
  `{{ site.release.perf_target_speed_pct }}%` target speed.
- Earlier performance release: **`v0.8.2-ps1`** — promoted the
  VISITOR3 guarded-read path and kept the 63-scene validation bar
  intact while the headless battle card stayed near target.
- Latest stability release: **`v0.8.1-ps1`** — clean-rect
  pressure estimator extended to include the ocean wave band
  and upper/lower split rects before allocation, fixing a
  long-run scene-load freeze surfaced during randomized
  DuckStation soak (root cause: `MARY 4` random load).
- Last pre-v0.8.0 stability release: **`v0.7.2-ps1`** — backdrop-
  key guard so story-loop walks only run when the next scene's
  background state matches the previous rendered tide / raft /
  night / holiday / island position.
- Freeplay/debug release: **`v0.5.0-ps1`** -- direct-control Johnny,
  pause-menu debug catalogs, frog loading transitions, and a
  no-allocation steady-state freeplay loop.
- Walking release: **`v0.4.20-ps1`** -- story-loop Johnny walking
  between scene endpoints.
- Last loop-stability release before the walking/freeplay milestones:
  **`v0.3.9-ps1`** (commit
  `111efa9f`) -- the fishing3 overnight loop-stability release;
  confirms the current runtime can run long sessions without the
  prior scene-to-scene leak.
- [PS-EXE]({{ '/docs/glossary/#ps-exe' | relative_url }}) size: **~208&nbsp;KiB** (104 × 2&nbsp;KiB CD-ROM
  sectors) at `{{ site.release.tag }}` after legacy ADS/TTM/FG1
  paths were stripped from the linker pass.
- Routed CD image: **~79&nbsp;MB** at `{{ site.release.tag }}`
  (mostly FG2 pack payload routed selectively onto the disc).
- Generated FG2 corpus on disk: **126 packs** (high-tide +
  low-tide for all 63 scenes), **~343&nbsp;MB**, routed onto
  the CD image selectively.
- Real PS1 hardware: smoke-tested on a `SCPH-7501` via the
  [TonyHax](https://github.com/socram8888/tonyhax) softmod path.
  Long-term hardware-soak observations are still on the wishlist.

## Related pages

- [Method]({{ '/about/method/' | relative_url }}) — the *what* and
  the *why* of the hybrid pipeline this history walked toward.
- [Status]({{ '/about/status/' | relative_url }}) — the current
  per-component state at `{{ site.release.tag }}`.
- [Releases]({{ '/releases/' | relative_url }}) — every tagged
  version with theme line and headline bullets.
- [Lab: the 63-scene grind]({{ '/lab/the-63-scene-grind/' | relative_url }})
  — magazine treatment of the validation arc that runs from
  *First scenes validated* through the v0.7.0 cap.
- [Lab: from 87 to 99.5]({{ '/lab/from-87-to-99-5/' | relative_url }})
  — the post-validation performance loop that defined the
  v0.8.0 baseline above.
- [Lab: v0.8.1 — what the soak found]({{ '/lab/v081-mary4-freeze/' | relative_url }})
  — the stability follow-on that motivated the latest line.
- [Lab: the chapter-select grind]({{ '/lab/chapter-select-grind/' | relative_url }})
  — the v0.8.4-ps1 retrospective on walking all 63 packs again
  on hardware to ship custom Scene Explorer thumbnails, plus
  the [caption-mismap]({{ '/docs/glossary/#caption-mismap' | relative_url }})
  reconciliation (boot / octopus / coconut-plane / jog and the
  rest) the body of this page covers as the v0.8.4 milestone.
- [Lab: 63 heroes]({{ '/lab/63-heroes/' | relative_url }})
  — retrospective on the per-scene captured-on-PS1 hero rollout
  that put a unique social card on every per-scene page during
  the same v0.8.4-ps1 era.

This isn't done. That's fine. A labor of love by Hunter Davis. The
original creator generously allows fan ports. If you paid for this,
you were cheated. Open source and free.
