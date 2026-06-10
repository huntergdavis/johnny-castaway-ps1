/*
 * Scene picker — Random / Sequential / Original (Original lands in PR 2).
 *
 * Designed for net-zero heap pressure: malloc/free/realloc/calloc are
 * poison-included below. All per-policy state lives in module-level
 * statics; total budget is documented at each declaration.
 *
 * Telemetry: emits one JCPICK line per pick.
 */

#include "scene_picker.h"
#include "mytypes.h"
#include <stdio.h>
#include <stdlib.h>      /* rand() — must come BEFORE the malloc poison */
#include <string.h>
#include "story.h"
/* island.h's prototypes take `struct TTtmThread *`, which is defined in
 * a header we don't include. A forward decl keeps the compiler happy
 * about the parameter type — we never dereference it from this TU. */
struct TTtmThread;
#include "island.h"      /* TIslandState — lowTide / xPos / yPos for filter */
/* story_data.h gives us `struct TStoryScene` so we can dereference the
 * pointer that fgLoopFindStorySceneBySlug returns. Unfortunately the
 * same header also defines `static struct TStoryScene storyScenes[]`,
 * which we don't touch here — silence the resulting unused-variable
 * warning instead of duplicating the struct definition or refactoring
 * the header. The linker's --gc-sections drops the unused copy. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#include "story_data.h"
#pragma GCC diagnostic pop

/* Catalog accessors live in jc_reborn.c — keeps the pool data private.
 * Pool count == 0 means "fall back to the All Scenes set" (all 63
 * scenes that ship with FG2 packs on disc). */
extern int   fgLoopGetPoolCount(int sceneSetIdx);
extern const char *fgLoopGetPoolSlug(int sceneSetIdx, int index);
extern int   fgLoopGetAllCount(void);
extern const char *fgLoopGetAllSlug(int index);

/* Slug → storyScenes[] entry lookup, owned by jc_reborn.c. We need the
 * full struct visibility from story_data.h above so we can dereference
 * the returned pointer for adsName/adsTagNo. Declared at file scope so
 * both helpers below see the same incomplete-vs-complete type. */
extern const struct TStoryScene *fgLoopFindStorySceneBySlug(const char *slug);

/* Walk-awareness predicates owned by jc_reborn.c — the Original mode
 * retry loop uses these to mirror Sierra's `storyHasValidStart/End`
 * filtering (skip scenes without spotStart/hdgStart when Johnny needs
 * a walk-in; skip without spotEnd/hdgEnd so the next iteration's
 * prevSpot stays meaningful). */
extern int fgLoopSceneHasValidStart(const struct TStoryScene *s);
extern int fgLoopSceneHasValidEnd(const struct TStoryScene *s);

/* islandState (tide + Johnny's xPos/yPos drift) is owned by src/island.c;
 * storyCurrentDay (1..11, advances when ps1Soft date rolls over) is
 * owned by src/jc_reborn.c. Original mode reads them through these
 * externs to mirror Sierra's filter rules — neither is written. */
extern struct TIslandState islandState;
extern int storyCurrentDay;

/* ---------------------------------------------------------------------------
 *  Malloc poison guard.
 *
 *  The picker is on a hot path that runs every screensaver iteration
 *  forever. The whole reason Original mode is feasible is the audit
 *  said "zero allocations." Make accidental future violations a
 *  compile error. If a contributor genuinely needs to allocate, they
 *  must do it outside this translation unit.
 *
 *  These #defines come AFTER the system headers so stdlib.h's
 *  prototypes are unaffected; the poison only applies to call sites
 *  inside this file.
 * ------------------------------------------------------------------------- */
#define malloc(...)  PICKER_NO_MALLOC_DO_NOT_ADD_HEAP_ALLOCATIONS_TO_THE_PICKER
#define calloc(...)  PICKER_NO_MALLOC_DO_NOT_ADD_HEAP_ALLOCATIONS_TO_THE_PICKER
#define realloc(...) PICKER_NO_MALLOC_DO_NOT_ADD_HEAP_ALLOCATIONS_TO_THE_PICKER
#define free(...)    PICKER_NO_FREE_NOTHING_TO_FREE_IN_THE_PICKER

/* ---------------------------------------------------------------------------
 *  Module-level state.
 * ------------------------------------------------------------------------- */

