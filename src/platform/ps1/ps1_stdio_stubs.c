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
extern int putchar(int ch);

typedef struct _FILE FILE;
#define stderr ((FILE*)2)
#define stdout ((FILE*)1)

static int ps1StdoutWrite(const char *text, size_t len)
{
    size_t i;

    if (text == NULL || len == 0)
        return 0;

    for (i = 0; i < len; i++)
        putchar((unsigned char)text[i]);
    return (int)len;
}

static int ps1StdoutChar(char ch)
{
    putchar((unsigned char)ch);
    return 1;
}

static int ps1StdoutStringN(const char *text, int maxChars)
{
    const char *start;
    size_t len = 0;

    if (text == NULL)
        text = "(null)";

    start = text;
    while (*text != '\0' && (maxChars < 0 || (int)len < maxChars)) {
        text++;
        len++;
    }
    return ps1StdoutWrite(start, len);
}

static int ps1StdoutPad(int count, char ch)
{
    int written = 0;

    while (count-- > 0)
        written += ps1StdoutChar(ch);
    return written;
}

static int ps1DigitsUnsigned(unsigned long value, unsigned int base,
                             int uppercase, char *digits)
{
    const char *alphabet = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
    char tmp[32];
    int count = 0;
    int out = 0;

    if (base < 2)
        base = 10;
    do {
        tmp[count++] = alphabet[value % base];
        value /= base;
    } while (value != 0 && count < (int)sizeof(tmp));

    while (count > 0)
        digits[out++] = tmp[--count];
    digits[out] = '\0';
    return out;
}

static int ps1StdoutUnsigned(unsigned long value, unsigned int base,
                             int uppercase, char signChar,
                             int width, int leftJustify, int zeroPad)
{
    char digits[32];
    int digitCount = ps1DigitsUnsigned(value, base, uppercase, digits);
    int total = digitCount + (signChar ? 1 : 0);
    int pad = (width > total) ? (width - total) : 0;
    int written = 0;
    int i;

    if (!leftJustify && !zeroPad)
        written += ps1StdoutPad(pad, ' ');
    if (signChar)
        written += ps1StdoutChar(signChar);
    if (!leftJustify && zeroPad)
        written += ps1StdoutPad(pad, '0');
    for (i = 0; i < digitCount; i++)
        written += ps1StdoutChar(digits[i]);
    if (leftJustify)
        written += ps1StdoutPad(pad, ' ');
    return written;
}

int vprintf(const char *format, __gnuc_va_list arg)
{
    int written = 0;

    if (format == NULL)
        return 0;

    while (*format != '\0') {
        const char *runStart;
        int leftJustify = 0;
        int plusSign = 0;
        int zeroPad = 0;
        int width = 0;
        int precision = -1;
        int longFlag = 0;
        char spec;

        runStart = format;
        while (*format != '\0' && *format != '%')
            format++;
        if (format != runStart)
            written += ps1StdoutWrite(runStart, (size_t)(format - runStart));
        if (*format == '\0')
            break;

        format++;
        if (*format == '%') {
            written += ps1StdoutChar('%');
            format++;
            continue;
        }

        for (;;) {
            if (*format == '-') {
                leftJustify = 1;
                format++;
            } else if (*format == '+') {
                plusSign = 1;
                format++;
            } else if (*format == '0') {
                zeroPad = 1;
                format++;
            } else {
                break;
            }
        }

        while (*format >= '0' && *format <= '9') {
            width = (width * 10) + (*format - '0');
            format++;
        }

        if (*format == '.') {
            format++;
            precision = 0;
            while (*format >= '0' && *format <= '9') {
                precision = (precision * 10) + (*format - '0');
                format++;
            }
        }

        if (*format == 'l') {
            longFlag = 1;
            format++;
        } else if (*format == 'h') {
            format++;
        }

        spec = *format;
        if (spec != '\0')
            format++;

        if (spec == 's') {
            const char *s = va_arg(arg, const char *);
            int len = 0;
            const char *scan = s ? s : "(null)";
            int pad;

            while (scan[len] != '\0' &&
                   (precision < 0 || len < precision)) {
                len++;
            }
            pad = (width > len) ? (width - len) : 0;
            if (!leftJustify)
                written += ps1StdoutPad(pad, ' ');
            written += ps1StdoutStringN(s, precision);
            if (leftJustify)
                written += ps1StdoutPad(pad, ' ');
        } else if (spec == 'd' || spec == 'i') {
            long value = longFlag ? va_arg(arg, long) : (long)va_arg(arg, int);
            unsigned long mag;
            char signChar = 0;

            if (value < 0) {
                signChar = '-';
                mag = 0UL - (unsigned long)value;
            } else {
                mag = (unsigned long)value;
                if (plusSign)
                    signChar = '+';
            }
            written += ps1StdoutUnsigned(mag, 10, 0, signChar,
                                         width, leftJustify, zeroPad);
        } else if (spec == 'u') {
            unsigned long value = longFlag
                ? va_arg(arg, unsigned long)
                : (unsigned long)va_arg(arg, unsigned int);
            written += ps1StdoutUnsigned(value, 10, 0, 0,
                                         width, leftJustify, zeroPad);
        } else if (spec == 'x' || spec == 'X') {
            unsigned long value = longFlag
                ? va_arg(arg, unsigned long)
                : (unsigned long)va_arg(arg, unsigned int);
            written += ps1StdoutUnsigned(value, 16, spec == 'X', 0,
                                         width, leftJustify, zeroPad);
        } else if (spec == 'p') {
            unsigned long value = (unsigned long)va_arg(arg, void *);
            written += ps1StdoutStringN("0x", -1);
            written += ps1StdoutUnsigned(value, 16, 0, 0, 8, 0, 1);
        } else if (spec == 'c') {
            written += ps1StdoutChar((char)va_arg(arg, int));
        } else if (spec != '\0') {
            written += ps1StdoutChar('%');
            written += ps1StdoutChar(spec);
        }
    }

    return written;
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
