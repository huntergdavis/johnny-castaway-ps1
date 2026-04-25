#include "ps1_perf.h"

#ifdef PS1_BUILD

#include <stdio.h>
#include <string.h>
#include <psxapi.h>
#include <psxgpu.h>

struct TTtmThread;
#include "island.h"

#define PS1_PERF_PHASE_SETUP 0
#define PS1_PERF_PHASE_LOOP 1
#define PS1_PERF_PHASE_CLEANUP 2
#define PS1_PERF_UNKNOWN_LBA 0xffffffffUL
#define PS1_PERF_CD_SECTOR_SIZE 2048UL

volatile uint8 ps1PerfEnabled = 0;
volatile uint8 ps1PerfLevel = PS1_PERF_LEVEL_OFF;

struct TPs1PerfCounters {
    char sceneName[16];
    char packPath[32];
    uint8 phase;
    uint8 packFormat;
    uint16 packFlags;
    uint32 sceneStartTick;
    uint32 loopStartTick;
    uint32 loopEndTick;
    uint32 cleanupStartTick;
    uint32 sceneEndTick;

    uint32 packBytes;
    uint32 packPadding;
    uint32 packLba;
    uint32 packSectors;
    uint16 packFrameCount;
    uint16 packEntryCount;
    uint16 packSoundCount;

    uint32 screenVBlanks;
    uint32 backdropVBlanks;
    uint32 packStartVBlanks;
    uint32 cleanRectVBlanks;
    uint32 firstFrameVBlanks;

    uint32 renderedLoops;
    uint32 heldLoops;
    uint32 advances;
    uint32 elapsedVBlanks;
    uint32 targetVBlanks;
    uint32 lateAdvances;
    uint16 maxElapsedVBlanks;
    uint16 maxElapsedFrameIndex;

    uint16 currentFrameIndex;
    uint16 currentSourceFrame;
    uint32 currentDataOffset;
    uint16 lastFrameIndex;
    uint16 lastSourceFrame;

    uint32 entries;
    uint32 emptyEntries;
    uint32 entryPayloadBytes;
    uint32 maxEntryPayloadBytes32;
    uint16 maxEntryPayloadBytes;
    uint16 maxEntryPayloadIndex;
    uint16 maxEntryPayloadSource;
    uint16 maxEntryHoldVBlanks;
    uint16 maxEntryHoldIndex;
    uint32 hold1;
    uint32 hold2To4;
    uint32 hold5To8;
    uint32 hold9Plus;
    uint32 payload0;
    uint32 payload1k;
    uint32 payload4k;
    uint32 payload16k;
    uint32 payload64k;
    uint32 payload64kPlus;

    /* Legacy JCPERF CD counters: keep the old payload-read view stable. */
    uint32 legacyCdReads;
    uint32 legacyCdReadFailures;
    uint32 legacyCdBytes;
    uint32 legacyCdSectors;
    uint16 legacyMaxCdSectors;
    uint32 legacyCdElapsedVBlanks;
    uint16 legacyMaxCdElapsedVBlanks;

    uint32 cdReads;
    uint32 cdSetloc;
    uint32 cdReadFailures;
    uint32 cdBytes;
    uint32 cdSectors;
    uint32 cdElapsedVBlanks;
    uint32 cdSetupReads;
    uint32 cdSetupBytes;
    uint32 cdSetupVBlanks;
    uint32 cdLoopReads;
    uint32 cdLoopBytes;
    uint32 cdLoopVBlanks;
    uint32 cdBlockingReads;
    uint32 cdBlockingVBlanks;
    uint32 cdHiddenReads;
    uint32 cdHiddenVBlanks;
    uint16 maxCdSectors;
    uint16 maxCdElapsedVBlanks;
    uint16 maxCdReadIndex;
    uint32 cdUnalignedStart;
    uint32 cdUnalignedEnd;
    uint32 cdOverreadBytes;
    uint32 cdScratchBytes;
    uint32 cdSequentialReads;
    uint32 cdSeekForwardReads;
    uint32 cdSeekBackwardReads;
    uint32 cdMaxSectorGap;
    uint8 cdHasLastRange;
    uint32 cdLastEndSector;
    uint32 cdSector1;
    uint32 cdSector2;
    uint32 cdSector3To4;
    uint32 cdSector5To8;
    uint32 cdSector9Plus;

    uint8 prefetchPolicy;
    uint8 cdPrefetchActive;
    uint16 cdPrefetchSlackVBlanks;
    uint32 prefetchAttempts;
    uint32 prefetchEligible;
    uint32 prefetchIneligible;
    uint32 prefetchHits;
    uint32 prefetchStageHits;
    uint32 prefetchMisses;
    uint32 prefetchDueMisses;
    uint32 prefetchSlackVBlanks;
    uint32 prefetchUsedVBlanks;
    uint32 prefetchOverrunVBlanks;
    uint16 prefetchLeadMin;
    uint16 prefetchLeadMax;
    uint32 prefetchSkippedNoSlack;
    uint32 prefetchDuplicate;
    uint32 prefetchWastedBytes;

    uint32 restoreCalls;
    uint32 restoreBytes;
    uint32 maxRestoreBytes;
    uint32 composeCalls;
    uint32 composeRows;
    uint32 composeSpans;
    uint32 composePixels;
    uint32 composePayloadBytes;
    uint32 uploadCalls;
    uint32 uploadRects;
    uint32 uploadBytes;
    uint32 uploadElapsedVBlanks;
    uint16 maxUploadElapsedVBlanks;
    uint32 maxUploadBytes;
    uint16 maxUploadRects;
    uint32 dirtyRows;
    uint32 dirtyExactBytes;
    uint32 dirtyRoundedBytes;
    uint32 dirtyMaxRows;
    uint32 dirtyMaxExactBytes;
    uint32 dirtyMaxRoundedBytes;
    uint32 dirtyCapHits;
    uint32 fullFallbacks;

    uint32 renderVBlanks;
    uint16 maxRenderVBlanks;
    uint16 maxRenderFrameIndex;
    uint32 restoreVBlanks;
    uint32 composeVBlanks;
    uint32 presentWaitVBlanks;
    uint32 eventWaitVBlanks;
    uint32 advanceVBlanks;
    uint32 crossedRestore;
    uint32 crossedCompose;
    uint32 crossedUpload;
    uint32 crossedAdvance;

