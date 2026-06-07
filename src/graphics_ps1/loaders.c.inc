void grLoadBmp(struct TTtmSlot *ttmSlot, uint16 slotNo, char *strArg)
{
    grLoadBmpRAM(ttmSlot, slotNo, strArg);
}

/*
 * Release BMP sprite sheet from slot
 */
void grReleaseBmp(struct TTtmSlot *ttmSlot, uint16 bmpSlotNo)
{
    if (ttmSlot == NULL || bmpSlotNo >= MAX_BMP_SLOTS) {
        return;
    }

    /* Replay records keep raw indexedPixels pointers into BMP/PSB storage.
     * If this slot is being replaced during the active thread's TTM tick,
     * those records must be dropped before the backing storage is freed. */
    if (grCurrentThread != NULL && grCurrentThread->ttmSlot == ttmSlot) {
        uint8 writeIdx = 0;
        for (uint8 readIdx = 0; readIdx < grCurrentThread->numDrawnSprites; readIdx++) {
            struct TDrawnSprite *ds = &grCurrentThread->drawnSprites[readIdx];
            if (ds->imageNo == bmpSlotNo)
                continue;
            if (writeIdx != readIdx)
                grCurrentThread->drawnSprites[writeIdx] = *ds;
            writeIdx++;
        }
        grCurrentThread->numDrawnSprites = writeIdx;
        if (grCurrentThread->replayWriteCursor > writeIdx)
            grCurrentThread->replayWriteCursor = writeIdx;
    }

    /* Invalidate replay records that reference previous contents of this slot. */
    ttmSlot->spriteGen[bmpSlotNo]++;
    ttmSlot->loadedBmp[bmpSlotNo] = NULL;
    ttmSlot->loadedBmpNames[bmpSlotNo] = NULL;

    /* Free all sprites in this slot (PS1Surface structs only;
     * indexedPixels with indexedOwned=0 are NOT freed here because
     * they point into either the BMP resource data or the PSB buffer). */
    for (int i = 0; i < ttmSlot->numSprites[bmpSlotNo]; i++) {
        if (ttmSlot->sprites[bmpSlotNo][i] != NULL) {
            grFreeLayer(ttmSlot->sprites[bmpSlotNo][i]);
            ttmSlot->sprites[bmpSlotNo][i] = NULL;
        }
    }

    /* Free PSB data buffer if this slot was loaded from a PSB file.
     * Must happen AFTER freeing sprites since they pointed into it. */
    if (ttmSlot->psbData[bmpSlotNo] != NULL) {
        /* R33-soak fix: PSB buffer comes from ps1PilotLoadPsb /
         * ps1_streamReadCache → CACHE region (post-memInit). The prior
         * libc free() was a silent no-op on CACHE pointers, leaving
         * the ~80–120 KB PSB block leaked in CACHE forever. memFree(CACHE)
         * range-checks the pointer and routes correctly to either the
         * CACHE free-list or libc free. The R33m diagnostic showed
         * cacheUsed=512 KB with LRU empty — that 512 KB was leaked PSB
         * buffers from prior scenes. */
        if (memIsReady())
            memFree(MEM_REGION_CACHE, ttmSlot->psbData[bmpSlotNo]);
        else
            free(ttmSlot->psbData[bmpSlotNo]);
        ttmSlot->psbData[bmpSlotNo] = NULL;
    }

    ttmSlot->numSprites[bmpSlotNo] = 0;
}

/*
 * Try to load a pre-transcoded PSB (PS1 Sprite Bundle) file for a BMP resource.
 * PSB files have nibbles pre-swapped to PS1 order, eliminating runtime swap.
 *
 * Uses the compile-time PSB registry for O(log N) lookup — no CD probe needed
 * for BMPs that don't have PSB versions.
 *
 * On success: sprites loaded into slot, psbData stored for lifecycle management,
 * bmpResource stored in loadedBmp for dedup.
 * Returns 1 on success, 0 if no PSB available or load failed.
 */
