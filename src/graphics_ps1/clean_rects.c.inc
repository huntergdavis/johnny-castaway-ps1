void grSetFullScreenScrCacheEnabled(int enabled)
{
    gFullScreenScrCacheEnabled = enabled ? 1 : 0;
    if (!gFullScreenScrCacheEnabled) {
        if (gFullScreenScrCache != NULL) {
            memFree(MEM_REGION_CACHE, gFullScreenScrCache);
            gFullScreenScrCache = NULL;
        }
        gFullScreenScrCacheBytes = 0;
        gFullScreenScrCacheValid = 0;
        gFullScreenScrCacheName[0] = '\0';
    }
}

/* ---- Rect-based clean-pixel backup (option B). Alternative to full-tile
 *      clean copies: scene declares one or more rectangles that cover its
 *      dynamic regions; only those rects are backed up + restored. Massive
 *      memory savings for scenes that only animate a small portion of the
 *      screen (fishing1: wave strip + Johnny area ≈ 181 KB instead of 614
 *      KB for all four full-size clean tiles). */

#define GR_MAX_CLEAN_RECTS 8

struct TGrCleanRect {
    sint16 x, y;
    uint16 width, height;
    uint16 *pixels;
    uint32 capacityBytes;
    /* MEM_REGION tag for the pixels allocation (0=TRANSIENT,
     * 1=CACHE). Round 13 introduced dynamic routing: clean-rect
     * snapshots prefer TRANSIENT (right semantic) but spill to
     * CACHE when TRANSIENT can't fit. memFree must use the same
     * region the alloc came from. */
    uint8 pixelsRegion;
};

static struct TGrCleanRect gGrCleanRects[GR_MAX_CLEAN_RECTS];
static int gGrCleanRectCount = 0;
static int gGrCleanBgBlackMode = 0;
static int gGrCleanBgRectsForceCache = 0;

#define GR_SPU_RECT_ROW_SCRATCH_BYTES 1024u
static uint32 gGrSpuRectRowScratch[GR_SPU_RECT_ROW_SCRATCH_BYTES /
                                   sizeof(uint32)];

/* Copy a rectangle's pixels out of the 4 bg tiles into a flat buffer. */
static void grCleanRectCopyOut(struct TGrCleanRect *r)
{
    int sy;
    if (r->pixels == NULL || r->width == 0 || r->height == 0) return;
    for (sy = 0; sy < (int)r->height; sy++) {
        int destY = r->y + sy;
        if (destY < 0 || destY >= 480) continue;
        {
            PS1Surface *tileLeft, *tileRight;
            int tileLocalY;
            if (destY < 240) { tileLocalY = destY; tileLeft = bgTile0; tileRight = bgTile1; }
            else             { tileLocalY = destY - 240; tileLeft = bgTile3; tileRight = bgTile4; }
            uint16 *dstRow = r->pixels + (uint32)sy * (uint32)r->width;
            int xStart = r->x;
            int xEnd   = r->x + (int)r->width;
            if (xStart < 0) xStart = 0;
            if (xEnd > 640) xEnd = 640;
            if (tileLeft && tileLeft->pixels && xStart < 320) {
                int lx0 = xStart;
                int lx1 = (xEnd < 320) ? xEnd : 320;
                uint16 *src = tileLeft->pixels + (tileLocalY * (int)tileLeft->width) + lx0;
                memcpy(dstRow + (lx0 - r->x), src,
                       (size_t)(lx1 - lx0) * sizeof(uint16));
            }
            if (tileRight && tileRight->pixels && xEnd > 320) {
                int rx0 = (xStart > 320) ? xStart : 320;
                int rx1 = xEnd;
                uint16 *src = tileRight->pixels + (tileLocalY * (int)tileRight->width) + (rx0 - 320);
                memcpy(dstRow + (rx0 - r->x), src,
                       (size_t)(rx1 - rx0) * sizeof(uint16));
            }
        }
    }
}

/* Copy only the previous frame's dirty rows from a clean-rect backup. Upload
 * is already driven by prevDirty in grDrawBackground, matching tile-mode. */
static void grCleanRectCopyIn(const struct TGrCleanRect *r)
{
    int sy;
    uint32 copiedBytes = 0;
    int perfTrack = ps1PerfEnabled;
    if (r->pixels == NULL || r->width == 0 || r->height == 0) return;
    grEnsureDirtyRowState();
    for (sy = 0; sy < (int)r->height; sy++) {
        int destY = r->y + sy;
        if (destY < 0 || destY >= 480) continue;
        {
            PS1Surface *tileLeft, *tileRight;
            int tileLocalY;
            if (destY < 240) { tileLocalY = destY; tileLeft = bgTile0; tileRight = bgTile1; }
            else             { tileLocalY = destY - 240; tileLeft = bgTile3; tileRight = bgTile4; }
            const uint16 *srcRow = r->pixels + (uint32)sy * (uint32)r->width;
            int xStart = r->x;
            int xEnd   = r->x + (int)r->width;
            int leftIdx = (destY < 240) ? 0 : 2;
            int rightIdx = leftIdx + 1;
            if (xStart < 0) xStart = 0;
            if (xEnd > 640) xEnd = 640;
            if (tileLeft && tileLeft->pixels && xStart < 320 &&
                prevDirtyRowMinX[leftIdx][tileLocalY] >= 0) {
                int lx0 = xStart;
                int lx1 = (xEnd < 320) ? xEnd : 320;
                if (lx0 < prevDirtyRowMinX[leftIdx][tileLocalY])
                    lx0 = prevDirtyRowMinX[leftIdx][tileLocalY];
                if (lx1 > prevDirtyRowMaxX[leftIdx][tileLocalY] + 1)
                    lx1 = prevDirtyRowMaxX[leftIdx][tileLocalY] + 1;
                if (lx0 < lx1) {
                    uint16 *dst = tileLeft->pixels + (tileLocalY * (int)tileLeft->width) + lx0;
                    size_t bytes = (size_t)(lx1 - lx0) * sizeof(uint16);
                    memcpy(dst, srcRow + (lx0 - r->x), bytes);
                    if (perfTrack)
                        copiedBytes += (uint32)bytes;
                }
            }
            if (tileRight && tileRight->pixels && xEnd > 320 &&
                prevDirtyRowMinX[rightIdx][tileLocalY] >= 0) {
                int rx0 = (xStart > 320) ? (xStart - 320) : 0;
                int rx1 = xEnd - 320;
                if (rx0 < prevDirtyRowMinX[rightIdx][tileLocalY])
                    rx0 = prevDirtyRowMinX[rightIdx][tileLocalY];
                if (rx1 > prevDirtyRowMaxX[rightIdx][tileLocalY] + 1)
                    rx1 = prevDirtyRowMaxX[rightIdx][tileLocalY] + 1;
                if (rx0 < rx1) {
                    uint16 *dst = tileRight->pixels + (tileLocalY * (int)tileRight->width) + rx0;
                    size_t bytes = (size_t)(rx1 - rx0) * sizeof(uint16);
                    memcpy(dst, srcRow + ((rx0 + 320) - r->x), bytes);
                    if (perfTrack)
                        copiedBytes += (uint32)bytes;
                }
            }
        }
    }
    if (perfTrack)
        ps1PerfMarkRestore(copiedBytes);
}

