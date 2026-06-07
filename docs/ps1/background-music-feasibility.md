# Ocean Ambience — PS1 Implementation Plan

Date: 2026-05-01
Branch context: `ocean-ambience-20260501`, shipped as `v0.6.0-ps1`.
Status: **shipped**. Asset, boot loader, pause-menu toggle, memcard
persistence, and CD-image embedding are all live.

## Shipped state (v0.6.0-ps1)

| Layer | Where | Status |
|---|---|---|
| Asset | `jc_resources/extracted/snd/OCEAN.VAG` (123 KB, 20-sec seamless loop) | Shipped |
| CD layout | `config/ps1/cd_layout.xml` adds OCEAN.VAG under `SND/` | Shipped |
| Boot loader | `src/platform/ps1/sound_ps1.c::soundInit` uploads VAG to SPU RAM after the SFX block; auto-keys voice 23 if `oceanAmbientEnabled` | Shipped |
| Start/Stop API | `oceanAmbientStart()` / `oceanAmbientStop()` / `oceanAmbientLoaded()` in `src/platform/ps1/sound_ps1.h` | Shipped |
| Pause-menu toggle | `ACCESS_OCEAN` row on the Accessibility page; LEFT/RIGHT/X flips the toggle and immediately starts/stops the SPU voice | Shipped |
| Memcard persistence | `MC_VERSION` 2 → 3; `oceanAmbientEnabled` field; v2 saves load gracefully with the new field defaulted ON | Shipped |
| Frame impact | Per-VBlank cost zero (SPU mixes + auto-loops in hardware); JCPERF2 unchanged | Verified |
| Source license | BigSoundBank.com sound 0266 ("Sea: Waves") — CC0 / public domain. Attributed in `docs/credits/index.html`. | Shipped |

The rest of this document is the original plan + research, retained
for the design rationale, the multi-slot architecture (which the
123 KB ship leaves room for), and the red-team review.

## Goal

> Add an optional, looping ocean ambience track as background audio for
> the screensaver. The user can toggle it on or off from the pause
> menu. Source the audio from a public-domain (CC0) recording. **Zero
> CPU overhead during steady-state playback** — the screensaver loop's
> per-VBlank cost must be identical with the ambience on or off.

This document is the plan for that goal. It explains the mechanism we
chose, the source recording, the loop construction, the pause-menu
integration sketch, and the multi-slot architecture the chosen
mechanism opens up for future tracks.

### What "zero CPU" rules out

The "zero CPU" constraint is the strongest constraint in the plan and
worth stating explicitly:

- **Per-frame CPU work in the screensaver loop:** zero. The
  `loop_vb` / `target_vb` / `compose_calls` numbers in JCPERF2 must be
  identical to the ambience-off baseline.
- **Per-IRQ CPU work for music streaming:** ruled out. Even an SPU IRQ
  handler that runs once per second is non-zero CPU (~10-20 cycles +
  the pipeline interlocks the IRQ entry costs).
- **Per-VBlank sequencer ticks:** ruled out (1-3 ms per VBlank for
  SEQ/VAB / MOD).
- **One-shot CPU work on user input** (toggling the pause menu option):
  acceptable. This is not in the screensaver loop's frame-time budget;
  it's in the menu's input handler, which only runs while the menu is
  open.
- **One-shot CPU work at boot** (loading the VAG into SPU RAM):
  acceptable. Same boot-time class as the existing SFX upload.

That leaves exactly one mechanism in scope.

## Mechanism Choice: Pre-loaded ADPCM Loop in SPU RAM

The five PS1 audio paths and why we want exactly one of them for this
goal:

