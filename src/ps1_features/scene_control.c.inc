void ps1SkipToNextScene(void)
{
    adsRequestStop();
}

void ps1RequestScene(int sceneIndex)
{
    ps1RequestedSceneIndex = sceneIndex;
}

int ps1GetRequestedScene(void)
{
    return ps1RequestedSceneIndex;
}

/* --- Scene ordering --- */
