# Ocean Ambience — PS1 Implementation Plan

Date: 2026-05-01
Branch context: read-only research; no code changes.
Author: research pass for the active perf iteration.

## Goal

> Add an optional, looping ocean ambience track as background audio for
> the screensaver. The user can toggle it on or off from the pause
> menu. Source the audio from a public-domain (CC0) recording. **Zero
> CPU overhead during steady-state playback** — the screensaver loop's
> per-VBlank cost must be identical with the ambience on or off.

This document is the plan for that specific goal. The earlier round of
research evaluated five PS1 audio mechanisms; this version trims to
the one that delivers strict zero-CPU playback from a CC0 source.

### What "zero CPU" rules out

The "zero CPU" constraint is the strongest constraint in the plan and
worth stating explicitly:

- **Per-frame CPU work in the screensaver loop:** zero. The
  `loop_vb` / `target_vb` / `compose_calls` numbers in JCPERF2 must be
  identical to the ambience-off baseline.
- **Per-IRQ CPU work for music streaming:** ruled out. Even an SPU IRQ
  handler that runs once per second is non-zero CPU (~10-20 cycles +
  the pipeline interlocks the IRQ entry costs). It would not show up in
  `loop_vb` granularity, but the user wants zero, not "zero to within
  measurement noise."
- **Per-VBlank sequencer ticks:** ruled out (1-3 ms per VBlank for
  SEQ/VAB / MOD).
- **One-shot CPU work on user input** (toggling the pause menu option):
  acceptable. This is not in the screensaver loop's frame-time budget;
  it's in the menu's input handler, which only runs while the menu is
  open.
- **One-shot CPU work at boot** (loading the VAG into SPU RAM):
  acceptable. Same boot-time class as the existing SFX upload.

That leaves exactly one mechanism in scope.

## Why Ocean Ambience Is a Special Case

Most "background music" feasibility analysis on PS1 has to balance
several competing constraints — sample rate vs disc bandwidth, loop
seam audibility vs RAM, reverb workspace vs voice count. Ocean
recordings sidestep most of those:

1. **Spectrum.** Ocean and surf are dominated by content below 1 kHz.
   The audible artifacts of Sony's 4-bit ADPCM live above ~4 kHz, so
   the compression is essentially transparent on this material. A
   30-90 second mono ADPCM clip at 11.025 kHz sounds clean.
2. **Loop forgiveness.** Wave patterns are aperiodic. A hard loop at
   the nearest zero-crossing isn't audible the way it would be on
   music with a clear meter or melodic phrase. We don't need a
   crossfade; we just need a clean cut.
3. **Reverb is the recording.** Ocean ambience is already a spatial
   field recording. Adding the SPU's hardware reverb on top would be
   double-dipping. We can skip the reverb workspace allocation
   entirely and reclaim that SPU RAM.
4. **Volume floor matches the screensaver.** Ambient ocean played
   quietly under a static-ish island view is the canonical Sierra
   screensaver mood. The audio is part of the fiction, not music
   layered on top.

These four observations turn a generic "PS1 background music" project
into a much smaller, much safer one.

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

Only A and D actually deliver the zero-CPU contract. A wins because
it's strictly simpler and doesn't touch the CD-image authoring
pipeline. D stays in reserve for if a future "many tracks" requirement
emerges.

Option A means: load one ADPCM-encoded ocean loop into SPU RAM at
boot, set the loop flag on the last 16-byte ADPCM block, key it on a
dedicated SPU voice. The SPU plays it forever with **literally zero**
CPU involvement once the voice is keyed on. Toggling the toggle is one
register write per direction, executed in the pause-menu input handler
— never in the screensaver frame loop.

Option A means: load one ADPCM-encoded ocean loop into SPU RAM at
boot, set the loop flag on the last 16-byte ADPCM block, key it on a
dedicated SPU voice. The SPU plays it forever with zero CPU
involvement. Toggling the toggle is a single register write per
direction.

## Public-Domain Audio Sourcing

