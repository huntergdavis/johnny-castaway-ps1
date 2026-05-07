---
layout: page
title: Curious Hacker's Guide
eyebrow: Learn by taking the machine apart
subtitle: A practical path through the codebase for people who want to port Johnny, learn C, or build their own weird little platform target.
description: A curious-hacker learning path for the Johnny Castaway PS1 port, covering the build, source map, visual debugging, memory constraints, regression harness, and new-platform porting strategy.
---

This section is for the reader who looks at a 1992 Windows screensaver
running on a 1994 console and thinks: I want to understand how that trick
works. Maybe you want to learn C by reading a real codebase. Maybe you want
to port Johnny to another strange machine. Maybe you just want to know which
tools actually moved the project forward when the elegant plan failed.

The answer is not one heroic abstraction. The answer is a stack of practical
loops: build the thing, see the thing, freeze the thing, compare the thing,
and then make one small change that survives the loop.

## The Path

<ul class="doc-grid">
  <li>
    <a href="{{ '/hack/start-here/' | relative_url }}">Start here</a>
    <p>The first day: clone, build, run, read the scene ledger, and learn what the repository is trying to prove.</p>
  </li>
  <li>
    <a href="{{ '/hack/learn-c/' | relative_url }}">Learn C from Johnny</a>
    <p>A guided tour through plain-C modules that show resource loading, state machines, renderer contracts, and PS1 hardware boundaries.</p>
  </li>
  <li>
    <a href="{{ '/hack/port-to-a-new-platform/' | relative_url }}">Port to a new platform</a>
    <p>How to decide whether a target should run the interpreter, replay packs, or do something stranger.</p>
  </li>
  <li>
    <a href="{{ '/hack/visual-debugging/' | relative_url }}">Visual debugging</a>
    <p>The screenshot scripts, overlays, frame diffs, and image ledgers that made progress possible when printf was not safe.</p>
  </li>
  <li>
    <a href="{{ '/hack/memory-wars/' | relative_url }}">Memory wars</a>
    <p>The 2 MB RAM problem, VRAM pressure, SPU budget, CD layout, and why generated packs beat live bytecode on this machine.</p>
  </li>
  <li>
    <a href="{{ '/hack/performance-loop/' | relative_url }}">The performance loop</a>
    <p>Printf, telemetry, headless DuckStation, and the long-running experiments that turned hunches into numbers.</p>
  </li>
</ul>

## The Four Maps

If you are learning the project, keep these open:

- [Build docs]({{ '/docs/build/' | relative_url }}) for the Docker and PSn00bSDK setup.
- [Scene ledger]({{ '/scenes/' | relative_url }}) for visual-signoff status per scene; the [headless-perf battle card]({{ '/perf/' | relative_url }}) is the second ledger, sortable.
- [Source library]({{ '/source/' | relative_url }}) for the complete Markdown archive, including old failed plans.
- [Resource catalog]({{ '/resources/' | relative_url }}) for the bitmaps, ADS scripts, TTM animations, sounds, sprite banks, and generated packs.

The magazine version of this story lives in the [Lab]({{ '/lab/' | relative_url }}).
The reference-manual version lives in [Docs]({{ '/docs/' | relative_url }}).
This section is the shop notebook: what to read, what to run, what to inspect,
and what not to trust until a frame diff agrees.
