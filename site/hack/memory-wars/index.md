---
layout: page
title: Memory Wars
eyebrow: Curious hacker path
subtitle: Two megabytes is enough only after you stop spending it twice.
description: Memory and asset-budget guide for the Johnny Castaway PS1 port.
---

The PS1 has 2 MB of main RAM, 1 MB of VRAM, and 512 KB of SPU RAM. Those numbers
sound generous until you try to carry a desktop interpreter, decoded art,
scene state, audio, overlays, and a CD streaming buffer at the same time.

The early port treated memory like a problem to optimize later. The working
port treats memory like the architecture.

## The Shape Of The Budget

Main RAM holds runtime code, pack buffers, control state, captions, holiday
metadata, and the current foreground replay data. VRAM holds backgrounds,
sprite banks, CLUTs, and transient uploads. SPU RAM holds all VAG sound effects
preloaded at boot because late audio streaming would fight the scene timing.
The CD drive is slow enough that layout and prefetch matter even when a file is
small.

That is why the project has separate pages for
[hardware]({{ '/docs/hardware/' | relative_url }}),
[performance]({{ '/docs/performance/' | relative_url }}),
[audio]({{ '/docs/audio/' | relative_url }}), and
[formats]({{ '/docs/formats/' | relative_url }}). They are not isolated topics;
they are the same budget seen from different angles.

## What Worked

The big unlock was the hybrid replay pipeline. Running Sierra's bytecode live
would keep too many possibilities resident at once. Captured foreground packs
turn each scene into a narrow contract: load this pack, replay these frames,
fire these sound cues. The PS1 no longer needs to be a general-purpose Sierra
engine.

Other practical wins were smaller:

- Preload the VAG sound set and rotate SPU voices.
- Keep overlays tiny and indexed.
- Use generated tables for holiday data and scene routing.
- Make pack formats dumpable, because "out of memory" without a file-level explanation is just folklore.
- Treat VRAM allocation as explicit state, not as a magical side effect of drawing.

The [resource catalog]({{ '/resources/' | relative_url }}) is the public map of
the files competing for space. The [source library]({{ '/source/' | relative_url }})
preserves the older memory notes and false starts.

## The Lesson

On a small machine, "can I compute this?" is usually the wrong first question.
The better question is: where does the data live when nobody is looking at it?
The PS1 port started working when the answer stopped being "everywhere."