/* Active policy. Defaults to RANDOM (current behaviour) until memcard
 * loads or the pause menu sets it. */
static int gPickerPolicy = SCENE_PICKER_RANDOM;

/* Sequential cursor. 0..pool.count-1; resets to 0 on Scene Set cycle.
 * 16 bits is more than enough — the All Scenes pool tops out at 63
 * today and the per-family sets are smaller. */
static uint16 gSequentialCursor = 0;

/* Repeat-prevention. Used by Random and Original (Sequential cycles
 * deterministically and doesn't need it). Tracks the last scene's
 * .ADS family + tag so we don't pick the same scene twice in a row.
 * adsName is at most 12 chars (ISO 9660 limit) plus NUL. */
static char   gLastAdsName[16] = {0};
static uint16 gLastAdsTag       = 0xFFFF;   /* sentinel: "no last pick yet" */

/* Telemetry counters — surfaced via JCPICK so soak tests can sanity
 * check that retries stay bounded. */
static uint16 gJcpickFrame = 0;

/* ---------------------------------------------------------------------------
 *  Original-mode (Sierra) state machine.
 *
 *  Sierra's storyPlay() treats one screensaver run as a mini-story:
 *
 *     PICK_FINAL  → choose the closing scene up front (filter: FINAL,
 *                   minus FIRST after the very first sequence).
 *     PLAY_INTERM → 6..19 intermediate scenes that walk Johnny around
 *                   the island, filtered by tide / island position
 *                   flags, repeat-prevented against the last pick.
 *     PLAY_FINAL  → finally, play the stashed final scene; loop back
 *                   to PICK_FINAL for the next mini-story.
 *
 *  All state lives in the fixed-size statics below — same memory
 *  footprint as the Sequential cursor (the picker still needs zero
 *  heap, full-stop). The only dynamic-looking buffer is the candidate
 *  index list in pickOriginal(); that lives on the stack and is
 *  released on return.
 * ------------------------------------------------------------------------- */

enum {
    SEQ_PICK_FINAL        = 0,
    SEQ_PLAY_INTERMEDIATE = 1,
    SEQ_PLAY_FINAL        = 2,
};

/* Path categorisation for telemetry — pickOriginal sets *outPath so
 * the JCPICK line can label each return honestly:
 *   INTM      — picked an intermediate scene (state machine running)
 *   FINAL     — returned the stashed final scene (mini-story climax)
 *   DEGRADED  — pool had no FINAL-flagged scene; fell through to
 *               pickRandom. Differentiates "Sierra logic ran" from
 *               "we silently became Random because the active set
 *               doesn't have any FINAL-tagged scenes." */
enum {
    ORIG_PATH_INTM     = 0,
    ORIG_PATH_FINAL    = 1,
    ORIG_PATH_DEGRADED = 2,
};

static uint8  gSequenceState         = SEQ_PICK_FINAL;
static uint8  gIntermediatesRemaining = 0;
/* gIsFirstSequence: gates the FIRST flag exclusion for finals. Sierra's
 * `if (firstSequence) unwantedFlags |= FIRST` excludes FIRST-flagged
 * scenes from being picked as the final on the *very first* mini-story
 * — the user lands on a normal scene with intermediates leading up to
 * it instead of an intro-only scene that has no walk-in. After the
 * first mini-story completes, FIRST is allowed again. */
static uint8  gIsFirstSequence       = 1;
static char   gFinalSlug[16]         = {0};   /* stashed from PICK_FINAL */

/* Walk-aware retry state. Sierra tracks these as locals inside
 * storyPlay(); we keep them as module-level statics because the picker
 * is invoked once per scene-loop iteration, not once per mini-story.
 * gPrevSpot = -1 means "fresh sequence, no prior position to walk
 * from." Updated after each successfully picked intermediate. */
static sint8  gPrevSpot              = -1;
static sint8  gPrevHdg               = -1;

const char *pickerPolicyName(int policy)
{
    switch (policy) {
    case SCENE_PICKER_RANDOM:     return "Random";
    case SCENE_PICKER_SEQUENTIAL: return "Sequential";
    case SCENE_PICKER_ORIGINAL:   return "Original";
    default:                      return "?";
    }
}

