#ifndef PS1_SPU_CACHE_H
#define PS1_SPU_CACHE_H

#include "mytypes.h"

#define PS1_SPU_CACHE_ALIGN 64u

void ps1SpuCacheInit(void);
int ps1SpuCacheReady(void);
uint32 ps1SpuCacheBase(void);
uint32 ps1SpuCacheCapacity(void);
int ps1SpuCacheWrite(uint32 offset, const void *src, uint32 size);
int ps1SpuCacheRead(uint32 offset, void *dst, uint32 size);
int ps1SpuCacheSelfTest(void);

#endif /* PS1_SPU_CACHE_H */