static int grTryLoadPsb(struct TTtmSlot *ttmSlot, uint16 slotNo,
                         char *strArg, struct TBmpResource *bmpResource)
{
    char psbPath[64];
    char psbName[32];
    int nameLen;
    int i;
    uint32 psbSize;
    uint8 *psbBuf;
    PSBHeader *hdr;
    PSBFrame *frames;
    uint8 *pixelBase;
    int numToLoad;
    int framesLoaded;
    uint32 frameTableEnd;

    /* JOHNWALK.BMP — Phase 1+ of the walk plan promotes this to the
     * PSB-backed sprite route. Earlier guard removed 2026-04-29; the
     * registry lookup below pulls JOHNWALK.PSB's size from cd_layout. */

    /* Fast registry lookup — avoids any CD access for unknown BMPs. */
    psbSize = psbRegistryLookup(strArg);
    if (psbSize == 0) return 0;

    /* Build PSB name: JOHNWALK.BMP -> JOHNWALK.PSB */
    nameLen = strlen(strArg);
    if (nameLen < 5 || nameLen > 28) return 0;
    memcpy(psbName, strArg, nameLen + 1);
    /* Replace .BMP extension with .PSB */
    if (psbName[nameLen-4] == '.' &&
        (psbName[nameLen-3] == 'B' || psbName[nameLen-3] == 'b') &&
        (psbName[nameLen-2] == 'M' || psbName[nameLen-2] == 'm') &&
        (psbName[nameLen-1] == 'P' || psbName[nameLen-1] == 'p')) {
        psbName[nameLen-3] = 'P';
        psbName[nameLen-2] = 'S';
        psbName[nameLen-1] = 'B';
    } else {
        return 0;
    }

    /* Try loading PSB from the active scene pack first (offset-based,
     * no CdSearchFile needed — much faster than standalone file lookup). */
    psbBuf = ps1PilotLoadPsb(psbName, &psbSize);

    /* Fallback: load standalone PSB file from CD PSB/ directory.
     * Both the pack path (ps1PilotLoadPsb) and this fallback now
     * return MEM_REGION_CACHE-allocated buffers — free uniformly
     * via memFree(CACHE) below. */
    if (psbBuf == NULL) {
        if (!grBuildPath3(psbPath, sizeof(psbPath), "PSB\\", psbName, ""))
            return 0;
        psbBuf = ps1_streamReadCache(psbPath, 0, psbSize);
        if (psbBuf == NULL) return 0;
    }

    /* Validate PSB header */
    if (psbSize < sizeof(PSBHeader)) {
        if (memIsReady()) memFree(MEM_REGION_CACHE, psbBuf); else free(psbBuf);
        return 0;
    }

    hdr = (PSBHeader *)psbBuf;
    if (hdr->magic != PSB_MAGIC || hdr->version != PSB_VERSION) {
        if (memIsReady()) memFree(MEM_REGION_CACHE, psbBuf); else free(psbBuf);
        return 0;
    }

    if (hdr->numFrames == 0 || hdr->dataOffset > psbSize) {
        if (memIsReady()) memFree(MEM_REGION_CACHE, psbBuf); else free(psbBuf);
        return 0;
    }

    /* Cross-check totalSize against actual buffer size */
    if (hdr->totalSize > psbSize) {
        if (memIsReady()) memFree(MEM_REGION_CACHE, psbBuf); else free(psbBuf);
        return 0;
    }

    /* Verify frame table fits */
    frameTableEnd = sizeof(PSBHeader) + (uint32)hdr->numFrames * sizeof(PSBFrame);
    if (frameTableEnd > hdr->dataOffset) {
        if (memIsReady()) memFree(MEM_REGION_CACHE, psbBuf); else free(psbBuf);
        return 0;
    }

    /* Release any existing sprites in this slot */
    if (ttmSlot->numSprites[slotNo])
        grReleaseBmp(ttmSlot, slotNo);

    frames = (PSBFrame *)(psbBuf + sizeof(PSBHeader));
    pixelBase = psbBuf + hdr->dataOffset;
    numToLoad = hdr->numFrames;
    if (numToLoad > MAX_SPRITES_PER_BMP)
        numToLoad = MAX_SPRITES_PER_BMP;

    framesLoaded = 0;
    for (i = 0; i < numToLoad; i++) {
        PSBFrame *fr = &frames[i];
        PS1Surface *surface;

        /* Validate frame bounds against actual buffer */
        if (hdr->dataOffset + fr->offset + fr->size > psbSize) break;

        /* Validate frame dimensions — reject corrupt entries that would
         * waste RAM or confuse the compositing path (max 640x480). */
        if (fr->width == 0 || fr->height == 0 ||
            fr->width > 640 || fr->height > 480) break;

        /* Kept on libc malloc — sprite frame descriptors live with
         * their owning resource (BMP/PSB). Lifecycle is "until LRU
         * eviction." Migrating to BOOT would block memFreezeBoot.
         * Future: route through MEM_REGION_CACHE with companion frees
         * inside grReleaseBmp / the LRU evictor. */
        surface = (PS1Surface*)malloc(sizeof(PS1Surface));
        if (!surface) break;

        surface->width = fr->width;
        surface->height = fr->height;
        surface->x = 0;  /* RAM-based, not in VRAM */
        surface->y = 0;
        surface->clutX = 0;
        surface->clutY = 0;
        surface->nextTile = NULL;
        surface->pixels = NULL;

        /* Point directly into PSB data buffer (zero-copy).
         * PSB data is in PS1 nibble order — no runtime swap needed. */
        surface->indexedPixels = pixelBase + fr->offset;
        surface->indexedOwned = 0;  /* Don't free per-frame; whole buffer freed on release */
        surface->psbNibbles = 1;    /* Flag: PS1 nibble order */

        /* Multi-tile fields (not used for PSB, but init for safety) */
        surface->fullWidth = fr->width;
        surface->fullHeight = fr->height;
        surface->tileOffsetX = 0;
        surface->tileOffsetY = 0;

        ttmSlot->sprites[slotNo][i] = surface;
        framesLoaded++;
    }

    ttmSlot->numSprites[slotNo] = framesLoaded;

    /* If no frames loaded (corruption or OOM), free everything and fall back. */
    if (framesLoaded == 0) {
        if (memIsReady()) memFree(MEM_REGION_CACHE, psbBuf); else free(psbBuf);
        return 0;
    }

    /* NOTE: The PSB header + frame table (~16 + 12*N bytes) remain in the
     * buffer. Trimming them via memmove + realloc + pointer fixup was
     * rejected: the savings are tiny (e.g. 1.5KB for 120 frames) compared
     * with the memmove cost on a 33MHz R3000 and the fragility of
     * post-realloc pointer adjustment.  Keeping the header in-place is
     * simpler and safer. */

    /* Lifecycle: store PSB buffer in slot so grReleaseBmp can free it. */
    ttmSlot->psbData[slotNo] = psbBuf;

    /* Dedup: store the BMP resource pointer so repeated loads of the same
     * BMP into the same slot are detected and skipped by grLoadBmpRAM. */
    ttmSlot->loadedBmp[slotNo] = bmpResource;
    ttmSlot->loadedBmpNames[slotNo] = bmpResource->resName;

    return 1;
}

