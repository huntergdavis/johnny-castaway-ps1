# v0.9.2-ps1 Release Notes

**Date:** 2026-05-31
**Tag:** `v0.9.2-ps1`
**Theme:** long-run CD and VISITOR3-low stability release

`v0.9.2-ps1` promotes the long-term soak fix from mainline. It is a
stability point release, not a speed-claim release: the goal is to remove the
two rare failure modes found only after extended randomized scene playback.

## Headline

- **CD directory searches now wait for the prior read path to become quiet.**
  `ps1_cdSearchFileQuiesced()` drains `CdReadSync(0, NULL)` before
  `CdSearchFile()` so PSn00bSDK's async `CdlPause` completion cannot clear the
  parameter FIFO while a later directory walk is issuing `CdlSetloc`.
- **VISITOR3-low no longer holds the clean-relief stream window.** High tide
  keeps its reduced VISITOR3 relief window, but low tide drops the 48 KB grouped
  prefetch window that competed with the low setup tail and five split
  clean-rect strips.
- **The VISITOR3-low fix is tide-specific.** Other scenes do not take the
  memory trade, and VISITOR3-high keeps the high-tide relief path that already
  works.

## Why This Release Exists

The prior long soak did not fail on scene routing or visual playback. It failed
after repeated VISITOR3-low hits when the final small clean-rect allocation had
to find room in a fragmented CACHE/libc fallback mix:

- `JCBSOD-FATAL CACHE exhausted (region+libc both)`
- scene `visitor3`, pack `VIST3LOW.FG2`
- the failure happened after dozens of successful low-tide VISITOR3 plays

The fix removes only the low-tide window allocation that made the final strip
depend on fragmentation luck. The earlier CD quiesce fix remains in the same
release because it closed the separate long-run `CdlSetloc` zero-parameter
failure seen during scene transitions.

## Verification

- `./scripts/build-ps1.sh` builds successfully.
- Targeted VISITOR3-low boot:
  `RUN_TIMEOUT_SECONDS=360 ... ./scripts/rebuild-and-let-run.sh noclean fgpilot visitor3 lowtide 1 ...`
  loaded `MRAFT.PSB` and `VIST3LOW.FG2`, logged
  `JCMEM clean-relief scene=visitor3 ... no-prefetch`, and did not allocate
  `fg-stream-window`.
- The follow-up randomized soak was intentionally stopped for this release
  rebuild after about 47.5 hours. Preserved log:
  `scratch/duckstation-soak-before-release-20260531-073555.log`. It had no
  `JCBSOD`, `JCSKIP`, `CACHE exhausted`, or DuckStation `Incorrect parameters`
  markers before the stop.
- The release script rebuilds the PS1 executable, CD image, and static website
  again before tagging.
