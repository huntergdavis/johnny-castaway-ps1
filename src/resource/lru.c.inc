/* ============================================================================
 * LRU Cache Implementation
 * ============================================================================ */

void initLRUCache(void) {
    globalTick = 0;
    totalMemoryUsed = 0;

#ifndef PS1_BUILD
    /* Check for JC_MEM_BUDGET_MB environment variable (PC only) */
    char *budgetEnv = getenv("JC_MEM_BUDGET_MB");
    if (budgetEnv != NULL) {
        int budgetMB = atoi(budgetEnv);
        if (budgetMB > 0) {
            memoryBudget = (size_t)budgetMB * 1024 * 1024;
            if (debugMode) {
                printf("LRU cache: Memory budget set to %d MB\n", budgetMB);
            }
        }
    }
#endif

    /* Initialize all resource LRU fields */
    for (int i = 0; i < numAdsResources; i++) {
        adsResources[i]->lastUsedTick = 0;
        adsResources[i]->pinCount = 0;
    }
    for (int i = 0; i < numBmpResources; i++) {
        bmpResources[i]->lastUsedTick = 0;
        bmpResources[i]->pinCount = 0;
    }
    for (int i = 0; i < numScrResources; i++) {
        scrResources[i]->lastUsedTick = 0;
        scrResources[i]->pinCount = 0;
    }
    for (int i = 0; i < numTtmResources; i++) {
        ttmResources[i]->lastUsedTick = 0;
        ttmResources[i]->pinCount = 0;
    }
}

void touchResource(void *resource) {
    /* All resource structs share the same layout for the first 3 fields:
     *   char *resName; uint8 resourceType; uint16 resourceIndex;
     * So we can cast to any struct type to read resourceType. */
    struct TAdsResource *r = (struct TAdsResource *)resource;
    globalTick++;

    switch (r->resourceType) {
    case RESOURCE_TYPE_ADS:
        adsResources[r->resourceIndex]->lastUsedTick = globalTick;
        break;
    case RESOURCE_TYPE_BMP:
        bmpResources[((struct TBmpResource *)resource)->resourceIndex]->lastUsedTick = globalTick;
        break;
    case RESOURCE_TYPE_SCR:
        scrResources[((struct TScrResource *)resource)->resourceIndex]->lastUsedTick = globalTick;
        break;
    case RESOURCE_TYPE_TTM:
        ttmResources[((struct TTtmResource *)resource)->resourceIndex]->lastUsedTick = globalTick;
        break;
    }
}

void pinResource(void *resource, uint32 size, const char *type) {
    struct TAdsResource *r = (struct TAdsResource *)resource;
    touchResource(resource);

    /* O(1) dispatch using resourceType field */
    switch (r->resourceType) {
    case RESOURCE_TYPE_ADS: {
        struct TAdsResource *ads = (struct TAdsResource *)resource;
        if (ads->pinCount == 0 && ads->uncompressedData != NULL) {
            totalMemoryUsed += size;
        }
        ads->pinCount++;
        break;
    }
    case RESOURCE_TYPE_TTM: {
        struct TTtmResource *ttm = (struct TTtmResource *)resource;
        if (ttm->pinCount == 0 && ttm->uncompressedData != NULL) {
            totalMemoryUsed += size;
        }
        ttm->pinCount++;
        break;
    }
    default:
        break;
    }
}

void unpinResource(void *resource, const char *type) {
    struct TAdsResource *r = (struct TAdsResource *)resource;

    /* O(1) dispatch using resourceType field */
    switch (r->resourceType) {
    case RESOURCE_TYPE_ADS: {
        struct TAdsResource *ads = (struct TAdsResource *)resource;
        if (ads->pinCount > 0) {
            ads->pinCount--;
            if (ads->pinCount == 0 && ads->uncompressedData != NULL) {
                if (totalMemoryUsed >= ads->uncompressedSize)
                    totalMemoryUsed -= ads->uncompressedSize;
                else
                    totalMemoryUsed = 0;
            }
        }
        break;
    }
    case RESOURCE_TYPE_TTM: {
        struct TTtmResource *ttm = (struct TTtmResource *)resource;
        if (ttm->pinCount > 0) {
            ttm->pinCount--;
            if (ttm->pinCount == 0 && ttm->uncompressedData != NULL) {
                if (totalMemoryUsed >= ttm->uncompressedSize)
                    totalMemoryUsed -= ttm->uncompressedSize;
                else
                    totalMemoryUsed = 0;
            }
        }
        break;
    }
    default:
        break;
    }
}

