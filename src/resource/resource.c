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
#else
/* PS1 freestanding: provide minimal FILE I/O declarations */
#include <stddef.h>
#ifndef _FILE_DEFINED
#define _FILE_DEFINED
typedef struct _FILE FILE;
#endif
extern FILE *fopen(const char *pathname, const char *mode);
extern size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
extern int fclose(FILE *stream);
extern int fseek(FILE *stream, long offset, int whence);
extern long ftell(FILE *stream);
extern int fflush(FILE *stream);
extern int printf(const char *format, ...);
extern void *malloc(size_t size);
extern void *calloc(size_t nmemb, size_t size);
extern void free(void *ptr);
/* stdio constants */
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#define stdout ((FILE*)1)
#endif
#include <string.h>

#include "mytypes.h"
#ifdef PS1_BUILD
#include "mem_region.h"
#endif
#include "utils.h"
#include "resource.h"
#include "uncompress.h"

#ifdef PS1_BUILD
#include "cdrom_ps1.h"
#include <psxgpu.h>
#include <psxgte.h>
#endif /* PS1_BUILD */

#define MAX_ADS_RESOURCES 100
#define MAX_BMP_RESOURCES 200
#define MAX_PAL_RESOURCES 1
#define MAX_SCR_RESOURCES 20
#define MAX_TTM_RESOURCES 100


struct TAdsResource *adsResources[MAX_ADS_RESOURCES];
struct TBmpResource *bmpResources[MAX_BMP_RESOURCES];
struct TPalResource *palResources[MAX_PAL_RESOURCES];
struct TScrResource *scrResources[MAX_SCR_RESOURCES];
struct TTtmResource *ttmResources[MAX_TTM_RESOURCES];
int numAdsResources = 0;
int numBmpResources = 0;
int numPalResources = 0;
int numScrResources = 0;
int numTtmResources = 0;

/* ============================================================================
 * Hash table for O(1) resource lookups by name
 * ============================================================================ */

#define RESOURCE_HASH_SIZE 256  /* power of 2, must be > max resource count (200) */

struct TResourceHashEntry {
    const char *name;
    uint16 index;  /* index into the resource array */
};

static struct TResourceHashEntry bmpHashTable[RESOURCE_HASH_SIZE];
static struct TResourceHashEntry ttmHashTable[RESOURCE_HASH_SIZE];
static struct TResourceHashEntry adsHashTable[RESOURCE_HASH_SIZE];
static struct TResourceHashEntry scrHashTable[RESOURCE_HASH_SIZE];

static uint32 hashString(const char *str) {
    uint32 hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return hash;
}

static void hashInsert(struct TResourceHashEntry *table, uint32 tableSize,
                       const char *name, uint16 index) {
    uint32 h = hashString(name) & (tableSize - 1);
    while (table[h].name != NULL) {
        h = (h + 1) & (tableSize - 1);
    }
    table[h].name = name;
    table[h].index = index;
}

static int hashLookup(const struct TResourceHashEntry *table, uint32 tableSize,
                      const char *name) {
    uint32 h = hashString(name) & (tableSize - 1);
    while (table[h].name != NULL) {
        if (strcmp(table[h].name, name) == 0)
            return table[h].index;
        h = (h + 1) & (tableSize - 1);
    }
    return -1;  /* not found */
}

static struct TMapFile mapFile;

/* LRU Cache globals */
static uint32 globalTick = 0;
static size_t totalMemoryUsed = 0;
#ifdef PS1_BUILD
/* Round 33-soak: shrink LRU budget from 600 KB to 320 KB.
 *
 * Post-R33 the CACHE region is 640 KB total. Other CACHE residents
 * (gFgFrameBuffer, gFgPrefetchFrameBuffer, gFgStreamScratch, occasional
 * stream window spill, metadata reads) consume up to ~250 KB. With LRU
 * at the prior 600 KB cap the CACHE peak hit 623 KB — within 17 KB of
 * the 640 KB budget — and free-list fragmentation prevented contiguous
 * allocations of 50–100 KB that the next scene's metadata read needed,
 * BSODing at 247s with `pack-start failed`.
 *
 * Capping LRU at 320 KB keeps CACHE peak under ~570 KB worst-case
 * (320 LRU + 250 other), leaving ~70 KB bump-tail headroom for the
 * scene-start metadata read + frame buffer alloc. Cost: more aggressive
 * eviction → more CD reloads of recently-touched BMP/TTM/SCR/ADS. The
 * `target_vb` perf gates measure scene-loop time including reloads;
 * any regression will surface in the matrix. */
static size_t memoryBudget = 600 * 1024;  /* PS1: original budget. R33-soak experiments showed reducing this to 320/200 KB had no measurable effect on CACHE peak — fgpilot scenes use FG2 packs that don't load through LRU (they alloc via foregroundPilotRuntimeStart's CACHE allocs directly). LRU residency stays <200 KB regardless of cap. The CACHE pressure is dominated by the 4 grow-and-release per-scene buffers + their size variation across scenes. */
#else
static size_t memoryBudget = 256 * 1024;  /* PC: Conservative for responsiveness */
#endif

#include "resource/host_parsers.c.inc"
#include "resource/catalog.c.inc"
#include "resource/lru.c.inc"
