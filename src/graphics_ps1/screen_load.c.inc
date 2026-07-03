static uint16 gSceneExplorerThumbChunk[
    SCENE_EXPLORER_THUMB_W * SCENE_EXPLORER_THUMB_ROWS_PER_READ
];

/* DrawSync alone can return before the GPU has fully consumed a queued
 * CPU->VRAM transfer on real silicon (emulators complete transfers
 * synchronously, so this never shows there). The thumbnail streamer
 * refills its chunk buffer from CD right after DrawSync — if the GPU is
 * still draining the previous strip, its tail is fed from the NEW data
 * and every later row shifts sideways ("jumbled strips"). Poll the
 * GPUSTAT ready bits before letting the buffer be reused. Bounded. */
static void grWaitGpuTransferReady(void)
{
    volatile uint32 *gpustat = (volatile uint32 *)0xBF801814;
    int i;
    for (i = 0; i < 200000; i++) {
        uint32 s = *gpustat;
        if ((s & (1u << 26)) != 0u && (s & (1u << 28)) != 0u)
            break;
    }
}

static void grVerifySceneExplorerStrip(const RECT *strip);

static void grClearSceneExplorerRect(int x, int y, int w, int h)
{
    RECT rect;

    memset(gSceneExplorerThumbChunk, 0, sizeof(gSceneExplorerThumbChunk));

    while (h > 0) {
        int rows = h;
        int clearX = x;
        int remainingW = w;

        if (rows > SCENE_EXPLORER_THUMB_ROWS_PER_READ)
            rows = SCENE_EXPLORER_THUMB_ROWS_PER_READ;

        while (remainingW > 0) {
            int chunkW = remainingW;
            if (chunkW > SCENE_EXPLORER_THUMB_W)
                chunkW = SCENE_EXPLORER_THUMB_W;

            setRECT(&rect, clearX, y, chunkW, rows);
            LoadImage(&rect, (uint32 *)gSceneExplorerThumbChunk);
            DrawSync(0);
            /* Same buffer-reuse hazard as the strips: the last clear's
             * transfer must be fully consumed before the first CD chunk
             * refills this buffer. */
            grWaitGpuTransferReady();

            clearX += chunkW;
            remainingW -= chunkW;
        }

        y += rows;
        h -= rows;
    }
}

