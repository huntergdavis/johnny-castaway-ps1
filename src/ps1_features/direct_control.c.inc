void ps1ToggleSceneOrder(void)
{
    ps1SceneOrderSequential = !ps1SceneOrderSequential;
}

/* --- Direct control --- */

void ps1DirectControlUpdate(uint16 buttons)
{
    if (!ps1DirectControlEnabled)
        return;

    if (buttons & PAD_UP)    grDy -= 2;
    if (buttons & PAD_DOWN)  grDy += 2;
    if (buttons & PAD_LEFT)  grDx -= 2;
    if (buttons & PAD_RIGHT) grDx += 2;
}

void ps1DirectControlReset(void)
{
    grDx = 0;
    grDy = 0;
}
