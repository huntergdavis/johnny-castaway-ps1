/*
 *  This file is part of 'Johnny Reborn'
 *
 *  An open-source engine for the classic
 *  'Johnny Castaway' screensaver by Sierra.
 *
 *  Copyright (C) 2019 Jeremie GUILLAUME
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

/* Conditional includes for PS1 freestanding build */
#ifndef PS1_BUILD
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#else
#include <stddef.h>
#include <string.h>
#include <psxgpu.h>  /* For RECT, LoadImage, setRECT */
#ifndef _FILE_DEFINED
#define _FILE_DEFINED
typedef struct _FILE FILE;
#endif
extern int rand(void);
extern void *malloc(size_t size);
extern void free(void *ptr);
extern int fprintf(FILE *stream, const char *format, ...);
extern int printf(const char *format, ...);
extern void *memcpy(void *dest, const void *src, size_t n);
extern int strcmp(const char *s1, const char *s2);
#define stderr ((FILE*)2)
#endif

#include "mytypes.h"
#include "utils.h"
#include "events.h"
#include "resource.h"
/* Platform-specific graphics headers */
#ifdef PS1_BUILD
#include "graphics_ps1.h"
#include "cdrom_ps1.h"
#else
#include "graphics.h"
#endif
#include "ttm.h"
#include "island.h"
#include "holidays.h"
#include "walk.h"
#include "bench.h"
#include "ads.h"
#include "foreground_pilot.h"
#ifdef PS1_BUILD
#include "ps1_debug.h"
#include "ps1_restore_pilots.h"
#endif

#ifndef PS1_BUILD
#define ADS_HOST_UNUSED __attribute__((unused))
#else
#define ADS_HOST_UNUSED
#endif

extern int grCaptureSequenceComplete(void);


#define MAX_RANDOM_OPS        10
#define MAX_ADS_CHUNKS        100
#define MAX_ADS_CHUNKS_LOCAL  1

#define OP_ADD_SCENE   0
#define OP_STOP_SCENE  1
#define OP_NOP         2
#define MAX_ADS_PLAY_COUNTS 64
#define MAX_ADS_PENDING_STOPS 16


/* ADS script catalog entry: owns the parsed scene header plus the byte offset
 * used by the interpreter to seek back into the ADS stream. */
struct TAdsChunk {
    struct TAdsScene scene;
    uint32 offset;
};

/* Deferred random-block operation. The interpreter records these while walking
 * RAND/OR blocks, then the scene scheduler resolves them once block weights
 * and play counts are known. */
struct TAdsRandOp {
    int    type;
    uint16 slot;
    uint16 tag;
    uint16 numPlays;
    uint16 weight;
};

struct TAdsPlayCount {
    uint16 slot;
    uint16 tag;
    uint16 count;
};

struct TAdsPendingStop {
    uint16 slot;
    uint16 tag;
};


static struct TAdsChunk adsChunks[MAX_ADS_CHUNKS];
static int    numAdsChunks;

static struct TAdsChunk adsChunksLocal[MAX_ADS_CHUNKS_LOCAL];
static int    numAdsChunksLocal;

static struct TTtmSlot ttmBackgroundSlot;
static struct TTtmSlot ttmHolidaySlot;
static struct TTtmSlot *ttmSlots = NULL;  /* Malloc'd, not static array! */

static struct TTtmThread ttmBackgroundThread;
static struct TTtmThread ttmHolidayThread;
static struct TTtmThread *ttmThreads = NULL;  /* Malloc'd, not static array! */

static struct TAdsChunk adsChunks[MAX_ADS_CHUNKS];

static struct TTtmTag *adsTags;
static int    adsNumTags = 0;
static int    adsTagCapacity = 0;

static struct TAdsRandOp adsRandOps[MAX_RANDOM_OPS];
static int    adsNumRandOps    = 0;
static struct TAdsPlayCount adsPlayCounts[MAX_ADS_PLAY_COUNTS];
static int    numAdsPlayCounts = 0;
static struct TAdsPendingStop adsPendingStops[MAX_ADS_PENDING_STOPS];
static int    numAdsPendingStops = 0;

static int    numThreads       = 0;
static int    adsStopRequested = 0;
int ps1AdsLastPlayLaunched = 0;
char ps1AdsCurrentName[16] = "";
uint16 ps1AdsCurrentTag = 0;

#include "ads/restore_pilot.c.inc"
#include "ads/replay_telemetry.c.inc"
#include "ads/script_index.c.inc"
#include "ads/scene_threads.c.inc"
#include "ads/interpreter.c.inc"
#include "ads/playback_loop.c.inc"
#include "ads/island_walk.c.inc"
#include "ads/foreground_bridge.c.inc"
