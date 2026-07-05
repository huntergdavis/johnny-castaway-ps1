/* Sized for BOTH uses: 16 thumbnail rows of pixels (10240 bytes,
 * chunked path) and 5 RAW 2340-byte sectors (11700, verified
 * continuous path — headers included so each sector can prove its
 * address; see ps1CdReadContinuousInto). */
#define SCENE_EXPLORER_CHUNK_BYTES 11712u
static uint16 gSceneExplorerThumbChunk[SCENE_EXPLORER_CHUNK_BYTES / 2u];

/* The boot loading indicator borrows this buffer for its RLE sprite
 * (SHIP.DAT): during boot the explorer is untouched, and the indicator
 * is finished before the pause menu can open. 10240 bytes. */
void *grSceneExplorerChunkBorrow(uint32 *bytesOut)
{
    *bytesOut = sizeof(gSceneExplorerThumbChunk);
    return gSceneExplorerThumbChunk;
}

/* ROOT CAUSE of the console "rotated strip" glitch: the SDK's queued
 * LoadImage races the pause menu's concurrent chrome rendering. On real
 * silicon DrawSync can return before a CPU->VRAM transfer fully drains
 * (proven during the boot-ship work), so the menu's next GP0 command
 * words INJECT into the thumbnail pixel stream — every pixel after the
 * injection point shifts horizontally until the next chunk's fresh
 * upload command restores alignment. That is exactly a partial-height
 * "rotated" band at a timing-dependent position, and exactly what
 * emulators (synchronous transfers) can never reproduce.
 *
 * Fix by construction: the explorer paints thumbnails with DIRECT GP0
 * port writes — no queue, no DMA, nothing to interleave with — after
 * one DrawSync to quiesce whatever the menu already queued. Same
 * pattern the boot loading-galleon uses, validated on console. */
#define GR_SX_GP0     (*(volatile uint32 *)0xBF801810)
#define GR_SX_GPUSTAT (*(volatile uint32 *)0xBF801814)
/* GPU DMA channel (DMA2) control — bit 24 is channel-busy, CPU-side and
 * immune to the silicon's early "queue drained" reporting. */
#define GR_SX_D2_CHCR (*(volatile uint32 *)0xBF8010A8)


/* Paint diagnostics, surfaced on the explorer hint line: which chunk
 * (if any) hit the pace ceiling and how often. A ceiling hit means we
 * wrote into a full FIFO — words drop, and the pixel stream shifts for
 * the rest of that chunk: the surviving "middle strip" candidate. */
static int gSxPaceCeilingHits = 0;
static int gSxLastStallChunk = -1;
static int gSxCurrentChunk = -1;
static int gSxQuiesceExpired = 0;

int grSxDiagCeilingHits(void)    { return gSxPaceCeilingHits; }
int grSxDiagStallChunk(void)     { return gSxLastStallChunk; }
int grSxDiagQuiesceExpired(void) { return gSxQuiesceExpired; }

static void grSxGpuPace(uint32 readyBit)
{
    int i;
    for (i = 0; i < 400000; i++) {
        if (GR_SX_GPUSTAT & readyBit)
            return;
    }
    /* Ceiling hit: keep going — bailing mid-command would desync the
     * FIFO for real. The next pace call re-checks. */
    gSxPaceCeilingHits++;
    gSxLastStallChunk = gSxCurrentChunk;
}

/* Proper quiesce before direct GP0 writes. DrawSync alone is NOT enough
 * on real silicon (proven twice now): it can return while the menu's OT
 * linked-list DMA is still walking, and direct port writes then
 * interleave with the DMA's words — the "band on every preview" that
 * survived the first direct-GP0 rewrite. Wait for the DMA2 channel
 * itself, then the GPU ready bits. Same CPU-side-register pattern that
 * fixed the SPU transfer races. */
