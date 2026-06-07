static const char *fpModeName(enum TFpMode mode)
{
    switch (mode) {
    case FP_MODE_IDLE: return "idle";
    case FP_MODE_WALK: return "walk";
    case FP_MODE_FISH: return "fish";
    case FP_MODE_ONESHOT: return "oneshot";
    case FP_MODE_STRUT: return "strut";
    case FP_MODE_RUNAWAY: return "runaway";
    default: return "?";
    }
}

static const struct TFpMenuMeta *fpMetaAt(const struct TFpMenuMeta *meta,
                                          int count, int index)
{
    if (meta == NULL || count <= 0)
        return NULL;
    if (index < 0)
        index = 0;
    if (index >= count)
        index = count - 1;
    return &meta[index];
}

int freeplayGagCount(void)
{
    return FP_GAG_COUNT;
}

const char *freeplayGagTitle(int index)
{
    const struct TFpMenuMeta *meta = fpMetaAt(kFpGagMeta, FP_GAG_COUNT, index);
    return meta ? meta->title : "";
}

const char *freeplayGagDescription(int index)
{
    const struct TFpMenuMeta *meta = fpMetaAt(kFpGagMeta, FP_GAG_COUNT, index);
    return meta ? meta->description : "";
}

const char *freeplayGagBmp(int index)
{
    const struct TFpMenuMeta *meta = fpMetaAt(kFpGagMeta, FP_GAG_COUNT, index);
    return meta ? meta->bmp : "";
}

int freeplayGagFrames(int index)
{
    const struct TFpMenuMeta *meta = fpMetaAt(kFpGagMeta, FP_GAG_COUNT, index);
    return meta ? meta->frames : 0;
}

int freeplayGagMemoryKB(int index)
{
    const struct TFpMenuMeta *meta = fpMetaAt(kFpGagMeta, FP_GAG_COUNT, index);
    return meta ? meta->memoryKB : 0;
}

int freeplayVisitorCount(void)
{
    return FP_SUMMON_COUNT;
}

const char *freeplayVisitorTitle(int index)
{
    const struct TFpMenuMeta *meta = fpMetaAt(kFpSummonMeta, FP_SUMMON_COUNT, index);
    return meta ? meta->title : "";
}

const char *freeplayVisitorDescription(int index)
{
    const struct TFpMenuMeta *meta = fpMetaAt(kFpSummonMeta, FP_SUMMON_COUNT, index);
    return meta ? meta->description : "";
}

const char *freeplayVisitorBmp(int index)
{
    const struct TFpMenuMeta *meta = fpMetaAt(kFpSummonMeta, FP_SUMMON_COUNT, index);
    return meta ? meta->bmp : "";
}

int freeplayVisitorFrames(int index)
{
    const struct TFpMenuMeta *meta = fpMetaAt(kFpSummonMeta, FP_SUMMON_COUNT, index);
    return meta ? meta->frames : 0;
}

int freeplayVisitorMemoryKB(int index)
{
    const struct TFpMenuMeta *meta = fpMetaAt(kFpSummonMeta, FP_SUMMON_COUNT, index);
    return meta ? meta->memoryKB : 0;
}