    uint32 frameBufferBytes;
    uint32 scratchBytes;
    uint32 prefetchBytes;
    uint32 peakPrefetchBytes;
    uint32 allocFailures;
    uint32 allocFailBytes;

    uint32 tripwires;
    uint32 fallbacks;
    uint32 staleGuards;
    uint32 frameMismatches;
    uint32 soundEvents;
    uint32 soundLate;
    uint16 soundCursorEnd;
};

static struct TPs1PerfCounters gPs1Perf;

static void ps1PerfCopyFixed(char *dst, uint16 dstSize, const char *src)
{
    if (dstSize == 0)
        return;
    if (src == NULL)
        src = "na";
    strncpy(dst, src, (size_t)dstSize - 1u);
    dst[dstSize - 1u] = '\0';
}

static void ps1PerfCopySceneName(const char *sceneName)
{
    ps1PerfCopyFixed(gPs1Perf.sceneName, sizeof(gPs1Perf.sceneName), sceneName);
}

static uint16 ps1PerfClampU16(uint32 value)
{
    return (uint16)((value > 0xffffu) ? 0xffffu : value);
}

static uint32 ps1PerfTickDiff(uint32 startTick, uint32 endTick)
{
    return (endTick >= startTick) ? (endTick - startTick) : 0;
}

static uint32 ps1PerfCurrentElapsed(uint32 startTick)
{
    return ps1PerfTickDiff(startTick, ps1PerfTick());
}

static const char *ps1PerfFormatName(void)
{
    if (gPs1Perf.packFormat == 2)
        return "fgp2_pal4";
    if (gPs1Perf.packFormat == 3)
        return "fgp2_indexed8";
    return "unknown";
}

static const char *ps1PerfPrefetchPolicyName(void)
{
    if (gPs1Perf.prefetchPolicy == PS1_PERF_PREFETCH_STAGE1)
        return "stage1";
    return "none";
}

void ps1PerfSetLevel(int level)
{
    if (level < PS1_PERF_LEVEL_OFF)
        level = PS1_PERF_LEVEL_OFF;
    if (level > PS1_PERF_LEVEL_DEBUG)
        level = PS1_PERF_LEVEL_DEBUG;

    ps1PerfLevel = (uint8)level;
    ps1PerfEnabled = (level != PS1_PERF_LEVEL_OFF) ? 1 : 0;
    if (!ps1PerfEnabled)
        memset(&gPs1Perf, 0, sizeof(gPs1Perf));
}

void ps1PerfSetEnabled(int enabled)
{
    ps1PerfSetLevel(enabled ? PS1_PERF_LEVEL_SUMMARY : PS1_PERF_LEVEL_OFF);
}

int ps1PerfDetailEnabled(void)
{
    return (ps1PerfLevel >= PS1_PERF_LEVEL_DETAIL) ? 1 : 0;
}

uint32 ps1PerfTick(void)
{
    return (uint32)VSync(-1);
}

uint16 ps1PerfElapsedVBlanks(uint32 startTick)
{
    return ps1PerfClampU16(ps1PerfCurrentElapsed(startTick));
}

void ps1PerfBeginScene(const char *sceneName)
{
    if (!ps1PerfEnabled)
        return;

    memset(&gPs1Perf, 0, sizeof(gPs1Perf));
    ps1PerfCopySceneName(sceneName);
    ps1PerfCopyFixed(gPs1Perf.packPath, sizeof(gPs1Perf.packPath), "na");
    gPs1Perf.phase = PS1_PERF_PHASE_SETUP;
    gPs1Perf.packLba = PS1_PERF_UNKNOWN_LBA;
    gPs1Perf.sceneStartTick = ps1PerfTick();

    printf(
        "JCPERF scene-start scene=%s lowtide=%d night=%d holiday=%d raft=%d pos=%d,%d\n",
        gPs1Perf.sceneName,
        islandState.lowTide,
        islandState.night,
        islandState.holiday,
        islandState.raft,
        islandState.xPos,
        islandState.yPos
    );
}

void ps1PerfMarkSetupPhase(uint8 phase, uint16 elapsedVBlanks)
{
    if (!ps1PerfEnabled)
        return;

    switch (phase) {
        case PS1_PERF_SETUP_SCREEN:
            gPs1Perf.screenVBlanks += elapsedVBlanks;
            break;
        case PS1_PERF_SETUP_BACKDROP:
            gPs1Perf.backdropVBlanks += elapsedVBlanks;
            break;
        case PS1_PERF_SETUP_PACK_START:
            gPs1Perf.packStartVBlanks += elapsedVBlanks;
            break;
        case PS1_PERF_SETUP_CLEAN_RECT:
            gPs1Perf.cleanRectVBlanks += elapsedVBlanks;
            break;
        case PS1_PERF_SETUP_FIRST_FRAME:
            gPs1Perf.firstFrameVBlanks += elapsedVBlanks;
            break;
        default:
            break;
    }
}

void ps1PerfMarkLoopStart(void)
{
    if (!ps1PerfEnabled)
        return;
    gPs1Perf.phase = PS1_PERF_PHASE_LOOP;
    gPs1Perf.loopStartTick = ps1PerfTick();
}

void ps1PerfMarkLoopEnd(void)
{
    if (!ps1PerfEnabled)
        return;
    gPs1Perf.loopEndTick = ps1PerfTick();
}

void ps1PerfMarkCleanupStart(void)
{
    if (!ps1PerfEnabled)
        return;
    gPs1Perf.phase = PS1_PERF_PHASE_CLEANUP;
    gPs1Perf.cleanupStartTick = ps1PerfTick();
}

