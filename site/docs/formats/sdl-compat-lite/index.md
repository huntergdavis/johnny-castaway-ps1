---
layout: page
title: SDL compat lite
eyebrow: Reference
subtitle: The narrow set of gr* graphics functions gameplay code is allowed to call. Both the host SDL2 build and the PS1 PSn00bSDK build implement this surface.
description: Reference for the SDL compat lite contract — the narrow graphics surface the project's gameplay code is allowed to depend on, regardless of whether it is running against SDL2 on the host or PSn00bSDK on the PS1. Layer lifecycle, frame lifecycle, sprite drawing, primitive drawing, region save/restore, the gap matrix, and the kill list of long-standing leaks.
---

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

A labor of love by Hunter Davis. The host capture build runs against
SDL2 because that is what the original Johnny Castaway engine ports use,
and SDL2 is what gives the host a pixel-accurate rendering of every
scene to capture from. The PS1 build cannot run SDL2 — there is no
SDL2 for PSn00bSDK, and even if there were, its memory and threading
assumptions would not survive the console. Both builds need to share
gameplay code anyway, because writing two divergent versions of `ttm.c`
or `walk.c` would mean two divergent renderers, two divergent bug
surfaces, and two pipelines that never agree on what scene N looks
like.

The bridge is the SDL compat lite contract. It is not a file format. It
is not even a piece of code that ships in either build. It is a written
specification — at
[`SDL_COMPAT_LITE_SPEC.md`]({{ site.github_url }}/blob/main/docs/ps1/research/SDL_COMPAT_LITE_SPEC.md)
— describing the narrow graphics surface gameplay code is allowed to
call. The host build implements that surface in terms of SDL2. The PS1
build implements it in terms of PSn00bSDK and the dirty-row tile
renderer. Gameplay code uses neither directly; it calls `gr*` functions
and accepts whatever result the build provides.

The contract surface is intentionally smaller than full SDL.

> Define the narrow graphics/runtime surface the PS1 path must satisfy
> so gameplay code can target stable semantics instead of PS1-specific
> replay and recovery workarounds.

If you paid for this, you were cheated. Open source and free.

## Contract surface

Six functional areas. Each lists the public API surface and the required
behavior — implementation details are explicitly *out of contract*.

### Layer lifecycle

```c
grNewLayer();
grFreeLayer();
grLoadScreen();
grInitEmptyBackground();
```

Required behavior: gameplay can acquire a logical per-thread layer.
Layers persist across frames until explicitly released. Background load
establishes the clean scene base for subsequent draw and restore. Layer
allocation and release are implementation details, not gameplay state.

### Frame lifecycle

```c
grBeginFrame();      /* PS1 only */
grUpdateDisplay();
```

Required behavior: the runtime starts a frame from a deterministic base
state. The current PS1 implementation achieves this with
clean-background tile copies plus dirty-row restore — not a full-frame
rebuild every tick. The runtime presents layers in deterministic order:
background, saved/restored zone overlay (if used), active ADS/TTM thread
layers in slot order, holiday/extra layer last. Gameplay must not know
whether present uses SDL blits, RAM compositing, or GPU primitives.

### Sprite drawing

```c
grLoadBmp();
grReleaseBmp();
grDrawSprite();
grDrawSpriteFlip();
```

Required behavior: sprite draw uses transparent blit semantics. Flipped
draw is behaviorally equivalent to horizontal flip of the same source.
Sprite resources are loaded and released by slot, not by caller-managed
surfaces. Caller-visible semantics are position, image selection, and
ordering only. The runtime may satisfy this through either legacy BMP
decode or offline-transcoded PSB sprite bundles; that choice must not
change gameplay-visible semantics.

### Primitive drawing

```c
grDrawPixel();
grDrawLine();
grDrawRect();
grDrawCircle();
grClearScreen();
```

Required behavior: primitives apply to the current logical target and
layer semantics. `grClearScreen()` clears the current logical layer, not
global gameplay state. Implementations may choose software or hardware
paths, but results must remain deterministic for the script patterns
that exist.

### Region save / restore

```c
grCopyZoneToBg();
grSaveImage1();
grSaveZone();
grRestoreZone();
```

Required behavior: the runtime supports the current script pattern of
saving and later restoring a bounded region. Gameplay does not need to
know whether restore comes from a separate saved layer, clean
background tiles, or offline templates. Restore behavior must not depend
on replay-record resurrection.