void checkMemoryBudget(void) {
    if (totalMemoryUsed <= memoryBudget) {
        return;
    }

    if (debugMode) {
        printf("LRU cache: Memory over budget (%.2f MB / %.2f MB), evicting...\n",
               totalMemoryUsed / (1024.0 * 1024.0),
               memoryBudget / (1024.0 * 1024.0));
    }

    /* Find and evict LRU unpinned resources */
    while (totalMemoryUsed > memoryBudget) {
        void *lruResource = NULL;
        uint32 lruTick = globalTick + 1;
        size_t lruSize = 0;
        char lruType[10] = "";

        /* Find LRU unpinned ADS */
        for (int i = 0; i < numAdsResources; i++) {
            if (adsResources[i]->uncompressedData != NULL &&
                adsResources[i]->pinCount == 0 &&
                adsResources[i]->lastUsedTick < lruTick) {
                lruResource = adsResources[i];
                lruTick = adsResources[i]->lastUsedTick;
                lruSize = adsResources[i]->uncompressedSize;
                strcpy(lruType, "ADS");
            }
        }

        /* Find LRU unpinned TTM */
        for (int i = 0; i < numTtmResources; i++) {
            if (ttmResources[i]->uncompressedData != NULL &&
                ttmResources[i]->pinCount == 0 &&
                ttmResources[i]->lastUsedTick < lruTick) {
                lruResource = ttmResources[i];
                lruTick = ttmResources[i]->lastUsedTick;
                lruSize = ttmResources[i]->uncompressedSize;
                strcpy(lruType, "TTM");
            }
        }

        /* Evict the LRU resource */
        if (lruResource != NULL) {
            if (strcmp(lruType, "ADS") == 0) {
                struct TAdsResource *ads = (struct TAdsResource *)lruResource;
                if (debugMode) {
                    printf("LRU cache: Evicting %s (%.2f KB)\n",
                           ads->resName, lruSize / 1024.0);
                }
                /* PS1: uncompressedData is CACHE-region allocated
                 * via ps1_streamReadFromCdFile (or ps1_fopen file
                 * buffer). LRU evictor frees through memFree. */
#ifdef PS1_BUILD
                memFree(MEM_REGION_CACHE, ads->uncompressedData);
#else
                free(ads->uncompressedData);
#endif
                ads->uncompressedData = NULL;
                totalMemoryUsed -= lruSize;
            } else if (strcmp(lruType, "TTM") == 0) {
                struct TTtmResource *ttm = (struct TTtmResource *)lruResource;
                if (debugMode) {
                    printf("LRU cache: Evicting %s (%.2f KB)\n",
                           ttm->resName, lruSize / 1024.0);
                }
#ifdef PS1_BUILD
                memFree(MEM_REGION_CACHE, ttm->uncompressedData);
#else
                free(ttm->uncompressedData);
#endif
                ttm->uncompressedData = NULL;
                totalMemoryUsed -= lruSize;
            }
        } else {
            /* No unpinned resources to evict */
            if (debugMode) {
                printf("LRU cache: All resources pinned, cannot evict\n");
            }
            break;
        }
    }
}

size_t getTotalMemoryUsed(void) {
    return totalMemoryUsed;
}

size_t getMemoryBudget(void) {
    return memoryBudget;
}

/* R33-soak panic mode: drop EVERY unpinned LRU resource regardless
 * of memoryBudget. Walks ADS/TTM AND BMP/SCR (checkMemoryBudget only
 * handles ADS+TTM by design); the R33j soak showed BMP/SCR
 * uncompressedData accounts for 200–400 KB of CACHE that
 * checkMemoryBudget can't reach, blocking the post-eviction CACHE
 * rewind. */
void lruEvictAllUnpinned(void) {
    /* Phase 1: existing ADS/TTM evictor (drives totalMemoryUsed
     * accounting that checkMemoryBudget cares about). */
    size_t savedBudget = memoryBudget;
    memoryBudget = 0;
    checkMemoryBudget();
    memoryBudget = savedBudget;

#ifdef PS1_BUILD
    /* Phase 2: walk BMP and SCR arrays directly. Their uncompressed
     * data buffers go through the same CACHE allocator but aren't
     * tracked in totalMemoryUsed / aren't handled by checkMemoryBudget.
     * For the CACHE rewind to succeed (g_cacheUsed must reach 0),
     * these must be freed too. */
    for (int i = 0; i < numBmpResources; i++) {
        struct TBmpResource *bmp = bmpResources[i];
        if (bmp != NULL &&
            bmp->uncompressedData != NULL &&
            bmp->pinCount == 0) {
            memFree(MEM_REGION_CACHE, bmp->uncompressedData);
            bmp->uncompressedData = NULL;
        }
    }
    for (int i = 0; i < numScrResources; i++) {
        struct TScrResource *scr = scrResources[i];
        if (scr != NULL &&
            scr->uncompressedData != NULL &&
            scr->pinCount == 0) {
            memFree(MEM_REGION_CACHE, scr->uncompressedData);
            scr->uncompressedData = NULL;
        }
    }
#endif
}