static void grCleanRectCopyInFull(const struct TGrCleanRect *r)
{
    int sy;
    if (r->pixels == NULL || r->width == 0 || r->height == 0) return;
    for (sy = 0; sy < (int)r->height; sy++) {
        int destY = r->y + sy;
        if (destY < 0 || destY >= 480) continue;
        {
            PS1Surface *tileLeft, *tileRight;
            int tileLocalY;
            const uint16 *srcRow = r->pixels + (uint32)sy * (uint32)r->width;
            int xStart = r->x;
            int xEnd = r->x + (int)r->width;
            if (destY < 240) {
                tileLocalY = destY;
                tileLeft = bgTile0;
                tileRight = bgTile1;
            } else {
                tileLocalY = destY - 240;
                tileLeft = bgTile3;
                tileRight = bgTile4;
            }
            if (xStart < 0) xStart = 0;
            if (xEnd > 640) xEnd = 640;
            if (tileLeft && tileLeft->pixels && xStart < 320) {
                int lx0 = xStart;
                int lx1 = (xEnd < 320) ? xEnd : 320;
                if (lx0 < lx1) {
                    uint16 *dst = tileLeft->pixels +
                        (tileLocalY * (int)tileLeft->width) + lx0;
                    memcpy(dst, srcRow + (lx0 - r->x),
                           (size_t)(lx1 - lx0) * sizeof(uint16));
                    grMarkRectDirty(lx0, destY, lx1, destY + 1);
                }
            }
            if (tileRight && tileRight->pixels && xEnd > 320) {
                int rx0 = (xStart > 320) ? (xStart - 320) : 0;
                int rx1 = xEnd - 320;
                if (rx0 < rx1) {
                    uint16 *dst = tileRight->pixels +
                        (tileLocalY * (int)tileRight->width) + rx0;
                    memcpy(dst, srcRow + ((rx0 + 320) - r->x),
                           (size_t)(rx1 - rx0) * sizeof(uint16));
                    grMarkRectDirty(rx0 + 320, destY, rx1 + 320, destY + 1);
                }
            }
        }
    }
}

static uint32 grRestoreCleanBgSpanFromRects(int x, int y, int width)
{
    int xEnd;
    uint32 copiedBytes = 0;

    if (width <= 0 || y < 0 || y >= 480)
        return 0;
    xEnd = x + width;
    if (x < 0)
        x = 0;
    if (xEnd > 640)
        xEnd = 640;
    if (x >= xEnd)
        return 0;

    if (gGrCleanBgBlackMode) {
        PS1Surface *tileLeft;
        PS1Surface *tileRight;
        int tileLocalY;

        if (y < 240) {
            tileLocalY = y;
            tileLeft = bgTile0;
            tileRight = bgTile1;
        } else {
            tileLocalY = y - 240;
            tileLeft = bgTile3;
            tileRight = bgTile4;
        }

        if (tileLeft != NULL && tileLeft->pixels != NULL && x < 320) {
            int lx0 = x;
            int lx1 = (xEnd < 320) ? xEnd : 320;
            if (lx0 < lx1) {
                uint16 *dst = tileLeft->pixels + (tileLocalY * (int)tileLeft->width) + lx0;
                size_t bytes = (size_t)(lx1 - lx0) * sizeof(uint16);
                memset(dst, 0, bytes);
                grMarkRectDirty(lx0, y, lx1, y + 1);
                copiedBytes += (uint32)bytes;
            }
        }
        if (tileRight != NULL && tileRight->pixels != NULL && xEnd > 320) {
            int rx0 = (x > 320) ? (x - 320) : 0;
            int rx1 = xEnd - 320;
            if (rx0 < rx1) {
                uint16 *dst = tileRight->pixels + (tileLocalY * (int)tileRight->width) + rx0;
                size_t bytes = (size_t)(rx1 - rx0) * sizeof(uint16);
                memset(dst, 0, bytes);
                grMarkRectDirty(rx0 + 320, y, rx1 + 320, y + 1);
                copiedBytes += (uint32)bytes;
            }
        }

        return copiedBytes;
    }

    for (int i = 0; i < gGrCleanRectCount; i++) {
        const struct TGrCleanRect *r = &gGrCleanRects[i];
        int rx0;
        int rx1;
        int sy;

        if (r->pixels == NULL || r->width == 0 || r->height == 0)
            continue;
        if (y < r->y || y >= r->y + (int)r->height)
            continue;

        rx0 = x;
        rx1 = xEnd;
        if (rx0 < r->x)
            rx0 = r->x;
        if (rx1 > r->x + (int)r->width)
            rx1 = r->x + (int)r->width;
        if (rx0 >= rx1)
            continue;

        sy = y - r->y;
        {
            PS1Surface *tileLeft;
            PS1Surface *tileRight;
            int tileLocalY;
            const uint16 *srcRow = r->pixels + (uint32)sy * (uint32)r->width;

            if (y < 240) {
                tileLocalY = y;
                tileLeft = bgTile0;
                tileRight = bgTile1;
            } else {
                tileLocalY = y - 240;
                tileLeft = bgTile3;
                tileRight = bgTile4;
            }

            if (tileLeft != NULL && tileLeft->pixels != NULL && rx0 < 320) {
                int lx0 = rx0;
                int lx1 = (rx1 < 320) ? rx1 : 320;
                if (lx0 < lx1) {
                    uint16 *dst = tileLeft->pixels + (tileLocalY * (int)tileLeft->width) + lx0;
                    size_t bytes = (size_t)(lx1 - lx0) * sizeof(uint16);
                    memcpy(dst, srcRow + (lx0 - r->x), bytes);
                    grMarkRectDirty(lx0, y, lx1, y + 1);
                    copiedBytes += (uint32)bytes;
                }
            }
            if (tileRight != NULL && tileRight->pixels != NULL && rx1 > 320) {
                int rxLocal0 = (rx0 > 320) ? (rx0 - 320) : 0;
                int rxLocal1 = rx1 - 320;
                if (rxLocal0 < rxLocal1) {
                    uint16 *dst = tileRight->pixels + (tileLocalY * (int)tileRight->width) + rxLocal0;
                    size_t bytes = (size_t)(rxLocal1 - rxLocal0) * sizeof(uint16);
                    memcpy(dst, srcRow + ((rxLocal0 + 320) - r->x), bytes);
                    grMarkRectDirty(rxLocal0 + 320, y, rxLocal1 + 320, y + 1);
                    copiedBytes += (uint32)bytes;
                }
            }
        }
    }

    return copiedBytes;
}

