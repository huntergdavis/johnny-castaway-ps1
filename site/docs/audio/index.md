---
layout: page
title: Audio pipeline
eyebrow: Reference
subtitle: SPU layout, sample preload, captured PLAY_SAMPLE replay, and where DuckStation's HLE diverges from the real hardware.
description: How audio works on the Johnny Castaway PS1 fan port — SPU RAM layout, the VAG sample preload at boot, the host-captured sound-event replay path, and the DuckStation HLE divergence.
---

A labor of love by Hunter Davis. This page describes the audio pipeline:
the SPU constraints, the conversion path from WAV to [VAG]({{ '/docs/glossary/#vag' | relative_url }}), the runtime
preload, and the playback path during scene replay. If you paid for this,
you were cheated. Open source and free.

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

## The SPU

The PS1's [Sound Processing Unit]({{ '/docs/glossary/#spu' | relative_url }}) is a separate processor with its own RAM
and its own job. Specs:

- **Voices:** 24 simultaneous, each with hardware ADSR.
- **SPU RAM:** 512 KB, dedicated. The CPU writes via DMA.
- **Sample format:** 4-bit [ADPCM]({{ '/docs/glossary/#adpcm' | relative_url }}) in 16-byte blocks (28 nibbles + flags).
- **Sample rate:** Up to 44.1 kHz.
- **Mixing:** Done in hardware. The CPU sets per-voice volume and pitch;
  the SPU mixes and outputs.
- **Reverb:** Built-in, configurable per-voice send.
- **Streaming:** Supported via XA-ADPCM CD sectors and SPU IRQ-driven
  double-buffering.

For a 1992-era screensaver port, that's well beyond the source material.
*Johnny Castaway* uses 23 short digitized SFX — splashes, footsteps,
clicks, the gull, the shark, the fishing-related thuds. The longest is
1.83 seconds. There is no music. There are no looping samples. Total
ADPCM data after silence trimming is under 80 KB — less than 16% of SPU
RAM. The constraint is comfortable; the interesting work is making the
playback timing align with the host capture.

## The pipeline

Original Sierra plays its digitized samples per scene event via the
TTM `PLAY_SAMPLE` opcode (`0xC051`) with a single argument: the sound
index. The opcode has no volume parameter. The PS1 port preserves that
shape end-to-end.

```
sound{N}.wav (11025 Hz mono, 8-bit unsigned)
    │
    ▼
scripts/wav2vag.py
    │   • silence trim (block-aligned)
    │   • 8-bit unsigned → 16-bit signed PCM
    │   • PS1 SPU ADPCM encode (4-bit, 28 samples / 16-byte block)
    │   • brute-force best-fit across 5 filters × 13 shifts
    │   • prepend 48-byte VAG header (magic "VAGp", BE sizes)
    │
    ▼
jc_resources/extracted/snd/SOUND{NN}.VAG
    │
    ▼   (mkpsxiso routes into \SND\ on the disc)
jcreborn.bin
    │
    ▼
runtime: soundInit() at boot
    │   • SpuInit(); SpuSetCommonMasterVolume(0x3FFF, 0x3FFF)
    │   • for each VAG: load from CD into a temp buffer, DMA to SPU RAM,
    │     free the buffer. Records start address, ADPCM size, sample rate.
    │
    ▼
runtime: foreground_pilot.c at scene-replay time
    │   • host capture writes 0xC051 PLAY_SAMPLE events into the FG2 pack
    │   • on the recorded frame, soundPlay(nb) is called
    │   • soundPlay() picks the next channel round-robin, programs ADSR,
    │     sets pitch from the sample rate, writes start address, keys on
```

The host capture binary is the source of truth for *when* a sample plays.
The PS1 runtime owns the SPU side: voice allocation, DMA, ADSR, key-on /
key-off. The 3-frame delay constant in `foreground_pilot.c` aligns key-on
with the visible frame — the SPU has its own pipeline and a sample
triggered on frame *N* is audible during frame *N+3*, so the capture
event is fired three frames early on the PS1 side.

## What the audio-optimization spec covers

The full reference is at
[`docs/ps1/audio-optimization-spec.md`]({{ site.github_url }}/blob/main/docs/ps1/audio-optimization-spec.md).
The relevant material:

### Sample inventory

23 WAV files exist on disk (indices 0–10, 12, 14–24; gaps at 11 and 13).
Of those, 22 are actually triggered by the game — sound 17 has a WAV
file but is never referenced by any TTM or C code, and sound 11 is
referenced once in `GJCATCH2.TTM` but the WAV file does not exist
(the desktop build silently warns; the PS1 build does the same).

Most-referenced samples:

| Idx | Refs | Description                  |
|----:|-----:|:-----------------------------|
|  16 |   78 | Walking / footsteps (most used) |
|   6 |   67 | Long splash / crash          |
|   9 |   49 | Short action (heavily used)  |
|   5 |   45 | Footstep / thud              |

Sound 0 is only triggered from `story.c` (not TTMs); it plays at every
daytime scene start as ambient day-scene cue.

