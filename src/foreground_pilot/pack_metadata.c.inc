static void fgApplySceneRelativeOffsets(struct TFgPilotHeader *header,
                                        struct TFgPilotEntryTable *table)
{
    uint16 i;

    if (header == NULL ||
        table == NULL ||
        table->entries == NULL ||
        (header->reserved0 & kFgPilotHeaderFlagSceneRelative) == 0)
        return;

    for (i = 0; i < table->count; i++) {
        table->entries[i].x = (sint16)(table->entries[i].x + gFgSceneDrawOffsetX);
        table->entries[i].y = (sint16)(table->entries[i].y + gFgSceneDrawOffsetY);
    }
    header->reserved0 = (uint16)(header->reserved0 & ~kFgPilotHeaderFlagSceneRelative);
}

static uint16 fgConvertHostTicksToVBlanks(uint16 ticks)
{
    uint32 scaled = (uint32)ticks * 6u;
    uint16 hold = (uint16)((scaled + 4u) / 5u);
    return hold > 0 ? hold : 1;
}

static uint16 fgEntryHoldVBlanks(const struct TFgPilotHeader *header,
                                 const struct TFgPilotEntry *entry,
                                 uint16 presentedVBlanks)
{
    uint16 hold = 0;

    if (entry != NULL)
        hold = entry->reserved0;
    if (hold == 0 && header != NULL)
        hold = header->displayVBlanks;
    if (hold == 0)
        hold = 1;

    if (header != NULL && (header->reserved0 & kFgPilotHeaderFlagHostDeadlines) != 0) {
        uint16 targetVBlanks = fgConvertHostTicksToVBlanks(hold);
        hold = (targetVBlanks > presentedVBlanks)
            ? (uint16)(targetVBlanks - presentedVBlanks)
            : 1;
    } else if (header != NULL && (header->reserved0 & kFgPilotHeaderFlagHostTicks) != 0) {
        hold = fgConvertHostTicksToVBlanks(hold);
    }

    return hold;
}

static int fgSceneRoutePrefixMatches(const char *sceneName,
                                     const char *prefix,
                                     const char **digitsOut)
{
    while (*prefix != '\0') {
        if (*sceneName != *prefix)
            return 0;
        sceneName++;
        prefix++;
    }
    if (*sceneName < '0' || *sceneName > '9')
        return 0;
    *digitsOut = sceneName;
    return 1;
}

static int fgParseCompactOverlayScene(const char *sceneName,
                                      const struct TFgPilotSceneFamily **familyOut,
                                      uint16 *tagOut)
{
    uint16 i;

    if (sceneName == NULL)
        return 0;

    for (i = 0; i < (uint16)(sizeof(kFgPilotSceneFamilies) /
                             sizeof(kFgPilotSceneFamilies[0])); i++) {
        const char *digits = NULL;
        uint16 tag = 0;
        if (!fgSceneRoutePrefixMatches(sceneName,
                                       kFgPilotSceneFamilies[i].scenePrefix,
                                       &digits))
            continue;
        while (*digits >= '0' && *digits <= '9') {
            tag = (uint16)((tag * 10u) + (uint16)(*digits - '0'));
            digits++;
        }
        if (*digits != '\0' || tag == 0 || tag >= 32)
            return 0;
        if ((kFgPilotSceneFamilies[i].tagMask & FG_TAG_MASK(tag)) == 0)
            return 0;
        if (familyOut != NULL)
            *familyOut = &kFgPilotSceneFamilies[i];
        if (tagOut != NULL)
            *tagOut = tag;
        return 1;
    }

    return 0;
}

static char *fgAppendText(char *dst, const char *src)
{
    while (*src != '\0') {
        *dst = *src;
        dst++;
        src++;
    }
    return dst;
}

static char *fgAppendTag(char *dst, uint16 tag)
{
    if (tag >= 10) {
        *dst = (char)('0' + (tag / 10));
        dst++;
    }
    *dst = (char)('0' + (tag % 10));
    dst++;
    return dst;
}

