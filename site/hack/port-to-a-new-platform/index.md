---
layout: page
title: Port To A New Platform
eyebrow: Curious hacker path
subtitle: How to decide whether your target wants an interpreter, a replay engine, or a custom compromise.
description: Practical platform-porting guide based on the Johnny Castaway PS1 port.
---

Johnny Castaway has been dragged onto enough strange machines now that the
first question has become predictable: what kind of target is this?

On a desktop, the answer can be "run the engine." On embedded Linux with a
framebuffer, it might still be "run the engine, but be careful." On the PS1,
that answer looked plausible and turned out to be wrong. The winning answer
was: run the engine on the host, capture what it does, and ship the console a
deterministic replay stream.

## The Decision

Ask these questions before writing code:

- Does the target have enough RAM for the original runtime plus decoded resources?
- Does it have a filesystem, or only a disc/card/ROM layout?
- Can it update the screen the way the original engine expects?
- Is audio streaming cheap, or should samples be preloaded?
- Can you debug with text logs, or do logs destabilize timing?
- Is the target meant to be exact, charming, or merely possible?

The PS1 answers point toward foreground packs. The Dreamcast or a Linux
handheld may point somewhere else. The method page explains the PS1 choice in
detail: [About / Method]({{ '/about/method/' | relative_url }}).

## The Porter's Checklist

First, build a minimal "island boots" program. Do not bring the whole engine.
Show a background, prove input, prove audio, prove that assets can be loaded
from the medium you intend to ship.

Second, build a deterministic host reference. If you cannot say what the target
should draw on frame 37, you cannot know when frame 37 is wrong.

Third, choose the runtime contract. For this port that contract is FG2: the PS1
gets frames, spans, palettes, and sound cues. The [format references]({{ '/docs/formats/' | relative_url }})
and [resource catalog]({{ '/resources/' | relative_url }}) show what that
contract looks like as public documentation.

Fourth, keep a boring escape hatch. The PS1 port has boot parameters, pause
menu toggles, scene forcing, and random-seed control because testing a scene
should not require waiting for a screensaver to randomly pick it.

## What Unlocked Milestones Here

The real milestones were not romantic. Bounded `printf` made long runs
possible again. Visual debugging made progress possible before text logging
was safe. The regtest harness made frame drift obvious. The pause menu made
runtime state inspectable. The holiday selector, tide toggle, raft toggle, and
date picker were features for players, but they were also instruments for the
person trying to verify the port.

Read [The performance loop]({{ '/hack/performance-loop/' | relative_url }})
next. Platform ports are less about big rewrites than about feedback loops that
are tight enough to survive frustration.
