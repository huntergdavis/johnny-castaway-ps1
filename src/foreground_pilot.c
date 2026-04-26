#include <string.h>
#include <stdio.h>

#include "foreground_pilot.h"

#ifdef PS1_BUILD
#include <stdlib.h>
#include <psxapi.h>

#include "mytypes.h"
#include "events_ps1.h"
#include "graphics_ps1.h"
#include "cdrom_ps1.h"
#include "island.h"
#include "pause_menu.h"
#include "sound_ps1.h"
#include "utils.h"
#include "ps1_perf.h"

uint16 ps1AdsDbgActiveThreads = 0;
uint16 ps1AdsDbgMini = 0;
uint16 ps1AdsDbgRunningThreads = 0;
uint16 ps1AdsDbgTerminatedThreads = 0;
uint16 ps1AdsDbgSceneSlot = 0;
uint16 ps1AdsDbgSceneTag = 0;
uint16 ps1AdsDbgReplayCount = 0;
uint16 ps1AdsDbgReplayTryFrame = 0;
uint16 ps1AdsDbgReplayDrawFrame = 0;
uint16 ps1AdsDbgMergeCarryFrame = 0;
uint16 ps1AdsDbgNoDrawThreadsFrame = 0;
uint16 ps1AdsDbgPlayedThreadsFrame = 0;
uint16 ps1AdsDbgRecordedSpritesFrame = 0;
uint16 ps1AdsDbgLastStopThread = 0;
uint16 ps1AdsDbgLastStopSceneSig = 0;
uint16 ps1AdsDbgLastReapThread = 0;
uint16 ps1AdsDbgLastReapSceneSig = 0;
uint16 ps1AdsDbgLastAddThread = 0;
uint16 ps1AdsDbgLastAddSceneSig = 0;

struct TFgPilotHeader {
    char magic[4];
    uint16 version;
    uint16 frameCount;
    uint16 displayVBlanks;
    uint16 reserved0;
    uint16 screenWidth;
    uint16 screenHeight;
    uint16 unionX;
    uint16 unionY;
    uint16 unionWidth;
    uint16 unionHeight;
    uint32 tableOffset;
    uint32 dataOffset;
    uint32 soundEventsOffset;
    uint16 soundEventCount;
    uint16 reserved1;
};

struct TFgPilotSoundEvent {
    uint16 sourceFrame;
    uint16 sampleId;
};

struct TFgPilotEntry {
    uint16 sourceFrame;
    sint16 x;
    sint16 y;
    uint16 width;
    uint16 height;
    uint16 reserved0;
    uint32 dataOffset;
    uint32 dataSize;
};

struct TFgPilotEntryTable {
    struct TFgPilotEntry *entries;
    uint16 count;
};

struct TFgPilotRuntime {
    uint8 active;
    uint8 mode;
    uint16 frameIndex;
    uint16 frameVBlank;
    uint16 displayVBlanks;
    uint16 holdFrames;
    uint16 presentedVBlanks;
    uint32 sceneClockTick;
    char sceneName[16];
    struct TFgPilotHeader header;
    struct TFgPilotEntryTable entryTable;
    struct TFgPilotEntry currentEntry;
    uint8 frameRendered;
    uint8 *currentFrameData;        /* Points inside frameBuffer (not separately alloc'd). */
    uint8 *frameBuffer;             /* Pre-allocated max-frame-size buffer; one per scene. */
    uint32 frameBufferSize;         /* Capacity of frameBuffer. */
    uint8 *prefetchFrameBuffer;     /* Second max-frame buffer used by stage1 prefetch. */
    uint32 prefetchFrameBufferSize; /* Capacity of prefetchFrameBuffer. */
    uint8 *streamWindowBuffer;      /* Optional larger FG2 window cache. */
    uint32 streamWindowSize;        /* Capacity of streamWindowBuffer. */
    uint32 streamWindowStart;       /* File offset of first byte in streamWindowBuffer. */
    uint32 streamWindowBytes;       /* Valid byte count in streamWindowBuffer. */
    uint8 streamWindowValid;
    uint8 *streamScratch;           /* Pre-allocated sector-aligned scratch for CD reads. */
    uint32 streamScratchSize;       /* Capacity of streamScratch. */
    CdlFILE packCdFile;             /* Resolved CD file handle for the pack (avoids per-frame CdSearchFile). */
    uint8 packCdFileValid;
    uint8 packFormat;
    uint8 stagedFrameValid;
    uint16 stagedFrameIndex;
    struct TFgPilotEntry stagedEntry;
    uint16 palette[256];
    struct TFgPilotSoundEvent *soundEvents;
    uint16 soundEventCount;
    uint16 soundEventCursor;
};

static char gForegroundPilotScene[16] = "";
static unsigned char gForegroundPilotRequestedMode = 0;
static const uint16 kFgPilotProbeHoldFrames = 1800;
static const uint16 kFgPilotHeaderFlagHostTicks = 0x0002;
static const uint16 kFgPilotHeaderFlagHostDeadlines = 0x0004;
static const uint16 kFgPilotHeaderFlagSceneRelative = 0x0008;
static const uint16 kFgPilotHeaderFlagBaseDiff = 0x0010;
static const uint8 kFgPilotPackFormatPal4Spans = 2;
static const uint8 kFgPilotPackFormatIndexed8Spans = 3;
#define FG_PREFETCH_DEFAULT_WINDOW_BYTES (16UL * 1024UL)
/* Below 3 VBlanks, window refills are more likely to become visible delay. */
#define FG_PREFETCH_WINDOW_MIN_SLACK_VBLANKS 3
#define FG_PREFETCH_FALLTHROUGH_MIN_SLACK_VBLANKS 6
static struct TFgPilotRuntime gFgRuntime = {0};
static uint8 gFgConfiguredEver = 0;
static uint8 gFgSetClearedEver = 0;
static uint8 gFgAdsMatchEver = 0;
static uint8 gFgStartAttemptEver = 0;
static uint8 gFgStartedEver = 0;
static uint8 gFgComposedEver = 0;
static uint8 gFgHeapProbeEnabled = 0;
static uint8 gFgPrefetchStage1Enabled = 1;
static uint32 gFgPrefetchWindowBytes = FG_PREFETCH_DEFAULT_WINDOW_BYTES;
static struct TTtmSlot gFgBackdropSlot;
static struct TTtmThread gFgBackdropThread;

enum {
    FG_RUNTIME_NONE = 0,
    FG_RUNTIME_TESTCARD = 1,
    FG_RUNTIME_SCENE_PACK = 2
};

static int fgSceneEquals(const char *a, const char *b);
static uint16 fgReadU16(const uint8 *p);
static sint16 fgReadS16(const uint8 *p);
static void fgBackdropPreloadBackgrndBmp(void);
static void fgBackdropEnableWaveBackdrop(void);
static int fgBackdropSaveCleanBgRectsForPack(sint16 fgX, sint16 fgY, uint16 fgW, uint16 fgH);
static void fgBackdropTickBackgroundWaves(void);
static void fgBackdropStampHoliday(void);
static void fgBackdropRelease(int keepBackgrnd);

static int fgEntryDrawX(const struct TFgPilotHeader *header, const struct TFgPilotEntry *entry)
{
    int x;

    if (entry == NULL)
        return 0;

    x = entry->x;
    if (header != NULL && (header->reserved0 & kFgPilotHeaderFlagSceneRelative) != 0)
        x += islandState.xPos;
    return x;
}

static int fgEntryDrawY(const struct TFgPilotHeader *header, const struct TFgPilotEntry *entry)
{
    int y;

    if (entry == NULL)
        return 0;

    y = entry->y;
    if (header != NULL && (header->reserved0 & kFgPilotHeaderFlagSceneRelative) != 0)
        y += islandState.yPos;
    return y;
}

static uint16 fgConvertHostTicksToVBlanks(uint16 ticks)
{
    uint32 scaled = (uint32)ticks * 6u;
    uint16 hold = (uint16)((scaled + 4u) / 5u);
    return hold > 0 ? hold : 1;
}

static uint16 fgEntryHoldVBlanks(const struct TFgPilotHeader *header,
                                 const struct TFgPilotEntry *entry,
                                 uint16 presentedVBlanks)
{
    uint16 hold = 0;

    if (entry != NULL)
        hold = entry->reserved0;
    if (hold == 0 && header != NULL)
        hold = header->displayVBlanks;
    if (hold == 0)
        hold = 1;

    if (header != NULL && (header->reserved0 & kFgPilotHeaderFlagHostDeadlines) != 0) {
        uint16 targetVBlanks = fgConvertHostTicksToVBlanks(hold);
        hold = (targetVBlanks > presentedVBlanks)
            ? (uint16)(targetVBlanks - presentedVBlanks)
            : 1;
    } else if (header != NULL && (header->reserved0 & kFgPilotHeaderFlagHostTicks) != 0) {
        hold = fgConvertHostTicksToVBlanks(hold);
    }

    return hold;
}

static uint32 fgReadTickCounter(void)
{
    return (uint32)VSync(-1);
}

static uint16 fgElapsedVBlanksSince(uint32 *lastTick)
{
    uint32 nowTick;
    uint32 elapsed;

    if (lastTick == NULL)
        return 0;

    nowTick = fgReadTickCounter();
    elapsed = (nowTick >= *lastTick) ? (nowTick - *lastTick) : 0;
    *lastTick = nowTick;
    return (uint16)(elapsed > 0 ? elapsed : 0);
}

static const char *fgCompactOverlayPackPathForScene(const char *sceneName)
{
    if (fgSceneEquals(sceneName, "fishing1")) {
        return islandState.lowTide ? "FG\\FISH1LOW.FG2" : "FG\\FISHING1.FG2";
    }
    if (fgSceneEquals(sceneName, "fishing2")) {
        return islandState.lowTide ? "FG\\FISH2LOW.FG2" : "FG\\FISHING2.FG2";
    }
    if (fgSceneEquals(sceneName, "fishing3")) {
        return islandState.lowTide ? "FG\\FISH3LOW.FG2" : "FG\\FISHING3.FG2";
    }
    return NULL;
}

static int fgRuntimeUsesBaseDiffBackdrop(void)
{
    return (gFgRuntime.active &&
            (gFgRuntime.header.reserved0 & kFgPilotHeaderFlagBaseDiff) != 0) ? 1 : 0;
}

static uint8 fgSceneModeForName(const char *sceneName)
{
    if (fgSceneEquals(sceneName, "testcard"))
        return FG_RUNTIME_TESTCARD;
    if (fgCompactOverlayPackPathForScene(sceneName) != NULL)
        return FG_RUNTIME_SCENE_PACK;
    return FG_RUNTIME_NONE;
}

static uint16 fgReadU16(const uint8 *p)
{
    return (uint16)((uint16)p[0] | ((uint16)p[1] << 8));
}

static sint16 fgReadS16(const uint8 *p)
{
    return (sint16)fgReadU16(p);
}

static uint32 fgReadU32(const uint8 *p)
{
    return (uint32)p[0] |
           ((uint32)p[1] << 8) |
           ((uint32)p[2] << 16) |
           ((uint32)p[3] << 24);
}

static int fgHeaderIsPal4Spans(const struct TFgPilotHeader *header)
{
    return (header != NULL &&
            memcmp(header->magic, "FGP2", 4) == 0 &&
            header->version == 1) ? 1 : 0;
}

static int fgHeaderIsIndexed8Spans(const struct TFgPilotHeader *header)
{
    return (header != NULL &&
            memcmp(header->magic, "FGP2", 4) == 0 &&
            header->version == 2) ? 1 : 0;
}