### Clip and offsets

```c
grSetClipZone();
extern int grDx, grDy;     /* Global offsets */
```

Required behavior: gameplay can request bounded drawing regions where
scripts rely on it. Coordinate offsets remain a runtime presentation
concern, not gameplay logic.

## Out of contract

These are implementation details. They appear on the PS1 path today but
must not leak back into gameplay logic:

- replay records
- actor continuity matching
- handoff replay injection
- background tile management
- dirty-row tracking and partial tile upload
- pack lookup and prefetch
- PSB registry lookup
- BMP/TTM caching policy

The dirty-row tracking subsystem, for example, is the engine the
[dirty-region template]({{ '/docs/formats/dirty-region/' | relative_url }})
feeds. Gameplay code does not see it. Gameplay code calls
`grUpdateDisplay()` and gets a presented frame.

## Current gameplay dependency map

Direct graphics call sites in gameplay/runtime code:

| File          | Calls used |
|---------------|------------|
| `ttm.c`       | clip zone, copy/save/restore zone, pixel/line/rect/circle, draw sprite / draw sprite flip, clear screen, load screen, load BMP |
| `walk.c`      | clear screen, draw sprite / draw sprite flip |
| `island.c`    | load screen, load BMP / release BMP, draw sprite / draw sprite flip |
| `ads.c`       | layer allocation/release, frame begin/present, restore background tiles, replay sprite path *(current workaround, not contract)* |

