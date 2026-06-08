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
#include "mem_region.h"
#include "resource.h"
#include "holidays.h"
#include "pause_menu.h"
#include "ps1_captions.h"
#include "sound_ps1.h"
#include "ps1_spu_cache.h"
#include "utils.h"
#include "ps1_perf.h"
#include "walk_pilot.h"
#include "ps1_debug.h"
#include "scene_freeplay.h"

#ifndef PS1_VERBOSE_DIAGNOSTICS
#define PS1_VERBOSE_DIAGNOSTICS 0
#endif
#ifndef FG_STAGE_DIAG_LOGS
#define FG_STAGE_DIAG_LOGS 0
#endif

#if FG_STAGE_DIAG_LOGS
#define FG_STAGE_PRINTF(...) do { printf(__VA_ARGS__); } while (0)
#else
#define FG_STAGE_PRINTF(...) do { } while (0)
#endif

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
    uint8 *setupSegment2Buffer;
    uint32 setupSegment2Start;
    uint32 setupSegment2Bytes;
    uint8 *setupSegment3Buffer;
    uint32 setupSegment3Start;
    uint32 setupSegment3Bytes;
    uint8 *setupSegment4Buffer;
    uint32 setupSegment4Start;
    uint32 setupSegment4Bytes;
    uint8 setupSegmentReusable;
    const struct TFgPilotReadGroup *streamReadGroups;
    uint8 streamReadGroupCount;
    uint8 streamWindowValid;
    uint8 setupWindowPrimed;
    uint8 setupSegmentPrimed;
    uint8 setupSegment2Primed;
    uint8 setupSegment3Primed;
    uint8 setupSegment4Primed;
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
static char gFgStageSceneName[16] = "";
static sint16 gFgSceneDrawOffsetX = 0;
static sint16 gFgSceneDrawOffsetY = 0;
static uint32 gFgCleanRectMaxBytes = 96UL * 1024UL;
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
static const uint8 kFgPilotPackFormatPal4CompactTemporalResidual = 6;
enum {
    FG_PACK_HEADER_SIZE = 40,
    FG_PACK_ENTRY_SIZE = 20,
    FG_PACK_METADATA_PREFIX_BYTES = 8192
};
#define FG_DELTA_PAYLOAD_SENTINEL 0xfffeu
#define FG_DELTA_PAYLOAD_MAGIC0 0x44u /* 'D' */
#define FG_DELTA_PAYLOAD_MAGIC1 0x34u /* '4' */
#define FG_LOCAL_LZ_PAYLOAD_SENTINEL 0xfffdu
#define FG_LOCAL_LZ_PAYLOAD_MAGIC0 0x4cu /* 'L' */
#define FG_LOCAL_LZ_PAYLOAD_MAGIC1 0x34u /* '4' */
#define FG_PREFETCH_DEFAULT_WINDOW_BYTES (16UL * 1024UL)
#define FG_ACTIVITY9_HIGH_WINDOW_BYTES (32UL * 1024UL)
#define FG_ACTIVITY9_LOW_WINDOW_BYTES (20UL * 1024UL)
#define FG_BUILDING4_HIGH_WINDOW_BYTES (20UL * 1024UL)
#define FG_BUILDING4_LOW_WINDOW_BYTES (24UL * 1024UL)
#define FG_BUILDING6_WINDOW_BYTES (24UL * 1024UL)
#define FG_WALKSTUF1_HIGH_RESIDUAL_WINDOW_BYTES (54UL * 1024UL)
#define FG_WALKSTUF1_LOW_RESIDUAL_WINDOW_BYTES (40UL * 1024UL)
#define FG_WALKSTUF1_SETUP_PRIME_BASE_BYTES (88UL * 1024UL)
#define FG_WALKSTUF1_HIGH_SETUP_PRIME_TRIM_BYTES (4UL * 1024UL)
#define FG_WALKSTUF1_HIGH_SETUP_PRIME_MAX_RESIDENT_BYTES (144UL * 1024UL)
#define FG_WALKSTUF1_LOW_SETUP_PRIME_MAX_RESIDENT_BYTES (160UL * 1024UL)
#define FG_PREFETCH_GROUP_WINDOW_BYTES (24UL * 2048UL)
#define FG_SETUP_PRIME_WINDOW_BYTES (320UL * 1024UL)
#define FG_SETUP_PRIME_MAX_RESIDENT_BYTES (128UL * 1024UL)
#define FG_ACTIVITY12_HIGH_SETUP_PRIME_WINDOW_BYTES (328UL * 1024UL)
#define FG_FISHING2_SETUP_PRIME_WINDOW_BYTES (352UL * 1024UL)
#define FG_FISHING6_HIGH_SETUP_PRIME_WINDOW_BYTES (312UL * 1024UL)
#define FG_FISHING7_HIGH_SETUP_PRIME_WINDOW_BYTES (328UL * 1024UL)
#define FG_JOHNNY3_HIGH_SETUP_PRIME_WINDOW_BYTES (312UL * 1024UL)
#define FG_VISITOR3_HIGH_SETUP_PRIME_WINDOW_BYTES (320UL * 1024UL)
#define FG_VISITOR3_LOW_SETUP_PRIME_WINDOW_BYTES (208UL * 1024UL)
#define FG_VISITOR3_SETUP_PRIME_MAX_RESIDENT_BYTES (320UL * 1024UL)
#define FG_VISITOR1_HIGH_SETUP_PRIME_WINDOW_BYTES (296UL * 1024UL)
#define FG_VISITOR7_HIGH_SETUP_PRIME_WINDOW_BYTES (368UL * 1024UL)
#define FG_SETUP_PRIME_AUTO_PACK_BYTES (288UL * 1024UL)
#define FG_CD_SECTOR_SIZE 2048UL
#define FG_NEXT_STAGE_METADATA_BYTES FG_PACK_METADATA_PREFIX_BYTES
#define FG_NEXT_STAGE_SIDE_BYTES (128UL * 1024UL)
#define FG_NEXT_STAGE_TARGET_BYTES (64UL * 1024UL)
#define FG_NEXT_STAGE_CHUNK_BYTES (8UL * 1024UL)
#define FG_NEXT_STAGE_PAYLOAD_SPU_OFFSET FG_NEXT_STAGE_METADATA_BYTES
#define fgSectorAlignDown(offset) ((uint32)((offset) & ~(FG_CD_SECTOR_SIZE - 1UL)))
#define fgSectorAlignUp(offset) ((uint32)(((offset) + FG_CD_SECTOR_SIZE - 1UL) & ~(FG_CD_SECTOR_SIZE - 1UL)))
#define FG_BUILDING2_HIGH_SETUP_SEGMENT_START (3UL * FG_CD_SECTOR_SIZE)
#define FG_BUILDING2_HIGH_SETUP_SEGMENT_BYTES (32UL * FG_CD_SECTOR_SIZE)
#define FG_BUILDING2_HIGH_SETUP_SEGMENT2_START (202UL * FG_CD_SECTOR_SIZE)
#define FG_BUILDING2_HIGH_SETUP_SEGMENT2_BYTES (40UL * FG_CD_SECTOR_SIZE)
#define FG_BUILDING2_HIGH_PHASE_VBLANKS 1
#define FG_WALKSTUF1_HIGH_PHASE_VBLANKS 4
#define FG_WALKSTUF1_LOW_PHASE_VBLANKS 1
#define FG_VISITOR3_LOW_PHASE_VBLANKS 1
#define FG_BUILDING2_LOW_SETUP_SEGMENT_START (112UL * FG_CD_SECTOR_SIZE)
#define FG_BUILDING2_LOW_SETUP_SEGMENT_BYTES (16UL * FG_CD_SECTOR_SIZE)
#define FG_BUILDING2_LOW_SETUP_SEGMENT2_START (226UL * FG_CD_SECTOR_SIZE)
#define FG_BUILDING2_LOW_SETUP_SEGMENT2_BYTES (36UL * FG_CD_SECTOR_SIZE)
#define FG_BUILDING4_HIGH_SETUP_SEGMENT_START (264UL * FG_CD_SECTOR_SIZE)
#define FG_BUILDING4_HIGH_SETUP_SEGMENT_BYTES (24UL * FG_CD_SECTOR_SIZE)
#define FG_WALKSTUF1_LOW_SETUP_SEGMENT_START (179UL * FG_CD_SECTOR_SIZE)
#define FG_WALKSTUF1_LOW_SETUP_SEGMENT_BYTES (104UL * FG_CD_SECTOR_SIZE)
#define FG_WALKSTUF1_LOW_SETUP_SEGMENT2_START (154UL * FG_CD_SECTOR_SIZE)
#define FG_WALKSTUF1_LOW_SETUP_SEGMENT2_BYTES (6UL * FG_CD_SECTOR_SIZE)
#define FG_WALKSTUF1_HIGH_SETUP_SEGMENT_START (198UL * FG_CD_SECTOR_SIZE)
#define FG_WALKSTUF1_HIGH_SETUP_SEGMENT_BYTES (46UL * FG_CD_SECTOR_SIZE)
#define FG_WALKSTUF1_HIGH_SETUP_SEGMENT2_START (286UL * FG_CD_SECTOR_SIZE)
#define FG_WALKSTUF1_HIGH_SETUP_SEGMENT2_BYTES (58UL * FG_CD_SECTOR_SIZE)
#define FG_WALKSTUF1_HIGH_SETUP_SEGMENT4_START (383UL * FG_CD_SECTOR_SIZE)
#define FG_WALKSTUF1_HIGH_SETUP_SEGMENT4_DISABLED_BYTES (16UL * FG_CD_SECTOR_SIZE)
#define FG_WALKSTUF1_HIGH_SETUP_SEGMENT4_BYTES (0UL * FG_CD_SECTOR_SIZE)
#define FG_FISHING3_HIGH_SETUP_SEGMENT_START (67UL * FG_CD_SECTOR_SIZE)
#define FG_FISHING3_HIGH_SETUP_SEGMENT_BYTES (6UL * FG_CD_SECTOR_SIZE)
#define FG_FISHING3_LOW_SETUP_SEGMENT_START (146UL * FG_CD_SECTOR_SIZE)
#define FG_FISHING3_LOW_SETUP_SEGMENT_BYTES (6UL * FG_CD_SECTOR_SIZE)
#define FG_VISITOR3_HIGH_SETUP_SEGMENT_START (277UL * FG_CD_SECTOR_SIZE)
#define FG_VISITOR3_HIGH_SETUP_SEGMENT_BYTES (16UL * FG_CD_SECTOR_SIZE)
#define FG_VISITOR3_HIGH_SETUP_SEGMENT2_START (203UL * FG_CD_SECTOR_SIZE)
#define FG_VISITOR3_HIGH_SETUP_SEGMENT2_BYTES (59UL * FG_CD_SECTOR_SIZE)
#define FG_VISITOR3_HIGH_SETUP_SEGMENT3_START (48UL * FG_CD_SECTOR_SIZE)
#define FG_VISITOR3_HIGH_SETUP_SEGMENT3_BYTES (7UL * FG_CD_SECTOR_SIZE)
#define FG_VISITOR3_LOW_SETUP_SEGMENT_START (281UL * FG_CD_SECTOR_SIZE)
#define FG_VISITOR3_LOW_SETUP_SEGMENT_BYTES (24UL * FG_CD_SECTOR_SIZE)
#define FG_VISITOR3_LOW_SETUP_SEGMENT2_START (150UL * FG_CD_SECTOR_SIZE)
#define FG_VISITOR3_LOW_SETUP_SEGMENT2_BYTES (27UL * FG_CD_SECTOR_SIZE)
#define FG_VISITOR3_LOW_SETUP_SEGMENT3_START (206UL * FG_CD_SECTOR_SIZE)
#define FG_VISITOR3_LOW_SETUP_SEGMENT3_BYTES (26UL * FG_CD_SECTOR_SIZE)
#define FG_VISITOR3_LOW_SETUP_SEGMENT4_START (38UL * FG_CD_SECTOR_SIZE)
#define FG_VISITOR3_LOW_SETUP_SEGMENT4_BYTES (41UL * FG_CD_SECTOR_SIZE)
#define fgRuntimeWindowReadSize() (gFgRuntime.streamWindowReadSize)
/* Below 3 VBlanks, window refills are more likely to become visible delay. */
#define FG_PREFETCH_WINDOW_MIN_SLACK_VBLANKS 3
/* MARY3 keeps prefetch under clean pressure; 8 VBlanks is the strict-safe
 * window-refill knee that avoids hidden refill debt on both tides. */
