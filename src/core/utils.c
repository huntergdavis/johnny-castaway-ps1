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

/* PS1 doesn't have full standard library */
#include <stdarg.h>  /* For va_list - must be before PS1 check */
#ifndef PS1_BUILD
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#else
/* Minimal declarations for PS1 freestanding environment */
#include <stddef.h>  /* For size_t */
#ifndef _FILE_DEFINED
#define _FILE_DEFINED
typedef struct _FILE FILE;
#endif
extern int printf(const char *format, ...);
extern int vprintf(const char *format, __gnuc_va_list arg);
extern int fprintf(FILE *stream, const char *format, ...);
extern int vfprintf(FILE *stream, const char *format, __gnuc_va_list arg);
extern FILE *fopen(const char *pathname, const char *mode);
extern int fgetc(FILE *stream);
extern void *malloc(size_t size);
extern void exit(int status) __attribute__((noreturn));
#define stderr ((FILE*)2)
#endif
#include <string.h>

#include "mytypes.h"
#include "holidays.h"


#define BUF_LEN 256

int debugMode = 0;

/* User-overridable date/time. ps1SoftTimeEnabled gates whether scene
 * runtime uses these. The pause menu's Set Time/Date confirm path sets
 * the flag; fgLoopApplyVariant in jc_reborn.c reads the flag and
 * overrides islandState.night and .holiday accordingly. */
int ps1SoftTimeEnabled = 0;
int ps1SoftHour  = 12;
int ps1SoftMinute = 0;
int ps1SoftMonth = 6;
int ps1SoftDay   = 30;
int ps1SoftYear  = 2026;  /* For movable-feast computation */

/* Map (month, day) → islandState.holiday code via the new
 * algorithm-driven holidays module. The legacy 4 IDs are preserved
 * (1=Halloween, 2=StPatrick, 3=Christmas, 4=NewYear) so existing
 * memcard saves with holidayOverride=1..4 keep their meaning.
 *
 * Year arg goes through ps1SoftYear (or a default 2026) since the
 * original PC API was (month, day) only. Movable feasts need a year
 * for accurate computation. */
extern int ps1SoftYear;
int ps1HolidayFromDate(int month, int day)
{
    int year = ps1SoftYear ? ps1SoftYear : 2026;
    return holidayForDate(year, month, day);
}

int ps1HolidayFromDateOriginal4(int month, int day)
{
    int year = ps1SoftYear ? ps1SoftYear : 2026;
    return holidayForDateOriginal4(year, month, day);
}

void fatalError(char *message, ... )
{
    va_list(args);

    va_start(args, message);
#ifdef PS1_BUILD
    /* TTY log first — keeps the soak-test grep ('JCBSOD-FATAL'-style
     * pattern) reliable even if the on-screen panel can't render. */
    printf("\n\n Fatal error : ");
    vprintf(message, args);
    printf("\n\n");
    va_end(args);

    /* Plan v9 step 10 / A10: render a minimal on-screen text panel
     * via ps1DebugError so users see a readable error message
     * instead of a frozen black screen. Requires ps1DebugInit to
     * have run first (boot sequence guarantees this). The panel
     * holds the screen until SELECT is pressed. */
    {
        extern void ps1DebugError(const char *fmt, ...);
        va_list args2;
        va_start(args2, message);
        /* Re-vararg through the on-screen path. ps1DebugError accepts
         * printf-style formatting; the same `message` reaches it. */
        ps1DebugError("FATAL: %s", message);
        /* Note: %s consumes message but discards remaining va args
         * — acceptable for the halt path; users can read the TTY
         * log for the formatted detail. The panel just shows
         * "FATAL: <unformatted message>". */
        va_end(args2);
    }
    while(1);  /* Halt — ps1DebugError returns; we trap here. */
#else
    fprintf(stderr, "\n\n Fatal error : ");
    vfprintf(stderr, message, args);
    fprintf(stderr, "\n\n");
    va_end(args);
    exit(1);
#endif
}


void debugMsg(char *message, ... )
{
    if (debugMode) {
        va_list(args);
        va_start(args, message);
        vprintf(message, args);
        printf("\n");
        va_end(args);
    }
}


void *safe_malloc(size_t size)
{
    /* Kept on libc malloc for now. Earlier attempt to route to
     * MEM_REGION_BOOT broke call paths that call libc free() on the
     * returned pointer (notably the LRU evictor in resource.c which
     * frees uncompressedData blobs).
     *
     * Per-call-site migration to memAlloc(REGION, n, tag) is the
     * correct path — each site needs to verify the lifetime semantics
     * and update both alloc and free. The safe_malloc wrapper itself
     * is a legacy shim that fans out to many lifetimes; routing it
     * monolithically to BOOT was a category error. */
    void *ptr = malloc(size);
    if (ptr == NULL)
        fatalError("failed to malloc() %d bytes", size);
    return ptr;
}