static int fgLoadHeader(const char *path, struct TFgPilotHeader *out)
{
    uint8 *data;

    if (!path || !out)
        return 0;

    data = ps1_streamRead(path, 0, 40);
    if (!data)
        return 0;

    memcpy(out->magic, data, 4);
    out->version = fgReadU16(data + 4);
    out->frameCount = fgReadU16(data + 6);
    out->displayVBlanks = fgReadU16(data + 8);
    out->reserved0 = fgReadU16(data + 10);
    out->screenWidth = fgReadU16(data + 12);
    out->screenHeight = fgReadU16(data + 14);
    out->unionX = fgReadU16(data + 16);
    out->unionY = fgReadU16(data + 18);
    out->unionWidth = fgReadU16(data + 20);
    out->unionHeight = fgReadU16(data + 22);
    out->tableOffset = fgReadU32(data + 24);
    out->dataOffset = fgReadU32(data + 28);
    out->soundEventsOffset = fgReadU32(data + 32);
    out->soundEventCount = fgReadU16(data + 36);
    out->reserved1 = fgReadU16(data + 38);
    free(data);

    if (!fgHeaderIsPal4Spans(out) && !fgHeaderIsIndexed8Spans(out))
        return 0;
    if (out->frameCount == 0)
        return 0;

    return 1;
}

static int fgLoadPalette(const char *path, const struct TFgPilotHeader *header,
                         uint16 *outPalette)
{
    uint8 *data;
    uint16 i;
    uint16 entryCount = 0;
    uint32 paletteBytes = 0;

    if (outPalette == NULL)
        return 0;

    for (i = 0; i < 256; i++)
        outPalette[i] = 0;

    if (fgHeaderIsPal4Spans(header)) {
        entryCount = 16;
        paletteBytes = 32;
    } else if (fgHeaderIsIndexed8Spans(header)) {
        entryCount = 256;
        paletteBytes = 512;
    } else {
        return 1;
    }

    data = ps1_streamRead(path, 40, paletteBytes);
    if (!data)
        return 0;

    for (i = 0; i < entryCount; i++)
        outPalette[i] = fgReadU16(data + ((uint32)i * 2u));
    free(data);
    return 1;
}

static void fgFreeEntryTable(struct TFgPilotEntryTable *table)
{
    if (table == NULL)
        return;
    if (table->entries != NULL) {
        free(table->entries);
        table->entries = NULL;
    }
    table->count = 0;
}

static int fgLoadEntryTable(const char *path, const struct TFgPilotHeader *header,
                            struct TFgPilotEntryTable *out)
{
    uint8 *data;
    uint32 tableSize;

    if (!path || !header || !out || header->frameCount == 0)
        return 0;

    memset(out, 0, sizeof(*out));
    tableSize = (uint32)header->frameCount * 20u;
    data = ps1_streamRead(path, header->tableOffset, tableSize);
    if (!data)
        return 0;

    out->entries = (struct TFgPilotEntry *)malloc((size_t)header->frameCount * sizeof(struct TFgPilotEntry));
    if (out->entries == NULL) {
        free(data);
        return 0;
    }

    out->count = header->frameCount;
    for (uint16 i = 0; i < header->frameCount; i++) {
        const uint8 *src = data + ((uint32)i * 20u);
        struct TFgPilotEntry *dst = &out->entries[i];
        dst->sourceFrame = fgReadU16(src + 0);
        dst->x = fgReadS16(src + 2);
        dst->y = fgReadS16(src + 4);
        dst->width = fgReadU16(src + 6);
        dst->height = fgReadU16(src + 8);
        dst->reserved0 = fgReadU16(src + 10);
        dst->dataOffset = fgReadU32(src + 12);
        dst->dataSize = fgReadU32(src + 16);
    }

    free(data);
    return 1;
}

static const struct TFgPilotEntry *fgGetEntryFromTable(const struct TFgPilotEntryTable *table,
                                                       uint16 frameIndex)
{
    if (table == NULL || table->entries == NULL || frameIndex >= table->count)
        return NULL;

    return &table->entries[frameIndex];
}

static int fgSceneEquals(const char *a, const char *b)
{
    return a && b && strcmp(a, b) == 0;
}

static void fgTelemetryUpdate(void)
{
    if (!gFgRuntime.active) {
        ps1AdsDbgActiveThreads = 0;
        ps1AdsDbgReplayCount = 0;
        ps1AdsDbgRunningThreads = 0;
        ps1AdsDbgReplayDrawFrame = 0;
        ps1AdsDbgMergeCarryFrame = 0;
        ps1AdsDbgNoDrawThreadsFrame = 0;
        return;
    }

    ps1AdsDbgActiveThreads = 55;
    ps1AdsDbgReplayCount = (uint16)(gFgRuntime.header.frameCount & 0x3F);
    ps1AdsDbgRunningThreads = (uint16)(gFgRuntime.frameIndex & 0x3F);
    ps1AdsDbgReplayDrawFrame = (uint16)(gFgRuntime.currentEntry.sourceFrame & 0x3F);
    ps1AdsDbgMergeCarryFrame = (uint16)(gFgRuntime.displayVBlanks & 0x3F);
    ps1AdsDbgNoDrawThreadsFrame = (uint16)((gFgRuntime.currentFrameData != NULL) ? 1 : 0);
}

static void fgInitVisiblePipeline(void)
{
    grUpdateDelay = 0;
}

static void fgInitBlackBackground(void)
{
    grInitEmptyBackground();
    grSaveCleanBgTiles();
}

static void fgBlit16ToBackgroundRect(int dstX, int dstY,
                                     uint16 width, uint16 height,
                                     const uint16 *srcPixels)
{
    if (srcPixels == NULL || width == 0 || height == 0)
        return;
    grCompositeDirect16ToBackground(srcPixels, width, height, (sint16)dstX, (sint16)dstY);
}

static void fgPresentCurrentBackground(uint16 holdFrames)
{
    uint16 i;

    for (i = 0; i < holdFrames; i++) {
        grUpdateDisplay(NULL, NULL, NULL);
    }
}

static uint8 *fgLoadRawFileDirect(const char *cdPath, uint32 *outSize)
{
    CdlFILE fileInfo;
    uint32 totalBytes;
    uint8 *buffer;
    int totalSectors;

    if (cdPath == NULL || outSize == NULL)
        return NULL;

    if (CdSearchFile(&fileInfo, (char *)cdPath) == NULL) {
        printf("FG pilot: CdSearchFile failed for %s\n", cdPath);
        return NULL;
    }

    totalBytes = (uint32)fileInfo.size;
    totalSectors = (int)((totalBytes + 2047u) / 2048u);
    buffer = (uint8 *)malloc((size_t)totalSectors * 2048u);
    if (buffer == NULL)
        return NULL;

    CdControl(CdlSetloc, (uint8 *)&fileInfo.pos, 0);
    CdRead(totalSectors, (uint32 *)buffer, CdlModeSpeed);
    if (CdReadSync(0, 0) < 0) {
        printf("FG pilot: CdReadSync failed for %s\n", cdPath);
        free(buffer);
        return NULL;
    }

    *outSize = totalBytes;
    cdromResetState();
    return buffer;
}

static void fgShowRawFrame(const char *cdPath, uint16 holdFrames)
{
    uint32 rawSize = 0;
    uint8 *screenBuffer;

    if (cdPath == NULL)
        return;

    screenBuffer = fgLoadRawFileDirect(cdPath, &rawSize);
    if (screenBuffer == NULL)
        return;
    if (rawSize < (uint32)(640 * 480 * 2)) {
        printf("FG pilot: short raw frame %s (%u bytes)\n", cdPath, (unsigned int)rawSize);
        free(screenBuffer);
        return;
    }

    fgInitVisiblePipeline();
    fgInitBlackBackground();
    grBeginFrame();
    grRestoreBgTiles();
    fgBlit16ToBackgroundRect(0, 0, 640, 480, (const uint16 *)screenBuffer);
    free(screenBuffer);
    fgPresentCurrentBackground(holdFrames);
}

static void fgPlayTestCard(void)
{
    static uint16 *colors[4] = { NULL, NULL, NULL, NULL };
    static const uint16 colorValues[4] = { 0x001f, 0x03e0, 0x03ff, 0x7c1f };
    const uint16 rectW = 120;
    const uint16 rectH = 80;
    uint16 i;

    fgInitVisiblePipeline();
    fgInitBlackBackground();

    for (int c = 0; c < 4; c++) {
        if (colors[c] == NULL) {
            colors[c] = (uint16 *)malloc((size_t)rectW * (size_t)rectH * sizeof(uint16));
            if (colors[c] == NULL)
                return;
            for (uint32 j = 0; j < (uint32)rectW * (uint32)rectH; j++)
                colors[c][j] = colorValues[c];
        }
    }

    for (i = 0; i < 120; i++) {
        grBeginFrame();
        grRestoreBgTiles();
        fgBlit16ToBackgroundRect(24, 24, rectW, rectH, colors[0]);
        fgBlit16ToBackgroundRect(176, 24, rectW, rectH, colors[1]);
        fgBlit16ToBackgroundRect(24, 136, rectW, rectH, colors[2]);
        fgBlit16ToBackgroundRect(176, 136, rectW, rectH, colors[3]);
        fgPresentCurrentBackground(1);
    }
}

/* Scene-local streaming buffers: allocated once per scene and reused for every
 * frame in that scene. They are intentionally released at scene boundaries so
 * the next backdrop load starts with maximum contiguous heap. */
static uint8 *gFgFrameBuffer = NULL;
static uint32 gFgFrameBufferSize = 0;
static uint8 *gFgPrefetchFrameBuffer = NULL;
static uint32 gFgPrefetchFrameBufferSize = 0;
static uint8 *gFgStreamWindowBuffer = NULL;
static uint32 gFgStreamWindowBufferSize = 0;
static uint8 *gFgStreamScratch = NULL;
static uint32 gFgStreamScratchSize = 0;

static void fgReleaseStreamBuffers(void)
{
    if (gFgFrameBuffer != NULL) {
        free(gFgFrameBuffer);
        gFgFrameBuffer = NULL;
    }
    gFgFrameBufferSize = 0;
    if (gFgPrefetchFrameBuffer != NULL) {
        free(gFgPrefetchFrameBuffer);
        gFgPrefetchFrameBuffer = NULL;
    }
    gFgPrefetchFrameBufferSize = 0;
    if (gFgStreamWindowBuffer != NULL) {
        free(gFgStreamWindowBuffer);
        gFgStreamWindowBuffer = NULL;
    }
    gFgStreamWindowBufferSize = 0;
    if (gFgStreamScratch != NULL) {
        free(gFgStreamScratch);
        gFgStreamScratch = NULL;
    }
    gFgStreamScratchSize = 0;
}

unsigned long fgProbeLargestAlloc(void)
{
    unsigned long lo = 0;
    unsigned long hi = 512ul * 1024ul;

    while ((hi - lo) > 1024ul) {
        unsigned long mid = ((lo + hi + 1023ul) / 2048ul) * 1024ul;
        void *p;
        if (mid <= lo)
            mid = lo + 1024ul;
        p = malloc((size_t)mid);
        if (p != NULL) {
            free(p);
            lo = mid;
        } else {
            hi = mid - 1024ul;
        }
    }

    return lo;
}

static void fgHeapProbe(const char *phase, const char *sceneName)
{
    unsigned long largest;

    if (!gFgHeapProbeEnabled)
        return;

    largest = fgProbeLargestAlloc();
    printf("FGHEAP phase=%s scene=%s largest=%lu fg=%lu prefetch=%lu window=%lu scratch=%lu rects=%d rect_bytes=%lu\n",
           phase != NULL ? phase : "?",
           sceneName != NULL ? sceneName : "?",
           largest,
           (unsigned long)gFgFrameBufferSize,
           (unsigned long)gFgPrefetchFrameBufferSize,
           (unsigned long)gFgStreamWindowBufferSize,
           (unsigned long)gFgStreamScratchSize,
           grCleanBgRectsCount(),
           grCleanBgRectsBytes());
}