void pickerOnSceneSetCycle(void)
{
    gSequentialCursor = 0;
    gLastAdsName[0]   = '\0';
    gLastAdsTag       = 0xFFFF;
    /* Original-mode sequence resets too — switching pools mid-mini-story
     * leaves you with a finalSlug that may not even exist in the new
     * pool, so start fresh. gIsFirstSequence stays as it was: the
     * "have we ever picked a final?" flag is per-session, not per-set. */
    gSequenceState         = SEQ_PICK_FINAL;
    gIntermediatesRemaining = 0;
    gFinalSlug[0]          = '\0';
    gPrevSpot              = -1;
    gPrevHdg               = -1;
}

void pickerSetPolicy(int policy)
{
    if (policy < 0 || policy >= SCENE_PICKER_COUNT)
        return;
    gPickerPolicy = policy;
    /* Don't reset cursor — switching policy mid-stream shouldn't snap
     * Sequential back to scene 0; user's mental model is "where we
     * were, but with a different draw rule." Repeat-prevention also
     * carries over for the same reason. */
}

int pickerGetPolicy(void)
{
    return gPickerPolicy;
}

/* ---------------------------------------------------------------------------
 *  Helper: how many scenes does the active set carry, and how do we
 *  read slug N out of it?  The set's pool is indexed by sceneSetIdx;
 *  if its count is 0 (empty / placeholder set, or the canonical
 *  "All Scenes" entry at index 0) we fall through to kAllScenes.
 *  The caller has already validated sceneSetIdx range.
 * ------------------------------------------------------------------------- */
static int activePoolCount(int sceneSetIdx)
{
    int n = fgLoopGetPoolCount(sceneSetIdx);
    return (n > 0) ? n : fgLoopGetAllCount();
}

static const char *activePoolSlug(int sceneSetIdx, int index)
{
    int n = fgLoopGetPoolCount(sceneSetIdx);
    if (n > 0)
        return fgLoopGetPoolSlug(sceneSetIdx, index);
    return fgLoopGetAllSlug(index);
}

/* ---------------------------------------------------------------------------
 *  Original-mode helpers: pool filtering by Sierra flag mask + dayNo.
 *
 *  islandState (lowTide / xPos / yPos) and storyCurrentDay are owned by
 *  the runtime; we read but never write. The stack scratch
 *  `outIndices[]` mirrors Sierra's `int scenes[NUM_SCENES]` pattern —
 *  one frame's worth, released on return.
 * ------------------------------------------------------------------------- */
static int filterPoolByFlags(int sceneSetIdx,
                             uint16 wantedFlags, uint16 unwantedFlags,
                             int *outIndices, int maxOut)
{
    int n = activePoolCount(sceneSetIdx);
    int count = 0;
    for (int i = 0; i < n && count < maxOut; i++) {
        const char *slug = activePoolSlug(sceneSetIdx, i);
        if (slug == NULL) continue;
        const struct TStoryScene *s = fgLoopFindStorySceneBySlug(slug);
        if (s == NULL) continue;
        if ((s->flags & wantedFlags) != wantedFlags) continue;
        if (s->flags & unwantedFlags) continue;
        if (s->dayNo != 0 && s->dayNo != storyCurrentDay) continue;
        outIndices[count++] = i;
    }
    return count;
}

/* ---------------------------------------------------------------------------
 *  Repeat-prevention: returns 1 if `slug` matches the last picked scene
 *  (same .ADS family AND same tag). The picker calls this and rejects
 *  matches via a small retry budget.
 * ------------------------------------------------------------------------- */
static int slugMatchesLastPick(const char *slug)
{
    if (gLastAdsName[0] == '\0' || gLastAdsTag == 0xFFFF)
        return 0;
    /* Walk storyScenes[] for this slug — story_data already maps slug
     * to (adsName, adsTagNo). 63-entry linear scan; ~1 microsecond. */
    const struct TStoryScene *s = fgLoopFindStorySceneBySlug(slug);
    if (s == NULL)
        return 0;
    if (strcmp(s->adsName, gLastAdsName) != 0)
        return 0;
    return s->adsTagNo == gLastAdsTag;
}

