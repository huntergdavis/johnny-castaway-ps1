/*
 *  This file is part of 'Johnny Reborn' - PS1 Port
 *
 *  FILE*, printf-family, and CD-ROM backed stdio shims for the PS1
 *  freestanding build.
 */

#include <stddef.h>
#include <stdarg.h>

#include "mytypes.h"
#include "cdrom_ps1.h"

extern int printf(const char *format, ...);
extern int vsnprintf(char *str, size_t size, const char *format, __gnuc_va_list arg);

typedef struct _FILE FILE;
#define stderr ((FILE*)2)
#define stdout ((FILE*)1)

int vprintf(const char *format, __gnuc_va_list arg)
{
    char buffer[512];
    int result = vsnprintf(buffer, sizeof(buffer), format, arg);
    if (result >= 0) {
        buffer[sizeof(buffer) - 1] = '\0';
        printf("%s", buffer);
    }
    return result;
}

FILE *fopen(const char *pathname, const char *mode)
{
    int handle = cdromOpen2(pathname);
    if (handle < 0) {
        return NULL;
    }
    return (FILE*)(size_t)(handle + 3);
}

int fclose(FILE *stream)
{
    if (stream == stdout || stream == stderr || stream == NULL) {
        return 0;
    }
    return cdromClose((int)(size_t)stream - 3);
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    int bytesRead;

    if (stream == stdout || stream == stderr || stream == NULL) {
        return 0;
    }

    bytesRead = cdromRead((int)(size_t)stream - 3, ptr, size * nmemb);
    if (bytesRead < 0) {
        return 0;
    }
    return bytesRead / size;
}

int fseek(FILE *stream, long offset, int whence)
{
    int result;

    if (stream == stdout || stream == stderr || stream == NULL) {
        return -1;
    }

    result = cdromSeek((int)(size_t)stream - 3, offset, whence);
    return (result < 0) ? -1 : 0;
}

long ftell(FILE *stream)
{
    if (stream == stdout || stream == stderr || stream == NULL) {
        return 0;
    }
    return cdromTell((int)(size_t)stream - 3);
}

int fgetc(FILE *f)
{
    unsigned char byte;
    int result;

    if (f == stdout || f == stderr || f == NULL) {
        return -1;
    }

    result = cdromRead((int)(size_t)f - 3, &byte, 1);
    if (result != 1) {
        return -1;
    }
    return (int)byte;
}

int fflush(FILE *stream)
{
    return 0;
}

int fputs(const char *s, FILE *stream)
{
    if (stream == stderr || stream == stdout) {
        printf("%s", s);
        return 0;
    }
    return -1;
}

int fprintf(FILE *stream, const char *format, ...)
{
    if (stream == stderr || stream == stdout) {
        va_list args;
        int result;

        va_start(args, format);
        result = vprintf(format, args);
        va_end(args);
        return result;
    }
    return 0;
}

int vfprintf(FILE *stream, const char *format, __gnuc_va_list args)
{
    if (stream == stderr || stream == stdout) {
        return vprintf(format, args);
    }
    return 0;
}

char *fgets(char *s, int size, FILE *stream)
{
    return NULL;
}

int feof(FILE *stream)
{
    uint32 pos;
    uint32 size;

    if (stream == stdout || stream == stderr || stream == NULL) {
        return 1;
    }

    pos = cdromTell((int)(size_t)stream - 3);
    size = cdromGetSize((int)(size_t)stream - 3);
    return (pos >= size) ? 1 : 0;
}
