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
- Drop optional prefetch buffers when a large
  [clean-rect]({{ '/docs/glossary/#clean-rect' | relative_url }})
  snapshot would otherwise fragment the heap. The
  [v0.8.0 clean-memory-relief drop-prefetch]({{ '/lab/from-87-to-99-5/' | relative_url }})
  pass made this a per-scene opt-in for fourteen random-position
  scenes; the
  [v0.8.1 generalization]({{ '/lab/v081-mary4-freeze/' | relative_url }})
  centralized the pressure estimator after a randomized soak found a
  `MARY 4` scene-load freeze the per-commit matrix never reached.

The [resource catalog]({{ '/resources/' | relative_url }}) is the public map of
the files competing for space. The [source library]({{ '/source/' | relative_url }})
preserves the older memory notes and false starts.

## The Lesson

On a small machine, "can I compute this?" is usually the wrong first question.
The better question is: where does the data live when nobody is looking at it?
The PS1 port started working when the answer stopped being "everywhere."

## Related pages

- [Hack: start here]({{ '/hack/start-here/' | relative_url }})
  — first-day flow if you haven't already done the build and
  boot loop.
- [Hack: learn C from Johnny]({{ '/hack/learn-c/' | relative_url }})
  — the ownership-and-explicit-state habits that make the
  budget above tractable in C.
- [Hack: visual debugging]({{ '/hack/visual-debugging/' | relative_url }})
  — most fragmentation and clean-rect regressions surfaced
  through the screenshot loop, not text logs.
- [Hack: performance loop]({{ '/hack/performance-loop/' | relative_url }})
  — the matrix that grades every memory change against the
  same target-speed bar.
- [Hack: port to a new platform]({{ '/hack/port-to-a-new-platform/' | relative_url }})
  — what changes when 2 MB / 1 MB / 512 KB becomes some other
  envelope.
- [Hardware]({{ '/docs/hardware/' | relative_url }}) —
  the canonical envelope reference: bus widths, SPU RAM
  sectors, CD seek timing, VRAM frame layout.
- [Performance reference]({{ '/docs/performance/' | relative_url }})
  — where each `loop_vb` / `target_vb` / `blocking_vb` column
  comes from and how memory pressure shows up in those columns.
- [Story-loop walks]({{ '/docs/walks/' | relative_url }}) —
  the persistent clean buffer named in the v0.8.0 / v0.8.1
  Evolution-by-release section is a direct response to this
  page's "VRAM allocation as explicit state" rule.
- [Lab: from 87 to 99.5]({{ '/lab/from-87-to-99-5/' | relative_url }})
  — the post-validation performance retrospective whose single
  biggest unlock was the [drop-prefetch]({{ '/docs/glossary/#drop-prefetch' | relative_url }})
  experiment: trading streaming headroom for the clean-rect bytes
  the budget on this page tracks. Cited inline above; surfaced
  here as a Related entry too.
- [v0.8.1 retrospective]({{ '/lab/v081-mary4-freeze/' | relative_url }})
  — the soak loop that found the clean-rect pressure freeze,
  driven by exactly the budget-overrun story above.
- [Glossary: clean-rect]({{ '/docs/glossary/#clean-rect' | relative_url }})
  · [Glossary: VRAM]({{ '/docs/glossary/#vram' | relative_url }})
  · [Glossary: SPU]({{ '/docs/glossary/#spu' | relative_url }})
  · [Glossary: drop-prefetch]({{ '/docs/glossary/#drop-prefetch' | relative_url }})