### SPU RAM layout

Reserved regions and the sample bank, all 16-byte aligned (ADPCM block
size). DMA transfers pad to 64-byte boundaries to match the PSn00bSDK
`vagsample` example.

```text
0x00000  4096 B  Reserved (SPU capture buffers)
0x01000    16 B  PSn00bSDK dummy sample block
0x01010   ...    Sound effect ADPCM data (packed sequentially)
0x146C0          End of used region
0x146C0  430 KB  Free
0x7FFFF          End of 512 KB SPU RAM
```

Total used after silence trimming: ~77 KB (15.3% of SPU RAM). The
remaining 430 KB is available for future music or reverb work but is
not consumed today.

### Why no streaming

All samples are under 2 seconds. Streaming requires dedicated SPU
channels, IRQ handlers, and double-buffering. None of that is
warranted for short one-shot effects, and the static preload at boot
eliminates CD seek latency during scene playback — which would
otherwise produce visible frame hitches. The desktop reference build
uses the same approach; the PS1 port mirrors it.

### Voice allocation

8 round-robin SPU channels for SFX. The game rarely plays more than 2
or 3 overlapping sounds; 8 is generous. The remaining 16 voices are
unused and available for music if the project grows that direction.

## The known SPU bugs

The audio path went through a long debugging pass during the
`v0.3.6-ps1` milestone. The bugs encountered, mostly in the VAG
encoder and the SPU upload path:

- **Shift exponent inversion** in the ADPCM encoder. The brute-force
  best-fit was searching the wrong direction in the shift table.
- **ADPCM nibble-pair order.** PS1 SPU expects the first sample in the
  high nibble, then the low. The encoder had it reversed; samples
  played at the right rate but garbled.
- **SPU DMA 64-byte alignment.** The PSn00bSDK `SpuWrite()` examples
  pad to 64-byte boundaries. Earlier code padded to 16-byte; transfers
  truncated unpredictably.
- **ADSR1 attack-rate orientation.** The attack-rate field is
  documented in two contradictory directions across reference docs.
  The working configuration matches the `vagsample` example exactly:
  `SPU_CH_ADSR1(ch) = 0x00FF` (max attack, no decay), `SPU_CH_ADSR2(ch) = 0x0000`
  (no sustain or release).

The fix list is captured in commit `355227fa` for anyone debugging
similar PS1 SPU issues.

## DuckStation HLE vs hardware

This is the place where audio behavior diverges measurably between
emulation and real hardware, and where the project has to be careful
about claims.

DuckStation's SPU emulation is HLE (high-level emulation) by default.
Most calls go through with the documented behavior. One that does not:
**`SpuSetCommonMasterVolume` is not honored by DuckStation HLE.**
Writing the documented PSn00bSDK helper does not actually change the
master output level on the emulator. The pause-menu mute toggle works
around this by writing the SPU master-volume register directly:

```c
*(volatile uint16_t *)0x1f801d80 = 0x0000;  /* left  master = 0 */
*(volatile uint16_t *)0x1f801d82 = 0x0000;  /* right master = 0 */
```

That direct write does work on DuckStation. It is also expected to
work on real hardware, since the registers are documented and the
helper is supposed to be a thin wrapper over them. **The author has
not yet validated audio mute on real hardware.** Until that pass
happens, the audio path on hardware is "believed correct, not signed
off."

What the author currently trusts most for audio validation:

1. **Host-captured sound events** as the source of truth for when a
   sample should play. The PS1 doesn't get to decide; it replays the
   host capture's sound-event log. If the SFX timing is wrong on PS1,
   the bug is in the SPU trigger path, not the timing data.
2. **Hardware capture** when it happens. The PS1 hardware sitting on
   the author's desk has been used to spot-check the most-reproducible
   SFX (splashes, footsteps), and on those samples the hardware output
   matches the desktop reference build.
3. **DuckStation TTY logs + spectrum** for everyday development.
   Useful for catching glitches like wrong-pitch playback or
   trailing-block artifacts. Not authoritative for level / mix.
4. **YouTube comparisons of the original Sierra DOS executable** for
   ground truth on what each sample should sound like in context.
   These are useful for the rare cases where the host capture is
   itself wrong — usually because a TTM script branch was taken on
   desktop that the PS1 reaches differently.

## What hasn't been tackled

A few audio quality items the author knows are open:

- **Volume balance per sample.** The original screensaver likely had
  per-sample volume variation, but the `0xC051 PLAY_SAMPLE` opcode
  takes only a sound index — no volume parameter. The PS1 voice
  registers (`SPU_CH_VOL_L/R`) support 15-bit signed volumes
  (0x0000–0x3FFF) so per-sample volume is trivially possible, but no
  one has authored the per-sample table yet. Today every SFX plays at
  the same channel volume.
- **Click on sample boundaries.** Some samples have audible clicks at
  the start or end. Silence trimming in `wav2vag.py` reduced this for
  the heavily-used effects (sounds 0, 5, 9, 16, 21, 24 all benefit
  >28% from trim), but the encoder's leading silent block plus the
  trailing stop-flag block still produce a faint click in some cases.
  Not yet investigated whether this is a fixable encoder issue or a
  hardware artifact of the ADPCM start/stop flags.
