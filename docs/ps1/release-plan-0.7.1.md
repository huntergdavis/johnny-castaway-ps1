# v0.7.1-ps1 Feature Notes

**Theme:** persisted options and holiday-mode polish
**Status:** released as a narrow holiday-mode point release

`v0.7.0-ps1` freezes the complete-scene milestone: all 63 scenes are validated.
`v0.7.1-ps1` became the first follow-up point release, focused specifically on
holiday mode defaults and persistence. The broader persisted-options list below
remains useful planning material, but captions, Scene Set, tide, raft, island
position, freeplay/perf preferences, and menu cursor defaults did not ship in
0.7.1.

## Memcard Persistence

The save schema is now `MC_VERSION` 6. It persists sound mute, ocean ambience,
story day, day/night override, holiday override, soft clock/date, scene picker
policy, and the new `holidayMode` field. The remaining user-facing options for
future releases are:

- captions enabled;
- active Scene Set;
- tide override;
- raft-stage override;
- island-position override and X/Y values;
- freeplay enabled/disabled state, if preserving that across boot still feels
  right after testing;
- perf logging level;
- any menu cursor defaults we decide are user preference rather than transient
  UI state.

Backward compatibility rule: v5 saves must still load. Old automatic holiday
saves migrate to the new `AUTO DATE:ORIG4` default; manual selections and
`NONE` remain explicit.

## Holiday Mode

Add a new holiday mode:

- **AUTO DATE: ORIGINAL 4**

This should be date-driven like today's `AUTO DATE`, but restricted to the
four Sierra-era holiday overlays:

- Halloween;
- St. Patrick's Day;
- Christmas;
- New Year's.

For `v0.7.1`, this became the fresh-boot default when no memory card is
installed and the first-time default written to a new save. In other words:
users without saved settings get the original Sierra holiday behavior by
default, while still being able to opt into the expanded generated holiday
calendar from the pause menu.

The existing expanded holiday table should remain available through the current
full auto-date path. The intended menu shape is:

- `AUTO DATE: ORIGINAL 4` — date-driven, original four only; default for
  no-card/fresh-save `v0.7.1` boot.
- `AUTO DATE` — date-driven, all generated holidays.
- `NONE` — no holiday overlay.
- `ORIGINAL 4` — manual selection among the original four.
- `EXPANDED` — manual selection among generated expanded holidays.

The persisted representation should distinguish automatic mode from manual
selection. Do not overload a positive holiday id to mean an auto policy.

## Acceptance Bar

- Fresh boot with no save keeps the existing defaults except holiday mode.
- Fresh boot with no save uses `AUTO DATE:ORIG4` as the default holiday policy.
- Save immediately after a fresh boot writes schema v6 with the mode separate
  from the manual holiday id.
- Loading a v5 auto save migrates to `AUTO DATE:ORIG4`; v5 manual holiday and
  `NONE` settings remain explicit.
- The new holiday mode is visible in the pause menu and survives power cycle.
- Full-scene validation remains green; no scene should need revalidation just
  because a saved option exists.