| Mechanism | Per-frame CPU | Verdict | Reason |
|---|---|---|---|
| **A. Pre-loaded ADPCM loop in SPU RAM** | **0 cycles** | **Selected** | The only option with strict zero CPU. SPU plays + auto-loops in hardware. |
| B. SPU streaming from CD | ~10-20 cycles per chunk IRQ | Rejected | Non-zero CPU. Also adds CD bandwidth contention. |
| C. CD-DA (Red Book) | 0 cycles | Rejected | Cannot share the drive with data reads — kills FG2 prefetch. |
| D. XA-ADPCM | 0 cycles | Rejected | Zero CPU but adds mkpsxiso XA authoring complexity for capability we don't need (8 channels). Defer until v2. |
| E. Sequenced (SEQ/VAB / MOD) | 1-3 ms / VBlank | Rejected | Real per-frame CPU cost. |

Option A means: load one ADPCM-encoded ocean loop into SPU RAM at
boot, set the loop flag on the last 16-byte ADPCM block, key it on a
dedicated SPU voice. The SPU plays it forever with **literally zero**
CPU involvement once the voice is keyed on.

## Source: BigSoundBank "Sea: Waves" (CC0)

The audio shipped in `jc_resources/extracted/snd/OCEAN.VAG` derives
from:

- **Source page:** <https://bigsoundbank.com/sea-waves-s0266.html>
- **Direct MP3:** <https://bigsoundbank.com/UPLOAD/mp3/0266.mp3>
- **License:** CC0 / WTFPL / public domain (per the source site).
  GPL-3 redistributable with attribution kept in
  `docs/credits/index.html`.
- **Material:** "Moderate waves, swirls, dull roar of the sea
  beyond." 57 seconds, 44.1 kHz stereo, 320 kbps MP3. No bird calls,
  no human voices, no breaking-wave spike transients — broadband
  low-frequency content, ideal for ADPCM.

The earlier research recommended freesound.org's `amholma — Gentle
Waves - Quiet Beach` (also CC0) but freesound.org requires
authentication for downloads. BigSoundBank is anonymous direct
download with the same license class and equivalent material.

## Loop Construction: 20-Second Crossfade-Replace Seam

The shipped asset is a **20-second seamless loop**, not the 60-90
second loop the original draft of this document sketched. Two reasons
the tighter loop is better:

1. **Seamlessness.** A long loop with simple fade-in/out at the seam
   has audibly-quiet silence at the wrap point. A short loop with a
   crossfade-replace at the seam transitions through a continuation
   of the source audio rather than through silence — perceptually
   indistinguishable from the ocean simply continuing.
2. **SPU RAM headroom.** A 20-second loop costs **~123 KB** instead
   of ~370 KB. That ~250 KB savings is enough room for additional
   ambience tracks (calmer, stormier, holiday-tagged) under the same
   pause-menu mechanism. See "Future track slots" below.

### How the crossfade-replace works

The technique is standard for game audio loops. Pick a window slightly
longer than the target loop length:

```
Source S (continuous waveform from the recording):
    ... A A A B B B C C C D D D E E E F F F ...
                |---------- 20 sec body -----------|--- 1s tail ---|

Loop body = S[t : t+20s]
Tail F    = S[t+20s : t+21s]   (the 1 second that naturally followed
                                the loop body in the source)
