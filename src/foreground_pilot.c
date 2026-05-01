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
#include "holidays.h"
#include "pause_menu.h"
#include "ps1_captions.h"
#include "sound_ps1.h"
#include "utils.h"
#include "ps1_perf.h"
#include "walk_pilot.h"
#include "ps1_debug.h"

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

struct TFgPilotReadGroup;

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
    uint32 streamWindowReadSize;    /* Normal read size; capacity may be larger for grouped appends. */
    uint32 streamWindowStart;       /* File offset of first byte in streamWindowBuffer. */
    uint32 streamWindowBytes;       /* Valid byte count in streamWindowBuffer. */
    uint32 setupPrimeWindowBytes;   /* Scene/tide-specific setup residency budget, or 0 when disabled. */
    uint8 *setupSegmentBuffer;      /* Small setup-read segment retained independently from streamScratch. */
    uint32 setupSegmentStart;
    uint32 setupSegmentBytes;
    const struct TFgPilotReadGroup *streamReadGroups;
    uint8 streamReadGroupCount;
    uint8 streamWindowValid;
    uint8 setupWindowPrimed;
    uint8 setupSegmentPrimed;
    uint8 *streamScratch;           /* Pre-allocated sector-aligned scratch for CD reads. */
    uint32 streamScratchSize;       /* Capacity of streamScratch. */
    CdlFILE packCdFile;             /* Resolved CD file handle for the pack (avoids per-frame CdSearchFile). */
    uint8 packCdFileValid;
    uint8 packFormat;
    uint8 stagedFrameValid;
    uint16 stagedFrameIndex;
    struct TFgPilotEntry stagedEntry;
    uint8 preparedFrameValid;
    uint16 preparedFrameIndex;
    uint16 palette[256];
    struct TFgPilotSoundEvent *soundEvents;
    uint16 soundEventCount;
    uint16 soundEventCursor;
};

static char gForegroundPilotScene[16] = "";
#ifndef FG_ENABLE_LEGACY_DIAGNOSTIC_SCENES
#define FG_ENABLE_LEGACY_DIAGNOSTIC_SCENES 0
#endif
#ifndef FG_HEAP_PROBE_LOGS
#define FG_HEAP_PROBE_LOGS 0
#endif
#if FG_ENABLE_LEGACY_DIAGNOSTIC_SCENES
static const uint16 kFgPilotProbeHoldFrames = 1800;
#endif
static const uint16 kFgPilotHeaderFlagHostTicks = 0x0002;
static const uint16 kFgPilotHeaderFlagHostDeadlines = 0x0004;
static const uint16 kFgPilotHeaderFlagSceneRelative = 0x0008;
static const uint16 kFgPilotHeaderFlagBaseDiff = 0x0010;
static const uint8 kFgPilotPackFormatPal4Spans = 2;
static const uint8 kFgPilotPackFormatIndexed8Spans = 3;
static const uint8 kFgPilotPackFormatPal4TemporalResidual = 4;
static const uint8 kFgPilotPackFormatIndexed8TemporalResidual = 5;
enum {
    FG_PACK_HEADER_SIZE = 40,
    FG_PACK_ENTRY_SIZE = 20,
    FG_PACK_METADATA_PREFIX_BYTES = 8192
};
#define FG_PREFETCH_DEFAULT_WINDOW_BYTES (16UL * 1024UL)
#define FG_ACTIVITY9_HIGH_WINDOW_BYTES (20UL * 1024UL)
#define FG_BUILDING4_HIGH_WINDOW_BYTES (24UL * 1024UL)
#define FG_BUILDING4_LOW_WINDOW_BYTES (32UL * 1024UL)
#define FG_BUILDING6_WINDOW_BYTES (24UL * 1024UL)
#define FG_WALKSTUF1_HIGH_RESIDUAL_WINDOW_BYTES (54UL * 1024UL)
#define FG_WALKSTUF1_LOW_RESIDUAL_WINDOW_BYTES (40UL * 1024UL)
#define FG_WALKSTUF1_SETUP_PRIME_BASE_BYTES (88UL * 1024UL)
#define FG_WALKSTUF1_HIGH_SETUP_PRIME_TRIM_BYTES (4UL * 1024UL)
#define FG_PREFETCH_GROUP_WINDOW_BYTES (13UL * 2048UL)
#define FG_SETUP_PRIME_WINDOW_BYTES (320UL * 1024UL)
#define FG_SETUP_PRIME_MAX_RESIDENT_BYTES (128UL * 1024UL)
#define FG_ACTIVITY12_HIGH_SETUP_PRIME_WINDOW_BYTES (328UL * 1024UL)
#define FG_FISHING2_SETUP_PRIME_WINDOW_BYTES (352UL * 1024UL)
#define FG_FISHING6_HIGH_SETUP_PRIME_WINDOW_BYTES (312UL * 1024UL)
#define FG_FISHING7_HIGH_SETUP_PRIME_WINDOW_BYTES (328UL * 1024UL)
#define FG_JOHNNY3_HIGH_SETUP_PRIME_WINDOW_BYTES (312UL * 1024UL)
#define FG_VISITOR3_HIGH_SETUP_PRIME_WINDOW_BYTES (216UL * 1024UL)
#define FG_VISITOR3_LOW_SETUP_PRIME_WINDOW_BYTES (208UL * 1024UL)
#define FG_VISITOR1_HIGH_SETUP_PRIME_WINDOW_BYTES (296UL * 1024UL)
#define FG_VISITOR7_HIGH_SETUP_PRIME_WINDOW_BYTES (368UL * 1024UL)
#define FG_SETUP_PRIME_AUTO_PACK_BYTES (288UL * 1024UL)
#define FG_CD_SECTOR_SIZE 2048UL
#define fgSectorAlignDown(offset) ((uint32)((offset) & ~(FG_CD_SECTOR_SIZE - 1UL)))
#define fgSectorAlignUp(offset) ((uint32)(((offset) + FG_CD_SECTOR_SIZE - 1UL) & ~(FG_CD_SECTOR_SIZE - 1UL)))
#define FG_FISHING3_HIGH_SETUP_SEGMENT_START (67UL * FG_CD_SECTOR_SIZE)
#define FG_FISHING3_HIGH_SETUP_SEGMENT_BYTES (6UL * FG_CD_SECTOR_SIZE)
#define FG_FISHING3_LOW_SETUP_SEGMENT_START (146UL * FG_CD_SECTOR_SIZE)
#define FG_FISHING3_LOW_SETUP_SEGMENT_BYTES (6UL * FG_CD_SECTOR_SIZE)
#define fgRuntimeWindowReadSize() (gFgRuntime.streamWindowReadSize)
/* Below 3 VBlanks, window refills are more likely to become visible delay. */
#define FG_PREFETCH_WINDOW_MIN_SLACK_VBLANKS 3
#define fgRuntimeWindowSlackEligible(slackVBlanks) \
    (((slackVBlanks) >= FG_PREFETCH_WINDOW_MIN_SLACK_VBLANKS) ? 1 : 0)
#define FG_PREFETCH_FALLTHROUGH_MIN_SLACK_VBLANKS 6
#define FG_PREFETCH_DIRECT_STAGE_MAX_BYTES (8UL * 1024UL)
#define FG_PREPARE_PRESENT_MIN_SLACK_VBLANKS 4
#define fgEntryHasPayload(entry) \
    (((entry) != NULL && \
      (entry)->dataSize > 0 && \
      (entry)->width > 0 && \
      (entry)->height > 0) ? 1 : 0)
#define fgRuntimeUsesTemporalResidual() \
    ((gFgRuntime.packFormat == kFgPilotPackFormatPal4TemporalResidual || \
      gFgRuntime.packFormat == kFgPilotPackFormatIndexed8TemporalResidual) ? 1 : 0)
#define fgRuntimeHeldSlackBeforeWait() \
    ((uint16)((!gFgRuntime.active || \
               gFgRuntime.displayVBlanks == 0 || \
               gFgRuntime.displayVBlanks <= gFgRuntime.frameVBlank) ? \
              0 : (gFgRuntime.displayVBlanks - gFgRuntime.frameVBlank)))
#define fgRuntimeCanHoldDisplayedFrame() \
    (gFgRuntime.active && \
     gFgRuntime.mode == FG_RUNTIME_SCENE_PACK && \
     gFgRuntime.frameRendered)
#define fgRuntimeCanPrepareStagedFrame() \
    (gFgRuntime.active && \
     gFgRuntime.mode == FG_RUNTIME_SCENE_PACK && \
     gFgRuntime.frameRendered && \
     gFgRuntime.stagedFrameValid && \
     !gFgRuntime.preparedFrameValid && \
     gFgRuntime.stagedFrameIndex == (uint16)(gFgRuntime.frameIndex + 1) && \
     fgRuntimeHeldSlackBeforeWait() == FG_PREPARE_PRESENT_MIN_SLACK_VBLANKS)
#define fgRuntimeCanPresentPreparedOnNextVBlank() \
    (gFgRuntime.active && \
     gFgRuntime.mode == FG_RUNTIME_SCENE_PACK && \
     gFgRuntime.frameRendered && \
     gFgRuntime.stagedFrameValid && \
     gFgRuntime.preparedFrameValid && \
     gFgRuntime.preparedFrameIndex == (uint16)(gFgRuntime.frameIndex + 1) && \
     gFgRuntime.stagedFrameIndex == gFgRuntime.preparedFrameIndex && \
     fgRuntimeHeldSlackBeforeWait() == 1)
#define fgRuntimeSetStagedFrame(frameIndex_, entry_) \
    do { \
        gFgRuntime.stagedEntry = *(entry_); \
        gFgRuntime.stagedFrameIndex = (frameIndex_); \
        gFgRuntime.stagedFrameValid = 1; \
    } while (0)
#define fgRuntimeWaitHeldVBlank() \
    do { \
        VSync(0); \
        eventsWaitTick(0); \
    } while (0)
#define fgReadTickCounter() ((uint32)VSync(-1))
#define fgRuntimeMarkFrameRendered() \
    do { \
        if (gFgRuntime.active && gFgRuntime.mode == FG_RUNTIME_SCENE_PACK) \
            gFgRuntime.frameRendered = 1; \
    } while (0)