- **No tide-state crossfade.** The runtime supports tide states (high
  / low) which select between FG2 pack pairs; transitions are
  hard-cut on both video and audio. A crossfade would smooth the
  audio side but isn't currently planned.
- **Hardware mute validation.** Documented above. Open until the
  author runs a real-hardware capture pass with the pause menu and
  confirms the direct register write actually mutes on hardware.
- **Music.** The remaining free SPU RAM (~300 KB after the SFX
  bank and the looping ocean ambience) could host a small streaming
  music layer or a handful of pre-mixed tracks. The author has not
  decided whether *Johnny Castaway* should have music — the original
  did not. This is a design question, not a technical one.

## Ocean ambience (v0.6.0-ps1)

The runtime carries one looping background track — a 20-second
[ocean-ambience]({{ '/docs/glossary/#ocean-ambience' | relative_url }}) sample on a dedicated SPU voice slot reserved at boot.
Toggleable via Pause → Accessibility → Ocean and persisted to the
[memcard]({{ '/docs/glossary/#memcard' | relative_url }}) alongside the other v6 schema settings. Zero per-frame
CPU cost: the SPU loops the sample in hardware; the main CPU
never touches the voice after boot.

The on-disc artifact is `OCEAN.VAG` (~126 KB, 4-bit ADPCM at
11.025 kHz mono). Source is [BigSoundBank.com sound 0266](https://bigsoundbank.com/sea-waves-s0266.html) ("Sea: Waves", CC0; full attribution at [/credits/#ocean-ambience]({{ '/credits/' | relative_url }}#ocean-ambience)); the
encoding pipeline lives in `scratch/ocean-ambience/`. The seam is
hidden by an equal-power crossfade with the recording's natural
continuation, so the SPU's hardware loop reads as unbroken ocean
rather than a wraparound.

Architecturally this is the second category of audio the runtime
manages. The 23 captured SFX live in voice slots `0..7` rotated
round-robin by `sound_ps1.c`; the ocean track lives on a fixed slot
outside that rotation so a busy SFX scene never evicts it. The
release entry is at
[/releases/#v060-ps1--ocean-ambience]({{ '/releases/#v060-ps1--ocean-ambience' | relative_url }}).

## Future-considered, not future-planned

The audio-optimization spec lists three further directions: SPU
streaming from CD via the `cdstream` reference pattern, XA-ADPCM
sectors as a higher-quality streaming path, and hardware reverb on a
small dedicated buffer. None are scoped for the next release. They
are noted in case the project ever needs them.

## Related pages

- [Hardware]({{ '/docs/hardware/' | relative_url }}) — SPU specs in
  the broader machine reference.
- [Performance work]({{ '/docs/performance/' | relative_url }}) —
  audio scheduling shows up as `sound_late` in the perf records.
- [Build & toolchain]({{ '/docs/build/' | relative_url }}) — where
  `psxspu` gets linked.
- [Method]({{ '/about/method/' | relative_url }}) — how the
  audio acceptance bar fits the project's overall standard.
- [v0.6.0-ps1 release entry]({{ '/releases/#v060-ps1--ocean-ambience' | relative_url }})
  — the milestone the ocean-ambience loop shipped in.
- [Devlog: ocean ambience v0.6]({{ '/devlog/ocean-ambience-v0-6/' | relative_url }})
  — the implementation worklog for the dedicated SPU voice that
  drives the ambience loop. Pairs with the Ocean ambience H2 above.
- [Glossary: ADPCM]({{ '/docs/glossary/#adpcm' | relative_url }})
  · [Glossary: VAG]({{ '/docs/glossary/#vag' | relative_url }})
  · [Glossary: SPU]({{ '/docs/glossary/#spu' | relative_url }})
  · [Glossary: PLAY_SAMPLE]({{ '/docs/glossary/#play-sample' | relative_url }})

## View source on GitHub

- [`docs/ps1/audio-optimization-spec.md`]({{ site.github_url }}/blob/main/docs/ps1/audio-optimization-spec.md)
- [`src/platform/ps1/sound_ps1.c`]({{ site.github_url }}/blob/main/src/platform/ps1/sound_ps1.c) —
  the SPU adapter; voice keying, master-volume register writes, the
  ocean ambience SPU slot, the VAG transfer wrappers.
- [`src/foreground_pilot/foreground_pilot.c`]({{ site.github_url }}/blob/main/src/foreground_pilot/foreground_pilot.c) —
  scene-replay-side audio dispatch; consumes captured `0xC051 PLAY_SAMPLE`
  events from FG2 packs and fires `soundPlay()` with the 3-frame
  key-on delay the body discusses.
- [`scripts/wav2vag.py`]({{ site.github_url }}/blob/main/scripts/wav2vag.py)
- [`scripts/convert-sounds.sh`]({{ site.github_url }}/blob/main/scripts/convert-sounds.sh)