The project is GPL-3.0 and the build is publicly distributed. We need
a clean license chain. **CC0 1.0** ("public-domain dedication") is the
gold standard — explicit, no attribution requirement, no edge cases
around derivative works. CC-BY 4.0 *would* be GPL-3 compatible if we
kept attribution, but CC0 is strictly simpler so we filter to CC0 only.

Source platforms in order of cleanest:

1. **freesound.org with the CC0 license filter.** The bulk of our
   options. Filter URL: `https://freesound.org/browse/tags/cc0/` plus
   search by tag.
2. **archive.org soundscape collections.** Pre-1928 recordings are
   categorically PD in the US; modern recordings explicitly released
   PD are also fine. Quality variable.
3. **NPS / National Park Service soundscape archive.** Federal-
   government works are public domain in the US. Ocean recordings
   from coastal parks (Acadia, Olympic, Cape Cod) available.

**Verify-and-avoid** sources, even if they look free:

- BBC Sound Effects archive — research/personal use only; not GPL-3
  redistributable.
- Pixabay — has its own "Content License" that's similar to CC0 but
  not identical (terms could change; CC0 cannot). Acceptable but not
  strictly preferred.
- Sites labeled "royalty-free" without an explicit CC0 declaration —
  attribution-required or non-commercial restrictions are common and
  conflict with GPL-3 redistribution.
- YouTube rips — license unclear; not safe.

### Verified CC0 Candidates (researched 2026-05-01)

Two recordings on freesound.org meet our requirements and are
pre-verified CC0:

#### Primary recommendation: amholma — "Gentle Waves - Quiet Beach"

- URL: <https://freesound.org/people/amholma/sounds/376795/>
- License: **CC0 1.0** (public-domain dedication)
- Duration: 100.7 seconds
- Source format: 48 kHz stereo
- Description: Field recording from Destin, Florida — emphasis on
  small waves washing ashore, "really nice and quiet beach." No
  loud crashing waves, no birds, no human voices.
- Why it's right for us:
  - The "gentle wash, no breakers" character means broadband low-
    frequency content with little high-frequency spike — ideal for
    11 kHz ADPCM.
  - 100 seconds gives ~40 seconds of slack to find a clean
    zero-crossing loop boundary. We can pick the calmest 60-second
    sub-region.
  - Quiet beach character matches the screensaver's mood (a sleepy
    island scene) better than crashing waves would.
  - One commenter on the page already asked about looping it, so
    we're not the first to consider this use.

#### Backup: INNORECORDS — "Zen Ocean Waves, Ocean Waves Ambience"

- URL: <https://freesound.org/people/INNORECORDS/sounds/456899/>
- License: **CC0 1.0**
- Duration: 95.8 seconds
- Source format: 44.1 kHz stereo
- Description: "Ambient ocean waves with a zen quality, featuring
  crashing water sounds typical of a beach or shoreline environment."
- Why it's a backup, not primary:
  - "Crashing waves" implies more high-frequency spray content that
    will compress slightly less cleanly at 11 kHz.
  - Slightly shorter (95 sec) so less margin for picking a clean
    loop boundary.
  - Net: fine if the primary doesn't audition well; first-choice if a
    more energetic ocean character is preferred.

#### Notes on candidates we considered and didn't pick

- tim.kahn — *Atlantic Ocean Waves* — high-quality 280-sec recording
  but **CC-BY 4.0**, not CC0. Would be acceptable with attribution
  but adds license-chain complexity for no benefit since we have
  CC0 candidates of similar quality.
- Luftrum — *oceanwavescrushing.wav* — also **CC-BY 4.0**. Skipped
  for the same reason.
- plasterbrain — *(Loop) By the Sea* — CC0, only 16 sec, and it's
  composed music with ocean *on top of* a beat. Not pure ambience.
  Wrong material.
- tim.kahn — *Oceans, Lakes, and Waves* pack — most of the pack is
  CC-BY; check each before sampling.

For v1 ship, **download `376795__amholma__gentle-waves-quiet-beach.wav`,
attribute amholma in `docs/credits/index.html`**, and proceed.

## Sizing the Loop

SPU RAM budget on this branch:

