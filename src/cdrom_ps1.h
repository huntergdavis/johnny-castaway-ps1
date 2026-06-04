/*
 *  This file is part of 'Johnny Reborn' - PS1 Port
 *
 *  CD-ROM file access layer for PlayStation 1 using PSn00bSDK
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 */

#ifndef CDROM_PS1_H
#define CDROM_PS1_H

#include "mytypes.h"

/*
 * Build 24: Test function at very start of cdrom_ps1.c
 */
int cdromFirstFunction(void);

/*
 * Initialize CD-ROM subsystem
 * Returns 0 on success, -1 on error
 */
int cdromInit(void);

/*
 * Reset CD state after external CD operations (like title screen loading)
 * Call this before using ps1_fopen after direct CD reads
 */
void cdromResetState(void);

/*
 * Build 19: Test function to see if we can call functions in cdrom_ps1.c
 */
int cdromTestCall(void);

/*
 * Build 20: Test function placed BEFORE static data
 */
int cdromTestCall2(void);

/*
 * Open a file from CD-ROM
 * Returns file handle ID (0-7) or -1 on error
 */
int cdromOpen2(const char *filename);

/*
 * Read data from CD file
 * Returns number of bytes read, or -1 on error
 */
int cdromRead(int fileHandle, void *buffer, uint32 size);

/*
 * Seek to position in CD file
 * whence: SEEK_SET=0, SEEK_CUR=1, SEEK_END=2
 * Returns new position, or -1 on error
 */
int cdromSeek(int fileHandle, int offset, int whence);

/*
 * Get current position in CD file
 * Returns position in bytes, or -1 on error
 */
int cdromTell(int fileHandle);

/*
 * Close CD file
 * Returns 0 on success, -1 on error
 */
int cdromClose(int fileHandle);

/*
 * Get file size
 * Returns size in bytes, or 0 on error
 */
uint32 cdromGetSize(int fileHandle);

/*
 * PS1 File I/O wrapper functions for resource loading
 * These provide FILE*-like interface using CD-ROM access
 */

#include <psxcd.h>

CdlFILE *ps1_cdSearchFileQuiesced(CdlFILE *file, const char *filename);

typedef void (*Ps1CdReadIdleHook)(void *userData);
void ps1_cdSetReadIdleHook(Ps1CdReadIdleHook hook, void *userData);

typedef struct {
    CdlFILE cdfile;
    long currentPos;
    int isOpen;
    char filename[32];
    uint8_t* buffer;      /* Whole-file buffer (legacy) OR small sector cache (stream mode) */
    uint32_t bufferSize;  /* Whole-file: file size; Stream: cache capacity */
    /* Stream mode: when nonzero, buffer is a sector cache (not the
     * whole file). Used by ps1_fopen_stream to read big files like
     * RESOURCE.001 without claiming all of libc. */
    int      isStream;
    uint32_t streamBase;     /* file offset of buffer[0] in stream mode */
    uint32_t streamBytesCached; /* valid bytes in buffer (stream mode) */
    int      bufferFromRegion; /* 1 = file->buffer is MEM_REGION_CACHE; 0 = libc malloc */
} PS1File;

PS1File* ps1_fopen(const char* filename, const char* mode);

/* Streaming open: opens a file using a small fixed-size sector cache
 * instead of loading the whole file into libc. ps1_fread/ps1_fseek
 * refill the cache on demand. cacheBytes is rounded up to a multiple
 * of 2048 (CD sector size); minimum effective cache is one sector.
 *
 * Designed for RESOURCE.001 (~1.1 MB catalog) which is too big to
 * fit in libc heap once the static region buffer is reserved. The
 * parser performs random-access reads through the file but never
 * actively re-reads (each entry is touched once); first-fit cache
 * is sufficient.
 *
 * Returns NULL on file-not-found or alloc failure. */
PS1File* ps1_fopen_stream(const char* filename, uint32_t cacheBytes);