static const char *fgBuildCompactOverlayPackPath(const struct TFgPilotSceneFamily *family,
                                                 uint16 tag,
                                                 int lowTide)
{
    char *dst = gFgCompactOverlayPackPath;
    char *baseStart;
    const char *prefix;
    int baseLen;

    *dst++ = 'F';
    *dst++ = 'G';
    *dst++ = '\\';
    baseStart = dst;
    prefix = lowTide ? family->lowPackPrefix : family->highPackPrefix;
    dst = fgAppendText(dst, prefix);
    dst = fgAppendTag(dst, tag);
    if (lowTide) {
        baseLen = (int)(dst - baseStart);
        if (baseLen + 3 <= 8)
            dst = fgAppendText(dst, "LOW");
        else
            dst = fgAppendText(dst, "L");
    }
    dst = fgAppendText(dst, ".FG2");
    *dst = '\0';
    return gFgCompactOverlayPackPath;
}

static const char *fgCompactOverlayPackPathForScene(const char *sceneName)
{
    const struct TFgPilotSceneFamily *family = NULL;
    uint16 tag = 0;

    if (!fgParseCompactOverlayScene(sceneName, &family, &tag))
        return NULL;

    return fgBuildCompactOverlayPackPath(family, tag, islandState.lowTide);
}

static uint16 fgReadU16(const uint8 *p)
{
    return (uint16)((uint16)p[0] | ((uint16)p[1] << 8));
}

static sint16 fgReadS16(const uint8 *p)
{
    return (sint16)fgReadU16(p);
}

static uint32 fgReadU32(const uint8 *p)
{
    return (uint32)p[0] |
           ((uint32)p[1] << 8) |
           ((uint32)p[2] << 16) |
           ((uint32)p[3] << 24);
}

static int
fgDecodeFrameDelta(uint8 *data,
                   uint32 dataCapacity,
                   struct TFgPilotEntry *entry,
                   const uint8 *baseData,
                   uint32 baseSize)
{
    uint8 *stream;
    uint16 expandedSize;
    uint16 commandCount;
    uint32 readOffset;
    uint32 writeOffset;

    if (data == NULL || entry == NULL)
        return 0;

    if (entry->dataSize < 8u ||
        fgReadU16(data) != FG_DELTA_PAYLOAD_SENTINEL ||
        data[2] != FG_DELTA_PAYLOAD_MAGIC0 ||
        data[3] != FG_DELTA_PAYLOAD_MAGIC1) {
        return 1;
    }

    if (baseData == NULL ||
        gFgRuntime.streamScratch == NULL ||
        entry->dataSize > gFgRuntime.streamScratchSize)
        return 0;

    memcpy(gFgRuntime.streamScratch, data, entry->dataSize);
    stream = gFgRuntime.streamScratch;
    expandedSize = fgReadU16(stream + 4);
    commandCount = fgReadU16(stream + 6);
    if ((uint32)expandedSize > dataCapacity)
        return 0;
    readOffset = 8u;
    writeOffset = 0;

    while (commandCount > 0) {
        uint8 opcode;
        uint16 length;

        if (readOffset >= entry->dataSize)
            return 0;
        opcode = stream[readOffset++];
        if (opcode == 0) {
            uint16 baseOffset;
            if (readOffset + 4u > entry->dataSize)
                return 0;
            baseOffset = fgReadU16(stream + readOffset);
            length = fgReadU16(stream + readOffset + 2u);
            readOffset += 4u;
            if ((uint32)baseOffset + (uint32)length > baseSize ||
                writeOffset + (uint32)length > (uint32)expandedSize)
                return 0;
            memcpy(data + writeOffset, baseData + baseOffset, length);
            writeOffset += length;
        } else {
            if (readOffset + 2u > entry->dataSize)
                return 0;
            length = fgReadU16(stream + readOffset);
            readOffset += 2u;
            if (readOffset + (uint32)length > entry->dataSize ||
                writeOffset + (uint32)length > (uint32)expandedSize)
                return 0;
            memcpy(data + writeOffset, stream + readOffset, length);
            readOffset += length;
            writeOffset += length;
        }
        commandCount--;
    }

    if (writeOffset != (uint32)expandedSize)
        return 0;
    entry->dataSize = expandedSize;
    return 1;
}