static void grResetCleanBgRects(int releasePixels)
{
    int i;
    for (i = 0; i < GR_MAX_CLEAN_RECTS; i++) {
        if (releasePixels && gGrCleanRects[i].pixels) {
            /* Use the recorded region; dynamic routing in
             * grSaveCleanBgTiles may have placed pixels in
             * EITHER TRANSIENT or CACHE. */
            MemRegion freeRegion = gGrCleanRects[i].pixelsRegion
                ? MEM_REGION_CACHE : MEM_REGION_TRANSIENT;
            memFree(freeRegion, gGrCleanRects[i].pixels);
            gGrCleanRects[i].pixels = NULL;
            gGrCleanRects[i].capacityBytes = 0;
            gGrCleanRects[i].pixelsRegion = 0;
        }
        gGrCleanRects[i].x = 0;
        gGrCleanRects[i].y = 0;
        gGrCleanRects[i].width = 0;
        gGrCleanRects[i].height = 0;
    }
    gGrCleanRectCount = 0;
}

void grFreeCleanBgRects(void)
{
    grResetCleanBgRects(1);
    gGrCleanBgBlackMode = 0;
}

void grDeactivateCleanBgRects(void)
{
    grResetCleanBgRects(1);
    gGrCleanBgBlackMode = 0;
}

void grSetCleanBgBlackMode(int enabled)
{
    gGrCleanBgBlackMode = enabled ? 1 : 0;
}

void grPreallocCleanBgRects(const uint32 *capBytes, int n)
{
    /* Pre-allocation is unnecessary under the new memory-region allocator.
     * Clean-rect snapshots now live in TRANSIENT, which is wiped between
     * scenes — every grSaveCleanBgRects call allocates fresh from a
     * deterministic region. There's no fragmentation to pre-empt and no
     * benefit to reserving capacity in advance.
     *
     * Kept as a public symbol so existing boot code that calls this
     * compiles. The boot caller could be deleted in Phase 2 of the
     * mem-region rollout. */
    (void)capBytes;
    (void)n;
}

int grCleanBgRectsCount(void)
{
    return gGrCleanRectCount;
}

void grSetCleanBgRectsForceCache(int enabled)
{
    gGrCleanBgRectsForceCache = enabled ? 1 : 0;
}

void grRestoreBgRectsFull(void)
{
    int i;
    for (i = 0; i < gGrCleanRectCount; i++)
        grCleanRectCopyInFull(&gGrCleanRects[i]);
}

unsigned long grCleanBgRectsBytes(void)
{
    unsigned long total = 0;
    int i;
    for (i = 0; i < gGrCleanRectCount; i++)
        total += (unsigned long)gGrCleanRects[i].width *
                 (unsigned long)gGrCleanRects[i].height *
                 (unsigned long)sizeof(uint16);
    return total;
}

/* Caller-owned rect copy wrappers. Used by walk_pilot's persistent
 * walk-area buffer — same per-tile splitting logic as the gGrCleanRects
 * snapshots, but the buffer is owned outside of graphics_ps1's rect
 * machinery (so grSaveCleanBgRects/grFreeCleanBgRects don't touch it).
 * The dst/src buffer must be sized w * h * sizeof(uint16). */
void grCaptureBgRect(uint16 *dst, sint16 x, sint16 y, uint16 w, uint16 h)
{
    struct TGrCleanRect r;
    if (dst == NULL || w == 0 || h == 0)
        return;
    r.x = x;
    r.y = y;
    r.width = w;
    r.height = h;
    r.pixels = dst;
    r.capacityBytes = (uint32)w * (uint32)h * sizeof(uint16);
    grCleanRectCopyOut(&r);
}

void grRestoreBgRect(const uint16 *src, sint16 x, sint16 y, uint16 w, uint16 h)
{
    struct TGrCleanRect r;
    if (src == NULL || w == 0 || h == 0)
        return;
    r.x = x;
    r.y = y;
    r.width = w;
    r.height = h;
    r.pixels = (uint16 *)src;
    r.capacityBytes = (uint32)w * (uint32)h * sizeof(uint16);
    grCleanRectCopyIn(&r);
}

int grCaptureBgRectToSpu(uint32 spuOffset, sint16 x, sint16 y,
                         uint16 w, uint16 h, uint32 rowStrideBytes)
{
    uint32 rowBytes;
    uint32 totalBytes;
    int sy;

    if (w == 0 || h == 0)
        return 0;
    rowBytes = (uint32)w * (uint32)sizeof(uint16);
    if (rowStrideBytes < rowBytes ||
        rowStrideBytes > GR_SPU_RECT_ROW_SCRATCH_BYTES ||
        (rowStrideBytes & (PS1_SPU_CACHE_ALIGN - 1u)) != 0u ||
        (spuOffset & (PS1_SPU_CACHE_ALIGN - 1u)) != 0u)
        return 0;
    totalBytes = rowStrideBytes * (uint32)h;

    ps1SpuCacheInit();
    if (!ps1SpuCacheReady() ||
        ps1SpuCacheCapacity() < spuOffset ||
        ps1SpuCacheCapacity() - spuOffset < totalBytes)
        return 0;

    for (sy = 0; sy < (int)h; sy++) {
        int destY = y + sy;
        uint16 *dstRow = (uint16 *)gGrSpuRectRowScratch;
        memset(gGrSpuRectRowScratch, 0, (size_t)rowStrideBytes);

        if (destY >= 0 && destY < 480) {
            PS1Surface *tileLeft, *tileRight;
            int tileLocalY;
            int xStart = x;
            int xEnd = x + (int)w;

            if (destY < 240) {
                tileLocalY = destY;
                tileLeft = bgTile0;
                tileRight = bgTile1;
            } else {
                tileLocalY = destY - 240;
                tileLeft = bgTile3;
                tileRight = bgTile4;
            }
            if (xStart < 0) xStart = 0;
            if (xEnd > 640) xEnd = 640;
            if (tileLeft && tileLeft->pixels && xStart < 320) {
                int lx0 = xStart;
                int lx1 = (xEnd < 320) ? xEnd : 320;
                if (lx0 < lx1) {
                    uint16 *src = tileLeft->pixels +
                        (tileLocalY * (int)tileLeft->width) + lx0;
                    memcpy(dstRow + (lx0 - x), src,
                           (size_t)(lx1 - lx0) * sizeof(uint16));
                }
            }
            if (tileRight && tileRight->pixels && xEnd > 320) {
                int rx0 = (xStart > 320) ? xStart : 320;
                int rx1 = xEnd;
                if (rx0 < rx1) {
                    uint16 *src = tileRight->pixels +
                        (tileLocalY * (int)tileRight->width) + (rx0 - 320);
                    memcpy(dstRow + (rx0 - x), src,
                           (size_t)(rx1 - rx0) * sizeof(uint16));
                }
            }
        }

        if (!ps1SpuCacheWrite(spuOffset + rowStrideBytes * (uint32)sy,
                              gGrSpuRectRowScratch,
                              rowStrideBytes))
            return 0;
    }
    return 1;
}