#define FG_MARY3_WINDOW_MIN_SLACK_VBLANKS 8
#define FG_BUILDING2_LOW_WINDOW_MIN_SLACK_VBLANKS 5
#define FG_BUILDING6_WINDOW_MIN_SLACK_VBLANKS 4
#define FG_VISITOR3_HIGH_WINDOW_MIN_SLACK_VBLANKS 4
#define FG_VISITOR3_LOW_DUAL_SEGMENT_MIN_SLACK_VBLANKS 4
#define FG_VISITOR3_HIGH_TIGHT_WINDOW_SLACK_VBLANKS 8
#define FG_VISITOR3_HIGH_TIGHT_WINDOW_BYTES (56UL * 1024UL)
#define FG_PREFETCH_FALLTHROUGH_MIN_SLACK_VBLANKS 6
#define FG_PREFETCH_DIRECT_STAGE_MAX_BYTES (8UL * 1024UL)
#define FG_PREPARE_PRESENT_MIN_SLACK_VBLANKS 4
#define FG_LARGE_CLEAN_SNAPSHOT_BYTES (384UL * 1024UL)
/* Round 17: restored to 256 KB after Round 16 (CACHE 1024 KB)
 * gave visitor3 enough headroom via force-relief override
 * (fgSceneForcesCleanMemoryRelief). The 192 KB lowering from
 * Round 15 was over-tight — it caused fishing1-3, johnny4-5,
 * and mary4-5 to skip prefetch (hits=0 due_misses=N) without
 * actually being needed for visitor3 (which uses the
 * scene-specific force-relief path, not the threshold). */
#define FG_CLEAN_SNAPSHOT_PRESSURE_BYTES (256UL * 1024UL)
#define FG_LARGE_FRAME_PAYLOAD_BYTES (128UL * 1024UL)
#define FG_LOW_MEMORY_STREAM_SCRATCH_BYTES (16UL * 1024UL)
#define FG_JOHNNY1_LOCAL_LZ_MAX_EXPANDED_BYTES (112UL * 1024UL)
#define fgEntryHasPayload(entry) \
    (((entry) != NULL && \
      (entry)->dataSize > 0 && \
      (entry)->width > 0 && \
      (entry)->height > 0) ? 1 : 0)
#define fgRuntimeUsesTemporalResidual() \
    ((gFgRuntime.packFormat == kFgPilotPackFormatPal4TemporalResidual || \
      gFgRuntime.packFormat == kFgPilotPackFormatIndexed8TemporalResidual || \
      gFgRuntime.packFormat == kFgPilotPackFormatPal4CompactTemporalResidual) ? 1 : 0)
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
     ((fgRuntimeHeldSlackBeforeWait() == FG_PREPARE_PRESENT_MIN_SLACK_VBLANKS) || \
      (!islandState.lowTide && \
       gFgRuntimeSceneId == FG_SCENE_WALKSTUF1 && \
       fgRuntimeHeldSlackBeforeWait() == 2 && \
       gFgRuntime.frameIndex >= 128 && \
       gFgRuntime.frameIndex <= 191)))
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

enum {
    FG_SCENE_UNKNOWN = 0,
    FG_SCENE_BUILDING2,
    FG_SCENE_BUILDING6,
    FG_SCENE_MARY3,
    FG_SCENE_VISITOR3,
    FG_SCENE_WALKSTUF1
};

static struct TFgPilotRuntime gFgRuntime = {0};
static uint8 gFgRuntimeSceneId = FG_SCENE_UNKNOWN;
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
static uint8 gFgLoadingWaveProofEnabled = 0;
static uint8 gFgSpuStageEnabled = 0;
struct TFgNextSceneStage {
    uint8 active;
    uint8 valid;
    uint8 lowTide;
    uint8 usesStreamWindow;
    uint8 parkedInSpu;
    uint8 metadataParkedInSpu;
    uint8 readInFlight;
    uint8 phase;
    char sceneName[16];
    CdlFILE packCdFile;
    uint32 metadataBytes;
    uint32 spuPayloadOffset;
    uint32 windowStart;
    uint32 windowBytes;
    uint32 loadedBytes;
    uint32 pendingBytes;
};
enum {
    FG_NEXT_STAGE_PHASE_NONE = 0,
    FG_NEXT_STAGE_PHASE_HEADER = 1,
    FG_NEXT_STAGE_PHASE_PAYLOAD = 2
};
struct TFgCleanOverlayKey {
    uint8 valid;
    uint8 lowTide;
    uint8 night;
    sint16 raft;
    sint16 holiday;
    sint16 islandX;
    sint16 islandY;
    sint16 drawX;
    sint16 drawY;
    char sceneName[16];
};
static struct TFgNextSceneStage gFgNextSceneStage;
static struct TFgCleanOverlayKey gFgCleanOverlayKey;
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
static int fgBackdropSaveVisitor3HighCleanBgRects(void);
static void fgBackdropStampHoliday(void);
static void fgBackdropRelease(int keepBackgrnd);
static void fgCleanOverlayInvalidate(void);
static int fgCleanOverlayMatches(const char *sceneName);
static void fgCleanOverlayRemember(const char *sceneName);
static void fgReleaseStreamBuffers(void);
static void fgReleaseStreamBuffersHard(void);
static void fgDropOptionalPrefetchBuffersForCleanSnapshot(void);
static void fgNextSceneStageInvalidate(void);
static int fgNextSceneStageAdoptFile(const char *sceneName, CdlFILE *outFile);
static int fgNextSceneStageReadMetadataPrefix(const char *sceneName,
                                              uint8 *dst,
                                              uint32 bytes);
static int fgNextSceneStageTryTick(const char *sceneName);
static int fgNextSceneStageFinishPending(void);
static int fgNextSceneStageAdoptWindow(const char *sceneName);
static void fgInitVisiblePipeline(void);
static uint8 fgSceneIdForName(const char *sceneName);