static int
fgDecodeLocalLzPayload(uint8 *data,
                       uint32 dataCapacity,
                       struct TFgPilotEntry *entry)
{
    uint8 *stream;
    uint32 expandedSize;
    uint32 readOffset;
    uint32 writeOffset;

    if (data == NULL || entry == NULL)
        return 0;

    if (entry->dataSize < 8u ||
        fgReadU16(data) != FG_LOCAL_LZ_PAYLOAD_SENTINEL ||
        data[2] != FG_LOCAL_LZ_PAYLOAD_MAGIC0 ||
        data[3] != FG_LOCAL_LZ_PAYLOAD_MAGIC1) {
        return 1;
    }

    expandedSize = fgReadU32(data + 4);
    if (entry->dataSize > gFgRuntime.streamScratchSize ||
        expandedSize > dataCapacity)
        return 0;

    memcpy(gFgRuntime.streamScratch, data, entry->dataSize);
    stream = gFgRuntime.streamScratch;
    readOffset = 8u;
    writeOffset = 0;

    while (readOffset < entry->dataSize && writeOffset < expandedSize) {
        uint8 opcode;
        uint16 length;

        if (readOffset >= entry->dataSize)
            return 0;
        opcode = stream[readOffset++];
        if (opcode == 0) {
            uint32 baseOffset;
            if (readOffset + 6u > entry->dataSize)
                return 0;
            baseOffset = fgReadU32(stream + readOffset);
            length = fgReadU16(stream + readOffset + 4u);
            readOffset += 6u;
            if (baseOffset + (uint32)length > writeOffset ||
                writeOffset + (uint32)length > expandedSize)
                return 0;
            memcpy(data + writeOffset, data + baseOffset, length);
            writeOffset += length;
        } else {
            if (readOffset + 2u > entry->dataSize)
                return 0;
            length = fgReadU16(stream + readOffset);
            readOffset += 2u;
            if (readOffset + (uint32)length > entry->dataSize ||
                writeOffset + (uint32)length > expandedSize)
                return 0;
            memcpy(data + writeOffset, stream + readOffset, length);
            readOffset += length;
            writeOffset += length;
        }
    }

    if (writeOffset != expandedSize)
        return 0;
    entry->dataSize = expandedSize;
    return 1;
}

static int fgRuntimeUsesPreviousFrameDelta(uint16 frameIndex)
{
    if (gFgRuntimeSceneId == FG_SCENE_VISITOR3) {
        if (islandState.lowTide)
            return frameIndex == 129 || frameIndex == 132 ||
                   frameIndex == 134 || frameIndex == 135 ||
                   frameIndex == 137;
        return frameIndex == 132 || frameIndex == 137;
    }
    if (!islandState.lowTide && gFgRuntimeSceneId == FG_SCENE_WALKSTUF1)
        return frameIndex == 92;
    if (islandState.lowTide && gFgRuntimeSceneId == FG_SCENE_BUILDING2)
        return frameIndex == 71 || frameIndex == 77;
    return 0;
}

static void fgParseHeader(const uint8 *data, struct TFgPilotHeader *out)
{
    memcpy(out->magic, data, 4);
    out->version = fgReadU16(data + 4);
    out->frameCount = fgReadU16(data + 6);
    out->displayVBlanks = fgReadU16(data + 8);
    out->reserved0 = fgReadU16(data + 10);
    out->screenWidth = fgReadU16(data + 12);
    out->screenHeight = fgReadU16(data + 14);
    out->unionX = fgReadU16(data + 16);
    out->unionY = fgReadU16(data + 18);
    out->unionWidth = fgReadU16(data + 20);
    out->unionHeight = fgReadU16(data + 22);
    out->tableOffset = fgReadU32(data + 24);
    out->dataOffset = fgReadU32(data + 28);
    out->soundEventsOffset = fgReadU32(data + 32);
    out->soundEventCount = fgReadU16(data + 36);
    out->reserved1 = fgReadU16(data + 38);
}

static int fgHeaderIsPal4Spans(const struct TFgPilotHeader *header)
{
    return (header != NULL &&
            memcmp(header->magic, "FGP2", 4) == 0 &&
            header->version == 1) ? 1 : 0;
}