void grRestoreBgRectFromSpu(uint32 spuOffset, sint16 x, sint16 y,
                            uint16 w, uint16 h, uint32 rowStrideBytes)
{
    uint32 rowBytes;
    uint32 totalBytes;
    uint32 copiedBytes = 0;
    int perfTrack = ps1PerfEnabled;
    int sy;

    if (w == 0 || h == 0)
        return;
    rowBytes = (uint32)w * (uint32)sizeof(uint16);
    if (rowStrideBytes < rowBytes ||
        rowStrideBytes > GR_SPU_RECT_ROW_SCRATCH_BYTES ||
        (rowStrideBytes & (PS1_SPU_CACHE_ALIGN - 1u)) != 0u ||
        (spuOffset & (PS1_SPU_CACHE_ALIGN - 1u)) != 0u)
        return;
    totalBytes = rowStrideBytes * (uint32)h;

    ps1SpuCacheInit();
    if (!ps1SpuCacheReady() ||
        ps1SpuCacheCapacity() < spuOffset ||
        ps1SpuCacheCapacity() - spuOffset < totalBytes)
        return;

    grEnsureDirtyRowState();
    for (sy = 0; sy < (int)h; sy++) {
        int destY = y + sy;
        PS1Surface *tileLeft, *tileRight;
        int tileLocalY;
        int xStart = x;
        int xEnd = x + (int)w;
        int leftIdx, rightIdx;
        int leftCopy = 0;
        int rightCopy = 0;
        int lx0 = 0, lx1 = 0;
        int rx0 = 0, rx1 = 0;
        const uint16 *srcRow;

        if (destY < 0 || destY >= 480)
            continue;
        if (destY < 240) {
            tileLocalY = destY;
            tileLeft = bgTile0;
            tileRight = bgTile1;
            leftIdx = 0;
        } else {
            tileLocalY = destY - 240;
            tileLeft = bgTile3;
            tileRight = bgTile4;
            leftIdx = 2;
        }
        rightIdx = leftIdx + 1;
        if (xStart < 0) xStart = 0;
        if (xEnd > 640) xEnd = 640;

        if (tileLeft && tileLeft->pixels && xStart < 320 &&
            prevDirtyRowMinX[leftIdx][tileLocalY] >= 0) {
            lx0 = xStart;
            lx1 = (xEnd < 320) ? xEnd : 320;
            if (lx0 < prevDirtyRowMinX[leftIdx][tileLocalY])
                lx0 = prevDirtyRowMinX[leftIdx][tileLocalY];
            if (lx1 > prevDirtyRowMaxX[leftIdx][tileLocalY] + 1)
                lx1 = prevDirtyRowMaxX[leftIdx][tileLocalY] + 1;
            leftCopy = (lx0 < lx1);
        }
        if (tileRight && tileRight->pixels && xEnd > 320 &&
            prevDirtyRowMinX[rightIdx][tileLocalY] >= 0) {
            rx0 = (xStart > 320) ? (xStart - 320) : 0;
            rx1 = xEnd - 320;
            if (rx0 < prevDirtyRowMinX[rightIdx][tileLocalY])
                rx0 = prevDirtyRowMinX[rightIdx][tileLocalY];
            if (rx1 > prevDirtyRowMaxX[rightIdx][tileLocalY] + 1)
                rx1 = prevDirtyRowMaxX[rightIdx][tileLocalY] + 1;
            rightCopy = (rx0 < rx1);
        }
        if (!leftCopy && !rightCopy)
            continue;

        if (!ps1SpuCacheRead(spuOffset + rowStrideBytes * (uint32)sy,
                             gGrSpuRectRowScratch,
                             rowStrideBytes))
            return;
        srcRow = (const uint16 *)gGrSpuRectRowScratch;

        if (leftCopy) {
            uint16 *dst = tileLeft->pixels +
                (tileLocalY * (int)tileLeft->width) + lx0;
            size_t bytes = (size_t)(lx1 - lx0) * sizeof(uint16);
            memcpy(dst, srcRow + (lx0 - x), bytes);
            if (perfTrack)
                copiedBytes += (uint32)bytes;
        }
        if (rightCopy) {
            uint16 *dst = tileRight->pixels +
                (tileLocalY * (int)tileRight->width) + rx0;
            size_t bytes = (size_t)(rx1 - rx0) * sizeof(uint16);
            memcpy(dst, srcRow + ((rx0 + 320) - x), bytes);
            if (perfTrack)
                copiedBytes += (uint32)bytes;
        }
    }
    if (perfTrack)
        ps1PerfMarkRestore(copiedBytes);
}

/* Set up rect-based clean backup. Drops any existing rects first (also any
 * full-tile clean copies from grSaveCleanBgTiles, via the caller's intent).
 * Returns count of rects successfully allocated. */
