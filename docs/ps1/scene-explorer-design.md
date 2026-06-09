# Scene Explorer — design doc

> Rendered version: `/docs/scene-explorer/` on the project website.

A new pause-menu sub-screen that lets the player browse all 63 scenes
in the catalog with a full-screen thumbnail, title, family, frame
count, and validation status. Cross plays the highlighted scene
immediately (via the same scene-pin path the freeplay rotation already
uses).

## UX

Single-up pager layout. Replaces one slot in the main pause menu.

```
┌──────────────────────────────────┐
│                                  │
│                                  │
│      [320×208 thumbnail]         │
│      (full source frame,         │
│       PS1 native 256-color)      │
│                                  │
│                                  │
├──────────────────────────────────┤
│ FISHING 1 ✅  12/63              │
│ Family: Fishing  Frames: 240     │
│ X play   O back   L1/R1 family   │
└──────────────────────────────────┘
```

The bottom **32 rows (320×32)** are a black overlay band carrying the
metadata + nav hints; the top **208 rows** show the thumbnail
unmodified. The thumbnail itself is 320×208 (cropped on extraction
from the 320×240 source by trimming the bottom 32 rows, which are
typically just ocean/sand).

| Input | Action |
|---|---|
| Left / Right | Previous / next scene |
| L1 / R1 | Jump to previous / next family (Fishing → Johnny → Mary → Visitor → Activity → Misc/Suzy → Stand → Walkstuf → Building → Miscgag) |
| Cross | Play this scene **once** and exit the menu — the next loop iteration plays it, then the screensaver returns to the active Scene Set |
| Triangle | Play this scene on **loop** — same scene every iteration until the user changes Scene Set or picks a different scene |
| Circle | Back to main menu |

**On "random":** the *random* case is already exposed at the main-menu level via Scene Set — `All Scenes` is "random across the catch-all pool"; `Fishing Only` is "random within the fishing family"; etc. Scene Explorer therefore covers the two cases the menu didn't already handle: *play exactly this one* (Cross) and *loop exactly this one* (Triangle). Cycling Scene Set after either one clears the pin and returns to pool-random behaviour, which is the existing scene-set-cycle semantics.

## Thumbnail format

PS1-native, sized to match the actual PS1 framebuffer:

- **320×208 pixels** (full PS1 framebuffer width × framebuffer height minus the 32-row chrome strip).
- **There IS a 2× downsample from the source.** The host capture pipeline emits 640×480 interlaced PNGs (the resolution we use for the website). The PS1 framebuffer is 320×240 — half resolution in each axis. Going to 320×208 thumbnails is therefore lossless *from the PS1's perspective*: we're producing the largest image the PS1 hardware can actually display. It's only a downsample relative to the website screenshots, never relative to what the user sees on the console.
- **8-bit indexed (256-color CLUT)** — same depth as Sierra's original BMPs and our SCR backgrounds.
- One CLUT per thumbnail (per-scene palette captures the dominant colors of each scene without the muddiness of a shared 256-color palette).
- **Default source frame: `floor(0.70 × FG2_frame_count)`** — late enough to capture the climax/punchline of most scenes, early enough to avoid the "Johnny walks back inland" outro tail. Per-scene override slots in the metadata table for the ones that land on a weak frame.
- Disc cost: 320 × 208 × 1 byte = 66,560 bytes per thumbnail; 63 × ~67 KB = **~4.2 MB total**, plus 63 × 512 bytes CLUT = 32 KB. Trivial in a 200 MB CD.

## Asset shape — one PSB per scene, not a single atlas

The PS1's existing PSB loader (`grLoadBmp` with PSB) loads **all
frames** of a multi-frame PSB into VRAM at once. A 63-frame atlas
would push 4.2 MB into VRAM, which is impossible (VRAM is 1 MB total).

**Solution: 63 separate `SCEXPL_<slug>.PSB` files.** Each holds one
thumbnail. The menu loads the current cursor's PSB on cursor change
and releases it before loading the next. Single-thumbnail VRAM cost
is ~67 KB pixels + 512 byte CLUT — comfortably small.

Trade-off: 63 entries in `cd_layout.xml` instead of one. Acceptable —
the layout already has 100+ FG2 entries, and one-per-scene aligns with
how the runtime addresses the rest of the catalog.

## Streaming + prefetch

- On menu entry: load the current cursor's PSB (cold-start cost: ~150
  ms CD seek + ~10 ms DMA upload).
- On Left/Right: release the current PSB, load the next one. Same cost
  per scroll — feels like a flipbook with a quarter-second hold per
  card. Acceptable for v1.