size_t ps1_fread(void* ptr, size_t size, size_t nmemb, PS1File* file);
int ps1_fseek(PS1File* file, long offset, int whence);
long ps1_ftell(PS1File* file);
int ps1_fclose(PS1File* file);

/* Stream read: Read a range from file without loading entire file into RAM.
 * For dynamic loading - only reads necessary CD sectors.
 * Returns malloc'd buffer (caller must free), or NULL on error. */
uint8_t* ps1_streamRead(const char* filename, uint32_t offset, uint32_t size);

/* Same as ps1_streamRead but returns a buffer allocated from
 * MEM_REGION_CACHE. Use for resource data that becomes
 * uncompressedData held by the LRU cache. Caller frees via
 * memFree(MEM_REGION_CACHE, ptr) — the LRU evictor in resource.c
 * does this automatically as resources get evicted. */
uint8_t* ps1_streamReadCache(const char* filename, uint32_t offset, uint32_t size);

int ps1_streamResolveFile(const char* filename, CdlFILE* outFile);
int ps1_streamReadIntoFile(const CdlFILE *cdfile, uint32_t offset, uint32_t size, uint8_t *dstBuffer);
int ps1_streamReadIntoFileBuffered(const CdlFILE *cdfile, uint32_t offset, uint32_t size,
                                   uint8_t *dstBuffer, uint8_t *sectorBuffer,
                                   uint32_t sectorBufferSize);
int ps1_streamReadAlignedIntoFile(const CdlFILE *cdfile, uint32_t offset, uint32_t size,
                                  uint8_t *dstBuffer);

/* Load an entire file from CD into a malloc'd buffer.
 * Path should be a PS1 CD path like "\\SND\\SOUND00.VAG;1".
 * Returns malloc'd buffer (caller must free), or NULL on error. */
uint8_t* ps1_loadRawFile(const char* path, uint32_t* outSize);

/* Wrap a buffer as a PS1File for use with decompress functions */
void ps1_wrapBuffer(PS1File* file, uint8_t* buffer, uint32_t size);

/* PS1-specific utility functions for resource parsing */
uint8 ps1_readUint8(PS1File *f);
uint16 ps1_readUint16(PS1File *f);
uint32 ps1_readUint32(PS1File *f);
char *ps1_getString(PS1File *f, int maxlen);
uint8 *ps1_readUint8Block(PS1File *f, int len);
uint16 *ps1_readUint16Block(PS1File *f, int count);

/* PS1-specific resource parsing functions */
struct TAdsResource* ps1_parseAdsResource(PS1File *f, const char *resName);
struct TBmpResource* ps1_parseBmpResource(PS1File *f, const char *resName);
struct TPalResource* ps1_parsePalResource(PS1File *f, const char *resName);
struct TScrResource* ps1_parseScrResource(PS1File *f, const char *resName);
struct TTtmResource* ps1_parseTtmResource(PS1File *f, const char *resName);

/* PS1-specific decompression functions */
uint8 *ps1_uncompress(PS1File *f, uint8 compressionMethod, uint32 inSize, uint32 outSize);
uint8 *ps1_uncompressRLE(PS1File *f, uint32 inSize, uint32 outSize);
uint8 *ps1_uncompressLZW(PS1File *f, uint32 inSize, uint32 outSize);

/* On-demand resource loading from pre-extracted files */
void ps1_loadBmpData(struct TBmpResource *bmpResource);
void ps1_loadScrData(struct TScrResource *scrResource);
void ps1_loadTtmData(struct TTtmResource *ttmResource);
void ps1_loadAdsData(struct TAdsResource *adsResource);
void ps1_pilotPrearmPackForAds(const char *adsName);

/* Try to load a PSB resource from the active scene pack.
 * Returns malloc'd buffer on success, NULL if not found.
 * On success, *outSize is set to the resource size. */
uint8_t *ps1PilotLoadPsb(const char *psbName, uint32_t *outSize);

#endif /* CDROM_PS1_H */