int grSaveCleanBgRects(const sint16 *xArr, const sint16 *yArr,
                       const uint16 *wArr, const uint16 *hArr, int n)
{
    int i;
    uint32 requiredBytes[GR_MAX_CLEAN_RECTS];
    int allocatedThisCall[GR_MAX_CLEAN_RECTS];

    grDeactivateCleanBgRects();
    gGrCleanBgBlackMode = 0;
    grFreeCleanBgTiles();  /* mutually exclusive: rect-mode replaces tile-mode */

    if (xArr == NULL || yArr == NULL || wArr == NULL || hArr == NULL || n <= 0)
        return 0;
    if (n > GR_MAX_CLEAN_RECTS) n = GR_MAX_CLEAN_RECTS;

    for (i = 0; i < GR_MAX_CLEAN_RECTS; i++) {
        requiredBytes[i] = 0;
        allocatedThisCall[i] = 0;
    }

    /* Atomic allocation phase: all requested rect buffers must exist before
     * any rect becomes active. Otherwise a partial clean-restore set can both
     * leak and leave stale pixels from a prior foreground frame. */
    /* Compute required bytes for every rect first, then allocate in
     * size-DESCENDING order. Round 15 fix: smaller rects allocated
     * first can fragment TRANSIENT such that a later large rect
     * doesn't fit, even though TRANSIENT had enough space at start.
     * Visitor5 specifically: 97 KB rect last in iteration order would
     * fail when 60 KB was left after smaller rects took TRANSIENT
     * first. Sorting biggest-first gives the large rect first crack
     * at the contiguous TRANSIENT space.
     *
     * Selection sort by index — n ≤ 8 so O(n²) is fine. */
    int sortedIdx[GR_MAX_CLEAN_RECTS];
    for (i = 0; i < n; i++) {
        requiredBytes[i] = (uint32)wArr[i] * (uint32)hArr[i] * (uint32)sizeof(uint16);
        if (requiredBytes[i] == 0)
            goto fail;
        sortedIdx[i] = i;
    }
    /* Sort indices by requiredBytes descending. */
    for (int a = 0; a < n - 1; a++) {
        int maxJ = a;
        for (int b = a + 1; b < n; b++) {
            if (requiredBytes[sortedIdx[b]] > requiredBytes[sortedIdx[maxJ]])
                maxJ = b;
        }
        if (maxJ != a) {
            int tmp = sortedIdx[a];
            sortedIdx[a] = sortedIdx[maxJ];
            sortedIdx[maxJ] = tmp;
        }
    }
    /* Allocate in size-descending order. The rect at original index
     * `idx` is still stored at gGrCleanRects[idx]. */
    for (int s = 0; s < n; s++) {
        int idx = sortedIdx[s];
        /* MEM_REGION_RATIONALE: per-scene clean-rect snapshot (one of
         * 6 atomic slots; see comment block above the for loop).
         * Dynamic routing (Round 14): prefer TRANSIENT for correct
         * wholesale-wipe semantics; spill to CACHE when TRANSIENT
         * lacks contiguous space. Each rect records which region
         * its pixels came from so the matching memFree is used
         * (memFree's range-check would otherwise mismatch and
         * double-free via the TransientLibcEntry list). */
        const size_t transRemaining = MEM_TRANSIENT_BUDGET -
            memRegionUsed((unsigned int)MEM_REGION_TRANSIENT);
        /* Round 33-soak update: drop the TRANSIENT_RESERVE that
         * previously forced clean-rects into CACHE when TRANSIENT
         * was within 16 KB of full. CACHE is NOT wiped per-scene,
         * so spilling a 70–100 KB clean-rect into it across many
         * scene transitions accumulates fragmentation that
         * eventually breaks a later CACHE alloc (R33 soak BSOD at
         * 226s on stand6 with CACHE peak 568 KB and 113 KB free
         * but fragmented below the 71 KB request). The "other
         * TRANSIENT allocs" (sound events, setup segment) that
         * the reserve was protecting have their own libc-fallback
         * via TransientLibcEntry — they don't lose correctness
         * if TRANSIENT fills, just transparently spill to libc and
         * still get per-scene-wiped at memSceneReset. */
        MemRegion target;
        if (gGrCleanBgRectsForceCache) {
            target = MEM_REGION_CACHE;
        } else if (requiredBytes[idx] <= transRemaining) {
            target = MEM_REGION_TRANSIENT;
        } else {
            target = MEM_REGION_CACHE;
        }
        /* MEM_REGION_RATIONALE: clean-rect pixels prefer TRANSIENT for
         * scene-lifetime snapshots and spill to CACHE only when needed. */
        gGrCleanRects[idx].pixels = (uint16 *)memAlloc(target,
                                                     requiredBytes[idx],
                                                     "grCleanRectPixels");
        gGrCleanRects[idx].pixelsRegion =
            (target == MEM_REGION_CACHE) ? 1u : 0u;
        gGrCleanRects[idx].capacityBytes = requiredBytes[idx];
        allocatedThisCall[idx] = 1;
    }

    for (i = 0; i < n; i++) {
        gGrCleanRects[i].x = xArr[i];
        gGrCleanRects[i].y = yArr[i];
        gGrCleanRects[i].width = wArr[i];
        gGrCleanRects[i].height = hArr[i];
        grCleanRectCopyOut(&gGrCleanRects[i]);
    }
    gGrCleanRectCount = n;

    /* Force a full first-frame upload. FG2 restores clean rects after setup,
     * but static pixels outside those rects still need an initial refresh. */
    grMarkAllTilesDirty();
    grMarkPrevAllTilesDirty();
    return gGrCleanRectCount;

fail:
    for (i = 0; i < n; i++) {
        if (allocatedThisCall[i] && gGrCleanRects[i].pixels != NULL) {
            /* Use the recorded region (dynamic routing). */
            MemRegion freeRegion = gGrCleanRects[i].pixelsRegion
                ? MEM_REGION_CACHE : MEM_REGION_TRANSIENT;
            memFree(freeRegion, gGrCleanRects[i].pixels);
            gGrCleanRects[i].pixels = NULL;
            gGrCleanRects[i].capacityBytes = 0;
            gGrCleanRects[i].pixelsRegion = 0;
        }
    }
    grDeactivateCleanBgRects();
    return 0;
}

int grSaveCleanBgRectsSplit(const sint16 *xArr, const sint16 *yArr,
                            const uint16 *wArr, const uint16 *hArr, int n,
                            uint32 maxBytesPerRect)
{
    sint16 sx[GR_MAX_CLEAN_RECTS];
    sint16 sy[GR_MAX_CLEAN_RECTS];
    uint16 sw[GR_MAX_CLEAN_RECTS];
    uint16 sh[GR_MAX_CLEAN_RECTS];
    int outCount = 0;
    int i;

    if (maxBytesPerRect == 0)
        return grSaveCleanBgRects(xArr, yArr, wArr, hArr, n);
    if (xArr == NULL || yArr == NULL || wArr == NULL || hArr == NULL || n <= 0)
        return 0;

    for (i = 0; i < n; i++) {
        uint16 remaining;
        sint16 curY;
        uint32 bytesPerRow;
        uint16 maxRows;

        if (wArr[i] == 0 || hArr[i] == 0)
            return 0;

        bytesPerRow = (uint32)wArr[i] * (uint32)sizeof(uint16);
        maxRows = (uint16)(maxBytesPerRect / bytesPerRow);
        if (maxRows == 0)
            maxRows = 1;

        remaining = hArr[i];
        curY = yArr[i];
        while (remaining > 0) {
            uint16 stripH = remaining;
            if (stripH > maxRows)
                stripH = maxRows;
            if (outCount >= GR_MAX_CLEAN_RECTS)
                return 0;
            sx[outCount] = xArr[i];
            sy[outCount] = curY;
            sw[outCount] = wArr[i];
            sh[outCount] = stripH;
            outCount++;
            curY = (sint16)(curY + stripH);
            remaining = (uint16)(remaining - stripH);
        }
    }

    return grSaveCleanBgRects(sx, sy, sw, sh, outCount);
}

