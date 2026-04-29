#ifndef FOREGROUND_PILOT_H
#define FOREGROUND_PILOT_H

void foregroundPilotSetScene(const char *sceneName);
void foregroundPilotSetHeapProbe(int enabled);
void foregroundPilotResetPrefetchDefaults(void);
void foregroundPilotSetPrefetchStage1(int enabled);
void foregroundPilotSetPrefetchWindow(unsigned long bytes);
void foregroundPilotPlay(void);
void foregroundPilotRuntimeCompose(void);
void foregroundPilotRuntimeAdvance(void);
int foregroundPilotRuntimeActive(void);
const char *foregroundPilotRuntimeModeName(void);
const char *foregroundPilotRuntimeSceneName(void);
unsigned short foregroundPilotRuntimeFrameIndex(void);
unsigned short foregroundPilotRuntimeFrameCount(void);
/* Binary-search heap probe — returns size in bytes of the largest
 * contiguous malloc that succeeds right now. Used by the pause menu's
 * Debug Info to show real free RAM. */
unsigned long fgProbeLargestAlloc(void);
void foregroundPilotRuntimeEnd(void);

/* Access to the island-background TTtmSlot that fg_pilot owns.
 * Used by walk_pilot.c (and freeplay) to source the trunk + leaf
 * cover-up sprites for behind-tree compositing. Returns NULL if the
 * backdrop hasn't been loaded yet (caller should fall back to
 * tree-front compositing being skipped — Johnny just renders on
 * top of the tree). Defined in foreground_pilot.c. */
struct TTtmSlot;
struct TTtmSlot *fgBackdropGetSlot(void);

/* Stamp the active holiday overlay on top of the current frame. Called
 * from walk_pilot's frame loop so holidays persist across walk
 * transitions (the emblem belongs to the island, not to a specific
 * scene). Existing foregroundPilotPlay frame loop calls this too. */
void fgBackdropStampHolidayPublic(void);

#endif
