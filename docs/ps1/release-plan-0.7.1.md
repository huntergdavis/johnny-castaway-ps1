# v0.7.1-ps1 Planned Feature Notes

**Theme:** persisted options and holiday-mode polish
**Status:** planned after `v0.7.0-ps1`

`v0.7.0-ps1` freezes the complete-scene milestone: all 63 scenes are validated.
`v0.7.1-ps1` should be the first follow-up feature release, focused on making
the pause-menu options behave like durable player settings.

## Memcard Persistence

The current save schema (`MC_VERSION` 5) persists sound mute, ocean ambience,
story day, day/night override, holiday override, soft clock/date, and scene
picker policy. The `v0.7.1` target is to bump the schema and persist the
remaining user-facing options:

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

Backward compatibility rule: v5 saves must still load and default the new
fields to current boot defaults.

## Holiday Mode

Add a new holiday mode:

- **AUTO DATE: ORIGINAL 4**

This should be date-driven like today's `AUTO DATE`, but restricted to the
four Sierra-era holiday overlays:

- Halloween;
- St. Patrick's Day;
- Christmas;
- New Year's.

For `v0.7.1`, this should become the fresh-boot default when no memory card is
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
- Fresh boot with no save uses `AUTO DATE: ORIGINAL 4` as the default holiday
  policy.
- Save immediately after a fresh boot produces deterministic defaults.
- Loading a v5 save produces the same behavior users had before `v0.7.1`.
- The new holiday mode is visible in the pause menu and survives power cycle.
- Full-scene validation remains green; no scene should need revalidation just
  because a saved option exists.
