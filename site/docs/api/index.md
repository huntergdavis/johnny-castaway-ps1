---
layout: page
title: API mapping (SDL2 → PSn00bSDK)
eyebrow: Reference
subtitle: Every SDL2 symbol the upstream engine called, mapped to the PSn00bSDK call that replaced it.
description: SDL2 to PSn00bSDK mapping reference for the Johnny Castaway PS1 port — graphics, input, audio, and file I/O surfaces with side-by-side code examples.
---

A labor of love by Hunter Davis. The original Sierra screensaver was a
1992 Windows 3.1 binary; the upstream engine code in this repo descends
from a host-side reimplementation that ran on SDL2. The PS1 build cannot
have SDL2 — there's no SDL on a PS1 — and the host capture pipeline still
needs SDL2 to render frames the PS1 plays back.

This page is the bridge. Every SDL2 symbol the engine calls maps to a
PSn00bSDK equivalent. The host build keeps the SDL2 path; the PS1 build
substitutes the PSn00bSDK path via `#ifdef PS1_BUILD` walls in
`graphics_ps1.c`, `sound_ps1.c`, `events_ps1.c`, and `cdrom_ps1.c`. There
is no SDL compatibility shim for the PS1 — the substitution is done at
the call site.

If you paid for this, you were cheated. Open source and free.

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

## Graphics

| SDL2                              | PSn00bSDK                       | Status   | Notes |
|-----------------------------------|---------------------------------|----------|-------|
| `SDL_Init(SDL_INIT_VIDEO)`        | `ResetGraph(0)`, `InitGeom()`   | done     | Reset GPU and GTE. |
| `SDL_CreateWindow()`              | `SetDefDispEnv()`               | done     | 640x480 interlaced. |
| `SDL_CreateRGBSurface()`          | `PS1Surface` struct             | done     | Custom surface type. |
| `SDL_BlitSurface()`               | `SPRT` primitive + `addPrim`    | done     | Hardware sprites. |
| `SDL_SetPaletteColors()`          | `LoadClut()`                    | done     | CLUT upload to VRAM. |
| `SDL_FillRect()`                  | `TILE` primitive                | done     | GPU rectangle. |
| `SDL_UpdateWindowSurface()`       | `PutDispEnv()`, `VSync()`       | done     | Buffer swap. |
| `SDL_RenderPresent()`             | `DrawOTag()`, `VSync()`         | done     | Display ordering table. |

### Initialization

```c
/* SDL2 */
SDL_Init(SDL_INIT_VIDEO);
SDL_Window *window = SDL_CreateWindow(...);

/* PSn00bSDK */
ResetGraph(0);                          /* Reset GPU */
SetVideoMode(MODE_NTSC);                /* Set video mode */
InitGeom();                             /* Initialize GTE */
SetDefDispEnv(&disp, 0, 0, 640, 480);
SetDefDrawEnv(&draw, 0, 0, 640, 480);
```

### Surface creation

```c
/* SDL2 */
SDL_Surface *surface = SDL_CreateRGBSurface(0, w, h, 8, ...);

/* PSn00bSDK */
PS1Surface *surface = malloc(sizeof(PS1Surface));
surface->width  = w;
surface->height = h;
surface->vramX  = nextVRAMX;            /* Allocate VRAM position */
surface->vramY  = nextVRAMY;
surface->pixels = malloc(w * h);
```

### Sprite blitting

```c
/* SDL2 */
SDL_BlitSurface(sprite, &srcRect, screen, &dstRect);

/* PSn00bSDK */
SPRT *sprt = (SPRT *)nextPrimitive;
setSprt(sprt);
setXY0  (sprt, x, y);
setWH   (sprt, w, h);
setUV0  (sprt, u, v);
setRGB0 (sprt, 128, 128, 128);
addPrim (ot, sprt);
```

### Display update

```c
/* SDL2 */
SDL_UpdateWindowSurface(window);

/* PSn00bSDK */
DrawSync(0);                            /* Wait for GPU */
VSync(0);                               /* Wait for vblank */
PutDrawEnv(&draw);                      /* Set draw environment */
DrawOTag(ot);                           /* Draw ordering table */
```

