#ifdef PS1_BUILD
/* Persistent debug telemetry for overlay (kept globals). */
uint16 ps1AdsDbgActiveThreads = 0;
uint16 ps1AdsDbgMini = 0;
uint16 ps1AdsDbgRunningThreads = 0;
uint16 ps1AdsDbgTerminatedThreads = 0;
uint16 ps1AdsDbgSceneSlot = 0;
uint16 ps1AdsDbgSceneTag = 0;
uint16 ps1AdsDbgReplayCount = 0;
uint16 ps1AdsDbgReplayTryFrame = 0;
uint16 ps1AdsDbgReplayDrawFrame = 0;
uint16 ps1AdsDbgMergeCarryFrame = 0;
uint16 ps1AdsDbgNoDrawThreadsFrame = 0;
uint16 ps1AdsDbgPlayedThreadsFrame = 0;
uint16 ps1AdsDbgRecordedSpritesFrame = 0;
uint16 ps1AdsDbgLastStopThread = 0;
uint16 ps1AdsDbgLastStopSceneSig = 0;
uint16 ps1AdsDbgLastReapThread = 0;
uint16 ps1AdsDbgLastReapSceneSig = 0;
uint16 ps1AdsDbgLastAddThread = 0;
uint16 ps1AdsDbgLastAddSceneSig = 0;
static struct TDrawnSprite gPrevReplayScratch[MAX_DRAWN_SPRITES];
/* One-shot carry used to bridge scene/thread handoff gaps. */
static struct TDrawnSprite gHandoffReplay[MAX_DRAWN_SPRITES];
static uint8 gHandoffReplayCount = 0;
static uint8 gHandoffReplayValid = 0;

static inline void adsDbgAddU16(uint16 *acc, uint16 add)
{
    uint32 sum;
    if (!acc) return;
    sum = (uint32)(*acc) + (uint32)add;
    *acc = (sum > 0xFFFFU ? 0xFFFFU : (uint16)sum);
}

static int adsIsActorCandidate(const struct TDrawnSprite *ds)
{
    uint32 area;
    if (!ds) return 0;
    if (ds->width >= 8 && ds->height >= 16 &&
        ds->width <= 96 && ds->height <= 140) {
        area = (uint32)ds->width * (uint32)ds->height;
        if (area >= 180 && area <= 4500)
            return 1;
    }
    return 0;
}

static int adsActorNearMatch(const struct TDrawnSprite *a, const struct TDrawnSprite *b)
{
    if (!a || !b) return 0;
    if ((unsigned)((int)a->x - (int)b->x + 18) > 36u) return 0;
    if ((unsigned)((int)a->y - (int)b->y + 18) > 36u) return 0;
    if ((unsigned)((int)a->width - (int)b->width + 12) > 24u) return 0;
    if ((unsigned)((int)a->height - (int)b->height + 12) > 24u) return 0;
    return 1;
}

static uint8 adsCaptureReplayRecords(struct TTtmThread *thread)
{
    uint8 count;
    if (!adsUseReplayRecovery()) return 0;
    if (!thread) return 0;
    count = thread->numDrawnSprites;
    if (count > 0) {
        int bestArea = -1;
        int haveNear = 0;
        memcpy(gPrevReplayScratch, thread->drawnSprites,
               (size_t)count * sizeof(struct TDrawnSprite));
        for (uint8 i = 0; i < count; i++) {
            const struct TDrawnSprite *ds = &thread->drawnSprites[i];
            int area;
            if (!adsIsActorCandidate(ds)) continue;
            if (thread->lastActorReplayValid &&
                !adsActorNearMatch(ds, &thread->lastActorReplay)) {
                continue;
            }
            area = (int)ds->width * (int)ds->height;
            if (area > bestArea) {
                bestArea = area;
                thread->lastActorReplay = *ds;
                haveNear = 1;
            }
        }
        if (!haveNear && !thread->lastActorReplayValid) {
            /* First-time actor discovery only. When lastActorReplayValid is
             * already true, we must NOT re-acquire blindly or fire/prop
             * sprites can hijack actor tracking. */
            for (uint8 i = 0; i < count; i++) {
                const struct TDrawnSprite *ds = &thread->drawnSprites[i];
                int area;
                if (!adsIsActorCandidate(ds)) continue;
                area = (int)ds->width * (int)ds->height;
                if (area > bestArea) {
                    bestArea = area;
                    thread->lastActorReplay = *ds;
                    haveNear = 1;
                }
            }
        }
        if (haveNear) thread->lastActorReplayValid = 1;
    }
    return count;
}

