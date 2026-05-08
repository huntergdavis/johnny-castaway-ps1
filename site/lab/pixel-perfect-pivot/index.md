---
layout: page
title: The pivot that almost did not happen
eyebrow: Lab · Retrospective
subtitle: The project could have shipped at "close enough." That would have been the wrong project.
description: A retrospective on the Johnny Castaway PS1 pivot from approximate SDL-to-PS1 porting toward host-captured, pixel-perfect foreground replay.
date: 2026-04-26
---

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

## Close enough was available

There is a version of this project that ships much earlier.

It interprets enough ADS and TTM bytecode on the PS1 to show Johnny walking,
fishing, and waving. The palette is mostly right. The island is mostly right.
The sounds play when something like the right thing happens. It runs in
DuckStation. It has a release zip. People say "neat" and move on.

That version was available. It was also the wrong project.

## The problem with approximate

Johnny Castaway lives in timing. The gag is not "Johnny goes fishing." The gag
is the pause before he reacts to the starfish. The small walk back. The way the
ocean keeps moving while the foreground holds. The holiday decoration that is
barely there unless you happen to notice it.

Approximate playback erases those edges. It makes the port impressive as a
technical demo and weak as a preservation object.

Once that became obvious, the target changed: the PS1 would replay captured
foreground packs, not reinterpret the whole Sierra runtime at scene time.

## The host-and-replay move

The host build does what the desktop is good at. It runs the decoded Sierra
engine, captures foreground frames, records sound events, and writes compact
FG2 packs.

The PS1 does what the console is good at. It loads those packs, composites
them over its own background, runs waves, stamps holidays, plays SPU samples,
and keeps VBlank cadence.

That split is the project. It is the reason a 1992 screensaver fits into a
1994 console in a way that can be tested.

## What the pivot cost

It cost tooling. A lot of tooling.

The moment you choose pixel-perfect replay, you need pack formats, manifests,
dirty-region templates, reference captures, validation scripts, build scripts,
scene ledgers, timing logs, and a way to compare the PS1 output against the
host without lying to yourself.

It also cost pride. A straight interpreter port is cleaner to brag about. A
hybrid pipeline is less romantic. It says: the console is not running the
whole original engine. It is running the right artifact for the hardware.

I am fine with that trade. The disc plays.

## What it bought

It bought a real acceptance bar.

FISHING 1 can be validated because there is a reference to validate against.
The scene either matches or it does not. The sound either lands on the right
frame or it does not. The runtime either survives night, tide, holiday, and
raft variants or it does not.

The rest of the project inherited that bar. Sixty-one scenes
remained at the time of the pivot; the
[63-scene grind]({{ '/lab/the-63-scene-grind/' | relative_url }})
closed on `2026-05-05` with `ACTIVITY 9` as the final row, and
the post-validation work moved to the
[performance arc]({{ '/lab/from-87-to-99-5/' | relative_url }}).

## Cross-links

- [Method]({{ '/about/method/' | relative_url }})
- [FG2 pack payload]({{ '/docs/formats/pack-payload/' | relative_url }})
- [Performance work]({{ '/docs/performance/' | relative_url }})
- [The 63-scene grind]({{ '/lab/the-63-scene-grind/' | relative_url }})
- [From 87 to 99.5]({{ '/lab/from-87-to-99-5/' | relative_url }}) —
  the post-validation arc this pivot enabled.
- [Performance battle card]({{ '/perf/' | relative_url }}) —
  the live timing matrix that followed the visual ledger.
