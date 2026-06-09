---
layout: page
title: Method
eyebrow: Host-capture · FG2 pack · PS1 replay
subtitle: How a 1992 Windows screensaver ends up running on a 1994 console without an emulator under it.
description: Technical deep-dive on the Johnny Castaway PS1 port — hybrid host-capture pipeline, FG2 pack format, and the PS1 hardware constraints that shaped both.
---

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

## The problem

Sierra's *Johnny Castaway* shipped in 1992 for Windows 3.1. It is a
screensaver. The engine is a small interpreter for two custom
bytecodes -- ADS, which selects scenes, and TTM, which scripts the
per-scene drawing -- backed by a flat resource pack
(`RESOURCE.MAP` / `RESOURCE.001`) of compressed bitmaps and screens.
On modern hardware the engine has been decoded, cleaned up, and made
portable; the upstream that this project descends from,
[jno6809/jc_reborn](https://github.com/jno6809/jc_reborn), runs on top
of SDL2.

A PS1 port is not "swap SDL2 for [PSn00bSDK]({{ '/docs/glossary/#psn00bsdk' | relative_url }})." Three things make a
straight port impossible:

1. **No comparable graphics pipeline.** The PS1 GPU does not draw
   into a CPU-addressable framebuffer. It pushes commands through an
   ordering table; sprites live in VRAM as 4-bit or 8-bit
   CLUT-indexed textures; "compositing a frame" means submitting
   `SPRT` and `POLY_FT4` primitives in z-sorted order. Sierra's TTM
   ops were authored for a flat 8-bit framebuffer with `SAVE_ZONE`
   / `RESTORE_ZONE` semantics, not for a tile-and-blit GPU.
2. **2&nbsp;MB of system RAM and no filesystem cache.** The desktop
   build trusts the OS to keep recently-read resources warm. The PS1
   has 2&nbsp;MB total, a 2x CD drive (~300&nbsp;KB/s, 150ms cold
   seek), no disk cache, no virtual memory, and no syscall layer.
   Anything you want quickly has to already be in RAM. Anything large
   has to be paged off the disc deterministically.
3. **SDL2 is not portable to the PS1.** PSn00bSDK is the modern
   open-source SDK; it gives you `psxgpu`, `psxcd`, `psxspu`,
   `psxapi`, `psxgte`, `psxsio`. There is no `printf` you can trust
   in a hot loop, no `malloc` worth leaning on for transients, no
   `pthread`, no `clock_gettime`. SDL's design assumptions are not
   even close. The symbol-by-symbol mapping the [host build]({{ '/docs/glossary/#host-build' | relative_url }}) hands
   off to the PS1 build is documented at
   [/docs/api/]({{ '/docs/api/' | relative_url }}).

The first prototype tried to brute-force these problems in the runtime
-- a faithful TTM/ADS interpreter on the PS1, replaying scenes from
extracted resources, recovering disappearing actors with heuristics.
It mostly worked. It also produced "Johnny disappears" bugs that moved
around as fast as they could be fixed, because the runtime was
trusting state that the desktop engine had built up across many
scenes. That class of bug is what motivated the pivot.

## The hybrid pipeline

```
desktop host  ---->  capture-host-scene.sh  ---->  high/low frames + frame-meta JSONs + sound-events.jsonl
                                  |
                                  v
            export-scene-foreground-pilot.sh
                                  |
                                  v
            build-scene-foreground-pack.py
                                  |
                                  v  (FG2: pal4/indexed8 spans + sound-event table)
               generated/ps1/foreground/*.FG2 ----> CD image ----> PS1
                                                                |
                                                                v
                                              foreground_pilot.c (replay)
                                                                |
                                                                v
                                             sound_ps1.c soundPlay() on cue
```

Each stage:

- **Host capture.** The desktop build (the same code, with the SDL2
  backend) is asked to play one ADS+tag scene under controlled boot
  state -- tide, night, holiday, raft-stage. It writes a high-tide
  capture and a low-tide capture: a sequence of full PNG frames, one
  per displayed game frame, plus a `frame-meta.json` with timing data
  and a `sound-events.jsonl` containing every `0xC051 PLAY_SAMPLE` op
  the TTM interpreter fired, with frame index, sound number, and
  pan / volume. This is run by [`scripts/capture-host-scene.sh`]({{ site.github_url }}/blob/main/scripts/capture-host-scene.sh).
- **Pack compile.** [`scripts/build-scene-foreground-pack.py`]({{ site.github_url }}/blob/main/scripts/build-scene-foreground-pack.py) turns a
  capture into one FG2 binary. Each output frame becomes either a
  base full-render (the first frame, or any frame the differ flagged
  as a forced base) or a diff-from-prior. Diffs are stored as runs
  of indexed-pixel spans: `(y, x_start, length, indexed_bytes)`.
  Sound events fold into a per-frame event table.
- **CD packaging.** [`mkpsxiso`]({{ '/docs/glossary/#mkpsxiso' | relative_url }}) consumes [`config/ps1/cd_layout.xml`]({{ site.github_url }}/blob/main/config/ps1/cd_layout.xml)
  and lays out the disc image. Each routed scene contributes two
  pack entries (high-tide, low-tide) plus the small fixed payload
  -- the executable, `RESOURCE.MAP`/`.001` for static metadata
  lookup, the SCR/PSB/SND assets the runtime still consults, and
  the title raw.
- **PS1 replay.** [`src/foreground_pilot/foreground_pilot.c`]({{ site.github_url }}/blob/main/src/foreground_pilot/foreground_pilot.c) opens the matching pack
  for the selected scene+tide, decodes its header and per-frame
  index, and during the scene loop stamps each frame's diff spans
  on top of the prior composite. Background, wave animation, and
  holiday overlays come from the PS1's own narrow runtime;
  sound events fire through [`src/platform/ps1/sound_ps1.c`]({{ site.github_url }}/blob/main/src/platform/ps1/sound_ps1.c) on a per-pack event
  cursor with a fixed 3-frame delay so SPU key-on lines up with
  the visible trigger.

This is the right shape because everything that needs the desktop
engine's full state (scene continuity, replay state, the ADS
selector) happens once at capture time on a 64-bit machine with
gigabytes of RAM. Everything that needs to be cheap on the PS1
(memory access, sprite stamping, audio key-on) is one straight pass
through a small, deterministic file.

