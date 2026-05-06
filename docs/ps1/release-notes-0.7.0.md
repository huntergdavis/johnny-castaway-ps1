# v0.7.0-ps1 Release Notes

**Date:** 2026-05-05
**Tag:** `v0.7.0-ps1`
**Theme:** Complete scene validation

`v0.7.0-ps1` is the first Johnny Castaway PS1 release where every original
scene is validated under the project's current bar: pixel-perfect visual
playback plus synced SFX, reviewed by human signoff across the applicable
night, low-tide, holiday, and raft-stage variants.

This is the line where scene coverage stops being speculative. All 63 scenes
can play correctly on the PS1 port. Future mainline work can now focus on
bugfixes, speed, memory pressure, release polish, and new features.

## Headline

- **63 / 63 scenes validated.** The live ledger is fully green under the
  FISHING 1 reference bar.
- **ACTIVITY 9 completed the sweep.** The final scene needed a wide-boat
  repair path because `BOAT.PSB` can extend past the legacy 640px scene clip.
- **The landing page now uses the Activity9 boat screenshot.** The new image
  shows the final validated scene running in the PS1 build.

## Final Scene Fix

`ACTIVITY 9` was rebuilt through an Activity9-specific wide stitch using
host capture/test island positions `x=-500,y=54`, `x=-154,y=54`, and
`x=500,y=54`. Production playback remains variable-position safe.

The new `scripts/patch-activity9-boat-foreground.py` helper fills missing
`BOAT.PSB` bow/stern pixels from the decoded source sprite at the legacy clip
edges, only into keyed foreground holes. It also adds a narrow clip-edge
overlap to remove the visible stitch seam and carries the last boat draw
across metadata-held frames so the late bow no longer flickers.

## Validation State

The validated families are:

- ACTIVITY 1, 4, 5, 6, 7, 8, 9, 10, 11, and 12.
- BUILDING 1 through 7.
- FISHING 1 through 8.
- JOHNNY 1 through 6.
- MARY 1 through 5.
- MISCGAG 1 and 2.
- STAND 1 through 12, 15, and 16.
- SUZY 1 and 2.
- VISITOR 1, 3, 4, 5, 6, and 7.
- WALKSTUF 1, 2, and 3.

## What Comes Next

The project is not finished, but the risk profile changed. The remaining work
is no longer "can every scene work?" It is:

- bugfixing regressions found during broader play;
- speed and loading optimization against the headless battle card;
- memory-pressure reduction and real-hardware verification;
- feature polish around freeplay, scene selection, settings, captions, and
  release packaging.

The first planned follow-up is `v0.7.1-ps1`: persisted pause-menu options and
holiday-mode polish. That work is intentionally not part of this release. The
target list is in [release-plan-0.7.1.md](release-plan-0.7.1.md) and includes
persisting captions, Scene Set, tide, raft, island-position, freeplay/perf
preferences, plus a new **AUTO DATE: ORIGINAL 4** holiday mode that becomes
the no-card/fresh-save default in 0.7.1.

## Verification

Release candidate checks:

- `ACTIVITY 9` visual playback signed off in DuckStation after the wide-boat
  stitch and held-frame repair.
- Scene ledger source updated to 63 / 63.
- Website source data and landing-page screenshot updated for the complete
  validation milestone.
- PS1 ISO rebuilt from the release tree.

The primary acceptance gate remains human visual and audible signoff. The
headless and build automation remain the guardrails for routing, packaging,
and future performance work.