- Total: 512 KB
- System / capture buffers / dummy block (low end): ~4 KB
- Existing SFX (verified by reading `src/sound_ps1.c` — VAG files
  total ~95 KB on disc, header stripped, so ~94 KB on SPU): ~94 KB
- Reserved headroom for any future SFX expansion: ~32 KB
- **Available for ocean loop:** ~380 KB

What that buys at 4-bit ADPCM mono:

| Sample rate | Bytes/sec | 60 sec | 90 sec | 120 sec |
|---|---|---|---|---|
| 8.000 kHz | 4571 | 268 KB | 401 KB | 536 KB |
| 11.025 kHz | 6300 | 369 KB | — (overflow) | — |
| 16.000 kHz | 9143 | — (overflow) | — | — |

**Recommendation: 11.025 kHz mono, 60 seconds = ~369 KB.** Comfortably
inside budget with margin. 11.025 kHz is the highest rate where 60
seconds fits cleanly; any higher and we'd have to drop loop length.
Ocean ambience at 11.025 kHz mono sounds correct for the material.

If we wanted variety, **8 kHz mono and three ~30-second loops** also
fits (~135 KB each, 405 KB total — a hair over). 11 kHz × 60s × 1 is
the cleaner answer.

The pitch shifter on each SPU voice means we can encode at a lower
rate than playback if we ever want to save more space. Ocean is also
forgiving here — pitching down by ~5% is inaudible.

## Authoring Pipeline

The end-to-end transform from a downloaded WAV to a PSn00bSDK-loadable
VAG file:

```
freesound.org CC0 .wav (typically 96 kHz stereo)
  → ffmpeg: downmix to mono, resample to 11025 Hz, normalize peak to ~-3 dBFS
    ffmpeg -i ocean.wav -ac 1 -ar 11025 -af "loudnorm=I=-18:TP=-3" ocean_11k.wav

  → trim to a clean loop boundary at zero crossing (Audacity / sox)
    Find a zero-crossing pair near 60 sec; cut at those samples.

  → encode to Sony VAG ADPCM with loop flag set on last block
    Use PSn00bSDK's VAG converter (look for tools/encvag or wav2vag):
      wav2vag --loop ocean_11k.wav ocean.vag

  → drop into config/ps1/cd_layout.xml under SND/ alongside existing VAGs
```

