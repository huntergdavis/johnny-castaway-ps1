/*
 *  This file is part of 'Johnny Reborn' — PS1 port (walk subsystem).
 *  Copyright (C) 2026 Hunter Davis. GPL-3.0.
 *
 *  walk_pilot — story-loop walk driver. Wraps walk.c + walk_data.h's
 *  pre-baked spot-graph state machine and feeds the walk_render kernel
 *  one frame at a time. Called by the screensaver-loop scene picker
 *  (jc_reborn.c fgLoopNextScene) BEFORE each scene plays, to walk
 *  Johnny from his current spot/heading to the next scene's start.
 *
 *  Freeplay direct-control walks do NOT go through this file —
 *  scene_freeplay.c calls walk_render directly with D-pad-driven
 *  position. This is the story-loop side only.
 */
#ifndef WALK_PILOT_H
#define WALK_PILOT_H

#include "mytypes.h"

struct TTtmSlot;

/*
 * Walk Johnny from (fromSpot, fromHdg) to (toSpot, toHdg) using
 * walk.c's pre-baked path data + the shared walk_render kernel.
 *
 * Returns 0 on normal completion, non-zero if the walk was aborted
 * (e.g. the caller signalled a capture/regtest abort during the loop).
 *
 * Pre-conditions:
 *   - islandState.xPos / yPos are set to the current island offset.
 *   - The island background (BACKGRND.PSB / clean-rect set) is loaded
 *     and grRestoreBgTiles() works in the current scene.
 *   - JOHNWALK.PSB is on disc (Phase 1 wired this).
 *
 * If fromSpot == toSpot AND fromHdg == toHdg, returns immediately
 * (no-op walk). If fromSpot is invalid (-1 sentinel from a
 * LEFT_ISLAND scene), returns immediately — the caller is expected
 * to pick a FIRST scene that resets position.
 */
int fgWalkRender(int fromSpot, int fromHdg, int toSpot, int toHdg);

/* Free the JOHNWALK.PSB slot. The story-loop walk driver releases it
 * at the end of each walk so the next FG2 scene can claim large setup
 * buffers; this remains exposed for shutdown and abort paths. */
void fgWalkRenderTeardown(void);

/* Walk-area pristine buffer.
 *
 * walk_pilot needs to clear bgTile back to a known-clean baseline each
 * walk frame so the previous Johnny pose's pixels don't bleed through.
 * Earlier approaches tried to (a) reuse the FG2 scene's per-scene clean
 * rect (heap-fragility — sometimes failed to allocate; b) re-grLoadScreen
 * at walk start (caused empty-water flash + ~600KB malloc churn that
 * starved the next scene's bg load).
 *
 * The current design: capture one tight walk-area buffer during scene
 * setup, keep that allocation resident, and refresh its pixels only when
 * the island state changes. This avoids repeated free/malloc churn, which
 * fragmented the heap and made later walks lose their erase baseline. */

/* Optional early allocation hook. Current story-loop playback can rely on
 * lazy allocation in walkPilotCaptureCleanWalkAreaIfStale; retaining this
 * API keeps boot/debug callers source-compatible. Returns 1 on success,
 * 0 if malloc failed. */
int walkPilotInit(void);

/* walkPilotCaptureCleanWalkAreaIfStale — call from the scene-setup path
 * AFTER fgBackdropEnableWaveBackdrop (when bgTile is freshly painted
 * with ocean+island sprites and no FG2 composite has run yet) and
 * BEFORE the FG2 pack's first frame composite. The function compares
 * the supplied state key to the last capture and re-snapshots only if
 * something changed; it's cheap to call every scene setup. */
void walkPilotCaptureCleanWalkAreaIfStale(int raft, int lowTide, int night,
                                          int holidayId, int xPos, int yPos);

/* Release the walk clean buffer. Intended for shutdown/debug paths; normal
 * story-loop playback keeps the buffer resident to preserve heap layout. */
void walkPilotReleaseCleanWalkArea(void);

/* Diagnostic accessors — used by the BSOD log snapshot to print
 * the walk subsystem's state at the moment of failure. */
int           walkPilotCleanBufferAllocated(void);  /* 0 / 1 */
unsigned long walkPilotCleanBufferBytes(void);      /* 0 if not alloc'd */
int           walkPilotJohnwalkSlotLoaded(void);    /* 0 / 1 */

/* Optional PS1 SPU cold-cache path for walk-adjacent assets. The foreground
 * scheduler calls the JOHNWALK tick only after current-scene payload reads
 * are done; boot priming also stages MRAFT.PSB and the walk-clean snapshot
 * uses SPU cold storage when capacity is available. */
void walkPilotSetSpuStage(int enabled);
/* CACHE pressure relief: free the persistent JOHNWALK PSB load slab
 * when no sprite slot points into it. Returns nonzero if freed; the
 * next walk re-allocates it. */
int  walkPilotReliefFreePsbSlab(void);

/* Bytes the relief tier above could free right now (idle PSB slab
 * size), or 0 — lets the relief hook skip the tier when its yield
 * cannot cover the failing request. */
unsigned long walkPilotPsbSlabIdleBytes(void);
void walkPilotReservePsbSlab(unsigned long bytes);
int  walkPilotStageJohnwalkSpuTick(void);
int  walkPilotPrimeSpuAssetsBlocking(void);
int  walkPilotLoadMraftFromSpu(struct TTtmSlot *slot, uint16 slotNo);

#endif /* WALK_PILOT_H */
