
static void parseMapFile(char *fileName)
{
#ifdef PS1_BUILD
    PS1File *f_map;

    /* Simplified PS1 resource parsing - no debug screens */
    f_map = ps1_fopen(fileName,"rb");

    if (f_map == NULL) {
        fatalError("Resources map file not found: %s\n", fileName);
    }

    /* Read header bytes */
    mapFile.unknown1 = ps1_readUint8(f_map);
    mapFile.unknown2 = ps1_readUint8(f_map);
    mapFile.unknown3 = ps1_readUint8(f_map);
    mapFile.unknown4 = ps1_readUint8(f_map);
    mapFile.unknown5 = ps1_readUint8(f_map);
    mapFile.unknown6 = ps1_readUint8(f_map);

    mapFile.resFileName = (char *) ps1_getString(f_map,13);
    mapFile.numEntries = ps1_readUint16(f_map);

    mapFile.Entries = safe_malloc(mapFile.numEntries * sizeof(struct TMapFileEntry));

    for (int i=0; i<mapFile.numEntries; i++) {
        mapFile.Entries[i].length = ps1_readUint32(f_map);
        mapFile.Entries[i].offset = ps1_readUint32(f_map);
    }

    ps1_fclose(f_map);
#else
    FILE *f_map;

    if (debugMode)
        printf("Opening map file: %s\n", fileName);

    f_map = fopen(fileName,"rb");

    if (f_map == NULL) {
        printf("ERROR: Cannot open map file: %s\n", fileName);
        fatalError("Resources map file not found: %s\n", fileName);
    }

    if (debugMode)
        printf("Map file opened successfully\n");

    mapFile.unknown1 = readUint8(f_map);   // first 5 uint8s unknown
    mapFile.unknown2 = readUint8(f_map);
    mapFile.unknown3 = readUint8(f_map);
    mapFile.unknown4 = readUint8(f_map);   // ? number of resources files available in this index
    mapFile.unknown5 = readUint8(f_map);
    mapFile.unknown6 = readUint8(f_map);

    mapFile.resFileName = (char *) getString(f_map,13);

    mapFile.numEntries = readUint16(f_map);

    mapFile.Entries = safe_malloc(mapFile.numEntries * sizeof(struct TMapFileEntry));

    for (int i=0; i<mapFile.numEntries; i++) {
        mapFile.Entries[i].length = readUint32(f_map);
        mapFile.Entries[i].offset = readUint32(f_map);
    }

    fclose(f_map);
#endif
}