```

Replace the first 1 second of the loop body with an equal-power
cosine crossfade between `body[0..1s]` (fading in) and `tail[0..1s]`
(fading out):

```
loop[i] = body[i] * sin(πi/2N) + tail[i] * cos(πi/2N)   for i in [0..N)
loop[i] = body[i]                                       for i in [N..]
```

When the SPU loops `loop[end] → loop[0]`, the listener hears: the end
of the body → the tail fading out (which is what would naturally
have come next in the source) → the body fading in. By the time the
crossfade has fully exposed the body, we're past the seam and into
normal looping.

For ocean material this is functionally invisible — the seam is buried
under one wave-cycle of broadband noise.

### Loop seam audibility

Decoded round-trip back through the exact SPU ADPCM math:

| Sample | Value |
|---|---|
| Last sample of loop body | -1089 |
| First sample of loop body | -3454 |
| Delta | 2365 (~ -22 dB FS) |
| Audibility | Buried in the crossfade-blended ocean noise floor |

The audition file
(`scratch/ocean-ambience/OCEAN_verify_loop_seam.wav`) plays the
decoded loop body once + 5 sec of replayed start. The seam at the
0:20 mark is not audibly distinguishable from the surrounding ocean.

### Final VAG specs

- File: `jc_resources/extracted/snd/OCEAN.VAG`
- Size: **126,064 bytes** (~123 KB)
- Format: Sony VAG (4-bit ADPCM), mono, 11,025 Hz
- Duration: 19.96 seconds
- Loop flags (per Nocash PSX docs §SPU):
  - **First data block** (file offset 64): `flag = 0x06` (loop start
    + repeat hint) — SPU records this address as `LOOP_ADDR`
    automatically when it plays.
  - **Last data block** (file offset 126,032): `flag = 0x03` (loop
    end + continue) — SPU jumps back to `LOOP_ADDR` and plays again,
    forever.
  - No trailing 0x07 stop block — the voice never keys itself off.

## Encoding Pipeline (Reproducible)

The pipeline is in `scratch/ocean-ambience/`. Once we want to
encode another track, move `make_tight_loop.py` and
`encode_vag_loop.py` into `scripts/`.

```bash
# 1. Source download (CC0)
curl -A "Mozilla/5.0" -o sea_waves_0266.mp3 \
    "https://bigsoundbank.com/UPLOAD/mp3/0266.mp3"

# 2. Decode → mono 11.025 kHz 16-bit PCM, skip the loud opening
#    transient, light loudnorm, 300 ms aesthetic fades on the source
ffmpeg -y -i sea_waves_0266.mp3 \
    -ss 1.0 -t 55 \
    -ac 1 -ar 11025 \
    -af "loudnorm=I=-18:TP=-3:LRA=7,afade=t=in:st=0:d=0.3,afade=t=out:st=54.7:d=0.3" \
    -acodec pcm_s16le ocean_mono_11k_v2.wav

# 3. Build a 20-sec seamless loop with crossfade-replace at the seam
python3 make_tight_loop.py ocean_mono_11k_v2.wav ocean_tight_20s.wav

# 4. Encode to PS1 VAG ADPCM with sample-defined loop flags
python3 encode_vag_loop.py ocean_tight_20s.wav OCEAN.VAG
```

`make_tight_loop.py` parameters: 20-second loop length, 1-second
crossfade window, 5-second source-head skip. All three are easy to
tune for other tracks.

## SPU RAM Map

The existing SFX system uses ~94 KB starting at SPU address `0x1010`.
After SFX, SPU memory looks like:

| Region | Bytes | Notes |
|---|---|---|
| System / capture / dummy block | 0x0000 - 0x0FFF | 4 KB reserved by SPU. |
| SFX VAG bank | 0x1010 - ~0x18000 | ~94 KB, 25 sound effects loaded by `soundInit`. |
| **Reserved for SFX expansion** | ~0x18000 - 0x20000 | ~32 KB headroom. |
| **OCEAN.VAG ambience loop** | 0x20000 - ~0x3F800 | ~123 KB. Address chosen to leave SFX expansion alone. |
| **Available for future tracks** | ~0x3F800 - 0x80000 | ~257 KB. Slot model below. |

(The exact addresses are implementation-side; what matters here is
that we have ~257 KB free after the ocean loop.)

## Pause-Menu Toggle: `oceanAmbientEnabled`

Wiring parallels existing `soundMuted` / `closedCaptionsEnabled` toggles.
Reuse the established pattern.

### Global state

In `src/platform/ps1/sound_ps1.c` (or a new `src/ambient_ps1.c` if we want to keep
the SFX path clean):

```c
int oceanAmbientEnabled = 1;     /* default on */
static uint32_t oceanAmbientSpuAddr = 0;   /* set by soundInit */
static uint16_t oceanAmbientPitch   = 0;
#define OCEAN_AMBIENT_VOICE 23   /* dedicated voice; SFX uses 0..7 round-robin */
```

### Boot path

`soundInit` already loads SFX VAGs starting at SPU address `0x1010`.
After the SFX loop, append:

```c
/* Load the ocean ambience loop into SPU RAM after the SFX block. */
uint8_t *vag = ps1_loadRawFile("\\SND\\OCEAN.VAG;1", &vagSize);
if (vag && vagSize > VAG_HEADER_SIZE) {
    /* Same upload pattern as the SFX loader: parse rate from VAG
     * header, DMA the ADPCM body into SPU RAM, record address. */
    uint32_t adpcmSize = vagSize - VAG_HEADER_SIZE;
    uint32_t dmaSize   = (adpcmSize + 63u) & ~63u;
    if (spuAddr + dmaSize <= 512 * 1024) {
        /* ... DMA upload identical to the SFX path ... */
        oceanAmbientSpuAddr = spuAddr;
        oceanAmbientPitch   = getSPUSampleRate(
                                  (uint16_t)readBE32(vag + 16));
    }
    free(vag);
}

