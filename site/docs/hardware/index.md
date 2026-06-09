---
layout: page
title: PS1 hardware constraints
eyebrow: Reference
subtitle: What the original PlayStation actually gives you, and what bites in practice when you try to use it for a 1992 screensaver port.
description: PlayStation 1 hardware specs as they apply to the Johnny Castaway PS1 fan port — CPU, RAM, VRAM, SPU, CD drive — plus the gotchas the project hit in practice.
---

A labor of love by Hunter Davis. This page is a reference manual for the
hardware the PS1 port actually runs on — and a short list of the places where
that hardware drew blood during development. If you paid for this, you were
cheated. Open source and free.

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

## The shape of the machine in 2026

The PlayStation 1 was released in 1994. In 2026 it is a museum piece, but it is
also the machine that runs `jcreborn.bin` on real hardware and inside
DuckStation. The numbers that matter, in the order you usually trip over them:

- **CPU:** [MIPS]({{ '/docs/glossary/#mips' | relative_url }}) R3000A at 33.8688 MHz. 32-bit, no hardware floating point.
- **System RAM:** 2 MB. Not 2 GB. Not 2 hundred MB. Two megabytes.
- **[VRAM]({{ '/docs/glossary/#vram' | relative_url }}):** 1 MB, owned by the GPU. Two 640&times;480 framebuffers eat 600 KB
  of that on their own.
- **SPU RAM:** 512 KB, owned by the audio co-processor. ADPCM samples only.
- **CD drive:** 2x speed, 300 KB/s sustained, ~150 ms cold seek.
- **Controller:** SIO0 serial bus, polled at [vblank]({{ '/docs/glossary/#vblank' | relative_url }})-ish rates.

For *Johnny Castaway*, a 1992 16-color VGA screensaver running at roughly 4 fps
of foreground change, that's an embarrassment of riches in some axes (a 33 MHz
CPU never has to run anything close to a real game loop) and tight in others
(2 MB has to hold the whole executable, the resource cache, the prepared
foreground frames, and every per-frame buffer the renderer touches).

The author has spent more time fighting the I/O budget than the CPU budget.
The CD's 150 ms seek and the SPU's separate-RAM model are what shape the
runtime; the MIPS core itself is rarely the bottleneck. That distinction
matters when reading the rest of these docs — most of the perf work is about
hiding seeks, not making the CPU faster. See
[Performance work]({{ '/docs/performance/' | relative_url }}) for the
experiment ledger.

## Hardware reference

### CPU and memory

| Component        | Spec                                       |
|------------------|--------------------------------------------|
| CPU              | MIPS R3000A @ 33.8688 MHz                  |
| Floating point   | None — soft-float via library              |
| System RAM       | 2 MB                                       |
| VRAM             | 1 MB                                       |
| SPU RAM          | 512 KB                                     |
| BSS budget       | Keep below ~50 KB for stability             |

Compiler flags inherited from PSn00bSDK's toolchain pin
`-msoft-float -G0 -march=mips1 -mabi=32 -ffreestanding`. The project layers
its own `-O2 -Wall -Wpedantic -ffunction-sections -fdata-sections` on top and
links with `-Wl,--gc-sections` so unused engine paths get stripped at the
linker. After that pass `jcreborn.elf` is around 924 KiB; the converted
`jcreborn.exe` lands at 208 KiB (104 × 2 KiB CD-ROM sectors) at
`{{ site.release.tag }}`.

### Graphics (GPU + VRAM)

The GPU is fixed-function. No shaders. Sprites, lines, triangles, quads, and
circles, with a hardware ordering table (`OT`) for back-to-front sorting.
Color is either 4-bit indexed (16-color CLUT), 8-bit indexed (256-color
CLUT), or 15-bit RGB. *Johnny Castaway* is a 16-color VGA program, so almost
everything renders through 4-bit CLUT spans.

Native resolution for the port is 640&times;480 interlaced, which exactly
matches the source material. The 1 MB VRAM laid out as:

```
Region            Size     Location
-----------------------------------------
Framebuffer 0     ~300 KB  (0,0) – 640x480x16
Framebuffer 1     ~300 KB  (0,480) – 640x480x16
Sprite cache      ~350 KB  (dynamic)
CLUTs             ~50 KB   (palette tables)
-----------------------------------------
Total             1.0 MB
```

The "sprite cache" line is what does most of the actual rendering work — once
a foreground frame is composed in main RAM, it's uploaded into VRAM as a
texture and drawn from there. Sprite cache contention is an issue for large
scenes; for *Johnny Castaway*'s 16-color tiles it's mostly fine.

### Audio (SPU)

