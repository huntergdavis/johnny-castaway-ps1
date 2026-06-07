/*
 * Shared PS1 pad helpers.
 *
 * PSn00bSDK exposes the analog axes in PADTYPE, but most of this port
 * consumes the active-high digital button mask. This helper folds the
 * left analog stick into the D-pad bits with a conservative dead zone.
 */

#ifndef PS1_PAD_INPUT_H
#define PS1_PAD_INPUT_H

#include <psxpad.h>

#include "mytypes.h"
#include "ps1_pad_script.h"

#define PS1_PAD_ANALOG_LOW_DEADZONE  96
#define PS1_PAD_ANALOG_HIGH_DEADZONE 160

static inline uint16 ps1PadButtonsWithAnalog(const PADTYPE *pad)
{
    uint16 buttons;

    if (pad == NULL)
        return 0;

    buttons = (uint16)(~pad->btn);

    if (pad->type == PAD_ID_ANALOG || pad->type == PAD_ID_ANALOG_STICK) {
        if (pad->ls_x < PS1_PAD_ANALOG_LOW_DEADZONE)
            buttons |= PAD_LEFT;
        else if (pad->ls_x > PS1_PAD_ANALOG_HIGH_DEADZONE)
            buttons |= PAD_RIGHT;

        if (pad->ls_y < PS1_PAD_ANALOG_LOW_DEADZONE)
            buttons |= PAD_UP;
        else if (pad->ls_y > PS1_PAD_ANALOG_HIGH_DEADZONE)
            buttons |= PAD_DOWN;
    }

    return ps1PadScriptMergeButtons(buttons);
}

#endif /* PS1_PAD_INPUT_H */