FILE *safe_fopen(const char *pathname, const char *mode)
{
    FILE *f;

    f = fopen(pathname, mode);

    if (f == NULL)
        fatalError("unable to open file %s in mode '%s'", pathname, mode);

    return f;
}


uint8 readUint8(FILE *f)
{
    return fgetc(f);
}


uint16 readUint16(FILE *f)
{
    uint16 a;

    a  = fgetc(f);
    a += fgetc(f) << 8;

    return a;
}


uint32 readUint32(FILE *f)
{
    uint32 a;

    a  = fgetc(f);
    a += fgetc(f) << 8;
    a += fgetc(f) << 16;
    a += fgetc(f) << 24;

    return a;
}


char *getString(FILE *f, int maxlen)
{
    int numread = 0;
    int lastread = -1;
    char buf[BUF_LEN];
    char *out;

    while ((numread < maxlen) && (numread < BUF_LEN) && (lastread != 0)) {

        lastread = buf[numread++] = fgetc(f);
    }

    out = safe_malloc(numread * sizeof(char));
    memcpy(out, buf, numread);
    return out;
}


uint8 *readUint8Block(FILE *f, int len)
{
    uint8 *out = safe_malloc(len * sizeof(char));

    for (int i=0; i < len; i++)
        out[i] = fgetc(f);

    return out;
}


uint16 *readUint16Block(FILE *f, int len)
{
    uint16 *out = safe_malloc(len * sizeof(uint16));

    for (int i=0; i < len; i++)
        out[i] = readUint16(f);

    return out;
}


uint16 peekUint16(uint8 *data, uint32 *offset)
{
    uint16 result;

    result  = data[(*offset)++];
    result |= data[(*offset)++] << 8;

    return result;
}


void peekUint16Block(uint8 *data, uint32 *offset, uint16 *dest, int len)
{
    for (int i=0; i < len ; i++)
        dest[i] = peekUint16(data, offset);
}


void hexdump(uint8 *data, uint32 len)
{
    if (data==NULL)
        printf("Can't dump NULL data\n");

    printf("\n");

    for (uint32 i=0; i < len; i++) {

        printf("%02x ",data[i]);

        if ((i & 0x0f) == 0x07) { printf (" ");  }
        if ((i & 0x0f) == 0x0f) { printf ("\n"); }
    }

    printf("\n");
}


#ifdef PS1_BUILD
static int utilsIsLeapYear(int year)
{
    return ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0);
}

static int utilsDayOfYearFromDate(int year, int month, int day)
{
    static const int beforeMonth[13] =
        { 0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
    int yday;

    if (month < 1 || month > 12) return 180;
    if (day < 1 || day > 31) return 180;

    yday = beforeMonth[month] + day - 1;
    if (month > 2 && utilsIsLeapYear(year))
        yday += 1;
    return yday;
}
#endif

int getDayOfYear()
{
#ifdef PS1_BUILD
    if (ps1SoftTimeEnabled)
        return utilsDayOfYearFromDate(ps1SoftYear, ps1SoftMonth, ps1SoftDay);
    return 180;  /* Mid-year default when no software clock is set. */
#else
    struct timeval tv;
    struct tm *localTime;

    gettimeofday(&tv, NULL);
    localTime = localtime(&tv.tv_sec);
    return localTime->tm_yday;
#endif
}


int getHour()
{
#ifdef PS1_BUILD
    if (ps1SoftTimeEnabled)
        return ps1SoftHour;
    return 12;
#else
    struct timeval tv;
    struct tm *localTime;

    gettimeofday(&tv, NULL);
    localTime = localtime(&tv.tv_sec);
    return localTime->tm_hour;
#endif
}


char *getMonthAndDay()
{
#ifdef PS1_BUILD
    static char result[5] = "0630";  /* June 30 */
    if (ps1SoftTimeEnabled) {
        result[0] = (char)('0' + (ps1SoftMonth / 10));
        result[1] = (char)('0' + (ps1SoftMonth % 10));
        result[2] = (char)('0' + (ps1SoftDay / 10));
        result[3] = (char)('0' + (ps1SoftDay % 10));
        result[4] = '\0';
    }
    return result;
#else
    struct timeval tv;
    struct tm *localTime;
    static char result[5];

    gettimeofday(&tv, NULL);
    localTime = localtime(&tv.tv_sec);
    strftime(result, 5, "%m%d", localTime);
    return result;
#endif
}
