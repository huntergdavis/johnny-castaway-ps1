static PS1Surface *createEmptyBgTileRAM(uint16 width, uint16 height)
{
    /* MEM_REGION_RATIONALE (Round 33): persistent-per-scene BG tile
     * surface (320x240, ~150 KB pixel buffer). 4 tiles × 150 KB =
     * 600 KB worst case — the largest single CACHE pressure source
     * across all 63 scenes. Moved to TRANSIENT because:
     *   - Pixel contents are overwritten by grLoadScreen at scene
     *     start anyway (no cross-scene content reuse).
     *   - Re-allocation via the TRANSIENT bump pointer is ~3
     *     instructions vs the prior O(n) CACHE free-list walk.
     *   - 600 KB out of CACHE eliminates the visitor3-class BSOD
     *     (req=97280 byte clean-rect fails to find a contiguous
     *     CACHE block when bg-tile + LRU + frame buffers occupy it).
     * grBackgroundTilesAssumeWiped (called from fgRuntimeReset right
     * after memSceneReset) NULLs the slot pointers so the next scene's
     * grLoadScreen re-allocates fresh in the new TRANSIENT frame. */
    /* MEM_REGION_RATIONALE: TRANSIENT background tile descriptor, paired
     * with per-scene pixel storage and wiped by memSceneReset. */
    PS1Surface *tile = memIsReady()
        ? (PS1Surface*)memAlloc(MEM_REGION_TRANSIENT, sizeof(PS1Surface), "bgtile-struct")
        : (PS1Surface*)safe_malloc(sizeof(PS1Surface));
    tile->width = width;
    tile->height = height;
    tile->x = 0;
    tile->y = 0;
    tile->indexedPixels = NULL;
    tile->indexedOwned = 0;
    tile->psbNibbles = 0;
    tile->nextTile = NULL;
    /* MEM_REGION_RATIONALE: TRANSIENT background tile pixels, overwritten
     * by each scene's screen load and not reused across scene boundaries. */
    tile->pixels = memIsReady()
        ? (uint16*)memAlloc(MEM_REGION_TRANSIENT, (size_t)width * height * 2u, "bgtile-pixels")
        : (uint16*)safe_malloc(width * height * 2);
    /* Fill with black (0x0000 = transparent/black) */
    for (uint32 i = 0; i < width * height; i++) {
        tile->pixels[i] = 0x0000;
    }
    return tile;
}

static void resetBgTileRAMFields(PS1Surface *tile, uint16 width, uint16 height)
{
    tile->width = width;
    tile->height = height;
    tile->x = 0;
    tile->y = 0;
    tile->indexedPixels = NULL;
    tile->indexedOwned = 0;
    tile->psbNibbles = 0;
    tile->nextTile = NULL;
}

static PS1Surface *ensureEmptyBgTileRAM(PS1Surface **slot, uint16 width, uint16 height)
{
    uint32 pixelCount = (uint32)width * (uint32)height;

    if (*slot == NULL || (*slot)->pixels == NULL ||
        (*slot)->width != width || (*slot)->height != height) {
        freeBgTile(slot);
        *slot = createEmptyBgTileRAM(width, height);
        return *slot;
    }

    resetBgTileRAMFields(*slot, width, height);
    memset((*slot)->pixels, 0, pixelCount * sizeof(uint16));
    return *slot;
}

static PS1Surface *ensureBgTileRAM(PS1Surface **slot, uint16 width, uint16 height)
{
    if (*slot == NULL || (*slot)->pixels == NULL ||
        (*slot)->width != width || (*slot)->height != height) {
        freeBgTile(slot);
        /* MEM_REGION_RATIONALE (Round 33): TRANSIENT bg-tile, see
         * createEmptyBgTileRAM rationale. The freeBgTile call above is
         * a defensive no-op after a scene wipe (slot is already NULL
         * thanks to grBackgroundTilesAssumeWiped); it handles the
         * cold-boot and width/height-change paths. */
        /* MEM_REGION_RATIONALE: TRANSIENT background tile descriptor, same
         * per-scene lifetime as createEmptyBgTileRAM. */
        *slot = memIsReady()
            ? (PS1Surface*)memAlloc(MEM_REGION_TRANSIENT, sizeof(PS1Surface), "bgtile-struct")
            : (PS1Surface*)safe_malloc(sizeof(PS1Surface));
        resetBgTileRAMFields(*slot, width, height);
        /* Diagnostic: trace bg-tile pixel allocs to pin the explorer-launch
         * TRANSIENT overflow (a 5th bgtile-pixels orphaning churn). */
        { extern int printf(const char *, ...);
          printf("JCTILE alloc slot=%p w=%u h=%u bytes=%lu\n",
                 (void*)slot, (unsigned)width, (unsigned)height,
                 (unsigned long)((uint32)width * height * 2u)); }
        /* MEM_REGION_RATIONALE: TRANSIENT background tile pixels, same
         * per-scene lifetime as createEmptyBgTileRAM. */
        (*slot)->pixels = memIsReady()
            ? (uint16*)memAlloc(MEM_REGION_TRANSIENT, (uint32)width * height * 2u, "bgtile-pixels")
            : (uint16*)safe_malloc((uint32)width * height * 2);
    } else {
        resetBgTileRAMFields(*slot, width, height);
    }

    return *slot;
}

