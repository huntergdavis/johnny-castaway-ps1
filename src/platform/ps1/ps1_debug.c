/*
 *  This file is part of 'Johnny Reborn' - PS1 Port
 *
 *  Visual debugging system for PS1
 *  Displays text on-screen since printf() doesn't work in DuckStation
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 */

#include <sys/types.h>
#include <stdarg.h>
#include <stdio.h>
#include <psxgpu.h>
#include <psxgte.h>
#include <psxapi.h>
#include <psxpad.h>

#include "ps1_debug.h"

/* Font stream ID - not static so jc_reborn.c can access it */
int fontID = -1;

/* Screen dimensions - 640x480 interlaced high res mode */
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

/* Text buffer for accumulating debug messages */
#define MAX_DEBUG_LINES 20
#define MAX_LINE_LENGTH 80
static char debugLines[MAX_DEBUG_LINES][MAX_LINE_LENGTH];
static int numDebugLines = 0;

/* Background color for debug screen - CHANGE THIS EACH BUILD! */
/* Build 24: Light green */
static int bgR = 220, bgG = 255, bgB = 220;  /* Light green */

#include "ps1_debug/text.c.inc"
#include "ps1_debug/bsod.c.inc"
