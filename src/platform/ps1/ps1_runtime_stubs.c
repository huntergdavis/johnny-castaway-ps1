/*
 *  This file is part of 'Johnny Reborn' - PS1 Port
 *
 *  Runtime and host-API compatibility shims for the PS1 freestanding build.
 */

#include <stddef.h>

extern int printf(const char *format, ...);

void exit(int status)
{
    printf("EXIT called with status %d - halting\n", status);
    while (1) {
    }
}

int atoi(const char *str)
{
    int result = 0;
    int sign = 1;

    if (str == NULL)
        return 0;

    while (*str == ' ' || *str == '\t' || *str == '\n')
        str++;

    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }

    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }

    return result * sign;
}

char *getenv(const char *name)
{
    return NULL;
}

struct stat {
    int st_mode;
};

int stat(const char *pathname, struct stat *statbuf)
{
    return -1;
}

int mkdir(const char *pathname, int mode)
{
    return -1;
}

unsigned int SDL_GetTicks(void)
{
    static unsigned int ticks = 0;
    ticks += 16;
    return ticks;
}
