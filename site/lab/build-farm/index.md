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

- [The dev environment, photographed]({{ '/about/dev-environment/' | relative_url }}) — the farm's bottom-monitor panel in the same field of view as the rest of the workflow.
- [Scene ledger]({{ '/scenes/' | relative_url }}) — visual signoff (the [FISHING 1 bar]({{ '/docs/glossary/#fishing1-bar' | relative_url }})).
- [Performance battle card]({{ '/perf/' | relative_url }}) — second ledger; per-scene timing against the target frame budget, sortable + color-coded.
- [Performance plan]({{ '/docs/performance/' | relative_url }})
- [Performance experiment log]({{ site.github_url }}/blob/main/docs/ps1/performance-experiment-log.md)
- [Scene performance matrix]({{ site.github_url }}/blob/main/docs/ps1/performance-scene-matrix.csv)
- [From 87 to 99.5: the post-validation performance loop]({{ '/lab/from-87-to-99-5/' | relative_url }}) — retrospective on the optimization arc this farm fed.
- [v0.8.1: what the soak found that the matrix didn't]({{ '/lab/v081-mary4-freeze/' | relative_url }}) — the soak loop the farm runs alongside the per-commit matrix.