static void fgRuntimeReset(void)
{
    /* currentFrameData now points inside gFgFrameBuffer — don't free it
     * separately. The persistent buffers (gFgFrameBuffer / gFgStreamScratch)
     * survive the reset on purpose; only per-scene state is cleared. */
    gFgRuntime.currentFrameData = NULL;
    gFgRuntime.frameBuffer = NULL;
    gFgRuntime.frameBufferSize = 0;
    gFgRuntime.prefetchFrameBuffer = NULL;
    gFgRuntime.prefetchFrameBufferSize = 0;
    gFgRuntime.streamWindowBuffer = NULL;
    gFgRuntime.streamWindowSize = 0;
    gFgRuntime.streamWindowStart = 0;
    gFgRuntime.streamWindowBytes = 0;
    gFgRuntime.streamWindowValid = 0;
    gFgRuntime.streamScratch = NULL;
    gFgRuntime.streamScratchSize = 0;
    gFgRuntime.packCdFileValid = 0;
    fgFreeEntryTable(&gFgRuntime.entryTable);
    if (gFgRuntime.soundEvents != NULL) {
        free(gFgRuntime.soundEvents);
        gFgRuntime.soundEvents = NULL;
    }
    memset(&gFgRuntime, 0, sizeof(gFgRuntime));
    fgTelemetryUpdate();
}

static int fgLoadSoundEvents(const char *path, const struct TFgPilotHeader *header,
                             struct TFgPilotSoundEvent **outEvents, uint16 *outCount)
{
    uint8 *data;
    uint32 byteCount;
    uint16 i;

    if (!outEvents || !outCount)
        return 0;

    *outEvents = NULL;
    *outCount = 0;

    if (!path || !header)
        return 1;
    if (header->soundEventCount == 0 || header->soundEventsOffset == 0)
        return 1;

    byteCount = (uint32)header->soundEventCount * 4u;
    data = ps1_streamRead(path, header->soundEventsOffset, byteCount);
    if (!data)
        return 0;

    *outEvents = (struct TFgPilotSoundEvent *)malloc(
        (size_t)header->soundEventCount * sizeof(struct TFgPilotSoundEvent));
    if (*outEvents == NULL) {
        free(data);
        return 0;
    }

    for (i = 0; i < header->soundEventCount; i++) {
        (*outEvents)[i].sourceFrame = fgReadU16(data + ((uint32)i * 4u));
        (*outEvents)[i].sampleId    = fgReadU16(data + ((uint32)i * 4u) + 2u);
    }
    *outCount = header->soundEventCount;
    free(data);
    return 1;
}

/* Sound events fire at frame-load time, but the loaded frame is not
 * composited and flipped to the display until the next vblank or two. To
 * keep the sample's key-on aligned with the frame the user actually sees,
 * delay event firing by this many source-frames. Tuned by ear on
 * fishing1 — audio landed ~2 frames ahead of the visible trigger. */
#define FG_SOUND_EVENT_DELAY_FRAMES 3

static void fgFireSoundEventsUpTo(uint16 sourceFrame)
{
    uint16 threshold;

    if (sourceFrame < FG_SOUND_EVENT_DELAY_FRAMES)
        return;
    threshold = (uint16)(sourceFrame - FG_SOUND_EVENT_DELAY_FRAMES);

    while (gFgRuntime.soundEventCursor < gFgRuntime.soundEventCount) {
        const struct TFgPilotSoundEvent *ev =
            &gFgRuntime.soundEvents[gFgRuntime.soundEventCursor];
        if (ev->sourceFrame > threshold)
            break;
        soundPlay((int)ev->sampleId);
        if (ps1PerfEnabled)
            ps1PerfMarkSoundEvent();
        gFgRuntime.soundEventCursor++;
    }
}

static void fgBackdropRelease(int keepBackgrnd)
{
    int slot;
    int firstSlot = keepBackgrnd ? 1 : 0;

    gFgBackdropThread.isRunning = 0;
    if (!keepBackgrnd)
        islandClearWaveCache();

    for (slot = firstSlot; slot < MAX_BMP_SLOTS; slot++) {
        if (gFgBackdropSlot.numSprites[slot])
            grReleaseBmp(&gFgBackdropSlot, (uint8)slot);
    }

    if (!keepBackgrnd)
        memset(&gFgBackdropSlot, 0, sizeof(gFgBackdropSlot));
}

static void fgBackdropPreloadBackgrndBmp(void)
{
    if (gFgBackdropSlot.numSprites[0] > 0 &&
        gFgBackdropSlot.loadedBmpNames[0] != NULL &&
        strcmp(gFgBackdropSlot.loadedBmpNames[0], "BACKGRND.BMP") == 0) {
        fgBackdropRelease(1);
        return;
    }

    fgBackdropRelease(0);
    grLoadBmp(&gFgBackdropSlot, 0, "BACKGRND.BMP");
}

static void fgBackdropEnableWaveBackdrop(void)
{
    gFgBackdropThread.ttmSlot   = &gFgBackdropSlot;
    gFgBackdropThread.ttmLayer  = grBackgroundSfc;
    gFgBackdropThread.isRunning = 3;
    gFgBackdropThread.delay     = 2;
    gFgBackdropThread.timer     = 0;

    if (gFgBackdropSlot.numSprites[0] == 0) {
        gFgBackdropThread.isRunning = 0;
        grSaveCleanBgTiles();
        return;
    }

    grDx = islandState.xPos;
    grDy = islandState.yPos;

    if (islandState.raft >= 1 && islandState.raft <= 5) {
        int xRaft = islandState.lowTide ? 529 : 512;
        int yRaft = islandState.lowTide ? 281 : 266;
        grLoadBmp(&gFgBackdropSlot, 1, "MRAFT.BMP");
        grDrawSprite(grBackgroundSfc, &gFgBackdropSlot,
                     xRaft, yRaft, (uint16)(islandState.raft - 1), 1);
        grReleaseBmp(&gFgBackdropSlot, 1);
    }

    grDrawSprite(grBackgroundSfc, &gFgBackdropSlot, 288, 279,  0, 0);
    grDrawSprite(grBackgroundSfc, &gFgBackdropSlot, 442, 148, 13, 0);
    grDrawSprite(grBackgroundSfc, &gFgBackdropSlot, 365, 122, 12, 0);
    grDrawSprite(grBackgroundSfc, &gFgBackdropSlot, 396, 279, 14, 0);
    if (islandState.lowTide) {
        grDrawSprite(grBackgroundSfc, &gFgBackdropSlot, 249, 303,  1, 0);
        grDrawSprite(grBackgroundSfc, &gFgBackdropSlot, 150, 328,  2, 0);
    }

    if (islandState.holiday >= 1 && islandState.holiday <= 4)
        grLoadBmp(&gFgBackdropSlot, 2, "HOLIDAY.BMP");

    for (int i = 0; i < 4; i++)
        islandAnimate(&gFgBackdropThread);
}

static int fgBackdropSaveCleanBgRectsForPack(sint16 fgX, sint16 fgY, uint16 fgW, uint16 fgH)
{
    const sint16 kWaveMinX = 129;
    const sint16 kWaveMinY = 303;
    const sint16 kWaveEndX = 608;
    const sint16 kWaveEndY = 356;
    const sint16 kUpperSplitY = 190;

    sint16 fgEndX = (sint16)(fgX + fgW);
    sint16 fgEndY = (sint16)(fgY + fgH);

    if (fgW == 0 || fgH == 0) {
        sint16 xs[1]; sint16 ys[1]; uint16 ws[1]; uint16 hs[1];
        xs[0] = kWaveMinX;
        ys[0] = kWaveMinY;
        ws[0] = (uint16)(kWaveEndX - kWaveMinX);
        hs[0] = (uint16)(kWaveEndY - kWaveMinY);
        return grSaveCleanBgRects(xs, ys, ws, hs, 1) == 1;
    }

    sint16 lowerMinX = fgX;
    sint16 lowerMinY = fgY >= kUpperSplitY ? fgY : kUpperSplitY;
    sint16 lowerEndX = fgEndX;
    sint16 lowerEndY = fgEndY;

    if (kWaveMinX < lowerMinX) lowerMinX = kWaveMinX;
    if (kWaveMinY < lowerMinY) lowerMinY = kWaveMinY;
    if (kWaveEndX > lowerEndX) lowerEndX = kWaveEndX;
    if (kWaveEndY > lowerEndY) lowerEndY = kWaveEndY;

    if (lowerMinX < 0) lowerMinX = 0;
    if (lowerMinY < 0) lowerMinY = 0;
    if (lowerEndX > 640) lowerEndX = 640;
    if (lowerEndY > 480) lowerEndY = 480;

    if (fgY < kUpperSplitY) {
        sint16 upperMinX = fgX;
        sint16 upperMinY = fgY;
        sint16 upperEndX = fgEndX;
        sint16 upperEndY = kUpperSplitY;
        sint16 xs[2]; sint16 ys[2]; uint16 ws[2]; uint16 hs[2];

        if (upperMinX < 0) upperMinX = 0;
        if (upperMinY < 0) upperMinY = 0;
        if (upperEndX > 640) upperEndX = 640;
        if (upperEndY > 480) upperEndY = 480;

        if (upperEndX <= upperMinX || upperEndY <= upperMinY ||
            lowerEndX <= lowerMinX || lowerEndY <= lowerMinY)
            return 0;

        xs[0] = lowerMinX;
        ys[0] = lowerMinY;
        ws[0] = (uint16)(lowerEndX - lowerMinX);
        hs[0] = (uint16)(lowerEndY - lowerMinY);
        xs[1] = upperMinX;
        ys[1] = upperMinY;
        ws[1] = (uint16)(upperEndX - upperMinX);
        hs[1] = (uint16)(upperEndY - upperMinY);
        return grSaveCleanBgRects(xs, ys, ws, hs, 2) == 2;
    } else {
        sint16 xs[1]; sint16 ys[1]; uint16 ws[1]; uint16 hs[1];
        if (lowerEndX <= lowerMinX || lowerEndY <= lowerMinY)
            return 0;
        xs[0] = lowerMinX;
        ys[0] = lowerMinY;
        ws[0] = (uint16)(lowerEndX - lowerMinX);
        hs[0] = (uint16)(lowerEndY - lowerMinY);
        return grSaveCleanBgRects(xs, ys, ws, hs, 1) == 1;
    }
}

static void fgBackdropTickBackgroundWaves(void)
{
    if (!gFgBackdropThread.isRunning)
        return;

    if (gFgBackdropThread.timer == 0) {
        gFgBackdropThread.timer = gFgBackdropThread.delay;
        islandAnimate(&gFgBackdropThread);
    } else {
        islandRedrawWave(&gFgBackdropThread);
        gFgBackdropThread.timer--;
    }
}

static void fgBackdropStampHoliday(void)
{
    int hx = 0;
    int hy = 0;
    uint16 hSpriteNo = 0;

    if (islandState.holiday < 1 || islandState.holiday > 4)
        return;
    if (gFgBackdropSlot.numSprites[2] == 0)
        return;

    switch (islandState.holiday) {
        case 1: hx = 410; hy = 298; hSpriteNo = 0; break;
        case 2: hx = 333; hy = 286; hSpriteNo = 1; break;
        case 3: hx = 404; hy = 267; hSpriteNo = 2; break;
        case 4: hx = 361; hy = 155; hSpriteNo = 3; break;
    }

    grDx = islandState.xPos;
    grDy = islandState.yPos;
    grDrawSprite(grBackgroundSfc, &gFgBackdropSlot, hx, hy, hSpriteNo, 2);
}

static uint16 fgRuntimeHeldSlackBeforeWait(void)
{
    if (!gFgRuntime.active || gFgRuntime.displayVBlanks == 0)
        return 0;

    if (gFgRuntime.displayVBlanks <= gFgRuntime.frameVBlank)
        return 0;
    return (uint16)(gFgRuntime.displayVBlanks - gFgRuntime.frameVBlank);
}