static int fgHeaderIsIndexed8Spans(const struct TFgPilotHeader *header)
{
    return (header != NULL &&
            memcmp(header->magic, "FGP2", 4) == 0 &&
            header->version == 2) ? 1 : 0;
}

static int fgHeaderIsPal4TemporalResidual(const struct TFgPilotHeader *header)
{
    return (header != NULL &&
            memcmp(header->magic, "FGP3", 4) == 0 &&
            (header->version == 1 ||
             header->version == 4)) ? 1 : 0;
}

static int fgHeaderIsIndexed8TemporalResidual(const struct TFgPilotHeader *header)
{
    return (header != NULL &&
            memcmp(header->magic, "FGP3", 4) == 0 &&
            header->version == 2) ? 1 : 0;
}

static uint32 fgPaletteByteCount(const struct TFgPilotHeader *header)
{
    if (fgHeaderIsPal4Spans(header) || fgHeaderIsPal4TemporalResidual(header))
        return 32;
    if (fgHeaderIsIndexed8Spans(header) || fgHeaderIsIndexed8TemporalResidual(header))
        return 512;
    return 0;
}

static int fgParseEntryTable(const uint8 *data, const struct TFgPilotHeader *header,
                             struct TFgPilotEntryTable *out)
{
    if (!data || !header || !out || header->frameCount == 0)
        return 0;

    memset(out, 0, sizeof(*out));
    /* MEM_REGION_RATIONALE: per-scene frame metadata table. Allocated
     * once per scene at pack-start, freed wholesale by memSceneReset
     * at the next fgRuntimeReset. INIT_FULL_WRITE — every entry is
     * populated by the loop below. */
    out->entries = (struct TFgPilotEntry *)memAlloc(
        MEM_REGION_TRANSIENT,
        (size_t)header->frameCount * sizeof(struct TFgPilotEntry),
        "fgPilotEntryTable");
    /* memAlloc never returns NULL (halts on exhaustion). */

    out->count = header->frameCount;
    for (uint16 i = 0; i < header->frameCount; i++) {
        const uint8 *src = data + ((uint32)i * FG_PACK_ENTRY_SIZE);
        struct TFgPilotEntry *dst = &out->entries[i];
        dst->sourceFrame = fgReadU16(src + 0);
        dst->x = fgReadS16(src + 2);
        dst->y = fgReadS16(src + 4);
        dst->width = fgReadU16(src + 6);
        dst->height = fgReadU16(src + 8);
        dst->reserved0 = fgReadU16(src + 10);
        dst->dataOffset = fgReadU32(src + 12);
        dst->dataSize = fgReadU32(src + 16);
    }

    return 1;
}

static void fgFreeEntryTable(struct TFgPilotEntryTable *table)
{
    if (table == NULL)
        return;
    if (table->entries != NULL) {
        /* TRANSIENT region — bytes are reclaimed by memSceneReset at
         * the top of fgRuntimeReset; this just decrements the balance
         * counter and clears the dangling pointer. */
        memFree(MEM_REGION_TRANSIENT, table->entries);
        table->entries = NULL;
    }
    table->count = 0;
}

static int fgLoadMetadataPrefixFromCdFile(const char *sceneName,
                                          const CdlFILE *cdfile,
                                          struct TFgPilotHeader *outHeader,
                                          uint16 *outPalette,
                                          struct TFgPilotEntryTable *outTable)
{
    uint8 *metadata;
    uint32 prefixBytes = FG_PACK_METADATA_PREFIX_BYTES;
    uint32 paletteBytes;
    uint32 tableSize;
    uint32 metadataBytes;
    uint16 paletteEntries;
    uint16 i;

    if (!cdfile || !outHeader || !outPalette || !outTable)
        return 0;

    memset(outTable, 0, sizeof(*outTable));
    for (i = 0; i < 256; i++)
        outPalette[i] = 0;

    metadata = (uint8 *)memAlloc(MEM_REGION_CACHE, prefixBytes,
                                 "fg-metadata-prefix");
    if (!metadata)
        return 0;

