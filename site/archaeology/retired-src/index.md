---
layout: page
title: Retired source code
eyebrow: Archaeology
subtitle: Three early PS1 bring-up sources, kept as a record of the smallest test programs the port started from.
description: The three retired C source files in the Johnny Castaway PS1 fan port's archaeology — the minimal bring-up programs that proved the GPU and the CD-ROM I/O before the full game integration began.
---

The `retired-src/` directory holds three small C source files from the
earliest weeks of the PS1 port. Each one was the smallest possible
program that would prove some specific part of the hardware worked.
They are kept as archaeology because they capture the bring-up
methodology in concrete form — start with the smallest test that
exercises one subsystem, get it running, then move to the next. The
production code that descends from these tests now lives in
`src/graphics_ps1/graphics_ps1.c`, `src/cdrom_ps1.c`, and `src/ps1_main.c`. The
retired versions are short enough to read in one sitting.

All three files are 1.4 to 4.4 KB. None of them are built by the
current CMake configuration. They were last touched in commit
`38a33aff` ("PS1: Fix rendering order — triangle and square now
display correctly") in late 2025, then carried forward unchanged into
`docs/ps1/archaeology/retired-src/` as part of the
[2026-04-04 repository reorganisation]({{ '/archaeology/timeline/' | relative_url }}).

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

## ps1_minimal_main.c (4.4 KB)

The minimal main was the first program that proved the PS1 GPU could
draw something the operator could see. It bypasses every part of the
game — no resource loading, no scene logic, no input — and runs a hard
render loop that draws a yellow triangle on the left and a cyan square
on the right against a dark blue background. The whole file is 159
lines.

What it teaches: the bring-up was empirical. A double-buffer pair of
`DISPENV`/`DRAWENV` structures, two ordering tables, two primitive
buffers, the `ResetGraph(0)` -> `SetVideoMode(MODE_NTSC)` sequence,
and the `PutDispEnv` -> `PutDrawEnv` -> `DrawOTag` -> `DrawSync` ->
`VSync` rhythm that every PSn00bSDK program starts from. The comment
on line 64 — `/* DON'T call CdInit() - it breaks when booting from CD! */`
— is one of the small painful discoveries from the era; calling the
CD initialiser too early hangs the boot.

The file's existence is what made the GPU layer's eventual 3,000+
lines tractable. Each new graphics feature could be added against a
known-working baseline — start from the minimal, verify the addition
displays, fold the addition into the real graphics module.

## ps1_cdtest_main.c (1.4 KB)

The CD test was the smallest possible CD-ROM I/O check. It calls
`cdromInit()`, opens `RESOURCE.MAP`, reads one byte, and reports
results through screen colour. Red means starting; dark red means
`cdromInit()` failed; magenta means file-not-found; cyan means file
opened; yellow means read failed; blue means success. Sixty-six
lines.

What it teaches: CD-ROM I/O on the PS1 is not file-system I/O. The
`cdrom_ps1.h` interface this program tests is the project's own thin
wrapper around the BIOS calls — `cdromInit`, `cdromOpen`, `cdromRead`,
`cdromClose` — and the wrapper itself was written and tuned during
exactly this kind of bring-up. The screen-colour-as-debug-output
pattern (no printf because the framebuffer is the only output the
operator can see during early bring-up) is the same pattern used in
the dirty-rect overlay and the bringup-status corner the project
shipped much later.

This file was retired once `RESOURCE.MAP` and `RESOURCE.001` were both
loading reliably as part of the full game's startup. The test
succeeded and there was nothing more to learn from it.

## ps1_ultra_minimal.c (2.5 KB)

A third minimal — even smaller than the first — that uses a single
buffer pair instead of double buffering and draws a yellow square (as
two `POLY_F3` triangles) and a red triangle. Ninety-one lines. The
comment on line 32 (`v12 - FINAL DEMO`) is the version tag from when
this was being iterated on every few hours.

What it teaches: the *order* of `PutDrawEnv` and `DrawOTag` matters.
The comment on line 82 is explicit: *CRITICAL: Call PutDrawEnv BEFORE
DrawOTag. This clears the background (isbg=1) THEN we draw on top.*
Reverse the order and the primitives get cleared right after they're
drawn, which renders an apparently-empty screen with the background
clear colour. That ordering bug ate hours during the original
bring-up, and the comment exists so the next person who hits the
symptom recognises it.

The file also drew its primitives by writing into a stack-allocated
8 KB buffer rather than a global ping-pong buffer, which let it stand
alone without the rest of the project's allocation infrastructure.
That single-buffer approach worked but didn't scale — it could not
support the project's eventual draw-list sizes — so the production
graphics layer moved to the double-buffer pattern from
`ps1_minimal_main.c` instead.

## Why these were excised from the active build

The minimals served their purpose. Each one proved a subsystem; the
real game replaced each one in turn. Keeping a `cdtest_main.c` in the
repository alongside the production main would have been clutter —
the CD code path is exercised every time the game boots, and a small
test that opens one file and reads one byte is no longer informative.

The graphics minimals fall into the same category. Once the production
graphics path was stable, the minimals were redundant. They couldn't
test the production path because they bypass it; they couldn't be
*part* of the production path because they don't load resources or
run the game loop.

What the project chose instead, for ongoing graphics validation, is
the
[regtest reference set]({{ '/archaeology/regtest-references/' | relative_url }}) —
captures of real scenes against real game state. That answers the
question *does the production renderer still produce the expected
pixels* directly, rather than indirectly through synthetic tests.

## What this teaches about the build's shape over time

The active build today is one program: `src/ps1_main.c`, with the
graphics, sound, input, CD, scene-playback, and bringup-status layers
linked alongside it. There is no `_test_main` or `_minimal_main` in
the production tree.

That wasn't always the build's shape. In late 2025 the build had
multiple entry points coexisting — one for each minimal, one for the
real game — managed through manual edits to the CMake target
selection. The project's standard practice was to switch entry points
by hand when verifying a specific subsystem.

The retired-src directory is the trace of that practice. Three files
that *used to be* `main()` in three different builds, now sitting in
an archaeology directory, replaced by exactly one `main()` in the
production tree. That progression — many small mains to one real one —
is the ordinary shape of an embedded project's bring-up phase.

## Cross-links

- [Era timeline]({{ '/archaeology/timeline/' | relative_url }}) —
  context for when these files were active (the embedded port
  bootstrap era, October 2025 through March 2026).
- [Build documentation]({{ '/docs/build/' | relative_url }}) — the
  current build system, which has one entry point.
- [Hardware documentation]({{ '/docs/hardware/' | relative_url }}) — the
  PS1 architectural notes that frame why each minimal had to exist.

## Source on GitHub

[docs/ps1/archaeology/retired-src/]({{ site.github_url }}/tree/main/docs/ps1/archaeology/retired-src/)