static void recordPick(const char *slug)
{
    const struct TStoryScene *s = fgLoopFindStorySceneBySlug(slug);
    if (s != NULL) {
        /* adsName is ≤12 chars per ISO 9660; truncate just in case. */
        size_t n = strlen(s->adsName);
        if (n >= sizeof(gLastAdsName)) n = sizeof(gLastAdsName) - 1;
        memcpy(gLastAdsName, s->adsName, n);
        gLastAdsName[n] = '\0';
        gLastAdsTag = (uint16)s->adsTagNo;
    }
}

/* ---------------------------------------------------------------------------
 *  Policy implementations.
 * ------------------------------------------------------------------------- */

/* Random: uniform draw with up to 4 retries to avoid same-scene-twice.
 * Budget caps at min(pool/2, 4) so a 2-element pool can still pick the
 * other element on retry; bigger pools cap at 4 to prevent worst-case
 * pathological pools that match the last pick on every entry from
 * spinning forever. */
static const char *pickRandom(int sceneSetIdx, uint8 *outRetries)
{
    int n = activePoolCount(sceneSetIdx);
    if (n <= 0) {
        *outRetries = 0;
        return NULL;
    }
    int budget = (n <= 8) ? (n - 1) : 4;
    if (budget < 0) budget = 0;
    const char *slug = NULL;
    int retries = 0;
    do {
        slug = activePoolSlug(sceneSetIdx, rand() % n);
        if (!slugMatchesLastPick(slug)) break;
        retries++;
    } while (retries <= budget);
    *outRetries = (uint8)retries;
    return slug;
}

/* Sequential: cycle through the pool in authored order. */
static const char *pickSequential(int sceneSetIdx, uint8 *outRetries)
{
    int n = activePoolCount(sceneSetIdx);
    if (n <= 0) {
        *outRetries = 0;
        return NULL;
    }
    if (gSequentialCursor >= n)
        gSequentialCursor = 0;
    const char *slug = activePoolSlug(sceneSetIdx, gSequentialCursor);
    gSequentialCursor = (uint16)((gSequentialCursor + 1) % n);
    *outRetries = 0;
    return slug;
}

/* ---------------------------------------------------------------------------
 *  Original: Sierra's storyPlay() distilled into one call per pick.
 *
 *  State machine (gSequenceState):
 *    PICK_FINAL:        Pool→filter(FINAL [, ¬FIRST]) → choose,
 *                       stash slug, draw intermediate budget 6..19,
 *                       transition to PLAY_INTERMEDIATE, fall through.
 *    PLAY_INTERMEDIATE: Pool→filter(¬FINAL ¬FIRST [+LOWTIDE_OK
 *                       +VARPOS_OK]) → walk-aware retry budget
 *                       min(8, count-1), repeat-prevent against
 *                       gLastAdsName/Tag. On budget exhaustion or
 *                       empty filter, transition to PLAY_FINAL.
 *    PLAY_FINAL:        Return the stashed slug verbatim, transition
 *                       back to PICK_FINAL for the next mini-story.
 *
 *  Empty-filter fallback: if the active pool has zero FINAL-flagged
 *  scenes (e.g. a future Scene Set authored without any), degrade
 *  gracefully to Random. The screensaver should never go dark.
 *
 *  Stack budget: one int[NUM_SCENES] candidate buffer (252 bytes), at
 *  most one frame deep. The SEQ_PICK_FINAL → SEQ_PLAY_INTERMEDIATE
 *  fall-through reuses the same frame's buffer via the for(;;) loop;
 *  no recursion.
 * ------------------------------------------------------------------------- */
