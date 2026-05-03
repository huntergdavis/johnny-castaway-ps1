---
layout: post
title: "Ocean Ambience — v0.6.0-ps1 — May 1, 2026"
date: 2026-05-01 12:00:00 -0700
editor_note: "A CC0-sourced 20-second ocean loop on a dedicated SPU voice. Pause-menu toggle, memcard-persisted, zero CPU cost in steady state. The screensaver's per-VBlank budget is unchanged; the SPU plays + auto-loops in hardware while the screensaver loop runs as before."
source_path: docs/ps1/background-music-feasibility.md
description: "v0.6.0-ps1 ships a CC0 ocean ambience loop on a dedicated SPU voice. Pause-menu toggle, memcard-persisted, zero CPU cost in steady state."
---

Sierra's *Johnny Castaway* never had continuous music. The 1992 PC release
drove a few PC-speaker tones and short Sound-Blaster digital effects but
ran in silence the rest of the time. `v0.6.0-ps1` adds an optional
looping ocean track — the sound of the place we're showing — without
touching the screensaver loop's per-VBlank cost.


## Why ocean, and not music

The PS1 has three independent audio paths: the SPU (dedicated coprocessor
with 24 voices, 512 KB of its own RAM, and hardware loop flags built into
the ADPCM block format), CD-DA (Red Book audio decoded by the drive), and
CD-XA ADPCM (decoded by the drive too, interleaved with data sectors).
Each has trade-offs; all of them *could* play music. The reasons we
picked ocean ambience as the v0.6 ship instead of "background music" in
general:

- **Spectrum.** Ocean recordings live below ~1 kHz. Sony 4-bit ADPCM's
  quantization noise is loudest above ~4 kHz. On ocean material the
  compression is essentially transparent at 11.025 kHz mono — the
  tradeoff that *would* hurt a music track is invisible on this
  material.
- **Loop forgiveness.** Wave patterns are aperiodic. A loop seam on
  music with a clear meter is obvious; on ocean it disappears under the
  broadband noise. We didn't need the seamless-loop acrobatics that a
  music track would demand.
- **Reverb is the recording.** The SPU has a hardware reverb unit. Music
  typically benefits from it. Ocean ambience already *is* the spatial
  sound — we don't apply reverb on top, and we get the SPU RAM the
  reverb workspace would have used back.
- **Mood matches the screensaver.** A castaway on a tiny island, ocean
  playing quietly underneath. That's the fiction. Anything else would
  have been music laid on top of it.


## The mechanism: pre-loaded ADPCM in SPU RAM

Of the five PS1 audio paths, exactly one delivers **strict zero CPU per
frame**: pre-load an ADPCM-encoded sample into SPU RAM at boot, set the
loop flags on the first and last data blocks of the sample, key on a
dedicated voice. The SPU plays it forever in hardware. The CPU never
sees another byte of audio work.

Compared to the alternatives:

- **SPU streaming from CD** would let us fit hours of music in 16 KB of
  SPU RAM, but it costs ~10-20 cycles per chunk-finish IRQ plus steady
  CD bandwidth competing with the FG2 prefetch the screensaver loop
  already uses. Not zero CPU.
- **CD-DA (Red Book)** is gorgeous and free of CPU cost but the PS1
  drive can't read data and play CD-DA at the same time. Enabling it
  would kill the FG2 prefetch and break scene playback. Wrong fit for
  our streaming pipeline.
- **XA-ADPCM** interleaves audio with data sectors — hardware decode,
  zero CPU, compatible with prefetch. The right answer for "many
  tracks", but it adds `mkpsxiso` XA-track authoring complexity for
  capability v0.6 doesn't need.
- **Sequenced (SEQ/VAB / MOD)** gives the deepest expression but a
  sequencer ticks the CPU 1-3 ms per VBlank. Real per-frame cost.

Pre-loaded ADPCM in SPU RAM was the only choice that delivered the
zero-CPU contract without changing the disc authoring pipeline.


## The loop: 20 seconds, crossfade-replace at the seam