static struct TFgPilotRuntime gFgRuntime = {0};
#if FG_HEAP_PROBE_LOGS
static uint8 gFgHeapProbeEnabled = 0;
#endif
/* Prefetch is required for scenes to play at full speed — without it
 * each frame is a synchronous CD read, dropping playback to ~1/10
 * speed. Earlier we defaulted this OFF to free heap during scene setup
 * (the boot pre-alloc strategy was over-budget). Now that the boot
 * pre-allocs are gone and the prefetch buffer is grow-only via
 * fgReleaseStreamBuffers (we keep gFgPrefetchFrameBuffer alive across
 * scenes once allocated), prefetch is back on by default. */
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
static void fgBackdropStampHoliday(void);
static void fgBackdropRelease(int keepBackgrnd);

/* Public accessor for walk_pilot — returns the slot holding
 * BACKGRND.PSB sprites (used for behind-tree trunk + leaf cover-up
 * during walk transitions). NULL if backdrop isn't loaded yet. */
struct TTtmSlot *fgBackdropGetSlot(void)
{
    if (gFgBackdropSlot.numSprites[0] == 0)
        return NULL;
    return &gFgBackdropSlot;
}

/* Public wrapper around the file-static fgBackdropStampHoliday().
 * Used by walk_pilot.c during walk transitions so the holiday
 * emblem persists. Defined below where the actual stamp logic is;
 * forward decl already exists at file top (line 243). */
void fgBackdropStampHolidayPublic(void)
{
    fgBackdropStampHoliday();
}

/* Public wrapper for the wave animation tick. walk_pilot calls this
 * each frame so the ocean keeps moving during walks. Mirrors what
 * the FG2 frame loop does at line 1062. */
void fgBackdropTickWavesPublic(void)
{
    if (gFgBackdropThread.isRunning) {
        islandAnimate(&gFgBackdropThread);
    }
}

/* No-op kept for ABI compatibility; walk_pilot now reuses the FG2 scene's
 * pre-playback rect snapshot rather than rebuilding bg at walk-time. The
 * earlier grLoadScreen approach worked but cost ~600KB malloc/free + a
 * CD reload per walk, fragmenting the heap so the *next* scene's bg load
 * stalled after a handful of transitions. */
void fgBackdropRebuildIslandBg(void)
{
}

/* Snapshot a walk-area bounding rect as clean. Spots A-F live in
 * approximately X∈[300..522], Y∈[213..255]. With Johnny's sprite
 * size (~40 wide × 60 tall) factored in, the walk's true draw area
 * is closer to X∈[240..580], Y∈[170..320]. Use a single generous
 * rect that covers all walk poses; the rect-clean machinery handles
 * the wave region separately via the same call shape used by
 * fgBackdropSaveCleanBgRectsForPack. */
int fgBackdropSaveCleanBgRectsForWalk(void)
{
    /* Generous walk bbox + the wave region merged. Same shape FG2
     * scenes use (see fgBackdropSaveCleanBgRectsForPack). */
    return fgBackdropSaveCleanBgRectsForPack(
        /* fgX  */ 240,
        /* fgY  */ 170,
        /* fgW  */ 340,
        /* fgH  */ 160);
}

void fgBackdropEndWalk(void)
{
    grFreeCleanBgRects();
}

struct TFgPilotReadGroup {
    uint16 startSector;
    uint16 endSector;
};

struct TFgRuntimeWindowPolicy {
    const char *sceneName;
    uint32 highWindowBytes;
    uint32 lowWindowBytes;
    uint8 requiredPackFormat;
};

struct TFgRuntimeSetupPrimePolicy {
    const char *sceneName;
    uint32 highWindowBytes;
    uint32 lowWindowBytes;
};

static const struct TFgRuntimeWindowPolicy kFgRuntimeWindowPolicies[] = {
    {"activity9", FG_ACTIVITY9_HIGH_WINDOW_BYTES, FG_PREFETCH_DEFAULT_WINDOW_BYTES, 0},
    {"building4", FG_BUILDING4_HIGH_WINDOW_BYTES, FG_BUILDING4_LOW_WINDOW_BYTES, 0},
    {"building6", FG_BUILDING6_WINDOW_BYTES, FG_BUILDING6_WINDOW_BYTES, 0},
    {
        "walkstuf1",
        FG_WALKSTUF1_HIGH_RESIDUAL_WINDOW_BYTES,
        FG_WALKSTUF1_LOW_RESIDUAL_WINDOW_BYTES,
        kFgPilotPackFormatIndexed8TemporalResidual
    }
};

static const struct TFgRuntimeSetupPrimePolicy kFgRuntimeSetupPrimePolicies[] = {
    {"fishing1", FG_SETUP_PRIME_WINDOW_BYTES, FG_SETUP_PRIME_WINDOW_BYTES},
    {
        "fishing2",
        FG_FISHING2_SETUP_PRIME_WINDOW_BYTES,
        FG_FISHING2_SETUP_PRIME_WINDOW_BYTES - (96UL * 1024UL)
    },
    {"activity12", FG_ACTIVITY12_HIGH_SETUP_PRIME_WINDOW_BYTES, 0},
    {"fishing3", 128UL * 1024UL, FG_SETUP_PRIME_WINDOW_BYTES - (32UL * 1024UL)},
    {"fishing6", FG_FISHING6_HIGH_SETUP_PRIME_WINDOW_BYTES, 0},
    {"fishing7", FG_FISHING7_HIGH_SETUP_PRIME_WINDOW_BYTES, 0},
    {"johnny3", FG_JOHNNY3_HIGH_SETUP_PRIME_WINDOW_BYTES, 0},
    {"visitor3", FG_VISITOR3_HIGH_SETUP_PRIME_WINDOW_BYTES, FG_VISITOR3_LOW_SETUP_PRIME_WINDOW_BYTES},
    {"visitor1", FG_VISITOR1_HIGH_SETUP_PRIME_WINDOW_BYTES, 0},
    {"visitor7", FG_VISITOR7_HIGH_SETUP_PRIME_WINDOW_BYTES, 0}
};

struct TFgPilotSceneFamily {
    const char *scenePrefix;
    const char *adsName;
    const char *highPackPrefix;
    const char *lowPackPrefix;
    uint32 tagMask;
};

#define FG_TAG_MASK(tag_) (1UL << (tag_))
#define FG_TAG_RANGE_1_2 (FG_TAG_MASK(1) | FG_TAG_MASK(2))
#define FG_TAG_RANGE_1_3 (FG_TAG_RANGE_1_2 | FG_TAG_MASK(3))
#define FG_TAG_RANGE_1_5 (FG_TAG_RANGE_1_3 | FG_TAG_MASK(4) | FG_TAG_MASK(5))
#define FG_TAG_RANGE_1_6 (FG_TAG_RANGE_1_5 | FG_TAG_MASK(6))
#define FG_TAG_RANGE_1_7 (FG_TAG_RANGE_1_6 | FG_TAG_MASK(7))
#define FG_TAG_RANGE_1_8 (FG_TAG_RANGE_1_7 | FG_TAG_MASK(8))

static const struct TFgPilotSceneFamily kFgPilotSceneFamilies[] = {
    { "activity", "ACTIVITY", "ACTV",    "ACTV",
      FG_TAG_MASK(1) | FG_TAG_MASK(4) | FG_TAG_MASK(5) |
      FG_TAG_MASK(6) | FG_TAG_MASK(7) | FG_TAG_MASK(8) |
      FG_TAG_MASK(9) | FG_TAG_MASK(10) | FG_TAG_MASK(11) |
      FG_TAG_MASK(12) },
    { "building", "BUILDING", "BUIL",    "BUIL", FG_TAG_RANGE_1_7 },
    { "fishing",  "FISHING",  "FISHING", "FISH", FG_TAG_RANGE_1_8 },
    { "johnny",   "JOHNNY",   "JOHNNY",  "JOHN", FG_TAG_RANGE_1_6 },
    { "mary",     "MARY",     "MARY",    "MARY", FG_TAG_RANGE_1_5 },
    { "miscgag",  "MISCGAG",  "MISCGAG", "MISC", FG_TAG_RANGE_1_2 },
    { "stand",    "STAND",    "STAND",   "STND",
      FG_TAG_RANGE_1_8 | FG_TAG_MASK(9) | FG_TAG_MASK(10) |
      FG_TAG_MASK(11) | FG_TAG_MASK(12) | FG_TAG_MASK(15) |
      FG_TAG_MASK(16) },
    { "suzy",     "SUZY",     "SUZY",    "SUZY", FG_TAG_RANGE_1_2 },
    { "visitor",  "VISITOR",  "VISITOR", "VIST",
      FG_TAG_MASK(1) | FG_TAG_MASK(3) | FG_TAG_MASK(4) |
      FG_TAG_MASK(5) | FG_TAG_MASK(6) | FG_TAG_MASK(7) },
    { "walkstuf", "WALKSTUF", "WALK",    "WALK", FG_TAG_RANGE_1_3 }
};

static char gFgCompactOverlayPackPath[24];

static const struct TFgPilotReadGroup kFishing3HighReadGroups12[] = {
    {223, 234},
    {234, 246},
    {345, 354}
};

static const struct TFgPilotReadGroup kFishing3LowReadGroups12[] = {
    {159, 171},
    {163, 175}
};

static const struct TFgPilotReadGroup kVisitor3HighReadGroups12[] = {
    {72, 84},
    {230, 242}
};

static void fgApplySceneRelativeOffsets(struct TFgPilotHeader *header,
                                        struct TFgPilotEntryTable *table)
{
    uint16 i;

    if (header == NULL ||
        table == NULL ||
        table->entries == NULL ||
        (header->reserved0 & kFgPilotHeaderFlagSceneRelative) == 0)
        return;