/* Preserve prior records that did not receive a nearby replacement this tick.
 * Match by screen proximity so animation frame/image changes still replace
 * the same logical actor and do not trail old frames. */
static uint8 adsMergeReplayByProximity(struct TTtmThread *thread,
                                       const struct TDrawnSprite *prevRecords,
                                       uint8 prevCount)
{
    uint8 carried = 0;
    if (!adsUseReplayRecovery()) return 0;
    if (!thread || !prevRecords || prevCount == 0) return 0;

    for (uint8 p = 0; p < prevCount; p++) {
        const struct TDrawnSprite *prev = &prevRecords[p];
        int matched = 0;
        /* If this prev record IS the tracked main actor, require that any
         * "replacement" also looks actor-sized.  Scenes often pack Johnny
         * AND props (fire, tools) into the same BMP so they share imageNo.
         * Without this guard, a nearby fire sprite can falsely match
         * Johnny's prev record, causing him to be "replaced" not carried. */
        int prevIsActor = (thread->lastActorReplayValid &&
                           adsActorNearMatch(prev, &thread->lastActorReplay));
        for (uint8 n = 0; n < thread->numDrawnSprites; n++) {
            const struct TDrawnSprite *cur = &thread->drawnSprites[n];

            /* Treat as same logical actor when it's near the prior position and
             * uses the same image bank with close animation frame index. */
            if (cur->imageNo == prev->imageNo &&
                (unsigned)((int)cur->spriteNo - (int)prev->spriteNo + 8) <= 16u &&
                (unsigned)((int)cur->x - (int)prev->x + 24) <= 48u &&
                (unsigned)((int)cur->y - (int)prev->y + 24) <= 48u) {
                /* Don't let a small prop/fire sprite replace the main actor. */
                if (prevIsActor && !adsIsActorCandidate(cur))
                    continue;
                matched = 1;
                break;
            }

            /* Tight fallback for static/slow props with same dimensions. */
            if ((unsigned)((int)cur->width - (int)prev->width + 4) <= 8u &&
                (unsigned)((int)cur->height - (int)prev->height + 4) <= 8u &&
                (unsigned)((int)cur->x - (int)prev->x + 8) <= 16u &&
                (unsigned)((int)cur->y - (int)prev->y + 8) <= 16u) {
                if (prevIsActor && !adsIsActorCandidate(cur))
                    continue;
                matched = 1;
                break;
            }
        }

        if (!matched && thread->numDrawnSprites < MAX_DRAWN_SPRITES) {
            /* Prevent duplicate trails: don't carry if an equivalent record
             * is already present after this frame's draws. */
            int dup = 0;
            for (uint8 n = 0; n < thread->numDrawnSprites; n++) {
                const struct TDrawnSprite *cur = &thread->drawnSprites[n];
                if (cur->imageNo == prev->imageNo &&
                    (unsigned)((int)cur->x - (int)prev->x + 6) <= 12u &&
                    (unsigned)((int)cur->y - (int)prev->y + 6) <= 12u) {
                    dup = 1;
                    break;
                }
            }
            if (dup) continue;
            thread->drawnSprites[thread->numDrawnSprites++] = *prev;
            carried++;
        }
    }

    return carried;
}

