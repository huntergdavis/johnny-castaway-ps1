#ifndef PS1_BUILD
/* Load resource data from extracted file if available, otherwise decompress */
static uint8 *loadOrUncompress(FILE *compressedFile,
                                const char *resourceName,
                                const char *resourceType,
                                uint8 compressionMethod,
                                uint32 compressedSize,
                                uint32 uncompressedSize)
{
    char extractedPath[512];
    FILE *extractedFile;
    uint8 *data;

    /* Try to load from extracted file first */
    snprintf(extractedPath, sizeof(extractedPath), "extracted/%s/%s",
             resourceType, resourceName);

    extractedFile = fopen(extractedPath, "rb");
    if (extractedFile != NULL) {
        /* Load directly from disk - no decompression needed */
        data = safe_malloc(uncompressedSize);
        if (fread(data, 1, uncompressedSize, extractedFile) != uncompressedSize) {
            if (debugMode) {
                printf("Warning: Failed to read %s, falling back to decompression\n",
                       extractedPath);
            }
            free(data);
            fclose(extractedFile);
            /* Fall through to decompression */
        } else {
            fclose(extractedFile);
            /* Skip past compressed data in resource file */
            fseek(compressedFile, compressedSize, SEEK_CUR);
            if (debugMode) {
                printf("Loaded %s from extracted file (saved ~16KB working memory)\n",
                       resourceName);
            }
            return data;
        }
    }

    /* Fall back to decompression */
    return uncompress(compressedFile, compressionMethod,
                      compressedSize, uncompressedSize);
}


static struct TAdsResource *parseAdsResource(FILE *f)
{
    struct TAdsResource *adsResource;
    uint8 *buffer;


    adsResource = safe_malloc(sizeof(struct TAdsResource));

    buffer = readUint8Block(f,4);
    if (memcmp(buffer,"VER:",4))
        fatalError("'VER:' string not found while parsing ADS resource");

    free(buffer);

    adsResource->versionSize = readUint32(f);
    adsResource->versionString = readUint8Block(f,5);

    buffer = readUint8Block(f,4);
    if (memcmp(buffer,"ADS:",4))
        fatalError("'ADS:' string not found while parsing ADS resource");

    free(buffer);

    adsResource->adsUnknown1 = readUint8(f);
    adsResource->adsUnknown2 = readUint8(f);
    adsResource->adsUnknown3 = readUint8(f);
    adsResource->adsUnknown4 = readUint8(f);

    buffer = readUint8Block(f,4);
    if (memcmp(buffer,"RES:",4))
        fatalError("'RES:' string not found while parsing ADS resource");

    free(buffer);

    adsResource->resSize = readUint32(f);
    adsResource->numRes = readUint16(f);

    adsResource->res = safe_malloc(adsResource->numRes * sizeof(struct TAdsRes));

    for (int i=0; i < adsResource->numRes; i++) {
        adsResource->res[i].id = readUint16(f);
        adsResource->res[i].name = getString(f,40);
    }

    buffer = readUint8Block(f,4);
    if (memcmp(buffer,"SCR:",4))
        fatalError("'SCR:' string not found while parsing ADS resource");

    free(buffer);

    adsResource->compressedSize = readUint32(f) - 5;
    adsResource->compressionMethod = readUint8(f);
    adsResource->uncompressedSize = readUint32(f);

    /* ADS lazy loading: Don't decompress at startup, just skip the compressed data */
    /* This saves ~15KB of memory at startup (only decompress when ADS is played) */
    adsResource->uncompressedData = NULL;  /* Will be loaded on demand in ads.c */
    fseek(f, adsResource->compressedSize, SEEK_CUR);  /* Skip compressed data for now */

    buffer = readUint8Block(f,4);
    if (memcmp(buffer,"TAG:",4))
        fatalError("'TAG:' string not found while parsing ADS resource");

    free(buffer);

    adsResource->tagSize = readUint32(f);
    adsResource->numTags = readUint16(f);

    adsResource->tags = safe_malloc(adsResource->numTags * sizeof(struct TTags));

    for (int i=0; i < adsResource->numTags; i++) {
        adsResource->tags[i].id = readUint16(f);
        adsResource->tags[i].description = getString(f,40);
    }

    return adsResource;
}


static struct TBmpResource *parseBmpResource(FILE *f)
{
    struct TBmpResource *bmpResource;
    uint8 *buffer;


    bmpResource = safe_malloc(sizeof(struct TBmpResource));

    buffer = readUint8Block(f,4);
    if (memcmp(buffer,"BMP:",4))
        fatalError("'BMP:' string not found while parsing BMP resource");

    free(buffer);

    bmpResource->width = readUint16(f);
    bmpResource->height = readUint16(f);

    buffer = readUint8Block(f,4);
    if (memcmp(buffer,"INF:",4))
        fatalError("'INF:' string not found while parsing BMP resource");

    free(buffer);

    bmpResource->dataSize = readUint32(f);
    bmpResource->numImages = readUint16(f);

    bmpResource->widths = readUint16Block(f, bmpResource->numImages);
    bmpResource->heights = readUint16Block(f, bmpResource->numImages);

    buffer = readUint8Block(f,4);
    if (memcmp(buffer,"BIN:",4))
        fatalError("'BIN:' string not found while parsing BMP resource");

    free(buffer);

    bmpResource->compressedSize = readUint32(f) - 5; // discard size of compressionmethod+uncompressedsize
    bmpResource->compressionMethod = readUint8(f);
    bmpResource->uncompressedSize = readUint32(f);

