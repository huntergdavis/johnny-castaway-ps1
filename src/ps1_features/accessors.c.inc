
uint16 ps1GetCurrentAdsTag(void)
{
    return ps1AdsCurrentTag;
}

int ps1GetActiveThreadCount(void)
{
    return (int)ps1AdsDbgActiveThreads;
}

uint32 ps1GetMemoryUsed(void)
{
    return (uint32)getTotalMemoryUsed();
}

uint32 ps1GetMemoryBudget(void)
{
    return (uint32)getMemoryBudget();
}

int ps1GetCurrentFrame(void)
{
    return grGetCurrentFrame();
}

int ps1GetSoundMuted(void)
{
    return soundDisabled;
}

int ps1GetSceneOrderSequential(void)
{
    return ps1SceneOrderSequential;
}

int ps1GetDirectControlEnabled(void)
{
    return ps1DirectControlEnabled;
}

int ps1GetCaptionsEnabled(void)
{
    return ps1CaptionsEnabled;
}
