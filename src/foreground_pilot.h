#ifndef FOREGROUND_PILOT_H
#define FOREGROUND_PILOT_H

int foregroundPilotRequested(void);
const char *foregroundPilotSceneName(void);
void foregroundPilotSetScene(const char *sceneName);
void foregroundPilotSetHeapProbe(int enabled);
void foregroundPilotResetPrefetchDefaults(void);
void foregroundPilotSetPrefetchStage1(int enabled);
void foregroundPilotSetPrefetchWindow(unsigned long bytes);
void foregroundPilotPlay(void);
int foregroundPilotShouldStartForAds(const char *adsName, unsigned short adsTag);
int foregroundPilotRuntimeStartRequested(void);
int foregroundPilotRuntimeStartIfRequested(void);
int foregroundPilotRuntimeStart(const char *sceneName);
void foregroundPilotRuntimeCompose(void);
void foregroundPilotRuntimeAdvance(void);
int foregroundPilotRuntimeActive(void);
int foregroundPilotRuntimeMode(void);
const char *foregroundPilotRuntimeModeName(void);
const char *foregroundPilotRuntimeSceneName(void);
unsigned short foregroundPilotRuntimeFrameIndex(void);
unsigned short foregroundPilotRuntimeFrameCount(void);
/* Binary-search heap probe — returns size in bytes of the largest
 * contiguous malloc that succeeds right now. Used by the pause menu's
 * Debug Info to show real free RAM. */
unsigned long fgProbeLargestAlloc(void);
unsigned short foregroundPilotRuntimeSourceFrame(void);
unsigned short foregroundPilotRuntimeDisplayVBlanks(void);
int foregroundPilotRuntimeHasFrameData(void);
int foregroundPilotRequestedNow(void);
void foregroundPilotRuntimeEnd(void);

#endif