static int fgRuntimeCanStageNextFrame(void)
{
    return gFgPrefetchStage1Enabled &&
           gFgRuntime.active &&
           gFgRuntime.mode == FG_RUNTIME_SCENE_PACK &&
           fgRuntimeUsesBaseDiffBackdrop() &&
           gFgRuntime.packCdFileValid &&
           gFgRuntime.streamScratch != NULL &&
           gFgRuntime.prefetchFrameBuffer != NULL;
}

static int fgRuntimeCanWindowCache(void)
{
    return gFgPrefetchWindowBytes > 0 &&
           gFgRuntime.active &&
           gFgRuntime.mode == FG_RUNTIME_SCENE_PACK &&
           fgRuntimeUsesBaseDiffBackdrop() &&
           gFgRuntime.packCdFileValid &&
           gFgRuntime.streamWindowBuffer != NULL &&
           gFgRuntime.streamWindowSize > 0;
}

static int fgEntryHasPayload(const struct TFgPilotEntry *entry)
{
    return (entry != NULL &&
            entry->dataSize > 0 &&
            entry->width > 0 &&
            entry->height > 0) ? 1 : 0;
}

static uint32 fgSectorAlignDown(uint32 offset)
{
    return offset & ~2047UL;
}

static int fgRuntimeEntryFitsWindow(const struct TFgPilotEntry *entry)
{
    uint32 windowStart;
    uint32 offsetInWindow;

    if (!fgRuntimeCanWindowCache() || !fgEntryHasPayload(entry))
        return 0;

    windowStart = fgSectorAlignDown(entry->dataOffset);
    offsetInWindow = entry->dataOffset - windowStart;
    return (offsetInWindow + entry->dataSize <= gFgRuntime.streamWindowSize) ? 1 : 0;
}

static int fgRuntimeWindowContainsEntry(const struct TFgPilotEntry *entry)
{
    uint32 entryEnd;
    uint32 windowEnd;

    if (!fgRuntimeCanWindowCache() ||
        !gFgRuntime.streamWindowValid ||
        !fgEntryHasPayload(entry))
        return 0;

    entryEnd = entry->dataOffset + entry->dataSize;
    windowEnd = gFgRuntime.streamWindowStart + gFgRuntime.streamWindowBytes;
    return (entry->dataOffset >= gFgRuntime.streamWindowStart &&
            entryEnd <= windowEnd) ? 1 : 0;
}

static int fgRuntimeCopyEntryFromWindow(const struct TFgPilotEntry *entry,
                                        uint8 *dst,
                                        uint8 countsAsDueHit)
{
    uint32 offsetInWindow;

    if (dst == NULL || !fgRuntimeWindowContainsEntry(entry))
        return 0;

    offsetInWindow = entry->dataOffset - gFgRuntime.streamWindowStart;
    memcpy(dst, gFgRuntime.streamWindowBuffer + offsetInWindow, entry->dataSize);
    if (ps1PerfEnabled)
        ps1PerfMarkPrefetchWindowHit(countsAsDueHit);
    return 1;
}

static int fgRuntimeTryExtendWindow(uint32 windowStart,
                                    uint32 readBytes,
                                    uint16 slackVBlanks,
                                    uint8 countAsPrefetch,
                                    uint16 *outElapsedVBlanks)
{
    uint32 currentStart;
    uint32 currentEnd;
    uint32 targetEnd;
    uint32 preserveOffset;
    uint32 preserveBytes;
    uint32 appendBytes;
    uint32 stageTick;
    uint16 elapsedVBlanks;
    int ok;

    if (!gFgRuntime.streamWindowValid)
        return 0;

    currentStart = gFgRuntime.streamWindowStart;
    currentEnd = currentStart + gFgRuntime.streamWindowBytes;
    targetEnd = windowStart + readBytes;

    if (windowStart < currentStart ||
        windowStart >= currentEnd ||
        targetEnd <= currentEnd)
        return 0;

    /* Append reads write whole sectors, so only extend fully aligned windows. */
    if ((currentEnd & 2047UL) != 0 || (targetEnd & 2047UL) != 0)
        return 0;

    preserveOffset = windowStart - currentStart;
    preserveBytes = currentEnd - windowStart;
    appendBytes = targetEnd - currentEnd;
    if (preserveBytes == 0 ||
        appendBytes == 0 ||
        preserveBytes >= gFgRuntime.streamWindowSize ||
        preserveBytes + appendBytes > gFgRuntime.streamWindowSize)
        return 0;

    stageTick = fgReadTickCounter();
    if (countAsPrefetch && ps1PerfEnabled)
        ps1PerfBeginPrefetchRead(slackVBlanks);

    if (preserveOffset > 0)
        memmove(gFgRuntime.streamWindowBuffer,
                gFgRuntime.streamWindowBuffer + preserveOffset,
                preserveBytes);

    ok = ps1_streamReadAlignedIntoFile(&gFgRuntime.packCdFile,
                                       currentEnd,
                                       appendBytes,
                                       gFgRuntime.streamWindowBuffer + preserveBytes);
    elapsedVBlanks = (uint16)ps1PerfElapsedVBlanks(stageTick);
    if (countAsPrefetch && ps1PerfEnabled)
        ps1PerfEndPrefetchRead(elapsedVBlanks, appendBytes, ok);
    if (outElapsedVBlanks != NULL)
        *outElapsedVBlanks = elapsedVBlanks;

    if (!ok) {
        gFgRuntime.streamWindowValid = 0;
        return -1;
    }

    gFgRuntime.streamWindowStart = windowStart;
    gFgRuntime.streamWindowBytes = readBytes;
    gFgRuntime.streamWindowValid = 1;
    return 1;
}

static int fgRuntimeFillWindowForEntry(const struct TFgPilotEntry *entry,
                                       uint16 slackVBlanks,
                                       uint8 countAsPrefetch,
                                       uint16 *outElapsedVBlanks)
{
    uint32 windowStart;
    uint32 readBytes;
    uint32 readEnd;
    uint32 stageTick;
    uint16 elapsedVBlanks;
    int ok;

    if (outElapsedVBlanks != NULL)
        *outElapsedVBlanks = 0;

    if (!fgRuntimeEntryFitsWindow(entry))
        return 0;

    windowStart = fgSectorAlignDown(entry->dataOffset);
    readBytes = gFgRuntime.streamWindowSize;
    if (windowStart >= (uint32)gFgRuntime.packCdFile.size)
        return 0;
    readEnd = windowStart + readBytes;
    if (readEnd > (uint32)gFgRuntime.packCdFile.size)
        readBytes = (uint32)gFgRuntime.packCdFile.size - windowStart;
    if (readBytes == 0)
        return 0;

    ok = fgRuntimeTryExtendWindow(windowStart,
                                  readBytes,
                                  slackVBlanks,
                                  countAsPrefetch,
                                  outElapsedVBlanks);
    if (ok != 0)
        return (ok > 0) ? 1 : 0;

    stageTick = fgReadTickCounter();
    if (countAsPrefetch && ps1PerfEnabled)
        ps1PerfBeginPrefetchRead(slackVBlanks);
    ok = ps1_streamReadAlignedIntoFile(&gFgRuntime.packCdFile,
                                       windowStart,
                                       readBytes,
                                       gFgRuntime.streamWindowBuffer);
    elapsedVBlanks = (uint16)ps1PerfElapsedVBlanks(stageTick);
    if (countAsPrefetch && ps1PerfEnabled)
        ps1PerfEndPrefetchRead(elapsedVBlanks, readBytes, ok);
    if (outElapsedVBlanks != NULL)
        *outElapsedVBlanks = elapsedVBlanks;

    if (!ok) {
        gFgRuntime.streamWindowValid = 0;
        return 0;
    }

    gFgRuntime.streamWindowStart = windowStart;
    gFgRuntime.streamWindowBytes = readBytes;
    gFgRuntime.streamWindowValid = 1;
    return 1;
}

static void fgRuntimeSetStagedFrame(uint16 frameIndex,
                                    const struct TFgPilotEntry *entry)
{
    gFgRuntime.stagedEntry = *entry;
    gFgRuntime.stagedFrameIndex = frameIndex;
    gFgRuntime.stagedFrameValid = 1;
}

static const struct TFgPilotEntry *fgRuntimeNextPayloadEntry(uint16 *outFrameIndex)
{
    uint16 frameIndex;

    if (!gFgRuntime.active)
        return NULL;

    frameIndex = gFgRuntime.stagedFrameValid
        ? (uint16)(gFgRuntime.stagedFrameIndex + 1)
        : (uint16)(gFgRuntime.frameIndex + 1);

    while (frameIndex < gFgRuntime.header.frameCount) {
        const struct TFgPilotEntry *entry =
            fgGetEntryFromTable(&gFgRuntime.entryTable, frameIndex);
        if (fgEntryHasPayload(entry)) {
            if (outFrameIndex != NULL)
                *outFrameIndex = frameIndex;
            return entry;
        }
        frameIndex++;
    }

    return NULL;
}

static int fgRuntimeConsumeStagedFrame(uint16 frameIndex)
{
    uint8 *freeBuffer;
    uint32 freeBufferSize;

    if (!gFgRuntime.stagedFrameValid)
        return 0;

    if (gFgRuntime.stagedFrameIndex != frameIndex) {
        if (ps1PerfEnabled)
            ps1PerfMarkTripwire();
        return -1;
    }

    freeBuffer = gFgRuntime.frameBuffer;
    freeBufferSize = gFgRuntime.frameBufferSize;
    gFgRuntime.frameBuffer = gFgRuntime.prefetchFrameBuffer;
    gFgRuntime.frameBufferSize = gFgRuntime.prefetchFrameBufferSize;
    gFgRuntime.prefetchFrameBuffer = freeBuffer;
    gFgRuntime.prefetchFrameBufferSize = freeBufferSize;

    gFgRuntime.currentEntry = gFgRuntime.stagedEntry;
    gFgRuntime.currentFrameData = gFgRuntime.frameBuffer;
    gFgRuntime.frameRendered = 0;
    gFgRuntime.stagedFrameValid = 0;

    gFgRuntime.displayVBlanks = fgEntryHoldVBlanks(&gFgRuntime.header,
                                                   &gFgRuntime.currentEntry,
                                                   gFgRuntime.presentedVBlanks);
    if (ps1PerfEnabled) {
        ps1PerfMarkPrefetchHit();
        ps1PerfMarkEntry(gFgRuntime.currentEntry.dataSize,
                         gFgRuntime.displayVBlanks,
                         0,
                         gFgRuntime.currentEntry.sourceFrame,
                         gFgRuntime.currentEntry.dataOffset);
    }
    fgFireSoundEventsUpTo(gFgRuntime.currentEntry.sourceFrame);
    fgTelemetryUpdate();
    return 1;
}

static int fgRuntimeWindowSlackEligible(uint16 slackVBlanks)
{
    return (slackVBlanks >= FG_PREFETCH_WINDOW_MIN_SLACK_VBLANKS) ? 1 : 0;
}

