#ifndef PS1_PERF_H
#define PS1_PERF_H

#include "mytypes.h"

#ifdef PS1_BUILD

extern volatile uint8 ps1PerfEnabled;

void ps1PerfSetEnabled(int enabled);
void ps1PerfBeginScene(const char *sceneName);
void ps1PerfEndScene(const char *sceneName);
uint32 ps1PerfTick(void);
uint16 ps1PerfElapsedVBlanks(uint32 startTick);
void ps1PerfMarkRenderedLoop(void);
void ps1PerfMarkHeldLoop(void);
void ps1PerfMarkAdvance(uint16 elapsedVBlanks, uint16 targetVBlanks);
void ps1PerfMarkEntry(uint32 payloadBytes, uint16 holdVBlanks);
void ps1PerfMarkCdRead(uint32 bytes, uint32 sectors, uint16 elapsedVBlanks, int ok);
void ps1PerfMarkRestore(uint32 bytes);
void ps1PerfMarkCompose(uint16 rows, uint16 spans, uint32 pixels, uint32 payloadBytes);
void ps1PerfMarkUpload(uint16 rects, uint32 bytes, uint16 elapsedVBlanks);

#else

#define ps1PerfEnabled 0
static inline void ps1PerfSetEnabled(int enabled) { (void)enabled; }
static inline void ps1PerfBeginScene(const char *sceneName) { (void)sceneName; }
static inline void ps1PerfEndScene(const char *sceneName) { (void)sceneName; }
static inline uint32 ps1PerfTick(void) { return 0; }
static inline uint16 ps1PerfElapsedVBlanks(uint32 startTick) { (void)startTick; return 0; }
static inline void ps1PerfMarkRenderedLoop(void) {}
static inline void ps1PerfMarkHeldLoop(void) {}
static inline void ps1PerfMarkAdvance(uint16 elapsedVBlanks, uint16 targetVBlanks) { (void)elapsedVBlanks; (void)targetVBlanks; }
static inline void ps1PerfMarkEntry(uint32 payloadBytes, uint16 holdVBlanks) { (void)payloadBytes; (void)holdVBlanks; }
static inline void ps1PerfMarkCdRead(uint32 bytes, uint32 sectors, uint16 elapsedVBlanks, int ok) { (void)bytes; (void)sectors; (void)elapsedVBlanks; (void)ok; }
static inline void ps1PerfMarkRestore(uint32 bytes) { (void)bytes; }
static inline void ps1PerfMarkCompose(uint16 rows, uint16 spans, uint32 pixels, uint32 payloadBytes) { (void)rows; (void)spans; (void)pixels; (void)payloadBytes; }
static inline void ps1PerfMarkUpload(uint16 rects, uint32 bytes, uint16 elapsedVBlanks) { (void)rects; (void)bytes; (void)elapsedVBlanks; }

#endif

#endif