/* Public accessor for walk_pilot — returns the slot holding
 * BACKGRND.PSB sprites (used for behind-tree trunk + leaf cover-up
 * during walk transitions). NULL if backdrop isn't loaded yet. */
#include "foreground_pilot/backdrop_public.c.inc"
#include "foreground_pilot/scene_catalog.c.inc"
#include "foreground_pilot/pack_metadata.c.inc"
#include "foreground_pilot/runtime_memory.c.inc"
#include "foreground_pilot/backdrop_clean.c.inc"
#include "foreground_pilot/stream_runtime.c.inc"
#line 4277 "/project/src/foreground_pilot.c"
static int fgCleanRectsNeedCacheForProof(const char *sceneName)
{
    if (!gFgLoadingWaveProofEnabled)
        return 0;
    if (gFgStageSceneName[0] != '\0' &&
        !fgSceneEquals(sceneName, gFgStageSceneName))
        return 0;
    return 1;
}

static int foregroundPilotRuntimeStart(const char *sceneName)
{
    /* Round 33: fgRuntimeReset() previously ran here, BUT that put it
     * AFTER fgPlayOceanRuntimeScene's grLoadScreen call. With bg-tile
     * pixels now living in TRANSIENT (Round 33 migration), memSceneReset
     * would have wiped the just-loaded backdrop bytes. The reset is now
     * hoisted to the very top of fgPlayOceanRuntimeScene, so by the time
     * we get here gFgRuntime is already zeroed and TRANSIENT is fresh.
     *
     * Error-recovery paths below still call fgRuntimeReset() on return-0
     * — that's safe because the caller treats return 0 as a JC_BSOD
     * (fatal halt); the JC_BSOD renderer NULL-checks the bg-tile slots
     * before drawing the panic screen and re-allocates fresh tiles. */
    if (sceneName == NULL)
        return 0;

#if FG_ENABLE_LEGACY_DIAGNOSTIC_SCENES
    if (fgSceneEquals(sceneName, "testcard")) {
        gFgRuntime.active = 1;
        gFgRuntime.mode = FG_RUNTIME_TESTCARD;
        gFgRuntimeSceneId = fgSceneIdForName(sceneName);
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
            uint32 cleanSnapshotEstimate = 0;
            uint8 cleanMemoryRelief = 0;
            uint16 packFlags;
            uint16 i;
            /* Trigger closed-caption lookup only when captions are active.
             * Normal playback keeps captions off, so avoid scene-name checks
             * and ADS caption lookup on the default path. */
            if (ps1CaptionsEnabled &&
                fgParseCompactOverlayScene(sceneName, &sceneFamily, &sceneTag))
                captionsOnAdsStart(sceneFamily->adsName, sceneTag);
            if (!fgNextSceneStageAdoptFile(sceneName, &gFgRuntime.packCdFile) &&
                !ps1_streamResolveFile(path, &gFgRuntime.packCdFile)) {
                fgRuntimeReset();
                return 0;
            }
            gFgRuntime.packCdFileValid = 1;
            if (!fgLoadMetadataPrefixFromCdFile(sceneName,
                                                &gFgRuntime.packCdFile,
                                                &gFgRuntime.header,
                                                gFgRuntime.palette,
                                                &gFgRuntime.entryTable))
                return 0;
            packFlags = gFgRuntime.header.reserved0;
            if (fgHeaderIsIndexed8Spans(&gFgRuntime.header))
                gFgRuntime.packFormat = kFgPilotPackFormatIndexed8Spans;
            else if (fgHeaderIsPal4TemporalResidual(&gFgRuntime.header))
                gFgRuntime.packFormat = (gFgRuntime.header.version == 4) ?
                        kFgPilotPackFormatPal4CompactTemporalResidual :
                        kFgPilotPackFormatPal4TemporalResidual;
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
            if (!fgLoadSoundEvents(&gFgRuntime.packCdFile, &gFgRuntime.header,
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
            if (fgSceneEquals(sceneName, "johnny1") &&
                maxDataSize < 65536UL &&
                maxDataSize < FG_JOHNNY1_LOCAL_LZ_MAX_EXPANDED_BYTES) {
                maxDataSize = FG_JOHNNY1_LOCAL_LZ_MAX_EXPANDED_BYTES;
            }
            cleanSnapshotEstimate = fgHeaderCleanSnapshotEstimate(&gFgRuntime.header);
            cleanMemoryRelief = (uint8)(
                fgSceneNeedsCleanMemoryRelief(sceneName, cleanSnapshotEstimate, maxDataSize)
                || fgSceneForcesCleanMemoryRelief(sceneName));
            if (maxDataSize > gFgFrameBufferSize) {
                if (gFgFrameBuffer != NULL)
                    memFree(MEM_REGION_CACHE, gFgFrameBuffer);
                /* MEM_REGION_RATIONALE: grow-only frame buffer, persistent
                 * across scenes; not LRU-tracked. CACHE region. */
                gFgFrameBuffer = (uint8 *)memAlloc(MEM_REGION_CACHE,
                                                   maxDataSize,
                                                   "fg-frame");
                if (gFgFrameBuffer == NULL) {
                    if (ps1PerfEnabled)
                        ps1PerfMarkAllocFail(maxDataSize);
                    gFgFrameBufferSize = 0;
                    fgRuntimeReset();
                    return 0;
                }
                gFgFrameBufferSize = maxDataSize;
            }
            if (cleanMemoryRelief) {
#if PS1_VERBOSE_DIAGNOSTICS
                printf("JCMEM clean-relief scene=%s clean=%lu maxFrame=%lu no-prefetch\n",
                       sceneName,
                       (unsigned long)cleanSnapshotEstimate,
                       (unsigned long)maxDataSize);
#endif
                fgDropOptionalPrefetchBuffersForCleanSnapshot();
                if (fgSceneForcesCleanMemoryRelief(sceneName)) {
                    /* Round 16: explicit-free of grow-only persistent
                     * buffers for force-relief scenes (visitor3 only).
                     * fgDropOptionalPrefetchBuffersForCleanSnapshot
                     * above only clears runtime mirror fields; the
                     * global CACHE-resident pointers persist across
                     * scene transitions per fgReleaseStreamBuffers'
                     * grow-only design. For visitor3, those bytes
                     * compete with the 2x 97 KB clean-rect snapshot
                     * and trigger the BSOD. Free them explicitly.
                     *
                     * memFree(MEM_REGION_CACHE, ptr) range-checks
                     * the pointer (mem_region.c:379-386), so it
                     * routes correctly whether the buffer was
                     * CACHE-allocated (prefetch, always) or
                     * libc-allocated (window, primary path is
                     * malloc with CACHE fallback at line 3328/3337).
                     *
                     * Cold-boot: both pointers are NULL, the if-NULL
                     * guards skip the free; Part 1's allocation-skip
                     * already prevents them from being allocated.
                     * Mid-session: prior scenes grew the buffers,
                     * this free reclaims them. */
                    if (!fgSceneKeepsStage1UnderCleanMemoryRelief(sceneName) &&
                        gFgPrefetchFrameBuffer != NULL) {
                        memFree(MEM_REGION_CACHE, gFgPrefetchFrameBuffer);
                        gFgPrefetchFrameBuffer = NULL;
                        gFgPrefetchFrameBufferSize = 0;
                    }
                    if (!fgSceneKeepsWindowUnderCleanMemoryRelief(sceneName) &&
                        gFgStreamWindowBuffer != NULL) {
                        memFree(MEM_REGION_CACHE, gFgStreamWindowBuffer);
                        gFgStreamWindowBuffer = NULL;
                        gFgStreamWindowBufferSize = 0;
                    }
                }
            }
            if (gFgPrefetchStage1Enabled &&
                (!cleanMemoryRelief ||
                 fgSceneKeepsStage1UnderCleanMemoryRelief(sceneName))) {
                if (maxDataSize > gFgPrefetchFrameBufferSize) {
                    if (gFgPrefetchFrameBuffer != NULL)
                        memFree(MEM_REGION_CACHE, gFgPrefetchFrameBuffer);
                    /* MEM_REGION_RATIONALE: grow-only prefetch frame
                     * buffer, peer of gFgFrameBuffer. CACHE region. */
                    gFgPrefetchFrameBuffer = (uint8 *)memAlloc(
                        MEM_REGION_CACHE, maxDataSize, "fg-prefetch-frame");
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
                (!cleanMemoryRelief ||
                 fgSceneKeepsWindowUnderCleanMemoryRelief(sceneName))) {
                uint32 windowBytes = ((gFgPrefetchWindowBytes + 2047u) / 2048u) * 2048u;
                uint32 windowCapacityBytes = windowBytes;
                const struct TFgPilotReadGroup *streamReadGroups = NULL;
                uint8 streamReadGroupCount = 0;
                windowBytes = fgRuntimeStreamWindowBytes(sceneName, windowBytes);
                if (cleanMemoryRelief &&
                    fgSceneKeepsWindowUnderCleanMemoryRelief(sceneName)) {
                    uint32 reliefWindowBytes =
                        fgSceneCleanReliefWindowBytes(sceneName);
                    if (reliefWindowBytes > 0)
                        windowBytes = reliefWindowBytes;
                }
                windowCapacityBytes = windowBytes;
                gFgRuntime.setupPrimeWindowBytes =
                    fgRuntimeSetupPrimeWindowBytes(sceneName, windowBytes);
                if (cleanMemoryRelief &&
                    fgSceneKeepsWindowUnderCleanMemoryRelief(sceneName))
                    gFgRuntime.setupPrimeWindowBytes = 0;
                if (gFgRuntime.setupPrimeWindowBytes > 0 &&
                    windowCapacityBytes < gFgRuntime.setupPrimeWindowBytes)
                    windowCapacityBytes = gFgRuntime.setupPrimeWindowBytes;
                if (windowBytes == FG_PREFETCH_DEFAULT_WINDOW_BYTES &&
                    fgSceneEquals(sceneName, "fishing3")) {
                    if (islandState.lowTide) {
                        streamReadGroups = kFishing3LowReadGroups12;
                        streamReadGroupCount =
                            (uint8)(sizeof(kFishing3LowReadGroups12) /
                                    sizeof(kFishing3LowReadGroups12[0]));
                    } else {
                        streamReadGroups = kFishing3HighReadGroups12;
                        streamReadGroupCount =
                            (uint8)(sizeof(kFishing3HighReadGroups12) /
                                    sizeof(kFishing3HighReadGroups12[0]));
                    }
                } else if (windowBytes == FG_PREFETCH_DEFAULT_WINDOW_BYTES &&
                           fgSceneEquals(sceneName, "building2")) {
                    if (islandState.lowTide) {
                        streamReadGroups = kBuilding2LowReadGroups12;
                        streamReadGroupCount =
                            (uint8)(sizeof(kBuilding2LowReadGroups12) /
                                    sizeof(kBuilding2LowReadGroups12[0]));
                    } else {
                        streamReadGroups = kBuilding2HighReadGroups12;
                        streamReadGroupCount =
                            (uint8)(sizeof(kBuilding2HighReadGroups12) /
                                    sizeof(kBuilding2HighReadGroups12[0]));
                    }
                } else if (fgSceneEquals(sceneName, "walkstuf1")) {
                    if (islandState.lowTide) {
                        streamReadGroups = kWalkstuf1LowReadGroups12;
                        streamReadGroupCount =
                            (uint8)(sizeof(kWalkstuf1LowReadGroups12) /
                                    sizeof(kWalkstuf1LowReadGroups12[0]));
                    } else {
                        streamReadGroups = kWalkstuf1HighReadGroups12;
                        streamReadGroupCount =
                            (uint8)(sizeof(kWalkstuf1HighReadGroups12) /
                                    sizeof(kWalkstuf1HighReadGroups12[0]));
                    }
                } else if (islandState.lowTide &&
                           gFgRuntime.packFormat == kFgPilotPackFormatPal4CompactTemporalResidual &&
                           fgSceneEquals(sceneName, "visitor3")) {
                    streamReadGroups = kVisitor3LowReadGroups16;
                    streamReadGroupCount =
                        (uint8)(sizeof(kVisitor3LowReadGroups16) /
                                sizeof(kVisitor3LowReadGroups16[0]));
                } else if (windowBytes == FG_PREFETCH_DEFAULT_WINDOW_BYTES &&
                           gFgRuntime.packFormat == kFgPilotPackFormatPal4CompactTemporalResidual &&
                           fgSceneEquals(sceneName, "visitor5")) {
                    if (islandState.lowTide) {
                        streamReadGroups = kVisitor5LowReadGroups12;
                        streamReadGroupCount =
                            (uint8)(sizeof(kVisitor5LowReadGroups12) /
                                    sizeof(kVisitor5LowReadGroups12[0]));
                    } else {
                        streamReadGroups = kVisitor5HighReadGroups12;
                        streamReadGroupCount =
                            (uint8)(sizeof(kVisitor5HighReadGroups12) /
                                    sizeof(kVisitor5HighReadGroups12[0]));
                    }
                }
                gFgRuntime.streamReadGroups = streamReadGroups;
                gFgRuntime.streamReadGroupCount = streamReadGroupCount;
                if (streamReadGroupCount > 0 &&
                    windowCapacityBytes < FG_PREFETCH_GROUP_WINDOW_BYTES)
                    windowCapacityBytes = FG_PREFETCH_GROUP_WINDOW_BYTES;
                if (windowCapacityBytes > gFgStreamWindowBufferSize) {
                    uint8 *newWindowBuffer;

                    /* Round 33-soak: route stream window through CACHE.
                     *
                     * The original libc-primary path (malloc here, CACHE
                     * fallback only on setup-prime size) was R8-era
                     * intent to preserve CACHE space for LRU. With the
                     * R33 budget retune (LRU 320 KB, CACHE 640 KB,
                     * ~320 KB CACHE headroom after grow-only stream
                     * buffers), CACHE has plenty of space — but libc
                     * post-region-allocation has only ~70 KB headroom
                     * total, so a 100–300 KB stream window malloc
                     * silently returns NULL when libc is tight,
                     * triggering the foregroundPilotRuntimeStart
                     * return-0 → JC_BSOD "pack-start failed" at
                     * visitor6 around 247s in random rotation.
                     *
                     * memAlloc(MEM_REGION_CACHE, ...) halts on
                     * exhaustion (region + libc both) instead of
                     * silently returning NULL — making this path
                     * deterministic. Setup-prime fallback still tries
                     * the smaller window size on alloc failure to
                     * keep visitor3-class behavior. */
                    /* MEM_REGION_RATIONALE: grow-only prefetch window.
                     * CACHE region. */
                    newWindowBuffer = (uint8 *)memAlloc(
                        MEM_REGION_CACHE,
                        windowCapacityBytes,
                        "fg-stream-window");
                    if (newWindowBuffer == NULL &&
                        gFgRuntime.setupPrimeWindowBytes > 0 &&
                        windowCapacityBytes > windowBytes) {
                        gFgRuntime.setupPrimeWindowBytes = 0;
                        windowCapacityBytes = windowBytes;
                        if (windowCapacityBytes > gFgStreamWindowBufferSize) {
                            /* MEM_REGION_RATIONALE: fallback allocation for
                             * same grow-only CACHE prefetch window above. */
                            newWindowBuffer = (uint8 *)memAlloc(
                                MEM_REGION_CACHE,
                                windowCapacityBytes,
                                "fg-stream-window");
                        }
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
                            memFree(MEM_REGION_CACHE, gFgStreamWindowBuffer);
                        gFgStreamWindowBuffer = newWindowBuffer;
                        gFgStreamWindowBufferSize = windowCapacityBytes;
                    } else if (newWindowBuffer != NULL) {
                        memFree(MEM_REGION_CACHE, newWindowBuffer);
                    }
                }
                gFgRuntime.streamWindowReadSize = windowBytes;
            }
            {
                uint32 requiredScratch = ((maxDataSize + 2047u) / 2048u) * 2048u + 2048u;
                if (cleanMemoryRelief &&
                    requiredScratch > FG_LOW_MEMORY_STREAM_SCRATCH_BYTES)
                    requiredScratch = FG_LOW_MEMORY_STREAM_SCRATCH_BYTES;
                if (cleanMemoryRelief &&
                    gFgStreamScratch != NULL &&
                    gFgStreamScratchSize > requiredScratch) {
                    memFree(MEM_REGION_CACHE, gFgStreamScratch);
                    gFgStreamScratch = NULL;
                    gFgStreamScratchSize = 0;
                }
                if (requiredScratch > gFgStreamScratchSize) {
                    if (gFgStreamScratch != NULL)
                        memFree(MEM_REGION_CACHE, gFgStreamScratch);
                    /* MEM_REGION_RATIONALE: grow-only alignment scratch.
                     * CACHE region. */
                    gFgStreamScratch = (uint8 *)memAlloc(MEM_REGION_CACHE,
                                                          requiredScratch,
                                                          "fg-stream-scratch");
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
            gFgRuntime.prefetchFrameBuffer =
                (cleanMemoryRelief &&
                 !fgSceneKeepsStage1UnderCleanMemoryRelief(sceneName)) ?
                NULL : gFgPrefetchFrameBuffer;
            gFgRuntime.prefetchFrameBufferSize =
                (cleanMemoryRelief &&
                 !fgSceneKeepsStage1UnderCleanMemoryRelief(sceneName)) ?
                0 : gFgPrefetchFrameBufferSize;
            gFgRuntime.streamWindowBuffer =
                (cleanMemoryRelief &&
                 !fgSceneKeepsWindowUnderCleanMemoryRelief(sceneName)) ?
                NULL : gFgStreamWindowBuffer;
            gFgRuntime.streamWindowSize =
                (cleanMemoryRelief &&
                 !fgSceneKeepsWindowUnderCleanMemoryRelief(sceneName)) ?
                0 : gFgStreamWindowBufferSize;
            gFgRuntime.streamWindowStart = 0;
            gFgRuntime.streamWindowBytes = 0;
            gFgRuntime.streamWindowValid = 0;
            gFgRuntime.streamScratch = gFgStreamScratch;
            gFgRuntime.streamScratchSize = gFgStreamScratchSize;
            fgNextSceneStageAdoptWindow(sceneName);
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
                    gFgPrefetchWindowBytes > 0 &&
                    gFgRuntime.streamWindowBuffer != NULL)
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
            gFgRuntimeSceneId = fgSceneIdForName(sceneName);
            strncpy(gFgRuntime.sceneName, sceneName, sizeof(gFgRuntime.sceneName) - 1);
            gFgRuntime.displayVBlanks = 1;
            gFgRuntime.holdFrames = 0;
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
        if (gFgRuntime.frameIndex + 1 >= gFgRuntime.header.frameCount) {
            gFgRuntime.frameVBlank = 0;
            gFgRuntime.active = 0;
            fgTelemetryUpdate();
            return;
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

void foregroundPilotTeardownForFreeplay(void)
{
    fgRuntimeReset();
    fgReleaseStreamBuffersHard();
    fgNextSceneStageInvalidate();
    fgCleanOverlayInvalidate();
    grSetFullScreenScrCacheEnabled(0);
    grSetCleanBgRectsForceCache(0);
    fgBackdropRelease(0);
    grReleaseBackgroundTiles();
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
    uint32 cleanRectEstimate = 0;
    uint32 perfPhaseTick = 0;
    int blackBackdrop = fgSceneUsesBlackBackdrop(sceneName);
    const char *sceneBackdropScreen = fgSceneBackdropScreen(sceneName);
    int sceneSpecificBackdrop = sceneBackdropScreen != NULL;
    int largeCleanSnapshot = 0;
    int deferWalkCleanRecapture = 0;
    int perfDetail = ps1PerfEnabled ? ps1PerfDetailEnabled() : 0;
    int keepBackgrndForProof = 0;
    int reuseCleanOverlayForProof = 0;

    grSetFullScreenScrCacheEnabled(gFgLoadingWaveProofEnabled &&
                                   !blackBackdrop &&
                                   !sceneSpecificBackdrop);

    if (!blackBackdrop && !sceneSpecificBackdrop) {
        if (gFgLoadingWaveProofEnabled &&
            gFgBackdropSlot.numSprites[0] > 0 &&
            gFgBackdropSlot.loadedBmpNames[0] != NULL &&
            strcmp(gFgBackdropSlot.loadedBmpNames[0], "BACKGRND.BMP") == 0)
            keepBackgrndForProof = 1;
        if (keepBackgrndForProof && fgCleanOverlayMatches(sceneName))
            reuseCleanOverlayForProof = 1;
    } else {
        fgCleanOverlayInvalidate();
    }

    /* Round 33: hoist fgRuntimeReset() to BEFORE grLoadScreen.
     *
     * Previously the per-scene reset lived inside foregroundPilotRuntimeStart
     * (called below at line ~3833) — but that put the TRANSIENT wipe AFTER
     * grLoadScreen had already populated the bg-tile pixel buffers. Under
     * Round 33's bg-tile-pixels-in-TRANSIENT design, memSceneReset would
     * have erased the just-loaded backdrop in place, producing a black or
     * stale-bytes scene.
     *
     * Resetting here ensures:
     *   1. The previous scene's TRANSIENT bytes (bg-tile pixels, sound
     *      events, entry table, etc.) are reclaimed wholesale.
     *   2. grBackgroundTilesAssumeWiped (inside fgRuntimeReset) NULLs the
     *      static bg-tile slot pointers so the upcoming grLoadScreen
     *      allocates fresh.
     *   3. gFgRuntime is zeroed; foregroundPilotRuntimeStart can rely on
     *      a clean slate without calling fgRuntimeReset itself. */
    fgRuntimeReset();

    /* Round 33-soak: full CACHE drain at scene boundary.
     *
     * Runs ONLY at fgPlayOceanRuntimeScene's top (true scene boundary),
     * not from the generic fgRuntimeReset (which is also called from
     * mid-setup error paths). Three-step drain:
     *   1. lruEvictAllUnpinned — drop every unpinned ADS/TTM/BMP/SCR
     *      resource (full panic-mode eviction across all 4 types).
     *   2. memCacheRewindIfEmpty — if g_cacheUsed dropped to 0,
     *      discard the (now-irrelevant) free-list and rewind
     *      bump_top to base. O(1) defragmentation.
     *
     * If step 2 fails (live bytes remain after eviction), log it and
     * continue — fragmentation will accumulate but the next scene's
     * allocs may still succeed via the existing free-list path.
     *
     * Cost: LRU re-loads its resources from CD per-scene. Bounded by
     * the scene's pack size. CACHE allocator returns to zero-fragmentation
     * state, breaking the 226s soak ceiling. */
    {
        if (reuseCleanOverlayForProof) {
#if PS1_VERBOSE_DIAGNOSTICS
            if (perfDetail) {
                printf("JCSCREEN clean-overlay-ready bytes=%lu\n",
                       grCleanBgRectsBytes());
            }
#endif
        } else {
            grFreeCleanBgRects();
            fgCleanOverlayInvalidate();
        }

        /* R33r: also release JOHNWALK.PSB (~93 KB CACHE-resident,
         * walk_pilot.c). walkPilotEnsureBmp reloads on demand. */
        fgWalkRenderTeardown();

        /* R33s: also release BACKGRND.BMP PSB in gFgBackdropSlot
         * (~93 KB CACHE-resident — the "ghost" allocation that
         * persists across scenes via fgBackdropPreloadBackgrndBmp's
         * keepBackgrnd=1 path). fgBackdropPreloadBackgrndBmp later
         * in fgPlayOceanRuntimeScene's body reloads it from CD
         * (BACKGRND.BMP is small, ~93 KB). With both JOHNWALK and
         * BACKGRND.BMP released here, cacheUsed should reach 0 and
         * the rewind will fire on every scene transition, breaking
         * the long-tail fragmentation accumulation that BSOD'd at
         * 790s in the R33-extended soak. */
        if (keepBackgrndForProof) {
            fgBackdropRelease(1);
#if PS1_VERBOSE_DIAGNOSTICS
            if (perfDetail)
                printf("JCWAVE keep-backgrnd\n");
#endif
        } else {
            fgBackdropRelease(0);
        }

        extern void lruEvictAllUnpinned(void);
        lruEvictAllUnpinned();
    }
    {
        int rewound = memCacheRewindIfEmpty();
        if (!rewound) {
#if PS1_VERBOSE_DIAGNOSTICS
            extern size_t memRegionUsed(unsigned int);
            extern int numAdsResources;
            extern int numTtmResources;
            extern int numBmpResources;
            extern int numScrResources;
            extern struct TAdsResource *adsResources[];
            extern struct TTtmResource *ttmResources[];
            extern struct TBmpResource *bmpResources[];
            extern struct TScrResource *scrResources[];
            int adsLive = 0, ttmLive = 0, bmpLive = 0, scrLive = 0;
            int adsPinned = 0, ttmPinned = 0, bmpPinned = 0, scrPinned = 0;
            size_t adsBytes = 0, ttmBytes = 0, bmpBytes = 0, scrBytes = 0;
            for (int i = 0; i < numAdsResources; i++) {
                if (adsResources[i] && adsResources[i]->uncompressedData) {
                    adsLive++; adsBytes += adsResources[i]->uncompressedSize;
                    if (adsResources[i]->pinCount > 0) adsPinned++;
                }
            }
            for (int i = 0; i < numTtmResources; i++) {
                if (ttmResources[i] && ttmResources[i]->uncompressedData) {
                    ttmLive++; ttmBytes += ttmResources[i]->uncompressedSize;
                    if (ttmResources[i]->pinCount > 0) ttmPinned++;
                }
            }
            for (int i = 0; i < numBmpResources; i++) {
                if (bmpResources[i] && bmpResources[i]->uncompressedData) {
                    bmpLive++; bmpBytes += bmpResources[i]->uncompressedSize;
                    if (bmpResources[i]->pinCount > 0) bmpPinned++;
                }
            }
            for (int i = 0; i < numScrResources; i++) {
                if (scrResources[i] && scrResources[i]->uncompressedData) {
                    scrLive++; scrBytes += scrResources[i]->uncompressedSize;
                    if (scrResources[i]->pinCount > 0) scrPinned++;
                }
            }
            printf("JCMEM CACHE-rewind-skip scene=%s cacheUsed=%lu "
                   "ADS=%d/%d/%luKB TTM=%d/%d/%luKB BMP=%d/%d/%luKB SCR=%d/%d/%luKB\n",
                   sceneName ? sceneName : "(?)",
                   (unsigned long)memRegionUsed((unsigned int)MEM_REGION_CACHE),
                   adsLive, adsPinned, (unsigned long)(adsBytes/1024),
                   ttmLive, ttmPinned, (unsigned long)(ttmBytes/1024),
                   bmpLive, bmpPinned, (unsigned long)(bmpBytes/1024),
                   scrLive, scrPinned, (unsigned long)(scrBytes/1024));
            memDumpCacheStats("JCMEM cache-stats-at-rewind-skip");
#endif
        }
    }

    fgHeapProbe("before_scene", sceneName);
    /* Clean-rect snapshots are tied to the current backdrop contents. The
     * wave proof may reuse the prior same-key snapshot, but only after the
     * full ocean SCR has been restored underneath it. */
    if (!reuseCleanOverlayForProof)
        grDeactivateCleanBgRects();
    grSetCleanBgBlackMode(0);
    if (blackBackdrop || sceneSpecificBackdrop)
        grFreeCleanBgRects();
    fgReleaseStreamBuffers();

    if (sceneSpecificBackdrop)
        fgBackdropRelease(0);

    if (!blackBackdrop && !sceneSpecificBackdrop) {
        /* Pre-load BACKGRND.BMP before any scene setup allocates bg tiles. At
         * this moment the heap is freshest and the ~93 KB PSB stream has room. */
        if (ps1PerfEnabled)
            perfPhaseTick = ps1PerfTick();
        fgBackdropPreloadBackgrndBmp();
        if (ps1PerfEnabled)
            ps1PerfMarkSetupPhase(PS1_PERF_SETUP_BACKDROP,
                                  ps1PerfElapsedVBlanks(perfPhaseTick));
    }

    fgInitVisiblePipeline();
    grSetPresentDuringScreenLoad(0);
    grSetSaveCleanOnScreenLoad(0);
    if (ps1PerfEnabled)
        perfPhaseTick = ps1PerfTick();
    if (blackBackdrop) {
        grInitEmptyBackground();
        grFreeCleanBgTiles();
        grSetCleanBgBlackMode(1);
    } else if (sceneSpecificBackdrop) {
        grLoadScreen((char *)sceneBackdropScreen);
    } else if (islandState.night) {
        /* NIGHT.SCR is the full night-ocean backdrop, no island baked in.
         * The FG2 backdrop helper draws the island sprites on top. */
        grLoadScreen("NIGHT.SCR");
    } else {
        /* OCEAN00.SCR has no island baked in. The island must be drawn through
         * the movable BACKGRND.BMP path so scene-relative FG2 packs can follow
         * randomized island placement. */
        grLoadScreen("OCEAN00.SCR");
    }
    if (reuseCleanOverlayForProof) {
        grRestoreBgRectsFull();
#if PS1_VERBOSE_DIAGNOSTICS
        if (perfDetail) {
            printf("JCSCREEN clean-overlay-apply bytes=%lu\n",
                   grCleanBgRectsBytes());
        }
#endif
    }
    grSetSaveCleanOnScreenLoad(1);
    if (ps1PerfEnabled)
        ps1PerfMarkSetupPhase(PS1_PERF_SETUP_SCREEN,
                              ps1PerfElapsedVBlanks(perfPhaseTick));
    /* FG2 disables grLoadScreen's full clean-tile snapshot and uses a smaller
     * rect-mode backup instead. Keep this as a defensive cleanup in case a
     * prior mode left full-tile clean buffers resident. */
    grFreeCleanBgTiles();
    if (!blackBackdrop && !sceneSpecificBackdrop) {
        if (ps1PerfEnabled)
            perfPhaseTick = ps1PerfTick();
        fgBackdropEnableWaveBackdrop();
        if (ps1PerfEnabled)
            ps1PerfMarkSetupPhase(PS1_PERF_SETUP_BACKDROP,
                                  ps1PerfElapsedVBlanks(perfPhaseTick));
    }
    grSetPresentDuringScreenLoad(1);

    if (ps1PerfEnabled)
        perfPhaseTick = ps1PerfTick();
    if (!foregroundPilotRuntimeStart(sceneName)) {
        /* Phase 2 of mem-region rollout deletes the JCSKIP/graceful-
         * skip pattern: under the deterministic allocator, allocations
         * cannot fail, so this branch should be unreachable in a
         * well-formed build. If it does fire, the failure is a real
         * bug (likely a non-allocator I/O issue — CD read or pack
         * format problem) and halting via JC_BSOD surfaces it for
         * triage instead of papering over it. See plan v9 manifest
         * item #1. */
        JC_BSOD(sceneName, "pack-start failed (non-recoverable I/O or data bug)");
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
        /* Phase 2 manifest item #2: drawBounds is computed from pack
         * data; failure here means the pack file is malformed or
         * mis-versioned. Halt loudly. */
        JC_BSOD(sceneName, "drawBounds failed (pack data invalid)");
    }
    if (!reuseCleanOverlayForProof) {
        cleanRectEstimate = fgBackdropCleanRectEstimateForPack(fgBoundsX,
                                                               fgBoundsY,
                                                               fgBoundsW,
                                                               fgBoundsH);
        if (!fgScenePreservesPrefetchUnderCleanPressure(sceneName) &&
            (cleanRectEstimate >= FG_LARGE_CLEAN_SNAPSHOT_BYTES ||
             fgSceneNeedsCleanMemoryRelief(sceneName, cleanRectEstimate,
                                           gFgRuntime.frameBufferSize))) {
            largeCleanSnapshot = 1;
            /* Wide scenes need the backdrop baseline more than optional
             * prefetch. The pixels stay exact; only the hidden-read cache is
             * sacrificed for enough contiguous heap to save clean rects. */
#if PS1_VERBOSE_DIAGNOSTICS
            printf("JCMEM large-clean scene=%s bytes=%lu drop-prefetch\n",
                   sceneName, (unsigned long)cleanRectEstimate);
#endif
            fgDropPressureCachesForCleanSnapshot(sceneName, cleanRectEstimate);
        }
        fgDropSetupResidencyForCleanSnapshot(sceneName, cleanRectEstimate);
    }
    if (blackBackdrop && fgRuntimeUsesTemporalResidual()) {
#if PS1_VERBOSE_DIAGNOSTICS
        printf("JCMEM black-clean scene=%s skip-clean-rects\n", sceneName);
#endif
        grFreeCleanBgRects();
        grSetCleanBgBlackMode(1);
    } else if (reuseCleanOverlayForProof) {
#if PS1_VERBOSE_DIAGNOSTICS
        if (perfDetail) {
            printf("JCSCREEN clean-overlay-skip-save bytes=%lu\n",
                   grCleanBgRectsBytes());
        }
#endif
    } else {
        int forceCleanCacheForProof = fgCleanRectsNeedCacheForProof(sceneName) &&
            !blackBackdrop &&
            !sceneSpecificBackdrop &&
            !largeCleanSnapshot;
        grSetCleanBgRectsForceCache(forceCleanCacheForProof);
        if (!fgBackdropSaveCleanBgRectsWithPressureFallback(sceneName,
                                                            fgBoundsX,
                                                            fgBoundsY,
                                                            fgBoundsW,
                                                            fgBoundsH,
                                                            cleanRectEstimate,
                                                            &deferWalkCleanRecapture)) {
            /* Phase 2 manifest item #3: clean-rect alloc is now first
             * in TRANSIENT setup order (plan v9 PR11/PR12). The full
             * 250 KB TRANSIENT region is available; if allocation
             * still fails, the budget is wrong — halt for tuning. */
            JC_BSOD(sceneName, "clean-rect alloc failed (TRANSIENT budget shortfall)");
        }
        grSetCleanBgRectsForceCache(0);
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
    if (!blackBackdrop && !sceneSpecificBackdrop)
        fgBackdropStampHoliday();

    /* Capture the pristine walk-area pixels into walk_pilot's persistent
     * buffer, gated on islandState change. bgTile here is ocean + island
     * sprites + raft + holiday — the same baseline a follow-up walk
     * needs to wipe its previous pose against. The function is a no-op
     * when the state key matches the last capture, so it's cheap to
     * call every scene setup. */
    if (!blackBackdrop && !sceneSpecificBackdrop && !deferWalkCleanRecapture) {
        walkPilotCaptureCleanWalkAreaIfStale(islandState.raft,
                                             islandState.lowTide,
                                             islandState.night,
                                             islandState.holiday,
                                             islandState.xPos,
                                             islandState.yPos);
    }
    /* Force a full-tile framebuffer upload on the FIRST scene-frame
     * upload. Without this, scene N+1's first grDrawBackground only
     * uploads its dirty-row union, so any framebuffer pixels left from
     * walk_pilot's last frame (e.g. walk Johnny's feet at y > 330 when
     * grDy is high) stay on screen until something else dirties them.
     * Showed up as "feet residue at bottom-left tile" after a
     * scene→walk→scene transition. */
    grForceFullRedrawNextFrame();

    if (!islandState.lowTide &&
        gFgRuntimeSceneId == FG_SCENE_WALKSTUF1 &&
        gFgRuntime.frameIndex == 0 &&
        gFgRuntime.stagedFrameValid &&
        gFgRuntime.stagedFrameIndex == 1 &&
        !fgRuntimePresentAndConsumeFirstFrameForW1High()) {
        if (ps1PerfEnabled)
            ps1PerfMarkTripwire();
        gFgRuntime.active = 0;
    }

    if (!islandState.lowTide &&
        gFgRuntimeSceneId == FG_SCENE_WALKSTUF1 &&
        gFgRuntime.active &&
        gFgRuntime.frameIndex == 1 &&
        gFgRuntime.stagedFrameValid &&
        gFgRuntime.stagedFrameIndex == 2) {
        grBeginResidualCleanBgFrame();
        grUpdateDisplay(NULL, NULL, NULL);
        gFgRuntime.presentedVBlanks = (uint16)(gFgRuntime.presentedVBlanks +
                                               gFgRuntime.displayVBlanks);
        gFgRuntime.frameIndex = 2;
        gFgRuntime.frameVBlank = 0;
        if (!fgRuntimeConsumeStagedFrame(gFgRuntime.frameIndex) ||
            fgRuntimePrimeNextFrameForSetup() < 0) {
            if (ps1PerfEnabled) ps1PerfMarkTripwire();
            gFgRuntime.active = 0;
        }
        fgRuntimeWaitHeldVBlank();
        fgRuntimeWaitHeldVBlank();
        fgRuntimeWaitHeldVBlank();
        gFgRuntime.sceneClockTick = fgReadTickCounter();
    }

    if (!islandState.lowTide &&
        gFgRuntimeSceneId == FG_SCENE_VISITOR3 &&
        gFgRuntime.frameIndex == 0 &&
        gFgRuntime.stagedFrameValid &&
        gFgRuntime.stagedFrameIndex == 1 &&
        !fgRuntimePresentFirstFrameNoWait()) {
        if (ps1PerfEnabled) ps1PerfMarkTripwire();
        gFgRuntime.active = 0;
    }
    if (!islandState.lowTide &&
        gFgRuntimeSceneId == FG_SCENE_VISITOR3 &&
        gFgRuntime.active &&
        gFgRuntime.frameIndex == 1 &&
        gFgRuntime.stagedFrameValid &&
        gFgRuntime.stagedFrameIndex == 2 &&
        !fgRuntimePresentNextStagedAndAdvance()) {
        if (ps1PerfEnabled) ps1PerfMarkTripwire();
        gFgRuntime.active = 0;
    }
    if (fgSceneEquals(gFgRuntime.sceneName, "johnny6") &&
        gFgRuntime.frameIndex == 0 &&
        gFgRuntime.stagedFrameValid &&
        gFgRuntime.stagedFrameIndex == 1 &&
        !fgRuntimePresentFirstFrameNoWait()) {
        if (ps1PerfEnabled) ps1PerfMarkTripwire();
        gFgRuntime.active = 0;
    }
    if (fgSceneEquals(gFgRuntime.sceneName, "johnny6") &&
        gFgRuntime.active &&
        gFgRuntime.frameIndex == 1 &&
        gFgRuntime.stagedFrameValid &&
        gFgRuntime.stagedFrameIndex == 2 &&
        !fgRuntimePresentNextStagedAndAdvance()) {
        if (ps1PerfEnabled) ps1PerfMarkTripwire();
        gFgRuntime.active = 0;
    }
    if (islandState.lowTide &&
        gFgRuntimeSceneId == FG_SCENE_WALKSTUF1 &&
        gFgRuntime.frameIndex == 0 &&
        gFgRuntime.stagedFrameValid &&
        gFgRuntime.stagedFrameIndex == 1 &&
        !fgRuntimePresentFirstFrameNoWait()) {
        if (ps1PerfEnabled) ps1PerfMarkTripwire();
        gFgRuntime.active = 0;
    }
    if (islandState.lowTide &&
        gFgRuntimeSceneId == FG_SCENE_WALKSTUF1 &&
        gFgRuntime.active &&
        gFgRuntime.frameIndex == 1 &&
        gFgRuntime.stagedFrameValid &&
        gFgRuntime.stagedFrameIndex == 2 &&
        !fgRuntimePresentNextStagedAndAdvance()) {
        if (ps1PerfEnabled) ps1PerfMarkTripwire();
        gFgRuntime.active = 0;
    }
    if (gFgRuntimeSceneId == FG_SCENE_BUILDING2 &&
        !islandState.lowTide &&
        gFgRuntime.frameIndex == 0 &&
        gFgRuntime.stagedFrameValid &&
        gFgRuntime.stagedFrameIndex == 1 &&
        !fgRuntimePresentFirstFrameNoWait()) {
        if (ps1PerfEnabled) ps1PerfMarkTripwire();
        gFgRuntime.active = 0;
    }
    if (gFgRuntimeSceneId == FG_SCENE_BUILDING2 &&
        !islandState.lowTide &&
        gFgRuntime.active &&
        gFgRuntime.frameIndex == 1 &&
        gFgRuntime.stagedFrameValid &&
        gFgRuntime.stagedFrameIndex == 2 &&
        !fgRuntimePresentNextStagedAndAdvance()) {
        if (ps1PerfEnabled) ps1PerfMarkTripwire();
        gFgRuntime.active = 0;
    }
    if (fgSceneEquals(gFgRuntime.sceneName, "building4") &&
        gFgRuntime.frameIndex == 0 &&
        gFgRuntime.stagedFrameValid &&
        gFgRuntime.stagedFrameIndex == 1 &&
        !fgRuntimePresentFirstFrameNoWait()) {
        if (ps1PerfEnabled) ps1PerfMarkTripwire();
        gFgRuntime.active = 0;
    }
    if (fgSceneEquals(gFgRuntime.sceneName, "building4") &&
        gFgRuntime.active &&
        gFgRuntime.frameIndex == 1 &&
        gFgRuntime.stagedFrameValid &&
        gFgRuntime.stagedFrameIndex == 2 &&
        !fgRuntimePresentNextStagedAndAdvance()) {
        if (ps1PerfEnabled) ps1PerfMarkTripwire();
        gFgRuntime.active = 0;
    }

    if (gFgRuntimeSceneId == FG_SCENE_VISITOR3 && !islandState.lowTide) {
        VSync(0);
        VSync(0);
#if FG_VISITOR3_LOW_PHASE_VBLANKS >= 1
    } else if (gFgRuntimeSceneId == FG_SCENE_VISITOR3 && islandState.lowTide) {
        if (gFgRuntime.frameIndex == 0 &&
            gFgRuntime.stagedFrameValid &&
            gFgRuntime.stagedFrameIndex == 1 &&
            !fgRuntimePresentFirstFrameNoWait()) {
            if (ps1PerfEnabled) ps1PerfMarkTripwire();
            gFgRuntime.active = 0;
        }
        if (gFgRuntime.active &&
            gFgRuntime.frameIndex == 1 &&
            gFgRuntime.stagedFrameValid &&
            gFgRuntime.stagedFrameIndex == 2 &&
            !fgRuntimePresentNextStagedAndAdvance()) {
            if (ps1PerfEnabled) ps1PerfMarkTripwire();
            gFgRuntime.active = 0;
        }
        VSync(0);
#if FG_VISITOR3_LOW_PHASE_VBLANKS >= 2
        VSync(0);
#endif
#if FG_VISITOR3_LOW_PHASE_VBLANKS >= 3
        VSync(0);
#endif
#if FG_VISITOR3_LOW_PHASE_VBLANKS >= 4
        VSync(0);
#endif
#endif
    } else if (gFgRuntimeSceneId == FG_SCENE_BUILDING2 && !islandState.lowTide) {
#if FG_BUILDING2_HIGH_PHASE_VBLANKS >= 1
        VSync(0);
#endif
#if FG_BUILDING2_HIGH_PHASE_VBLANKS >= 2
        VSync(0);
#endif
#if FG_BUILDING2_HIGH_PHASE_VBLANKS >= 3
        VSync(0);
#endif
#if FG_BUILDING2_HIGH_PHASE_VBLANKS >= 4
        VSync(0);
#endif
    } else if (gFgRuntimeSceneId == FG_SCENE_WALKSTUF1 && !islandState.lowTide) {
#if FG_WALKSTUF1_HIGH_PHASE_VBLANKS >= 1
        VSync(0);
#endif
#if FG_WALKSTUF1_HIGH_PHASE_VBLANKS >= 2
        VSync(0);
#endif
#if FG_WALKSTUF1_HIGH_PHASE_VBLANKS >= 3
        VSync(0);
#endif
#if FG_WALKSTUF1_HIGH_PHASE_VBLANKS >= 4
        VSync(0);
#endif
#if FG_WALKSTUF1_LOW_PHASE_VBLANKS >= 1
    } else if (gFgRuntimeSceneId == FG_SCENE_WALKSTUF1 && islandState.lowTide) {
        VSync(0);
#if FG_WALKSTUF1_LOW_PHASE_VBLANKS >= 2
        VSync(0);
#endif
#if FG_WALKSTUF1_LOW_PHASE_VBLANKS >= 3
        VSync(0);
#endif
#if FG_WALKSTUF1_LOW_PHASE_VBLANKS >= 4
        VSync(0);
#endif
#endif
    }

    if (ps1PerfEnabled)
        ps1PerfMarkLoopStart();
    {
        while (foregroundPilotRuntimeActive()) {
            int advancedThisLoop = 0;

            /* Pause-menu request: break out so jc_reborn's outer loop can
             * advance to next scene or restart the loop. The flag is
             * cleared by the consumer in jc_reborn.c. */
            if (pauseMenuRequestNextScene ||
                pauseMenuRequestResetLoop ||
                pauseMenuRequestFreeplay ||
                pauseMenuRequestSceneSetCycle ||
                pauseMenuRequestPlayScene >= 0 ||
                pauseMenuRequestLoopScene >= 0) {
                break;
            }
        if (fgRuntimeCanHoldDisplayedFrame()) {
            uint16 prefetchElapsedVBlanks = 0;
            uint16 prepareElapsedVBlanks = 0;
            uint16 heldSlackVBlanks = 0;
            uint8 schedOwner = PS1_PERF_SCHED_WAIT;
            int didPrefetch = 0;
            int didPrepare = 0;
            int didNextStage = 0;
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
                /* WALKSTUF1 is visual-work bound at this slack point;
                 * preparing first can reduce due-path blocking more than the
                 * skipped speculative window refill costs. */
                if (windowWouldRead &&
                    gFgRuntimeSceneId == FG_SCENE_WALKSTUF1 &&
                    fgRuntimeCanPrepareStagedFrame()) {
                    didPrepare = fgRuntimePrepareStagedFrameForPresent(&prepareElapsedVBlanks,
                                                                       perfDetail);
                    if (didPrepare)
                        schedOwner = PS1_PERF_SCHED_VISUAL_PREPARE;
                } else {
                    didPrefetch = windowWouldRead
                        ? fgRuntimeTryPrefetchWindow(&prefetchElapsedVBlanks)
                        : 0;
                    if (didPrefetch)
                        schedOwner = PS1_PERF_SCHED_CD_WINDOW;
                }
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
            if (!advancedThisLoop &&
                !didPrefetch &&
                !didPrepare) {
                const char *stageSceneName =
                    (gFgStageSceneName[0] != '\0') ? gFgStageSceneName : sceneName;
                didNextStage = fgNextSceneStageTryTick(stageSceneName);
                if (didNextStage)
                    schedOwner = PS1_PERF_SCHED_CD_WINDOW;
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
                } else if (didNextStage) {
                    fgRuntimeWaitHeldVBlank();
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
    }
    if (ps1PerfEnabled) {
        fgNextSceneStageFinishPending();
        ps1PerfMarkLoopEnd();
        ps1PerfMarkSoundCursor(gFgRuntime.soundEventCursor);
        ps1PerfMarkCleanupStart();
    } else {
        fgNextSceneStageFinishPending();
    }

    if (deferWalkCleanRecapture && !blackBackdrop && !sceneSpecificBackdrop &&
        grCleanBgRectsCount() > 0) {
        grForceFullRedrawNextFrame();
        grRestoreBgFromRects();
        fgBackdropStampHoliday();
    }

    /* End-of-scene heap cleanup — the screensaver loop replays scenes
     * indefinitely, so every per-scene allocation needs to come back to
     * the heap before the next scene starts. Without this the fragmented
     * state accumulates and a later scene's large contiguous alloc
     * (clean-rect buffer ~270 KB, frame payload ~90 KB, BACKGRND PSB
     * ~93 KB) fails silently after 2-3 iterations. */
    fgRuntimeReset();
    fgReleaseStreamBuffers();
    if (blackBackdrop || sceneSpecificBackdrop ||
        largeCleanSnapshot || deferWalkCleanRecapture) {
        grFreeCleanBgRects();
        grSetCleanBgBlackMode(0);
        fgCleanOverlayInvalidate();
    } else if (fgCleanRectsNeedCacheForProof(sceneName)) {
        fgCleanOverlayRemember(sceneName);
    } else if (gFgLoadingWaveProofEnabled) {
        fgCleanOverlayInvalidate();
    } else {
        /* Deactivate the rect-snapshot — keep the buffer alive at its
         * boot-prealloc address so we don't fragment normal island scenes. */
        grDeactivateCleanBgRects();
        fgCleanOverlayInvalidate();
    }
    /* Keep BACKGRND.BMP in slot 0 across scenes; release only
     * variant-dependent overlay slots to avoid needless PSB churn. */
    if (deferWalkCleanRecapture && !blackBackdrop && !sceneSpecificBackdrop) {
        walkPilotCaptureCleanWalkAreaIfStale(islandState.raft,
                                             islandState.lowTide,
                                             islandState.night,
                                             islandState.holiday,
                                             islandState.xPos,
                                             islandState.yPos);
    }
    fgBackdropRelease((blackBackdrop || sceneSpecificBackdrop) ? 0 : 1);
    fgHeapProbe("after_scene_cleanup", sceneName);
}

static int __attribute__((noinline, optimize("Os")))
fgBackdropSaveVisitor3HighCleanBgRects(void)
{
    const sint16 dx = gFgSceneDrawOffsetX;
    const sint16 dy = gFgSceneDrawOffsetY;
    sint16 xs[6];
    sint16 ys[6];
    uint16 ws[6];
    uint16 hs[6];

    /* VISITOR3 high's header bbox overstates the visible ship/cleanup area.
     * These rects cover the validated foreground extent plus the high-tide
     * wave band while keeping every allocation below the 64 KiB clean cap. */
    xs[0] = dx;                  ys[0] = (sint16)(dy + 97);  ws[0] = 522; hs[0] = 62;
    xs[1] = dx;                  ys[1] = (sint16)(dy + 159); ws[1] = 522; hs[1] = 62;
    xs[2] = dx;                  ys[2] = (sint16)(dy + 221); ws[2] = 522; hs[2] = 62;
    xs[3] = dx;                  ys[3] = (sint16)(dy + 283); ws[3] = 522; hs[3] = 62;
    xs[4] = dx;                  ys[4] = (sint16)(dy + 345); ws[4] = 522; hs[4] = 54;
    xs[5] = (sint16)(dx + 522);  ys[5] = (sint16)(dy + 97);  ws[5] = 86;  hs[5] = 259;

    return grSaveCleanBgRects(xs, ys, ws, hs, 6) > 0;
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

void foregroundPilotSetSceneDrawOffset(int x, int y)
{
    gFgSceneDrawOffsetX = (sint16)x;
    gFgSceneDrawOffsetY = (sint16)y;
}

void foregroundPilotSetHeapProbe(int enabled)
{
#if FG_HEAP_PROBE_LOGS
    gFgHeapProbeEnabled = enabled ? 1 : 0;
#else
    (void)enabled;
#endif
}

void foregroundPilotSetLoadingWaveProof(int enabled)
{
    gFgLoadingWaveProofEnabled = enabled ? 1 : 0;
    grSetFullScreenScrCacheEnabled(gFgLoadingWaveProofEnabled);
    if (!gFgLoadingWaveProofEnabled) {
        fgNextSceneStageInvalidate();
        fgCleanOverlayInvalidate();
        grSetCleanBgRectsForceCache(0);
    }
}

void foregroundPilotSetSpuStage(int enabled)
{
    gFgSpuStageEnabled = enabled ? 1 : 0;
    if (!gFgSpuStageEnabled)
        fgNextSceneStageInvalidate();
}

void foregroundPilotSetStageScene(const char *sceneName)
{
    size_t i;

    if (!sceneName) {
        gFgStageSceneName[0] = '\0';
        return;
    }

    for (i = 0; i + 1 < sizeof(gFgStageSceneName) && sceneName[i] != '\0'; i++)
        gFgStageSceneName[i] = sceneName[i];
    gFgStageSceneName[i] = '\0';
}

void foregroundPilotResetPrefetchDefaults(void)
{
    /* Match the file-static default: prefetch is ON. Scenes need it for
     * full-speed playback. See gFgPrefetchStage1Enabled rationale. */
    gFgPrefetchStage1Enabled = 1;
    gFgPrefetchWindowBytes = FG_PREFETCH_DEFAULT_WINDOW_BYTES;
    gFgSpuStageEnabled = 0;
    gFgStageSceneName[0] = '\0';
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
    if (fgSceneEquals(gForegroundPilotScene, "freeplay")) {
        freeplayRun();
        return;
    }

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

    /* PS1 perf is layout-sensitive: keep compact-scene branch simplifications
     * from moving downstream hot text. Compact fgpilot scenes return above. */
    __asm__ volatile("nop\nnop\nnop");

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
static char gFgStageSceneName[16] = "";

void foregroundPilotSetScene(const char *sceneName)
{
    if (!sceneName) {
        gForegroundPilotScene[0] = '\0';
        return;
    }
    strncpy(gForegroundPilotScene, sceneName, sizeof(gForegroundPilotScene) - 1);
    gForegroundPilotScene[sizeof(gForegroundPilotScene) - 1] = '\0';
}

void foregroundPilotSetSceneDrawOffset(int x, int y)
{
    (void)x;
    (void)y;
}

void foregroundPilotSetHeapProbe(int enabled)
{
    (void)enabled;
}

void foregroundPilotSetLoadingWaveProof(int enabled)
{
    (void)enabled;
}

void foregroundPilotSetSpuStage(int enabled)
{
    (void)enabled;
}

void foregroundPilotSetStageScene(const char *sceneName)
{
    if (!sceneName) {
        gFgStageSceneName[0] = '\0';
        return;
    }
    strncpy(gFgStageSceneName, sceneName, sizeof(gFgStageSceneName) - 1);
    gFgStageSceneName[sizeof(gFgStageSceneName) - 1] = '\0';
}

void foregroundPilotResetPrefetchDefaults(void)
{
    gFgStageSceneName[0] = '\0';
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
