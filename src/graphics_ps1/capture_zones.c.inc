void grCopyZoneToBg(PS1Surface *sfc, uint16 x, uint16 y, uint16 width, uint16 height)
{
    int screenX;
    int screenY;

    (void)sfc;

    screenX = (int)x + grDx;
    screenY = (int)y + grDy;

    /* PS1 draws directly into the composited background tiles instead of
     * keeping a separate saved-zones overlay layer. Treat COPY_ZONE_TO_BG as
     * committing the current rectangle into the clean restore baseline. */
    grCommitRectToCleanBg(screenX, screenY, (int)width + 2, height);
}
void grSaveImage1(PS1Surface *sfc, uint16 x, uint16 y, uint16 width, uint16 height)
{
    /* Johnny's current use of SAVE_IMAGE1 is the same class of operation as
     * COPY_ZONE_TO_BG: define a bounded region that should survive subsequent
     * frame restores. Keep the PS1 behavior explicit and deterministic by
     * committing the current rectangle into the clean background baseline. */
    grCopyZoneToBg(sfc, x, y, width, height);
}
void grSaveZone(PS1Surface *sfc, uint16 x, uint16 y, uint16 width, uint16 height)
{
    (void)sfc;
    (void)x;
    (void)y;
    (void)width;
    (void)height;
    grPs1SavedZone.valid = 1;
}

void grRestoreZone(PS1Surface *sfc, uint16 x, uint16 y, uint16 width, uint16 height)
{
    int screenX;
    int screenY;

    (void)sfc;

    if (grPs1SavedZone.valid) {
        grRestoreRectFromCleanBg(grPs1SavedZone.x,
                                 grPs1SavedZone.y,
                                 grPs1SavedZone.width,
                                 grPs1SavedZone.height);
        grPs1SavedZone.valid = 0;
        return;
    }

    if (width == 0 || height == 0)
        return;

    screenX = (int)x + grDx;
    screenY = (int)y + grDy;
    grRestoreRectFromCleanBg(screenX, screenY, width, height);
}

/*
 * Frame capture (not implemented on PS1)
 */
int grCaptureFrame(const char *filename)
{
    /* Frame capture not supported on PS1 hardware */
    return -1;
}

int grCaptureSequenceComplete(void)
{
    return 0;
}

void grCaptureSetSceneLabel(const char *sceneLabel)
{
    (void)sceneLabel;
}

void grCaptureSoundEvent(int sampleNo)
{
    (void)sampleNo;
}