static int fgRuntimePrimeNextFrameForSetup(void)
{
    uint16 nextFrameIndex;
    const struct TFgPilotEntry *entry;

    if (!fgRuntimeCanStageNextFrame() || gFgRuntime.stagedFrameValid)
        return 0;

    if (gFgRuntime.frameIndex + 1 >= gFgRuntime.header.frameCount)
        return 0;
    nextFrameIndex = (uint16)(gFgRuntime.frameIndex + 1);
    entry = fgGetEntryFromTable(&gFgRuntime.entryTable, nextFrameIndex);
    if (entry == NULL)
        return -1;

    if (!fgEntryHasPayload(entry))
        return 0;

    if (entry->dataSize > gFgRuntime.prefetchFrameBufferSize)
        return -1;

    if (fgRuntimeEntryFitsWindow(entry)) {
        if (!fgRuntimeWindowContainsEntry(entry) &&
            !fgRuntimeFillWindowForEntry(entry, 0, 0, NULL)) {
            return -1;
        }
        if (!fgRuntimeCopyEntryFromWindow(entry, gFgRuntime.prefetchFrameBuffer, 0))
            return -1;
        fgRuntimeSetStagedFrame(nextFrameIndex, entry);
        return 1;
    }

    if (!ps1_streamReadIntoFileBuffered(&gFgRuntime.packCdFile,
                                        entry->dataOffset,
                                        entry->dataSize,
                                        gFgRuntime.prefetchFrameBuffer,
                                        gFgRuntime.streamScratch,
                                        gFgRuntime.streamScratchSize)) {
        return -1;
    }

    fgRuntimeSetStagedFrame(nextFrameIndex, entry);
    return 1;
}

static int fgRuntimeTryStageNextFrame(uint16 *outElapsedVBlanks)
{
    uint16 nextFrameIndex;
    uint16 slackVBlanks;
    const struct TFgPilotEntry *entry;
    int ok;
    uint32 stageTick = 0;
    uint16 elapsedVBlanks = 0;

    if (outElapsedVBlanks != NULL)
        *outElapsedVBlanks = 0;

    if (!fgRuntimeCanStageNextFrame())
        return 0;

    if (gFgRuntime.stagedFrameValid) {
        if (ps1PerfEnabled)
            ps1PerfMarkPrefetchDuplicate();
        return 0;
    }

    if (gFgRuntime.frameIndex + 1 >= gFgRuntime.header.frameCount)
        return 0;
    nextFrameIndex = (uint16)(gFgRuntime.frameIndex + 1);
    entry = fgGetEntryFromTable(&gFgRuntime.entryTable, nextFrameIndex);
    if (entry == NULL) {
        if (ps1PerfEnabled)
            ps1PerfMarkTripwire();
        gFgRuntime.active = 0;
        return 1;
    }

    if (!fgEntryHasPayload(entry))
        return 0;

    slackVBlanks = fgRuntimeHeldSlackBeforeWait();
    if (slackVBlanks == 0) {
        if (ps1PerfEnabled)
            ps1PerfMarkPrefetchAttempt(slackVBlanks, slackVBlanks, 0);
        if (ps1PerfEnabled)
            ps1PerfMarkPrefetchSkipNoSlack();
        return 0;
    }

    if (entry->dataSize > gFgRuntime.prefetchFrameBufferSize) {
        if (ps1PerfEnabled)
            ps1PerfMarkTripwire();
        gFgRuntime.active = 0;
        return 1;
    }

    if (fgRuntimeEntryFitsWindow(entry)) {
        if (!fgRuntimeWindowContainsEntry(entry)) {
            int eligible = fgRuntimeWindowSlackEligible(slackVBlanks);
            if (ps1PerfEnabled)
                ps1PerfMarkPrefetchAttempt(slackVBlanks, slackVBlanks, eligible);
            if (!eligible) {
                if (ps1PerfEnabled)
                    ps1PerfMarkPrefetchSkipNoSlack();
                return 0;
            }
            if (!fgRuntimeFillWindowForEntry(entry, slackVBlanks, 1, &elapsedVBlanks)) {
                if (ps1PerfEnabled)
                    ps1PerfMarkTripwire();
                gFgRuntime.active = 0;
                return 1;
            }
            if (outElapsedVBlanks != NULL)
                *outElapsedVBlanks = elapsedVBlanks;
        } else if (ps1PerfEnabled) {
            ps1PerfMarkPrefetchAttempt(slackVBlanks, slackVBlanks, 1);
        }
        if (!fgRuntimeCopyEntryFromWindow(entry, gFgRuntime.prefetchFrameBuffer, 0)) {
            if (ps1PerfEnabled)
                ps1PerfMarkTripwire();
            gFgRuntime.active = 0;
            return 1;
        }
        fgRuntimeSetStagedFrame(nextFrameIndex, entry);
        return 1;
    }

    if (ps1PerfEnabled)
        ps1PerfMarkPrefetchAttempt(slackVBlanks, slackVBlanks, 1);
    stageTick = fgReadTickCounter();
    if (ps1PerfEnabled) {
        ps1PerfBeginPrefetchRead(slackVBlanks);
    }
    ok = ps1_streamReadIntoFileBuffered(&gFgRuntime.packCdFile,
                                        entry->dataOffset,
                                        entry->dataSize,
                                        gFgRuntime.prefetchFrameBuffer,
                                        gFgRuntime.streamScratch,
                                        gFgRuntime.streamScratchSize);
    elapsedVBlanks = (uint16)ps1PerfElapsedVBlanks(stageTick);
    if (ps1PerfEnabled) {
        ps1PerfEndPrefetchRead(elapsedVBlanks, entry->dataSize, ok);
    }
    if (outElapsedVBlanks != NULL)
        *outElapsedVBlanks = elapsedVBlanks;

    if (!ok) {
        if (ps1PerfEnabled)
            ps1PerfMarkTripwire();
        gFgRuntime.active = 0;
        return 1;
    }

    fgRuntimeSetStagedFrame(nextFrameIndex, entry);
    return 1;
}

static int fgRuntimeTryPrefetchWindow(uint16 *outElapsedVBlanks)
{
    uint16 targetFrameIndex = 0;
    uint16 slackVBlanks;
    const struct TFgPilotEntry *entry;

    if (outElapsedVBlanks != NULL)
        *outElapsedVBlanks = 0;

    if (!fgRuntimeCanWindowCache())
        return 0;

    entry = fgRuntimeNextPayloadEntry(&targetFrameIndex);
    (void)targetFrameIndex;
    if (entry == NULL || !fgRuntimeEntryFitsWindow(entry))
        return 0;

    if (fgRuntimeWindowContainsEntry(entry)) {
        if (ps1PerfEnabled)
            ps1PerfMarkPrefetchDuplicate();
        return 0;
    }

    slackVBlanks = fgRuntimeHeldSlackBeforeWait();
    if (slackVBlanks == 0) {
        if (ps1PerfEnabled)
            ps1PerfMarkPrefetchAttempt(slackVBlanks, slackVBlanks, 0);
        if (ps1PerfEnabled)
            ps1PerfMarkPrefetchSkipNoSlack();
        return 0;
    }

    if (!fgRuntimeWindowSlackEligible(slackVBlanks)) {
        if (ps1PerfEnabled)
            ps1PerfMarkPrefetchAttempt(slackVBlanks, slackVBlanks, 0);
        if (ps1PerfEnabled)
            ps1PerfMarkPrefetchSkipNoSlack();
        return 0;
    }

    if (ps1PerfEnabled)
        ps1PerfMarkPrefetchAttempt(slackVBlanks, slackVBlanks, 1);

    if (!fgRuntimeFillWindowForEntry(entry, slackVBlanks, 1, outElapsedVBlanks)) {
        if (ps1PerfEnabled)
            ps1PerfMarkTripwire();
        gFgRuntime.active = 0;
        return 1;
    }
    return 1;
}

static int fgRuntimeWindowPrefetchWouldRead(void)
{
    uint16 targetFrameIndex = 0;
    const struct TFgPilotEntry *entry;

    if (!fgRuntimeCanWindowCache())
        return 0;

    entry = fgRuntimeNextPayloadEntry(&targetFrameIndex);
    (void)targetFrameIndex;
    if (entry == NULL || !fgRuntimeEntryFitsWindow(entry))
        return 0;

    return fgRuntimeWindowContainsEntry(entry) ? 0 : 1;
}

static int fgRuntimeLoadSceneFrame(uint16 frameIndex)
{
    const struct TFgPilotEntry *entry = fgGetEntryFromTable(&gFgRuntime.entryTable, frameIndex);
    int entryIsEmpty;
    int stagedResult;

    if (entry == NULL)
        return 0;

    if (ps1PerfEnabled)
        ps1PerfSetCurrentFrame(frameIndex, entry->sourceFrame, entry->dataOffset);

    stagedResult = fgRuntimeConsumeStagedFrame(frameIndex);
    if (stagedResult != 0)
        return stagedResult > 0 ? 1 : 0;

    /* An empty entry (w=0, h=0, no payload) is a capture artifact from a
     * host frame where the ledger was blank (mid-refresh). Hold the previous
     * frame's visible state rather than wiping, so the viewer doesn't see a
     * one-frame flash. Advance the sound-event cursor + hold timing, but
     * keep currentEntry + currentFrameData pointing at the prior frame. */
    entryIsEmpty = (entry->dataSize == 0 && entry->width == 0 && entry->height == 0);

    if (entryIsEmpty && gFgRuntime.currentFrameData != NULL) {
        gFgRuntime.frameRendered = 1;
        gFgRuntime.displayVBlanks = fgEntryHoldVBlanks(&gFgRuntime.header,
                                                       entry,
                                                       gFgRuntime.presentedVBlanks);
        if (ps1PerfEnabled)
            ps1PerfMarkEntry(0, gFgRuntime.displayVBlanks, 1,
                             entry->sourceFrame, entry->dataOffset);
        fgFireSoundEventsUpTo(entry->sourceFrame);
        fgTelemetryUpdate();
        return 1;
    }

    gFgRuntime.currentEntry = *entry;
    gFgRuntime.currentFrameData = NULL;
    gFgRuntime.frameRendered = 0;

    /* Stream the frame payload into the pre-allocated frameBuffer. Avoids
     * the per-frame malloc+free churn of ps1_streamRead — important for
     * screensaver-loop scenes with big per-frame payloads (fishing3's
     * 89 KB squid-emerge frame was failing its second-iteration contiguous
     * alloc after the heap fragmented). frameBuffer and streamScratch are
     * allocated once at foregroundPilotRuntimeStart. */
    if (gFgRuntime.currentEntry.dataSize > 0 &&
        gFgRuntime.currentEntry.width > 0 &&
        gFgRuntime.currentEntry.height > 0) {
        if (gFgRuntime.frameBuffer == NULL ||
            gFgRuntime.currentEntry.dataSize > gFgRuntime.frameBufferSize ||
            !gFgRuntime.packCdFileValid ||
            gFgRuntime.streamScratch == NULL) {
            if (ps1PerfEnabled)
                ps1PerfMarkTripwire();
            return 0;
        }
        if (fgRuntimeCopyEntryFromWindow(&gFgRuntime.currentEntry,
                                         gFgRuntime.frameBuffer,
                                         1)) {
            gFgRuntime.currentFrameData = gFgRuntime.frameBuffer;
        } else if (fgRuntimeEntryFitsWindow(&gFgRuntime.currentEntry)) {
            if (!fgRuntimeFillWindowForEntry(&gFgRuntime.currentEntry, 0, 0, NULL) ||
                !fgRuntimeCopyEntryFromWindow(&gFgRuntime.currentEntry,
                                              gFgRuntime.frameBuffer,
                                              0)) {
                if (ps1PerfEnabled)
                    ps1PerfMarkTripwire();
                return 0;
            }
            gFgRuntime.currentFrameData = gFgRuntime.frameBuffer;
        } else {
            if (!ps1_streamReadIntoFileBuffered(&gFgRuntime.packCdFile,
                                                gFgRuntime.currentEntry.dataOffset,
                                                gFgRuntime.currentEntry.dataSize,
                                                gFgRuntime.frameBuffer,
                                                gFgRuntime.streamScratch,
                                                gFgRuntime.streamScratchSize)) {
                if (ps1PerfEnabled)
                    ps1PerfMarkTripwire();
                return 0;
            }
            gFgRuntime.currentFrameData = gFgRuntime.frameBuffer;
        }
    }

    gFgRuntime.displayVBlanks = fgEntryHoldVBlanks(&gFgRuntime.header,
                                                   &gFgRuntime.currentEntry,
                                                   gFgRuntime.presentedVBlanks);
    if (ps1PerfEnabled)
        ps1PerfMarkEntry(gFgRuntime.currentEntry.dataSize,
                         gFgRuntime.displayVBlanks,
                         entryIsEmpty ? 1 : 0,
                         gFgRuntime.currentEntry.sourceFrame,
                         gFgRuntime.currentEntry.dataOffset);
    fgFireSoundEventsUpTo(gFgRuntime.currentEntry.sourceFrame);
    fgTelemetryUpdate();
    return 1;
}

