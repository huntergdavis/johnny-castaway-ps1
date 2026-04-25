#include "ps1_perf.h"

#ifdef PS1_BUILD

#include <stdio.h>
#include <string.h>
#include <psxapi.h>
#include <psxgpu.h>

struct TTtmThread;
#include "island.h"

volatile uint8 ps1PerfEnabled = 0;

struct TPs1PerfCounters {
    char sceneName[16];
    uint32 startTick;
    uint32 renderedLoops;
    uint32 heldLoops;
    uint32 advances;
    uint32 elapsedVBlanks;
    uint32 targetVBlanks;
    uint32 lateAdvances;
    uint16 maxElapsedVBlanks;
    uint32 entries;
    uint32 entryPayloadBytes;
    uint16 maxEntryPayloadBytes;
    uint16 maxEntryHoldVBlanks;
    uint32 cdReads;
    uint32 cdReadFailures;
    uint32 cdBytes;
    uint32 cdSectors;
    uint16 maxCdSectors;
    uint32 cdElapsedVBlanks;
    uint16 maxCdElapsedVBlanks;
    uint32 restoreCalls;
    uint32 restoreBytes;
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
};

static struct TPs1PerfCounters gPs1Perf;

static void ps1PerfCopySceneName(const char *sceneName)
{
    if (sceneName == NULL)
        sceneName = "?";
    strncpy(gPs1Perf.sceneName, sceneName, sizeof(gPs1Perf.sceneName) - 1);
    gPs1Perf.sceneName[sizeof(gPs1Perf.sceneName) - 1] = '\0';
}

void ps1PerfSetEnabled(int enabled)
{
    ps1PerfEnabled = enabled ? 1 : 0;
    if (!ps1PerfEnabled)
        memset(&gPs1Perf, 0, sizeof(gPs1Perf));
}

uint32 ps1PerfTick(void)
{
    return (uint32)VSync(-1);
}

uint16 ps1PerfElapsedVBlanks(uint32 startTick)
{
    uint32 nowTick = ps1PerfTick();
    uint32 elapsed = (nowTick >= startTick) ? (nowTick - startTick) : 0;
    return (uint16)((elapsed > 0xffffu) ? 0xffffu : elapsed);
}

void ps1PerfBeginScene(const char *sceneName)
{
    if (!ps1PerfEnabled)
        return;

    memset(&gPs1Perf, 0, sizeof(gPs1Perf));
    ps1PerfCopySceneName(sceneName);
    gPs1Perf.startTick = ps1PerfTick();

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

void ps1PerfEndScene(const char *sceneName)
{
    uint16 totalSceneVBlanks;

    if (!ps1PerfEnabled)
        return;

    if (sceneName != NULL && sceneName[0] != '\0')
        ps1PerfCopySceneName(sceneName);
    totalSceneVBlanks = ps1PerfElapsedVBlanks(gPs1Perf.startTick);

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
        (unsigned long)gPs1Perf.cdReads,
        (unsigned long)gPs1Perf.cdReadFailures,
        (unsigned long)gPs1Perf.cdBytes,
        (unsigned long)gPs1Perf.cdSectors,
        (unsigned int)gPs1Perf.maxCdSectors,
        (unsigned long)gPs1Perf.cdElapsedVBlanks,
        (unsigned int)gPs1Perf.maxCdElapsedVBlanks
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
    if (elapsedVBlanks > gPs1Perf.maxElapsedVBlanks)
        gPs1Perf.maxElapsedVBlanks = elapsedVBlanks;
}

void ps1PerfMarkEntry(uint32 payloadBytes, uint16 holdVBlanks)
{
    if (!ps1PerfEnabled)
        return;
    gPs1Perf.entries++;
    gPs1Perf.entryPayloadBytes += payloadBytes;
    gPs1Perf.targetVBlanks += holdVBlanks;
    if (payloadBytes > gPs1Perf.maxEntryPayloadBytes)
        gPs1Perf.maxEntryPayloadBytes = (uint16)((payloadBytes > 0xffffu) ? 0xffffu : payloadBytes);
    if (holdVBlanks > gPs1Perf.maxEntryHoldVBlanks)
        gPs1Perf.maxEntryHoldVBlanks = holdVBlanks;
}

void ps1PerfMarkCdRead(uint32 bytes, uint32 sectors, uint16 elapsedVBlanks, int ok)
{
    if (!ps1PerfEnabled)
        return;
    gPs1Perf.cdReads++;
    if (!ok)
        gPs1Perf.cdReadFailures++;
    gPs1Perf.cdBytes += bytes;
    gPs1Perf.cdSectors += sectors;
    gPs1Perf.cdElapsedVBlanks += elapsedVBlanks;
    if (sectors > gPs1Perf.maxCdSectors)
        gPs1Perf.maxCdSectors = (uint16)((sectors > 0xffffu) ? 0xffffu : sectors);
    if (elapsedVBlanks > gPs1Perf.maxCdElapsedVBlanks)
        gPs1Perf.maxCdElapsedVBlanks = elapsedVBlanks;
}

void ps1PerfMarkRestore(uint32 bytes)
{
    if (!ps1PerfEnabled)
        return;
    gPs1Perf.restoreCalls++;
    gPs1Perf.restoreBytes += bytes;
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
    if (elapsedVBlanks > gPs1Perf.maxUploadElapsedVBlanks)
        gPs1Perf.maxUploadElapsedVBlanks = elapsedVBlanks;
}

#endif
