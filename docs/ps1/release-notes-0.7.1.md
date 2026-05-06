# v0.7.1-ps1 Release Notes

**Date:** 2026-05-05
**Tag:** `v0.7.1-ps1`
**Theme:** persisted holiday mode and first-run defaults

`v0.7.1-ps1` is the first post-complete-scene point release. It keeps the
63/63 scene-validation milestone intact and focuses on settings behavior around
holiday overlays.

## Headline

- **AUTO DATE:ORIG4 is now the fresh/no-card default.** New boots start with
  Sierra's original four holiday overlays as the automatic policy.
- **Holiday mode is persisted separately from manual selection.** The memory
  card schema is now v6 and stores `holidayMode` independently from
  `holidayOverride`.
- **Manual and automatic holiday paths are distinct.** The pause menu now
  exposes `AUTO DATE:ORIG4`, `AUTO DATE`, `NONE`, `ORIGINAL 4`, and `EXPANDED`
  as separate policies.
- **Walking holiday z-order was corrected.** Story-loop walk frames stamp
  holiday overlays before Johnny so default holiday decorations do not paint
  over the walking sprite.

## Compatibility

Existing v2-v5 saves still load. Old automatic holiday saves migrate to the
new `AUTO DATE:ORIG4` default. Manual holiday selections and `NONE` remain
manual. New saves write schema v6.

The date picker still drives automatic holiday selection when soft time/date is
enabled. `AUTO DATE:ORIG4` restricts that lookup to Halloween, St. Patrick's
Day, Christmas, and New Year's; `AUTO DATE` keeps the expanded generated
holiday table.

## Known Follow-up

A walking regression that predates this point release can still place Johnny
over water during some randomized story-loop transitions and leave repeated
walk poses on the ocean. That is tracked as the first `v0.7.2` bugfix target.
It is not part of the holiday-mode change and should be fixed in the next pass
without holding this release.

## Verification

Release candidate checks:

- Fresh/default holiday menu behavior visually checked in DuckStation.
- Story-loop walk holiday overlay ordering rebuilt and visually checked.
- `./scripts/build-ps1.sh clean`
- `./scripts/make-cd-image.sh`

The primary acceptance gate remains human visual and audible signoff.