## Input

| SDL2                  | PSn00bSDK              | Status | Notes |
|-----------------------|------------------------|--------|-------|
| `SDL_PollEvent()`     | PAD library polling    | done   | Controller polling. |
| `SDL_KEYDOWN`         | Direct button state    | done   | Per-frame button bitmask. |
| `SDL_GetTicks()`      | `VSync` counter        | done   | Frame-based timing. |

### Initialization

```c
/* SDL2 */
SDL_Init(SDL_INIT_EVENTS);

/* PSn00bSDK */
InitPAD(pad_buff[0], 34, pad_buff[1], 34);
StartPAD();
ChangeClearPAD(0);
```

### Event polling

```c
/* SDL2 */
SDL_Event event;
while (SDL_PollEvent(&event)) {
    if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_ESCAPE) quit = 1;
    }
}

/* PSn00bSDK */
PADTYPE *pad = (PADTYPE *)pad_buff[0];
if (pad->stat == 0) {                   /* Controller connected */
    if (!(pad->btn & PAD_SELECT)) quit  = 1;
    if (!(pad->btn & PAD_START))  pause = !pause;
}
```

### Timing

```c
/* SDL2 */
uint32 now = SDL_GetTicks();

/* PSn00bSDK */
static int frameTick = 0;
VSync(0);                               /* Wait for frame */
frameTick++;
```

## Audio

| SDL2                  | PSn00bSDK                 | Status | Notes |
|-----------------------|---------------------------|--------|-------|
| `Mix_Init()`          | `SpuInit()`               | done   | Initialize SPU. |
| `Mix_LoadWAV()`       | VAG load + transfer       | partial | ADPCM conversion needed. |
| `Mix_PlayChannel()`   | `SpuSetKey()`             | partial | Voice allocation. |
| `Mix_Volume()`        | `SpuSetVoiceAttr()`       | partial | Volume control. |

### Initialization

```c
/* SDL2 */
Mix_Init(MIX_INIT_WAV);
Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 2048);

/* PSn00bSDK */
SpuInit();
SpuSetMute(SPU_OFF);
SpuSetCommonMasterVolume(0x3FFF, 0x3FFF);
```

### Sound load

```c
/* SDL2 */
Mix_Chunk *sound = Mix_LoadWAV("sound.wav");

/* PSn00bSDK */
/* 1. Convert WAV to VAG (ADPCM) — host-side step in the asset pipeline.
 * 2. Load the VAG buffer.
 * 3. DMA-transfer to SPU RAM. */
SpuSetTransferMode(SPU_TRANSFER_BY_DMA);
SpuSetTransferStartAddr(spu_addr);
SpuWrite(vag_data, vag_size);
SpuIsTransferCompleted(SPU_TRANSFER_WAIT);
```

### Sound playback

```c
/* SDL2 */
Mix_PlayChannel(-1, sound, 0);

/* PSn00bSDK */
SpuSetVoiceAttr(voice, ...);            /* Set voice parameters */
SpuSetKey(SPU_ON, voice);               /* Start playback */
```

## File I/O

| Standard C  | PSn00bSDK                 | Status | Notes |
|-------------|---------------------------|--------|-------|
| `fopen()`   | `CdSearchFile()`          | partial | Locate file on CD. |
| `fread()`   | `CdRead()`                | partial | Read sectors. |
| `fseek()`   | sector calculation        | partial | Manual positioning. |
| `fclose()`  | n/a                       | done   | No handles needed. |

### File opening

```c
/* Standard C */
FILE *f = fopen("RESOURCE.MAP", "rb");

/* PSn00bSDK */
CdlFILE file;
if (CdSearchFile(&file, "\\RESOURCE.MAP;1") == NULL) {
    /* file not found */
}
/* Store file position for later reads. */
```

### File reading

