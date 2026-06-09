#ifndef PS1_PERF_H
#define PS1_PERF_H

#include "mytypes.h"

#ifndef PS1_PERF_DEEP_TRACE
#define PS1_PERF_DEEP_TRACE 0
#endif

#ifdef PS1_BUILD

enum {
    PS1_PERF_LEVEL_OFF = 0,
    PS1_PERF_LEVEL_SUMMARY = 1,
    PS1_PERF_LEVEL_DETAIL = 2,
    PS1_PERF_LEVEL_DEBUG = 3
};

enum {
    PS1_PERF_SETUP_SCREEN = 1,
    PS1_PERF_SETUP_BACKDROP = 2,
    PS1_PERF_SETUP_PACK_START = 3,
    PS1_PERF_SETUP_CLEAN_RECT = 4,
    PS1_PERF_SETUP_FIRST_FRAME = 5
};

/* Next-scene stage adoption flags: which transition costs were satisfied
 * from the lookahead stage instead of cold CD work. OR-combined per scene. */
enum {
    PS1_PERF_STAGE_ADOPT_FILE = 0x01,     /* pack CdlFILE adopted (no CdSearchFile) */
    PS1_PERF_STAGE_ADOPT_METADATA = 0x02, /* metadata prefix read from SPU stage */
    PS1_PERF_STAGE_ADOPT_WINDOW = 0x04    /* first payload window adopted */
};

enum {
    PS1_PERF_RENDER_RESTORE = 1,
    PS1_PERF_RENDER_COMPOSE = 2,
    PS1_PERF_RENDER_PRESENT_WAIT = 3,
    PS1_PERF_RENDER_UPLOAD = 4,
    PS1_PERF_RENDER_EVENT_WAIT = 5,
    PS1_PERF_RENDER_ADVANCE = 6
};

enum {
    PS1_PERF_PREFETCH_NONE = 0,
    PS1_PERF_PREFETCH_STAGE1 = 1,
    PS1_PERF_PREFETCH_WINDOW = 2,
    PS1_PERF_PREFETCH_STAGE1_WINDOW = 3
};

enum {
    PS1_PERF_SCHED_WAIT = 0,
    PS1_PERF_SCHED_PRESENT = 1,
    PS1_PERF_SCHED_CD_STAGE = 2,
    PS1_PERF_SCHED_CD_WINDOW = 3,
    PS1_PERF_SCHED_VISUAL_PREPARE = 4,
    PS1_PERF_SCHED_CD_RESERVED = 5,
    PS1_PERF_SCHED_PREP_BLOCKED_CD = 6,
    PS1_PERF_SCHED_PREPARED_READY = 7,
    PS1_PERF_SCHED_PREPARED_USED = 8,
    PS1_PERF_SCHED_PREPARED_WASTED = 9
};

enum {
    PS1_PERF_PIPE_NONE = 0,
    PS1_PERF_PIPE_DUE_RENDER = 1,
    PS1_PERF_PIPE_PREPARE = 2,
    PS1_PERF_PIPE_PREPARED_PRESENT = 3
};

extern volatile uint8 ps1PerfEnabled;
extern volatile uint8 ps1PerfLevel;

void ps1PerfSetEnabled(int enabled);
void ps1PerfSetLevel(int level);
int ps1PerfDetailEnabled(void);
void ps1PerfBeginScene(const char *sceneName);
void ps1PerfEndScene(const char *sceneName);
uint32 ps1PerfTick(void);
uint16 ps1PerfElapsedVBlanks(uint32 startTick);
void ps1PerfMarkSetupPhase(uint8 phase, uint16 elapsedVBlanks);
void ps1PerfMarkStageAdopt(uint8 flags);
void ps1PerfMarkLoopStart(void);
void ps1PerfMarkLoopEnd(void);
void ps1PerfMarkCleanupStart(void);
void ps1PerfSetPackInfo(const char *packPath, uint32 packBytes,
                        uint32 packLba, uint32 packSectors,
                        uint16 frameCount, uint16 entryCount,
                        uint16 soundCount, uint16 flags,
                        uint8 packFormat, uint32 frameBufferBytes,
                        uint32 scratchBytes);
void ps1PerfSetCurrentFrame(uint16 frameIndex, uint16 sourceFrame,
                            uint32 dataOffset);