if (oceanAmbientEnabled && oceanAmbientSpuAddr != 0)
    oceanAmbientStart();
```

### Start / Stop

The loop flags in the source ADPCM make `SPU_CH_LOOP_ADDR` writes
optional — the SPU records the loop point itself when it hits the
flag-0x06 first data block. We only need to set address + pitch +
volume + envelope and key on:

```c
void oceanAmbientStart(void)
{
    if (oceanAmbientSpuAddr == 0) return;
    int ch = OCEAN_AMBIENT_VOICE;
    SPU_CH_FREQ(ch)  = oceanAmbientPitch;
    SPU_CH_ADDR(ch)  = getSPUAddr(oceanAmbientSpuAddr);
    SPU_CH_VOL_L(ch) = 0x2800;     /* slightly under SFX so it sits
                                    * underneath, not on top of, them */
    SPU_CH_VOL_R(ch) = 0x2800;
    /* Long-attack ADSR so a toggle-on doesn't click. */
    SPU_CH_ADSR1(ch) = ADSR1_ATTACK_LONG;
    SPU_CH_ADSR2(ch) = ADSR2_SUSTAIN_HOLD;
    SPU_KEY_ON = (1u << ch);
}

void oceanAmbientStop(void)
{
    SPU_KEY_OFF = (1u << OCEAN_AMBIENT_VOICE);
}
```

### Pause-menu row

In `src/pause_menu/pause_menu.c`, add an `OPT_OCEAN_AMBIENT` enum entry near
the other audio toggles in the Accessibility submenu. Add a row that:

- displays "Ocean ambience: ON / OFF"
- on toggle, flips `oceanAmbientEnabled` and calls
  `oceanAmbientStart()` / `oceanAmbientStop()`

Same shape as the existing `OPT_SOUND` row; ~20 lines of
copy-paste.

### Memcard persistence

Bump `MC_VERSION` from 2 to 3 in `src/platform/ps1/memcard.c`. Add
`oceanAmbientEnabled` to `JCMCSettings`. Read/write alongside
`soundMuted`. Migration is the standard "field
not present in v2 → use default" path the codebase already has.

### Master mute interaction

`soundMuted` already manipulates `SpuSetCommonMasterVolume`, which
affects all voices including the ambience voice. No extra wiring
needed for master mute.

## Frame-Impact Honest Accounting

| What runs per frame with ocean ambience on | Cost |
|---|---|
| SPU mixing 1 extra voice | 0 cycles (hardware) |
| Loop restart at end of sample | 0 cycles (hardware loop flag) |
| Master volume gate | 0 cycles (already in path) |
| `oceanAmbientStart` / `Stop` | One-shot register writes (only fires on toggle) |

Zero per-frame CPU cost. Zero per-frame CD bandwidth. The screensaver
runs at exactly the same FPS / `loop_vb` budget with ocean ambience on
as off, modulo measurement noise. The headless perf gate
(`scripts/ps1-perf-iterate.sh`) should be flat across this change.

## Memory Cost Summary

| Resource | Before | After | Notes |
|---|---|---|---|
| SPU RAM (512 KB total) | 94 KB used | ~217 KB used | +123 KB ocean loop |
| Main RAM | n/a | +0 | ADPCM lives in SPU RAM only |
| Code (ELF) | — | +~1 KB | Boot loader + start/stop helpers + menu row |
| CD image | — | +~123 KB | One additional VAG file |
| Boot heap (peak, transient) | — | +~123 KB during DMA upload, freed | Same class as existing SFX upload |

CD image grows by ~123 KB. Boot ramp grows by ~30-60 ms (extra CD
read + DMA + checksum). After boot, no impact.

## Future Track Slots

123 KB of SPU RAM for one ambience leaves ~257 KB free for additional
tracks, all using the same pipeline:

| Slot | Use | Size estimate |
|---|---|---|
| 0 | Ocean ambience (the current `OCEAN.VAG`) | 123 KB |
| 1 | Calmer alternate (smaller waves, deeper night) | ~80-120 KB |
| 2 | Stormier alternate (heavier surf, wind) | ~80-120 KB |
| 3 | Holiday-tagged ambience (winter shore, summer breeze, etc.) | ~30-60 KB each, swap as `islandState.holiday` changes |
| 4-6 | Short cues / stings (scene-start chimes, holiday markers) | 5-30 KB each |

Switching slots = `SPU_KEY_OFF` old voice + write new `SPU_CH_ADDR`
+ `SPU_KEY_ON`. **Free CPU cost.** All tracks pre-loaded to SPU RAM
at boot, all addressed by their offset in the SPU memory map.

The pause-menu toggle naturally extends from on/off to a small list:
"Background audio: Off / Ocean / Calm / Stormy". Same memcard field,
different enum values. Same VAG-encoding pipeline for each new track.

The encode scripts (`make_tight_loop.py` + `encode_vag_loop.py`) are
already general-purpose — they don't know or care about ocean. Drop
any 16-bit mono WAV in, get a tight loop VAG out.

## Red-Team / Thesis Review

A defender of the previous-version research should poke at the
following claims; here's the defense.

### "11.025 kHz ADPCM is transparent on ocean material."

**Defended.** Ocean and surf spectra are dominated by broadband noise
below ~2 kHz with low-energy high-frequency content above. Sony 4-bit
ADPCM's quantization noise is most audible on tonal content above
~4 kHz; on ocean material it disappears into the existing
high-frequency hiss. **Concession:** for a future stormier track with
spray content peaking above 8 kHz, artifacts may be audible — pick a
broadband-low-frequency source and this isn't a concern.

### "Crossfade-replace at the seam is enough."

**Defended.** The verify-decode round-trip through the exact SPU
ADPCM math produces a -22 dB FS sample-value delta at the seam,
buried under the crossfade-blended ocean noise floor. Listener tests
on the audition file confirm the wraparound is not audibly
distinguishable from continued playback.

### "We have ~257 KB SPU RAM available for other tracks."

The accounting:

- 512 KB total SPU RAM
- 4 KB system reservation
- ~94 KB existing SFX
- 32 KB headroom for future SFX
- ~123 KB for `OCEAN.VAG`

Remaining: 259 KB. Quoted as ~257 KB above to leave a small safety
margin. Plenty for 2-3 additional 20-sec ambience tracks or many
short cues.

### "Public-domain CC0 licensing is clean for GPL-3 redistribution."

CC0 1.0 is explicit public-domain dedication and compatible with
GPL-3. We attribute in `docs/credits/index.html` for the courtesy /
discoverability, not because the license requires it. BigSoundBank's
license string includes "CC0 / WTFPL / public domain" — all three are
GPL-3 compatible.

### "Zero per-frame CPU cost."

**Defended in the steady state.** Once playing, the SPU mixer is
fully autonomous. The CPU does nothing per frame.

**Concession:** the *toggle event* (start/stop) is a small set of
register writes that runs in the pause-menu input handler, not in the
screensaver loop. Even the toggle is not in the frame critical path.

### Things to verify before promoting

- **VAG encoder loop-flag bytes.** Verified via decode round-trip
  matching the SPU ADPCM math; the audition file confirms loop
  semantics.
- **Memcard schema migration.** Bumping `MC_VERSION` to 3 needs the
  loader to handle v2 → v3 migration cleanly. Existing path does
  this for v1 → v2; mirror the same pattern.
- **SFX channel boundary.** Voice 23 is the highest available. Verify
  no other system uses voices 8-22 (probably none, but check with
  `grep` before committing).
- **Perf gate flat.** `scripts/ps1-perf-iterate.sh --scene fishing1`
  should be flat (loop_vb / blocking_vb / overrun_vb identical to
  baseline). If not, something the research missed.

## Concrete Next Steps

The asset is in place at `jc_resources/extracted/snd/OCEAN.VAG`. To
finish wiring it in:

1. **Add to CD layout.** One line in `config/ps1/cd_layout.xml` under
   the `<dir name="SND">` block:
   ```xml
   <file name="OCEAN.VAG" type="data"
         source="../../jc_resources/extracted/snd/OCEAN.VAG"/>
   ```
2. **Move encoding scripts** (optional, for future tracks):
   ```bash
   mv scratch/ocean-ambience/encode_vag_loop.py scripts/
   mv scratch/ocean-ambience/make_tight_loop.py scripts/
   ```
3. **Wire boot loader.** Extend `soundInit` in `src/platform/ps1/sound_ps1.c` to
   load `OCEAN.VAG` into SPU RAM after the existing SFX block.
4. **Add start/stop helpers.** `oceanAmbientStart()` /
   `oceanAmbientStop()` in `src/platform/ps1/sound_ps1.c`. ADSR with slow
   attack/release so toggles don't click.
5. **Wire pause-menu row.** Add `OPT_OCEAN_AMBIENT` to the menu enum;
   render row; toggle calls start/stop.
6. **Persist to memcard.** Bump `MC_VERSION` to 3; add field to
   `JCMCSettings`; serialize/deserialize alongside existing fields.
7. **Add credits row.** Update `docs/credits/index.html`:
   "OCEAN.VAG sourced from BigSoundBank.com sound 0266 'Sea: Waves'
   — CC0 / public domain. Encoded via `scripts/make_tight_loop.py` +
   `scripts/encode_vag_loop.py`."
8. **Run perf gate.** `scripts/ps1-perf-iterate.sh` should be flat
   on fishing1 vs baseline.

## Files Worth Reading Before Implementation

- `src/platform/ps1/sound_ps1.c` — existing SFX path, SpuInit, VAG loader, mute.
- `src/pause_menu/pause_menu.c` — existing toggle rows (`OPT_SOUND`,
  `OPT_CAPTIONS`); copy this pattern.
- `src/platform/ps1/memcard.c` — `JCMCSettings`, `MC_VERSION`, migration pattern.
- `scratch/psn00b-src/examples/sound/vagsample/main.c` — minimal
  one-shot VAG playback example.
- `scratch/psn00b-src/libpsn00b/include/psxspu.h` — full SPU API,
  voice loop register macros.
- `config/ps1/cd_layout.xml` — where to add the new VAG entry.
- `docs/credits/index.html` — where the source attribution goes.
- Nocash PSX docs (`https://problemkaputt.de/psx-spx.htm`) §SPU for
  the ADPCM block-flag bit semantics.
- `scratch/ocean-ambience/README.md` — encode workspace notes.