```c
/* Standard C */
fread(buffer, 1, size, f);

/* PSn00bSDK */
int sector = file.pos.minute * 60 * 75 +
             file.pos.second * 75 +
             file.pos.sector;
CdControlB(CdlSetloc, (u_char *)&file.pos, 0);
CdRead(num_sectors, (u_long *)buffer, CdlModeSpeed);
CdReadSync(0, 0);                       /* Wait for completion */
```

## Key model differences

| Aspect              | SDL2                              | PS1 |
|---------------------|-----------------------------------|-----|
| Surface management  | Software surfaces, direct pixels  | Hardware surfaces in VRAM, accessed via DMA |
| Rendering model     | Immediate mode (draw, flip)       | Ordering tables (queue primitives, submit) |
| Color format        | 8-bit indexed or 24/32-bit RGBA   | 16-bit [RGB555]({{ '/docs/glossary/#rgb555' | relative_url }}), or 8-/4-bit indexed with [CLUT]({{ '/docs/glossary/#clut' | relative_url }}) |
| Timing              | Millisecond timer (`SDL_GetTicks`)| Frame counter (`VSync`) |
| File I/O            | Standard C `stdio` (buffered)     | CD-ROM sectors (asynchronous, unbuffered) |

The biggest semantic gap is the rendering model. SDL2 lets you draw and
flip in arbitrary order; the PS1 GPU consumes an *[ordering table]({{ '/docs/glossary/#ot' | relative_url }})* — a
linked list of primitives sorted back-to-front — and renders the whole
list when you call `DrawOTag`. Every PS1 module in this codebase ends up
shaped around "build the OT, then submit", which is why
[`graphics_ps1.c`]({{ site.github_url }}/blob/main/src/graphics_ps1/graphics_ps1.c) is
much longer than the SDL host counterpart.

## PSn00bSDK quick reference

### Graphics — `psxgpu.h`

| Function                  | What it does |
|---------------------------|--------------|
| `ResetGraph(mode)`        | Reset GPU subsystem. |
| `SetDefDispEnv(env, x, y, w, h)` | Configure display environment. |
| `SetDefDrawEnv(env, x, y, w, h)` | Configure drawing environment. |
| `PutDispEnv(env)`         | Apply display environment. |
| `PutDrawEnv(env)`         | Apply drawing environment. |
| `LoadImage(rect, data)`   | DMA transfer to VRAM. |
| `MoveImage(src, dst)`     | Copy within VRAM. |
| `DrawSync(mode)`          | Wait for GPU drawing completion. |
| `VSync(mode)`             | Wait for vertical blank. |
| `ClearOTagR(ot, len)`     | Clear ordering table (reverse order). |
| `addPrim(ot, prim)`       | Add primitive to ordering table. |
| `DrawOTag(ot)`            | Process ordering table. |

### Audio — `psxspu.h`

| Function                              | What it does |
|---------------------------------------|--------------|
| `SpuInit()`                           | Initialize SPU. |
| `SpuQuit()`                           | Shutdown SPU. |
| `SpuSetMute(mode)`                    | Mute / unmute SPU. |
| `SpuSetCommonMasterVolume(L, R)`      | Set master volume. |
| `SpuSetVoiceAttr(voice, attr)`        | Configure voice parameters. |
| `SpuSetKey(on_off, voice_bit)`        | Start / stop voice. |
| `SpuSetTransferMode(mode)`            | Set DMA transfer mode. |
| `SpuSetTransferStartAddr(addr)`       | Set SPU RAM address. |
| `SpuWrite(data, size)`                | Write data to SPU RAM. |

### Input — `psxpad.h`

| Function                      | What it does |
|-------------------------------|--------------|
| `InitPAD(b1, l1, b2, l2)`     | Initialize controller buffers. |
| `StartPAD()`                  | Begin controller polling. |
| `ChangeClearPAD(mode)`        | Configure V-Blank acknowledgment. |
| Button constants              | `PAD_SELECT`, `PAD_START`, `PAD_UP`, `PAD_DOWN`, `PAD_LEFT`, `PAD_RIGHT`, `PAD_CROSS`, `PAD_CIRCLE`, `PAD_TRIANGLE`, `PAD_SQUARE`, `PAD_L1`, `PAD_L2`, `PAD_R1`, `PAD_R2`. |