/*
 * Initialize empty background
 */
void grInitEmptyBackground()
{
    if (grBackgroundSfc != NULL) {
        /* grLoadScreen sets grBackgroundSfc = bgTile0, so freeing it
         * would leave bgTile0 as a dangling pointer. Null out any match. */
        if (grBackgroundSfc == bgTile0) bgTile0 = NULL;
        if (grBackgroundSfc == bgTile1) bgTile1 = NULL;
        if (grBackgroundSfc == bgTile3) bgTile3 = NULL;
        if (grBackgroundSfc == bgTile4) bgTile4 = NULL;
        grFreeLayer(grBackgroundSfc);
    }

    /* Reset VRAM allocation cursor before grNewEmptyBackground bumps it.
     * Without this the screensaver loop walks the cursor past the PS1 VRAM
     * width (1024 px) after ~2 iterations, and the 3rd scene's background
     * surface lands at an invalid VRAM coordinate. */
    grResetVramCursor();

    grBackgroundSfc = grNewEmptyBackground();

    /* Create empty RAM tiles for sprite compositing (needed by grCompositeToBackground)
     * If tiles already exist (e.g. after grFadeOut darkened them), zero their pixels */
    if (bgTile0 == NULL) bgTile0 = createEmptyBgTileRAM(320, 240);
    else memset(bgTile0->pixels, 0, 320 * 240 * 2);
    if (bgTile1 == NULL) bgTile1 = createEmptyBgTileRAM(320, 240);
    else memset(bgTile1->pixels, 0, 320 * 240 * 2);
    if (bgTile3 == NULL) bgTile3 = createEmptyBgTileRAM(320, 240);
    else memset(bgTile3->pixels, 0, 320 * 240 * 2);
    if (bgTile4 == NULL) bgTile4 = createEmptyBgTileRAM(320, 240);
    else memset(bgTile4->pixels, 0, 320 * 240 * 2);
}

/* Frames the frog-clock animation runs for. The original single-call
 * variant painted one frame and returned; looping here makes the hand
 * fly around the way it does in JOHNNY 6 before the next load step
 * takes over the screen. */
#define MEANWHILE_ANIMATE_VBLANKS  36

/* Per-sprite anchor table for MEANWHIL.BMP, lifted from MEANWHIL.TTM
 * DRAW_SPRITE opcodes. Sprite 0 is the card; sprites 5..16 are the
 * 12 hour-hand positions (one per hour); sprites 1..4 are the 4
 * minute-hand positions. Each (x, y) is the script's draw position
 * for that frame — the bounding boxes vary because each rotated hand
 * has a different bbox, so a fixed top-left would visibly drift. */
static const sint16 kMeanwhilePos[17][2] = {
    {254, 100},  /* 0  card */
    {300, 152},  /* 1  minute */
    {275, 152},  /* 2  minute */
    {291, 161},  /* 3  minute */
    {290, 137},  /* 4  minute */
    {297, 139},  /* 5  hour */
    {301, 143},  /* 6  hour */
    {301, 149},  /* 7  hour */
    {299, 159},  /* 8  hour */
    {299, 161},  /* 9  hour */
    {300, 162},  /* 10 hour */
    {294, 162},  /* 11 hour */
    {285, 163},  /* 12 hour */
    {278, 162},  /* 13 hour */
    {278, 157},  /* 14 hour */
    {281, 147},  /* 15 hour */
    {287, 141},  /* 16 hour */
};