void ps1PerfSetPackInfo(const char *packPath, uint32 packBytes,
                        uint32 packLba, uint32 packSectors,
                        uint16 frameCount, uint16 entryCount,
                        uint16 soundCount, uint16 flags,
                        uint8 packFormat, uint32 frameBufferBytes,
                        uint32 scratchBytes)
{
    uint32 roundedBytes;

    if (!ps1PerfEnabled)
        return;

    ps1PerfCopyFixed(gPs1Perf.packPath, sizeof(gPs1Perf.packPath), packPath);
    gPs1Perf.packBytes = packBytes;
    gPs1Perf.packLba = packLba;
    gPs1Perf.packSectors = packSectors;
    roundedBytes = packSectors * PS1_PERF_CD_SECTOR_SIZE;
    gPs1Perf.packPadding = (roundedBytes > packBytes) ? (roundedBytes - packBytes) : 0;
    gPs1Perf.packFrameCount = frameCount;
    gPs1Perf.packEntryCount = entryCount;
    gPs1Perf.packSoundCount = soundCount;
    gPs1Perf.packFlags = flags;
    gPs1Perf.packFormat = packFormat;
    gPs1Perf.frameBufferBytes = frameBufferBytes;
    gPs1Perf.scratchBytes = scratchBytes;
}

void ps1PerfSetCurrentFrame(uint16 frameIndex, uint16 sourceFrame,
                            uint32 dataOffset)
{
    if (!ps1PerfEnabled)
        return;
    gPs1Perf.currentFrameIndex = frameIndex;
    gPs1Perf.currentSourceFrame = sourceFrame;
    gPs1Perf.currentDataOffset = dataOffset;
}

void ps1PerfMarkRenderedLoop(void)
{
    if (ps1PerfEnabled)
        gPs1Perf.renderedLoops++;
}

void ps1PerfMarkHeldLoop(void)
{
    if (ps1PerfEnabled)
        gPs1Perf.heldLoops++;
}

void ps1PerfMarkAdvance(uint16 elapsedVBlanks, uint16 targetVBlanks)
{
    if (!ps1PerfEnabled)
        return;
    (void)targetVBlanks;
    gPs1Perf.advances++;
    gPs1Perf.elapsedVBlanks += elapsedVBlanks;
    if (elapsedVBlanks > 1)
        gPs1Perf.lateAdvances++;
    if (elapsedVBlanks > gPs1Perf.maxElapsedVBlanks) {
        gPs1Perf.maxElapsedVBlanks = elapsedVBlanks;
        gPs1Perf.maxElapsedFrameIndex = gPs1Perf.currentFrameIndex;
    }
}

void ps1PerfMarkEntry(uint32 payloadBytes, uint16 holdVBlanks,
                      uint8 emptyEntry, uint16 sourceFrame,
                      uint32 dataOffset)
{
    if (!ps1PerfEnabled)
        return;

    gPs1Perf.entries++;
    gPs1Perf.entryPayloadBytes += payloadBytes;
    gPs1Perf.targetVBlanks += holdVBlanks;
    gPs1Perf.lastFrameIndex = gPs1Perf.currentFrameIndex;
    gPs1Perf.lastSourceFrame = sourceFrame;
    gPs1Perf.currentDataOffset = dataOffset;
    if (emptyEntry)
        gPs1Perf.emptyEntries++;

    if (payloadBytes > gPs1Perf.maxEntryPayloadBytes32) {
        gPs1Perf.maxEntryPayloadBytes32 = payloadBytes;
        gPs1Perf.maxEntryPayloadBytes = ps1PerfClampU16(payloadBytes);
        gPs1Perf.maxEntryPayloadIndex = gPs1Perf.currentFrameIndex;
        gPs1Perf.maxEntryPayloadSource = sourceFrame;
    }
    if (holdVBlanks > gPs1Perf.maxEntryHoldVBlanks) {
        gPs1Perf.maxEntryHoldVBlanks = holdVBlanks;
        gPs1Perf.maxEntryHoldIndex = gPs1Perf.currentFrameIndex;
    }

    if (holdVBlanks <= 1)
        gPs1Perf.hold1++;
    else if (holdVBlanks <= 4)
        gPs1Perf.hold2To4++;
    else if (holdVBlanks <= 8)
        gPs1Perf.hold5To8++;
    else
        gPs1Perf.hold9Plus++;

    if (payloadBytes == 0)
        gPs1Perf.payload0++;
    else if (payloadBytes <= 1024u)
        gPs1Perf.payload1k++;
    else if (payloadBytes <= 4096u)
        gPs1Perf.payload4k++;
    else if (payloadBytes <= 16384u)
        gPs1Perf.payload16k++;
    else if (payloadBytes <= 65536u)
        gPs1Perf.payload64k++;
    else
        gPs1Perf.payload64kPlus++;
}

void ps1PerfMarkCdRead(uint32 bytes, uint32 sectors, uint16 elapsedVBlanks, int ok)
{
    ps1PerfMarkCdReadDetailed(bytes, sectors, elapsedVBlanks, ok,
                              PS1_PERF_UNKNOWN_LBA, 0, 1);
}

