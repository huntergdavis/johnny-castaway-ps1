---
layout: page
title: The 24/7 Build Farm
eyebrow: Lab . Infrastructure
subtitle: Docker builds, headless DuckStation, and the boring machinery that lets one person test a PS1 port continuously.
description: "A short lab note on the Johnny Castaway PS1 build farm: Dockerized toolchains, headless emulator runs, performance logs, and why the project treats regression testing as infrastructure."
date: 2026-04-29
---

The build farm is not a rack of machines. It is a repeatable loop:
Dockerized PS1 builds, generated CD images, headless DuckStation runs,
log parsing, and a rule that every accepted performance change becomes
the next baseline.

The useful part is discipline. A candidate optimization is not "faster"
because it feels faster once. It is faster when the headless harness
shows fewer VBlanks, no new blocking reads, no worse scene-end metrics,
and no visual regression when the human pass catches up. Rejected tests
still get logged because a failed idea can become valid after a later
pack-format or scheduler change.

## Current Loop

- Build the PS1 executable and CD image in the Docker toolchain.
- Run headless DuckStation against the selected scene or scene matrix.
- Parse `JCPERF` and `JCPERF2` logs into VBlank, CD, compose, upload,
  wait, and blocking counters.
- Promote only changes that beat the current baseline without violating
  deterministic playback.
- Commit accepted changes separately, and record rejected experiments in
  the log.

## Pointers

- [Scene ledger + perf battle card]({{ '/scenes/' | relative_url }})
- [Performance plan]({{ '/docs/performance/' | relative_url }})
- [Performance experiment log]({{ site.github_url }}/blob/main/docs/ps1/performance-experiment-log.md)
- [Scene performance matrix]({{ site.github_url }}/blob/main/docs/ps1/performance-scene-matrix.csv)