The ocean recording is from [BigSoundBank sound 0266 — "Sea: Waves"](https://bigsoundbank.com/sea-waves-s0266.html),
CC0 public domain. Moderate waves, swirls, no birds, no breakers. 57
seconds of source material at 44.1 kHz stereo.

The naive plan was a 60-90 second loop with a fade-in/fade-out at the
seam. That was clean but had audibly-quiet silence at the wraparound —
the listener could hear the loop boundary even if the seam was
click-free. We swapped to a 20-second loop with a 1-second equal-power
crossfade hidden at the start of the body:

```
Source S (continuous waveform):
    ... A A A B B B C C C D D D E E E F F F ...
                |---------- 20 sec body -----------|--- 1s tail ---|

loop[i] = body[i] * sin(πi/2N) + tail[i] * cos(πi/2N)   for i in [0..N)
loop[i] = body[i]                                       for i in [N..]
```

When the SPU loops `loop[end] → loop[0]`, the listener hears: end of
body → tail fading out (which is what would naturally have come next in
the source) → body fading in. By the time the crossfade has fully
exposed the body, we're past the seam. For ocean material this is
functionally invisible — the seam buries itself in broadband noise.

Bonus: a 20-second loop is ~123 KB instead of the original sketch's
~370 KB. That ~250 KB savings in SPU RAM is enough room for 2-3
additional ambience tracks under the same toggle mechanism if we ever
want them (calmer, stormier, holiday-tagged). The encoding pipeline
(`scratch/ocean-ambience/make_tight_loop.py` + `encode_vag_loop.py`) is
already general-purpose — drop any mono 16-bit WAV in, get a tight loop
VAG out.


## The toggle: pause-menu, memcard, zero-flicker

Pause → Accessibility → **Ocean: ON / OFF**. LEFT, RIGHT, or X on the
row toggles `oceanAmbientEnabled` and immediately calls
`oceanAmbientStart()` / `oceanAmbientStop()`. The SPU voice keys on or
off in one register write — no fade, no clicking, no audible delay.
The Save Settings to Memcard option persists the choice (`MC_VERSION`
bumped 2 → 3, with a graceful v2-load fallback that defaults the new
field to ON). On the next boot `soundInit` reads the saved value and
either auto-keys the voice or leaves it silent.

The wiring mirrors the existing `soundMuted` accessibility toggle —
same accessor pattern, same memcard schema bump, same register-write
toggle. About 200 lines net across five files.


## Frame impact: zero, verified

The screensaver's `JCPERF2` per-scene metrics (`loop_vb`,
`blocking_vb`, `overrun_vb`, `compose_calls`, etc.) are identical with
the ambience on or off. The SPU mixes its 24 voices in hardware
regardless of how many are active; an additional voice costs nothing.
The loop wraparound is a single bit-flag check the SPU hardware does
at the end of each ADPCM block; no IRQ, no DMA, no CPU.

The total cost is ~123 KB of SPU RAM + ~30-60 ms of additional boot
time (the extra CD read and DMA upload of `OCEAN.VAG` into SPU RAM).
After boot, no impact at all.


## What the headroom leaves

The SPU has 512 KB of RAM. Existing SFX uses ~94 KB. The ocean loop
uses ~123 KB. After both, ~257 KB is still free — enough for 2-3
additional 20-second ambience tracks under the same pause-menu toggle,
switching tracks via a single `SPU_KEY_OFF`/`SPU_KEY_ON` pair. The
pause-menu toggle could naturally extend from **ON / OFF** to a short
list (**Off / Ocean / Calm / Stormy**) without changing the
underlying mechanism. Nothing in v0.6 forces that direction; the
headroom is just present if we want it.

Alternately, the same SPU RAM budget supports holiday-tagged ambience
(the holiday system already carries per-holiday metadata in
`gHolidays[]`; an optional ambience index per holiday would swap loops
at scene boundaries) or short scene-start stings on spare voices.
These are future work; `v0.6.0-ps1` ships exactly one CC0 ocean loop
and a binary toggle.