void ps1PerfMarkCdReadDetailed(uint32 bytes, uint32 sectors,
                               uint16 elapsedVBlanks, int ok,
                               uint32 fileLba, uint32 fileOffset,
                               uint8 includeLegacy)
{
    uint32 overreadBytes;
    uint32 startSector;
    uint32 endSector;

    if (!ps1PerfEnabled)
        return;

    if (includeLegacy) {
        gPs1Perf.legacyCdReads++;
        if (!ok)
            gPs1Perf.legacyCdReadFailures++;
        gPs1Perf.legacyCdBytes += bytes;
        gPs1Perf.legacyCdSectors += sectors;
        gPs1Perf.legacyCdElapsedVBlanks += elapsedVBlanks;
        if (sectors > gPs1Perf.legacyMaxCdSectors)
            gPs1Perf.legacyMaxCdSectors = ps1PerfClampU16(sectors);
        if (elapsedVBlanks > gPs1Perf.legacyMaxCdElapsedVBlanks)
            gPs1Perf.legacyMaxCdElapsedVBlanks = elapsedVBlanks;
    }

    gPs1Perf.cdReads++;
    gPs1Perf.cdSetloc++;
    if (!ok)
        gPs1Perf.cdReadFailures++;
    gPs1Perf.cdBytes += bytes;
    gPs1Perf.cdSectors += sectors;
    gPs1Perf.cdElapsedVBlanks += elapsedVBlanks;
    if (sectors > gPs1Perf.maxCdSectors)
        gPs1Perf.maxCdSectors = ps1PerfClampU16(sectors);
    if (elapsedVBlanks > gPs1Perf.maxCdElapsedVBlanks) {
        gPs1Perf.maxCdElapsedVBlanks = elapsedVBlanks;
        gPs1Perf.maxCdReadIndex = gPs1Perf.currentFrameIndex;
    }

    if ((fileOffset % PS1_PERF_CD_SECTOR_SIZE) != 0)
        gPs1Perf.cdUnalignedStart++;
    if (((fileOffset + bytes) % PS1_PERF_CD_SECTOR_SIZE) != 0)
        gPs1Perf.cdUnalignedEnd++;
    overreadBytes = (sectors * PS1_PERF_CD_SECTOR_SIZE);
    if (overreadBytes > bytes)
        gPs1Perf.cdOverreadBytes += (overreadBytes - bytes);
    gPs1Perf.cdScratchBytes += overreadBytes;

    if (sectors <= 1)
        gPs1Perf.cdSector1++;
    else if (sectors == 2)
        gPs1Perf.cdSector2++;
    else if (sectors <= 4)
        gPs1Perf.cdSector3To4++;
    else if (sectors <= 8)
        gPs1Perf.cdSector5To8++;
    else
        gPs1Perf.cdSector9Plus++;

    if (fileLba != PS1_PERF_UNKNOWN_LBA) {
        startSector = fileLba + (fileOffset / PS1_PERF_CD_SECTOR_SIZE);
        endSector = startSector + sectors;
        if (gPs1Perf.cdHasLastRange) {
            uint32 gap;
            if (startSector == gPs1Perf.cdLastEndSector) {
                gPs1Perf.cdSequentialReads++;
            } else if (startSector > gPs1Perf.cdLastEndSector) {
                gPs1Perf.cdSeekForwardReads++;
                gap = startSector - gPs1Perf.cdLastEndSector;
                if (gap > gPs1Perf.cdMaxSectorGap)
                    gPs1Perf.cdMaxSectorGap = gap;
            } else {
                gPs1Perf.cdSeekBackwardReads++;
                gap = gPs1Perf.cdLastEndSector - startSector;
                if (gap > gPs1Perf.cdMaxSectorGap)
                    gPs1Perf.cdMaxSectorGap = gap;
            }
        }
        gPs1Perf.cdLastEndSector = endSector;
        gPs1Perf.cdHasLastRange = 1;
    }

    if (gPs1Perf.phase == PS1_PERF_PHASE_SETUP) {
        gPs1Perf.cdSetupReads++;
        gPs1Perf.cdSetupBytes += bytes;
        gPs1Perf.cdSetupVBlanks += elapsedVBlanks;
    } else if (gPs1Perf.phase == PS1_PERF_PHASE_LOOP) {
        gPs1Perf.cdLoopReads++;
        gPs1Perf.cdLoopBytes += bytes;
        gPs1Perf.cdLoopVBlanks += elapsedVBlanks;
        if (gPs1Perf.cdPrefetchActive) {
            uint16 hiddenVBlanks = elapsedVBlanks;
            uint16 blockingVBlanks = 0;

            if (hiddenVBlanks > gPs1Perf.cdPrefetchSlackVBlanks) {
                blockingVBlanks = (uint16)(hiddenVBlanks - gPs1Perf.cdPrefetchSlackVBlanks);
                hiddenVBlanks = gPs1Perf.cdPrefetchSlackVBlanks;
            }

            gPs1Perf.cdHiddenReads++;
            gPs1Perf.cdHiddenVBlanks += hiddenVBlanks;
            if (blockingVBlanks > 0) {
                gPs1Perf.cdBlockingReads++;
                gPs1Perf.cdBlockingVBlanks += blockingVBlanks;
            }
        } else {
            gPs1Perf.cdBlockingReads++;
            gPs1Perf.cdBlockingVBlanks += elapsedVBlanks;
            gPs1Perf.prefetchMisses++;
            gPs1Perf.prefetchDueMisses++;
        }
    }
}

void ps1PerfSetPrefetchPolicy(uint8 policy, uint32 bufferBytes)
{
    if (!ps1PerfEnabled)
        return;
    gPs1Perf.prefetchPolicy = policy;
    gPs1Perf.prefetchBytes = bufferBytes;
    if (bufferBytes > gPs1Perf.peakPrefetchBytes)
        gPs1Perf.peakPrefetchBytes = bufferBytes;
}

void ps1PerfMarkPrefetchAttempt(uint16 leadVBlanks, uint16 slackVBlanks,
                                uint8 eligible)
{
    if (!ps1PerfEnabled)
        return;
    gPs1Perf.prefetchAttempts++;
    gPs1Perf.prefetchSlackVBlanks += slackVBlanks;
    if (eligible)
        gPs1Perf.prefetchEligible++;
    else
        gPs1Perf.prefetchIneligible++;

    if (leadVBlanks > 0) {
        if (gPs1Perf.prefetchLeadMin == 0 || leadVBlanks < gPs1Perf.prefetchLeadMin)
            gPs1Perf.prefetchLeadMin = leadVBlanks;
        if (leadVBlanks > gPs1Perf.prefetchLeadMax)
            gPs1Perf.prefetchLeadMax = leadVBlanks;
    }
}

void ps1PerfMarkPrefetchSkipNoSlack(void)
{
    if (ps1PerfEnabled)
        gPs1Perf.prefetchSkippedNoSlack++;
}

void ps1PerfMarkPrefetchDuplicate(void)
{
    if (ps1PerfEnabled)
        gPs1Perf.prefetchDuplicate++;
}

void ps1PerfBeginPrefetchRead(uint16 slackVBlanks)
{
    if (!ps1PerfEnabled)
        return;
    gPs1Perf.cdPrefetchActive = 1;
    gPs1Perf.cdPrefetchSlackVBlanks = slackVBlanks;
}

void ps1PerfEndPrefetchRead(uint16 elapsedVBlanks, uint32 bytes, int ok)
{
    if (!ps1PerfEnabled)
        return;
    gPs1Perf.prefetchUsedVBlanks += elapsedVBlanks;
    if (elapsedVBlanks > gPs1Perf.cdPrefetchSlackVBlanks)
        gPs1Perf.prefetchOverrunVBlanks += (uint16)(elapsedVBlanks - gPs1Perf.cdPrefetchSlackVBlanks);
    if (!ok)
        gPs1Perf.prefetchWastedBytes += bytes;
    gPs1Perf.cdPrefetchActive = 0;
    gPs1Perf.cdPrefetchSlackVBlanks = 0;
}