static int fgRuntimeCanHoldDisplayedFrame(void)
{
    return gFgRuntime.active &&
           gFgRuntime.mode == FG_RUNTIME_SCENE_PACK &&
           fgRuntimeUsesBaseDiffBackdrop() &&
           gFgRuntime.frameRendered;
}

static void fgRuntimeMarkFrameRendered(void)
{
    if (gFgRuntime.active && gFgRuntime.mode == FG_RUNTIME_SCENE_PACK)
        gFgRuntime.frameRendered = 1;
}

static void fgRuntimeWaitHeldVBlank(void)
{
    VSync(0);
    eventsWaitTick(0);
}

static int fgRuntimeComputeDrawBounds(sint16 *outX, sint16 *outY,
                                      uint16 *outW, uint16 *outH)
{
    int haveBounds = 0;
    int minX = 0;
    int minY = 0;
    int maxX = 0;
    int maxY = 0;
    uint16 i;

    if (outX == NULL || outY == NULL || outW == NULL || outH == NULL)
        return 0;

    for (i = 0; i < gFgRuntime.entryTable.count; i++) {
        const struct TFgPilotEntry *entry = &gFgRuntime.entryTable.entries[i];
        int x;
        int y;
        int endX;
        int endY;

        if (entry->width == 0 || entry->height == 0 || entry->dataSize == 0)
            continue;

        x = fgEntryDrawX(&gFgRuntime.header, entry);
        y = fgEntryDrawY(&gFgRuntime.header, entry);
        endX = x + (int)entry->width;
        endY = y + (int)entry->height;

        if (!haveBounds) {
            minX = x;
            minY = y;
            maxX = endX;
            maxY = endY;
            haveBounds = 1;
        } else {
            if (x < minX) minX = x;
            if (y < minY) minY = y;
            if (endX > maxX) maxX = endX;
            if (endY > maxY) maxY = endY;
        }
    }

    if (!haveBounds || maxX <= minX || maxY <= minY)
        return 0;

    *outX = (sint16)minX;
    *outY = (sint16)minY;
    *outW = (uint16)(maxX - minX);
    *outH = (uint16)(maxY - minY);
    return 1;
}

int foregroundPilotRuntimeStart(const char *sceneName)
{
    fgRuntimeReset();

    if (sceneName == NULL)
        return 0;

    if (fgSceneEquals(sceneName, "testcard")) {
        gFgRuntime.active = 1;
        gFgRuntime.mode = FG_RUNTIME_TESTCARD;
        strncpy(gFgRuntime.sceneName, sceneName, sizeof(gFgRuntime.sceneName) - 1);
        gFgRuntime.holdFrames = kFgPilotProbeHoldFrames;
        gFgRuntime.sceneClockTick = fgReadTickCounter();
        gFgStartedEver = 1;
        fgTelemetryUpdate();
        return 1;
    }

    {
        const char *path = fgCompactOverlayPackPathForScene(sceneName);
        if (path != NULL) {
            uint32 maxDataSize = 0;
            uint16 i;
            if (!fgLoadHeader(path, &gFgRuntime.header))
                return 0;
            if (fgHeaderIsIndexed8Spans(&gFgRuntime.header))
                gFgRuntime.packFormat = kFgPilotPackFormatIndexed8Spans;
            else
                gFgRuntime.packFormat = kFgPilotPackFormatPal4Spans;
            if (!fgLoadPalette(path, &gFgRuntime.header, gFgRuntime.palette)) {
                fgRuntimeReset();
                return 0;
            }
            if (!fgLoadEntryTable(path, &gFgRuntime.header, &gFgRuntime.entryTable)) {
                fgRuntimeReset();
                return 0;
            }
            if (!fgLoadSoundEvents(path, &gFgRuntime.header,
                                   &gFgRuntime.soundEvents,
                                   &gFgRuntime.soundEventCount)) {
                fgRuntimeReset();
                return 0;
            }
            /* Allocate one streaming buffer pair for this scene and reuse it for
             * every frame. This removes per-frame malloc churn without carrying
             * the buffers across the next backdrop load. */
            for (i = 0; i < gFgRuntime.entryTable.count; i++) {
                if (gFgRuntime.entryTable.entries[i].dataSize > maxDataSize)
                    maxDataSize = gFgRuntime.entryTable.entries[i].dataSize;
            }
            if (maxDataSize > gFgFrameBufferSize) {
                if (gFgFrameBuffer != NULL)
                    free(gFgFrameBuffer);
                gFgFrameBuffer = (uint8 *)malloc(maxDataSize);
                if (gFgFrameBuffer == NULL) {
                    if (ps1PerfEnabled)
                        ps1PerfMarkAllocFail(maxDataSize);
                    gFgFrameBufferSize = 0;
                    fgRuntimeReset();
                    return 0;
                }
                gFgFrameBufferSize = maxDataSize;
            }
            if (gFgPrefetchStage1Enabled &&
                (gFgRuntime.header.reserved0 & kFgPilotHeaderFlagBaseDiff) != 0) {
                if (maxDataSize > gFgPrefetchFrameBufferSize) {
                    if (gFgPrefetchFrameBuffer != NULL)
                        free(gFgPrefetchFrameBuffer);
                    gFgPrefetchFrameBuffer = (uint8 *)malloc(maxDataSize);
                    if (gFgPrefetchFrameBuffer == NULL) {
                        if (ps1PerfEnabled)
                            ps1PerfMarkAllocFail(maxDataSize);
                        gFgPrefetchFrameBufferSize = 0;
                        fgRuntimeReset();
                        return 0;
                    }
                    gFgPrefetchFrameBufferSize = maxDataSize;
                }
            }
            if (gFgPrefetchWindowBytes > 0 &&
                (gFgRuntime.header.reserved0 & kFgPilotHeaderFlagBaseDiff) != 0) {
                uint32 windowBytes = ((gFgPrefetchWindowBytes + 2047u) / 2048u) * 2048u;
                if (windowBytes > gFgStreamWindowBufferSize) {
                    if (gFgStreamWindowBuffer != NULL)
                        free(gFgStreamWindowBuffer);
                    gFgStreamWindowBuffer = (uint8 *)malloc(windowBytes);
                    if (gFgStreamWindowBuffer == NULL) {
                        if (ps1PerfEnabled)
                            ps1PerfMarkAllocFail(windowBytes);
                        gFgStreamWindowBufferSize = 0;
                        fgRuntimeReset();
                        return 0;
                    }
                    gFgStreamWindowBufferSize = windowBytes;
                }
            }
            {
                uint32 requiredScratch = ((maxDataSize + 2047u) / 2048u) * 2048u + 2048u;
                if (requiredScratch > gFgStreamScratchSize) {
                    if (gFgStreamScratch != NULL)
                        free(gFgStreamScratch);
                    gFgStreamScratch = (uint8 *)malloc(requiredScratch);
                    if (gFgStreamScratch == NULL) {
                        if (ps1PerfEnabled)
                            ps1PerfMarkAllocFail(requiredScratch);
                        gFgStreamScratchSize = 0;
                        fgRuntimeReset();
                        return 0;
                    }
                    gFgStreamScratchSize = requiredScratch;
                }
            }
            if (ps1PerfEnabled)
                ps1PerfMarkBufferSizes(gFgFrameBufferSize, gFgStreamScratchSize);
            gFgRuntime.frameBuffer = gFgFrameBuffer;
            gFgRuntime.frameBufferSize = gFgFrameBufferSize;
            gFgRuntime.prefetchFrameBuffer = gFgPrefetchFrameBuffer;
            gFgRuntime.prefetchFrameBufferSize = gFgPrefetchFrameBufferSize;
            gFgRuntime.streamWindowBuffer = gFgStreamWindowBuffer;
            gFgRuntime.streamWindowSize = gFgStreamWindowBufferSize;
            gFgRuntime.streamWindowStart = 0;
            gFgRuntime.streamWindowBytes = 0;
            gFgRuntime.streamWindowValid = 0;
            gFgRuntime.streamScratch = gFgStreamScratch;
            gFgRuntime.streamScratchSize = gFgStreamScratchSize;
            if (!ps1_streamResolveFile(path, &gFgRuntime.packCdFile)) {
                fgRuntimeReset();
                return 0;
            }
            gFgRuntime.packCdFileValid = 1;
            if (ps1PerfEnabled) {
                uint32 packBytes = gFgRuntime.packCdFile.size;
                uint32 packSectors = (packBytes + 2047u) / 2048u;
                uint32 packLba = (uint32)CdPosToInt((CdlLOC *)&gFgRuntime.packCdFile.pos);
                ps1PerfSetPackInfo(path,
                                   packBytes,
                                   packLba,
                                   packSectors,
                                   gFgRuntime.header.frameCount,
                                   gFgRuntime.entryTable.count,
                                   gFgRuntime.soundEventCount,
                                   gFgRuntime.header.reserved0,
                                   gFgRuntime.packFormat,
                                   gFgFrameBufferSize,
                                   gFgStreamScratchSize);
                if ((gFgRuntime.header.reserved0 & kFgPilotHeaderFlagBaseDiff) != 0 &&
                    gFgPrefetchStage1Enabled &&
                    gFgPrefetchWindowBytes > 0)
                    ps1PerfSetPrefetchPolicy(PS1_PERF_PREFETCH_STAGE1_WINDOW,
                                             gFgPrefetchFrameBufferSize +
                                             gFgStreamWindowBufferSize);
                else if ((gFgRuntime.header.reserved0 & kFgPilotHeaderFlagBaseDiff) != 0 &&
                         gFgPrefetchStage1Enabled)
                    ps1PerfSetPrefetchPolicy(PS1_PERF_PREFETCH_STAGE1,
                                             gFgPrefetchFrameBufferSize);
                else if ((gFgRuntime.header.reserved0 & kFgPilotHeaderFlagBaseDiff) != 0 &&
                         gFgPrefetchWindowBytes > 0)
                    ps1PerfSetPrefetchPolicy(PS1_PERF_PREFETCH_WINDOW,
                                             gFgStreamWindowBufferSize);
                else
                    ps1PerfSetPrefetchPolicy(PS1_PERF_PREFETCH_NONE, 0);
            }
            gFgRuntime.soundEventCursor = 0;
            gFgRuntime.active = 1;
            gFgRuntime.mode = FG_RUNTIME_SCENE_PACK;
            strncpy(gFgRuntime.sceneName, sceneName, sizeof(gFgRuntime.sceneName) - 1);
            gFgRuntime.displayVBlanks = 1;
            gFgRuntime.holdFrames = 150;
            gFgRuntime.sceneClockTick = fgReadTickCounter();
            {
                uint32 perfFirstFrameTick = 0;
                if (ps1PerfEnabled)
                    perfFirstFrameTick = ps1PerfTick();
                if (!fgRuntimeLoadSceneFrame(0)) {
                    if (ps1PerfEnabled)
                        ps1PerfMarkTripwire();
                    fgRuntimeReset();
                    return 0;
                }
                if (fgRuntimePrimeNextFrameForSetup() < 0) {
                    if (ps1PerfEnabled)
                        ps1PerfMarkTripwire();
                    fgRuntimeReset();
                    return 0;
                }
                if (ps1PerfEnabled)
                    ps1PerfMarkSetupPhase(PS1_PERF_SETUP_FIRST_FRAME,
                                          ps1PerfElapsedVBlanks(perfFirstFrameTick));
            }
            gFgStartedEver = 1;
            fgTelemetryUpdate();
            return 1;
        }
    }

    return 0;
}

