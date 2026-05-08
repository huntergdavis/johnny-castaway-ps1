---
layout: page
title: Why I keep porting Johnny Castaway
eyebrow: Lab · Personal history
subtitle: Dreamcast, embedded Linux, picture frames, text edition, PS1. Same little island. Different machines.
description: A personal retrospective on Hunter Davis repeatedly porting Johnny Castaway across Dreamcast, embedded Linux, picture frames, text editions, and now PlayStation 1.
date: 2026-04-26
---

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

## The recurring island

I keep coming back to Johnny Castaway.

That is not an accident. I have ported Johnny to Dreamcast. I have run him on
embedded Linux boxes and framebuffer devices. I have built versions for weird
little picture-frame class machines. I have done a text edition. Now there is
a PS1 disc.

At some point a repeated port stops being a novelty and becomes a practice.

## Why Johnny works

Johnny is small enough to fit in your head and rich enough to fight back.

There are finite scenes. Finite sprites. Finite sounds. A tiny state machine:
day, night, tide, raft, holiday. That makes it approachable. Then you start
porting it and discover bytecode interpreters, dirty rectangles, palette
quirks, audio timing, resource archives, and thirty-year-old assumptions about
Windows 3.1 that do not survive first contact with a console.

That is the ideal hobby project. Contained, but not trivial.

## The platform teaches the project back to you

Every port reveals a different Johnny.

Dreamcast makes you care about optical media and homebrew ergonomics.
Embedded Linux makes you care about framebuffer modes and SDL assumptions.
Picture frames make you care about memory and boot friction. Text edition
makes you ask what the gag is when the pixels are gone. PS1 makes you care
about [VBlank]({{ '/docs/glossary/#vblank' | relative_url }}), VRAM, SPU RAM, CD seek latency, and what it means to be faithful
when the hardware is both older and stranger than the PC target.

Same island. Different lesson.

## Small joys

There is also the plain answer: it makes me smile.

Johnny is a little man on a little island doing little things. He fishes. He
gets bonked by coconuts. He misses boats. A pumpkin appears in October. It is
not important software in the normal sense. That is why it matters to me.

The internet I like is full of small, unnecessary, carefully made things.
Johnny belongs there.

## Why PS1 now

The PS1 port is the most serious version because the target is least forgiving.
Two megabytes of RAM. One megabyte of VRAM. A 2x CD-ROM. A fixed-function GPU.
No SDL safety net. No desktop filesystem. No "just allocate another buffer."

That constraint makes the project honest. If the disc boots and the scene
plays, the work is real.

## Cross-links

- [About Hunter Davis](https://hunterdavis.com/)
- [History]({{ '/about/history/' | relative_url }})
- [Hardware]({{ '/docs/hardware/' | relative_url }}) — the
  "two megabytes of RAM, one megabyte of VRAM, a 2x CD-ROM"
  envelope this essay closes on, documented in detail.
- [The pivot that almost did not happen]({{ '/lab/pixel-perfect-pivot/' | relative_url }})
- [The 63-scene grind]({{ '/lab/the-63-scene-grind/' | relative_url }})
- [Scene ledger]({{ '/scenes/' | relative_url }}) and
  [performance battle card]({{ '/perf/' | relative_url }}) — the
  two ledgers that measure what "the work is real" actually means
  on this port.