void ps1PerfMarkPrefetchHit(void)
{
    if (!ps1PerfEnabled)
        return;
    gPs1Perf.prefetchHits++;
    gPs1Perf.prefetchStageHits++;
}

void ps1PerfMarkRestore(uint32 bytes)
{
    if (!ps1PerfEnabled)
        return;
    gPs1Perf.restoreCalls++;
    gPs1Perf.restoreBytes += bytes;
    if (bytes > gPs1Perf.maxRestoreBytes)
        gPs1Perf.maxRestoreBytes = bytes;
}

void ps1PerfMarkCompose(uint16 rows, uint16 spans, uint32 pixels, uint32 payloadBytes)
{
    if (!ps1PerfEnabled)
        return;
    gPs1Perf.composeCalls++;
    gPs1Perf.composeRows += rows;
    gPs1Perf.composeSpans += spans;
    gPs1Perf.composePixels += pixels;
    gPs1Perf.composePayloadBytes += payloadBytes;
}

void ps1PerfMarkUpload(uint16 rects, uint32 bytes, uint16 elapsedVBlanks)
{
    if (!ps1PerfEnabled)
        return;
    gPs1Perf.uploadCalls++;
    gPs1Perf.uploadRects += rects;
    gPs1Perf.uploadBytes += bytes;
    gPs1Perf.uploadElapsedVBlanks += elapsedVBlanks;
    gPs1Perf.dirtyRoundedBytes += bytes;
    if (elapsedVBlanks > gPs1Perf.maxUploadElapsedVBlanks)
        gPs1Perf.maxUploadElapsedVBlanks = elapsedVBlanks;
    if (bytes > gPs1Perf.maxUploadBytes)
        gPs1Perf.maxUploadBytes = bytes;
    if (rects > gPs1Perf.maxUploadRects)
        gPs1Perf.maxUploadRects = rects;
    if (bytes > gPs1Perf.dirtyMaxRoundedBytes)
        gPs1Perf.dirtyMaxRoundedBytes = bytes;
}

void ps1PerfMarkDirtyRect(uint16 rows, uint32 exactBytes)
{
    if (!ps1PerfEnabled)
        return;
    gPs1Perf.dirtyRows += rows;
    gPs1Perf.dirtyExactBytes += exactBytes;
    if (rows > gPs1Perf.dirtyMaxRows)
        gPs1Perf.dirtyMaxRows = rows;
    if (exactBytes > gPs1Perf.dirtyMaxExactBytes)
        gPs1Perf.dirtyMaxExactBytes = exactBytes;
}

void ps1PerfMarkRenderTotal(uint16 elapsedVBlanks)
{
    if (ps1PerfLevel < PS1_PERF_LEVEL_DETAIL)
        return;
    gPs1Perf.renderVBlanks += elapsedVBlanks;
    if (elapsedVBlanks > gPs1Perf.maxRenderVBlanks) {
        gPs1Perf.maxRenderVBlanks = elapsedVBlanks;
        gPs1Perf.maxRenderFrameIndex = gPs1Perf.currentFrameIndex;
    }
}

void ps1PerfMarkRenderPhase(uint8 phase, uint16 elapsedVBlanks)
{
    if (ps1PerfLevel < PS1_PERF_LEVEL_DETAIL)
        return;

    switch (phase) {
        case PS1_PERF_RENDER_RESTORE:
            gPs1Perf.restoreVBlanks += elapsedVBlanks;
            if (elapsedVBlanks > 0)
                gPs1Perf.crossedRestore++;
            break;
        case PS1_PERF_RENDER_COMPOSE:
            gPs1Perf.composeVBlanks += elapsedVBlanks;
            if (elapsedVBlanks > 0)
                gPs1Perf.crossedCompose++;
            break;
        case PS1_PERF_RENDER_PRESENT_WAIT:
            gPs1Perf.presentWaitVBlanks += elapsedVBlanks;
            break;
        case PS1_PERF_RENDER_UPLOAD:
            if (elapsedVBlanks > 0)
                gPs1Perf.crossedUpload++;
            break;
        case PS1_PERF_RENDER_EVENT_WAIT:
            gPs1Perf.eventWaitVBlanks += elapsedVBlanks;
            break;
        case PS1_PERF_RENDER_ADVANCE:
            gPs1Perf.advanceVBlanks += elapsedVBlanks;
            if (elapsedVBlanks > 0)
                gPs1Perf.crossedAdvance++;
            break;
        default:
            break;
    }
}

void ps1PerfMarkBufferSizes(uint32 frameBufferBytes, uint32 scratchBytes)
{
    if (!ps1PerfEnabled)
        return;
    gPs1Perf.frameBufferBytes = frameBufferBytes;
    gPs1Perf.scratchBytes = scratchBytes;
}

void ps1PerfMarkAllocFail(uint32 bytes)
{
    if (!ps1PerfEnabled)
        return;
    gPs1Perf.allocFailures++;
    gPs1Perf.allocFailBytes = bytes;
}

void ps1PerfMarkSoundEvent(void)
{
    if (ps1PerfEnabled)
        gPs1Perf.soundEvents++;
}

void ps1PerfMarkSoundCursor(uint16 cursor)
{
    if (ps1PerfEnabled)
        gPs1Perf.soundCursorEnd = cursor;
}

void ps1PerfMarkTripwire(void)
{
    if (ps1PerfEnabled)
        gPs1Perf.tripwires++;
}

void ps1PerfMarkFallback(void)
{
    if (ps1PerfEnabled)
        gPs1Perf.fallbacks++;
}

void ps1PerfMarkFullFallback(void)
{
    if (ps1PerfEnabled)
        gPs1Perf.fullFallbacks++;
}