### CD-ROM — `psxcd.h`

| Function                          | What it does |
|-----------------------------------|--------------|
| `CdInit()`                        | Initialize CD-ROM. **Do not call when booting from CD** — the BIOS already did it and a second init breaks things. |
| `CdSearchFile(file, name)`        | Locate file in ISO9660 filesystem. |
| `CdControlB(command, param, res)` | Send CD-ROM command. |
| `CdRead(sectors, dest, mode)`     | Read sectors asynchronously. |
| `CdReadSync(mode, result)`        | Wait for read completion. |

## A note on the SDL compat shim

There is no `src/sdl_compat_lite/` directory in the active tree. The
PSn00bSDK substitution happens at the call site behind `#ifdef PS1_BUILD`,
not via a translation shim. A specification document for what such a
shim would look like exists at
[`docs/ps1/research/SDL_COMPAT_LITE_SPEC.md`]({{ site.github_url }}/blob/main/docs/ps1/research/SDL_COMPAT_LITE_SPEC.md)
in the research directory, but it has not been implemented.

## Related pages

- [Build & toolchain]({{ '/docs/build/' | relative_url }}) — how
  PSn00bSDK is installed in the dev image.
- [Hardware]({{ '/docs/hardware/' | relative_url }}) — the
  underlying GPU / SPU / CD specs the mappings target.
- [Audio pipeline]({{ '/docs/audio/' | relative_url }}) — the
  SDL2 audio surface mapped to the PSn00bSDK SPU side.
- [Pause menu]({{ '/docs/pause-menu/' | relative_url }}) — extensive use
  of `POLY_F4`, `SPRT`, and the OT pattern.
- [Freeplay mode]({{ '/docs/freeplay/' | relative_url }}) — the
  largest live consumer of the `gr*` graphics surface.
- [Closed captions]({{ '/docs/captions/' | relative_url }}) — `SPRT`-based
  glyph rendering against a shared font atlas.
- [Lab: the two-day SPI bug]({{ '/lab/two-day-spi-bug/' | relative_url }})
  — war story for the SIO0 polling fix behind the
  `SDL_PollEvent` → SPI driver row in the Input section.
- [Glossary]({{ '/docs/glossary/' | relative_url }}) —
  definitions for the project-specific terms referenced
  throughout this page:
  [`OT`]({{ '/docs/glossary/#ot' | relative_url }}),
  [`SPRT`]({{ '/docs/glossary/#sprt' | relative_url }}),
  [`FntFlush`]({{ '/docs/glossary/#fntflush' | relative_url }}),
  [`SPI driver`]({{ '/docs/glossary/#spi' | relative_url }}),
  [`tx_len`]({{ '/docs/glossary/#tx-len' | relative_url }}),
  [`VAG`]({{ '/docs/glossary/#vag' | relative_url }}),
  [`SPU`]({{ '/docs/glossary/#spu' | relative_url }}),
  [`VRAM`]({{ '/docs/glossary/#vram' | relative_url }}),
  [`RGB555`]({{ '/docs/glossary/#rgb555' | relative_url }}),
  [`CLUT`]({{ '/docs/glossary/#clut' | relative_url }}).
  Grouped by area, not alphabetical.

## View source on GitHub

- [`docs/ps1/api-mapping.md`]({{ site.github_url }}/blob/main/docs/ps1/api-mapping.md) — original document.
- [`src/graphics_ps1/graphics_ps1.c`]({{ site.github_url }}/blob/main/src/graphics_ps1/graphics_ps1.c) — the GPU surface in practice.
- [`src/platform/ps1/events_ps1.c`]({{ site.github_url }}/blob/main/src/platform/ps1/events_ps1.c) — controller polling.
- [`src/platform/ps1/sound_ps1.c`]({{ site.github_url }}/blob/main/src/platform/ps1/sound_ps1.c) — SPU side.
- [`src/cdrom_ps1.c`]({{ site.github_url }}/blob/main/src/cdrom_ps1.c) — CD I/O.