void grShowMeanwhileLoadingFrame(uint16 tick)
{
    struct TTtmSlot slot;
    int i;

    memset(&slot, 0, sizeof(slot));
    grInitEmptyBackground();   /* once — fresh bg surface, zeros bgTile */
    grDx = 0;
    grDy = 0;
    grLoadBmp(&slot, 0, "MEANWHIL.BMP");

    /* Loop, painting card + hour hand + minute hand each VBlank. The
     * MEANWHILE card is opaque enough that re-stamping it before the
     * hands each frame covers the previous hands.
     *
     * NOTE: this leaves bgTile* zeroed when it returns. Callers that
     * follow with code expecting a populated bg (e.g., the walk
     * subsystem) must either reload the bg themselves or set
     * fgLoopSequenceJustReset so the walk is skipped. */
    for (i = 0; i < MEANWHILE_ANIMATE_VBLANKS; i++) {
        if (slot.numSprites[0] > 0) {
            int kf = (int)tick + i;
            int hourSprite   = 5 + ((kf >> 2) % 12);   /* 5..16 */
            int minuteSprite = 1 + (kf & 3);           /* 1..4 */

            /* Card. */
            grDrawSprite(grBackgroundSfc, &slot,
                         kMeanwhilePos[0][0], kMeanwhilePos[0][1], 0, 0);
            /* Hour hand (advances every 4 frames). */
            if (hourSprite < slot.numSprites[0]) {
                grDrawSprite(grBackgroundSfc, &slot,
                             kMeanwhilePos[hourSprite][0],
                             kMeanwhilePos[hourSprite][1],
                             (uint16)hourSprite, 0);
            }
            /* Minute hand (cycles every frame). */
            if (minuteSprite < slot.numSprites[0]) {
                grDrawSprite(grBackgroundSfc, &slot,
                             kMeanwhilePos[minuteSprite][0],
                             kMeanwhilePos[minuteSprite][1],
                             (uint16)minuteSprite, 0);
            }
        }
        grForceFullRedrawNextFrame();
        VSync(0);
        grDrawBackground();
        DrawSync(0);
    }

    grReleaseBmp(&slot, 0);
}

/*
 * Save clean copies of background tiles (after loading background, before any sprite compositing).
 * Used by composite pattern to restore pristine background each frame.
 *
 * Called from grLoadScreen after background is loaded/composited.
 * Always updates ALL clean copies to current tile state.
 */
void grSaveCleanBgTiles(void)
{
    uint32 tileSize = 320 * 240 * 2;  /* 320x240 @ 16-bit = 153,600 bytes per tile */

    /* Reuse existing buffers when possible to avoid 600KB free+malloc churn.
     * Only allocate if buffer doesn't exist yet; always overwrite content. */
    if (bgTile0 && bgTile0->pixels) {
        if (!bgTile0Clean) bgTile0Clean = (uint16*)malloc(tileSize);
        if (bgTile0Clean) memcpy(bgTile0Clean, bgTile0->pixels, tileSize);
    } else if (bgTile0Clean) { free(bgTile0Clean); bgTile0Clean = NULL; }

    if (bgTile1 && bgTile1->pixels) {
        if (!bgTile1Clean) bgTile1Clean = (uint16*)malloc(tileSize);
        if (bgTile1Clean) memcpy(bgTile1Clean, bgTile1->pixels, tileSize);
    } else if (bgTile1Clean) { free(bgTile1Clean); bgTile1Clean = NULL; }

    /* Bottom tiles - for partial height images (like ISLETEMP), the bottom
     * tiles have been composited with scene data over the ocean base. */
    if (bgTile3 && bgTile3->pixels) {
        if (!bgTile3Clean) bgTile3Clean = (uint16*)malloc(tileSize);
        if (bgTile3Clean) memcpy(bgTile3Clean, bgTile3->pixels, tileSize);
    } else if (bgTile3Clean) { free(bgTile3Clean); bgTile3Clean = NULL; }

    if (bgTile4 && bgTile4->pixels) {
        if (!bgTile4Clean) bgTile4Clean = (uint16*)malloc(tileSize);
        if (bgTile4Clean) memcpy(bgTile4Clean, bgTile4->pixels, tileSize);
    } else if (bgTile4Clean) { free(bgTile4Clean); bgTile4Clean = NULL; }

    /* New clean baseline: mark all tiles dirty so first frame uploads everything.
     * Set prevDirty too since the framebuffer may not match the new background. */
    grMarkAllTilesDirty();
    grMarkPrevAllTilesDirty();
}

void grSetSaveCleanOnScreenLoad(int enabled)
{
    grSaveCleanOnScreenLoad = enabled ? 1 : 0;
}