static void parseResourceFile(char * filename)
{
#ifdef PS1_BUILD
    PS1File *f;

    /* Stream RESOURCE.001 via a 64 KB sector cache rather than loading
     * the whole ~1.1 MB file. This frees ~1 MB of libc heap for the
     * memory-region allocator's static buffer. The parser's access
     * pattern is many independent fseek+readBlock pairs (one per
     * resource entry), each touching ~50-200 bytes; the streaming
     * cache absorbs nearby entries without re-reading CD sectors. */
    f = ps1_fopen_stream(mapFile.resFileName, 64 * 1024);

    if (f == NULL) {
        fatalError("Main resources file not found: %s\n", mapFile.resFileName);
    }

    for (int i=0; i < mapFile.numEntries; i++) {

        ps1_fseek(f, mapFile.Entries[i].offset, SEEK_SET);

        mapFile.Entries[i].resName = (char *) ps1_readUint8Block(f,13);
        mapFile.Entries[i].resSize = ps1_readUint32(f);

        char *resName = mapFile.Entries[i].resName;
        char *resType = resName + strlen(resName) - 4;  // get the extension .BMP .ADS etc.

        /* Resource type debug removed - printf hangs on PS1 */

        /* PS1 FG2 runtime only needs BMP/SCR/PAL metadata. ADS/TTM metadata
         * belongs to the retired interpreter path and is skipped at startup. */
        if (!strcmp(resType, ".ADS")) {
            continue;
        }
        else if (!strcmp(resType, ".BMP")) {
            bmpResources[numBmpResources] = ps1_parseBmpResource(f, resName);
            if (bmpResources[numBmpResources] != NULL) {
                bmpResources[numBmpResources]->resourceType = RESOURCE_TYPE_BMP;
                bmpResources[numBmpResources]->resourceIndex = (uint16)numBmpResources;
                hashInsert(bmpHashTable, RESOURCE_HASH_SIZE, bmpResources[numBmpResources]->resName, (uint16)numBmpResources);
                numBmpResources++;
            }
        }
        else if (!strcmp(resType, ".PAL")) {
            palResources[numPalResources] = ps1_parsePalResource(f, resName);
            if (palResources[numPalResources] != NULL) {
                numPalResources++;
            }
        }
        else if (!strcmp(resType, ".SCR")) {
            scrResources[numScrResources] = ps1_parseScrResource(f, resName);
            if (scrResources[numScrResources] != NULL) {
                scrResources[numScrResources]->resourceType = RESOURCE_TYPE_SCR;
                scrResources[numScrResources]->resourceIndex = (uint16)numScrResources;
                hashInsert(scrHashTable, RESOURCE_HASH_SIZE, scrResources[numScrResources]->resName, (uint16)numScrResources);
                numScrResources++;
            }
        }
        else if (!strcmp(resType, ".TTM")) {
            continue;
        }
    }

    ps1_fclose(f);

    /* Init-time diagnostic: safe to printf here (before game loop) */
    printf("Resources: %d ADS, %d TTM, %d BMP, %d SCR, %d PAL\n",
           numAdsResources, numTtmResources, numBmpResources,
           numScrResources, numPalResources);
#else
    FILE *f;

    f = fopen(mapFile.resFileName,"rb");

    if (f == NULL)
        fatalError("Main resources file not found: %s\n", mapFile.resFileName);

    if (debugMode) {
        printf("Loading resources ");
        fflush (stdout);
    }

    for (int i=0; i < mapFile.numEntries; i++) {

        fseek(f, mapFile.Entries[i].offset, SEEK_SET);

        mapFile.Entries[i].resName = (char *) readUint8Block(f,13);
        mapFile.Entries[i].resSize = readUint32(f);

        char *resName = mapFile.Entries[i].resName;
        char *resType = resName + strlen(resName) - 4;  // get the extension .BMP .ADS etc.

        if (debugMode) {
             putchar('.');
             fflush(stdout);
        }

        if (!strcmp(resType, ".ADS")) {
            adsResources[numAdsResources] = parseAdsResource(f);
            adsResources[numAdsResources]->resName = resName;
            adsResources[numAdsResources]->resourceType = RESOURCE_TYPE_ADS;
            adsResources[numAdsResources]->resourceIndex = (uint16)numAdsResources;
            hashInsert(adsHashTable, RESOURCE_HASH_SIZE, resName, (uint16)numAdsResources);
            numAdsResources++;
        }
        else if (!strcmp(resType, ".BMP")) {
            bmpResources[numBmpResources] = parseBmpResource(f);
            bmpResources[numBmpResources]->resName = resName;
            bmpResources[numBmpResources]->resourceType = RESOURCE_TYPE_BMP;
            bmpResources[numBmpResources]->resourceIndex = (uint16)numBmpResources;
            hashInsert(bmpHashTable, RESOURCE_HASH_SIZE, resName, (uint16)numBmpResources);
            numBmpResources++;
        }
        else if (!strcmp(resType, ".PAL")) {
            palResources[numPalResources] = parsePalResource(f);
            palResources[numPalResources]->resName = resName;
            numPalResources++;
        }
        else if (!strcmp(resType, ".SCR")) {
            scrResources[numScrResources] = parseScrResource(f);
            scrResources[numScrResources]->resName = resName;
            scrResources[numScrResources]->resourceType = RESOURCE_TYPE_SCR;
            scrResources[numScrResources]->resourceIndex = (uint16)numScrResources;
            hashInsert(scrHashTable, RESOURCE_HASH_SIZE, resName, (uint16)numScrResources);
            numScrResources++;
        }
        else if (!strcmp(resType, ".TTM")) {
            ttmResources[numTtmResources] = parseTtmResource(f);
            ttmResources[numTtmResources]->resName = resName;
            ttmResources[numTtmResources]->resourceType = RESOURCE_TYPE_TTM;
            ttmResources[numTtmResources]->resourceIndex = (uint16)numTtmResources;
            hashInsert(ttmHashTable, RESOURCE_HASH_SIZE, resName, (uint16)numTtmResources);
            numTtmResources++;
        }
        // Note: there is one .VIN type file too (FILES.VIN)
        // We dont process it since it's nothing else than a list
        // of files, which we dont need
    }

    fclose(f);

    if (debugMode)
        putchar('\n');
#endif
}


void parseResourceFiles(char * filename)
{
    parseMapFile(filename);
    parseResourceFile(filename);
}


/* Plan v9 removal manifest items #10-13: unified resource-not-found
 * handling across PS1 and PC. The "PS1 returns NULL, caller handles
 * gracefully" pattern was a workaround for boot-time crashes when
 * fatalError tried to printf during the game loop. With the
 * deterministic allocator + memHalt routing (JC_BSOD when graphics
 * is up, ps1DebugError pre-graphics), both halt mechanisms are
 * always safe to call. Missing-resource = data bug; surface it. */

struct TAdsResource *findAdsResource(char *searchString)
{
    int idx = hashLookup(adsHashTable, RESOURCE_HASH_SIZE, searchString);
    if (idx >= 0)
        return adsResources[idx];
    fatalError("ADS resource %s not found.", searchString);
    return NULL;  /* unreachable; satisfies compiler */
}


struct TBmpResource *findBmpResource(char *searchString)
{
    int idx = hashLookup(bmpHashTable, RESOURCE_HASH_SIZE, searchString);
    if (idx >= 0)
        return bmpResources[idx];
    fatalError("BMP resource %s not found.", searchString);
    return NULL;
}


struct TScrResource *findScrResource(char *searchString)
{
    int idx = hashLookup(scrHashTable, RESOURCE_HASH_SIZE, searchString);
    if (idx >= 0)
        return scrResources[idx];
    fatalError("SCR resource %s not found.", searchString);
    return NULL;
}


struct TTtmResource *findTtmResource(char *searchString)
{
    int idx = hashLookup(ttmHashTable, RESOURCE_HASH_SIZE, searchString);
    if (idx >= 0)
        return ttmResources[idx];
    fatalError("TTM resource %s not found.", searchString);
    return NULL;
}