static void ps1PerfPrintLegacy(uint16 totalSceneVBlanks)
{
    printf(
        "JCPERF scene-end scene=%s scene_vb=%u render=%lu held=%lu entries=%lu late=%lu max_elapsed=%u\n",
        gPs1Perf.sceneName,
        (unsigned int)totalSceneVBlanks,
        (unsigned long)gPs1Perf.renderedLoops,
        (unsigned long)gPs1Perf.heldLoops,
        (unsigned long)gPs1Perf.entries,
        (unsigned long)gPs1Perf.lateAdvances,
        (unsigned int)gPs1Perf.maxElapsedVBlanks
    );
    printf(
        "JCPERF timing advances=%lu elapsed_vb=%lu target_vb=%lu max_hold=%u payload=%lu max_payload=%u\n",
        (unsigned long)gPs1Perf.advances,
        (unsigned long)gPs1Perf.elapsedVBlanks,
        (unsigned long)gPs1Perf.targetVBlanks,
        (unsigned int)gPs1Perf.maxEntryHoldVBlanks,
        (unsigned long)gPs1Perf.entryPayloadBytes,
        (unsigned int)gPs1Perf.maxEntryPayloadBytes
    );
    printf(
        "JCPERF cd reads=%lu fail=%lu bytes=%lu sectors=%lu max_sectors=%u cd_vb=%lu max_cd_vb=%u\n",
        (unsigned long)gPs1Perf.legacyCdReads,
        (unsigned long)gPs1Perf.legacyCdReadFailures,
        (unsigned long)gPs1Perf.legacyCdBytes,
        (unsigned long)gPs1Perf.legacyCdSectors,
        (unsigned int)gPs1Perf.legacyMaxCdSectors,
        (unsigned long)gPs1Perf.legacyCdElapsedVBlanks,
        (unsigned int)gPs1Perf.legacyMaxCdElapsedVBlanks
    );
    printf(
        "JCPERF gfx restore_calls=%lu restore_bytes=%lu compose_calls=%lu rows=%lu spans=%lu pixels=%lu payload=%lu uploads=%lu rects=%lu upload_bytes=%lu upload_vb=%lu max_upload_vb=%u\n",
        (unsigned long)gPs1Perf.restoreCalls,
        (unsigned long)gPs1Perf.restoreBytes,
        (unsigned long)gPs1Perf.composeCalls,
        (unsigned long)gPs1Perf.composeRows,
        (unsigned long)gPs1Perf.composeSpans,
        (unsigned long)gPs1Perf.composePixels,
        (unsigned long)gPs1Perf.composePayloadBytes,
        (unsigned long)gPs1Perf.uploadCalls,
        (unsigned long)gPs1Perf.uploadRects,
        (unsigned long)gPs1Perf.uploadBytes,
        (unsigned long)gPs1Perf.uploadElapsedVBlanks,
        (unsigned int)gPs1Perf.maxUploadElapsedVBlanks
    );
}