static void grSxQuiesce(void)
{
    int i;
    /* SDK QUEUE first — the piece every previous quiesce missed. A
     * queued-not-yet-started LoadImage (menu reprime bgTiles on
     * explorer entry) is invisible to BOTH DrawSync(0) (early-returns
     * on silicon) and the CHCR busy bit (transfer not started yet); it
     * auto-starts from the DMA IRQ in the middle of our direct
     * uploads. DrawSync(1) reports the queue length without lying —
     * wait for empty with a generous bound (a full bgTile reprime is
     * ~600 KB of DMA, several ms). */
    for (i = 0; i < 200000 && DrawSync(1) > 0; i++)
        ;
    if (DrawSync(1) > 0) {
        gSxQuiesceExpired++;
        gSxLastStallChunk = gSxCurrentChunk;
        /* Queue stuck = wedged GPU or lying SDK counter; either way
         * ResetGraph(1) restores a consistent idle state (bounded
         * recovery, not a crawl — see pmGpuDrainBounded). */
        ResetGraph(1);
    }
    /* No DrawSync(0) here: it is UNBOUNDED and spins forever on a
     * wedged GPU (console freeze with audio alive). Queue-empty above
     * plus the CHCR drain and sequential ready-bit waits below cover
     * everything DrawSync(0) was for, all bounded. */
    for (i = 0; i < 400000 && (GR_SX_D2_CHCR & (1u << 24)); i++)
        ;
    /* Drain expiry is the P0-invisible failure mode: DMA2 still busy
     * after the bound means our direct writes WILL interleave with the
     * in-flight linked-list walk. Count it (Q on the diag line) —
     * gSxGpuPace's P counter never sees this path. */
    if (GR_SX_D2_CHCR & (1u << 24)) {
        gSxQuiesceExpired++;
        gSxLastStallChunk = gSxCurrentChunk;
    }
    /* SEQUENTIAL ready-bit waits. The old single pace on (26|28)
     * returned when EITHER bit asserted — during an in-flight transfer
     * bit 28 (wants data) is set while bit 26 (idle) is not, so the
     * quiesce could pass mid-transfer. Require idle first, then
     * DMA-ready. */
    grSxGpuPace(1u << 26);
    grSxGpuPace(1u << 28);
}

/* Close out a direct upload: wait until the GPU has CONSUMED the whole
 * transfer and is back in command mode (GPUSTAT bit 26). Writing the
 * last word is not completion — the FIFO still holds the tail, and if
 * the caller returns and the menu's DrawOTag fires DMA2 into a GPU
 * still in CPU->VRAM receive mode, real silicon wedges (console: the
 * SQUARE test pattern froze the menu hard, then DrawSync on the wedged
 * GPU spun forever on the next swap). Bounded; expiry surfaces on the
 * Q diag counter. */
static void grSxCloseUpload(void)
{
    int i;
    for (i = 0; i < 400000; i++) {
        if (GR_SX_GPUSTAT & (1u << 26))
            return;
    }
    gSxQuiesceExpired++;
    gSxLastStallChunk = gSxCurrentChunk;
}

/* Direct CPU->VRAM rect upload. w must be even (GPU moves pixel pairs). */
static void grSxBlitRect(int x, int y, int w, int h, const uint16 *src)
{
    int i, words = (w * h) / 2;

    grSxGpuPace(1u << 26);
    GR_SX_GP0 = 0xA0000000u;
    GR_SX_GP0 = ((uint32)(uint16)y << 16) | (uint16)x;
    GR_SX_GP0 = ((uint32)(uint16)h << 16) | (uint16)w;
    for (i = 0; i < words; i++) {
        if ((i & 15) == 0)
            grSxGpuPace(1u << 28);
        GR_SX_GP0 = ((uint32)src[i * 2 + 1] << 16) | src[i * 2];
    }
    grSxCloseUpload();
}

static void grSxFill(int x, int y, int w, int h)
{
    if (w <= 0 || h <= 0)
        return;
    grSxGpuPace(1u << 26);
    GR_SX_GP0 = 0x02000000u;    /* fill, black */
    GR_SX_GP0 = ((uint32)(uint16)y << 16) | (uint16)x;
    GR_SX_GP0 = ((uint32)(uint16)h << 16) | (uint16)w;
}

static void grClearSceneExplorerRect(int x, int y, int w, int h)
{
    /* Band coordinates are all multiples of 16, matching the GPU fill
     * command's column granularity. Direct GP0 — see root-cause note. */
    grSxFill(x, y, w, h);
}

static int grResolveSceneExplorerFile(const char *slug, CdlFILE *cdfile)
{
    char path[24];
    char family[16];
    int i, fam_len;
    const char *digits;
    const char *abbrev;

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
    return ps1_streamResolveFile(path, cdfile);
}

/* Continuous-read group blit: 5 sectors = 10240 bytes = exactly 16
 * thumbnail rows. Runs between sector arrivals (~6.7 ms apart at 2x);
 * the paced blit + close-out is sub-ms, so the ring never laps. */