## What's in a pack

An `FG2` pack -- e.g. `FISHING_1.FG2` or `FISHIN_L1.FG2` for the
low-tide twin -- is a flat little-endian binary. The relevant
research notes are in
[`docs/ps1/research/PACK_PAYLOAD_LAYOUT.md`](https://github.com/{{ site.repo }}/blob/main/docs/ps1/research/PACK_PAYLOAD_LAYOUT.md)
and
[`docs/ps1/research/PACK_MANIFEST_SCHEMA.md`](https://github.com/{{ site.repo }}/blob/main/docs/ps1/research/PACK_MANIFEST_SCHEMA.md).

Concretely, a pack contains:

- **Header.** Magic bytes, format version, frame count, base-frame
  count, palette size, indexed-bit-depth flag (`pal4` for 4-bit /
  16-color CLUT, `indexed8` for 8-bit / 256-color), high-or-low
  tide marker, source-scene identifier (ADS family + tag), and the
  byte offsets of the entry table, palette, base-frame block, and
  diff block. Sector alignment is 2048 bytes so each block lands on
  a CD-ROM sector boundary -- the loader can read what it needs
  without straddling sectors and forcing extra seeks.
- **Palette.** A single CLUT for the whole scene, packed as PS1
  16-bit [BGR-1555]({{ '/docs/glossary/#rgb555' | relative_url }}) entries. The host-side capture is constrained to a
  scene-stable palette so the pack does not have to re-upload CLUTs
  per frame.
- **Entry table.** One row per displayed frame. Each row is a fixed
  struct: frame index, source kind (`base` or `diff`), block offset,
  block length, and the frame's intended display duration in
  60ths-of-a-second ticks. The entry table is what the replay loop
  walks; the diff/base blocks are loaded lazily.
- **Base frames.** Full-render, indexed-pixel grids covering the
  scene's authored compose region (not the whole 640x480 -- only the
  rectangle the foreground actually touches). One base at the start
  of the scene; a small number of forced bases mid-scene where the
  differ found a discontinuity it didn't want to encode.
- **Diff frames.** Run-length-encoded spans of changed indexed
  pixels, addressed against the rectangle the prior frame
  established. Each span: `(row, x_start, run_length, bytes)`. A
  PS1 frame typically spends 80-95% fewer pixel-writes than a full
  redraw because most of a Castaway frame is unchanged background
  ocean and unchanged island.
- **Sound-event table.** Per-frame list of `(sound_number, pan,
  volume)` triples lifted from the captured `PLAY_SAMPLE` events.
  This is what `foreground_pilot.c` uses to fire `soundPlay()` on
  cue.
- **Frame-meta tail.** Source frame timing in milliseconds, used at
  capture-validate time and preserved in the pack so a [regtest]({{ '/docs/glossary/#regtest' | relative_url }}) can
  confirm the on-PS1 cadence still matches the host capture.

The runtime also carries a small companion JSON sidecar (`pack_index`
on the host side) used by the regtest harness, but on the disc the
runtime only needs the binary. There is one pack per scene per tide,
so the routed disc image carries up to 126 packs (`63 x 2`) -- the
generated FG2 corpus is roughly 343&nbsp;MB, which is why packs are
routed onto the CD selectively rather than all at once during
bring-up.

## PS1 hardware constraints we hit

These are the gotchas that actually cost wall-clock days. Most of
them are in
[`docs/ps1/hardware-specs.md`](https://github.com/{{ site.repo }}/blob/main/docs/ps1/hardware-specs.md)
or the dated worklogs under
[`docs/ps1/research/`](https://github.com/{{ site.repo }}/tree/main/docs/ps1/research).

**SPI pad polling needs `tx_len=5`, not 4.** PSn00bSDK 0.24's BIOS
pad driver (`InitPAD` / `StartPAD`) does not auto-poll under
DuckStation in the project's runtime context. The fix was to lift
the SPI controller driver from spicyjpeg's `pads` example and run
it directly: timer-2 plus SIO0 IRQ at 250&nbsp;Hz, in [`src/platform/ps1/spi.c`]({{ site.github_url }}/blob/main/src/platform/ps1/spi.c).
That driver, as published, sends a 4-byte poll TX. Under DuckStation
the controller bytes never make it back; the read returns
`0xFFFF`. The console only delivers button bytes when the full
5-byte poll sequence comes from the TX buffer. Bumping `tx_len`
from 4 to 5 made the controller work. This is documented in
`docs/ps1/hardware-specs.md` and pinned in the project's working
notes. If you copy the spicyjpeg driver, change `tx_len`.

**`FntFlush` is empirically broken in the scene-runtime context.**
The pause menu needed on-screen text. The PSn00bSDK font path
(`FntLoad` / `FntPrint` / `FntFlush`) accepts the calls without
error but produces no visible pixels in the running scene context
-- primitives accumulate in the OT and never present. Rather than
chase the root cause through PSn00bSDK internals, the pause menu
ships a custom embedded 8x8 ASCII font, drawn with `POLY_F4` glyph
quads on the same OT as the scene. Captions reuse that same font
atlas. New on-screen text should not regress to `FntFlush`.

**VRAM corruption across scenes -- `grRestoreBgTiles` wipes
`currDirty`.** The dirty-rectangle bookkeeping in
[`src/graphics_ps1/graphics_ps1.c`]({{ site.github_url }}/blob/main/src/graphics_ps1/graphics_ps1.c) tracks per-frame dirty regions in `currDirty`
and `prevDirty`. On a normal frame, `currDirty` is the spans the
foreground touched this frame; `prevDirty` is what it touched last
frame and now needs background restoration. The pause menu opens
mid-scene, dims everything, and on resume needs the entire scene
to redraw cleanly. The first attempt called `grRestoreBgTiles()` on
resume, which uses `currDirty` to know what to restore -- but
`grRestoreBgTiles` itself wipes `currDirty` as it goes. A full
redraw on resume needs both `prevDirty` and `currDirty` honored,
which is why the codebase now exposes `grForceFullRedrawNextFrame()`
to flag the next frame as a forced full background restore. This
is pinned in the project's memory and showed up multiple times
during pause-menu bring-up.

**SPU HLE vs hardware divergence under DuckStation.**
`SpuSetCommonMasterVolume` is not honored by DuckStation's HLE
audio path. The pause menu's mute toggle had to be reimplemented
as a direct write to the SPU master-volume registers. This was
isolated during the `v0.3.6-ps1` audio bring-up, alongside a
batch of VAG-encoder bugs (`scripts/wav2vag.py`): inverted
shift-exponent, swapped ADPCM nibble pair order, missing 64-byte
SPU DMA alignment, ADSR1 attack-rate orientation. Audio-on-real-
hardware behavior is presumed-correct but unverified -- one of
the open items. See commit `355227fa` for the full bug list.

**[TTY]({{ '/docs/glossary/#tty' | relative_url }}) printf is the only real debug surface, and it has a price.**
For most of 2026-Q1 the project ran with `debugMode=0` and
"visual debugging" -- colored pixels via `LoadImage`, the
[five-panel telemetry overlay]({{ '/docs/glossary/#telemetry-overlay' | relative_url }}), gated [`JCPERF`]({{ '/docs/glossary/#jcperf' | relative_url }}) summaries during
scene transitions only. Per-frame `vprintf` was outright
destabilizing scene playback (unbounded format buffers, hot-path
text I/O changing timing). As of 2026-04-25, bounded `vprintf`
plus DuckStation TTY/file logging restored gated `printf()`
breadcrumbs for setup/teardown -- the `JCSPI`, `JCPAD`, `JCPERF`
prefixes downstream tools key off. It still must not be called
per frame; that's why `ps1_perf` is level-gated
(`OFF`/`SUMMARY`/`DETAIL`/`DEBUG`).

Other gotchas worth flagging in passing:

- The PSn00bSDK 0.24 toolchain runs in Docker on `linux/amd64`
  (`config/ps1/Dockerfile.ps1`). Native macOS toolchains were attempted and
  abandoned -- missing `cc1` / `cc1plus`, source builds need
  Linux. Docker was the cheapest path that worked.
- 4-bit indexed sprite format (`indexedPixels`) saved roughly 4x
  the RAM of the original 15-bit direct-color path, which is what
  let multi-sprite scenes fit in 2&nbsp;MB at all.
- Hash-based O(1) resource lookup replaced the original O(N)
  `strcmp` scan during the 2026-03 perf push. Worth ~15-25% of
  compositing time.
- BSS budget was held under ~57&nbsp;KB through development;
  `malloc` is used for transients rather than static arrays
  precisely because static arrays push BSS into the danger zone.

## Why hybrid won

The PS1 does not have to be smart. The host build does the smart
work -- runs the real engine, captures the real frames, encodes the
diffs, lays out the disc -- and the PS1 just plays back. That is
why 63 scenes can fit on a single CD-ROM at all, why the
executable is around 208&nbsp;KiB at `{{ site.release.tag }}` after
the dead ADS/TTM/FG1 paths were stripped (down from a much larger
pre-strip ELF), and why scene continuity bugs stopped being a
runtime concern: the
runtime no longer carries the state that those bugs lived in.

The cost is that every scene needs a verified host capture before
it joins the validated count. At `{{ site.release.tag }}` that
count is `{{ site.release.scenes_validated }} / {{ site.release.scenes_total }}` —
every routed scene the original game had now plays pixel-perfect
on the PS1 with synced SFX across every applicable variant. The
path from the first signed-off scene to all 63 was the same
repeatable loop on every row: capture, pack, route, replay, sign
off. The hard work was the loop's edges — multi-view foreground
stitches for the wide scenes, residual-cleanup pack fixes when a
few pixels missed, the backdrop-key guard that kept
[story-loop walks]({{ '/docs/walks/' | relative_url }})
from running across stale islands. That is the property
the project was reaching for.

The second bar, the
[performance battle card]({{ '/perf/' | relative_url }}), is its
own ledger. It moved from `+17.4%` over target / [`87.1%` target
speed]({{ '/docs/glossary/#target-speed' | relative_url }}) at the compact full-matrix baseline to
`{{ site.release.perf_target_speed_pct }}%` target speed at
`{{ site.release.tag }}` — closed without changing pixels, sound
event timing, scene identity, or long-run heap stability. The
[reference manual]({{ '/docs/performance/' | relative_url }})
explains what each column means; the
[retrospective]({{ '/lab/from-87-to-99-5/' | relative_url }})
walks through which experiments landed (FGP3 packs, scene-local
prefetch relief, stream-window retuning, padded residual packs,
scoped read groups) and which did not (`-O2`, naive read-group
probes); the
[v0.8.1 follow-on]({{ '/lab/v081-mary4-freeze/' | relative_url }})
documents the soak loop that catches what the per-commit matrix
doesn't. Visual signoff and headless perf stay separate ledgers
because their failure modes are uncorrelated; mixing them is how
regressions ship.

## Related pages

- [Development workflow]({{ '/docs/dev-workflow/' | relative_url }})
  — the author's per-scene runbook (capture, encode, replay,
  screenshot, validate); this method page is the *why*, that
  page is the *what to type*.
- [File formats]({{ '/docs/formats/' | relative_url }}) — the
  five formats this pipeline produces and consumes (FG2 pack
  payload, pack manifest, dirty-region template, transition
  prefetch schema, SDL compat lite).
- [Hardware]({{ '/docs/hardware/' | relative_url }}) — the PS1
  envelope (33.8688 MHz MIPS, 2 MB RAM, 1 MB VRAM, 512 KB SPU,
  2× CD) every constraint above traces back to.
- [Glossary]({{ '/docs/glossary/' | relative_url }}) — the
  technical vocabulary used throughout (ADS, TTM, FG2 pack,
  capture, replay, dirty-rect, FntFlush, FISHING 1 bar).
- [History]({{ '/about/history/' | relative_url }}) — the longer
  narrative version, dated, eras-and-milestones.
- [Status]({{ '/about/status/' | relative_url }}) — the
  component-level state at the current release.
- [Lab: the pivot that almost didn't happen]({{ '/lab/pixel-perfect-pivot/' | relative_url }})
  — magazine retrospective on the choice between "looks similar"
  and pixel-perfect-with-host-capture that defined the rest of
  this method. The decision behind every section above.
- [Lab: the 63-scene grind]({{ '/lab/the-63-scene-grind/' | relative_url }})
  — magazine treatment of applying this method to every routed
  scene, one capture-encode-replay-validate loop at a time. This
  page is the recipe; that essay is what running it 63 times
  actually looked like, including the last-cluster hard cases.