static void adsCaptureHandoffReplay(struct TTtmThread *thread)
{
    int bestArea = -1;
    if (!adsUseReplayRecovery()) return;
    if (!thread || thread->numDrawnSprites == 0) return;

    if (adsUseRestorePilotReplayPolicy()) {
        gHandoffReplayValid = 0;
        gHandoffReplayCount = 0;
        return;
    }

    /* Transition carry is actor-only to avoid propagating unrelated props. */
    for (uint8 i = 0; i < thread->numDrawnSprites; i++) {
        struct TDrawnSprite *ds = &thread->drawnSprites[i];
        if (adsIsActorCandidate(ds)) {
            int area = (int)ds->width * (int)ds->height;
            if (area > bestArea) {
                bestArea = area;
                gHandoffReplay[0] = *ds;
            }
        }
    }

    if (bestArea < 0) {
        gHandoffReplayValid = 0;
        gHandoffReplayCount = 0;
        return;
    }

    gHandoffReplayCount = 1;
    gHandoffReplayValid = 1;
}

static void adsSeedFromHandoffReplay(struct TTtmThread *thread)
{
    if (!adsUseReplayRecovery()) return;
    if (!thread || !gHandoffReplayValid || gHandoffReplayCount == 0) return;
    if (adsUseRestorePilotReplayPolicy()) return;
    if (thread->numDrawnSprites != 0) return;

    memcpy(thread->drawnSprites, gHandoffReplay,
           (size_t)gHandoffReplayCount * sizeof(struct TDrawnSprite));
    thread->numDrawnSprites = gHandoffReplayCount;
    thread->lastActorReplay = gHandoffReplay[0];
    thread->lastActorReplayValid = 1;
    for (uint8 i = 0; i < thread->numDrawnSprites; i++)
        thread->drawnSprites[i].sceneEpoch = thread->sceneEpoch;

    /* One-shot seed; avoid stale carry leaking into unrelated scenes. */
    gHandoffReplayValid = 0;
    gHandoffReplayCount = 0;
}

/* Mid-scene fail-safe: if this frame lost the main actor entirely but previous
 * frame had one, inject one record for continuity. This is one-frame recovery,
 * not persistent carry, so it avoids trail accumulation. */
static void adsRecoverMissingActor(struct TTtmThread *thread,
                                   const struct TDrawnSprite *prevRecords,
                                   uint8 prevCount)
{
    struct TDrawnSprite candidate;
    int bestArea = -1;
    uint8 haveCandidate = 0;

    if (!adsUseReplayRecovery()) return;
    if (!thread) return;

    for (uint8 i = 0; i < thread->numDrawnSprites; i++) {
        const struct TDrawnSprite *cur = &thread->drawnSprites[i];
        if (thread->lastActorReplayValid) {
            if (adsActorNearMatch(cur, &thread->lastActorReplay)) return;
        } else if (adsIsActorCandidate(cur)) {
            return;
        }
    }

    for (uint8 i = 0; i < prevCount; i++) {
        const struct TDrawnSprite *prev = &prevRecords[i];
        if (thread->lastActorReplayValid &&
            !adsActorNearMatch(prev, &thread->lastActorReplay))
            continue;
        if (adsIsActorCandidate(prev)) {
            int area = (int)prev->width * (int)prev->height;
            if (area > bestArea) {
                bestArea = area;
                candidate = *prev;
                haveCandidate = 1;
            }
        }
    }
    if (!haveCandidate && thread->lastActorReplayValid) {
        candidate = thread->lastActorReplay;
        haveCandidate = 1;
    }
    if (!haveCandidate) return;
    if (thread->numDrawnSprites >= MAX_DRAWN_SPRITES) return;

    /* Do not duplicate if an equivalent actor is already present. */
    for (uint8 i = 0; i < thread->numDrawnSprites; i++) {
        if (adsActorNearMatch(&thread->drawnSprites[i], &candidate)) return;
    }

    candidate.sceneEpoch = thread->sceneEpoch;
    thread->drawnSprites[thread->numDrawnSprites++] = candidate;
    thread->lastActorReplay = candidate;
    thread->lastActorReplayValid = 1;
}
#endif