    bmpResource->uncompressedData = loadOrUncompress(f,
                                      bmpResource->resName,
                                      "bmp",
                                      bmpResource->compressionMethod,
                                      bmpResource->compressedSize,
                                      bmpResource->uncompressedSize
                                    );

    return bmpResource;
}


static struct TPalResource *parsePalResource(FILE *f)
{
    struct TPalResource *palResource;
    uint8 *buffer;


    palResource = safe_malloc(sizeof(struct TPalResource));

    buffer = readUint8Block(f,4);
    if (memcmp(buffer,"PAL:",4))
        fatalError("'PAL:' string not found while parsing PAL resource");

    free(buffer);

    palResource->size = readUint16(f);
    palResource->unknown1 = readUint8(f);
    palResource->unknown2 = readUint8(f);

    buffer = readUint8Block(f,4);
    if (memcmp(buffer,"VGA:",4))
        fatalError("'VGA:' string not found while parsing PAL resource");

    free(buffer);

    readUint8(f);   // size ?
    readUint8(f);
    readUint8(f);
    readUint8(f);

    for (int i=0; i < 256; i++) {
        palResource->colors[i].r = readUint8(f);
        palResource->colors[i].g = readUint8(f);
        palResource->colors[i].b = readUint8(f);
    }

    return palResource;
}


static struct TScrResource *parseScrResource(FILE *f)
{
    struct TScrResource *scrResource;
    uint8 *buffer;


    scrResource = safe_malloc(sizeof(struct TScrResource));

    buffer = readUint8Block(f,4);
    if (memcmp(buffer,"SCR:",4))
        fatalError("'SCR:' string not found while parsing SCR resource");

    free(buffer);

    scrResource->totalSize = readUint16(f);
    scrResource->flags = readUint16(f);

    buffer = readUint8Block(f,4);
    if (memcmp(buffer,"DIM:",4))
        fatalError("'DIM:' string not found while parsing SCR resource");

    free(buffer);

    scrResource->dimSize = readUint32(f);
    scrResource->width = readUint16(f);
    scrResource->height = readUint16(f);

    buffer = readUint8Block(f,4);
    if (memcmp(buffer,"BIN:",4))
        fatalError("'BIN:' string not found while parsing SCR resource");

    free(buffer);

    scrResource->compressedSize = readUint32(f) - 5; // discard size of compressionmethod+uncompressedsize
    scrResource->compressionMethod = readUint8(f);
    scrResource->uncompressedSize = readUint32(f) ;

    scrResource->uncompressedData = loadOrUncompress(f,
                                      scrResource->resName,
                                      "scr",
                                      scrResource->compressionMethod,
                                      scrResource->compressedSize,
                                      scrResource->uncompressedSize
                                    );

    return scrResource;
}


static struct TTtmResource *parseTtmResource(FILE *f)
{
    struct TTtmResource *ttmResource;
    uint8 *buffer;

    ttmResource = safe_malloc(sizeof(struct TTtmResource));

    buffer = readUint8Block(f,4);
    if (memcmp(buffer,"VER:",4))
        fatalError("'VER:' string not found while parsing TTM resource");

    free(buffer);

    ttmResource->versionSize = readUint32(f);
    ttmResource->versionString = readUint8Block(f,5);

    buffer = readUint8Block(f,4);
    if (memcmp(buffer,"PAG:",4))
        fatalError("'PAG:' string not found while parsing TTM resource");

    free(buffer);

    ttmResource->numPages = readUint32(f);
    ttmResource->pagUnknown1 = readUint8(f);
    ttmResource->pagUnknown2 = readUint8(f);

    buffer = readUint8Block(f,4);
    if (memcmp(buffer,"TT3:",4))
        fatalError("'TT3:' string not found while parsing TTM resource");

    free(buffer);

    ttmResource->compressedSize = readUint32(f) - 5; // discard size of compressionmethod+uncompressedsize
    ttmResource->compressionMethod = readUint8(f);
    ttmResource->uncompressedSize = readUint32(f);

    /* TTM lazy loading: Don't decompress at startup, just skip the compressed data */
    /* This saves ~284KB of memory at startup (only decompress when TTM is played) */
    ttmResource->uncompressedData = NULL;  /* Will be loaded on demand in ttmLoadTtm() */
    fseek(f, ttmResource->compressedSize, SEEK_CUR);  /* Skip compressed data for now */

    buffer = readUint8Block(f,4);
    if (memcmp(buffer,"TTI:",4))
        fatalError("'TTI:' string not found while parsing TTM resource");

    free(buffer);

    ttmResource->ttiUnknown1 = readUint8(f);
    ttmResource->ttiUnknown2 = readUint8(f);
    ttmResource->ttiUnknown3 = readUint8(f);
    ttmResource->ttiUnknown4 = readUint8(f);

    buffer = readUint8Block(f,4);
    if (memcmp(buffer,"TAG:",4))
        fatalError("'TAG:' string not found while parsing TTM resource");

    free(buffer);

    ttmResource->tagSize = readUint32(f);
    ttmResource->numTags = readUint16(f);

    ttmResource->tags = safe_malloc(ttmResource->numTags * sizeof(struct TTags));

    for (int i=0; i < ttmResource->numTags; i++) {
        ttmResource->tags[i].id = readUint16(f);
        ttmResource->tags[i].description = getString(f,40);
    }

    return ttmResource;
}
#endif /* !PS1_BUILD */
