static void adsReapTerminatedThreads(void)
{
    for (int i = 0; i < MAX_TTM_THREADS; i++) {
        if (ttmThreads[i].isRunning == 2) {
            adsStopScene(i);
        }
    }
}

static int adsStoreChunk(uint16 slot, uint16 tag, uint32 offset)
{
    if (numAdsChunks >= MAX_ADS_CHUNKS) {
        debugMsg("Warning : ADS chunk table overflow (%d,%d) at %lu", slot, tag, (unsigned long)offset);
        return 0;
    }

    adsChunks[numAdsChunks].scene.slot = slot;
    adsChunks[numAdsChunks].scene.tag  = tag;
    adsChunks[numAdsChunks].offset     = offset;
    numAdsChunks++;
    return 1;
}

static int adsStoreLocalChunk(uint16 slot, uint16 tag, uint32 offset)
{
    if (numAdsChunksLocal >= MAX_ADS_CHUNKS_LOCAL) {
        debugMsg("Warning : ADS local chunk table overflow (%d,%d) at %lu", slot, tag, (unsigned long)offset);
        return 0;
    }

    adsChunksLocal[numAdsChunksLocal].scene.slot = slot;
    adsChunksLocal[numAdsChunksLocal].scene.tag  = tag;
    adsChunksLocal[numAdsChunksLocal].offset     = offset;
    numAdsChunksLocal++;
    return 1;
}

static int adsStoreTag(uint16 id, uint32 offset)
{
    if (adsTags == NULL || adsNumTags >= adsTagCapacity) {
        debugMsg("Warning : ADS tag table overflow (tag=%u offset=%lu)", id, (unsigned long)offset);
        return 0;
    }

    adsTags[adsNumTags].id     = id;
    adsTags[adsNumTags].offset = offset;
    adsNumTags++;
    return 1;
}

static int adsEnsureWordArgs(uint32 dataSize, uint32 offset, uint8 numArgs)
{
    if (numArgs > 10)
        return 0;
    if (offset > dataSize)
        return 0;
    if ((uint32)numArgs > ((dataSize - offset) >> 1))
        return 0;
    return 1;
}

static int adsPeekWordArgs(uint8 *data, uint32 dataSize, uint32 *offset, uint16 *args, uint8 numArgs)
{
    if (data == NULL || offset == NULL || args == NULL)
        return 0;
    if (!adsEnsureWordArgs(dataSize, *offset, numArgs))
        return 0;
    peekUint16Block(data, offset, args, numArgs);
    return 1;
}

static int adsSkipWordArgs(uint32 dataSize, uint32 *offset, uint8 numArgs)
{
    if (offset == NULL)
        return 0;
    if (!adsEnsureWordArgs(dataSize, *offset, numArgs))
        return 0;
    *offset += ((uint32)numArgs << 1);
    return 1;
}