static const char *pickOriginal(int sceneSetIdx,
                                uint8 *outRetries,
                                uint8 *outPath)
{
    *outRetries = 0;
    *outPath    = ORIG_PATH_INTM;     /* assume happy path; override on degrade/final */
    int n = activePoolCount(sceneSetIdx);
    if (n <= 0)
        return NULL;

    int indices[NUM_SCENES];   /* 252-byte stack scratch, like Sierra */

    /* Bounded transition loop: PICK_FINAL → PLAY_INTERMEDIATE returns
     * an intermediate (one transition); PLAY_FINAL on empty pool can
     * fall through to PICK_FINAL once. 4 iterations is a safety cap;
     * normal flow returns in ≤2. */
    for (int safety = 0; safety < 4; safety++) {

        if (gSequenceState == SEQ_PICK_FINAL) {
            /* Sierra resets per-mini-story locals at the top of each
             * storyPlay() outer iteration: prevSpot=-1, lastAdsName="".
             * Mirror that here so the new sequence's first intermediate
             * isn't blocked by repeat-prevention against the previous
             * sequence's last scene, and isn't walk-aware-skipped on
             * the basis of a stale prevSpot. */
            gLastAdsName[0] = '\0';
            gLastAdsTag     = 0xFFFF;
            gPrevSpot       = -1;
            gPrevHdg        = -1;

            uint16 wanted   = FINAL;
            /* Sierra: `if (firstSequence) unwantedFlags |= FIRST` —
             * exclude FIRST-flagged scenes from finals on the very
             * first mini-story (so the screensaver opens with a
             * walk-in scene, not an intro that starts at SPOT 0). */
            uint16 unwanted = gIsFirstSequence ? FIRST : 0;
            int count = filterPoolByFlags(sceneSetIdx, wanted, unwanted,
                                          indices, NUM_SCENES);
            if (count == 0) {
                /* No FINAL-flagged scene in this pool — degrade to
                 * Random and tell the caller so the JCPICK line can
                 * say origpath=degraded. With kAllScenes (the
                 * default) this never fires — 14 of the 63 scenes
                 * are FINAL-tagged. Only happens for tiny
                 * family-curated sets that omit finals. */
                *outPath = ORIG_PATH_DEGRADED;
                return pickRandom(sceneSetIdx, outRetries);
            }
            const char *slug = activePoolSlug(sceneSetIdx,
                                              indices[rand() % count]);
            if (slug == NULL) {
                *outPath = ORIG_PATH_DEGRADED;
                return pickRandom(sceneSetIdx, outRetries);
            }
            size_t len = strlen(slug);
            if (len >= sizeof(gFinalSlug)) len = sizeof(gFinalSlug) - 1;
            memcpy(gFinalSlug, slug, len);
            gFinalSlug[len] = '\0';
            /* 6..19 intermediates, matching Sierra's `6 + rand() % 14`. */
            gIntermediatesRemaining = (uint8)(6 + (rand() % 14));
            gIsFirstSequence = 0;
            gSequenceState   = SEQ_PLAY_INTERMEDIATE;
            /* Fall through (next iteration of the safety loop) so we
             * actually pick + return an intermediate this call. */
            continue;
        }

        if (gSequenceState == SEQ_PLAY_INTERMEDIATE) {
            uint16 wanted   = 0;
            uint16 unwanted = FINAL | FIRST;
            if (islandState.lowTide)
                wanted |= LOWTIDE_OK;
            if (islandState.xPos || islandState.yPos)
                wanted |= VARPOS_OK;
            int count = filterPoolByFlags(sceneSetIdx, wanted, unwanted,
                                          indices, NUM_SCENES);
            if (count == 0) {
                /* No intermediates available with current filters —
                 * skip ahead to the final scene (matches Sierra
                 * behaviour when the inner pickTry loop runs dry). */
                gSequenceState = SEQ_PLAY_FINAL;
                continue;
            }
            int budget = (count <= 8) ? (count - 1) : 8;
            if (budget < 0) budget = 0;
            const char *slug = NULL;
            const struct TStoryScene *scene = NULL;
            int retries = 0;
            do {
                slug  = activePoolSlug(sceneSetIdx,
                                       indices[rand() % count]);
                if (slug == NULL) break;
                scene = fgLoopFindStorySceneBySlug(slug);
                if (scene == NULL) {
                    /* Pool entry doesn't map to storyScenes — skip
                     * (couldn't apply walk-aware/repeat checks). */
                    retries++;
                    continue;
                }
                /* Sierra's walk-aware skip: if Johnny needs a walk-in
                 * (prevSpot != -1) and this scene has no valid start
                 * coords, skip and retry. */
                if (gPrevSpot != -1
                    && !fgLoopSceneHasValidStart(scene)) {
                    retries++;
                    continue;
                }
                /* Sierra requires a valid end too so the next pick's
                 * prevSpot is meaningful. Skip dead-end scenes. */
                if (!fgLoopSceneHasValidEnd(scene)) {
                    retries++;
                    continue;
                }
                /* Repeat-prevention: same family + tag as last pick? */
                if (!slugMatchesLastPick(slug))
                    break;
                retries++;
            } while (retries <= budget);
            *outRetries = (uint8)retries;
            /* Update walk-aware state from whichever scene we landed
             * on (even if the budget was exhausted — Sierra accepts
             * the pickTry-8 result either way). */
            if (scene != NULL && fgLoopSceneHasValidEnd(scene)) {
                gPrevSpot = (sint8)scene->spotEnd;
                gPrevHdg  = (sint8)scene->hdgEnd;
            } else {
                gPrevSpot = -1;
                gPrevHdg  = -1;
            }
            if (gIntermediatesRemaining > 0)
                gIntermediatesRemaining--;
            if (gIntermediatesRemaining == 0)
                gSequenceState = SEQ_PLAY_FINAL;
            *outPath = ORIG_PATH_INTM;
            return slug;
        }

        if (gSequenceState == SEQ_PLAY_FINAL) {
            const char *slug = (gFinalSlug[0] != '\0') ? gFinalSlug : NULL;
            gSequenceState = SEQ_PICK_FINAL;
            if (slug == NULL) {
                /* Lost the stash somehow (e.g. scene-set cycle in
                 * mid-sequence). Fall through to PICK_FINAL on the
                 * next loop iteration to start a fresh mini-story. */
                continue;
            }
            *outPath = ORIG_PATH_FINAL;
            return slug;
        }

        /* Defensive: unknown state → snap back to a fresh sequence. */
        gSequenceState = SEQ_PICK_FINAL;
    }

    /* Shouldn't reach here — Random is the safety net. */
    *outPath = ORIG_PATH_DEGRADED;
    return pickRandom(sceneSetIdx, outRetries);
}