int grLoadSceneExplorerThumbnail(const char *slug)
{
    CdlFILE cdfile;
    char path[24];
    char family[16];
    int i, fam_len;
    const char *digits;
    const char *abbrev;
    RECT rect;

    if (slug == NULL || slug[0] == '\0')
        return 0;

    /* Split slug into family prefix + numeric tag.
     * "fishing1" -> family="fishing", digits="1" */
    fam_len = 0;
    while (slug[fam_len] >= 'a' && slug[fam_len] <= 'z'
           && fam_len < (int)sizeof(family) - 1)
        fam_len++;
    if (fam_len == 0 || fam_len >= (int)sizeof(family))
        return 0;
    for (i = 0; i < fam_len; i++) family[i] = slug[i];
    family[fam_len] = '\0';
    digits = slug + fam_len;
    if (*digits < '0' || *digits > '9')
        return 0;

    /* Family -> 2-letter abbrev. Must match FAMILY_ABBREV in
     * scripts/build-scene-explorer-thumbnails.py. */
    if      (!strcmp(family, "fishing"))  abbrev = "FI";
    else if (!strcmp(family, "johnny"))   abbrev = "JO";
    else if (!strcmp(family, "mary"))     abbrev = "MA";
    else if (!strcmp(family, "visitor"))  abbrev = "VI";
    else if (!strcmp(family, "activity")) abbrev = "AC";
    else if (!strcmp(family, "suzy"))     abbrev = "SU";
    else if (!strcmp(family, "miscgag"))  abbrev = "MG";
    else if (!strcmp(family, "stand"))    abbrev = "ST";
    else if (!strcmp(family, "walkstuf")) abbrev = "WK";
    else if (!strcmp(family, "building")) abbrev = "BL";
    else return 0;

    if (!grBuildPath3(path, sizeof(path), "SCR\\SX", abbrev, ""))
        return 0;
    i = strlen(path);
    if (!grBuildPath3(path + i, sizeof(path) - (size_t)i, digits, ".SCR", ""))
        return 0;
    if (!ps1_streamResolveFile(path, &cdfile))
        return 0;

    /* Clear the four bands around the centered thumbnail to black so
     * the previous menu/scene pixels don't show through. Widths greater
     * than 320 are split because the reusable chunk buffer is 320 pixels
     * wide. */
    grClearSceneExplorerRect(0, 0, 640, 120);
    grClearSceneExplorerRect(0, 360, 640, 120);
    grClearSceneExplorerRect(0, 120, 160, 240);
    grClearSceneExplorerRect(480, 120, 160, 240);

    for (int y = 0; y < SCENE_EXPLORER_THUMB_H;
         y += SCENE_EXPLORER_THUMB_ROWS_PER_READ) {
        uint32 offset = (uint32)y * SCENE_EXPLORER_THUMB_W * 2UL;
        uint32 bytes = SCENE_EXPLORER_THUMB_W *
                       SCENE_EXPLORER_THUMB_ROWS_PER_READ * 2UL;

        if (!ps1_streamReadAlignedIntoFile(&cdfile, offset, bytes,
                                           (uint8 *)gSceneExplorerThumbChunk))
            return 0;

        setRECT(&rect, 160, 120 + y,
                SCENE_EXPLORER_THUMB_W,
                SCENE_EXPLORER_THUMB_ROWS_PER_READ);
        LoadImage(&rect, (uint32 *)gSceneExplorerThumbChunk);
        DrawSync(0);
        grWaitGpuTransferReady();
        grVerifySceneExplorerStrip(&rect);
    }

    return 1;
}

/* Read the strip's first and last rows back out of VRAM and compare
 * with the source; a shifted/corrupt upload differs and gets
 * re-uploaded (bounded). A shift can begin mid-strip, hence both ends.
 * This makes the console-only "jumbled strips" self-healing regardless
 * of which side (GPU timing, DMA contention) drops the ball — and if
 * the CD data itself arrives shifted, VRAM matches the buffer, the
 * verify passes, and the SQUARE test pattern pins the blame instead. */
static void grVerifySceneExplorerStrip(const RECT *strip)
{
    static uint16 verifyRow[SCENE_EXPLORER_THUMB_W];
    RECT rowRect;
    int tries;

    for (tries = 0; tries < 3; tries++) {
        int lastRow = strip->h - 1;
        int ok;
        setRECT(&rowRect, strip->x, strip->y, strip->w, 1);
        StoreImage(&rowRect, (uint32 *)verifyRow);
        DrawSync(0);
        ok = memcmp(verifyRow, gSceneExplorerThumbChunk,
                    sizeof(verifyRow)) == 0;
        if (ok && lastRow > 0) {
            setRECT(&rowRect, strip->x, strip->y + lastRow, strip->w, 1);
            StoreImage(&rowRect, (uint32 *)verifyRow);
            DrawSync(0);
            ok = memcmp(verifyRow,
                        gSceneExplorerThumbChunk +
                            (size_t)lastRow * SCENE_EXPLORER_THUMB_W,
                        sizeof(verifyRow)) == 0;
        }
        if (ok)
            return;
        LoadImage((RECT *)strip, (uint32 *)gSceneExplorerThumbChunk);
        DrawSync(0);
        grWaitGpuTransferReady();
    }
}

/* Diagnostic: stream a computed pattern through the exact same chunked
 * upload path the CD thumbnails use, but with RAM-generated data. On
 * console: pattern clean but CD thumbs jumbled -> the CD data path is
 * corrupting; pattern jumbled too -> the GPU upload path is. Green/red
 * 16 px bars with a white marker column stepping 20 px right per strip
 * and a distinct first row make any horizontal shift obvious and
 * attributable to a specific strip. */
