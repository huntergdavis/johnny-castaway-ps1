/*
 *  This file is part of 'Johnny Reborn' - PS1 Port
 *
 *  Manifest for the PS1 freestanding libc compatibility layer.
 *
 *  Ownership is split by behavior:
 *  - ps1_stdio_stubs.c   maps FILE* calls onto CD-ROM handles and TTY output.
 *  - ps1_runtime_stubs.c provides process, environment, filesystem, and SDL
 *                         compatibility shims.
 */

typedef int ps1_stubs_manifest_translation_unit;