static int grSxThumbGroupBlit(void *ud, const uint8 *group,
                              uint32 firstSector, uint32 groupSectors)
{
    int y = (int)((firstSector * 2048u) / (SCENE_EXPLORER_THUMB_W * 2u));
    int rows = (int)((groupSectors * 2048u) / (SCENE_EXPLORER_THUMB_W * 2u));

    (void)ud;
    gSxCurrentChunk = (int)(firstSector / groupSectors);
    grSxQuiesce();
    grSxBlitRect(160, 120 + y, SCENE_EXPLORER_THUMB_W, rows,
                 (const uint16 *)group);
    return 1;
}

int grLoadSceneExplorerThumbnail(const char *slug)
{
    CdlFILE cdfile;

    if (!grResolveSceneExplorerFile(slug, &cdfile))
        return 0;

    /* Quiesce the menu's queued rendering AND its in-flight OT DMA;
     * from here on we only touch GP0 directly. */
    grSxQuiesce();

    /* Clear the four bands around the centered thumbnail to black so
     * the previous menu/scene pixels don't show through. Widths greater
     * than 320 are split because the reusable chunk buffer is 320 pixels
     * wide. */
    grClearSceneExplorerRect(0, 0, 640, 120);
    grClearSceneExplorerRect(0, 360, 640, 120);
    grClearSceneExplorerRect(0, 120, 160, 240);
    grClearSceneExplorerRect(480, 120, 160, 240);

    /* Continuous CdlReadN path, 0.9.14 VERIFIED edition. The 0.9.13-era
     * version trusted "1 data-ready event = 1 sector, in order" and real
     * silicon broke that silently (bands with every counter clean). Now
     * sectors arrive RAW (2340 bytes) carrying their own MSF address;
     * ps1CdReadContinuousInto verifies every sector's self-declared
     * position and compacts the 2048-byte payloads before the group
     * blit. A sector that lies aborts the read and the seek-exact
     * chunked loop below repaints — corruption is structurally unable
     * to reach VRAM. */
#define SX_CONTINUOUS_THUMB_READ 1
#if SX_CONTINUOUS_THUMB_READ
    {
        uint32 thumbBytes = (uint32)SCENE_EXPLORER_THUMB_W *
                            SCENE_EXPLORER_THUMB_H * 2u;
        uint32 thumbSectors = (thumbBytes + 2047u) / 2048u;
        uint32 ringSectors = sizeof(gSceneExplorerThumbChunk) / 2340u;

        if (ps1CdReadContinuousInto(&cdfile, thumbSectors,
                                    (uint8 *)gSceneExplorerThumbChunk,
                                    ringSectors, grSxThumbGroupBlit, NULL)) {
            gSxCurrentChunk = -1;
            return 1;
        }
        gSxCurrentChunk = -1;
        printf("JCSX cont-read fallback %s\n", slug ? slug : "?");
    }
#endif

    for (int y = 0; y < SCENE_EXPLORER_THUMB_H;
         y += SCENE_EXPLORER_THUMB_ROWS_PER_READ) {
        uint32 offset = (uint32)y * SCENE_EXPLORER_THUMB_W * 2UL;
        uint32 bytes = SCENE_EXPLORER_THUMB_W *
                       SCENE_EXPLORER_THUMB_ROWS_PER_READ * 2UL;

        gSxCurrentChunk = y / SCENE_EXPLORER_THUMB_ROWS_PER_READ;

        if (!ps1_streamReadAlignedIntoFile(&cdfile, offset, bytes,
                                           (uint8 *)gSceneExplorerThumbChunk)) {
            /* Partial load: black out the rows that never arrived so a
             * failed read shows as "not loaded" instead of a mix of the
             * new preview over the previous one (console D1 "one garbled
             * line"). y is 16-aligned, matching FILL granularity. */
            grSxQuiesce();
            grSxFill(160, 120 + y, SCENE_EXPLORER_THUMB_W,
                     SCENE_EXPLORER_THUMB_H - y);
            gSxCurrentChunk = -1;
            return 0;
        }

        /* Re-quiesce per chunk, not just at entry: each CD read above is
         * many ms with interrupts live, and anything that touched the GPU
         * in that window (ISR-side or otherwise) would inject into the
         * next blit. Entry-only quiesce left this door open. */
        grSxQuiesce();
        grSxBlitRect(160, 120 + y,
                     SCENE_EXPLORER_THUMB_W,
                     SCENE_EXPLORER_THUMB_ROWS_PER_READ,
                     gSceneExplorerThumbChunk);
    }
    gSxCurrentChunk = -1;

    return 1;
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
    int strip = 0;

    grSxQuiesce();
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
        grSxBlitRect(160, 120 + y,
                     SCENE_EXPLORER_THUMB_W,
                     SCENE_EXPLORER_THUMB_ROWS_PER_READ,
                     gSceneExplorerThumbChunk);
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
