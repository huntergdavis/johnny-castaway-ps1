/*
 * Scene picker policies — Random / Sequential / Original.
 *
 * The PS1 screensaver loop in jc_reborn.c calls pickerNextScene() once
 * per iteration; the picker decides which scene to play based on the
 * active Scene Set pool and the user-selected policy.
 *
 * The whole module is engineered for zero heap pressure: the
 * implementation file poison-includes malloc/free/realloc/calloc to
 * make accidental allocations a compile error, and all per-policy
 * state lives in module-level statics totalling ~30 bytes.
 *
 * Telemetry: emits a single JCPICK line per pick (policy, picked slug,
 * retries, etc.) so soak tests can verify both behaviour and zero
 * heap delta vs. baseline.
 */

#ifndef SCENE_PICKER_H
#define SCENE_PICKER_H

#include "mytypes.h"

/* Policy enum. Order is the cycle order in the pause menu (Left/Right). */
enum ScenePickerPolicy {
    SCENE_PICKER_RANDOM      = 0,
    SCENE_PICKER_SEQUENTIAL  = 1,
    SCENE_PICKER_ORIGINAL    = 2,   /* lands in PR 2; falls back to RANDOM today */
    SCENE_PICKER_COUNT
};

/* User-facing name for the policy. NULL slot for unknown values. */
const char *pickerPolicyName(int policy);

/* Pick the next scene slug given the current active Scene Set.
 *
 *   explicitScene     If non-NULL/non-empty, return this slug verbatim
 *                     (Scene Explorer pin / fgpilot CLI / fgLoopSequenceJustReset
 *                     paths set this; pinning trumps policy).
 *   sceneSetIdx       Index into gSceneSetPools[]. If the set's pool is
 *                     empty, the picker falls back to kAllScenes (every
 *                     scene that ships with an FG2 pack on disc).
 *
 * Returns a slug pointer that is stable across the next iteration of
 * the screensaver loop (it points into either gSceneSetPools[].scenes
 * or kAllScenes, both of which are static). Never NULL on success.
 */
const char *pickerNextScene(const char *explicitScene, int sceneSetIdx);

/* Reset per-policy state. Called when the active Scene Set changes
 * (so Sequential's cursor restarts at 0, repeat-prevention's
 * lastAdsName clears, etc.). */
void pickerOnSceneSetCycle(void);

/* Set the active picker policy. Resets per-policy state so the next
 * pick starts fresh. Persisted to the memcard separately. */
void pickerSetPolicy(int policy);

/* Get the active picker policy. */
int  pickerGetPolicy(void);

#endif /* SCENE_PICKER_H */