The SPU is a separate processor with its own 512 KB of dedicated RAM. It
plays 24 simultaneous voices of 4-bit ADPCM at up to 44.1 kHz, mixes them in
hardware, and supports per-voice ADSR and stereo panning. The CPU does not
mix audio; it uploads samples once and triggers them via voice keys.

For this project the entire sound bank — 23 effect samples, total ~95 KB
ADPCM — preloads into SPU RAM at boot. There is no streaming, no music. See
[Audio pipeline]({{ '/docs/audio/' | relative_url }}) for the SPU layout map.

### Storage (CD-ROM)

The CD is a 2x drive — 300 KB/s sustained, 150 ms cold seek. Both numbers
matter. The seek dominates for small reads; the bandwidth dominates for the
foreground packs (the largest active pack is ~1.75 MB). The runtime treats
the CD as an explicit prefetch surface, not a transparent filesystem; reads
are scheduled inside held VBlanks where possible, so seek latency lives
under already-idle frames.

### Input (controller)

Everything goes through the SIO0 serial bus. PSn00bSDK ships a BIOS pad
driver, but it does not auto-poll in the project's PSn00bSDK 0.24 +
DuckStation environment. The port does its own SPI driver in `src/platform/ps1/spi.c`,
adapted from spicyjpeg's PSn00bSDK example, polling at 250 Hz off Timer 2.

## What bit us in practice

Hardware specs are easy. The hard part is the half-dozen places where the
hardware (or the SDK on top of it) works in a way that is not obvious from
the specs and that costs days of debugging when first encountered.

### SPI pad polling needs `tx_len=5`, not `4`

The spicyjpeg PSn00bSDK `examples/io/pads/` reference polls the controller
with a 4-byte TX sequence. That works on hardware. On DuckStation it
returns `0xFFFF` — the button bytes are simply not delivered unless the
full 5-byte poll sequence comes from the TX buffer. The fix is one
constant; finding the constant took an evening of staring at SIO0 and
guessing whether the bug was in our code, the SDK, or the emulator. It was
in the emulator, and the emulator is what almost everyone playtests on, so
the fix is permanent. `src/platform/ps1/spi.c` carries `tx_len=5`.

### `FntFlush` is empirically broken in scene-runtime context

The PSn00bSDK font path (`FntLoad` / `FntFlush`) is a useful debug surface in
toy programs. In the scene-playback runtime context it is broken: text
primitives accumulate in the OT but produce no visible pixels, with no
diagnostic. The pause menu uses an embedded 8&times;8 ASCII font and
hand-drawn `POLY_F4` quads instead. Do not regress to `FntFlush` for
on-screen text without a fresh empirical test.

### SPU HLE diverges from real hardware

DuckStation's SPU emulation is HLE (high-level emulation) by default, and
`SpuSetCommonMasterVolume` is one of the calls it does not honor — writes
to the SPU master-volume register go through, but the documented PSn00bSDK
helper does not actually mute the bus. The pause menu's mute toggle writes
the SPU master-volume registers directly via `*(uint16_t volatile *)0x1f801d80`
to work around this. The author has not yet validated this on real
hardware; until that happens, treat the audio path on hardware as
"believed correct, not signed off."

### Dirty-rect bookkeeping eats `currDirty` on full restores

The graphics layer tracks dirty rectangles per tile so that only changed
rows are uploaded each frame. `grRestoreBgTiles` clears `currDirty` as a
side effect — which is correct for normal frames, but wrong for resume
paths where a full redraw needs the *previous* frame's dirty extents too.
After the pause menu lands, returning to scene playback uses
`grForceFullRedrawNextFrame` so `prevDirty` and `currDirty` agree on the
post-resume frame. This was a one-line fix that took three runs of
"why did the screen come back wrong?" to find.

### VRAM corruption when scenes don't restore the CLUT

The 16-color palette (`JOHNCAST.PAL`) is loaded once at startup and never
changes — but specific scenes can stomp on the CLUT region of VRAM during
upload if the source rectangle math is off by even a few pixels. The
visible symptom is a single corrupted color across every scene that runs
afterward, until the next clean palette upload. The fix is to gate every
`LoadImage` call against the known CLUT rectangle and to keep palette
uploads aligned to 16-pixel boundaries.

### [TTY]({{ '/docs/glossary/#tty' | relative_url }}) printf is the only debug surface, and it's a sharp tool