int grLoadSceneExplorerTestPattern(void)
{
    RECT rect;
    int strip = 0;

    for (int y = 0; y < SCENE_EXPLORER_THUMB_H;
         y += SCENE_EXPLORER_THUMB_ROWS_PER_READ, strip++) {
        for (int r = 0; r < SCENE_EXPLORER_THUMB_ROWS_PER_READ; r++) {
            for (int x = 0; x < SCENE_EXPLORER_THUMB_W; x++) {
                uint16 c = ((x >> 4) & 1) ? 0x001F : 0x03E0; /* red/green */
                if (x == (strip * 20) % SCENE_EXPLORER_THUMB_W)
                    c = 0x7FFF;                               /* white marker */
                if (r == 0)
                    c |= 0x7C00;                              /* first row tint */
                gSceneExplorerThumbChunk[r * SCENE_EXPLORER_THUMB_W + x] = c;
            }
        }
        setRECT(&rect, 160, 120 + y,
                SCENE_EXPLORER_THUMB_W,
                SCENE_EXPLORER_THUMB_ROWS_PER_READ);
        LoadImage(&rect, (uint32 *)gSceneExplorerThumbChunk);
        DrawSync(0);
        grWaitGpuTransferReady();
        grVerifySceneExplorerStrip(&rect);
    }

    return 1;
}

void grFreeSceneExplorerThumbnailBuffer(void)
{
    /* Thumbnails now stream through a static 16-row chunk buffer, so
     * there is no per-entry heap state to release. Keep the function as
     * the pause-menu cleanup hook. */
}

/*
 * Load background screen
 */
