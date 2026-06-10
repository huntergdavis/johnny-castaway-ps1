#ifndef FOREGROUND_PILOT_H
#define FOREGROUND_PILOT_H

void foregroundPilotSetScene(const char *sceneName);
void foregroundPilotSetSceneDrawOffset(int x, int y);
void foregroundPilotSetHeapProbe(int enabled);
void foregroundPilotSetLoadingWaveProof(int enabled);
void foregroundPilotSetSpuStage(int enabled);
void foregroundPilotSetStageScene(const char *sceneName);
/* One async-chunk tick of the next-scene stage payload read, called
 * per frame from the inter-scene walk loops where the CD is idle.
 * No-op outside the loading-waves proof. Returns nonzero while work
 * is pending. */
int foregroundPilotStageWalkTick(void);
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

/* Per-scene streaming-buffer sizes for the BSOD snapshot. 0 = not
 * currently allocated; otherwise the byte size of the buffer last
 * sized (current pack's maxDataSize). */
unsigned long fgGetFrameBufferBytes(void);
unsigned long fgGetPrefetchFrameBufferBytes(void);

/* fgPrePrimeStreamBuffers deleted (Phase 2 manifest item #9).
 * Was orphaned (no callers) after the v0.8.11 rollback; new
 * memory-region allocator replaces the pre-prime pattern. */

void foregroundPilotRuntimeEnd(void);
void foregroundPilotTeardownForFreeplay(void);

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

/* Tick the wave animation by one frame. Used by walk_pilot's frame
 * loop so the ocean keeps moving while Johnny walks between scenes
 * — without this, the surf freezes during transitions and the loop
 * looks seamed. Wraps islandAnimate(&gFgBackdropThread). */
void fgBackdropTickWavesPublic(void);

/* Runtime-scene backdrop setup for PS1-only modes that do not stream an
 * FG2 pack, such as freeplay. This prepares OCEAN00/NIGHT + island
 * sprites + wave thread using the current islandState, but deliberately
 * does not save clean rects. The caller stamps its own persistent state
 * first, then calls grSaveCleanBgRects with its chosen rect set. */
void fgBackdropPrepareIslandRuntimePublic(void);

/* Release non-sticky island backdrop slots from runtime-scene code.
 * keepBackgrnd follows the internal fgBackdropRelease convention:
 * non-zero keeps BACKGRND.BMP slot 0 resident for the next scene. */
void fgBackdropReleasePublic(int keepBackgrnd);

/* Re-stamp the island bg sprites (raft, palm, beach decor) from
 * BACKGRND.PSB into the bg mirror. Wipes any leftover Johnny pixels
 * the previous FG2 scene's last frame baked in. Idempotent — safe
 * to call between scenes. Without this, the walk-time clean snapshot
 * captures the scene's last-frame Johnny as part of "clean" and
 * every walk frame pastes him back. */
void fgBackdropRebuildIslandBg(void);

/* Snapshot a generous walk-area bounding rect (plus the wave region)
 * as clean. walk_pilot calls grRestoreBgFromRects() each frame to
 * erase the previous walk-sprite stamp before the next pose. Mirrors
 * the rect-based pattern fgBackdropSaveCleanBgRectsForPack uses for
 * FG2 scene playback — same mechanism, walk-sized bounding rect.
 * Call AFTER fgBackdropRebuildIslandBg so the snapshot is pure
 * island, not island+Johnny. Returns 1 on success, 0 on failure. */
int fgBackdropSaveCleanBgRectsForWalk(void);

/* Deactivate the rect snapshots taken by fgBackdropSaveCleanBgRectsForWalk.
 * Call when the walk completes so the next FG2 scene's setup can claim
 * the clean-rect slots while retaining their grown buffers. */
void fgBackdropEndWalk(void);

/* Suppress foregroundPilotRuntimeCompose for the duration of a walk. The
 * runtime compose path stamps the prior FG2 scene frame (with Johnny baked
 * in) onto bg every frame; without suppression, walk_pilot's own composite
 * lands UNDER that stamp and gets overwritten — yielding the previous
 * scene's Johnny on top of the walking sprite. Pair every (1) call with
 * a (0) call before exit. */
void foregroundPilotSuppressCompose(int suppressed);

#endif
