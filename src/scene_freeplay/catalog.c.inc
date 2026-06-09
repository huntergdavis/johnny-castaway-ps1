#if FREEPLAY_DIAG_LOGS || PS1_VERBOSE_DIAGNOSTICS
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
#endif

static int fpClampIndex(int count, int index)
{
    if (count <= 0)
        return 0;
    if (index < 0)
        index = 0;
    if (index >= count)
        index = count - 1;
    return index;
}

int freeplayGagCount(void)
{
    return FP_GAG_COUNT;
}

int freeplayGagFrames(int index)
{
    index = fpClampIndex(FP_GAG_COUNT, index);
    return kFpGagFrameCounts[index];
}

int freeplayGagMemoryKB(int index)
{
    index = fpClampIndex(FP_GAG_COUNT, index);
    return kFpGagMemoryKB[index];
}

int freeplayVisitorCount(void)
{
    return FP_SUMMON_COUNT;
}

int freeplayVisitorFrames(int index)
{
    index = fpClampIndex(FP_SUMMON_COUNT, index);
    return kFpSummonFrameCounts[index];
}

int freeplayVisitorMemoryKB(int index)
{
    index = fpClampIndex(FP_SUMMON_COUNT, index);
    return kFpSummonMemoryKB[index];
}
