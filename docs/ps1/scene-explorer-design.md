# Scene Explorer — design doc

> Rendered version: `/docs/scene-explorer/` on the project website.

A new pause-menu sub-screen that lets the player browse all 63 scenes
in the catalog with a thumbnail, description, characters, frame count,
and validation status. Cross plays the highlighted scene immediately
(via the same scene-pin path the freeplay rotation already uses).

## UX

Single-up pager layout. Replaces one slot in the main pause menu (the
5–6-items-per-screen rule still holds because Scene Explorer itself is
just one cell).

```
┌──────────────────────────────────┐
│        SCENE EXPLORER            │
│ ──────────────────────────────── │
│  ┌──────────────────────┐        │
│  │   [thumbnail 160×120]│        │
│  └──────────────────────┘        │
│  FISHING 1   ✅ validated        │
│  Johnny casts a line.            │
│                                  │
│  Family: Fishing  Frames: 240    │
│  Pack: FG/FISHING1.FG2           │
│ ──────────────────────────────── │
│  ◀ 12/63 ▶   L1/R1 jump family   │
│  X play     O back               │
└──────────────────────────────────┘
```

| Input | Action |
|---|---|
| Left / Right | Previous / next scene |
| L1 / R1 | Jump to previous / next family (Fishing → Johnny → Mary → …) |
| Cross | Pin this scene and exit the menu — the next loop iteration plays it once before resuming the active Scene Set |
| Circle | Back to main menu |

## Thumbnail rule

**Default: frame at the 70% mark of the FG2 pack's frame count** —
late enough to capture the climax/punchline of most scenes, early
enough to avoid the "Johnny walks back inland" outro tail. Per-scene
overrides live in the metadata table so any thumbnail that lands on a
weak frame can be moved without touching the extraction script.

## Asset shape

- 63 thumbnails, 160×120 each
- 8-bit indexed (256-color CLUT) by default — disc cost ~1.2 MB,
  visible quality stays close to the host capture
- 4-bit fallback if VRAM gets tight (~600 KB) — a per-thumbnail flag
  could mix bit depths, but starting uniform is simpler
- All packed into a single `SCENEXPL.PSB` (one frame per scene, 63
  frames). The existing PSB loader streams a single frame at a time,
  which suits the menu's one-up display.

## Streaming

Loading all 63 thumbnails into RAM is wasteful — only one is on screen
at a time. The menu calls `grLoadBmp("SCENEXPL.PSB")` once on entry,
then `grDrawSprite` for the current frame. The PSB's CLUT goes into
VRAM; the per-frame indexed pixels live in RAM only as long as the
current cursor frame.

Adjacent-frame prefetch (the cursor's ±1) is a nice-to-have for fast
scrolling but not required for v1 — the PSB header lookup is cheap and
the read-from-disc-into-VRAM path already runs at FG2 timing budgets.

## Implementation phases

### Phase 1 — Metadata pipeline

`scripts/build-scene-explorer-data.py` reads:
- `docs/ps1/scene-status.md` for slug + family + tag + validation status
- `site/scenes/<slug>/index.md` for description + characters + caption text
- `src/story_data.h` for `dayNo`, `flags`, `spotStart`/`spotEnd`

Emits `src/scene_explorer_data.h`:

```c
struct TSceneExplorerEntry {
    const char *slug;          /* "fishing1" */
    const char *display_name;  /* "FISHING 1" */
    const char *family;        /* "Fishing" */
    const char *description;   /* one-line, ~40 chars */
    const char *pack;          /* "FG/FISHING1.FG2" */
    uint16      frame_count;
    uint8       validated;
    uint8       thumbnail_idx; /* index into SCENEXPL.PSB */
};

extern const struct TSceneExplorerEntry gSceneExplorer[];
extern const int                       gSceneExplorerCount;
```

### Phase 2 — Thumbnail extraction

`scripts/build-scene-explorer-thumbnails.py`:

1. For each scene in the metadata table, choose a source frame:
   - If a curated capture exists in `site/assets/img/<slug>-ps1-*.png`,
     use that
   - Otherwise extract frame `floor(0.70 * frame_count)` from the
     scene's FG2 pack via the existing host playback pipeline
2. Downsample 320×240 → 160×120 (bicubic)
3. Palettize each thumbnail's 256 colors via per-frame median-cut
4. Emit a Sierra-style 8-bit BMP per scene
5. Pack 63 BMPs into `jc_resources/extracted/bmp/SCENEXPL.BMP` (one
   frame per scene)

### Phase 3 — Asset packaging

- `transcode-bmp-ps1.py` already builds `SCENEXPL.PSB` from the BMP
  given a `psb_registry.h` size entry. Add the entry.
- `config/ps1/cd_layout.xml` adds the `SCENEXPL.PSB` line under
  `<files>`.

### Phase 4 — Menu state machine

- New `PAUSE_MENU_SCENE_EXPLORER` enum value
- `pauseMenuExplorerCursor` static (0..62)
- `drawSceneExplorer()` composes the layout: thumbnail at fixed top
  position, text below using the existing `pmPrintf` 8×8 font
- `handleSceneExplorerInput()` cycles cursor on Left/Right, jumps
  family on L1/R1, fires play on Cross, returns on Circle
- Family-jump table built once at startup from `gSceneExplorer`

### Phase 5 — Scene-pin playback

New flag in `pause_menu.h`:

```c
extern int pauseMenuRequestPlayScene;   /* -1 = idle, 0..62 = scene index */
```

`jc_reborn.c` consumer at the top of the screensaver loop (mirrors
the scene-set-cycle handler):

```c
if (pauseMenuRequestPlayScene >= 0) {
    int idx = pauseMenuRequestPlayScene;
    pauseMenuRequestPlayScene = -1;
    explicitScene = gSceneExplorer[idx].slug;
    storyCurrentSpot = -1;
    storyCurrentHdg  = -1;
    fgLoopSequenceJustReset = 1;
    ps1ShowFreeplayLoadingFrame("playing scene", 0);
    captionsShowText("Playing once: <name>", 240);
}
```

After the explicitly-pinned scene plays, `explicitScene` is cleared so
the loop returns to the active Scene Set's pool.

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

## Risks

1. **Thumbnail aesthetic at the chosen palette depth.** Mitigation:
   start at 8-bit per frame, drop to 4-bit only if VRAM fights.
2. **Frame-70%-of-pack auto-pick.** Some scenes climax earlier or
   later. The metadata table has a `thumbnail_frame_override` slot for
   hand-picked frames; default empty.
3. **Pin-scene playback through the menu.** The `explicitScene` path
   was built for the `fgpilot` CLI; needs a one-scene smoke test
   before all 63 are wired.
4. **CD-seek lag on rapid Left/Right.** v1 ships without prefetch and
   sees how it feels; if scrolling chugs, add `±1` prefetch in phase
   4b.

## Effort

2–3 focused days for a polished v1. Phase 4 (menu rendering) is the
single biggest commit; the rest is short.