The "loop flag" detail: Sony ADPCM uses a 16-byte block format with a
flag byte per block. Bit 0 = end-of-sample (key off voice), bit 1 =
loop-end-with-jump-to-loop-start, bit 2 = loop-start. The encoder
needs to set bit 2 on the first block and bit 1 on the last block.
PSn00bSDK's VAG tools have a `--loop` flag or equivalent; verify
exactly which one with the encoder we end up using (Lameguy64's
`vag2wav`/`wav2vag` suite, spicyjpeg's revisions, or others).

## Pause-Menu Toggle: `oceanAmbientEnabled`

The wiring parallels the existing `soundMuted` and `footstepsEnabled`
toggles. Reuse the established pattern.

### Global state

In `src/sound_ps1.c` (or a new `src/ambient_ps1.c` if we want to keep
the SFX path clean):

```c
int oceanAmbientEnabled = 1;     /* default on */
static uint32_t oceanAmbientSpuAddr = 0;   /* set by soundInit */
static uint32_t oceanAmbientSize    = 0;
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
    /* Same upload pattern as the SFX loader: parse rate from the
     * VAG header, DMA the body into SPU RAM, record the address. */
    uint32_t adpcmSize = vagSize - VAG_HEADER_SIZE;
    uint32_t dmaSize   = (adpcmSize + 63u) & ~63u;
    if (spuAddr + dmaSize <= 512 * 1024) {
        /* ... DMA upload identical to the SFX path ... */
        oceanAmbientSpuAddr = spuAddr;
        oceanAmbientSize    = adpcmSize;
        oceanAmbientPitch   = getSPUSampleRate(
                                  (uint16_t)readBE32(vag + 16));
    }
    free(vag);
}

if (oceanAmbientEnabled && oceanAmbientSpuAddr != 0)
    oceanAmbientStart();
```

### Start / Stop

```c
void oceanAmbientStart(void)
{
    if (oceanAmbientSpuAddr == 0) return;
    int ch = OCEAN_AMBIENT_VOICE;
    SPU_CH_FREQ(ch)  = oceanAmbientPitch;
    SPU_CH_ADDR(ch)  = getSPUAddr(oceanAmbientSpuAddr);
    SPU_CH_LOOP_ADDR(ch) = getSPUAddr(oceanAmbientSpuAddr); /* loop start */
    SPU_CH_VOL_L(ch) = 0x2800;  /* slightly under SFX so it sits underneath */
    SPU_CH_VOL_R(ch) = 0x2800;
    /* Long-attack ADSR so a toggle-on doesn't click. */
    SPU_CH_ADSR1(ch) = ...;
    SPU_CH_ADSR2(ch) = ...;
    SPU_KEY_ON = (1u << ch);
}

void oceanAmbientStop(void)
{
    SPU_KEY_OFF = (1u << OCEAN_AMBIENT_VOICE);
}
```

The loop flag set in the source ADPCM means the SPU restarts at the
loop point automatically when it hits the end-of-sample-with-loop
flag byte. No CPU intervention needed.

### Pause-menu row

In `src/pause_menu.c`, add an `OPT_OCEAN_AMBIENT` enum entry between
`OPT_FOOTSTEPS` and `OPT_PERF` (or wherever it fits the existing
ordering). Add a row that:

- displays "Ocean ambience: ON / OFF"
- on toggle, flips `oceanAmbientEnabled` and calls
  `oceanAmbientStart()` / `oceanAmbientStop()`

Same shape as the existing `OPT_FOOTSTEPS` row; this is ~20 lines of
copy-paste.

### Memcard persistence

Bump `MC_VERSION` from 2 to 3 in `src/memcard.c`. Add
`oceanAmbientEnabled` to `JCMCSettings`. Read/write alongside
`soundMuted` and `footstepsEnabled`. Migration is the standard "field
not present in v2 → use default" path the codebase already has.

### Master mute interaction

When `soundMuted` is on, the ocean voice should also be inaudible
(master mute is master). `soundMuteToggle` already manipulates
`SpuSetCommonMasterVolume`; that affects all voices including ours.
No extra wiring needed for master mute.

But: the user might want to mute SFX while keeping ocean on
(scenario: ocean is the screensaver mood, SFX is a distraction). The
pause menu can hold either:

- **One toggle** (`oceanAmbientEnabled`): on/off independently.
- **Two toggles + a sub-policy:** SFX volume slider + ocean volume
  slider. More UI, more memcard, more field changes.

Recommend the single toggle for v1; the volume-slider version is a
follow-up if anyone asks for it.

## Frame-Impact Honest Accounting

| What runs per frame with ocean ambience on | Cost |
|---|---|
| SPU mixing 1 extra voice | 0 cycles (hardware) |
| Loop restart at end of sample | 0 cycles (hardware loop flag) |
| Master volume gate | 0 cycles (already in path) |
| `oceanAmbientStart` / `Stop` | One-shot register writes (only fires on toggle) |

There is zero per-frame CPU cost. There is zero per-frame CD bandwidth.
The screensaver runs at exactly the same FPS / `loop_vb` budget with
ocean ambience on as off, modulo measurement noise. The headless
perf gate (`scripts/ps1-perf-iterate.sh`) should be flat across this
change.

## Memory Cost Summary

| Resource | Before | After | Notes |
|---|---|---|---|
| SPU RAM (512 KB total) | 94 KB used | ~463 KB used | +369 KB ocean loop |
| Main RAM | n/a | +0 | ADPCM lives in SPU RAM only |
| Code (ELF) | — | +~1 KB | Boot loader + start/stop helpers + menu row |
| CD image | — | +~370 KB | One additional VAG file |
| Boot heap | — | +~370 KB peak (DMA-temp during upload, freed) | Same as existing SFX upload pattern |

CD image grows by ~370 KB. Boot ramp grows by ~50-100 ms (extra CD
read + DMA + checksum). After boot, no impact.

## Red-Team / Thesis Review

A defender of the previous-version research should poke at the
following claims; here's the defense.

### "11.025 kHz ADPCM is transparent on ocean material."

**Defended for ocean.** Ocean and surf spectra are dominated by
broadband noise below ~2 kHz with low-energy high-frequency content
above. Sony 4-bit ADPCM's quantization noise is most audible on tonal
content above ~4 kHz; on ocean material it disappears into the
existing high-frequency hiss. This is *not* a general claim about
ADPCM — for music, especially percussive or vocal, 11 kHz is
audibly compressed.

**Concession:** if the source recording is unusually high-frequency-
heavy (recorded on rocks where the spray content peaks above 8 kHz),
artifacts may be audible. Pick a source with broadband-low-frequency
character (long open beach, gentle surf) and this isn't a concern.

### "60-second loop is enough."

This is the most opinionated claim in the doc.

A 60-second ocean loop *will* be perceptible if the user pays attention
— there's a "felt repeat" point around 90-120 seconds for ambient
material. For a screensaver where the user isn't actively listening,
60 seconds is below the "this feels like a loop" threshold. Most
public ambient sound apps loop in this range without complaint.

**Concession:** if a user actually sits and listens, they'll notice
after 5-10 cycles (~5-10 minutes). For a screensaver this is fine.
For a music-listening app it would not be.

### "Hard cut at zero-crossing is enough — no crossfade."

**Defended for ocean.** Wave patterns are stochastic; the listener has
no expectation of phase continuity across a loop boundary. The seam
audibility threshold is much lower than for music. A hard cut at
zero-crossing on both ends produces a click-free seam.

**Caveat:** if the loop point lands mid-wave-roll (during the loud
break-of-wave portion), the energy-level discontinuity at the seam
*can* be perceptible. Pick a loop boundary in a quiet inter-wave
trough.

### "We have ~380 KB SPU RAM available."

The accounting:

- 512 KB total
- 4 KB system reservation at the bottom of SPU RAM
- ~94 KB existing SFX (verified empirically by reading the loader)
- 32 KB headroom for future SFX

Actual remaining: 382 KB. The 369 KB clip fits with 13 KB of margin.
Tight but workable.

**Concession:** if anyone later wants to add a music *layer* (e.g.,
tide-themed cue, holiday music), we run out of room fast. The
single-loop-only direction is by design.

### "Public-domain CC0 licensing is clean for GPL-3 redistribution."

CC0 1.0 is explicit public-domain dedication and compatible with GPL-3.
We attribute in `docs/credits/` for the courtesy / discoverability,
not because the license requires it.

**Concession:** verify the specific recording's license string is
"CC0 1.0 Universal (Public Domain Dedication)" not "CC-BY 4.0" or
"CC-BY-NC 4.0" or "free for personal use." The freesound filter is
the easy way to enforce this.

### "Zero per-frame CPU cost."

**Defended in the steady state.** Once playing, the SPU mixer is
fully autonomous — confirmed in PSn00bSDK's `psxspu.h` and the Sony
SPU register reference. The CPU does nothing per frame.

**Concession:** the *toggle event* (start/stop) is a small set of
register writes that runs in the pause menu's input handler, not in
the screensaver loop. So even the toggle is not in the frame critical
path.

### Things I might still be wrong about

- **PSn00bSDK VAG-encoder loop flag.** The encoder we end up using
  (`wav2vag` from PSn00bSDK or a third-party tool) needs a `--loop`
  flag or equivalent. I have not personally verified the exact CLI
  on the SDK version in use. **Verify before committing source paths.**
- **Memcard schema migration.** Bumping `MC_VERSION` to 3 needs the
  loader to handle v2 → v3 migration cleanly. Existing path does
  this for v1 → v2; mirror the same pattern.
- **SFX channel boundary.** I assumed 8 voices for SFX (round-robin)
  per the existing code. Voice 23 is the highest available. Verify
  no other system uses voices 8-22 (probably none, but check with
  `grep` before committing).

## Concrete Next Steps

1. **Find the source recording.** Filter freesound.org by CC0,
   "ocean waves", duration ≥ 60 sec. Pick a calm, broadband, single-
   character recording (one beach, one wind condition). Download the
   highest-quality WAV.
2. **Encode.** ffmpeg downmix → resample → normalize → trim to clean
   loop boundary → wav2vag with `--loop`. Output `OCEAN.VAG`.
3. **Verify VAG.** Play it in a desktop emulator's standalone VAG
   tester (PSn00bSDK has one) to confirm seamless looping.
4. **Add to CD layout.** One line in `config/ps1/cd_layout.xml`
   under the `<dir name="SND">` block. Re-bake the disc.
5. **Wire boot loader.** Extend `soundInit` in `src/sound_ps1.c` to
   load the new VAG into SPU RAM after the existing SFX block.
6. **Add start/stop helpers.** `oceanAmbientStart()` /
   `oceanAmbientStop()` in `src/sound_ps1.c` (or a new
   `src/ambient_ps1.c`). ADSR with slow attack/release so toggles
   don't click.
7. **Wire pause-menu row.** Add `OPT_OCEAN_AMBIENT` to the menu enum;
   render row; toggle calls start/stop.
8. **Persist to memcard.** Bump `MC_VERSION` to 3; add field to
   `JCMCSettings`; serialize/deserialize alongside existing fields.
9. **Run perf gate.** `scripts/ps1-perf-iterate.sh --scene fishing1`
   should be flat (loop_vb / blocking_vb / overrun_vb identical to
   baseline). If it isn't, something the research missed.
10. **Add credits row.** Update `docs/credits/index.html` with the
    freesound.org username and the source URL.

## Creative Angles Worth Considering

If the simple "one loop" version ships and works, these are cheap
follow-ups the SPU enables for free.

1. **Tide-aware mix.** `islandState.lowTide` is already in the state
   key. Two short loops — calm rolling waves for high tide,
   tide-pool / shore detail for low tide — at 8 kHz × 30 sec each
   would fit (~270 KB total). Switch at sequence boundary.
2. **Subtle volume bias by night/day.** `islandState.night` flips
   per session. At night, drop ocean volume slightly (e.g., 0x2000
   → 0x1800) for a "evening hush" feel. One register write per
   session start.
3. **Holiday-tagged ambience.** Each holiday in `gHolidays[]` could
   carry an optional ambience track. Christmas could replace the
   ocean with a quieter winter-shore loop. This is a *data* change
   (new VAG per holiday) more than a code change. Memory tight at
   one slot per holiday; better as a future XA-track design.
4. **Wind layer.** A second voice playing a low-amplitude wind loop
   at variable volume can sit underneath the ocean. ~50 KB extra.
   Adds depth at near-zero cost.
5. **One-shot stings on holidays.** Standard SPU voices can fire
   short cues on holiday days (Christmas chime, July 4 distant
   firework crackle). 4-8 KB each, plays on a spare voice without
   touching the ambience loop.

These are all "nice to have"; the v1 ship is the single toggleable
loop.

## Files Worth Reading Before Implementation

- `src/sound_ps1.c` — existing SFX path, SpuInit, VAG loader, mute.
- `src/pause_menu.c` — existing toggle rows (`OPT_FOOTSTEPS`,
  `OPT_CAPTIONS`); copy this pattern.
- `src/memcard.c` — `JCMCSettings`, `MC_VERSION`, migration pattern.
- `scratch/psn00b-src/examples/sound/vagsample/main.c` — minimal
  one-shot VAG playback example.
- `scratch/psn00b-src/libpsn00b/include/psxspu.h` — full SPU API,
  voice loop register macros.
- `config/ps1/cd_layout.xml` — where to add the new VAG entry.
- `docs/credits/index.html` — where the source attribution goes.
- Nocash PSX docs (`https://problemkaputt.de/psx-spx.htm`) §SPU for
  the ADPCM block-flag bit semantics.
