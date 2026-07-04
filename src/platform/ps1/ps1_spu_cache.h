#ifndef PS1_SPU_CACHE_H
#define PS1_SPU_CACHE_H

#include "mytypes.h"

#define PS1_SPU_CACHE_ALIGN 64u
#define PS1_SPU_CACHE_FG_METADATA_OFFSET 0u
#define PS1_SPU_CACHE_FG_METADATA_BYTES (8u * 1024u)
#define PS1_SPU_CACHE_FG_PAYLOAD_OFFSET \
    (PS1_SPU_CACHE_FG_METADATA_OFFSET + PS1_SPU_CACHE_FG_METADATA_BYTES)
#define PS1_SPU_CACHE_FG_PAYLOAD_BYTES (64u * 1024u)
#define PS1_SPU_CACHE_WALK_PSB_OFFSET \
    (PS1_SPU_CACHE_FG_PAYLOAD_OFFSET + PS1_SPU_CACHE_FG_PAYLOAD_BYTES)
#define PS1_SPU_CACHE_WALK_PSB_BYTES (48u * 1024u)
#define PS1_SPU_CACHE_WALK_CLEAN_OFFSET \
    (PS1_SPU_CACHE_WALK_PSB_OFFSET + PS1_SPU_CACHE_WALK_PSB_BYTES)
#define PS1_SPU_CACHE_WALK_CLEAN_BYTES (144u * 1024u)
#define PS1_SPU_CACHE_MRAFT_PSB_OFFSET \
    (PS1_SPU_CACHE_WALK_CLEAN_OFFSET + PS1_SPU_CACHE_WALK_CLEAN_BYTES)
#define PS1_SPU_CACHE_MRAFT_PSB_BYTES (12u * 1024u)

/* Write-only SPU DMA (no SPU register reads — safe on consoles whose SPU
 * register reads return bus garbage, e.g. some chainloaded units).
 * bytes must be 64-aligned. ps1SpuNoteInitDone() must run right after
 * SpuInit() to probe register-read health; ps1SpuRegReadsOk() reports it. */
int  ps1SpuDmaWrite(uint32 spuByteAddr, const void *src, uint32 bytes);
int  ps1SpuDmaRead(uint32 spuByteAddr, void *dst, uint32 bytes);
void ps1SpuNoteInitDone(void);
int  ps1SpuRegReadsOk(void);

void ps1SpuCacheInit(void);
int ps1SpuCacheReady(void);
uint32 ps1SpuCacheBase(void);
uint32 ps1SpuCacheCapacity(void);
int ps1SpuCacheWrite(uint32 offset, const void *src, uint32 size);
int ps1SpuCacheRead(uint32 offset, void *dst, uint32 size);
int ps1SpuCacheSelfTest(void);

#endif /* PS1_SPU_CACHE_H */