static void ps1PerfPrintSchema2(uint32 sceneVBlanks, uint32 loopVBlanks,
                                uint32 setupVBlanks, uint32 cleanupVBlanks)
{
    uint32 overrunVBlanks = (loopVBlanks > gPs1Perf.targetVBlanks)
        ? (loopVBlanks - gPs1Perf.targetVBlanks)
        : 0;

    printf(
        "JCPERF2 scene schema=2 level=%u scene=%s pack=%s pack_bytes=%lu pack_padding=%lu pack_lba=%lu pack_sectors=%lu fmt=%s flags=0x%04x base_diff=%u scene_relative=%u frames=%u entries=%u sounds=%u lowtide=%d night=%d holiday=%d raft=%d pos=%d,%d seed=0\n",
        (unsigned int)ps1PerfLevel,
        gPs1Perf.sceneName,
        gPs1Perf.packPath,
        (unsigned long)gPs1Perf.packBytes,
        (unsigned long)gPs1Perf.packPadding,
        (unsigned long)((gPs1Perf.packLba == PS1_PERF_UNKNOWN_LBA) ? 0 : gPs1Perf.packLba),
        (unsigned long)gPs1Perf.packSectors,
        ps1PerfFormatName(),
        (unsigned int)gPs1Perf.packFlags,
        (unsigned int)((gPs1Perf.packFlags & 0x0010u) ? 1 : 0),
        (unsigned int)((gPs1Perf.packFlags & 0x0008u) ? 1 : 0),
        (unsigned int)gPs1Perf.packFrameCount,
        (unsigned int)gPs1Perf.packEntryCount,
        (unsigned int)gPs1Perf.packSoundCount,
        islandState.lowTide,
        islandState.night,
        islandState.holiday,
        islandState.raft,
        islandState.xPos,
        islandState.yPos
    );
    printf(
        "JCPERF2 timing scene_start=%lu loop_start=%lu loop_end=%lu scene_end=%lu scene_vb=%lu loop_vb=%lu target_vb=%lu overrun_vb=%lu advances=%lu render=%lu held=%lu entries=%lu empty=%lu late=%lu max_elapsed=%u max_elapsed_idx=%u\n",
        (unsigned long)gPs1Perf.sceneStartTick,
        (unsigned long)gPs1Perf.loopStartTick,
        (unsigned long)gPs1Perf.loopEndTick,
        (unsigned long)gPs1Perf.sceneEndTick,
        (unsigned long)sceneVBlanks,
        (unsigned long)loopVBlanks,
        (unsigned long)gPs1Perf.targetVBlanks,
        (unsigned long)overrunVBlanks,
        (unsigned long)gPs1Perf.advances,
        (unsigned long)gPs1Perf.renderedLoops,
        (unsigned long)gPs1Perf.heldLoops,
        (unsigned long)gPs1Perf.entries,
        (unsigned long)gPs1Perf.emptyEntries,
        (unsigned long)gPs1Perf.lateAdvances,
        (unsigned int)gPs1Perf.maxElapsedVBlanks,
        (unsigned int)gPs1Perf.maxElapsedFrameIndex
    );
    printf(
        "JCPERF2 setup setup_vb=%lu screen_vb=%lu backdrop_vb=%lu pack_start_vb=%lu clean_rect_vb=%lu first_frame_vb=%lu cleanup_vb=%lu setup_reads=%lu setup_bytes=%lu\n",
        (unsigned long)setupVBlanks,
        (unsigned long)gPs1Perf.screenVBlanks,
        (unsigned long)gPs1Perf.backdropVBlanks,
        (unsigned long)gPs1Perf.packStartVBlanks,
        (unsigned long)gPs1Perf.cleanRectVBlanks,
        (unsigned long)gPs1Perf.firstFrameVBlanks,
        (unsigned long)cleanupVBlanks,
        (unsigned long)gPs1Perf.cdSetupReads,
        (unsigned long)gPs1Perf.cdSetupBytes
    );
    printf(
        "JCPERF2 frame payload=%lu max_payload=%lu max_payload_idx=%u max_payload_src=%u rows=%lu spans=%lu pixels=%lu hold_max=%u hold_max_idx=%u hold_1=%lu hold_2_4=%lu hold_5_8=%lu hold_9p=%lu payload_0=%lu payload_1k=%lu payload_4k=%lu payload_16k=%lu payload_64k=%lu payload_64kp=%lu\n",
        (unsigned long)gPs1Perf.entryPayloadBytes,
        (unsigned long)gPs1Perf.maxEntryPayloadBytes32,
        (unsigned int)gPs1Perf.maxEntryPayloadIndex,
        (unsigned int)gPs1Perf.maxEntryPayloadSource,
        (unsigned long)gPs1Perf.composeRows,
        (unsigned long)gPs1Perf.composeSpans,
        (unsigned long)gPs1Perf.composePixels,
        (unsigned int)gPs1Perf.maxEntryHoldVBlanks,
        (unsigned int)gPs1Perf.maxEntryHoldIndex,
        (unsigned long)gPs1Perf.hold1,
        (unsigned long)gPs1Perf.hold2To4,
        (unsigned long)gPs1Perf.hold5To8,
        (unsigned long)gPs1Perf.hold9Plus,
        (unsigned long)gPs1Perf.payload0,
        (unsigned long)gPs1Perf.payload1k,
        (unsigned long)gPs1Perf.payload4k,
        (unsigned long)gPs1Perf.payload16k,
        (unsigned long)gPs1Perf.payload64k,
        (unsigned long)gPs1Perf.payload64kPlus
    );
    printf(
        "JCPERF2 cd reads=%lu setup_reads=%lu loop_reads=%lu fail=%lu setloc=%lu bytes=%lu sectors=%lu read_vb=%lu setup_read_vb=%lu loop_read_vb=%lu blocking_vb=%lu hidden_vb=%lu max_read_vb=%u max_read_idx=%u max_read_sectors=%u unaligned_start=%lu unaligned_end=%lu overread_bytes=%lu scratch_bytes=%lu seq=%lu seek_fwd=%lu seek_back=%lu max_gap=%lu s1=%lu s2=%lu s3_4=%lu s5_8=%lu s9p=%lu\n",
        (unsigned long)gPs1Perf.cdReads,
        (unsigned long)gPs1Perf.cdSetupReads,
        (unsigned long)gPs1Perf.cdLoopReads,
        (unsigned long)gPs1Perf.cdReadFailures,
        (unsigned long)gPs1Perf.cdSetloc,
        (unsigned long)gPs1Perf.cdBytes,
        (unsigned long)gPs1Perf.cdSectors,
        (unsigned long)gPs1Perf.cdElapsedVBlanks,
        (unsigned long)gPs1Perf.cdSetupVBlanks,
        (unsigned long)gPs1Perf.cdLoopVBlanks,
        (unsigned long)gPs1Perf.cdBlockingVBlanks,
        (unsigned long)gPs1Perf.cdHiddenVBlanks,
        (unsigned int)gPs1Perf.maxCdElapsedVBlanks,
        (unsigned int)gPs1Perf.maxCdReadIndex,
        (unsigned int)gPs1Perf.maxCdSectors,
        (unsigned long)gPs1Perf.cdUnalignedStart,
        (unsigned long)gPs1Perf.cdUnalignedEnd,
        (unsigned long)gPs1Perf.cdOverreadBytes,
        (unsigned long)gPs1Perf.cdScratchBytes,
        (unsigned long)gPs1Perf.cdSequentialReads,
        (unsigned long)gPs1Perf.cdSeekForwardReads,
        (unsigned long)gPs1Perf.cdSeekBackwardReads,
        (unsigned long)gPs1Perf.cdMaxSectorGap,
        (unsigned long)gPs1Perf.cdSector1,
        (unsigned long)gPs1Perf.cdSector2,
        (unsigned long)gPs1Perf.cdSector3To4,
        (unsigned long)gPs1Perf.cdSector5To8,
        (unsigned long)gPs1Perf.cdSector9Plus
    );
    printf(
        "JCPERF2 prefetch policy=%s buf=%lu attempts=%lu eligible=%lu ineligible=%lu hits=%lu misses=%lu due_misses=%lu stage_hits=%lu window_hits=0 group_hits=0 partial_hits=0 hidden_reads=%lu blocking_reads=%lu slack_vb=%lu used_vb=%lu overrun_vb=%lu lead_min=%u lead_max=%u skipped_no_slack=%lu skipped_busy=0 duplicate=%lu wasted_bytes=%lu\n",
        ps1PerfPrefetchPolicyName(),
        (unsigned long)gPs1Perf.prefetchBytes,
        (unsigned long)gPs1Perf.prefetchAttempts,
        (unsigned long)gPs1Perf.prefetchEligible,
        (unsigned long)gPs1Perf.prefetchIneligible,
        (unsigned long)gPs1Perf.prefetchHits,
        (unsigned long)gPs1Perf.prefetchMisses,
        (unsigned long)gPs1Perf.prefetchDueMisses,
        (unsigned long)gPs1Perf.prefetchStageHits,
        (unsigned long)gPs1Perf.cdHiddenReads,
        (unsigned long)gPs1Perf.cdBlockingReads,
        (unsigned long)gPs1Perf.prefetchSlackVBlanks,
        (unsigned long)gPs1Perf.prefetchUsedVBlanks,
        (unsigned long)gPs1Perf.prefetchOverrunVBlanks,
        (unsigned int)gPs1Perf.prefetchLeadMin,
        (unsigned int)gPs1Perf.prefetchLeadMax,
        (unsigned long)gPs1Perf.prefetchSkippedNoSlack,
        (unsigned long)gPs1Perf.prefetchDuplicate,
        (unsigned long)gPs1Perf.prefetchWastedBytes
    );
    printf(
        "JCPERF2 async async_start=0 async_poll=0 async_done=0 async_timeout=0 async_cancel=0 async_blocking_vb=0\n"
    );
    printf(
        "JCPERF2 render render_vb=%lu max_render_vb=%u max_render_idx=%u restore_vb=%lu compose_vb=%lu present_wait_vb=%lu upload_vb=%lu event_wait_vb=%lu advance_vb=%lu crossed_restore=%lu crossed_compose=%lu crossed_upload=%lu crossed_advance=%lu\n",
        (unsigned long)gPs1Perf.renderVBlanks,
        (unsigned int)gPs1Perf.maxRenderVBlanks,
        (unsigned int)gPs1Perf.maxRenderFrameIndex,
        (unsigned long)gPs1Perf.restoreVBlanks,
        (unsigned long)gPs1Perf.composeVBlanks,
        (unsigned long)gPs1Perf.presentWaitVBlanks,
        (unsigned long)gPs1Perf.uploadElapsedVBlanks,
        (unsigned long)gPs1Perf.eventWaitVBlanks,
        (unsigned long)gPs1Perf.advanceVBlanks,
        (unsigned long)gPs1Perf.crossedRestore,
        (unsigned long)gPs1Perf.crossedCompose,
        (unsigned long)gPs1Perf.crossedUpload,
        (unsigned long)gPs1Perf.crossedAdvance
    );
    printf(
        "JCPERF2 gfx restore_calls=%lu restore_bytes=%lu max_restore_bytes=%lu compose_calls=%lu upload_calls=%lu upload_rects=%lu upload_bytes=%lu max_upload_bytes=%lu max_upload_rects=%u dirty_rows=%lu dirty_exact_bytes=%lu dirty_rounded_bytes=%lu dirty_max_rows=%lu dirty_max_exact=%lu dirty_max_rounded=%lu cap_hits=%lu full_fallbacks=%lu\n",
        (unsigned long)gPs1Perf.restoreCalls,
        (unsigned long)gPs1Perf.restoreBytes,
        (unsigned long)gPs1Perf.maxRestoreBytes,
        (unsigned long)gPs1Perf.composeCalls,
        (unsigned long)gPs1Perf.uploadCalls,
        (unsigned long)gPs1Perf.uploadRects,
        (unsigned long)gPs1Perf.uploadBytes,
        (unsigned long)gPs1Perf.maxUploadBytes,
        (unsigned int)gPs1Perf.maxUploadRects,
        (unsigned long)gPs1Perf.dirtyRows,
        (unsigned long)gPs1Perf.dirtyExactBytes,
        (unsigned long)gPs1Perf.dirtyRoundedBytes,
        (unsigned long)gPs1Perf.dirtyMaxRows,
        (unsigned long)gPs1Perf.dirtyMaxExactBytes,
        (unsigned long)gPs1Perf.dirtyMaxRoundedBytes,
        (unsigned long)gPs1Perf.dirtyCapHits,
        (unsigned long)gPs1Perf.fullFallbacks
    );
    printf(
        "JCPERF2 heap start_free=0 end_free=0 min_free=0 largest_start=0 largest_end=0 framebuf=%lu scratch=%lu prefetch=%lu peak_prefetch=%lu alloc_fail=%lu alloc_fail_bytes=%lu\n",
        (unsigned long)gPs1Perf.frameBufferBytes,
        (unsigned long)gPs1Perf.scratchBytes,
        (unsigned long)gPs1Perf.prefetchBytes,
        (unsigned long)gPs1Perf.peakPrefetchBytes,
        (unsigned long)gPs1Perf.allocFailures,
        (unsigned long)gPs1Perf.allocFailBytes
    );
    printf(
        "JCPERF2 correctness trip=%lu fallback=%lu stale_guard=%lu frame_mismatch=%lu sound_events=%lu sound_late=%lu sound_cursor_end=%u last_frame=%u last_source=%u expected_frames=%u cd_fail=%lu\n",
        (unsigned long)gPs1Perf.tripwires,
        (unsigned long)gPs1Perf.fallbacks,
        (unsigned long)gPs1Perf.staleGuards,
        (unsigned long)gPs1Perf.frameMismatches,
        (unsigned long)gPs1Perf.soundEvents,
        (unsigned long)gPs1Perf.soundLate,
        (unsigned int)gPs1Perf.soundCursorEnd,
        (unsigned int)gPs1Perf.lastFrameIndex,
        (unsigned int)gPs1Perf.lastSourceFrame,
        (unsigned int)gPs1Perf.packFrameCount,
        (unsigned long)gPs1Perf.cdReadFailures
    );
}

