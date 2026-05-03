---
layout: page
title: History
eyebrow: 2025-10 to v0.6.5-ps1
subtitle: Pre-port era, first PS1 attempts, the hybrid pivot, the 63-scene grind. Quote dates where they exist.
description: Project history of the Johnny Castaway PS1 fan port — from the upstream jc_reborn engine decode through the hybrid host-and-replay pivot to v0.6.5-ps1.
---

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

The first phase was infrastructure. PSn00bSDK 0.24 was selected over
the official Sony SDK -- modern, open source, CMake-friendly. A
precompiled macOS toolchain was attempted and abandoned (missing
`cc1` / `cc1plus`); building from source needed Linux. The fix was
Docker: `Dockerfile.ps1` on `linux/amd64`, which works on Intel Mac,
Apple Silicon (Rosetta), Linux x86-64, and WSL2. Build configured
through CMake with PSn00bSDK's toolchain file; CD packaging via
mkpsxiso; one shell script (`build-ps1.sh`, later
`rebuild-and-let-run.sh`) on top.

Core implementation came next. The plan was: port only the platform
layer, leave the engine alone. That principle held: only three files
needed real porting -- `graphics_ps1.c`, `events_ps1.c`,
`sound_ps1.c` -- plus a CD I/O reimplementation in `cdrom_ps1.c`
(~2,280 lines, replacing `fopen` / `fread` against CD sectors).
Roughly 4,000+ lines of upstream engine code went unchanged. By the
end of Phase 4 the game booted on DuckStation and loaded resources
from the CD image.

What didn't work, in this era:

- **`vprintf` in hot paths was destabilizing.** Unbounded format
  buffers plus per-frame text I/O changed timing in ways that
  showed up as scene playback corruption. The interim answer was
  "visual debugging" -- colored pixels via `LoadImage`, the
  five-panel telemetry overlay, and `debugMode=0` to disable
  noisy text paths. Reliable TTY printf would not return until
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
binary (an **FG2 pack**: full-render base frames plus per-frame
indexed-pixel diff spans plus a sound-event table). Ship the packs
on the CD. On the PS1, replay the packs and own only the narrow
runtime surface: background, wave animation, holiday overlay,
controller input, SPU playback.

Internally this was called `fgpilot` (after the `foreground_pilot.c`
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

The same day, `FISHING 7` and `FISHING 8` were revalidated under the new
capture-position rule: controlled host/test placement can prove pack
completeness, but production runtime should stay random-position safe
unless a scene proves otherwise.

The remaining unvalidated scenes are queued. The per-scene workflow is
the same loop, repeated:

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
- **Holidays expansion.** From a small original set to **35
  holidays**, code-generated, with an emblem sprite sheet packed
  into the holiday overlay. Selectable from the pause menu and
  from `BOOTMODE.TXT`. Design notes in
  `docs/ps1/holidays-expansion-design.md`.
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
- **Story-loop walking.** `v0.4.20-ps1` made Johnny walk between
  scene endpoints instead of teleporting, with palm-tree occlusion,
  wave motion, holiday restamping, and a persistent walk-erase buffer
  that survived a long DuckStation soak.
- **Freeplay/debug mode.** `v0.5.0-ps1` made Johnny controllable.
  The mode launches from the pause menu, shows the original meanwhile
  frog during teardown/rebuild, lets the player walk with D-pad or
  analog, fish, clear the screen, and change world state immediately.
  The rest of the debug surface moved into menu catalogs: gags,
  visitors, sound effects, controls, world options, accessibility,
  and system pages. The important engineering rule was the same as
  the walking release: the steady-state frame loop does not allocate.

## Where it stands at {{ site.release.tag }}

- Build: **`{{ site.release.tag }}`**.
- Validated scenes: **{{ site.release.scenes_validated }} / {{ site.release.scenes_total }}**
  (`FISHING 1`, `FISHING 2`, `FISHING 3`, `FISHING 4`, `FISHING 5`, `FISHING 6`, `FISHING 7`, `FISHING 8`, `JOHNNY 1`, `JOHNNY 2`, `JOHNNY 3`, `JOHNNY 4`, `JOHNNY 5`, `JOHNNY 6`, `MARY 1`).
- Next scene in bring-up: **`MARY 2`**.
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
- PS-EXE size: **~84&nbsp;KB** after legacy ADS/TTM/FG1 paths
  were stripped.
- Routed CD image: **~9.9&nbsp;MB**.
- Generated FG2 corpus on disk: **126 packs** (high-tide +
  low-tide for all 63 scenes), **~343&nbsp;MB**, routed onto
  the CD image selectively.
- Real PS1 hardware: untested. Long-term list.

This isn't done. That's fine. A labor of love by Hunter Davis. The
original creator generously allows fan ports. If you paid for this,
you were cheated. Open source and free.