void ps1PerfMarkRenderedLoop(void);
void ps1PerfMarkHeldLoop(void);
void ps1PerfMarkAdvance(uint16 elapsedVBlanks);
void ps1PerfMarkEntry(uint32 payloadBytes, uint16 holdVBlanks,
                      uint8 emptyEntry, uint16 sourceFrame,
                      uint32 dataOffset);
void ps1PerfMarkCdRead(uint32 bytes, uint32 sectors, uint16 elapsedVBlanks, int ok);
void ps1PerfMarkCdReadDetailed(uint32 bytes, uint32 sectors,
                               uint16 elapsedVBlanks, int ok,
                               uint32 fileLba, uint32 fileOffset,
                               uint8 includeLegacy);
void ps1PerfSetPrefetchPolicy(uint8 policy, uint32 bufferBytes);
void ps1PerfMarkPrefetchAttempt(uint16 leadVBlanks, uint16 slackVBlanks,
                                uint8 eligible);
void ps1PerfMarkPrefetchSkipNoSlack(void);
void ps1PerfMarkPrefetchDuplicate(void);
void ps1PerfBeginPrefetchRead(uint16 slackVBlanks);
void ps1PerfEndPrefetchRead(uint16 elapsedVBlanks, uint32 bytes, int ok);
void ps1PerfMarkPrefetchHit(void);
void ps1PerfMarkPrefetchWindowHit(uint8 countsAsDueHit);
void ps1PerfMarkScheduler(uint8 event, uint16 slackVBlanks);
void ps1PerfMarkRestore(uint32 bytes);
void ps1PerfMarkCompose(uint16 rows, uint16 spans, uint32 pixels, uint32 payloadBytes);
void ps1PerfMarkUploadDirty(uint16 rects, uint16 rows, uint32 bytes, uint16 elapsedVBlanks);
void ps1PerfMarkRenderTotal(uint16 elapsedVBlanks);
void ps1PerfMarkRenderPhase(uint8 phase, uint16 elapsedVBlanks);
#if PS1_PERF_DEEP_TRACE
void ps1PerfBeginPipeline(uint8 path);
void ps1PerfEndPipeline(uint8 path, uint16 elapsedVBlanks);
#else
static inline void ps1PerfBeginPipeline(uint8 path) { (void)path; }
static inline void ps1PerfEndPipeline(uint8 path, uint16 elapsedVBlanks) { (void)path; (void)elapsedVBlanks; }
#endif
void ps1PerfMarkBufferSizes(uint32 frameBufferBytes, uint32 scratchBytes);
void ps1PerfMarkAllocFail(uint32 bytes);
void ps1PerfMarkSoundEvent(void);
void ps1PerfMarkSoundCursor(uint16 cursor);
void ps1PerfMarkTripwire(void);
void ps1PerfMarkFallback(void);
void ps1PerfMarkFullFallback(void);

#else

