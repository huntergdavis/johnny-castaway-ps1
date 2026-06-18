void grClearScreen(PS1Surface *sfc)
{
    /* Clear entire screen using TILE primitive */
    grDrawRect(sfc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
}

/*
 * Draw background surface to screen
 * Re-LoadImages all tiles to framebuffer to restore background each frame.
 * Required when isbg=0 to erase previous frame's sprites.
 */
__attribute__((optimize("Os")))
void grDrawBackground(void)
{
    enum {
        GR_MAX_UPLOAD_RECTS = 8,
        GR_UPLOAD_BAND_MERGE_GAP = 0
    };
    /* Upload only dirty rows: union(prevDirty, currDirty) per tile.
     * prevDirty = rows restored at frame start (framebuffer still has old content).
     * currDirty = rows composited this frame (framebuffer has clean/old content). */
    PS1Surface *tiles[4] = { bgTile0, bgTile1, bgTile3, bgTile4 };
    RECT rect;
    int minYs[4];
    int maxYs[4];
    int bandTile[GR_MAX_UPLOAD_RECTS];
    int bandMinY[GR_MAX_UPLOAD_RECTS];
    int bandMaxY[GR_MAX_UPLOAD_RECTS];
    int bandCount = 0;
    int capped = 0;
    int dirtyCount = 0;
    uint16 uploadRects = 0;
    uint16 uploadRows = 0;
    uint32 uploadBytes = 0;
    uint32 perfStartTick = 0;

    for (int i = 0; i < 4; i++) {
        int minY = -1;
        int maxY = -1;

        minYs[i] = -1;
        maxYs[i] = -1;
        if (!tiles[i] || !tiles[i]->pixels)
            continue;

        /* Compute upload range = union(prevDirty, currDirty) */
        if (prevDirtyMinY[i] >= 0) {
            minY = prevDirtyMinY[i];
            maxY = prevDirtyMaxY[i];
        }
        if (currDirtyMinY[i] >= 0) {
            if (minY < 0) {
                minY = currDirtyMinY[i];
                maxY = currDirtyMaxY[i];
            } else {
                if (currDirtyMinY[i] < minY) minY = currDirtyMinY[i];
                if (currDirtyMaxY[i] > maxY) maxY = currDirtyMaxY[i];
            }
        }
        if (minY < 0) continue;  /* tile is fully clean — skip upload */

        minYs[i] = minY;
        maxYs[i] = maxY;
        dirtyCount++;
    }

    for (int i = 0; i < 4 && !capped; i++) {
        int y;

        if (minYs[i] < 0)
            continue;

        y = minYs[i];
        while (y <= maxYs[i]) {
            int startY;

            while (y <= maxYs[i] &&
                   prevDirtyRowMinX[i][y] < 0 &&
                   currDirtyRowMinX[i][y] < 0) {
                y++;
            }
            if (y > maxYs[i])
                break;

            startY = y;
            {
                int scanY = y + 1;
                int lastDirtyY = y;
                int cleanGap = 0;

                while (scanY <= maxYs[i]) {
                    if (prevDirtyRowMinX[i][scanY] >= 0 ||
                        currDirtyRowMinX[i][scanY] >= 0) {
                        lastDirtyY = scanY;
                        cleanGap = 0;
                    } else {
                        cleanGap++;
                        if (cleanGap > GR_UPLOAD_BAND_MERGE_GAP)
                            break;
                    }
                    scanY++;
                }
                y = lastDirtyY;
            }

            if (bandCount >= GR_MAX_UPLOAD_RECTS) {
                capped = 1;
                break;
            }
            bandTile[bandCount] = i;
            bandMinY[bandCount] = startY;
            bandMaxY[bandCount] = y;
            bandCount++;
            y++;
        }
    }

    if (ps1PerfEnabled && dirtyCount > 0)
        perfStartTick = ps1PerfTick();

    if (!capped && bandCount > 0) {
        for (int b = 0; b < bandCount; b++) {
            int i = bandTile[b];
            int minY = bandMinY[b];
            int h = bandMaxY[b] - minY + 1;
            uint32 w = tiles[i]->width;

            setRECT(&rect, (i & 1) ? 320 : 0, ((i & 2) ? 240 : 0) + minY, w, h);
            uploadRects++;
            uploadRows = (uint16)(uploadRows + (uint16)h);
            uploadBytes += (uint32)w * (uint32)h * sizeof(uint16);
            LoadImage(&rect, (uint32 *)(tiles[i]->pixels + minY * w));
        }

        DrawSync(0);
    } else {
        for (int i = 0; i < 4; i++) {
            if (minYs[i] < 0)
                continue;

            int minY = minYs[i];
            int h = maxYs[i] - minY + 1;
            uint32 w = tiles[i]->width;

            setRECT(&rect, (i & 1) ? 320 : 0, ((i & 2) ? 240 : 0) + minY, w, h);
            uploadRects++;
            uploadRows = (uint16)(uploadRows + (uint16)h);
            uploadBytes += (uint32)w * (uint32)h * sizeof(uint16);
            LoadImage(&rect, (uint32 *)(tiles[i]->pixels + minY * w));
        }

        if (dirtyCount > 0)
            DrawSync(0);
    }

    if (ps1PerfEnabled && dirtyCount > 0) {
        ps1PerfMarkUploadDirty(uploadRects, uploadRows, uploadBytes,
                               ps1PerfElapsedVBlanks(perfStartTick));
    }

    /* Advance dirty state: this frame's compositing becomes next frame's restore set. */
    grPromoteCurrDirtyToPrev();
}

/*
 * Fade out effect
 */
void grFadeOut()
{
    /* Force full dirty for fade — modifies all pixels */
    grMarkAllTilesDirty();
    grMarkPrevAllTilesDirty();

    /* 16 fade steps, ~2 frames each = ~0.5 sec at 60fps.
     * Uses (c >> 1) & 0x3DEF to halve all 3 color channels simultaneously:
     * the mask prevents bit leakage between R/G/B fields and clears STP. */
    for (int step = 0; step < 16; step++) {
        PS1Surface *tiles[] = { bgTile0, bgTile1, bgTile3, bgTile4 };
        for (int t = 0; t < 4; t++) {
            if (!tiles[t] || !tiles[t]->pixels) continue;
            uint32 count = tiles[t]->width * tiles[t]->height;
            uint32 *px32 = (uint32 *)tiles[t]->pixels;
            uint32 count32 = count >> 1;
            /* Process 2 pixels per uint32.
             * (c >> 1) & 0x3DEF works per-pixel; the mask at bit 15
             * also prevents leakage between the two packed pixels. */
            for (uint32 i = 0; i < count32; i++) {
                px32[i] = (px32[i] >> 1) & 0x3DEF3DEFu;
            }
        }

        /* Mark all dirty for each step's upload */
        grMarkAllTilesDirty();

        VSync(0);
        grDrawBackground();
    }
}

/* Helper to free a tile.
 *
 * Round 33: pixels + struct live in MEM_REGION_TRANSIENT post-Round-33.
 * memFree(TRANSIENT, ptr) only decrements the scene-alloc balance — the
 * underlying bytes survive until the next memSceneReset wipes the whole
 * region wholesale. That's intentional: between explicit freeBgTile calls
 * and the next scene boundary, freed tiles' bytes are dead but the
 * TRANSIENT bump pointer doesn't reclaim them (this is normal TRANSIENT
 * semantics). The bump pointer rewinds on memSceneReset.
 *
 * Pre-memInit allocations (the libc-fallback safe_malloc branch in
 * createEmptyBgTileRAM) are never actually hit in practice — bg-tiles
 * are first allocated by graphicsInit(), which runs after memInit().
 * The `else free()` path remains as defensive code for any future
 * pre-memInit caller. */
static void freeBgTile(PS1Surface **tile)
{
    if (*tile != NULL) {
        if ((*tile)->pixels) {
            if (memIsReady()) memFree(MEM_REGION_TRANSIENT, (*tile)->pixels);
            else free((*tile)->pixels);
        }
        if (memIsReady()) memFree(MEM_REGION_TRANSIENT, *tile);
        else free(*tile);
        *tile = NULL;
    }
}

void grReleaseBackgroundTiles(void)
{
    grFreeCleanBgTiles();
    grFreeCleanBgRects();

    freeBgTile(&bgTile0);
    freeBgTile(&bgTile1);
    freeBgTile(&bgTile2a);
    freeBgTile(&bgTile2b);
    freeBgTile(&bgTile3);
    freeBgTile(&bgTile4);
    freeBgTile(&bgTile5a);
    freeBgTile(&bgTile5b);

    grBackgroundSfc = NULL;

    if (grSavedZonesLayer != NULL) {
        grFreeLayer(grSavedZonesLayer);
        grSavedZonesLayer = NULL;
    }

    grForceFullRedrawNextFrame();
}

/* Round 33: per-scene TRANSIENT wipe hook.
 *
 * The bg-tile struct AND pixel buffer both live in MEM_REGION_TRANSIENT
 * (post-Round-33 migration), so memSceneReset has already reclaimed
 * every byte. The static slot pointers still hold the dangling
 * addresses; we NULL them so the next grLoadScreen / grInitEmptyBackground
 * sees them as "uninitialised" and re-allocates fresh in the new
 * TRANSIENT bump frame.
 *
 * Do NOT call freeBgTile here — that would dereference the dangling
 * struct (to read its pixels pointer) and call memFree(TRANSIENT, ...)
 * which would decrement the scene-alloc balance that memSceneReset
 * already zeroed. */
void grBackgroundTilesAssumeWiped(void)
{
    bgTile0 = NULL;
    bgTile1 = NULL;
    bgTile2a = NULL;
    bgTile2b = NULL;
    bgTile3 = NULL;
    bgTile4 = NULL;
    bgTile5a = NULL;
    bgTile5b = NULL;
    /* grBackgroundSfc points at one of the bgTile slots when a scene
     * is active. After the wipe the bytes behind it are gone — NULL it
     * so callers see "no background yet" and grLoadScreen / friends
     * repopulate it. */
    grBackgroundSfc = NULL;
    /* Force a full first-frame upload after the new scene's tiles
     * are populated; we know every pixel just got wiped. */
    grForceFullRedrawNextFrame();
}

/*
 * Helper: Create a background tile stored in RAM only (no VRAM upload)
 * For use with LoadImage directly to framebuffer
 * srcHeight parameter allows partial source data (rest filled with black)
 */
static void fillBgTileRAMPartial(PS1Surface *tile, uint8 *src,
                                 uint16 srcWidth, uint16 srcHeight,
                                 uint16 srcStartX, uint16 srcStartY,
                                 uint16 tileWidth)
{
    uint16 *dst = tile->pixels;

    /* Process 2 pixels per byte using palette LUT, with bounds checking.
     * Row base increment avoids per-pixel multiply. */
    for (uint16 y = 0; y < BG_TILE_HEIGHT; y++) {
        uint32 srcY = srcStartY + y;
        uint16 *dstRow = dst + (uint32)y * tileWidth;
        if (srcY >= srcHeight) {
            /* Entire row is outside source bounds — fill with black */
            memset(dstRow, 0, tileWidth * sizeof(uint16));
            continue;
        }
        uint32 srcOff = srcY * (uint32)srcWidth + srcStartX;
        /* Calculate how many pixels are within source bounds */
        uint16 validW = (srcStartX + tileWidth <= srcWidth) ? tileWidth
                        : (srcStartX < srcWidth ? srcWidth - srcStartX : 0);
        uint16 x = 0;
        /* Handle odd start pixel */
        if ((srcStartX & 1) && x < validW) {
            uint8 packed = src[srcOff >> 1];
            dstRow[0] = ttmPalette[packed & 0x0F];
            x = 1;
            srcOff++;
        }
        /* Process 2 pixels per byte */
        for (; x + 1 < validW; x += 2, srcOff += 2) {
            uint32 pair = palLutSierra[src[srcOff >> 1]];
            dstRow[x]     = (uint16)pair;
            dstRow[x + 1] = (uint16)(pair >> 16);
        }
        /* Handle trailing valid pixel */
        if (x < validW) {
            uint8 packed = src[srcOff >> 1];
            dstRow[x] = ttmPalette[(packed >> 4) & 0x0F];
            x++;
        }
        /* Fill remaining with black */
        if (x < tileWidth) {
            memset(&dstRow[x], 0, (tileWidth - x) * sizeof(uint16));
        }
    }

}

static PS1Surface *ensureBgTileRAMPartial(PS1Surface **slot, uint8 *src,
                                           uint16 srcWidth, uint16 srcHeight,
                                           uint16 srcStartX, uint16 srcStartY,
                                           uint16 tileWidth)
{
    if (*slot == NULL || (*slot)->pixels == NULL ||
        (*slot)->width != tileWidth || (*slot)->height != BG_TILE_HEIGHT) {
        ensureBgTileRAM(slot, tileWidth, BG_TILE_HEIGHT);
    } else {
        resetBgTileRAMFields(*slot, tileWidth, BG_TILE_HEIGHT);
    }

    fillBgTileRAMPartial(*slot, src, srcWidth, srcHeight,
                         srcStartX, srcStartY, tileWidth);
    return *slot;
}

static void grExpandScrPackedRow(uint16 *dst, const uint8 *src, uint16 byteCount)
{
    uint16 x = 0;
    for (uint16 i = 0; i < byteCount; i++, x += 2) {
        uint32 pair = palLutSierra[src[i]];
        dst[x] = (uint16)pair;
        dst[x + 1] = (uint16)(pair >> 16);
    }
}

static int grIsFullScreenScrResource(const struct TScrResource *scrResource)
{
    return scrResource != NULL &&
           scrResource->width == 640 &&
           scrResource->height == 480 &&
           scrResource->uncompressedSize == 153600UL;
}

static void grCopyFullScreenScrName(char *dst, uint32 dstBytes,
                                    const char *src)
{
    uint32 i;

    if (dst == NULL || dstBytes == 0)
        return;
    if (src == NULL) {
        dst[0] = '\0';
        return;
    }
    for (i = 0; i + 1 < dstBytes && src[i] != '\0'; i++)
        dst[i] = src[i];
    dst[i] = '\0';
}

static int grScrNameIsIslandBackdrop(const char *resName)
{
    /* OCEAN00.SCR / NIGHT.SCR are the island constants every island
     * scene reloads. Custom one-shot SCRs (suzy/building interiors)
     * have no reuse value: the next scene reverts to the island, so
     * letting them into the single cache slot evicts the constant and
     * forces a full CD re-read on the way back (walkstuf2 after suzy1:
     * screen_vb 85). */
    return resName != NULL &&
           (strncmp(resName, "OCEAN", 5) == 0 ||
            strncmp(resName, "NIGHT", 5) == 0);
}

static int (*gGrScrCacheReadmitHook)(void) = NULL;

/* Consecutive island screen loads whose SCR refill could not get a
 * buffer — the boundary rebuild trigger reads this. */
static int gGrScrRefillFailStreak = 0;

int grScrCacheRefillFailStreak(void)
{
    return gGrScrRefillFailStreak;
}

void grSetScrCacheReadmitHook(int (*fn)(void))
{
    gGrScrCacheReadmitHook = fn;
}

static int grEnsureFullScreenScrCache(const char *resName, uint32 bytes)
{
    if (!gFullScreenScrCacheEnabled ||
        resName == NULL ||
        bytes == 0 ||
        !grScrNameIsIslandBackdrop(resName))
        return 0;

    if (gFullScreenScrCache != NULL &&
        gFullScreenScrCacheBytes >= bytes) {
        gGrScrRefillFailStreak = 0;
        return 1;
    }

    if (gFullScreenScrCache != NULL) {
        memFree(MEM_REGION_CACHE, gFullScreenScrCache);
        gFullScreenScrCache = NULL;
    }
    gFullScreenScrCacheBytes = 0;
    gFullScreenScrCacheValid = 0;
    gFullScreenScrCacheName[0] = '\0';

    /* MEM_REGION_RATIONALE: optional packed full-screen SCR cache for the
     * async-loading branch. Stores 4bpp source bytes (150KB), not the 600KB
     * expanded bg tiles, and is freed when the proof toggle is disabled.
     * Opportunistic alloc: the SCR cache is the LEAST valuable retained
     * block, so its refill must never trigger relief — a 153600 refill
     * request exceeds every relief tier's early-stop and demolishes the
     * boot stable shape (walk slab, floors, window), which was the
     * scene-105 cascade behind the 400c/d/e soak BSODs. No hole means
     * no cache this round; retry on the next island screen load. */
    gFullScreenScrCache = (uint8 *)memTryAlloc(MEM_REGION_CACHE, bytes,
                                               "gr-scr-cache");
    if (gFullScreenScrCache == NULL &&
        grFlushCleanBgRectSlabs() > 0) {
        /* (The reserved relief hole is allowed on the allocator's
         * pressure pass, so this retry can land in the protected
         * slot once the squatting parked slabs are flushed.) */
        /* Re-admission may take strictly CHEAPER retention: sub-floor
         * parked slabs are re-creatable churn, while a permanently
         * absent SCR cache costs ~90 vb on every island hit (the
         * post-cascade steady state of the first fixed 150-soak:
         * the one-time evicted rect squatted in the SCR hole forever).
         * Floors and the boot stable shape stay untouched. */
        gFullScreenScrCache = (uint8 *)memTryAlloc(MEM_REGION_CACHE, bytes,
                                                   "gr-scr-cache");
    }
    /* (Floor-flushing escalation removed: it caused a floor<->SCR
     * ping-pong relief storm — 70x req=98304 — that fragmented the
     * region and BSOD'd scrfix3 at scene 232. The pool retention cap
     * in grCleanRectSlabPoolPark now keeps the SCR slot's hole free
     * instead, so re-admission lands without evicting anything.) */
    /* Escalation 2: ask the owner layer to drop the re-readable island
     * sheets (BACKGRND 94K + HOLIDAY 26K) wedged beside the slot. Costs
     * one sheet re-read in the next backdrop phase. Registered only
     * while the proof is on. */
    if (gFullScreenScrCache == NULL &&
        gGrScrCacheReadmitHook != NULL &&
        gGrScrCacheReadmitHook()) {
        gFullScreenScrCache = (uint8 *)memTryAlloc(MEM_REGION_CACHE, bytes,
                                                   "gr-scr-cache");
    }
    if (gFullScreenScrCache == NULL) {
        if (gGrScrRefillFailStreak < 1000)
            gGrScrRefillFailStreak++;
        return 0;
    }
    gGrScrRefillFailStreak = 0;
    /* Resident again: the slot no longer needs protection. */
    memCacheClearReservedHole();
    gFullScreenScrCacheBytes = bytes;
    grCopyFullScreenScrName(gFullScreenScrCacheName,
                            sizeof(gFullScreenScrCacheName),
                            resName);
    return 1;
}

static void grExpandFullScreenScrPackedToTiles(const uint8 *packed)
{
    uint16 y;

    ensureBgTileRAM(&bgTile0, 320, BG_TILE_HEIGHT);
    ensureBgTileRAM(&bgTile1, 320, BG_TILE_HEIGHT);
    ensureBgTileRAM(&bgTile3, 320, BG_TILE_HEIGHT);
    ensureBgTileRAM(&bgTile4, 320, BG_TILE_HEIGHT);
    bgTile2a = NULL;
    bgTile2b = NULL;
    bgTile5a = NULL;
    bgTile5b = NULL;

    for (y = 0; y < 480; y++) {
        const uint8 *srcRow = packed + ((uint32)y * SCR_STREAM_ROW_BYTES);
        if (y < BG_TILE_HEIGHT) {
            grExpandScrPackedRow(bgTile0->pixels + ((uint32)y * 320),
                                 srcRow, 160);
            grExpandScrPackedRow(bgTile1->pixels + ((uint32)y * 320),
                                 srcRow + 160, 160);
        } else {
            uint16 tileY = (uint16)(y - BG_TILE_HEIGHT);
            grExpandScrPackedRow(bgTile3->pixels + ((uint32)tileY * 320),
                                 srcRow, 160);
            grExpandScrPackedRow(bgTile4->pixels + ((uint32)tileY * 320),
                                 srcRow + 160, 160);
        }
    }
}

static void grUploadFullScreenBgTilesIfNeeded(void)
{
    RECT rect0, rect1, rect3, rect4;

    if (!grPresentDuringScreenLoad)
        return;

    if (bgTile0 && bgTile0->pixels) {
        setRECT(&rect0, 0, 0, bgTile0->width, bgTile0->height);
        LoadImage(&rect0, (uint32*)bgTile0->pixels);
    }
    if (bgTile1 && bgTile1->pixels) {
        setRECT(&rect1, 320, 0, bgTile1->width, bgTile1->height);
        LoadImage(&rect1, (uint32*)bgTile1->pixels);
    }
    if (bgTile3 && bgTile3->pixels) {
        setRECT(&rect3, 0, 240, bgTile3->width, bgTile3->height);
        LoadImage(&rect3, (uint32*)bgTile3->pixels);
    }
    if (bgTile4 && bgTile4->pixels) {
        setRECT(&rect4, 320, 240, bgTile4->width, bgTile4->height);
        LoadImage(&rect4, (uint32*)bgTile4->pixels);
    }
    DrawSync(0);
}

static int grLoadFullScreenScrCached(struct TScrResource *scrResource)
{
    if (!grIsFullScreenScrResource(scrResource) ||
        !gFullScreenScrCacheEnabled ||
        !gFullScreenScrCacheValid ||
        gFullScreenScrCache == NULL ||
        gFullScreenScrCacheBytes < scrResource->uncompressedSize ||
        strcmp(gFullScreenScrCacheName, scrResource->resName) != 0)
        return 0;

    grExpandFullScreenScrPackedToTiles(gFullScreenScrCache);
    grUploadFullScreenBgTilesIfNeeded();
    grBackgroundSfc = bgTile0;
    if (grSaveCleanOnScreenLoad)
        grSaveCleanBgTiles();
#if PS1_VERBOSE_DIAGNOSTICS
    printf("JCSCREEN scr-cache-apply %s bytes=%lu\n",
           scrResource->resName,
           (unsigned long)scrResource->uncompressedSize);
#endif
    return 1;
}

static int grLoadFullScreenScrStreamed(struct TScrResource *scrResource)
{
    CdlFILE cdfile;
    char path[32];
    uint16 y;
    int cacheTarget = 0;

    if (!grIsFullScreenScrResource(scrResource)) {
        return 0;
    }

    if (!grBuildPath3(path, sizeof(path), "SCR\\", scrResource->resName, ""))
        return 0;
    if (!ps1_streamResolveFile(path, &cdfile))
        return 0;

    ensureBgTileRAM(&bgTile0, 320, BG_TILE_HEIGHT);
    ensureBgTileRAM(&bgTile1, 320, BG_TILE_HEIGHT);
    ensureBgTileRAM(&bgTile3, 320, BG_TILE_HEIGHT);
    ensureBgTileRAM(&bgTile4, 320, BG_TILE_HEIGHT);
    bgTile2a = NULL;
    bgTile2b = NULL;
    bgTile5a = NULL;
    bgTile5b = NULL;
    cacheTarget = grEnsureFullScreenScrCache(scrResource->resName,
                                             scrResource->uncompressedSize);

    for (y = 0; y < 480; y = (uint16)(y + SCR_STREAM_ROWS)) {
        uint16 rows = (uint16)((480 - y) > SCR_STREAM_ROWS ? SCR_STREAM_ROWS : (480 - y));
        uint32 offset = (uint32)y * SCR_STREAM_ROW_BYTES;
        uint32 bytes = (uint32)rows * SCR_STREAM_ROW_BYTES;

        if (!ps1_streamReadAlignedIntoFile(&cdfile, offset, bytes,
                                           gScrStreamRows)) {
            if (cacheTarget)
                gFullScreenScrCacheValid = 0;
            return 0;
        }
        if (cacheTarget)
            memcpy(gFullScreenScrCache + offset, gScrStreamRows, bytes);

        for (uint16 row = 0; row < rows; row++) {
            uint16 screenY = (uint16)(y + row);
            const uint8 *srcRow = gScrStreamRows + ((uint32)row * SCR_STREAM_ROW_BYTES);
            if (screenY < BG_TILE_HEIGHT) {
                grExpandScrPackedRow(bgTile0->pixels + ((uint32)screenY * 320), srcRow, 160);
                grExpandScrPackedRow(bgTile1->pixels + ((uint32)screenY * 320), srcRow + 160, 160);
            } else {
                uint16 tileY = (uint16)(screenY - BG_TILE_HEIGHT);
                grExpandScrPackedRow(bgTile3->pixels + ((uint32)tileY * 320), srcRow, 160);
                grExpandScrPackedRow(bgTile4->pixels + ((uint32)tileY * 320), srcRow + 160, 160);
            }
        }
    }

    if (cacheTarget) {
        gFullScreenScrCacheValid = 1;
#if PS1_VERBOSE_DIAGNOSTICS
        printf("JCSCREEN scr-cache-fill %s bytes=%lu\n",
               scrResource->resName,
               (unsigned long)scrResource->uncompressedSize);
#endif
    }
    return 1;
}

/*
 * Load a Scene Explorer thumbnail SCR — bypasses the Sierra
 * scrResources registry so we don't have to register synthetic SCR
 * resources for the 27 thumbnails (and grow as scenes validate).
 *
 * Format: raw 320x240 16-bit RGB555 LE, 153,600 bytes. The file is
 * streamed in 16-row chunks directly to the framebuffer so Scene
 * Explorer no longer needs a 153 KB heap allocation while paused. That
 * allocation was fragile after the runtime grew persistent scene caches
 * and could fail from heap fragmentation, leaving the explorer text-only.
 *
 * Returns 1 on success, 0 if the file isn't on disc.
 */
enum {
    SCENE_EXPLORER_THUMB_W = 320,
    SCENE_EXPLORER_THUMB_H = 240,
    /* MUST keep each chunk's byte offset sector-aligned (multiple of
     * 2048): ps1_streamReadAlignedFromCdFileInto rejects unaligned reads
     * (returns 0). Chunk offset = chunkIndex * ROWS_PER_READ * (320*2).
     * ROWS_PER_READ=16 -> 10240 bytes = 5 sectors (aligned). The
     * 2026-06-12 change to 8 (5120 bytes = 2.5 sectors) made every odd
     * chunk unaligned, so the read failed and grLoadSceneExplorerThumbnail
     * returned 0 EVERY frame — the explorer re-cleared its chrome bands to
     * black on each retry, which read as the "purple bands flash" bug. The
     * 10 KB static chunk buffer is worth a working thumbnail. */
    SCENE_EXPLORER_THUMB_ROWS_PER_READ = 16,
};