    for (i = 0; i < table->count; i++) {
        table->entries[i].x = (sint16)(table->entries[i].x + islandState.xPos);
        table->entries[i].y = (sint16)(table->entries[i].y + islandState.yPos);
    }
    header->reserved0 = (uint16)(header->reserved0 & ~kFgPilotHeaderFlagSceneRelative);
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

static int fgSceneRoutePrefixMatches(const char *sceneName,
                                     const char *prefix,
                                     const char **digitsOut)
{
    while (*prefix != '\0') {
        if (*sceneName != *prefix)
            return 0;
        sceneName++;
        prefix++;
    }
    if (*sceneName < '0' || *sceneName > '9')
        return 0;
    *digitsOut = sceneName;
    return 1;
}

static int fgParseCompactOverlayScene(const char *sceneName,
                                      const struct TFgPilotSceneFamily **familyOut,
                                      uint16 *tagOut)
{
    uint16 i;

    if (sceneName == NULL)
        return 0;

    for (i = 0; i < (uint16)(sizeof(kFgPilotSceneFamilies) /
                             sizeof(kFgPilotSceneFamilies[0])); i++) {
        const char *digits = NULL;
        uint16 tag = 0;
        if (!fgSceneRoutePrefixMatches(sceneName,
                                       kFgPilotSceneFamilies[i].scenePrefix,
                                       &digits))
            continue;
        while (*digits >= '0' && *digits <= '9') {
            tag = (uint16)((tag * 10u) + (uint16)(*digits - '0'));
            digits++;
        }
        if (*digits != '\0' || tag == 0 || tag >= 32)
            return 0;
        if ((kFgPilotSceneFamilies[i].tagMask & FG_TAG_MASK(tag)) == 0)
            return 0;
        if (familyOut != NULL)
            *familyOut = &kFgPilotSceneFamilies[i];
        if (tagOut != NULL)
            *tagOut = tag;
        return 1;
    }

    return 0;
}

static char *fgAppendText(char *dst, const char *src)
{
    while (*src != '\0') {
        *dst = *src;
        dst++;
        src++;
    }
    return dst;
}

static char *fgAppendTag(char *dst, uint16 tag)
{
    if (tag >= 10) {
        *dst = (char)('0' + (tag / 10));
        dst++;
    }
    *dst = (char)('0' + (tag % 10));
    dst++;
    return dst;
}

static const char *fgBuildCompactOverlayPackPath(const struct TFgPilotSceneFamily *family,
                                                 uint16 tag,
                                                 int lowTide)
{
    char *dst = gFgCompactOverlayPackPath;
    char *baseStart;
    const char *prefix;
    int baseLen;

    *dst++ = 'F';
    *dst++ = 'G';
    *dst++ = '\\';
    baseStart = dst;
    prefix = lowTide ? family->lowPackPrefix : family->highPackPrefix;
    dst = fgAppendText(dst, prefix);
    dst = fgAppendTag(dst, tag);
    if (lowTide) {
        baseLen = (int)(dst - baseStart);
        if (baseLen + 3 <= 8)
            dst = fgAppendText(dst, "LOW");
        else
            dst = fgAppendText(dst, "L");
    }
    dst = fgAppendText(dst, ".FG2");
    *dst = '\0';
    return gFgCompactOverlayPackPath;
}

static const char *fgCompactOverlayPackPathForScene(const char *sceneName)
{
    const struct TFgPilotSceneFamily *family = NULL;
    uint16 tag = 0;

    if (!fgParseCompactOverlayScene(sceneName, &family, &tag))
        return NULL;

    return fgBuildCompactOverlayPackPath(family, tag, islandState.lowTide);
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

static void fgParseHeader(const uint8 *data, struct TFgPilotHeader *out)
{
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

static int fgHeaderIsPal4TemporalResidual(const struct TFgPilotHeader *header)
{
    return (header != NULL &&
            memcmp(header->magic, "FGP3", 4) == 0 &&
            header->version == 1) ? 1 : 0;
}

static int fgHeaderIsIndexed8TemporalResidual(const struct TFgPilotHeader *header)
{
    return (header != NULL &&
            memcmp(header->magic, "FGP3", 4) == 0 &&
            header->version == 2) ? 1 : 0;
}

static uint32 fgPaletteByteCount(const struct TFgPilotHeader *header)
{
    if (fgHeaderIsPal4Spans(header) || fgHeaderIsPal4TemporalResidual(header))
        return 32;
    if (fgHeaderIsIndexed8Spans(header) || fgHeaderIsIndexed8TemporalResidual(header))
        return 512;
    return 0;
}

static int fgParseEntryTable(const uint8 *data, const struct TFgPilotHeader *header,
                             struct TFgPilotEntryTable *out)
{
    if (!data || !header || !out || header->frameCount == 0)
        return 0;

    memset(out, 0, sizeof(*out));
    out->entries = (struct TFgPilotEntry *)malloc((size_t)header->frameCount * sizeof(struct TFgPilotEntry));
    if (out->entries == NULL)
        return 0;

    out->count = header->frameCount;
    for (uint16 i = 0; i < header->frameCount; i++) {
        const uint8 *src = data + ((uint32)i * FG_PACK_ENTRY_SIZE);
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

static int fgLoadMetadataPrefix(const char *path, struct TFgPilotHeader *outHeader,
                                uint16 *outPalette, struct TFgPilotEntryTable *outTable)
{
    uint8 *metadata;
    uint32 prefixBytes = FG_PACK_METADATA_PREFIX_BYTES;
    uint32 paletteBytes;
    uint32 tableSize;
    uint32 metadataBytes;
    uint16 paletteEntries;
    uint16 i;

    if (!path || !outHeader || !outPalette || !outTable)
        return 0;

    memset(outTable, 0, sizeof(*outTable));
    for (i = 0; i < 256; i++)
        outPalette[i] = 0;

    metadata = ps1_streamRead(path, 0, prefixBytes);
    if (!metadata)
        return 0;

    fgParseHeader(metadata, outHeader);
    if ((!fgHeaderIsPal4Spans(outHeader) &&
         !fgHeaderIsIndexed8Spans(outHeader) &&
         !fgHeaderIsPal4TemporalResidual(outHeader) &&
         !fgHeaderIsIndexed8TemporalResidual(outHeader)) ||
        outHeader->frameCount == 0) {
        free(metadata);
        return 0;
    }

    paletteBytes = fgPaletteByteCount(outHeader);
    tableSize = (uint32)outHeader->frameCount * FG_PACK_ENTRY_SIZE;
    metadataBytes = outHeader->tableOffset + tableSize;
    if (metadataBytes < FG_PACK_HEADER_SIZE + paletteBytes ||
        metadataBytes > outHeader->dataOffset) {
        free(metadata);
        return 0;
    }

    if (metadataBytes > prefixBytes) {
        uint8 *expanded;
        uint8 *tail;
        uint32 tailBytes = metadataBytes - prefixBytes;

        expanded = (uint8 *)malloc(metadataBytes);
        if (!expanded) {
            free(metadata);
            return 0;
        }
        memcpy(expanded, metadata, prefixBytes);
        free(metadata);

        tail = ps1_streamRead(path, prefixBytes, tailBytes);
        if (!tail) {
            free(expanded);
            return 0;
        }
        memcpy(expanded + prefixBytes, tail, tailBytes);
        free(tail);
        metadata = expanded;
    }

    paletteEntries = (uint16)(paletteBytes / 2u);
    for (i = 0; i < paletteEntries; i++)
        outPalette[i] = fgReadU16(metadata + FG_PACK_HEADER_SIZE + ((uint32)i * 2u));

    if (!fgParseEntryTable(metadata + outHeader->tableOffset, outHeader, outTable)) {
        free(metadata);
        return 0;
    }

    free(metadata);
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
}

static void fgInitVisiblePipeline(void)
{
    grUpdateDelay = 0;
}

#if FG_ENABLE_LEGACY_DIAGNOSTIC_SCENES
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
        printf("FG search %s\n", cdPath);
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
        printf("FG read %s\n", cdPath);
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
        printf("FG raw %s %u\n", cdPath, (unsigned int)rawSize);
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
#endif

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
static uint8 *gFgSetupSegmentBuffer = NULL;

static void fgReleaseStreamBuffers(void)
{
    /* Grow-only buffers — DO NOT free here. The free+malloc cycle is
     * what fragmented the heap and starved scene N+1 around minute 11
     * of an organic session. The size fields stay set, so
     * foregroundPilotRuntimeStart's "capacity sufficient?" check skips
     * the alloc on every subsequent scene unless a bigger pack comes
     * along. Buffers grow over the first few scenes, then stabilize.
     *
     *   gFgFrameBuffer           — grow-only (per-frame payload)
     *   gFgPrefetchFrameBuffer   — grow-only (prefetch payload)
     *   gFgStreamWindowBuffer    — grow-only (prefetch read window)
     *   gFgStreamScratch         — grow-only (alignment scratch)
     *
     * gFgSetupSegmentBuffer is fishing3-specific and small; we still
     * cycle it because it's zero-cost and makes the failure path
     * easier to read. */
    if (gFgSetupSegmentBuffer != NULL) {
        free(gFgSetupSegmentBuffer);
        gFgSetupSegmentBuffer = NULL;
    }
}

unsigned long fgGetFrameBufferBytes(void)
{
    return (unsigned long)gFgFrameBufferSize;
}

unsigned long fgGetPrefetchFrameBufferBytes(void)
{
    return (unsigned long)gFgPrefetchFrameBufferSize;
}

int fgPrePrimeStreamBuffers(unsigned long frameMaxBytes,
                            unsigned long scratchMaxBytes)
{
    extern int printf(const char *, ...);
    int ok = 1;

    if (frameMaxBytes > 0 && gFgFrameBuffer == NULL) {
        gFgFrameBuffer = (uint8 *)malloc((size_t)frameMaxBytes);
        if (gFgFrameBuffer == NULL) {
            printf("JCSTREAM frameBuffer prealloc failed (need %lu)\n",
                   frameMaxBytes);
            ok = 0;
        } else {
            gFgFrameBufferSize = (uint32)frameMaxBytes;
        }
    }

    if (scratchMaxBytes > 0 && gFgStreamScratch == NULL) {
        gFgStreamScratch = (uint8 *)malloc((size_t)scratchMaxBytes);
        if (gFgStreamScratch == NULL) {
            printf("JCSTREAM streamScratch prealloc failed (need %lu)\n",
                   scratchMaxBytes);
            ok = 0;
        } else {
            gFgStreamScratchSize = (uint32)scratchMaxBytes;
        }
    }

    return ok;
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

#if FG_HEAP_PROBE_LOGS
static void fgHeapProbe(const char *phase, const char *sceneName)
{
    unsigned long largest;

    if (!gFgHeapProbeEnabled)
        return;

    largest = fgProbeLargestAlloc();
    printf("FH p=%s s=%s l=%lu f=%lu p=%lu w=%lu s=%lu r=%d b=%lu\n",
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
#else
#define fgHeapProbe(phase, sceneName) ((void)0)
#endif

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
    gFgRuntime.streamWindowReadSize = 0;
    gFgRuntime.streamWindowStart = 0;
    gFgRuntime.streamWindowBytes = 0;
    gFgRuntime.setupPrimeWindowBytes = 0;
    gFgRuntime.setupSegmentBuffer = NULL;
    gFgRuntime.setupSegmentStart = 0;
    gFgRuntime.setupSegmentBytes = 0;
    gFgRuntime.streamReadGroups = NULL;
    gFgRuntime.streamReadGroupCount = 0;
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

    if (holidayById(islandState.holiday))
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
        printf("JCRECT 2-rect lower=(%d,%d,%u,%u) upper=(%d,%d,%u,%u) heapKB=%lu\n",
               (int)xs[0], (int)ys[0], (unsigned)ws[0], (unsigned)hs[0],
               (int)xs[1], (int)ys[1], (unsigned)ws[1], (unsigned)hs[1],
               fgProbeLargestAlloc() / 1024UL);
        int rc = grSaveCleanBgRects(xs, ys, ws, hs, 2);
        printf("JCRECT 2-rect grSaveCleanBgRects=%d (need 2)\n", rc);
        return rc == 2;
    } else {
        sint16 xs[1]; sint16 ys[1]; uint16 ws[1]; uint16 hs[1];
        if (lowerEndX <= lowerMinX || lowerEndY <= lowerMinY)
            return 0;
        xs[0] = lowerMinX;
        ys[0] = lowerMinY;
        ws[0] = (uint16)(lowerEndX - lowerMinX);
        hs[0] = (uint16)(lowerEndY - lowerMinY);
        printf("JCRECT 1-rect lower=(%d,%d,%u,%u) heapKB=%lu\n",
               (int)xs[0], (int)ys[0], (unsigned)ws[0], (unsigned)hs[0],
               fgProbeLargestAlloc() / 1024UL);
        int rc = grSaveCleanBgRects(xs, ys, ws, hs, 1);
        printf("JCRECT 1-rect grSaveCleanBgRects=%d (need 1)\n", rc);
        return rc == 1;
    }
}

static void fgBackdropStampHoliday(void)
{
    static int cachedHolidayId = -2;
    static const struct Holiday *cachedHoliday = NULL;
    int holidayId = islandState.holiday;
    const struct Holiday *holiday;

    if (holidayId <= 0)
        return;

    if (cachedHolidayId != holidayId) {
        cachedHoliday = holidayById(holidayId);
        cachedHolidayId = holidayId;
    }
    holiday = cachedHoliday;
    if (holiday == NULL)
        return;
    if (gFgBackdropSlot.numSprites[2] == 0)
        return;

    grDx = islandState.xPos;
    grDy = islandState.yPos;
    grDrawSprite(grBackgroundSfc, &gFgBackdropSlot,
                 holiday->island_x, holiday->island_y,
                 (uint16)holiday->sprite_index, 2);
}

static int fgRuntimeCanStageNextFrame(void)
{
    return gFgPrefetchStage1Enabled &&
           gFgRuntime.active &&
           gFgRuntime.mode == FG_RUNTIME_SCENE_PACK &&
           gFgRuntime.packCdFileValid &&
           gFgRuntime.streamScratch != NULL &&
           gFgRuntime.prefetchFrameBuffer != NULL;
}

static uint32 fgRuntimeGroupedAppendTargetEnd(uint32 appendStart,
                                              uint32 windowStart,
                                              uint32 targetEnd)
{
    uint16 startSector;
    uint8 i;

    if (gFgRuntime.streamReadGroupCount == 0 ||
        (appendStart & 2047UL) != 0)
        return targetEnd;

    startSector = (uint16)(appendStart >> 11);
    for (i = 0; i < gFgRuntime.streamReadGroupCount; i++) {
        if (gFgRuntime.streamReadGroups[i].startSector == startSector) {
            uint32 candidateEnd = ((uint32)gFgRuntime.streamReadGroups[i].endSector) << 11;
            if (candidateEnd > targetEnd &&
                candidateEnd - windowStart <= gFgRuntime.streamWindowSize)
                return candidateEnd;
            return targetEnd;
        }
    }

    return targetEnd;
}

static int fgRuntimeEntryFitsWindow(const struct TFgPilotEntry *entry)
{
    uint32 windowStart;
    uint32 offsetInWindow;
    uint32 readBytes;

    if (gFgPrefetchWindowBytes == 0 ||
        !gFgRuntime.active ||
        gFgRuntime.mode != FG_RUNTIME_SCENE_PACK ||
        !gFgRuntime.packCdFileValid ||
        gFgRuntime.streamWindowBuffer == NULL ||
        gFgRuntime.streamWindowSize == 0 ||
        !fgEntryHasPayload(entry))
        return 0;

    windowStart = fgSectorAlignDown(entry->dataOffset);
    offsetInWindow = entry->dataOffset - windowStart;
    readBytes = fgRuntimeWindowReadSize();
    return (offsetInWindow + entry->dataSize <= readBytes) ? 1 : 0;
}

static int fgRuntimeWindowContainsEntry(const struct TFgPilotEntry *entry)
{
    uint32 entryEnd;
    uint32 windowEnd;

    entryEnd = entry->dataOffset + entry->dataSize;

    if (gFgRuntime.streamWindowValid) {
        windowEnd = gFgRuntime.streamWindowStart + gFgRuntime.streamWindowBytes;
        if (entry->dataOffset >= gFgRuntime.streamWindowStart &&
            entryEnd <= windowEnd)
            return 1;
    }

    return (gFgRuntime.setupSegmentPrimed &&
            entry->dataOffset >= gFgRuntime.setupSegmentStart &&
            entryEnd <= gFgRuntime.setupSegmentStart +
                gFgRuntime.setupSegmentBytes) ? 1 : 0;
}

static int fgRuntimeCopyEntryFromWindow(const struct TFgPilotEntry *entry,
                                        uint8 *dst,
                                        uint8 countsAsDueHit)
{
    uint32 entryEnd;
    uint32 offsetInWindow;

    entryEnd = entry->dataOffset + entry->dataSize;

    if (gFgRuntime.setupSegmentPrimed &&
        entry->dataOffset >= gFgRuntime.setupSegmentStart &&
        entryEnd <= gFgRuntime.setupSegmentStart + gFgRuntime.setupSegmentBytes) {
        offsetInWindow = entry->dataOffset - gFgRuntime.setupSegmentStart;
        memcpy(dst, gFgRuntime.setupSegmentBuffer + offsetInWindow, entry->dataSize);
        gFgRuntime.setupSegmentPrimed = 0;
    } else {
        uint32 windowEnd;
        if (!gFgRuntime.streamWindowValid)
            return 0;
        windowEnd = gFgRuntime.streamWindowStart + gFgRuntime.streamWindowBytes;
        if (entry->dataOffset < gFgRuntime.streamWindowStart ||
            entryEnd > windowEnd)
            return 0;
        offsetInWindow = entry->dataOffset - gFgRuntime.streamWindowStart;
        memcpy(dst, gFgRuntime.streamWindowBuffer + offsetInWindow, entry->dataSize);
    }
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
    targetEnd = fgRuntimeGroupedAppendTargetEnd(currentEnd, windowStart, targetEnd);
    readBytes = targetEnd - windowStart;

    if (windowStart < currentStart ||
        windowStart > currentEnd ||
        targetEnd <= currentEnd)
        return 0;

    /* Append reads write whole sectors, so only extend fully aligned windows. */
    if ((currentEnd & 2047UL) != 0 || (targetEnd & 2047UL) != 0)
        return 0;

    preserveOffset = windowStart - currentStart;
    preserveBytes = currentEnd - windowStart;
    appendBytes = targetEnd - currentEnd;
    if (preserveBytes + appendBytes > gFgRuntime.streamWindowSize)
        return 0;

    stageTick = fgReadTickCounter();
    if (countAsPrefetch && ps1PerfEnabled)
        ps1PerfBeginPrefetchRead(slackVBlanks);

    if (preserveOffset > 0 && preserveBytes > 0)
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

    if (!fgRuntimeEntryFitsWindow(entry))
        return 0;

    windowStart = fgSectorAlignDown(entry->dataOffset);
    readBytes = fgRuntimeWindowReadSize();
    if (windowStart >= (uint32)gFgRuntime.packCdFile.size)
        return 0;
    readEnd = windowStart + readBytes;
    if (readEnd > (uint32)gFgRuntime.packCdFile.size)
        readBytes = (uint32)gFgRuntime.packCdFile.size - windowStart;

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

static const struct TFgPilotEntry *fgRuntimeNextPayloadEntry(void)
{
    uint16 frameIndex;

    frameIndex = gFgRuntime.stagedFrameValid
        ? (uint16)(gFgRuntime.stagedFrameIndex + 1)
        : (uint16)(gFgRuntime.frameIndex + 1);

    while (frameIndex < gFgRuntime.header.frameCount) {
        const struct TFgPilotEntry *entry =
            fgGetEntryFromTable(&gFgRuntime.entryTable, frameIndex);
        if (fgEntryHasPayload(entry)) {
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

    gFgRuntime.setupSegmentPrimed = 0;
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

static uint32 fgRuntimePackPayloadEndBytes(void)
{
    uint16 i;
    uint32 payloadEnd = 0;

    for (i = 0; i < gFgRuntime.entryTable.count; i++) {
        const struct TFgPilotEntry *entry = &gFgRuntime.entryTable.entries[i];
        if (entry->dataSize > 0) {
            uint32 entryEnd = entry->dataOffset + entry->dataSize;
            if (entryEnd > payloadEnd)
                payloadEnd = entryEnd;
        }
    }

    if (gFgRuntime.header.soundEventsOffset != 0) {
        uint32 soundEnd = gFgRuntime.header.soundEventsOffset +
            ((uint32)gFgRuntime.header.soundEventCount * 4UL);
        if (soundEnd > payloadEnd)
            payloadEnd = soundEnd;
    }

    return payloadEnd;
}

static uint32 fgRuntimeStreamWindowBytes(const char *sceneName,
                                         uint32 requestedWindowBytes)
{
    uint8 i;

    if (requestedWindowBytes != FG_PREFETCH_DEFAULT_WINDOW_BYTES)
        return requestedWindowBytes;

    for (i = 0;
         i < sizeof(kFgRuntimeWindowPolicies) / sizeof(kFgRuntimeWindowPolicies[0]);
         i++) {
        if (kFgRuntimeWindowPolicies[i].requiredPackFormat != 0 &&
            kFgRuntimeWindowPolicies[i].requiredPackFormat != gFgRuntime.packFormat)
            continue;
        if (fgSceneEquals(sceneName, kFgRuntimeWindowPolicies[i].sceneName))
            return islandState.lowTide ?
                kFgRuntimeWindowPolicies[i].lowWindowBytes :
                kFgRuntimeWindowPolicies[i].highWindowBytes;
    }

    return requestedWindowBytes;
}

static uint32 fgRuntimeSetupPrimeWindowBytes(const char *sceneName,
                                             uint32 normalWindowBytes)
{
    uint8 i;
    uint32 requested = 0;

    if (gFgRuntime.packFormat == kFgPilotPackFormatIndexed8TemporalResidual &&
        fgSceneEquals(sceneName, "walkstuf1")) {
        requested = (normalWindowBytes << 2) + FG_WALKSTUF1_SETUP_PRIME_BASE_BYTES -
            (islandState.lowTide ? 0 : FG_WALKSTUF1_HIGH_SETUP_PRIME_TRIM_BYTES);
        return requested > FG_SETUP_PRIME_MAX_RESIDENT_BYTES ?
            FG_SETUP_PRIME_MAX_RESIDENT_BYTES : requested;
    }

    if (normalWindowBytes != FG_PREFETCH_DEFAULT_WINDOW_BYTES)
        return 0;

    if (fgRuntimeUsesTemporalResidual()) {
        uint32 payloadEnd = fgRuntimePackPayloadEndBytes();
        uint32 windowStart = fgSectorAlignDown(gFgRuntime.header.dataOffset);
        if (payloadEnd > windowStart) {
            uint32 windowBytes = fgSectorAlignUp(payloadEnd - windowStart);
            if (windowBytes <= FG_SETUP_PRIME_AUTO_PACK_BYTES) {
                requested = windowBytes;
                return requested > FG_SETUP_PRIME_MAX_RESIDENT_BYTES ?
                    FG_SETUP_PRIME_MAX_RESIDENT_BYTES : requested;
            }
        }
    }

    /* Scene-specific until generated prime budgets exist for all variants. */
    for (i = 0;
         i < sizeof(kFgRuntimeSetupPrimePolicies) / sizeof(kFgRuntimeSetupPrimePolicies[0]);
         i++) {
        if (fgSceneEquals(sceneName, kFgRuntimeSetupPrimePolicies[i].sceneName)) {
            requested = islandState.lowTide ?
                kFgRuntimeSetupPrimePolicies[i].lowWindowBytes :
                kFgRuntimeSetupPrimePolicies[i].highWindowBytes;
            return requested > FG_SETUP_PRIME_MAX_RESIDENT_BYTES ?
                FG_SETUP_PRIME_MAX_RESIDENT_BYTES : requested;
        }
    }
    return 0;
}

static int fgRuntimePrimeSetupWindow(void)
{
    uint32 windowStart;
    uint32 windowBytes;
    uint32 fileBytes;
    uint32 validBytes;

    if (!gFgRuntime.packCdFileValid ||
        gFgRuntime.setupPrimeWindowBytes == 0 ||
        gFgRuntime.streamWindowBuffer == NULL ||
        gFgRuntime.streamWindowSize < gFgRuntime.setupPrimeWindowBytes)
        return 1;

    windowStart = (gFgRuntime.header.dataOffset / FG_CD_SECTOR_SIZE) * FG_CD_SECTOR_SIZE;
    fileBytes = gFgRuntime.packCdFile.size;
    if (windowStart >= fileBytes)
        return 1;

    validBytes = fileBytes - windowStart;
    if (validBytes > gFgRuntime.setupPrimeWindowBytes)
        validBytes = gFgRuntime.setupPrimeWindowBytes;
    windowBytes = ((validBytes + FG_CD_SECTOR_SIZE - 1u) / FG_CD_SECTOR_SIZE) *
        FG_CD_SECTOR_SIZE;
    if (windowBytes > gFgRuntime.streamWindowSize)
        return 1;

    if (!ps1_streamReadAlignedIntoFile(&gFgRuntime.packCdFile,
                                       windowStart,
                                       windowBytes,
                                       gFgRuntime.streamWindowBuffer)) {
        gFgRuntime.streamWindowValid = 0;
        return 0;
    }

    gFgRuntime.streamWindowStart = windowStart;
    gFgRuntime.streamWindowBytes = windowBytes;
    gFgRuntime.streamWindowValid = 1;
    gFgRuntime.setupWindowPrimed = 1;
    return 1;
}

static int fgRuntimePrimeSetupSegment(const char *sceneName)
{
    uint32 segmentStart;
    uint32 segmentBytes;
    uint8 *segmentBuffer;

    if (!fgSceneEquals(sceneName, "fishing3"))
        return 1;

    if (islandState.lowTide) {
        segmentStart = FG_FISHING3_LOW_SETUP_SEGMENT_START;
        segmentBytes = FG_FISHING3_LOW_SETUP_SEGMENT_BYTES;
        if (gFgSetupSegmentBuffer == NULL) {
            gFgSetupSegmentBuffer = (uint8 *)malloc(FG_FISHING3_LOW_SETUP_SEGMENT_BYTES);
            if (gFgSetupSegmentBuffer == NULL) {
                if (ps1PerfEnabled)
                    ps1PerfMarkAllocFail(FG_FISHING3_LOW_SETUP_SEGMENT_BYTES);
                return 0;
            }
        }
        segmentBuffer = gFgSetupSegmentBuffer;
    } else {
        segmentStart = FG_FISHING3_HIGH_SETUP_SEGMENT_START;
        segmentBytes = FG_FISHING3_HIGH_SETUP_SEGMENT_BYTES;
        if (gFgRuntime.streamScratch == NULL ||
            gFgRuntime.streamScratchSize < segmentBytes)
            return 1;
        segmentBuffer = gFgRuntime.streamScratch;
    }

    if (!ps1_streamReadAlignedIntoFile(&gFgRuntime.packCdFile,
                                       segmentStart,
                                       segmentBytes,
                                       segmentBuffer)) {
        gFgRuntime.setupSegmentPrimed = 0;
        return 0;
    }

    gFgRuntime.setupSegmentBuffer = segmentBuffer;
    gFgRuntime.setupSegmentStart = segmentStart;
    gFgRuntime.setupSegmentBytes = segmentBytes;
    gFgRuntime.setupSegmentPrimed = 1;
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
            if (slackVBlanks == FG_PREFETCH_WINDOW_MIN_SLACK_VBLANKS &&
                entry->dataSize <= FG_PREFETCH_DIRECT_STAGE_MAX_BYTES) {
                stageTick = fgReadTickCounter();
                if (ps1PerfEnabled)
                    ps1PerfBeginPrefetchRead(slackVBlanks);
                gFgRuntime.setupSegmentPrimed = 0;
                ok = ps1_streamReadIntoFileBuffered(&gFgRuntime.packCdFile,
                                                    entry->dataOffset,
                                                    entry->dataSize,
                                                    gFgRuntime.prefetchFrameBuffer,
                                                    gFgRuntime.streamScratch,
                                                    gFgRuntime.streamScratchSize);
                elapsedVBlanks = (uint16)ps1PerfElapsedVBlanks(stageTick);
                if (ps1PerfEnabled)
                    ps1PerfEndPrefetchRead(elapsedVBlanks, entry->dataSize, ok);
                *outElapsedVBlanks = elapsedVBlanks;

                if (!ok) {
                    if (ps1PerfEnabled)
                        ps1PerfMarkTripwire();
                    gFgRuntime.active = 0;
                    return 1;
                }

                fgRuntimeSetStagedFrame(nextFrameIndex, entry);
                {
                    uint32 windowStart = fgSectorAlignDown(entry->dataOffset);
                    uint32 windowBytes =
                        fgSectorAlignUp(entry->dataOffset + entry->dataSize) -
                        windowStart;
                    memcpy(gFgRuntime.streamWindowBuffer,
                           gFgRuntime.streamScratch,
                           windowBytes);
                    gFgRuntime.streamWindowStart = windowStart;
                    gFgRuntime.streamWindowBytes = windowBytes;
                    gFgRuntime.streamWindowValid = 1;
                }
                return 1;
            }
            if (!fgRuntimeFillWindowForEntry(entry, slackVBlanks, 1, &elapsedVBlanks)) {
                if (ps1PerfEnabled)
                    ps1PerfMarkTripwire();
                gFgRuntime.active = 0;
                return 1;
            }
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
    gFgRuntime.setupSegmentPrimed = 0;
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
    uint16 slackVBlanks;
    const struct TFgPilotEntry *entry;

    entry = fgRuntimeNextPayloadEntry();
    if (!fgRuntimeEntryFitsWindow(entry))
        return 0;

    if (fgRuntimeWindowContainsEntry(entry)) {
        if (ps1PerfEnabled)
            ps1PerfMarkPrefetchDuplicate();
        return 0;
    }

    slackVBlanks = fgRuntimeHeldSlackBeforeWait();
    if (slackVBlanks == 0) {
        if (ps1PerfEnabled) {
            ps1PerfMarkPrefetchAttempt(slackVBlanks, slackVBlanks, 0);
            ps1PerfMarkPrefetchSkipNoSlack();
        }
        return 0;
    }

    if (!fgRuntimeWindowSlackEligible(slackVBlanks)) {
        if (ps1PerfEnabled) {
            ps1PerfMarkPrefetchAttempt(slackVBlanks, slackVBlanks, 0);
            ps1PerfMarkPrefetchSkipNoSlack();
        }
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
    const struct TFgPilotEntry *entry;

    entry = fgRuntimeNextPayloadEntry();
    if (!fgRuntimeEntryFitsWindow(entry))
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
            gFgRuntime.setupSegmentPrimed = 0;
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

static void fgRuntimeComposeEntryToBackground(const struct TFgPilotEntry *entry,
                                              uint8 *frameData)
{
    if (gFgRuntime.packFormat == kFgPilotPackFormatPal4Spans) {
        grCompositePacked4SpansToBackground(frameData,
                                            entry->dataSize,
                                            gFgRuntime.palette,
                                            entry->x,
                                            entry->y);
    } else if (gFgRuntime.packFormat == kFgPilotPackFormatPal4TemporalResidual) {
        grCompositePacked4TemporalResidualToBackground(frameData,
                                                       entry->dataSize,
                                                       gFgRuntime.palette,
                                                       entry->x,
                                                       entry->y);
    } else if (gFgRuntime.packFormat == kFgPilotPackFormatIndexed8TemporalResidual) {
        grCompositeIndexed8TemporalResidualToBackground(frameData,
                                                        entry->dataSize,
                                                        gFgRuntime.palette,
                                                        entry->x,
                                                        entry->y);
    } else if (gFgRuntime.packFormat == kFgPilotPackFormatIndexed8Spans) {
        grCompositeIndexed8SpansToBackground(frameData,
                                             entry->dataSize,
                                             gFgRuntime.palette,
                                             entry->x,
                                             entry->y);
    }

    /* Stamp holiday overlay on top of the pack so Johnny walks behind
     * the holiday decoration, matching islandInitHoliday's z-order. */
    fgBackdropStampHoliday();
}

static int fgRuntimePrepareStagedFrameForPresent(uint16 *outElapsedVBlanks,
                                                 int perfDetail)
{
    uint32 prepTick;
    uint32 perfTick = 0;
    uint16 elapsedVBlanks;

    prepTick = fgReadTickCounter();

    if (perfDetail)
        ps1PerfBeginPipeline(PS1_PERF_PIPE_PREPARE);

    if (perfDetail)
        perfTick = ps1PerfTick();
    if (fgRuntimeUsesTemporalResidual())
        grBeginResidualCleanBgFrame();
    else
        grRestoreBgFromRects();
    if (perfDetail)
        ps1PerfMarkRenderPhase(PS1_PERF_RENDER_RESTORE,
                               ps1PerfElapsedVBlanks(perfTick));

    if (perfDetail)
        perfTick = ps1PerfTick();
    fgRuntimeComposeEntryToBackground(&gFgRuntime.stagedEntry,
                                      gFgRuntime.prefetchFrameBuffer);
    if (perfDetail)
        ps1PerfMarkRenderPhase(PS1_PERF_RENDER_COMPOSE,
                               ps1PerfElapsedVBlanks(perfTick));

    elapsedVBlanks = (uint16)ps1PerfElapsedVBlanks(prepTick);
    *outElapsedVBlanks = elapsedVBlanks;
    if (perfDetail)
        ps1PerfEndPipeline(PS1_PERF_PIPE_PREPARE, elapsedVBlanks);

    gFgRuntime.preparedFrameValid = 1;
    gFgRuntime.preparedFrameIndex = gFgRuntime.stagedFrameIndex;
    if (ps1PerfEnabled)
        ps1PerfMarkScheduler(PS1_PERF_SCHED_PREPARED_READY, 0);
    return 1;
}

static int fgRuntimePresentPreparedFrame(int perfDetail)
{
    uint32 perfRenderTick = 0;
    uint32 perfTick = 0;
    uint16 preparedFrameIndex;

    preparedFrameIndex = gFgRuntime.preparedFrameIndex;
    if (ps1PerfEnabled)
        ps1PerfMarkRenderedLoop();
    if (perfDetail)
        ps1PerfBeginPipeline(PS1_PERF_PIPE_PREPARED_PRESENT);
    if (perfDetail)
        perfRenderTick = ps1PerfTick();

    if (perfDetail)
        perfTick = ps1PerfTick();
    VSync(0);
    if (perfDetail)
        ps1PerfMarkRenderPhase(PS1_PERF_RENDER_PRESENT_WAIT,
                               ps1PerfElapsedVBlanks(perfTick));

    foregroundPilotRuntimeAdvance();
    if (!gFgRuntime.active || gFgRuntime.frameIndex != preparedFrameIndex) {
        if (ps1PerfEnabled)
            ps1PerfMarkTripwire();
        gFgRuntime.preparedFrameValid = 0;
        if (perfDetail)
            ps1PerfEndPipeline(PS1_PERF_PIPE_PREPARED_PRESENT,
                               ps1PerfElapsedVBlanks(perfRenderTick));
        return 1;
    }

    if (perfDetail)
        perfTick = ps1PerfTick();
    grDrawBackground();
    if (perfDetail)
        ps1PerfMarkRenderPhase(PS1_PERF_RENDER_UPLOAD,
                               ps1PerfElapsedVBlanks(perfTick));

    if (perfDetail)
        perfTick = ps1PerfTick();
    eventsWaitTick(0);
    if (perfDetail)
        ps1PerfMarkRenderPhase(PS1_PERF_RENDER_EVENT_WAIT,
                               ps1PerfElapsedVBlanks(perfTick));

    if (perfDetail) {
        uint16 renderElapsed = ps1PerfElapsedVBlanks(perfRenderTick);
        ps1PerfMarkRenderTotal(renderElapsed);
        ps1PerfEndPipeline(PS1_PERF_PIPE_PREPARED_PRESENT, renderElapsed);
    }
    gFgRuntime.preparedFrameValid = 0;
    if (ps1PerfEnabled)
        ps1PerfMarkScheduler(PS1_PERF_SCHED_PREPARED_USED, 0);
    fgRuntimeMarkFrameRendered();
    return 1;
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

        x = entry->x;
        y = entry->y;
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

static int foregroundPilotRuntimeStart(const char *sceneName)
{
    fgRuntimeReset();

    if (sceneName == NULL)
        return 0;

#if FG_ENABLE_LEGACY_DIAGNOSTIC_SCENES
    if (fgSceneEquals(sceneName, "testcard")) {
        gFgRuntime.active = 1;
        gFgRuntime.mode = FG_RUNTIME_TESTCARD;
        strncpy(gFgRuntime.sceneName, sceneName, sizeof(gFgRuntime.sceneName) - 1);
        gFgRuntime.holdFrames = kFgPilotProbeHoldFrames;
        gFgRuntime.sceneClockTick = fgReadTickCounter();
        fgTelemetryUpdate();
        return 1;
    }
#endif

    {
        const char *path = fgCompactOverlayPackPathForScene(sceneName);
        if (path != NULL) {
            const struct TFgPilotSceneFamily *sceneFamily = NULL;
            uint16 sceneTag = 0;
            uint32 maxDataSize = 0;
            uint16 packFlags;
            uint16 i;
            /* Trigger closed-caption lookup only when captions are active.
             * Normal playback keeps captions off, so avoid scene-name checks
             * and ADS caption lookup on the default path. */
            if (ps1CaptionsEnabled &&
                fgParseCompactOverlayScene(sceneName, &sceneFamily, &sceneTag))
                captionsOnAdsStart(sceneFamily->adsName, sceneTag);
            if (!fgLoadMetadataPrefix(path,
                                      &gFgRuntime.header,
                                      gFgRuntime.palette,
                                      &gFgRuntime.entryTable))
                return 0;
            packFlags = gFgRuntime.header.reserved0;
            if (fgHeaderIsIndexed8Spans(&gFgRuntime.header))
                gFgRuntime.packFormat = kFgPilotPackFormatIndexed8Spans;
            else if (fgHeaderIsPal4TemporalResidual(&gFgRuntime.header))
                gFgRuntime.packFormat = kFgPilotPackFormatPal4TemporalResidual;
            else if (fgHeaderIsIndexed8TemporalResidual(&gFgRuntime.header))
                gFgRuntime.packFormat = kFgPilotPackFormatIndexed8TemporalResidual;
            else
                gFgRuntime.packFormat = kFgPilotPackFormatPal4Spans;
            if ((gFgRuntime.header.reserved0 & kFgPilotHeaderFlagBaseDiff) == 0) {
                printf("FG not base-diff %s\n", path);
                fgRuntimeReset();
                return 0;
            }
            fgApplySceneRelativeOffsets(&gFgRuntime.header,
                                        &gFgRuntime.entryTable);
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
            if (gFgPrefetchStage1Enabled) {
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
            if (gFgPrefetchWindowBytes > 0) {
                uint32 windowBytes = ((gFgPrefetchWindowBytes + 2047u) / 2048u) * 2048u;
                uint32 windowCapacityBytes = windowBytes;
                windowBytes = fgRuntimeStreamWindowBytes(sceneName, windowBytes);
                windowCapacityBytes = windowBytes;
                gFgRuntime.setupPrimeWindowBytes =
                    fgRuntimeSetupPrimeWindowBytes(sceneName, windowBytes);
                if (gFgRuntime.setupPrimeWindowBytes > 0 &&
                    windowCapacityBytes < gFgRuntime.setupPrimeWindowBytes)
                    windowCapacityBytes = gFgRuntime.setupPrimeWindowBytes;
                if (windowBytes == FG_PREFETCH_DEFAULT_WINDOW_BYTES &&
                    fgSceneEquals(sceneName, "fishing3")) {
                    if (islandState.lowTide) {
                        gFgRuntime.streamReadGroups = kFishing3LowReadGroups12;
                        gFgRuntime.streamReadGroupCount =
                            (uint8)(sizeof(kFishing3LowReadGroups12) /
                                    sizeof(kFishing3LowReadGroups12[0]));
                    } else {
                        gFgRuntime.streamReadGroups = kFishing3HighReadGroups12;
                        gFgRuntime.streamReadGroupCount =
                            (uint8)(sizeof(kFishing3HighReadGroups12) /
                                    sizeof(kFishing3HighReadGroups12[0]));
                    }
                } else if (windowBytes == FG_PREFETCH_DEFAULT_WINDOW_BYTES &&
                           !islandState.lowTide &&
                           fgSceneEquals(sceneName, "visitor3")) {
                    gFgRuntime.streamReadGroups = kVisitor3HighReadGroups12;
                    gFgRuntime.streamReadGroupCount =
                        (uint8)(sizeof(kVisitor3HighReadGroups12) /
                                sizeof(kVisitor3HighReadGroups12[0]));
                } else {
                    gFgRuntime.streamReadGroups = NULL;
                    gFgRuntime.streamReadGroupCount = 0;
                }
                if (gFgRuntime.streamReadGroupCount > 0 &&
                    windowCapacityBytes < FG_PREFETCH_GROUP_WINDOW_BYTES)
                    windowCapacityBytes = FG_PREFETCH_GROUP_WINDOW_BYTES;
                if (windowCapacityBytes > gFgStreamWindowBufferSize) {
                    uint8 *newWindowBuffer;

                    /* Setup-prime is a startup cache, not scene state.
                     * If the larger cache allocation ever fails, keep the
                     * scene deterministic by falling back to the normal
                     * streaming window. The small window remains mandatory:
                     * if that allocation fails, the scene really cannot run. */
                    newWindowBuffer = (uint8 *)malloc(windowCapacityBytes);
                    if (newWindowBuffer == NULL &&
                        gFgRuntime.setupPrimeWindowBytes > 0 &&
                        windowCapacityBytes > windowBytes) {
                        gFgRuntime.setupPrimeWindowBytes = 0;
                        windowCapacityBytes = windowBytes;
                        if (windowCapacityBytes > gFgStreamWindowBufferSize)
                            newWindowBuffer = (uint8 *)malloc(windowCapacityBytes);
                    }
                    if (windowCapacityBytes > gFgStreamWindowBufferSize) {
                        if (newWindowBuffer == NULL) {
                            if (ps1PerfEnabled)
                                ps1PerfMarkAllocFail(windowCapacityBytes);
                            gFgStreamWindowBufferSize = 0;
                            fgRuntimeReset();
                            return 0;
                        }
                        if (gFgStreamWindowBuffer != NULL)
                            free(gFgStreamWindowBuffer);
                        gFgStreamWindowBuffer = newWindowBuffer;
                        gFgStreamWindowBufferSize = windowCapacityBytes;
                    } else if (newWindowBuffer != NULL) {
                        free(newWindowBuffer);
                    }
                }
                gFgRuntime.streamWindowReadSize = windowBytes;
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
            if (!fgRuntimePrimeSetupWindow()) {
                fgRuntimeReset();
                return 0;
            }
            if (!fgRuntimePrimeSetupSegment(sceneName)) {
                fgRuntimeReset();
                return 0;
            }
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
                                   packFlags,
                                   gFgRuntime.packFormat,
                                   gFgFrameBufferSize,
                                   gFgStreamScratchSize);
                if (gFgPrefetchStage1Enabled &&
                    gFgPrefetchWindowBytes > 0)
                    ps1PerfSetPrefetchPolicy(PS1_PERF_PREFETCH_STAGE1_WINDOW,
                                             gFgPrefetchFrameBufferSize +
                                             gFgStreamWindowBufferSize);
                else if (gFgPrefetchStage1Enabled)
                    ps1PerfSetPrefetchPolicy(PS1_PERF_PREFETCH_STAGE1,
                                             gFgPrefetchFrameBufferSize);
                else if (gFgPrefetchWindowBytes > 0)
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
                if (!fgEntryHasPayload(&gFgRuntime.currentEntry) &&
                    gFgRuntime.stagedFrameValid &&
                    gFgRuntime.stagedFrameIndex == 1) {
                    /* Frame 0 is a non-payload capture artifact; consume it
                     * before loop_start, then settle once so CD cadence stays
                     * aligned with the active-loop prefetch schedule. */
                    gFgRuntime.presentedVBlanks = (uint16)(gFgRuntime.presentedVBlanks +
                                                           gFgRuntime.displayVBlanks);
                    gFgRuntime.frameIndex = 1;
                    if (!fgRuntimeConsumeStagedFrame(gFgRuntime.frameIndex)) {
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
                    fgRuntimeWaitHeldVBlank();
                }
                if (ps1PerfEnabled)
                    ps1PerfMarkSetupPhase(PS1_PERF_SETUP_FIRST_FRAME,
                                          ps1PerfElapsedVBlanks(perfFirstFrameTick));
            }
            fgTelemetryUpdate();
            return 1;
        }
    }

    return 0;
}

/* Walk-time suppression. grUpdateDisplay calls runtimeCompose every frame,
 * which composes the previous FG2 scene's frame data (incl. baked-in Johnny)
 * onto bg. During walks that races with walk_pilot's own per-frame composite
 * and yields two Johnnies. walk_pilot sets this on for the duration of the
 * walk loop (incl. hold frames). */
static int gFgComposeSuppressed = 0;

void foregroundPilotSuppressCompose(int suppressed)
{
    gFgComposeSuppressed = suppressed ? 1 : 0;
}

void foregroundPilotRuntimeCompose(void)
{
    if (gFgComposeSuppressed)
        return;
#if FG_ENABLE_LEGACY_DIAGNOSTIC_SCENES
    const uint16 rectW = 120;
    const uint16 rectH = 80;
#endif

#if FG_ENABLE_LEGACY_DIAGNOSTIC_SCENES
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
#endif

    if (gFgRuntime.mode == FG_RUNTIME_SCENE_PACK)
        fgRuntimeComposeEntryToBackground(&gFgRuntime.currentEntry,
                                          gFgRuntime.currentFrameData);
}

void foregroundPilotRuntimeAdvance(void)
{
    uint32 nowTick;
    uint32 elapsedTicks;
    uint16 elapsedVBlanks;

    if (!gFgRuntime.active)
        return;

    nowTick = fgReadTickCounter();
    elapsedTicks = (nowTick >= gFgRuntime.sceneClockTick)
        ? (nowTick - gFgRuntime.sceneClockTick)
        : 0;
    gFgRuntime.sceneClockTick = nowTick;
    elapsedVBlanks = (uint16)elapsedTicks;
    if (elapsedVBlanks == 0)
        elapsedVBlanks = 1;
    if (ps1PerfEnabled)
        ps1PerfMarkAdvance(elapsedVBlanks);

#if FG_ENABLE_LEGACY_DIAGNOSTIC_SCENES
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
#endif

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

        {
            uint16 presentedAdvance = frameHoldVBlanks;
            uint16 catchupThreshold = gFgRuntime.setupWindowPrimed ? 4 : 5;
            /* Long host-deadline holds can absorb one late VBlank without
             * skipping frames; broader catch-up starves FG2 prefetch. */
            if (frameHoldVBlanks >= catchupThreshold &&
                gFgRuntime.frameVBlank > frameHoldVBlanks)
                presentedAdvance = (uint16)(frameHoldVBlanks + 1);
            gFgRuntime.presentedVBlanks = (uint16)(gFgRuntime.presentedVBlanks +
                                                   presentedAdvance);
        }
        gFgRuntime.frameVBlank = 0;
        gFgRuntime.frameIndex++;
        if (!fgRuntimeLoadSceneFrame(gFgRuntime.frameIndex))
            gFgRuntime.active = 0;
        fgTelemetryUpdate();
    }
}

int foregroundPilotRuntimeActive(void)
{
    return gFgRuntime.active;
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

void foregroundPilotRuntimeEnd(void)
{
    fgRuntimeReset();
}

#if FG_ENABLE_LEGACY_DIAGNOSTIC_SCENES
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
#endif

static void fgPlayOceanRuntimeScene(const char *sceneName)
{
    sint16 fgBoundsX = 0;
    sint16 fgBoundsY = 0;
    uint16 fgBoundsW = 0;
    uint16 fgBoundsH = 0;
    uint32 perfPhaseTick = 0;
    int perfDetail = ps1PerfEnabled ? ps1PerfDetailEnabled() : 0;

    fgHeapProbe("before_scene", sceneName);
    /* Clean-rect snapshots are tied to the current backdrop contents. Deactivate
     * (don't free) so the boot-prealloc'd buffers stay at their fixed addresses
     * across scenes — eliminates the per-scene fragmentation that built up to a
     * 200 KB lower-rect alloc failure around minute 11 of a free-running session. */
    grDeactivateCleanBgRects();
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
        grDeactivateCleanBgRects();
        fgBackdropRelease(0);
        fgHeapProbe("start_failed_cleanup", sceneName);
        /* On a deterministic test build any pack-start failure is a
         * code/data bug we want surfaced loudly. The BSOD does not
         * return — caller must reset to recover. */
        JC_BSOD(sceneName, "foregroundPilotRuntimeStart returned 0 "
                          "(pack header / streaming buffer alloc failed)");
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
                                    &fgBoundsW, &fgBoundsH)) {
        fgRuntimeReset();
        fgReleaseStreamBuffers();
        grDeactivateCleanBgRects();
        fgBackdropRelease(0);
        fgHeapProbe("draw_bounds_failed_cleanup", sceneName);
        JC_BSOD(sceneName, "fgRuntimeComputeDrawBounds returned 0 "
                          "(scene metadata missing or pack header malformed)");
    }
    if (!fgBackdropSaveCleanBgRectsForPack(fgBoundsX, fgBoundsY,
                                           fgBoundsW, fgBoundsH)) {
        fgRuntimeReset();
        fgReleaseStreamBuffers();
        grDeactivateCleanBgRects();
        fgBackdropRelease(0);
        fgHeapProbe("clean_rect_failed_cleanup", sceneName);
        JC_BSOD(sceneName, "fgBackdropSaveCleanBgRectsForPack returned 0 "
                          "(clean-rect alloc failed — heap fragmented?)");
    }
    if (ps1PerfEnabled)
        ps1PerfMarkSetupPhase(PS1_PERF_SETUP_CLEAN_RECT,
                              ps1PerfElapsedVBlanks(perfPhaseTick));
    fgHeapProbe("after_clean_rect_save", sceneName);

    /* Stamp the holiday emblem into bgTile NOW so it's part of the
     * pristine baseline the walk-area buffer captures. Without this the
     * captured pixels lack holiday — walk_pilot does re-stamp each frame,
     * but the order (restore → composite Johnny → stamp holiday) means
     * the previous pose's holiday region briefly flashes during scene
     * transitions. With holiday baked in, restore alone shows it. */
    fgBackdropStampHoliday();

    /* Capture the pristine walk-area pixels into walk_pilot's persistent
     * buffer, gated on islandState change. bgTile here is ocean + island
     * sprites + raft + holiday — the same baseline a follow-up walk
     * needs to wipe its previous pose against. The function is a no-op
     * when the state key matches the last capture, so it's cheap to
     * call every scene setup. */
    walkPilotCaptureCleanWalkAreaIfStale(islandState.raft,
                                         islandState.lowTide,
                                         islandState.night,
                                         islandState.holiday,
                                         islandState.xPos,
                                         islandState.yPos);

    /* Force a full-tile framebuffer upload on the FIRST scene-frame
     * upload. Without this, scene N+1's first grDrawBackground only
     * uploads its dirty-row union, so any framebuffer pixels left from
     * walk_pilot's last frame (e.g. walk Johnny's feet at y > 330 when
     * grDy is high) stay on screen until something else dirties them.
     * Showed up as "feet residue at bottom-left tile" after a
     * scene→walk→scene transition. */
    grForceFullRedrawNextFrame();

    if (ps1PerfEnabled)
        ps1PerfMarkLoopStart();
    while (foregroundPilotRuntimeActive()) {
        int advancedThisLoop = 0;

        /* Pause-menu request: break out so jc_reborn's outer loop can
         * advance to next scene or restart the loop. The flag is
         * cleared by the consumer in jc_reborn.c. */
        if (pauseMenuRequestNextScene || pauseMenuRequestResetLoop) {
            break;
        }
        if (fgRuntimeCanHoldDisplayedFrame()) {
            uint16 prefetchElapsedVBlanks = 0;
            uint16 prepareElapsedVBlanks = 0;
            uint16 heldSlackVBlanks = 0;
            uint8 schedOwner = PS1_PERF_SCHED_WAIT;
            int didPrefetch = 0;
            int didPrepare = 0;
            if (ps1PerfEnabled) {
                ps1PerfMarkHeldLoop();
                heldSlackVBlanks = fgRuntimeHeldSlackBeforeWait();
            }
            if (fgRuntimeCanPresentPreparedOnNextVBlank()) {
                advancedThisLoop = fgRuntimePresentPreparedFrame(perfDetail);
                if (advancedThisLoop)
                    schedOwner = PS1_PERF_SCHED_PRESENT;
            } else if (gFgRuntime.preparedFrameValid ||
                       gFgRuntime.stagedFrameValid) {
                int windowWouldRead = fgRuntimeWindowPrefetchWouldRead();
                if (windowWouldRead && ps1PerfEnabled) {
                    ps1PerfMarkScheduler(PS1_PERF_SCHED_CD_RESERVED,
                                         heldSlackVBlanks);
                    if (!gFgRuntime.preparedFrameValid &&
                        fgRuntimeCanPrepareStagedFrame()) {
                        ps1PerfMarkScheduler(PS1_PERF_SCHED_PREP_BLOCKED_CD,
                                             heldSlackVBlanks);
                    }
                }
                didPrefetch = windowWouldRead
                    ? fgRuntimeTryPrefetchWindow(&prefetchElapsedVBlanks)
                    : 0;
                if (didPrefetch)
                    schedOwner = PS1_PERF_SCHED_CD_WINDOW;
            } else {
                didPrefetch = fgRuntimeTryStageNextFrame(&prefetchElapsedVBlanks);
                if (didPrefetch)
                    schedOwner = PS1_PERF_SCHED_CD_STAGE;
                if (didPrefetch &&
                    prefetchElapsedVBlanks == 0 &&
                    gFgRuntime.stagedFrameValid &&
                    fgRuntimeHeldSlackBeforeWait() >= FG_PREFETCH_FALLTHROUGH_MIN_SLACK_VBLANKS &&
                    fgRuntimeWindowPrefetchWouldRead()) {
                    uint16 windowElapsedVBlanks = 0;
                    if (fgRuntimeTryPrefetchWindow(&windowElapsedVBlanks)) {
                        prefetchElapsedVBlanks = windowElapsedVBlanks;
                        schedOwner = PS1_PERF_SCHED_CD_WINDOW;
                    }
                }
            }
            if (!advancedThisLoop &&
                !didPrefetch &&
                !gFgRuntime.stagedFrameValid) {
                didPrefetch = fgRuntimeTryPrefetchWindow(&prefetchElapsedVBlanks);
                if (didPrefetch)
                    schedOwner = PS1_PERF_SCHED_CD_WINDOW;
            }
            if (!advancedThisLoop &&
                !didPrefetch &&
                fgRuntimeCanPrepareStagedFrame()) {
                if (fgRuntimeWindowPrefetchWouldRead()) {
                    if (ps1PerfEnabled)
                        ps1PerfMarkScheduler(PS1_PERF_SCHED_PREP_BLOCKED_CD,
                                             heldSlackVBlanks);
                } else {
                    didPrepare = fgRuntimePrepareStagedFrameForPresent(&prepareElapsedVBlanks,
                                                                       perfDetail);
                    if (didPrepare)
                        schedOwner = PS1_PERF_SCHED_VISUAL_PREPARE;
                }
            }
            if (ps1PerfEnabled)
                ps1PerfMarkScheduler(schedOwner, heldSlackVBlanks);
            if (!advancedThisLoop) {
                if (didPrepare) {
                    if (prepareElapsedVBlanks == 0)
                        fgRuntimeWaitHeldVBlank();
                    else
                        eventsWaitTick(0);
                } else if (didPrefetch) {
                    if (prefetchElapsedVBlanks == 0)
                        fgRuntimeWaitHeldVBlank();
                    else
                        eventsWaitTick(0);
                } else {
                    fgRuntimeWaitHeldVBlank();
                }
            }
        } else {
            uint32 perfRenderTick = 0;
            uint32 perfDetailTick = 0;
            if (gFgRuntime.preparedFrameValid &&
                gFgRuntime.preparedFrameIndex == gFgRuntime.frameIndex) {
                if (ps1PerfEnabled)
                    ps1PerfMarkScheduler(PS1_PERF_SCHED_PREPARED_WASTED, 0);
                gFgRuntime.preparedFrameValid = 0;
            }
            if (ps1PerfEnabled)
                ps1PerfMarkRenderedLoop();
            if (perfDetail)
                ps1PerfBeginPipeline(PS1_PERF_PIPE_DUE_RENDER);
            if (perfDetail)
                perfRenderTick = ps1PerfTick();
            if (perfDetail)
                perfDetailTick = ps1PerfTick();
            if (fgRuntimeUsesTemporalResidual()) {
                if (gFgRuntime.frameRendered)
                    grBeginResidualCleanBgFrame();
                else
                    grBeginResidualCleanBgFirstFrame();
            } else {
                grRestoreBgFromRects();
            }
            if (perfDetail)
                ps1PerfMarkRenderPhase(PS1_PERF_RENDER_RESTORE,
                                       ps1PerfElapsedVBlanks(perfDetailTick));
            grUpdateDisplay(NULL, NULL, NULL);
            if (perfDetail) {
                uint16 renderElapsed = ps1PerfElapsedVBlanks(perfRenderTick);
                ps1PerfMarkRenderTotal(renderElapsed);
                ps1PerfEndPipeline(PS1_PERF_PIPE_DUE_RENDER, renderElapsed);
            }
            fgRuntimeMarkFrameRendered();
        }
        if (!advancedThisLoop) {
            if (perfDetail)
                perfPhaseTick = ps1PerfTick();
            foregroundPilotRuntimeAdvance();
            if (perfDetail)
                ps1PerfMarkRenderPhase(PS1_PERF_RENDER_ADVANCE,
                                       ps1PerfElapsedVBlanks(perfPhaseTick));
        }
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
    /* Deactivate the rect-snapshot — keep the buffer alive at its boot-prealloc
     * address so we don't fragment the heap across hundreds of scene cycles. */
    grDeactivateCleanBgRects();
    /* Keep BACKGRND.BMP in slot 0 across scenes; release only
     * variant-dependent overlay slots to avoid needless PSB churn. */
    fgBackdropRelease(1);
    fgHeapProbe("after_scene_cleanup", sceneName);
}

#if FG_ENABLE_LEGACY_DIAGNOSTIC_SCENES
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
#endif

void foregroundPilotSetScene(const char *sceneName)
{
    size_t i;

    if (!sceneName) {
        gForegroundPilotScene[0] = '\0';
        return;
    }

    for (i = 0; i + 1 < sizeof(gForegroundPilotScene) && sceneName[i] != '\0'; i++)
        gForegroundPilotScene[i] = sceneName[i];
    gForegroundPilotScene[i] = '\0';
}

void foregroundPilotSetHeapProbe(int enabled)
{
#if FG_HEAP_PROBE_LOGS
    gFgHeapProbeEnabled = enabled ? 1 : 0;
#else
    (void)enabled;
#endif
}

void foregroundPilotResetPrefetchDefaults(void)
{
    /* Match the file-static default: prefetch is ON. Scenes need it for
     * full-speed playback. See gFgPrefetchStage1Enabled rationale. */
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

void foregroundPilotPlay(void)
{
#if FG_ENABLE_LEGACY_DIAGNOSTIC_SCENES
    if (fgSceneEquals(gForegroundPilotScene, "testcard")) {
        fgPlayTestCard();
        return;
    }
#endif

    if (fgCompactOverlayPackPathForScene(gForegroundPilotScene) != NULL) {
        fgPlayOceanRuntimeScene(gForegroundPilotScene);
        return;
    }

#if FG_ENABLE_LEGACY_DIAGNOSTIC_SCENES
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
#endif

    printf("FG unknown scene %s\n", gForegroundPilotScene);
}

#else

static char gForegroundPilotScene[16] = "";

void foregroundPilotSetScene(const char *sceneName)
{
    if (!sceneName) {
        gForegroundPilotScene[0] = '\0';
        return;
    }
    strncpy(gForegroundPilotScene, sceneName, sizeof(gForegroundPilotScene) - 1);
    gForegroundPilotScene[sizeof(gForegroundPilotScene) - 1] = '\0';
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

unsigned short foregroundPilotRuntimeFrameIndex(void)
{
    return 0;
}

void foregroundPilotPlay(void)
{
    fprintf(stderr, "foreground pilot is PS1-only for now (%s)\n", gForegroundPilotScene);
}

#endif