void grLoadScreen(char *strArg)
{
    struct TScrResource *scrResource = findScrResource(strArg);
    if (scrResource == NULL) return;

    /* Determine partial height from metadata (available without loading data) */
    uint16 srcHeight = scrResource->height;
    int isPartialHeight = (srcHeight < 480);

    /* Drop clean copies before loading SCR data. Keep the 320x240 bg tile
     * buffers resident and refill them in place; repeated 153600-byte
     * free+malloc cycles fragment the PS1 heap during long screensaver runs. */
    grFreeCleanBgTiles();

    /* Retired split-tile layout; release any stale remnants but preserve the
     * active 320px tiles (bgTile0/1/3/4) for reuse. */
    freeBgTile(&bgTile2a);
    freeBgTile(&bgTile2b);

    freeBgTile(&bgTile5a);
    freeBgTile(&bgTile5b);

    grBackgroundSfc = NULL;

    if (grSavedZonesLayer != NULL) {
        grFreeLayer(grSavedZonesLayer);
        grSavedZonesLayer = NULL;
    }

    /* Full-screen ocean/night SCR files are fixed-size raw 4bpp payloads.
     * Stream them directly into bg tiles so the scene loop never needs a
     * 153600-byte temporary SCR heap allocation. */
    if (scrResource->uncompressedData == NULL &&
        grLoadFullScreenScrCached(scrResource)) {
        return;
    }

    if (scrResource->uncompressedData == NULL &&
        grLoadFullScreenScrStreamed(scrResource)) {
        grUploadFullScreenBgTilesIfNeeded();
        grBackgroundSfc = bgTile0;
        if (grSaveCleanOnScreenLoad)
            grSaveCleanBgTiles();
        return;
    }

    /* Partial/non-standard SCR files still use the legacy temporary load. */
    if (scrResource->uncompressedData == NULL) {
        ps1_loadScrData(scrResource);
    }
    if (scrResource->uncompressedData == NULL &&
        (bgTile0 != NULL || bgTile1 != NULL || bgTile3 != NULL || bgTile4 != NULL)) {
        /* Boot/menu paths can be tighter than steady-state scene playback.
         * Retry once with tile RAM released; steady-state FG2 should not
         * normally take this path. */
        freeBgTile(&bgTile0);
        freeBgTile(&bgTile1);
        freeBgTile(&bgTile3);
        freeBgTile(&bgTile4);
        ps1_loadScrData(scrResource);
    }
    if (scrResource->uncompressedData == NULL) {
        /* Failed to load — recreate empty tiles so rendering doesn't crash */
        ensureEmptyBgTileRAM(&bgTile0, 320, 240);
        ensureEmptyBgTileRAM(&bgTile1, 320, 240);
        if (!isPartialHeight) {
            ensureEmptyBgTileRAM(&bgTile3, 320, 240);
            ensureEmptyBgTileRAM(&bgTile4, 320, 240);
        }
        grBackgroundSfc = bgTile0;
        return;
    }

    if ((scrResource->width % 2) == 1) {
        GR_DIAG_PRINTF("Warning: grLoadScreen(): can't manage odd widths\n");
    }

    if (scrResource->width > 640 || scrResource->height > 480) {
        fatalError("grLoadScreen(): can't manage more than 640x480 resolutions");
    }

    uint16 srcWidth  = scrResource->width;
    uint8 *src = scrResource->uncompressedData;

    /* Create tiles for top row (y=0-239)
     * VRAM layout:
     * - Tile 0  (256x240) at VRAM(640, 4)   - srcX=0,   y=4-243
     * - Tile 1  (256x240) at VRAM(640, 244) - srcX=256, y=244-483
     * DEBUG: Test single 64px tile at x=896 to isolate VRAM issue
     */
    /* Top row: LoadImage directly to framebuffer at init
     * Use 2 tiles of 320px each to cover 640px total */
    bgTile0  = ensureBgTileRAMPartial(&bgTile0, src, srcWidth, srcHeight, 0,   0, 320);   /* top row, x=0-319 */
    bgTile1  = ensureBgTileRAMPartial(&bgTile1, src, srcWidth, srcHeight, 320, 0, 320);   /* top row, x=320-639 */
    bgTile2a = NULL;
    bgTile2b = NULL;

    if (grPresentDuringScreenLoad) {
        /* LoadImage top row directly to framebuffer
         * Use separate RECTs - LoadImage is async and may read RECT after return */
        RECT topRect0, topRect1;
        if (bgTile0 && bgTile0->pixels) {
            setRECT(&topRect0, 0, 0, bgTile0->width, bgTile0->height);
            LoadImage(&topRect0, (uint32*)bgTile0->pixels);
        }
        if (bgTile1 && bgTile1->pixels) {
            setRECT(&topRect1, 320, 0, bgTile1->width, bgTile1->height);
            LoadImage(&topRect1, (uint32*)bgTile1->pixels);
        }
        DrawSync(0);
    }

    /* Bottom row: Handle based on image height and existing tiles
     * For partial height images (like ISLETEMP 640x350), preserve existing ocean tiles */

    /* Calculate how many lines we can read for bottom half (y=240+) */
    uint16 bottomRowLines = (srcHeight > 240) ? (srcHeight - 240) : 0;

    if (bottomRowLines >= 240) {
        /* Full 640x480 image - create bottom row tiles (2x320 to match top row) */
        bgTile3  = ensureBgTileRAMPartial(&bgTile3, src, srcWidth, srcHeight, 0,   240, 320);
        bgTile4  = ensureBgTileRAMPartial(&bgTile4, src, srcWidth, srcHeight, 320, 240, 320);
        bgTile5a = NULL;
        bgTile5b = NULL;
    } else if (isPartialHeight && bgTile3 != NULL && bgTile4 != NULL &&
               bgTile3->pixels != NULL && bgTile4->pixels != NULL &&
               bottomRowLines > 0) {
        /* Partial height image with existing bottom tiles (ocean) - composite on top!
         * Copy the available rows (240 to srcHeight-1) onto existing tiles */
        uint16 *dst3 = bgTile3->pixels;
        uint16 *dst4 = bgTile4->pixels;

        for (uint16 y = 0; y < bottomRowLines && y < 240; y++) {
            uint16 srcY = 240 + y;
            for (uint16 x = 0; x < 320; x++) {
                /* Left tile (bgTile3) */
                uint32 srcOffset = (srcY * srcWidth + x) / 2;
                uint8 palIndex;
                if (x & 1) {
                    palIndex = src[srcOffset] & 0x0F;
                } else {
                    palIndex = (src[srcOffset] >> 4) & 0x0F;
                }
                dst3[y * 320 + x] = ttmPalette[palIndex & 0x0F];

                /* Right tile (bgTile4) */
                uint16 srcX = 320 + x;
                srcOffset = (srcY * srcWidth + srcX) / 2;
                if (srcX & 1) {
                    palIndex = src[srcOffset] & 0x0F;
                } else {
                    palIndex = (src[srcOffset] >> 4) & 0x0F;
                }
                dst4[y * 320 + x] = ttmPalette[palIndex & 0x0F];
            }
        }
    } else if (bottomRowLines > 0) {
        /* Partial bottom row with no existing tiles - create with partial data */
        bgTile3  = ensureBgTileRAMPartial(&bgTile3, src, srcWidth, srcHeight, 0,   240, 320);
        bgTile4  = ensureBgTileRAMPartial(&bgTile4, src, srcWidth, srcHeight, 320, 240, 320);
        bgTile5a = NULL;
        bgTile5b = NULL;
    } else {
        /* SCR is only 240 lines or less - create empty tiles for sprite compositing */
        ensureEmptyBgTileRAM(&bgTile3, 320, 240);
        ensureEmptyBgTileRAM(&bgTile4, 320, 240);
        bgTile5a = NULL;
        bgTile5b = NULL;
    }

    if (grPresentDuringScreenLoad) {
        /* Disable display during bottom row LoadImage to avoid tearing/corruption */
        SetDispMask(0);

        /* LoadImage bottom row tiles directly to framebuffer (2x320 layout)
         * Use separate RECTs - LoadImage is async and may read RECT after return */
        RECT botRect3, botRect4;

        if (bgTile3 && bgTile3->pixels) {
            setRECT(&botRect3, 0, 240, bgTile3->width, bgTile3->height);
            LoadImage(&botRect3, (uint32*)bgTile3->pixels);
        }
        if (bgTile4 && bgTile4->pixels) {
            setRECT(&botRect4, 320, 240, bgTile4->width, bgTile4->height);
            LoadImage(&botRect4, (uint32*)bgTile4->pixels);
        }

        DrawSync(0);  /* Sync bottom row uploads */
        SetDispMask(1);
    }

    /* Set grBackgroundSfc to first tile for compatibility with existing code */
    grBackgroundSfc = bgTile0;

    /* Free SCR data after converting - saves memory. The buffer comes
     * from ps1_streamReadCache → ps1_streamReadFromCdFile → memAlloc(CACHE)
     * post-memInit, so libc free() is a silent no-op for that range —
     * route through memFree(CACHE), which range-checks and dispatches
     * to free() for pre-memInit libc fallback pointers. Plugging this
     * leak is the difference between a soak that hits CACHE-exhaustion
     * after ~5 scene transitions and one that stays bounded. */
    if (scrResource->uncompressedData) {
        memFree(MEM_REGION_CACHE, scrResource->uncompressedData);
        scrResource->uncompressedData = NULL;
    }

    /* Save clean background tiles immediately after loading unless the
     * caller is about to use rect-mode snapshots. Freeplay loads an island
     * backdrop and then saves its own sparse clean rects; allocating four
     * full-tile backups here can exhaust the contiguous PS1 heap. */
    if (grSaveCleanOnScreenLoad)
        grSaveCleanBgTiles();
}

/*
 * Copy zone operations - minimal PS1 implementation.
 */