- Prefetch (load N±1 in the background between scrolls) is a v1.1
  optimization; not required for ship.

## Implementation phases

### Phase 1 — Metadata pipeline

`scripts/build-scene-explorer-data.py` reads:
- `docs/ps1/scene-status.md` for slug + family + tag + validation status
- `site/scenes/<slug>/index.md` frontmatter `title` field for display name (already in the form `"FISHING 1 — Johnny casts a line"`, which doubles as the menu's one-line description)
- `src/host/story_data.h` for `dayNo`, `flags`, `spotStart`/`spotEnd`
- FG2 pack headers (existing reader in scripts/) for frame count

Emits `src/pause_menu/scene_explorer_data.h`:

```c
struct TSceneExplorerEntry {
    const char *slug;          /* "fishing1" */
    const char *display_name;  /* "FISHING 1 — Johnny casts a line" */
    const char *family;        /* "Fishing" */
    const char *pack;          /* "FG/FISHING1.FG2" */
    const char *thumb_psb;     /* "BMP/SCEXPL_FISHING1.PSB" */
    uint16      frame_count;
    uint8       validated;
    uint8       _pad;
};

extern const struct TSceneExplorerEntry gSceneExplorer[];
extern const int                       gSceneExplorerCount;
extern const int                       gSceneExplorerFamilyStart[]; /* indices where each family begins */
extern const int                       gSceneExplorerFamilyCount;
```

### Phase 2 — Thumbnail extraction

`scripts/build-scene-explorer-thumbnails.py`:

1. For each scene:
   - Read FG2 pack frame count
   - Choose source frame: `override_frame[slug]` if present in
     `scripts/scene-explorer-overrides.json`, else
     `floor(0.70 × frame_count)`
   - Render the chosen frame via the existing host capture path (or
     extract from a pre-rendered PNG if available in
     `site/assets/img/<slug>-ps1-*.png`)
   - Crop the bottom 32 rows → 320×208 source
   - Median-cut to 256 colors (per-image CLUT)
   - Emit `jc_resources/extracted/bmp/SCEXPL_<SLUG>.BMP` (8-bit
     indexed, 320×208)
2. `transcode-bmp-ps1.py` then bakes each BMP to its own PSB.

### Phase 3 — Asset packaging

- Add 63 `<file path="BMP/SCEXPL_<SLUG>.PSB">` lines to
  `config/ps1/cd_layout.xml` via the same script that emits the
  metadata header (so layout stays in sync automatically).
- Add 63 entries to `src/resource/psb_registry.h` so the loader knows the
  per-frame size of each.

### Phase 4 — Menu state machine

- New `PAUSE_MENU_SCENE_EXPLORER` enum value
- `pauseMenuExplorerCursor` static (0..62)
- `drawSceneExplorer()`:
  - On cursor change: `grLoadBmp(&slot, 0, gSceneExplorer[cur].thumb_psb)`
  - Each frame: `grDrawSprite(grBackgroundSfc, &slot, 0, 0, 0, 0)` for
    the thumbnail
  - PS1 OT primitives draw a 320×32 black rect at y=208 + the menu
    chrome text on top
- `handleSceneExplorerInput()`: Left/Right cycles cursor with PSB
  reload; L1/R1 binary-searches `gSceneExplorerFamilyStart[]` to jump;
  Cross fires `pauseMenuRequestPlayScene`; Circle returns to main.

### Phase 5 — Scene-pin playback

Two new flags in `pause_menu.h`:

```c
extern int pauseMenuRequestPlayScene;   /* -1 = idle, 0..62 = scene index */
extern int pauseMenuRequestLoopScene;   /* -1 = idle, 0..62 = scene index */
```

`jc_reborn.c` consumer at the top of the screensaver loop (mirrors
the scene-set-cycle handler):

```c
/* Cross — play once, then return to active Scene Set. */
if (pauseMenuRequestPlayScene >= 0) {
    int idx = pauseMenuRequestPlayScene;
    pauseMenuRequestPlayScene = -1;
    explicitScene = gSceneExplorer[idx].slug;
    sceneExplorerOneShot = 1;          /* clear after this iteration */
    storyCurrentSpot = -1;
    storyCurrentHdg  = -1;
    fgLoopSequenceJustReset = 1;
    ps1ShowFreeplayLoadingFrame("now playing", 0);
    captionsShowText("Now playing: <name>", 240);
}
/* Triangle — loop this scene every iteration until the user changes
 * Scene Set or picks a different scene. Same as the CLI fgpilot's
 * persistent explicitScene path, no auto-clear. */
if (pauseMenuRequestLoopScene >= 0) {
    int idx = pauseMenuRequestLoopScene;
    pauseMenuRequestLoopScene = -1;
    explicitScene = gSceneExplorer[idx].slug;
    sceneExplorerOneShot = 0;          /* persistent */
    storyCurrentSpot = -1;
    storyCurrentHdg  = -1;
    fgLoopSequenceJustReset = 1;
    ps1ShowFreeplayLoadingFrame("looping", 0);
    captionsShowText("Looping: <name>", 240);
}
/* End-of-iteration cleanup: clear explicitScene if one-shot. */
if (sceneExplorerOneShot && playedScene) {
    sceneExplorerOneShot = 0;
    explicitScene = NULL;
}
```

`sceneExplorerOneShot` is a new file-static in `jc_reborn.c`. It
distinguishes the menu's one-shot Cross-press from the CLI fgpilot
path (which sets `explicitScene` persistently) and from the menu's
Triangle-press loop (also persistent). Cycling Scene Set continues to
clear `explicitScene` and `sceneExplorerOneShot` together.

### Phase 6 — Docs + site

| File | Change |
|---|---|
| `docs/ps1/pause-menu-design.md` | New "Scene Explorer" section + menu-table row |
| `docs/ps1/scene-explorer-design.md` | This file |
| `README.md` | Mention Scene Explorer in the pause-menu paragraph |
| `site/help/menu/index.md` | New section (harness recaptures the screenshot later) |
| `site/docs/freeplay/index.md` | Passing reference |
| `site/scenes/index.md` | Link to `/docs/scene-explorer/` |
| `site/_posts/2026-05-XX-scene-explorer.md` | Devlog post |

---

## Red Team Review

This section enumerates known design risks, with each tagged as
**RESOLVED**, **OPEN-NEEDS-DECISION**, or **ACCEPTED-RISK**. The
implementation does not start until OPEN items are answered.

### R1. Atlas vs per-scene PSB *(RESOLVED → per-scene)*

A single 63-frame atlas would push 4.2 MB into VRAM at load time, which
is impossible. Per-scene PSBs sidestep the issue. Cost: 63 cd_layout
entries; each cycle adds one CD seek (~150 ms).

### R2. Per-scene 256-color CLUT vs shared CLUT *(RESOLVED → per-scene)*

Per-scene CLUTs preserve detail (a fishing scene's blues are wasted
slots in a Mary-visit scene's earth-toned palette). The CD has the
space; the runtime cost is a single CLUT upload per scroll.

### R3. CD seek lag during fast scroll *(ACCEPTED-RISK)*

Holding Left/Right will seek per-step (~150 ms). Acceptable for v1.
v1.1 can prefetch ±1 if it bothers anyone.

### R4. Pre-validated scene playback may surface bugs *(ACCEPTED-RISK)*

Some unvalidated scenes may have FG2 packs that look broken or even
fail to start. A failed `foregroundPilotRuntimeStart` currently calls
`JC_BSOD` (hard fault). Mitigation: the Scene Explorer's pin path
should soft-fail (caption "scene unavailable", return to the rotation)
instead of bsod-ing on a user-triggered play.

**Action:** add a `pauseMenuPlayWasUserTriggered` flag so the BSOD
path becomes a soft-fail when set.

### R5. Description copy *(RESOLVED → reuse `title`)*

`site/scenes/<slug>/index.md` titles already follow the form
`"FISHING 1 — Johnny casts a line"`, which is exactly the menu line.
No new copywriting required.

### R6. Family ordering *(RESOLVED → see UX table)*

Locked: `Fishing → Johnny → Mary → Visitor → Activity → Misc/Suzy →
Stand → Walkstuf → Building → Miscgag`. Matches the order families
already appear in `scene-status.md` with curated families first.

### R7. Codegen drift *(RESOLVED → auto-run in release.sh + CI dry-run drift check)*

`src/pause_menu/scene_explorer_data.h` is regenerated as part of `release.sh`.
A CI step re-runs the codegen in dry-run mode and fails the build if
the committed file drifts from what the source data implies.

### R8. Soft-fail on scene-pack-start failure *(RESOLVED → soft-fail at runtime)*

If `foregroundPilotRuntimeStart` fails on a Scene-Explorer-pinned
scene, suppress the BSOD path and instead caption "Scene unavailable"
and clear the pin so the next iteration falls back to the active
Scene Set. The BSOD path stays in place for non-user-triggered
failures (boot, normal rotation hitting a bug). A
`pauseMenuPlayWasUserTriggered` flag toggles between the two
behaviours.

### R9. Scene-pin re-entrancy / clear *(RESOLVED → separate `sceneExplorerOneShot` flag)*

A new `sceneExplorerOneShot` file-static in `jc_reborn.c` marks pins
that should auto-clear after one play. CLI `fgpilot` leaves it false;
menu Cross sets it true; menu Triangle sets it false (loop). The end-
of-iteration cleanup checks the flag to decide whether to clear
`explicitScene`. See the Phase 5 code sketch above.

### R10. Thumbnail-extraction automation *(RESOLVED → reuse host capture pipeline)*

Adapt `scripts/export-scene-foreground-pilot.sh` to dump a single
frame at index N as a PNG. Builds on infrastructure that already
runs in CI for FG2 capture, no new emulation code required.

### R11. Override mechanism scale *(ACCEPTED-RISK)*

`scripts/scene-explorer-overrides.json` lets us hand-pick frames per
scene. With 63 scenes and a 70% default, expect 5–15 overrides. Not
worth a UI; just a JSON file.

### R12. Build-time cost *(ACCEPTED-RISK)*

Re-running thumbnail extraction for all 63 scenes is ~10 minutes via
the host pipeline. Acceptable as a release-only step. Skip on
incremental PS1 builds (the BMPs ship as committed files; only
re-build the BMPs when a scene's source changes).

### R13. Menu chrome contrast *(RESOLVED → black band)*

The 320×32 chrome strip at the bottom is opaque black. Readable
across all 63 scenes without per-scene tuning, at the cost of
hiding 32 rows of the thumbnail.

### R14. Pin caption text *(RESOLVED → "Now playing" / "Looping")*

- Cross press: caption `"Now playing: <DISPLAY NAME>"` for 240
  vblanks (~4 s)
- Triangle press: caption `"Looping: <DISPLAY NAME>"` for 240 vblanks

### R15. Random vs loop coverage in the menu *(RESOLVED → see UX section)*

*Random* is already exposed via Scene Set (the existing pool
selector). Scene Explorer adds the missing two cases — *play exactly
this one* (Cross) and *loop exactly this one* (Triangle). A
hypothetical "play random scene from the current Scene Explorer
family" would duplicate the Scene Set rotation; intentionally
omitted.

---

## Sign-off status

All R-items are RESOLVED or ACCEPTED-RISK. Implementation cleared to
begin.

## v1 ship status (2026-05-03)

| Phase | Status |
|---|---|
| 1 — Metadata pipeline | ✅ shipped (`scripts/build-scene-explorer-data.py` → `src/pause_menu/scene_explorer_data.h`) |
| 2 — Thumbnail extractor | ✅ shipped (`scripts/build-scene-explorer-thumbnails.py`) |
| 3 — Asset packaging (cd_layout entries) | ⏳ deferred to runtime-integration follow-up |
| 4 — Menu state machine | ✅ shipped (text-only Scene Explorer sub-screen) |
| 5 — Scene-pin playback | ✅ shipped (`pauseMenuRequestPlayScene`/`LoopScene`, `sceneExplorerOneShot`, FG2 break-out) |
| 6 — Docs + site | ⏳ partial (this design doc + scene-status hook); README + help-menu + devlog deferred |

**v1 covers**: navigation through all 63 scenes with text metadata,
play-once/loop scene pinning with the same frog-clock transition Scene
Set uses, immediate FG2 break-out so scenes fire on press without
waiting for the current scene to finish.

**Generated assets staged for follow-up**: 16 validated-scene SCR
thumbnails (320×208 from PS1 captures, packed as 320×240 SCR with
black 32-row chrome strip), plus 11 bonus unvalidated thumbnails from
ACTIVITY/BUILDING captures the user already had on disc. Stored in
`jc_resources/extracted/scr/SCEXPL_<SLUG>.SCR`. Ready for the
runtime-integration PR to wire them into `grLoadScreen` on cursor
change.

**Validation hook (R7+R10 follow-up)**: when a scene moves from ⏳ to
✅ in `docs/ps1/scene-status.md`, the recommended workflow is:

```bash
./scripts/capture-reference-frames.sh --scene "FAMILY N" --frames 1800 --interval 5
python3 scripts/build-scene-explorer-thumbnails.py --slug familyN
git add jc_resources/extracted/scr/SCEXPL_FAMILYN.SCR
```

That single 60-second loop produces a real PS1 thumbnail and stages
it for the next release. As validated count climbs from 16 → 63, the
explorer's thumbnail coverage climbs with it; nothing else has to
change in code.

## Effort

2–3 focused days. Phase 4 (menu rendering) is the largest single
commit; Phase 2 (thumbnail extraction) is the most likely place to
hit unexpected pipeline issues.