/*
 * Load BMP sprites into RAM as 4-bit indexed data (compact storage)
 * Stores raw 4-bit packed pixels in indexedPixels, palette lookup at composite time.
 * This uses 4x less memory than the previous 15-bit direct color approach.
 *
 * Load order:
 * 1. Dedup check — if this exact BMP is already loaded in the slot, keep it.
 * 2. PSB fast path — if a pre-transcoded PSB exists on CD, use it (zero-copy,
 *    no runtime nibble swap, compile-time registry lookup).
 * 3. BMP fallback — decompress from RESOURCE.001 / extracted BMP files.
 */
void grLoadBmpRAM(struct TTtmSlot *ttmSlot, uint16 slotNo, char *strArg)
{
    struct TBmpResource *bmpResource = findBmpResource(strArg);
    gStatLastBmpSlot = (uint16)(slotNo + 1U);
    gStatLastBmpFrames = 0;
    gStatLastBmpStatus = 1;  /* attempt */
    if (!bmpResource) {
        gStatLastBmpStatus = 2;  /* resource missing */
        return;
    }

    /* Dedup: if slot already has this exact BMP loaded (via PSB or BMP), keep it. */
    if (ttmSlot->numSprites[slotNo] > 0 && ttmSlot->loadedBmp[slotNo] == bmpResource) {
        gStatLastBmpFrames = ttmSlot->numSprites[slotNo];
        gStatLastBmpStatus = 3;  /* reused existing slot */
        return;
    }

    /* PSB fast path: try pre-transcoded PSB file (skips nibble swap).
     * Pass bmpResource so PSB loader can set loadedBmp for dedup. */
    if (grTryLoadPsb(ttmSlot, slotNo, strArg, bmpResource)) {
        return;
    }

    /* BMP fallback path */
    if (ttmSlot->numSprites[slotNo])
        grReleaseBmp(ttmSlot, slotNo);

    /* On-demand loading: load BMP data if not already loaded */
    if (!bmpResource->uncompressedData) {
        ps1_loadBmpData(bmpResource);
    }

    if (!bmpResource->uncompressedData) {
        gStatLastBmpStatus = 4;  /* no BMP bytes after load */
        return;
    }
    if (bmpResource->numImages < 1) {
        gStatLastBmpStatus = 5;  /* zero image metadata */
        return;
    }

    {
        int numToLoad = bmpResource->numImages;
        uint8 *srcPtr = bmpResource->uncompressedData;
        int framesLoaded = 0;

        if (numToLoad > MAX_SPRITES_PER_BMP) {
            grPs1StatBmpFrameCap((uint16)bmpResource->numImages, MAX_SPRITES_PER_BMP);
            fatalError("BMP frame overflow: %s has %d frames, MAX_SPRITES_PER_BMP=%d",
                       strArg, numToLoad, MAX_SPRITES_PER_BMP);
        }

        for (int frameIdx = 0; frameIdx < numToLoad; frameIdx++) {
            uint16 width = bmpResource->widths[frameIdx];
            uint16 height = bmpResource->heights[frameIdx];
            uint32 indexedSize = ((uint32)width * (uint32)height + 1) / 2;

            /* Kept on libc malloc — same rationale as PSB frame
             * surfaces above: lifecycle is "until LRU eviction,"
             * not strict boot. */
            PS1Surface *surface = (PS1Surface*)malloc(sizeof(PS1Surface));
            if (!surface) {
                gStatLastBmpStatus = 6;  /* allocation failure / partial install */
                break;
            }

            surface->width = width;
            surface->height = height;
            surface->x = 0;  /* Not in VRAM - RAM only */
            surface->y = 0;
            surface->clutX = 0;
            surface->clutY = 0;
            surface->nextTile = NULL;
            surface->pixels = NULL;  /* Not using 15-bit direct color */

            /* Zero-copy indexed frame: reference BMP resource memory directly.
             * This removes per-frame malloc/memcpy churn and cuts fragmentation. */
            surface->indexedPixels = srcPtr;
            surface->indexedOwned = 0;
            surface->psbNibbles = 0;

            /* Advance source pointer for next frame */
            srcPtr += indexedSize;

            /* Store in slot */
            ttmSlot->sprites[slotNo][frameIdx] = surface;
            framesLoaded++;
        }

        ttmSlot->numSprites[slotNo] = framesLoaded;
        ttmSlot->loadedBmp[slotNo] = bmpResource;
        ttmSlot->loadedBmpNames[slotNo] = bmpResource->resName;
        gStatLastBmpFrames = (uint16)framesLoaded;
        gStatLastBmpStatus = (framesLoaded == numToLoad) ? 7 : 8;  /* ok / short install */
        if (framesLoaded < numToLoad) {
            grPs1StatBmpShortLoad((uint16)numToLoad, (uint16)framesLoaded);
        }
    }
}

/*
 * Blit a RAM-stored sprite directly to framebuffer using LoadImage
 * Sprite must have been loaded with grLoadBmpRAM (15-bit direct color)
 * NOTE: No transparency support yet - black pixels will show
 */