static void adsLoad(uint8 *data, uint32 dataSize, uint16 numTags, uint16 tag, uint32 *tagOffset)
{
    uint32 offset = 0;
    uint16 args[10];
    int bookmarkingChunks = 0;
    int bookmarkingIfNotRunnings = 0;

    numAdsChunks      = 0;
    numAdsChunksLocal = 0;
    *tagOffset        = 0;
    adsNumTags        = 0;
    adsTagCapacity    = numTags;
    adsTags           = safe_malloc(numTags * sizeof(struct TTtmTag));


    while (offset < dataSize) {

        uint16 opcode = peekUint16(data, &offset);

        switch (opcode) {

            case 0x1350:     // IF_LASTPLAYED

                if (bookmarkingChunks) {
                    bookmarkingIfNotRunnings = 0;
                    if (!adsPeekWordArgs(data, dataSize, &offset, args, 2))
                        goto done;
                    adsStoreChunk(args[0], args[1], offset);
                }
                else {
                    if (!adsSkipWordArgs(dataSize, &offset, 2))
                        goto done;
                }

                break;

            case 0x1360:     // IF_NOT_RUNNING

                // We only bookmark the IF_NOT_RUNNINGs
                // preceding the first IF_LAST_PLAYED or IF_IS_RUNNING

                if (bookmarkingChunks && bookmarkingIfNotRunnings) {
                    if (!adsPeekWordArgs(data, dataSize, &offset, args, 2))
                        goto done;
                    adsStoreChunk(args[0], args[1], offset);
                }
                else {
                    if (!adsSkipWordArgs(dataSize, &offset, 2))
                        goto done;
                }

                break;

            case 0x1370:     // IF_IS_RUNNING
                bookmarkingIfNotRunnings = 0;
                if (!adsSkipWordArgs(dataSize, &offset, 2))
                    goto done;
                break;

            case 0x1070: if (!adsSkipWordArgs(dataSize, &offset, 2)) goto done; break;
            case 0x1330: if (!adsSkipWordArgs(dataSize, &offset, 2)) goto done; break;
            case 0x1420: break;
            case 0x1430: break; /* OR has no payload; IF_LASTPLAYED state is handled by the interpreter pass. */
            case 0x1510: break;
            case 0x1520: if (!adsSkipWordArgs(dataSize, &offset, 5)) goto done; break;
            case 0x2005: if (!adsSkipWordArgs(dataSize, &offset, 4)) goto done; break;
            case 0x2010: if (!adsSkipWordArgs(dataSize, &offset, 3)) goto done; break;
            case 0x2014: break;
            case 0x3010: break;
            case 0x3020: if (!adsSkipWordArgs(dataSize, &offset, 1)) goto done; break;
            case 0x30ff: break;
            case 0x4000: if (!adsSkipWordArgs(dataSize, &offset, 3)) goto done; break;
            case 0xf010: break;
            case 0xf200: if (!adsSkipWordArgs(dataSize, &offset, 1)) goto done; break;
            case 0xffff: break;
            case 0xfff0: break;

            default:
                adsStoreTag(opcode, offset);

                if (opcode == tag) {
                    *tagOffset = offset;
                    bookmarkingChunks = 1;
                    bookmarkingIfNotRunnings = 1;
                }
                else {
                    bookmarkingChunks = 0;
                    bookmarkingIfNotRunnings = 0;
                }

                break;
        }
    }

done:
    if (adsNumTags != numTags)
        debugMsg("Warning : didn't find every tag in ADS data");

    if (*tagOffset == 0)
        debugMsg("Warning : ADS tag #%d not found, starting from offset 0", tag);
}


static void adsReleaseAds()
{
    free(adsTags);
}

static uint16 adsGetScenePlayCount(uint16 ttmSlotNo, uint16 ttmTag)
{
    for (int i = 0; i < numAdsPlayCounts; i++) {
        if (adsPlayCounts[i].slot == ttmSlotNo && adsPlayCounts[i].tag == ttmTag)
            return adsPlayCounts[i].count;
    }

    return 0;
}


static void adsNoteSceneStarted(uint16 ttmSlotNo, uint16 ttmTag)
{
    for (int i = 0; i < numAdsPlayCounts; i++) {
        if (adsPlayCounts[i].slot == ttmSlotNo && adsPlayCounts[i].tag == ttmTag) {
            if (adsPlayCounts[i].count < 0xFFFF)
                adsPlayCounts[i].count++;
            return;
        }
    }

    if (numAdsPlayCounts < MAX_ADS_PLAY_COUNTS) {
        adsPlayCounts[numAdsPlayCounts].slot = ttmSlotNo;
        adsPlayCounts[numAdsPlayCounts].tag = ttmTag;
        adsPlayCounts[numAdsPlayCounts].count = 1;
        numAdsPlayCounts++;
    }
}


static int adsConsumePendingStop(uint16 ttmSlotNo, uint16 ttmTag)
{
    for (int i = 0; i < numAdsPendingStops; i++) {
        if (adsPendingStops[i].slot == ttmSlotNo && adsPendingStops[i].tag == ttmTag) {
            numAdsPendingStops--;
            adsPendingStops[i] = adsPendingStops[numAdsPendingStops];
            return 1;
        }
    }

    return 0;
}


static void adsRecordPendingStop(uint16 ttmSlotNo, uint16 ttmTag)
{
    for (int i = 0; i < numAdsPendingStops; i++) {
        if (adsPendingStops[i].slot == ttmSlotNo && adsPendingStops[i].tag == ttmTag)
            return;
    }

    if (numAdsPendingStops < MAX_ADS_PENDING_STOPS) {
        adsPendingStops[numAdsPendingStops].slot = ttmSlotNo;
        adsPendingStops[numAdsPendingStops].tag = ttmTag;
        numAdsPendingStops++;
    }
}


static uint32 adsFindTag(uint16 reqdTag)
{
    uint32 result = 0;
    int i = 0;

    while (result == 0 && i < adsNumTags) {

        if (adsTags[i].id == reqdTag)
            result = adsTags[i].offset;
        else
            i++;
    }

    if (result == 0) {
#ifndef PS1_BUILD
        fprintf(stderr, "Warning : ADS tag #%d not found, returning offset 0000\n", reqdTag);
#endif
    }

    return result;
}
