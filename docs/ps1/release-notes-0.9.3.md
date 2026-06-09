# v0.9.3-ps1 Release Notes

**Date:** 2026-06-09
**Tag:** `v0.9.3-ps1`
**Theme:** SPU async staging and Original-order soak release

`v0.9.3-ps1` promotes the async-staging branch after the long-run soak held
clean. This is the release where unused SPU RAM becomes a first-class runtime
storage surface and scene prefetch starts using non-blocking CD reads instead
of waiting for the transition boundary to do all of the work.

## Headline

- **Unused SPU RAM is now a cold cache.** The runtime initializes an
  SPU-backed cache after the audio bank and ocean ambience reserve their
  samples. Data parked there is not directly CPU-addressable; callers DMA it
  into SPU RAM with `SpuWrite()` and DMA it back with `SpuRead()` before CPU
  or GPU use.
- **The SPU cache has a release layout.** The first release allocation keeps
  8 KB for next-scene metadata, 64 KB for next-scene foreground payload,
  48 KB for `JOHNWALK.PSB`, 144 KB for the walk clean buffer, and 12 KB for
  `MRAFT.PSB`.
- **Foreground transition staging uses non-blocking CD reads.** The next-scene
  stage starts aligned async reads for pack metadata and payload windows,
  polls for completion during held slack, and drains the async path before any
  later blocking read or buffer mutation.
- **Walk scenes no longer need the large main-RAM clean allocation.** The walk
  clean snapshot is captured to SPU cold storage and restored from there,
  avoiding the roughly 130 KB main-RAM allocation that was the fragile part of
  long Original-order soaks.

## Why This Release Exists

The original branch goal was "keep the ocean waves animating while the next
scene loads." The wave-ticking experiments proved too invasive: they could
move animation out of phase, create catch-up bursts, or still leave a frozen
transition when the CD path blocked.

The useful part of the experiment was the bigger model: start reading the next
scene before the transition boundary, park what has already landed somewhere
other than the main scene stream buffer, and make all blocking reads wait for
the async path to become quiet first. SPU RAM is a good fit for that parking
lot because its DMA transfer is far faster than another CD seek, and because
the port has substantial unused SPU RAM after its sound assets load.

## Verification

- `./scripts/build-ps1.sh` and `./scripts/make-cd-image.sh` built the PS1 disc
  image successfully before release.
- The 420-second timed proof in
  `scratch/duckstation-longrun-20260609-092955/` booted the Original-order
  picker, advanced through varied scenes and walk scenes, and logged no
  `JCWALK` allocation failures or fatal markers.
- The visible long soak in `scratch/duckstation-longrun-20260609-093902/`
  ran from `2026-06-09T09:39:02-07:00` to
  `2026-06-09T12:03:28-07:00` and exited with code `0`.
- That soak reached roughly 293 Original-order picks with recurring SPU walk
  loads and no `JCBSOD`, fatal, invalid-read, or `JCWALK` allocation-failure
  markers.
- Two nonfatal walk frame caps were still observed during the long soak. They
  forced `walkDone` after 601 frames instead of crashing the run, and remain a
  follow-up polish item rather than a release blocker.

## Notes

The SPU cache is deliberately described as cold storage. It is not a new heap
and it is not random-access RAM from the CPU's point of view. Its value is
that it lets the runtime free scarce main RAM after a CD read has landed, then
pull the bytes back quickly when the destination scene or walk segment actually
needs them.