/* Per-frame: restore each clean rect from its saved buffer into bg tiles.
 * Caller uses this INSTEAD of grRestoreBgTiles when in rect-mode. */
void grRestoreBgFromRects(void)
{
    int i;
    /* Clear currDirty at start of new frame, mirroring grRestoreBgTiles. */
    grClearCurrDirtyState();
    for (i = 0; i < gGrCleanRectCount; i++) {
        if (gGrCleanRects[i].pixels)
            grCleanRectCopyIn(&gGrCleanRects[i]);
    }
}

/*
 * Free clean tile copies to reclaim memory (~600KB).
 * Called when switching to non-island (black) backgrounds where clean copies aren't needed.
 */
void grFreeCleanBgTiles(void)
{
    if (bgTile0Clean) { free(bgTile0Clean); bgTile0Clean = NULL; }
    if (bgTile1Clean) { free(bgTile1Clean); bgTile1Clean = NULL; }
    if (bgTile3Clean) { free(bgTile3Clean); bgTile3Clean = NULL; }
    if (bgTile4Clean) { free(bgTile4Clean); bgTile4Clean = NULL; }

    /* No clean copies → force full upload on next frame */
    grMarkAllTilesDirty();
    grMarkPrevAllTilesDirty();
}

/*
 * Ensure clean background copies exist before frame restore/composite.
 */
void grEnsureCleanBgTiles(void)
{
    if (bgTile0Clean && bgTile1Clean && bgTile3Clean && bgTile4Clean) {
        return;
    }
    grSaveCleanBgTiles();
}

/*
 * Restore background tiles from clean copies (call at start of each frame).
 * Only restores rows that were dirtied by the previous frame's compositing.
 * prevDirty is preserved for grDrawBackground's union upload.
 */
void grRestoreBgTiles(void)
{
    PS1Surface *tiles[4] = { bgTile0, bgTile1, bgTile3, bgTile4 };
    const uint16 *clean[4] = { bgTile0Clean, bgTile1Clean, bgTile3Clean, bgTile4Clean };
    uint32 restoredBytes = 0;
    int perfTrack = ps1PerfEnabled;

    /* Clear currDirty for new frame's compositing */
    grClearCurrDirtyState();

    for (int i = 0; i < 4; i++) {
        if (!tiles[i] || !tiles[i]->pixels || !clean[i]) continue;

        int minY = prevDirtyMinY[i];
        int maxY = prevDirtyMaxY[i];
        uint32 w = tiles[i]->width;
        if (minY < 0) continue;  /* tile was clean last frame */

        for (int row = minY; row <= maxY; row++) {
            int minX = prevDirtyRowMinX[i][row];
            int maxX = prevDirtyRowMaxX[i][row];
            int copyWidth;
            uint32 copyBytes;
            uint16 *dst;
            const uint16 *src;

            if (minX < 0)
                continue;
            if (maxX < minX || maxX >= (int)w) {
                minX = 0;
                maxX = (int)w - 1;
            }

            copyWidth = maxX - minX + 1;
            copyBytes = (uint32)copyWidth * sizeof(uint16);
            dst = tiles[i]->pixels + (row * (int)w) + minX;
            src = clean[i] + (row * (int)w) + minX;
            memcpy(dst, src, (size_t)copyBytes);
            if (perfTrack)
                restoredBytes += copyBytes;
        }
    }

    if (perfTrack)
        ps1PerfMarkRestore(restoredBytes);
}

void grRestoreBackgroundRectForFrame(int x, int y, int width, int height)
{
    grClearCurrDirtyState();

    if (width <= 0 || height <= 0)
        return;

    grRestoreRectFromCleanBg(x, y, width, height);
}