`ads.c`'s replay sprite path is the largest current contract violation.
It exists because PS1 frame correctness used to depend on remembered
sprite identity rather than explicit restore data. Removing that
dependency is on the
[kill list](#kill-list-for-long-standing-bug-sources) below.

## Gap matrix

Legend:

- **Complete** — behavior exists and matches the current contract.
- **Partial** — behavior exists but is implemented through PS1-specific
  machinery or only covers current script patterns.
- **Stub** — declared but not meaningfully implemented on PS1.
- **Leak** — gameplay currently depends on non-contract implementation
  details.

| Area | PC SDL path | PS1 path | Status | Notes |
|------|-------------|----------|--------|-------|
| Layer allocation/release | Yes | Yes | Complete | Both paths expose logical layers. |
| Background load/base scene | Yes | Yes | Complete | PS1 uses clean background tiles internally. |
| Frame present ordering | Yes | Yes | Complete | PS1 order is deterministic, though implemented differently. |
| Frame begin/reset | Implicit | Explicit | Partial | PS1 requires `grBeginFrame()`; contract should standardize this lifecycle. |
| Draw sprite | Yes | Yes | Complete | PS1 path is pack-backed and authoritative now; sprite source may be BMP or PSB. |
| Draw sprite flip | Yes | Yes | Complete | Same semantics, different backend. |
| Transparent blit semantics | Yes | Yes | Partial | Works on current paths; still split across BMP and PSB backends and needs semantic convergence. |
| Load/release BMP by slot | Yes | Yes | Complete | Runtime implementation differs, caller contract matches. |
| Draw rect | Yes | Yes | Partial | PS1 uses optimized software tile writes and dirty-row tracking instead of SDL fill. |
| Draw pixel | Yes | Yes | Complete | Present on both. |
| Draw line | Yes | Weak | Partial | PS1 is still effectively stub/cosmetic for now. |
| Draw circle | Yes | Weak | Partial | PS1 path is limited and should be validated against actual script usage. |
| Set clip zone | Yes | Minimal | Partial | Exists on PS1 but needs tighter semantic validation. |
| Copy zone to background | Yes | Yes | Partial | PS1 commits the rectangle into the clean-background restore baseline instead of keeping a separate saved-zones overlay layer. |
| Save image1 | Minimal | Yes | Partial | PS1 routes it through the same bounded clean-background commit behavior as `COPY_ZONE_TO_BG`. |
| Save zone | Yes | Yes | Partial | PS1 tracks one active zone, matching current script assumptions. |
| Restore zone | Yes | Yes | Partial | PS1 restores from clean background tiles, but only for the simple active-zone pattern. |
| Clear screen | Yes | Divergent | Partial | PS1 intentionally suppresses some clears to avoid blinking; this needs to become offline/runtime policy. |
| Replay sprite | N/A | Yes | Leak | Legacy PS1-only workaround, now a shrinking boundary. |
| Actor continuity / recovery injection | N/A | Yes | Leak | Must continue moving out of gameplay-visible correctness path. |
| Dirty-row tile restore/upload | N/A | Yes | Implementation detail | Renderer optimization boundary; stays invisible to gameplay semantics. |
| PSB sprite path | N/A | Yes | Partial | Good fit for the target architecture, still needs route-by-route convergence with legacy BMP behavior. |

The matrix is the canonical state-of-things. Status downgrades and
upgrades land here as the renderer changes.

## Which functions appear in which build

Both builds expose the entire contract surface. The differences are in
the implementation file each build links:

| Build | Implementation file        | Backend                 |
|-------|----------------------------|--------------------------|
| Host  | `graphics.c` (host)        | SDL2 surfaces, blits     |
| PS1   | `graphics_ps1.c`           | PSn00bSDK GPU + tile renderer |

Gameplay code in `ttm.c`, `walk.c`, `island.c`, `ads.c` is one set of
files compiled twice — see the `CORE_SOURCES` list in
[Build & toolchain]({{ '/docs/build/' | relative_url }}). The build
system selects which graphics implementation to link based on the
target.

## Kill list for long-standing bug sources

The schema doc keeps a running list of the highest-value leaks to
remove as the new architecture advances:

1. `ads.c` dependence on replay carry/merge/recover for visual
   correctness.
2. PS1-only interpretation of `CLEAR_SCREEN` to suppress blinking.
3. PS1 frame correctness depending on remembered sprite identity rather
   than explicit restore data.
4. Remaining sprite-path divergence where PSB and BMP do not yet behave
   identically.
5. Remaining stubbed or partial zone/image operations that force
   unrelated replay behavior to carry correctness.

The list is the prioritized work. The dirty-region template work
(item 2) is the active front; offline templates plus
`grForceFullRedrawNextFrame` are the lever that lets `CLEAR_SCREEN`
become a runtime policy decision instead of a gameplay-visible
divergence.

## Immediate next implementation targets

Direct from the schema:

1. Replace `CLEAR_SCREEN` divergence with explicit restore/template
   policy on the pilot route.
2. Emit offline dirty-region templates for a pilot family and consume
   them from pack-backed runtime data.
3. Reduce `ads.c` replay continuity on the pilot path until it is no
   longer a correctness dependency.

(2) is largely done — the templates exist and ship as compiler
sidecars. (1) and (3) are in progress.

## Related references

- [API mapping]({{ '/docs/api/' | relative_url }}) — the SDL2 ↔
  PSn00bSDK symbol table this contract sits on top of.
- [Dirty region template]({{ '/docs/formats/dirty-region/' | relative_url }}) —
  the offline data the contract's restore behavior leans on.
- [Build & toolchain]({{ '/docs/build/' | relative_url }}) — where the
  two graphics implementations get selected.

## Source on GitHub

The contract is a written spec, two divergent implementations, and a
small set of gameplay-code consumers. The body cites all of them; this
section collects them.

- [`docs/ps1/research/SDL_COMPAT_LITE_SPEC.md`]({{ site.github_url }}/blob/main/docs/ps1/research/SDL_COMPAT_LITE_SPEC.md)
  — the canonical spec; the "Direct from the schema" next-targets list
  above is verbatim from this file.

**Implementations** (one per build):

- [`src/host/graphics.c`]({{ site.github_url }}/blob/main/src/host/graphics.c)
  — host SDL2 surfaces and blits.
- [`src/graphics_ps1/graphics_ps1.c`]({{ site.github_url }}/blob/main/src/graphics_ps1/graphics_ps1.c)
  — PS1 PSn00bSDK GPU + tile renderer.

**Gameplay-code consumers** (the four modules that hold to the
contract instead of writing two divergent renderers):

- [`src/host/ttm.c`]({{ site.github_url }}/blob/main/src/host/ttm.c)
  — clip zone, copy/save/restore zone, pixel/line/rect/circle,
  draw sprite, clear/load screen, BMP load.
- [`src/walk/walk.c`]({{ site.github_url }}/blob/main/src/walk/walk.c)
  — clear screen, draw sprite (flip).
- [`src/scene/island.c`]({{ site.github_url }}/blob/main/src/scene/island.c)
  — load screen, BMP load/release, draw sprite (flip).
- [`src/ads/ads.c`]({{ site.github_url }}/blob/main/src/ads/ads.c)
  — layer allocation/release, frame begin/present, background
  restore. The replay sprite path is the largest current contract
  violation (named in the section above).