void ps1PerfEndScene(const char *sceneName)
{
    uint32 totalSceneVBlanks;
    uint32 loopVBlanks = 0;
    uint32 setupVBlanks = 0;
    uint32 cleanupVBlanks = 0;

    if (!ps1PerfEnabled)
        return;

    if (sceneName != NULL && sceneName[0] != '\0')
        ps1PerfCopySceneName(sceneName);
    gPs1Perf.sceneEndTick = ps1PerfTick();

    if (gPs1Perf.loopEndTick == 0)
        gPs1Perf.loopEndTick = gPs1Perf.sceneEndTick;
    if (gPs1Perf.loopStartTick != 0) {
        loopVBlanks = ps1PerfTickDiff(gPs1Perf.loopStartTick, gPs1Perf.loopEndTick);
        setupVBlanks = ps1PerfTickDiff(gPs1Perf.sceneStartTick, gPs1Perf.loopStartTick);
    }
    if (gPs1Perf.cleanupStartTick != 0)
        cleanupVBlanks = ps1PerfTickDiff(gPs1Perf.cleanupStartTick, gPs1Perf.sceneEndTick);
    totalSceneVBlanks = ps1PerfTickDiff(gPs1Perf.sceneStartTick, gPs1Perf.sceneEndTick);

    ps1PerfPrintLegacy(ps1PerfClampU16(totalSceneVBlanks));
    ps1PerfPrintSchema2(totalSceneVBlanks, loopVBlanks, setupVBlanks, cleanupVBlanks);
}

#endif