void grRestoreAndCompositeDirect16BackgroundRectForFrame(int x, int y, int width, int height,
                                                         const uint16 *srcPixels)
{
    int rectEndX;
    int rectEndY;

    grClearCurrDirtyState();

    if (srcPixels == NULL || width <= 0 || height <= 0)
        return;

    if (x < 0) {
        width += x;
        x = 0;
    }
    if (y < 0) {
        height += y;
        y = 0;
    }
    if (x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT)
        return;
    if (x + width > SCREEN_WIDTH)
        width = SCREEN_WIDTH - x;
    if (y + height > SCREEN_HEIGHT)
        height = SCREEN_HEIGHT - y;
    if (width <= 0 || height <= 0)
        return;

    rectEndX = x + width;
    rectEndY = y + height;

    if (x >= 0 && y >= 0 && rectEndX <= 640 && rectEndY <= 480) {
        int tileBaseX = (x >= 320) ? 320 : 0;
        if (rectEndX <= tileBaseX + 320) {
            grMarkSingleColumnDirty(tileBaseX, x, width, y, rectEndY);
            if (y < 240 && rectEndY > 240) {
                PS1Surface *topTile = (tileBaseX == 0) ? bgTile0 : bgTile1;
                PS1Surface *bottomTile = (tileBaseX == 0) ? bgTile3 : bgTile4;
                const uint16 *topClean = (tileBaseX == 0) ? bgTile0Clean : bgTile1Clean;
                const uint16 *bottomClean = (tileBaseX == 0) ? bgTile3Clean : bgTile4Clean;
                int tileLocalX = x - tileBaseX;
                int topRows = 240 - y;
                if (topRows < 0)
                    topRows = 0;
                if (topRows > height)
                    topRows = height;

                grEnsureCleanBgTiles();

                if (topTile != NULL && topTile->pixels != NULL && topClean != NULL) {
                    for (int row = 0; row < topRows; row++) {
                        uint16 *dst = topTile->pixels + ((y + row) * (int)topTile->width) + tileLocalX;
                        const uint16 *clean = topClean + ((y + row) * (int)topTile->width) + tileLocalX;
                        const uint16 *src = srcPixels + ((uint32)row * (uint32)width);
                        memcpy(dst, clean, (size_t)width * sizeof(uint16));
                        compositeDirectOpaqueRuns(dst, src, width);
                    }
                }
                if (bottomTile != NULL && bottomTile->pixels != NULL && bottomClean != NULL) {
                    for (int row = 0; row < (height - topRows); row++) {
                        uint16 *dst = bottomTile->pixels + (row * (int)bottomTile->width) + tileLocalX;
                        const uint16 *clean = bottomClean + (row * (int)bottomTile->width) + tileLocalX;
                        const uint16 *src = srcPixels + ((uint32)(topRows + row) * (uint32)width);
                        memcpy(dst, clean, (size_t)width * sizeof(uint16));
                        compositeDirectOpaqueRuns(dst, src, width);
                    }
                }
                return;
            }

            {
                PS1Surface *tile;
                const uint16 *clean;
                int tileLocalX = x - tileBaseX;
                int startLocalY;

                if (y < 240) {
                    tile = (tileBaseX == 0) ? bgTile0 : bgTile1;
                    clean = (tileBaseX == 0) ? bgTile0Clean : bgTile1Clean;
                    startLocalY = y;
                } else {
                    tile = (tileBaseX == 0) ? bgTile3 : bgTile4;
                    clean = (tileBaseX == 0) ? bgTile3Clean : bgTile4Clean;
                    startLocalY = y - 240;
                }

                grEnsureCleanBgTiles();

                if (tile == NULL || tile->pixels == NULL || clean == NULL)
                    return;

                for (int row = 0; row < height; row++) {
                    uint16 *dst = tile->pixels + ((startLocalY + row) * (int)tile->width) + tileLocalX;
                    const uint16 *cleanRow = clean + ((startLocalY + row) * (int)tile->width) + tileLocalX;
                    const uint16 *srcRow = srcPixels + ((uint32)row * (uint32)width);
                    memcpy(dst, cleanRow, (size_t)width * sizeof(uint16));
                    compositeDirectOpaqueRuns(dst, srcRow, width);
                }
            }
            return;
        }
    }

    grEnsureCleanBgTiles();
    grMarkRectDirty(x, y, rectEndX, rectEndY);
    for (int row = y; row < rectEndY; row++) {
        int tileLocalY;
        PS1Surface *tileLeft;
        PS1Surface *tileRight;
        const uint16 *cleanLeft;
        const uint16 *cleanRight;
        int destStartX = x;
        int destEndX = rectEndX;
        int srcRow = row - y;

        if (row < 240) {
            tileLocalY = row;
            tileLeft = bgTile0;
            tileRight = bgTile1;
            cleanLeft = bgTile0Clean;
            cleanRight = bgTile1Clean;
        } else {
            tileLocalY = row - 240;
            tileLeft = bgTile3;
            tileRight = bgTile4;
            cleanLeft = bgTile3Clean;
            cleanRight = bgTile4Clean;
        }

        if (tileLeft != NULL && tileLeft->pixels != NULL && cleanLeft != NULL && destStartX < 320) {
            int lx0 = destStartX;
            int lx1 = (destEndX < 320) ? destEndX : 320;
            uint16 *dst = tileLeft->pixels + (tileLocalY * (int)tileLeft->width) + lx0;
            const uint16 *clean = cleanLeft + (tileLocalY * (int)tileLeft->width) + lx0;
            const uint16 *src = srcPixels + ((uint32)srcRow * (uint32)width) + (uint32)(lx0 - x);
            int span = lx1 - lx0;
            memcpy(dst, clean, (size_t)span * sizeof(uint16));
            compositeDirectOpaqueRuns(dst, src, span);
        }

        if (tileRight != NULL && tileRight->pixels != NULL && cleanRight != NULL && destEndX > 320) {
            int rx0 = (destStartX > 320) ? destStartX : 320;
            int rx1 = destEndX;
            uint16 *dst = tileRight->pixels + (tileLocalY * (int)tileRight->width) + (rx0 - 320);
            const uint16 *clean = cleanRight + (tileLocalY * (int)tileRight->width) + (rx0 - 320);
            const uint16 *src = srcPixels + ((uint32)srcRow * (uint32)width) + (uint32)(rx0 - x);
            int span = rx1 - rx0;
            memcpy(dst, clean, (size_t)span * sizeof(uint16));
            compositeDirectOpaqueRuns(dst, src, span);
        }
    }
}

static void grRestoreTileRect(PS1Surface *dstTile,
                              const uint16 *srcClean,
                              int tileScreenX,
                              int tileScreenY,
                              int rectX,
                              int rectY,
                              int rectW,
                              int rectH)
{
    int copyStartX;
    int copyStartY;
    int copyEndX;
    int copyEndY;
    int row;

    if (dstTile == NULL || dstTile->pixels == NULL || srcClean == NULL)
        return;
    if (rectW <= 0 || rectH <= 0)
        return;

    copyStartX = (rectX > tileScreenX) ? rectX : tileScreenX;
    copyStartY = (rectY > tileScreenY) ? rectY : tileScreenY;
    copyEndX = rectX + rectW;
    copyEndY = rectY + rectH;

    if (copyEndX > tileScreenX + (int)dstTile->width)
        copyEndX = tileScreenX + (int)dstTile->width;
    if (copyEndY > tileScreenY + (int)dstTile->height)
        copyEndY = tileScreenY + (int)dstTile->height;

    if (copyStartX >= copyEndX || copyStartY >= copyEndY)
        return;

    for (row = copyStartY; row < copyEndY; row++) {
        int tileRow = row - tileScreenY;
        int tileCol = copyStartX - tileScreenX;
        int copyWidth = copyEndX - copyStartX;
        uint16 *dst = dstTile->pixels + (tileRow * dstTile->width) + tileCol;
        const uint16 *src = srcClean + (tileRow * dstTile->width) + tileCol;
        memcpy(dst, src, (size_t)copyWidth * sizeof(uint16));
    }
}