/* ---------------------------------------------------------------------------
 *  Public dispatcher.
 * ------------------------------------------------------------------------- */
const char *pickerNextScene(const char *explicitScene, int sceneSetIdx)
{
    /* Pinning short-circuits all policy work. Mirrors the legacy
     * fgLoopNextScene's first line — Scene Explorer Cross/Triangle
     * and the CLI fgpilot path both rely on this. */
    if (explicitScene != NULL && explicitScene[0] != '\0')
        return explicitScene;

    uint8 retries = 0;
    const char *slug = NULL;
    int policy = gPickerPolicy;

    uint8 originalPath = ORIG_PATH_INTM;

    switch (policy) {
    case SCENE_PICKER_SEQUENTIAL:
        slug = pickSequential(sceneSetIdx, &retries);
        break;
    case SCENE_PICKER_ORIGINAL:
        slug = pickOriginal(sceneSetIdx, &retries, &originalPath);
        break;
    case SCENE_PICKER_RANDOM:
    default:
        slug = pickRandom(sceneSetIdx, &retries);
        policy = SCENE_PICKER_RANDOM;
        break;
    }

    if (slug == NULL)
        return NULL;

    recordPick(slug);

    /* JCPICK telemetry. Original mode reports `origpath=` (intm /
     * final / degraded) so soak tests can verify Sierra logic
     * actually ran instead of silently falling through to Random;
     * `left=` is the intermediate counter AFTER this pick (so an
     * intm row decrementing toward 0 marks the path to PLAY_FINAL).
     * Other policies stick to the original short format for cheap
     * grep-ability. */
    if (policy == SCENE_PICKER_ORIGINAL) {
        const char *pathLabel =
            (originalPath == ORIG_PATH_INTM)     ? "intm"     :
            (originalPath == ORIG_PATH_FINAL)    ? "final"    :
            (originalPath == ORIG_PATH_DEGRADED) ? "degraded" : "?";
#if PS1_VERBOSE_DIAGNOSTICS
        printf("JCPICK frame=%u policy=%d set=%d picked=%s retries=%u origpath=%s left=%d\n",
               (unsigned)gJcpickFrame++, policy, sceneSetIdx,
               slug, (unsigned)retries, pathLabel,
               (int)gIntermediatesRemaining);
#endif
    } else {
#if PS1_VERBOSE_DIAGNOSTICS
        printf("JCPICK frame=%u policy=%d set=%d picked=%s retries=%u\n",
               (unsigned)gJcpickFrame++, policy, sceneSetIdx,
               slug, (unsigned)retries);
#endif
    }

    return slug;
}