#define ps1PerfEnabled 0
#define ps1PerfLevel 0
static inline void ps1PerfSetEnabled(int enabled) { (void)enabled; }
static inline void ps1PerfSetLevel(int level) { (void)level; }
static inline int ps1PerfDetailEnabled(void) { return 0; }
static inline void ps1PerfBeginScene(const char *sceneName) { (void)sceneName; }
static inline void ps1PerfEndScene(const char *sceneName) { (void)sceneName; }
static inline uint32 ps1PerfTick(void) { return 0; }
static inline uint16 ps1PerfElapsedVBlanks(uint32 startTick) { (void)startTick; return 0; }
static inline void ps1PerfMarkSetupPhase(uint8 phase, uint16 elapsedVBlanks) { (void)phase; (void)elapsedVBlanks; }
static inline void ps1PerfMarkStageAdopt(uint8 flags) { (void)flags; }
static inline void ps1PerfMarkLoopStart(void) {}
static inline void ps1PerfMarkLoopEnd(void) {}
static inline void ps1PerfMarkCleanupStart(void) {}
static inline void ps1PerfSetPackInfo(const char *packPath, uint32 packBytes,
                                      uint32 packLba, uint32 packSectors,
                                      uint16 frameCount, uint16 entryCount,
                                      uint16 soundCount, uint16 flags,
                                      uint8 packFormat, uint32 frameBufferBytes,
                                      uint32 scratchBytes) {
    (void)packPath; (void)packBytes; (void)packLba; (void)packSectors;
    (void)frameCount; (void)entryCount; (void)soundCount; (void)flags;
    (void)packFormat; (void)frameBufferBytes; (void)scratchBytes;
}
static inline void ps1PerfSetCurrentFrame(uint16 frameIndex, uint16 sourceFrame,
                                          uint32 dataOffset) {
    (void)frameIndex; (void)sourceFrame; (void)dataOffset;
}
static inline void ps1PerfMarkRenderedLoop(void) {}
static inline void ps1PerfMarkHeldLoop(void) {}
static inline void ps1PerfMarkAdvance(uint16 elapsedVBlanks) { (void)elapsedVBlanks; }
static inline void ps1PerfMarkEntry(uint32 payloadBytes, uint16 holdVBlanks,
                                    uint8 emptyEntry, uint16 sourceFrame,
                                    uint32 dataOffset) {
    (void)payloadBytes; (void)holdVBlanks; (void)emptyEntry;
    (void)sourceFrame; (void)dataOffset;
}
static inline void ps1PerfMarkCdRead(uint32 bytes, uint32 sectors, uint16 elapsedVBlanks, int ok) { (void)bytes; (void)sectors; (void)elapsedVBlanks; (void)ok; }
static inline void ps1PerfMarkCdReadDetailed(uint32 bytes, uint32 sectors,
                                             uint16 elapsedVBlanks, int ok,
                                             uint32 fileLba, uint32 fileOffset,
                                             uint8 includeLegacy) {
    (void)bytes; (void)sectors; (void)elapsedVBlanks; (void)ok;
    (void)fileLba; (void)fileOffset; (void)includeLegacy;
}
static inline void ps1PerfSetPrefetchPolicy(uint8 policy, uint32 bufferBytes) { (void)policy; (void)bufferBytes; }
static inline void ps1PerfMarkPrefetchAttempt(uint16 leadVBlanks, uint16 slackVBlanks,
                                              uint8 eligible) {
    (void)leadVBlanks; (void)slackVBlanks; (void)eligible;
}
static inline void ps1PerfMarkPrefetchSkipNoSlack(void) {}
static inline void ps1PerfMarkPrefetchDuplicate(void) {}
static inline void ps1PerfBeginPrefetchRead(uint16 slackVBlanks) { (void)slackVBlanks; }
static inline void ps1PerfEndPrefetchRead(uint16 elapsedVBlanks, uint32 bytes, int ok) { (void)elapsedVBlanks; (void)bytes; (void)ok; }
static inline void ps1PerfMarkPrefetchHit(void) {}
static inline void ps1PerfMarkPrefetchWindowHit(uint8 countsAsDueHit) { (void)countsAsDueHit; }
static inline void ps1PerfMarkScheduler(uint8 event, uint16 slackVBlanks) { (void)event; (void)slackVBlanks; }
static inline void ps1PerfMarkRestore(uint32 bytes) { (void)bytes; }
static inline void ps1PerfMarkCompose(uint16 rows, uint16 spans, uint32 pixels, uint32 payloadBytes) { (void)rows; (void)spans; (void)pixels; (void)payloadBytes; }
static inline void ps1PerfMarkUploadDirty(uint16 rects, uint16 rows, uint32 bytes, uint16 elapsedVBlanks) { (void)rects; (void)rows; (void)bytes; (void)elapsedVBlanks; }
static inline void ps1PerfMarkRenderTotal(uint16 elapsedVBlanks) { (void)elapsedVBlanks; }
static inline void ps1PerfMarkRenderPhase(uint8 phase, uint16 elapsedVBlanks) { (void)phase; (void)elapsedVBlanks; }
static inline void ps1PerfBeginPipeline(uint8 path) { (void)path; }
static inline void ps1PerfEndPipeline(uint8 path, uint16 elapsedVBlanks) { (void)path; (void)elapsedVBlanks; }
static inline void ps1PerfMarkBufferSizes(uint32 frameBufferBytes, uint32 scratchBytes) { (void)frameBufferBytes; (void)scratchBytes; }
static inline void ps1PerfMarkAllocFail(uint32 bytes) { (void)bytes; }
static inline void ps1PerfMarkSoundEvent(void) {}
static inline void ps1PerfMarkSoundCursor(uint16 cursor) { (void)cursor; }
static inline void ps1PerfMarkTripwire(void) {}
static inline void ps1PerfMarkFallback(void) {}
static inline void ps1PerfMarkFullFallback(void) {}

#endif

#endif