static void grRestoreRectSingleColumn(const uint16 *srcCleanTop,
                                      PS1Surface *dstTop,
                                      const uint16 *srcCleanBottom,
                                      PS1Surface *dstBottom,
                                      int tileBaseX,
                                      int x,
                                      int y,
                                      int width,
                                      int height)
{
    int tileLocalX;
    int topRows;
    int bottomRows;

    if (dstTop == NULL || dstBottom == NULL ||
        dstTop->pixels == NULL || dstBottom->pixels == NULL ||
        srcCleanTop == NULL || srcCleanBottom == NULL) {
        return;
    }

    tileLocalX = x - tileBaseX;
    topRows = 240 - y;
    if (topRows < 0)
        topRows = 0;
    if (topRows > height)
        topRows = height;
    bottomRows = height - topRows;

    for (int row = 0; row < topRows; row++) {
        int tileLocalY = y + row;
        uint16 *dst = dstTop->pixels + (tileLocalY * (int)dstTop->width) + tileLocalX;
        const uint16 *src = srcCleanTop + (tileLocalY * (int)dstTop->width) + tileLocalX;
        memcpy(dst, src, (size_t)width * sizeof(uint16));
    }

    for (int row = 0; row < bottomRows; row++) {
        uint16 *dst = dstBottom->pixels + (row * (int)dstBottom->width) + tileLocalX;
        const uint16 *src = srcCleanBottom + (row * (int)dstBottom->width) + tileLocalX;
        memcpy(dst, src, (size_t)width * sizeof(uint16));
    }
}

static void grRestoreRectFromCleanBg(int x, int y, int width, int height)
{
    int rectEndX;
    int rectEndY;

    if (width <= 0 || height <= 0)
        return;

    if (x < 0) {
        width += x;
        x = 0;
    }
    if (y < 0) {
        height += y;
        y = 0;
    }
    if (x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT)
        return;
    if (x + width > SCREEN_WIDTH)
        width = SCREEN_WIDTH - x;
    if (y + height > SCREEN_HEIGHT)
        height = SCREEN_HEIGHT - y;

    rectEndX = x + width;
    rectEndY = y + height;

    /* Tile pixels are being modified — mark dirty for upload */
    if (x >= 0 && y >= 0 && rectEndX <= 640 && rectEndY <= 480) {
        int tileBaseX = (x >= 320) ? 320 : 0;
        if (rectEndX <= tileBaseX + 320) {
            grMarkSingleColumnDirty(tileBaseX, x, width, y, rectEndY);
            if (width <= 0 || height <= 0)
                return;

            grEnsureCleanBgTiles();
            if (y < 240 && rectEndY > 240) {
                if (tileBaseX == 0) {
                    grRestoreRectSingleColumn(bgTile0Clean, bgTile0,
                                              bgTile3Clean, bgTile3,
                                              tileBaseX, x, y, width, height);
                } else {
                    grRestoreRectSingleColumn(bgTile1Clean, bgTile1,
                                              bgTile4Clean, bgTile4,
                                              tileBaseX, x, y, width, height);
                }
                return;
            }

            {
                PS1Surface *tile;
                const uint16 *clean;
                int tileLocalX = x - tileBaseX;
                int startLocalY;

                if (y < 240) {
                    tile = (tileBaseX == 0) ? bgTile0 : bgTile1;
                    clean = (tileBaseX == 0) ? bgTile0Clean : bgTile1Clean;
                    startLocalY = y;
                } else {
                    tile = (tileBaseX == 0) ? bgTile3 : bgTile4;
                    clean = (tileBaseX == 0) ? bgTile3Clean : bgTile4Clean;
                    startLocalY = y - 240;
                }

                if (tile == NULL || tile->pixels == NULL || clean == NULL)
                    return;

                for (int row = 0; row < height; row++) {
                    int tileLocalY = startLocalY + row;
                    uint16 *dst = tile->pixels + (tileLocalY * (int)tile->width) + tileLocalX;
                    const uint16 *src = clean + (tileLocalY * (int)tile->width) + tileLocalX;
                    memcpy(dst, src, (size_t)width * sizeof(uint16));
                }
            }
            return;
        }
    }

    grMarkRectDirty(x, y, rectEndX, rectEndY);
    if (width <= 0 || height <= 0)
        return;

    grEnsureCleanBgTiles();

    grRestoreTileRect(bgTile0, bgTile0Clean, 0, 0, x, y, width, height);
    grRestoreTileRect(bgTile1, bgTile1Clean, 320, 0, x, y, width, height);
    grRestoreTileRect(bgTile3, bgTile3Clean, 0, 240, x, y, width, height);
    grRestoreTileRect(bgTile4, bgTile4Clean, 320, 240, x, y, width, height);
}

static void grCommitTileRectToClean(PS1Surface *srcTile,
                                    uint16 *dstClean,
                                    int tileScreenX,
                                    int tileScreenY,
                                    int rectX,
                                    int rectY,
                                    int rectW,
                                    int rectH)
{
    int copyStartX;
    int copyStartY;
    int copyEndX;
    int copyEndY;
    int row;

    if (srcTile == NULL || srcTile->pixels == NULL || dstClean == NULL)
        return;
    if (rectW <= 0 || rectH <= 0)
        return;

    copyStartX = (rectX > tileScreenX) ? rectX : tileScreenX;
    copyStartY = (rectY > tileScreenY) ? rectY : tileScreenY;
    copyEndX = rectX + rectW;
    copyEndY = rectY + rectH;

    if (copyEndX > tileScreenX + (int)srcTile->width)
        copyEndX = tileScreenX + (int)srcTile->width;
    if (copyEndY > tileScreenY + (int)srcTile->height)
        copyEndY = tileScreenY + (int)srcTile->height;

    if (copyStartX >= copyEndX || copyStartY >= copyEndY)
        return;

    for (row = copyStartY; row < copyEndY; row++) {
        int tileRow = row - tileScreenY;
        int tileCol = copyStartX - tileScreenX;
        int copyWidth = copyEndX - copyStartX;
        const uint16 *src = srcTile->pixels + (tileRow * srcTile->width) + tileCol;
        uint16 *dst = dstClean + (tileRow * srcTile->width) + tileCol;
        memcpy(dst, src, (size_t)copyWidth * sizeof(uint16));
    }
}

static void grCommitRectToCleanBg(int x, int y, int width, int height)
{
    if (width <= 0 || height <= 0)
        return;

    if (x < 0) {
        width += x;
        x = 0;
    }
    if (y < 0) {
        height += y;
        y = 0;
    }
    if (x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT)
        return;
    if (x + width > SCREEN_WIDTH)
        width = SCREEN_WIDTH - x;
    if (y + height > SCREEN_HEIGHT)
        height = SCREEN_HEIGHT - y;
    if (width <= 0 || height <= 0)
        return;

    grEnsureCleanBgTiles();
    grCommitTileRectToClean(bgTile0, bgTile0Clean, 0, 0, x, y, width, height);
    grCommitTileRectToClean(bgTile1, bgTile1Clean, 320, 0, x, y, width, height);
    grCommitTileRectToClean(bgTile3, bgTile3Clean, 0, 240, x, y, width, height);
    grCommitTileRectToClean(bgTile4, bgTile4Clean, 320, 240, x, y, width, height);
}

/*
 * Clear screen to black
 */