DuckStation's TTY logging works reliably as of 2026-04-25 — the project can
print from anywhere in the runtime and capture the output to a host log
file. But text I/O changes timing. A `printf` in a per-frame hot path will
move VBlank cadence enough to mask or invent the very bugs being debugged.
The project uses TTY for one-shot snapshots ([`JCPAUSE`]({{ '/docs/glossary/#jcpause' | relative_url }}), [`JCPERF2`]({{ '/docs/glossary/#jcperf' | relative_url }}),
legacy `JCPERF` when compile-enabled, [`JCPAD`]({{ '/docs/glossary/#jcspi-jcpad' | relative_url }}), `JCSPI`) and a colored on-framebuffer telemetry
overlay for steady-state visibility. See
[Performance work]({{ '/docs/performance/' | relative_url }}) for how the
perf module gates print levels.

### `vprintf` was unbounded, and that forced `debugMode=0`

Early in the port, the legacy debug path called `vprintf` from a hot loop
without bounding the format buffer. Under sustained text output the stack
would walk into BSS and the executable would crash several minutes into a
session, sometimes silently. The proximate fix was to disable
`debugMode` in release builds; the deeper fix was to replace `vprintf`
with the bounded `JCPERF2` gated formatter in `src/platform/ps1/ps1_perf.c`,
which truncate inside a static buffer. The lesson, repeated across this
project: standard C library functions assume a host-class environment,
and the PS1 is not that.

## Where this leaves the runtime

The current build (`{{ site.release.tag }}`) lives well inside every hardware
budget: ~208 KiB of executable code, ~95 KB of audio in SPU RAM, two
640&times;480 framebuffers, and a few hundred KB of resource cache in main
RAM. The bottleneck that remains is CD-side prefetch timing, not memory or
CPU. If a future scene exposes a CPU bound, the next move is per-scene
specialized compositors, not a clock change — the clock isn't going up.

## Related pages

- [Devices — what it runs on]({{ '/docs/devices/' | relative_url }})
  — the specific tested instances of the hardware envelope this
  page describes: DuckStation as the every-commit reference,
  SCPH-7501 via TonyHax as the smoke-tested real PS1, plus the
  should-work-unverified and unsupported lists.
- [Build & toolchain]({{ '/docs/build/' | relative_url }}) — the cross-compile
  setup that targets this hardware.
- [Build infrastructure]({{ '/docs/infrastructure/' | relative_url }}) — what
  the dev environment looks like beyond the toolchain.
- [Performance work]({{ '/docs/performance/' | relative_url }}) — the
  experiment ledger that lives on top of these constraints.
- [Performance battle card]({{ '/perf/' | relative_url }}) — live
  per-scene timing against the hardware budget described above.
- [Audio pipeline]({{ '/docs/audio/' | relative_url }}) — SPU layout and the
  HLE-vs-hardware divergence in detail.
- [API mapping (SDL2 → PSn00bSDK)]({{ '/docs/api/' | relative_url }})
  — the call-by-call surface between the host build and the
  hardware described here.
- [Lab: the two-day SPI bug]({{ '/lab/two-day-spi-bug/' | relative_url }})
  — the retrospective on the `tx_len=5` pad-polling fix the
  hardware section above documents in one paragraph.
- [Story-loop walks]({{ '/docs/walks/' | relative_url }}) — the
  walk subsystem's persistent clean buffer is a direct response
  to the 2 MB envelope above; the page documents why
  re-allocating per walk fragmented the heap.
- [Method]({{ '/about/method/' | relative_url }}) — how a one-person port
  decides what to ship.
- [Devlog]({{ '/devlog/' | relative_url }}) — day-by-day worklog where most of
  these bugs got triaged.
- [Glossary]({{ '/docs/glossary/' | relative_url }}) —
  definitions for hardware-specific terms used above
  (`SPU`, `VRAM`, `VBlank`, `OT`, `mkpsxiso`, `PSn00bSDK`,
  `TonyHax`, `SPI driver`, `tx_len`). Grouped by area, not
  alphabetical.

## View source on GitHub

- [`docs/ps1/hardware-specs.md`]({{ site.github_url }}/blob/main/docs/ps1/hardware-specs.md) — original design doc.
- [`src/platform/ps1/spi.c`]({{ site.github_url }}/blob/main/src/platform/ps1/spi.c) —
  the project's own SPI driver (250 Hz Timer 2 polling, `tx_len=5`
  for DuckStation parity). Carries the fix the
  [two-day SPI bug retrospective]({{ '/lab/two-day-spi-bug/' | relative_url }})
  walks through.
- [`src/platform/ps1/ps1_perf.c`]({{ site.github_url }}/blob/main/src/platform/ps1/ps1_perf.c) —
  bounded `JCPERF2` gated formatter; the safe substitute
  for per-frame `printf()` in timing-sensitive paths.