void foregroundPilotRuntimeCompose(void)
{
    const uint16 rectW = 120;
    const uint16 rectH = 80;

    if (!gFgRuntime.active)
        return;

    gFgComposedEver = 1;

    if (gFgRuntime.mode == FG_RUNTIME_TESTCARD) {
        static uint16 *colors[4] = { NULL, NULL, NULL, NULL };
        static const uint16 colorValues[4] = { 0x001f, 0x03e0, 0x03ff, 0x7c1f };

        for (int c = 0; c < 4; c++) {
            if (colors[c] == NULL) {
                colors[c] = (uint16 *)malloc((size_t)rectW * (size_t)rectH * sizeof(uint16));
                if (colors[c] == NULL)
                    return;
                for (uint32 j = 0; j < (uint32)rectW * (uint32)rectH; j++)
                    colors[c][j] = colorValues[c];
            }
        }

        fgBlit16ToBackgroundRect(24, 24, rectW, rectH, colors[0]);
        fgBlit16ToBackgroundRect(176, 24, rectW, rectH, colors[1]);
        fgBlit16ToBackgroundRect(24, 136, rectW, rectH, colors[2]);
        fgBlit16ToBackgroundRect(176, 136, rectW, rectH, colors[3]);
        return;
    }

    if (gFgRuntime.mode == FG_RUNTIME_SCENE_PACK && gFgRuntime.currentFrameData != NULL) {
        if (gFgRuntime.packFormat == kFgPilotPackFormatPal4Spans) {
            grCompositePacked4SpansToBackground(gFgRuntime.currentFrameData,
                                                gFgRuntime.currentEntry.dataSize,
                                                gFgRuntime.palette,
                                                (sint16)fgEntryDrawX(&gFgRuntime.header, &gFgRuntime.currentEntry),
                                                (sint16)fgEntryDrawY(&gFgRuntime.header, &gFgRuntime.currentEntry));
        } else if (gFgRuntime.packFormat == kFgPilotPackFormatIndexed8Spans) {
            grCompositeIndexed8SpansToBackground(gFgRuntime.currentFrameData,
                                                 gFgRuntime.currentEntry.dataSize,
                                                 gFgRuntime.palette,
                                                 (sint16)fgEntryDrawX(&gFgRuntime.header, &gFgRuntime.currentEntry),
                                                 (sint16)fgEntryDrawY(&gFgRuntime.header, &gFgRuntime.currentEntry));
        }
        /* Stamp holiday overlay on top of the pack so Johnny walks behind
         * the holiday decoration, matching islandInitHoliday's z-order. */
        fgBackdropStampHoliday();
    }
}

void foregroundPilotRuntimeAdvance(void)
{
    uint16 elapsedVBlanks;

    if (!gFgRuntime.active)
        return;

    elapsedVBlanks = fgElapsedVBlanksSince(&gFgRuntime.sceneClockTick);
    if (elapsedVBlanks == 0)
        elapsedVBlanks = 1;
    if (ps1PerfEnabled)
        ps1PerfMarkAdvance(elapsedVBlanks, gFgRuntime.displayVBlanks);

    if (gFgRuntime.mode == FG_RUNTIME_TESTCARD) {
        if (gFgRuntime.holdFrames > elapsedVBlanks)
            gFgRuntime.holdFrames = (uint16)(gFgRuntime.holdFrames - elapsedVBlanks);
        else
            gFgRuntime.holdFrames = 0;
        if (gFgRuntime.holdFrames == 0)
            gFgRuntime.active = 0;
        fgTelemetryUpdate();
        return;
    }

    if (gFgRuntime.mode == FG_RUNTIME_SCENE_PACK) {
        uint16 frameHoldVBlanks = gFgRuntime.displayVBlanks;

        if (gFgRuntime.frameIndex + 1 >= gFgRuntime.header.frameCount) {
            if (gFgRuntime.holdFrames > elapsedVBlanks)
                gFgRuntime.holdFrames = (uint16)(gFgRuntime.holdFrames - elapsedVBlanks);
            else
                gFgRuntime.holdFrames = 0;
            if (gFgRuntime.holdFrames == 0)
                gFgRuntime.active = 0;
            fgTelemetryUpdate();
            return;
        }

        gFgRuntime.frameVBlank = (uint16)(gFgRuntime.frameVBlank + elapsedVBlanks);
        if (gFgRuntime.frameVBlank < frameHoldVBlanks) {
            gFgRuntime.displayVBlanks = frameHoldVBlanks;
            fgTelemetryUpdate();
            return;
        }

        gFgRuntime.frameVBlank = 0;
        gFgRuntime.presentedVBlanks = (uint16)(gFgRuntime.presentedVBlanks + frameHoldVBlanks);
        gFgRuntime.frameIndex++;
        if (!fgRuntimeLoadSceneFrame(gFgRuntime.frameIndex))
            gFgRuntime.active = 0;
        fgTelemetryUpdate();
    }
}

int foregroundPilotRuntimeActive(void)
{
    return gFgRuntime.active ? 1 : 0;
}

int foregroundPilotRuntimeMode(void)
{
    return gFgRuntime.active ? (int)gFgRuntime.mode : 0;
}

unsigned short foregroundPilotRuntimeFrameIndex(void)
{
    return gFgRuntime.active ? gFgRuntime.frameIndex : 0;
}

unsigned short foregroundPilotRuntimeFrameCount(void)
{
    return gFgRuntime.active ? gFgRuntime.header.frameCount : 0;
}

const char *foregroundPilotRuntimeSceneName(void)
{
    return gFgRuntime.active ? gFgRuntime.sceneName : "";
}

const char *foregroundPilotRuntimeModeName(void)
{
    if (!gFgRuntime.active) return "INACTIVE";
    switch ((int)gFgRuntime.mode) {
    case 0: return "NONE";
    case 1: return "TESTCARD";
    case 2: return "SCENE_PACK";
    default: return "?";
    }
}

unsigned short foregroundPilotRuntimeSourceFrame(void)
{
    return (gFgRuntime.active && gFgRuntime.currentFrameData != NULL)
        ? gFgRuntime.currentEntry.sourceFrame
        : 0;
}

unsigned short foregroundPilotRuntimeDisplayVBlanks(void)
{
    return gFgRuntime.active ? gFgRuntime.displayVBlanks : 0;
}

int foregroundPilotRuntimeHasFrameData(void)
{
    return (gFgRuntime.active && gFgRuntime.currentFrameData != NULL) ? 1 : 0;
}

int foregroundPilotConfiguredEver(void)
{
    return gFgConfiguredEver ? 1 : 0;
}

int foregroundPilotSetClearedEver(void)
{
    return gFgSetClearedEver ? 1 : 0;
}

int foregroundPilotRequestedNow(void)
{
    return foregroundPilotRequested();
}

int foregroundPilotRuntimeAdsMatchEver(void)
{
    return gFgAdsMatchEver ? 1 : 0;
}

int foregroundPilotRuntimeStartAttemptedEver(void)
{
    return gFgStartAttemptEver ? 1 : 0;
}

int foregroundPilotRuntimeStartedEver(void)
{
    return gFgStartedEver ? 1 : 0;
}

int foregroundPilotRuntimeComposedEver(void)
{
    return gFgComposedEver ? 1 : 0;
}

void foregroundPilotRuntimeEnd(void)
{
    fgRuntimeReset();
}

static void fgPlayTitleCopy(void)
{
    fgShowRawFrame("\\TITLE.RAW;1", kFgPilotProbeHoldFrames);
}

static void fgPlayIsleTest(void)
{
    uint16 i;

    fgInitVisiblePipeline();
    grLoadScreen("ISLETEMP.SCR");
    for (i = 0; i < kFgPilotProbeHoldFrames; i++)
        grUpdateDisplay(NULL, NULL, NULL);
}

static void fgPlayOceanTest(void)
{
    uint16 i;

    fgInitVisiblePipeline();
    grLoadScreen("OCEAN00.SCR");
    for (i = 0; i < kFgPilotProbeHoldFrames; i++)
        grUpdateDisplay(NULL, NULL, NULL);
}

