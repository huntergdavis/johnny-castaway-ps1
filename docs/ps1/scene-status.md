# PS1 Scene Pipeline Status

> 🌐 **Rendered version:** **[/scenes/](https://hunterdavis.com/johnny-castaway-ps1/scenes/)** — this doc rendered on the project website with cross-links and prose context. The GitHub copy here is the source.


Tracks per-scene readiness under the **fishing1 bar**: pixel-perfect visuals
plus synced SFX, across every applicable variant.

**Legend**
- ✅ done (committed, on main release line)
- ⏳ not yet
- ~~strike~~ variant not applicable to this scene
- — TBD (will be filled in when the scene is worked)

## Progress: 27 / 63 (next: stand7)

Milestone scene releases should be cut every 10 ✅/✅ scenes under this
bar. Smaller stability releases may happen between milestones; the
current public release is `v0.6.10-ps1`.

| ADS | Tag | Slug | Visuals | SFX | Variants | Last verified | Notes |
|-----|-----|------|:-:|:-:|---|---|---|
| ACTIVITY | 1 | activity1 | ⏳ | ⏳ | — | — |  |
| ACTIVITY | 4 | activity4 | ⏳ | ⏳ | — | — |  |
| ACTIVITY | 5 | activity5 | ⏳ | ⏳ | — | — |  |
| ACTIVITY | 6 | activity6 | ⏳ | ⏳ | — | — |  |
| ACTIVITY | 7 | activity7 | ⏳ | ⏳ | — | — |  |
| ACTIVITY | 8 | activity8 | ⏳ | ⏳ | — | — |  |
| ACTIVITY | 9 | activity9 | ⏳ | ⏳ | — | — |  |
| ACTIVITY | 10 | activity10 | ⏳ | ⏳ | — | — |  |
| ACTIVITY | 11 | activity11 | ⏳ | ⏳ | — | — |  |
| ACTIVITY | 12 | activity12 | ⏳ | ⏳ | — | — |  |
| BUILDING | 1 | building1 | ⏳ | ⏳ | — | — |  |
| BUILDING | 2 | building2 | ⏳ | ⏳ | — | — |  |
| BUILDING | 3 | building3 | ⏳ | ⏳ | — | — |  |
| BUILDING | 4 | building4 | ⏳ | ⏳ | — | — |  |
| BUILDING | 5 | building5 | ⏳ | ⏳ | — | — |  |
| BUILDING | 6 | building6 | ⏳ | ⏳ | — | — |  |
| BUILDING | 7 | building7 | ⏳ | ⏳ | — | — |  |
| FISHING | 1 | fishing1 | ✅ | ✅ | night · low-tide · holiday · raft-stage | v0.3.6-ps1 | reference scene; template for remaining |
| FISHING | 2 | fishing2 | ✅ | ✅ | night · low-tide · holiday · raft-stage | 2026-04-23 |  |
| FISHING | 3 | fishing3 | ✅ | ✅ | night · low-tide · holiday · raft-stage | 2026-05-01 | visual + audible signoff on PS1/DuckStation from `v0.6.1-ps1` baseline |
| FISHING | 4 | fishing4 | ✅ | ✅ | night · low-tide · holiday · raft-stage | 2026-05-01 | visual + audible signoff after `LEFT_ISLAND` fgpilot draw-offset fix |
| FISHING | 5 | fishing5 | ✅ | ✅ | night · low-tide · holiday · raft-stage | 2026-05-02 | visual + audible signoff after rebuilding high/low FG2 with a full-frame keyed current-ledger overlay; stale full-host shark overpaint and outline-only foreground-mask shark frames are fixed |
| FISHING | 6 | fishing6 | ✅ | ✅ | night · low-tide · holiday · raft-stage | 2026-05-01 | visual + audible signoff after terminal FGP3 cleanup fix for splash/pole residue |
| FISHING | 7 | fishing7 | ✅ | ✅ | night · low-tide · holiday · raft-stage · host capture/test island position `x=-300,y=54` · production variable island position | 2026-05-03 | visual + audible signoff after rebuilding high/low packs from a far-left full-frame foreground-only capture; far-left stress playback proved the recaptured scene-relative pixels are complete, so the old runtime island-position pin is removed |
| FISHING | 8 | fishing8 | ✅ | ✅ | night · low-tide · holiday · raft-stage · host capture/test island position `x=-300,y=54` · production variable island position | 2026-05-03 | visual + audible signoff after the same full-frame foreground-only recapture pattern as `FISHING 7`; pack completeness is proven at the far-left stress position and production playback stays random-position safe |
| JOHNNY | 1 | johnny1 | ✅ | ✅ | black backdrop · high/low pack parity | 2026-05-02 | visual signoff after full-screen black-backdrop playback fixed placement/scaling and removed the clean-rect memory pressure; saved memcard mute is applied before audio init |
| JOHNNY | 2 | johnny2 | ✅ | ✅ | captured island position `x=-64,y=54` · high/low pack parity | 2026-05-02 | visual + audible signoff after rebuilding high/low FG2 with lower-band keyed overlay cleanup and hold-advance/hold-adjust timing for the island/SOS thought bubbles |
| JOHNNY | 3 | johnny3 | ✅ | ✅ | night · low-tide · holiday · raft-stage · variable island position | 2026-05-02 | visual + audible signoff after right-shift island probe confirmed the full source pixels are present; no captured-position pin is required |
| JOHNNY | 4 | johnny4 | ✅ | ✅ | host capture/test island position `x=-64,y=54` · high/low pack parity · production variable island position | 2026-05-03 | visual + audible signoff after rebuilding high/low packs with a full-frame keyed foreground-only overlay; stale bottle overpaint and full-host SOS bubble blue-line contamination are fixed without adding a runtime island-position pin |
| JOHNNY | 5 | johnny5 | ✅ | ✅ | host capture/test island position `x=80,y=54` · high/low pack parity · production variable island position | 2026-05-03 | visual + audible signoff after rebuilding high/low packs at the current-position host capture so the thrown-bottle splash is in frame; full-frame keyed foreground-only overlay fixes stale lower-band overpaint, and SOS note timing now holds on the note instead of the blank post-bubble rows |
| JOHNNY | 6 | johnny6 | ✅ | ✅ | black backdrop · high/low pack parity | 2026-05-03 | visual + audible signoff after routing the scene through the full-screen black-backdrop runtime path; no ocean/island background is painted |
| MARY | 1 | mary1 | ✅ | ✅ | night · low-tide · holiday · raft-stage · validation route `x=-124,y=37` · raft-stage `5` | 2026-05-03 | visual + audible signoff on the legacy MARY1 route; no pack or runtime changes required |
| MARY | 2 | mary2 | ✅ | ✅ | night · low-tide · holiday · raft-stage · host capture/test island positions `x=-154,y=54`, `x=80,y=54`, `x=300,y=54` · production variable island position | 2026-05-03 | visual + audible signoff after rebuilding high/low packs from a wide scene-relative multi-view stitch: foreground-only captures restore the line, mermaid, boot/splash, and lower-water action across island placements, while full-host bubble injection restores the fish thought-bubble shell; far-right and true far-left stress playback passed |
| MARY | 3 | mary3 | ✅ | ✅ | night · low-tide · holiday · raft-stage · host capture/test island position `x=80,y=54` · production variable island position | 2026-05-03 | visual + audible signoff after rebuilding high/low packs from a far-right full-frame keyed foreground-only capture; host capture now invalidates stale captured sprite surfaces before BMP/layer frees, MARY3 uses low-memory clean-snapshot relief, and the late dinner/thought beat holds on the readable frames |
| MARY | 4 | mary4 | ✅ | ✅ | night · low-tide · holiday · raft-stage · host capture/test island positions `x=-154,y=54`, `x=-300,y=54`, `x=300,y=54` · production variable island position | 2026-05-03 | visual + audible signoff after rebuilding high/low packs through the generic multi-view scene-relative stitch; normal, far-left, and far-right foreground-only captures restore both sides of the island-relative action, and far-right stress playback at `x=300,y=54` passed |
| MARY | 5 | mary5 | ✅ | ✅ | night · low-tide · holiday · ~~raft-stage~~ (`NORAFT`) · full-wipe scene (`FIRST`, no walk prelude) · host capture/test island positions `x=-154,y=54`, `x=-300,y=54`, `x=300,y=54` · production variable island position | 2026-05-03 | visual + audible signoff after rebuilding high/low packs through the generic multi-view scene-relative stitch with raft-stage `0`; runtime story-flag policy now clamps `NORAFT` scenes to no generic raft even when a broad `raft-stage` token is present, and `FIRST` scenes skip the walk-in before the frog/full-wipe transition |
| MISCGAG | 1 | miscgag1 | ✅ | ✅ | night · low-tide · holiday · raft-stage · host capture/test island positions `x=-154,y=54`, `x=-300,y=54`, `x=300,y=54` · production variable island position | 2026-05-03 | visual + audible signoff after regenerating high/low packs through the generic normal/far-left/far-right foreground-only multi-view stitch; validated on the normal high-tide/night route |
| MISCGAG | 2 | miscgag2 | ✅ | ✅ | night · ~~low-tide~~ (not randomized by story flags; high/low packs regenerated) · holiday · raft-stage · host capture/test island positions `x=-154,y=54`, `x=-300,y=54`, `x=300,y=54` · production variable island position | 2026-05-03 | visual + audible signoff after regenerating high/low packs through the generic normal/far-left/far-right foreground-only multi-view stitch; validated on the normal high-tide/night route |
| STAND | 1 | stand1 | ✅ | ✅ | night · low-tide · holiday · raft-stage · host capture/test island positions `x=-154,y=54`, `x=-300,y=54`, `x=300,y=54` · production variable island position | 2026-05-03 | visual signoff after regenerating high/low packs through the generic multi-view stitch; source pack is a short 35-frame / 169-vblank idle loop with no captured SFX events, so the hard-to-see reset is expected |
| STAND | 2 | stand2 | ✅ | ✅ | night · low-tide · holiday · raft-stage · validation route `x=-154,y=54` | 2026-05-04 | visual signoff on the normal high-tide/night route; short pants-adjust idle loop played cleanly with no visible residue or scene-loader walk prelude |
| STAND | 3 | stand3 | ✅ | ✅ | night · low-tide · holiday · raft-stage · validation route `x=-154,y=54` | 2026-05-04 | visual signoff on the normal high-tide/night route; short hat-lift idle loop played cleanly |
| STAND | 4 | stand4 | ✅ | ✅ | night · low-tide · holiday · raft-stage · host capture/test island positions `x=-154,y=54`, `x=-300,y=54`, `x=300,y=54` · production variable island position | 2026-05-04 | visual signoff after regenerating high/low packs through the generic multi-view stitch; tapping-foot idle loop played cleanly on the normal high-tide/night route |
| STAND | 5 | stand5 | ✅ | ✅ | night · low-tide · holiday · raft-stage · validation route `x=-154,y=54` · no-stitch host export with full-frame foreground-only overlay | 2026-05-04 | visual signoff after regenerating high/low packs through the STAND no-stitch fast path; first no-stitch attempt faded Johnny's legs because pure base-diff treated frame-0 static pixels as background, so the fast path now keeps a single-position foreground-only overlay |
| STAND | 6 | stand6 | ✅ | ✅ | night · low-tide · holiday · raft-stage · validation route `x=-154,y=54` · no-stitch host export with full-frame foreground-only overlay | 2026-05-04 | visual signoff on the normal high-tide/night route after regenerating high/low packs through the STAND no-stitch fast path |
| STAND | 7 | stand7 | ⏳ | ⏳ | — | — |  |
| STAND | 8 | stand8 | ⏳ | ⏳ | — | — |  |
| STAND | 9 | stand9 | ⏳ | ⏳ | — | — |  |
| STAND | 10 | stand10 | ⏳ | ⏳ | — | — |  |
| STAND | 11 | stand11 | ⏳ | ⏳ | — | — |  |
| STAND | 12 | stand12 | ⏳ | ⏳ | — | — |  |
| STAND | 15 | stand15 | ⏳ | ⏳ | — | — |  |
| STAND | 16 | stand16 | ⏳ | ⏳ | — | — |  |
| SUZY | 1 | suzy1 | ⏳ | ⏳ | — | — |  |
| SUZY | 2 | suzy2 | ⏳ | ⏳ | — | — |  |
| VISITOR | 1 | visitor1 | ⏳ | ⏳ | — | — |  |
| VISITOR | 3 | visitor3 | ⏳ | ⏳ | — | — |  |
| VISITOR | 4 | visitor4 | ⏳ | ⏳ | — | — |  |
| VISITOR | 5 | visitor5 | ⏳ | ⏳ | — | — |  |
| VISITOR | 6 | visitor6 | ⏳ | ⏳ | — | — |  |
| VISITOR | 7 | visitor7 | ⏳ | ⏳ | — | — |  |
| WALKSTUF | 1 | walkstuf1 | ⏳ | ⏳ | — | — |  |
| WALKSTUF | 2 | walkstuf2 | ⏳ | ⏳ | — | — |  |
| WALKSTUF | 3 | walkstuf3 | ⏳ | ⏳ | — | — |  |

## Per-scene workflow

For each iteration:

1. Run `./scripts/export-scene-foreground-pilot.sh <output_dir> <slug> '<ADS TAG>' <PACK_BASENAME> 0 1.0 <LOW_PACK_BASENAME>`
   to produce the high-tide and low-tide `.FG2` packs and sound-event JSONL.
   The default export path captures normal, far-left, and far-right
   foreground-only host views and stitches them into one scene-relative
   foreground canvas, because island-relative content can extend beyond a
   single screen-width capture. Opt out only for validated scene-specific
   paths such as `JOHNNY 2`.
2. Confirm both `<SCENE>.FG2` and low-tide `<LOW_PACK_BASENAME>.FG2` entries exist in `config/ps1/cd_layout.xml`.
3. Confirm the scene's routing entries exist in `foreground_pilot.c`
   (`fgCompactOverlayPackPathForScene`).
4. `./scripts/make-cd-image.sh` then launch via `rebuild-and-let-run.sh noclean`.
5. User verifies; iterate on bugs.
6. After ✅ visual+audible signoff, run
   `./scripts/capture-scene-explorer-thumbnail.sh <slug>` to grab the
   70%-mark frame as a Scene Explorer thumbnail. The script invokes the
   headless DuckStation regtest pipeline, picks the 70th-percentile
   frame from the captured sequence, and writes
   `jc_resources/extracted/scr/SX<FAM2><TAG>.SCR` (320x240 RGB555).
   Add a matching `<file name="SX<FAM2><TAG>.SCR" ...>` entry under
   `<dir name="SCR">` in `config/ps1/cd_layout.xml` and commit the SCR
   alongside the validation update.
7. Update this table, commit. Every 10 ✅/✅ rows → `./scripts/release.sh "<milestone message>"`.

FG1 packs, FOC packs, per-scene establishing RAWs, and direct/fallback
FG1 runtime routes are retired. They are historical archaeology only, not
valid scene bring-up inputs.

## Variant definitions

- **night** — dusk/night palette, BOOTMODE `night 1`
- **low-tide** — tide state variant, BOOTMODE `lowtide 1`
- **holiday** — holiday overlay variants (christmas/halloween/etc), BOOTMODE `holiday N`
- **raft-stage** — cumulative raft-build state, BOOTMODE `raft-stage N`
