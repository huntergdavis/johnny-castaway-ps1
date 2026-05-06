# v0.7.2-ps1 Release Notes

**Date:** 2026-05-05
**Tag:** `v0.7.2-ps1`
**Theme:** story-loop walk backdrop guard

`v0.7.2-ps1` fixes a randomized story-loop walking regression where Johnny
could walk over water and leave repeated walking poses when the next scene's
island backdrop state differed from the framebuffer left by the previous
scene.

## Headline

- **Walks now require a matching backdrop key.** The runtime remembers the
  island state that produced the previous scene framebuffer and only allows
  an inter-scene walk when the next scene uses the same tide, raft, night,
  holiday, and island X/Y values.
- **Scene-policy changes now force a clean scene load.** Moving from a
  variable-position scene to a fixed/left-island/no-raft/tide/holiday variant
  inside the same story sequence no longer draws Johnny over stale water.
- **Menu and freeplay resets clear the walk context.** Scene Set changes,
  Scene Explorer launches, and Freeplay exits all invalidate the remembered
  backdrop so the next scene owns its own background.

## Root Cause

The previous guard only skipped walking when the story sequence counter
rerolled. That was insufficient because scene-specific policies can change the
actual island backdrop inside an otherwise continuing sequence. The result was
a stale framebuffer from one island placement plus walking coordinates for the
next placement, which made Johnny appear over the ocean and left repeated walk
poses.

## Verification

Release candidate checks:

- User watched the rebuilt candidate in DuckStation and did not reproduce the
  water-walk trail.
- `./scripts/build-ps1.sh clean`
- `./scripts/make-cd-image.sh`

The primary acceptance gate remains human visual and audible signoff.