static void fgPlayOceanRuntimeScene(const char *sceneName)
{
    sint16 fgBoundsX = 0;
    sint16 fgBoundsY = 0;
    uint16 fgBoundsW = 0;
    uint16 fgBoundsH = 0;
    uint32 perfPhaseTick = 0;
    int perfDetail = ps1PerfEnabled ? ps1PerfDetailEnabled() : 0;

    fgHeapProbe("before_scene", sceneName);
    /* Clean-rect snapshots are tied to the current backdrop contents. Carrying
     * their large buffers into the next backdrop load starves the second scene
     * start on PS1, so release them before any new SCR/BMP loads. */
    grFreeCleanBgRects();
    fgReleaseStreamBuffers();

    /* Pre-load BACKGRND.BMP before any scene setup allocates bg tiles. At
     * this moment the heap is freshest and the ~93 KB PSB stream has room. */
    if (ps1PerfEnabled)
        perfPhaseTick = ps1PerfTick();
    fgBackdropPreloadBackgrndBmp();
    if (ps1PerfEnabled)
        ps1PerfMarkSetupPhase(PS1_PERF_SETUP_BACKDROP,
                              ps1PerfElapsedVBlanks(perfPhaseTick));

    fgInitVisiblePipeline();
    grSetPresentDuringScreenLoad(0);
    if (ps1PerfEnabled)
        perfPhaseTick = ps1PerfTick();
    if (islandState.night) {
        /* NIGHT.SCR is the full night-ocean backdrop, no island baked in.
         * The FG2 backdrop helper draws the island sprites on top. */
        grLoadScreen("NIGHT.SCR");
    } else {
        /* OCEAN00.SCR has no island baked in. The island must be drawn through
         * the movable BACKGRND.BMP path so scene-relative FG2 packs can follow
         * randomized island placement. */
        grLoadScreen("OCEAN00.SCR");
    }
    if (ps1PerfEnabled)
        ps1PerfMarkSetupPhase(PS1_PERF_SETUP_SCREEN,
                              ps1PerfElapsedVBlanks(perfPhaseTick));
    /* grLoadScreen saves full clean-tile copies. FG2 uses a smaller
     * rect-mode backup instead, so free the full copies before the pack
     * allocates its streaming buffer. */
    grFreeCleanBgTiles();
    if (ps1PerfEnabled)
        perfPhaseTick = ps1PerfTick();
    fgBackdropEnableWaveBackdrop();
    if (ps1PerfEnabled)
        ps1PerfMarkSetupPhase(PS1_PERF_SETUP_BACKDROP,
                              ps1PerfElapsedVBlanks(perfPhaseTick));
    grSetPresentDuringScreenLoad(1);

    if (ps1PerfEnabled)
        perfPhaseTick = ps1PerfTick();
    if (!foregroundPilotRuntimeStart(sceneName)) {
        fgRuntimeReset();
        fgReleaseStreamBuffers();
        grFreeCleanBgRects();
        fgBackdropRelease(0);
        fgHeapProbe("start_failed_cleanup", sceneName);
        return;
    }
    if (ps1PerfEnabled)
        ps1PerfMarkSetupPhase(PS1_PERF_SETUP_PACK_START,
                              ps1PerfElapsedVBlanks(perfPhaseTick));
    fgHeapProbe("after_pack_start", sceneName);

    /* Now that the pack header is loaded, size the rect-mode clean backup
     * to cover the actual runtime draw bbox. Pack entries may be scene-relative.
     * Fishing3 exposes the difference: some squid frames draw above the header
     * union after the current island offset is applied. */
    if (ps1PerfEnabled)
        perfPhaseTick = ps1PerfTick();
    if (!fgRuntimeComputeDrawBounds(&fgBoundsX, &fgBoundsY,
                                    &fgBoundsW, &fgBoundsH) ||
        !fgBackdropSaveCleanBgRectsForPack(fgBoundsX, fgBoundsY,
                                           fgBoundsW, fgBoundsH)) {
        fgRuntimeReset();
        fgReleaseStreamBuffers();
        grFreeCleanBgRects();
        fgBackdropRelease(0);
        fgHeapProbe("clean_rect_failed_cleanup", sceneName);
        return;
    }
    if (ps1PerfEnabled)
        ps1PerfMarkSetupPhase(PS1_PERF_SETUP_CLEAN_RECT,
                              ps1PerfElapsedVBlanks(perfPhaseTick));
    fgHeapProbe("after_clean_rect_save", sceneName);

    if (ps1PerfEnabled)
        ps1PerfMarkLoopStart();
    while (foregroundPilotRuntimeActive()) {
        /* Pause-menu request: break out so jc_reborn's outer loop can
         * advance to next scene or restart the loop. The flag is
         * cleared by the consumer in jc_reborn.c. */
        if (pauseMenuRequestNextScene || pauseMenuRequestResetLoop) {
            break;
        }
        if (fgRuntimeCanHoldDisplayedFrame()) {
            uint16 prefetchElapsedVBlanks = 0;
            int didPrefetch;
            if (ps1PerfEnabled)
                ps1PerfMarkHeldLoop();
            if (gFgRuntime.stagedFrameValid) {
                didPrefetch = fgRuntimeWindowPrefetchWouldRead()
                    ? fgRuntimeTryPrefetchWindow(&prefetchElapsedVBlanks)
                    : 0;
            } else {
                didPrefetch = fgRuntimeTryStageNextFrame(&prefetchElapsedVBlanks);
                if (didPrefetch &&
                    prefetchElapsedVBlanks == 0 &&
                    gFgRuntime.stagedFrameValid &&
                    fgRuntimeHeldSlackBeforeWait() >= FG_PREFETCH_FALLTHROUGH_MIN_SLACK_VBLANKS &&
                    fgRuntimeWindowPrefetchWouldRead()) {
                    uint16 windowElapsedVBlanks = 0;
                    if (fgRuntimeTryPrefetchWindow(&windowElapsedVBlanks))
                        prefetchElapsedVBlanks = windowElapsedVBlanks;
                }
            }
            if (!didPrefetch && !gFgRuntime.stagedFrameValid)
                didPrefetch = fgRuntimeTryPrefetchWindow(&prefetchElapsedVBlanks);
            if (didPrefetch) {
                if (prefetchElapsedVBlanks == 0)
                    fgRuntimeWaitHeldVBlank();
                else
                    eventsWaitTick(0);
            } else {
                fgRuntimeWaitHeldVBlank();
            }
        } else {
            uint32 perfRenderTick = 0;
            uint32 perfDetailTick = 0;
            int usesBaseDiffBackdrop = fgRuntimeUsesBaseDiffBackdrop();
            if (ps1PerfEnabled)
                ps1PerfMarkRenderedLoop();
            if (perfDetail)
                perfRenderTick = ps1PerfTick();
            if (!usesBaseDiffBackdrop)
                grBeginFrame();
            if (perfDetail)
                perfDetailTick = ps1PerfTick();
            grRestoreBgFromRects();
            if (perfDetail)
                ps1PerfMarkRenderPhase(PS1_PERF_RENDER_RESTORE,
                                       ps1PerfElapsedVBlanks(perfDetailTick));
            if (!usesBaseDiffBackdrop)
                fgBackdropTickBackgroundWaves();
            grUpdateDisplay(NULL, NULL, NULL);
            if (perfDetail)
                ps1PerfMarkRenderTotal(ps1PerfElapsedVBlanks(perfRenderTick));
            fgRuntimeMarkFrameRendered();
        }
        if (perfDetail)
            perfPhaseTick = ps1PerfTick();
        foregroundPilotRuntimeAdvance();
        if (perfDetail)
            ps1PerfMarkRenderPhase(PS1_PERF_RENDER_ADVANCE,
                                   ps1PerfElapsedVBlanks(perfPhaseTick));
    }
    if (ps1PerfEnabled) {
        ps1PerfMarkLoopEnd();
        ps1PerfMarkSoundCursor(gFgRuntime.soundEventCursor);
        ps1PerfMarkCleanupStart();
    }

    /* End-of-scene heap cleanup — the screensaver loop replays scenes
     * indefinitely, so every per-scene allocation needs to come back to
     * the heap before the next scene starts. Without this the fragmented
     * state accumulates and a later scene's large contiguous alloc
     * (clean-rect buffer ~270 KB, frame payload ~90 KB, BACKGRND PSB
     * ~93 KB) fails silently after 2-3 iterations. */
    fgRuntimeReset();
    fgReleaseStreamBuffers();
    grFreeCleanBgRects();
    /* Keep BACKGRND.BMP in slot 0 across scenes; release only
     * variant-dependent overlay slots to avoid needless PSB churn. */
    fgBackdropRelease(1);
    fgHeapProbe("after_scene_cleanup", sceneName);
}

static void fgShowSolidColor(uint8 r, uint8 g, uint8 b, uint16 holdFrames)
{
    DISPENV disp;
    DRAWENV draw;
    uint16 i;

    ResetGraph(0);
    SetDefDispEnv(&disp, 0, 0, 640, 480);
    SetDefDrawEnv(&draw, 0, 0, 640, 480);
    disp.isinter = 1;
    setRGB0(&draw, r, g, b);
    draw.isbg = 1;

    PutDispEnv(&disp);
    PutDrawEnv(&draw);
    SetDispMask(1);

    DrawSync(0);
    for (i = 0; i < holdFrames; i++)
        VSync(0);
}

static void fgPlaySolidRed(void)
{
    fgShowSolidColor(255, 0, 0, kFgPilotProbeHoldFrames);
}

int foregroundPilotRequested(void)
{
    return gForegroundPilotRequestedMode != FG_RUNTIME_NONE;
}

const char *foregroundPilotSceneName(void)
{
    return gForegroundPilotScene;
}

void foregroundPilotSetScene(const char *sceneName)
{
    size_t i;

    if (!sceneName) {
        gForegroundPilotScene[0] = '\0';
        gForegroundPilotRequestedMode = FG_RUNTIME_NONE;
        gFgSetClearedEver = 1;
        return;
    }

    for (i = 0; i + 1 < sizeof(gForegroundPilotScene) && sceneName[i] != '\0'; i++)
        gForegroundPilotScene[i] = sceneName[i];
    gForegroundPilotScene[i] = '\0';
    gForegroundPilotRequestedMode = fgSceneModeForName(gForegroundPilotScene);
    gFgConfiguredEver = 1;
}

void foregroundPilotSetHeapProbe(int enabled)
{
    gFgHeapProbeEnabled = enabled ? 1 : 0;
}

void foregroundPilotResetPrefetchDefaults(void)
{
    gFgPrefetchStage1Enabled = 1;
    gFgPrefetchWindowBytes = FG_PREFETCH_DEFAULT_WINDOW_BYTES;
}

void foregroundPilotSetPrefetchStage1(int enabled)
{
    gFgPrefetchStage1Enabled = enabled ? 1 : 0;
}

void foregroundPilotSetPrefetchWindow(unsigned long bytes)
{
    gFgPrefetchWindowBytes = (uint32)bytes;
}

int foregroundPilotShouldStartForAds(const char *adsName, unsigned short adsTag)
{
    (void)adsName;
    (void)adsTag;
    return 0;
}

int foregroundPilotRuntimeStartRequested(void)
{
    if (!foregroundPilotRequested())
        return 0;

    gFgStartAttemptEver = 1;
    switch (gForegroundPilotRequestedMode) {
        case FG_RUNTIME_TESTCARD:
            return foregroundPilotRuntimeStart("testcard");
        default:
            return foregroundPilotRuntimeStart(gForegroundPilotScene);
    }
}

int foregroundPilotRuntimeStartIfRequested(void)
{
    if (!foregroundPilotRequested())
        return 1;
    if (foregroundPilotRuntimeActive())
        return 1;
    return foregroundPilotRuntimeStartRequested();
}

void foregroundPilotPlay(void)
{
    if (fgSceneEquals(gForegroundPilotScene, "testcard")) {
        fgPlayTestCard();
        return;
    }

    if (fgCompactOverlayPackPathForScene(gForegroundPilotScene) != NULL) {
        fgPlayOceanRuntimeScene(gForegroundPilotScene);
        return;
    }

    if (fgSceneEquals(gForegroundPilotScene, "titlecopy")) {
        fgPlayTitleCopy();
        return;
    }

    if (fgSceneEquals(gForegroundPilotScene, "isletest")) {
        fgPlayIsleTest();
        return;
    }

    if (fgSceneEquals(gForegroundPilotScene, "oceantest")) {
        fgPlayOceanTest();
        return;
    }

    if (fgSceneEquals(gForegroundPilotScene, "solidred")) {
        fgPlaySolidRed();
        return;
    }

    printf("FG pilot: unknown scene '%s'\n", gForegroundPilotScene);
}

#else

static char gForegroundPilotScene[16] = "";
static unsigned char gForegroundPilotRequestedMode = 0;

int foregroundPilotRequested(void)
{
    return gForegroundPilotRequestedMode != 0;
}

const char *foregroundPilotSceneName(void)
{
    return gForegroundPilotScene;
}

void foregroundPilotSetScene(const char *sceneName)
{
    if (!sceneName) {
        gForegroundPilotScene[0] = '\0';
        gForegroundPilotRequestedMode = 0;
        return;
    }
    strncpy(gForegroundPilotScene, sceneName, sizeof(gForegroundPilotScene) - 1);
    gForegroundPilotScene[sizeof(gForegroundPilotScene) - 1] = '\0';
    gForegroundPilotRequestedMode = 1;
}

void foregroundPilotSetHeapProbe(int enabled)
{
    (void)enabled;
}

void foregroundPilotResetPrefetchDefaults(void)
{
}

void foregroundPilotSetPrefetchStage1(int enabled)
{
    (void)enabled;
}

void foregroundPilotSetPrefetchWindow(unsigned long bytes)
{
    (void)bytes;
}

int foregroundPilotShouldStartForAds(const char *adsName, unsigned short adsTag)
{
    (void)adsName;
    (void)adsTag;
    return 0;
}

int foregroundPilotRuntimeStartRequested(void)
{
    return 0;
}

int foregroundPilotRuntimeStartIfRequested(void)
{
    return foregroundPilotRequested() ? 0 : 1;
}

int foregroundPilotRuntimeMode(void)
{
    return 0;
}

unsigned short foregroundPilotRuntimeFrameIndex(void)
{
    return 0;
}

unsigned short foregroundPilotRuntimeSourceFrame(void)
{
    return 0;
}

unsigned short foregroundPilotRuntimeDisplayVBlanks(void)
{
    return 0;
}

int foregroundPilotRuntimeHasFrameData(void)
{
    return 0;
}

int foregroundPilotConfiguredEver(void)
{
    return foregroundPilotRequested() ? 1 : 0;
}

int foregroundPilotSetClearedEver(void)
{
    return 0;
}

int foregroundPilotRequestedNow(void)
{
    return foregroundPilotRequested() ? 1 : 0;
}

int foregroundPilotRuntimeAdsMatchEver(void)
{
    return 0;
}

int foregroundPilotRuntimeStartAttemptedEver(void)
{
    return 0;
}

int foregroundPilotRuntimeStartedEver(void)
{
    return 0;
}

int foregroundPilotRuntimeComposedEver(void)
{
    return 0;
}

void foregroundPilotPlay(void)
{
    fprintf(stderr, "foreground pilot is PS1-only for now (%s)\n", gForegroundPilotScene);
}

#endif