    if (!fgNextSceneStageReadMetadataPrefix(sceneName, metadata, prefixBytes) &&
        !ps1_streamReadAlignedIntoFile(cdfile, 0, prefixBytes, metadata)) {
        memFree(MEM_REGION_CACHE, metadata);
        return 0;
    }

    fgParseHeader(metadata, outHeader);
    if ((!fgHeaderIsPal4Spans(outHeader) &&
         !fgHeaderIsIndexed8Spans(outHeader) &&
         !fgHeaderIsPal4TemporalResidual(outHeader) &&
         !fgHeaderIsIndexed8TemporalResidual(outHeader)) ||
        outHeader->frameCount == 0) {
        memFree(MEM_REGION_CACHE, metadata);
        return 0;
    }

    paletteBytes = fgPaletteByteCount(outHeader);
    tableSize = (uint32)outHeader->frameCount * FG_PACK_ENTRY_SIZE;
    metadataBytes = outHeader->tableOffset + tableSize;
    if (metadataBytes < FG_PACK_HEADER_SIZE + paletteBytes ||
        metadataBytes > outHeader->dataOffset) {
        memFree(MEM_REGION_CACHE, metadata);
        return 0;
    }

    if (metadataBytes > prefixBytes) {
        uint8 *expanded;
        uint8 *tail;
        uint32 tailBytes = metadataBytes - prefixBytes;

        /* MEM_REGION_RATIONALE: metadata expansion follows CACHE resource
         * lifetime. Both alloc and free route through CACHE. ps1_streamRead returns
         * CACHE-region memory post-memInit, so libc free() on metadata/tail
         * silently leaked. Allocating `expanded` through CACHE too keeps
         * the libc heap (only ~77 KB free) out of the metadata path. */
        expanded = (uint8 *)memAlloc(MEM_REGION_CACHE, metadataBytes,
                                     "fg-metadata-expanded");
        if (!expanded) {
            memFree(MEM_REGION_CACHE, metadata);
            return 0;
        }
        memcpy(expanded, metadata, prefixBytes);
        memFree(MEM_REGION_CACHE, metadata);

        tail = (uint8 *)memAlloc(MEM_REGION_CACHE, tailBytes,
                                 "fg-metadata-tail");
        if (!tail ||
            !ps1_streamReadIntoFile(cdfile, prefixBytes, tailBytes, tail)) {
            if (tail)
                memFree(MEM_REGION_CACHE, tail);
            memFree(MEM_REGION_CACHE, expanded);
            return 0;
        }
        memcpy(expanded + prefixBytes, tail, tailBytes);
        memFree(MEM_REGION_CACHE, tail);
        metadata = expanded;
    }

    paletteEntries = (uint16)(paletteBytes / 2u);
    for (i = 0; i < paletteEntries; i++)
        outPalette[i] = fgReadU16(metadata + FG_PACK_HEADER_SIZE + ((uint32)i * 2u));

    if (!fgParseEntryTable(metadata + outHeader->tableOffset, outHeader, outTable)) {
        memFree(MEM_REGION_CACHE, metadata);
        return 0;
    }

    memFree(MEM_REGION_CACHE, metadata);
    return 1;
}

static const struct TFgPilotEntry *fgGetEntryFromTable(const struct TFgPilotEntryTable *table,
                                                       uint16 frameIndex)
{
    if (table == NULL || table->entries == NULL || frameIndex >= table->count)
        return NULL;

    return &table->entries[frameIndex];
}

static int fgSceneEquals(const char *a, const char *b)
{
    return a && b && strcmp(a, b) == 0;
}

static uint8 fgSceneIdForName(const char *sceneName)
{
    if (fgSceneEquals(sceneName, "building2"))
        return FG_SCENE_BUILDING2;
    if (fgSceneEquals(sceneName, "building6"))
        return FG_SCENE_BUILDING6;
    if (fgSceneEquals(sceneName, "mary3"))
        return FG_SCENE_MARY3;
    if (fgSceneEquals(sceneName, "visitor3"))
        return FG_SCENE_VISITOR3;
    if (fgSceneEquals(sceneName, "walkstuf1"))
        return FG_SCENE_WALKSTUF1;
    return FG_SCENE_UNKNOWN;
}
