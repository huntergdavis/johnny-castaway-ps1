/*
 *  This file is part of 'Johnny Reborn' - PS1 Port
 *
 *  Feature toggles and scene control for the PS1 pause menu.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include <psxpad.h>

#include "mytypes.h"
#include "ps1_features.h"
#include "graphics_ps1.h"
#include "resource.h"

/* These are defined in ads.c / ads.h */
extern char ps1AdsCurrentName[16];
extern uint16 ps1AdsCurrentTag;
extern uint16 ps1AdsDbgActiveThreads;
void adsRequestStop(void);

/* Defined in graphics_ps1.c */
extern int grDx;
extern int grDy;
int grGetCurrentFrame(void);

/* Defined in sound_ps1.c */
extern int soundDisabled;

/* Defined in utils.c (PS1 software clock) */
extern int ps1SoftMonth;
extern int ps1SoftDay;
extern int ps1SoftYear;
extern int ps1SoftHour;
extern int ps1SoftMinute;

/* --- Feature toggle state --- */
int ps1SceneOrderSequential = 0;
int ps1DirectControlEnabled = 0;
int ps1CaptionsEnabled      = 0;
int ps1AudioDescEnabled     = 0;

/* --- Scene request --- */
static int ps1RequestedSceneIndex = -1;

/* --- Scene control --- */

#include "ps1_features/scene_control.c.inc"
#include "ps1_features/direct_control.c.inc"
#include "ps1_features/clock.c.inc"
#include "ps1_features/accessors.c.inc"
