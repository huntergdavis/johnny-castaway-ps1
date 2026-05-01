---
layout: page
title: Start Here
eyebrow: Curious hacker path
subtitle: The first day with the Johnny Castaway PS1 codebase.
description: First-day guide for reading, building, and navigating the Johnny Castaway PS1 port.
---

The fastest way into this project is to stop trying to understand every file
up front. Build it, run it, then use the running program as the index.

Start with the [build guide]({{ '/docs/build/' | relative_url }}). It explains
the Docker image, PSn00bSDK 0.24, the `cmake` invocation, and the `mkpsxiso`
step that creates the disc image. Then open the [play page]({{ '/play/' | relative_url }})
and the [scene ledger]({{ '/scenes/' | relative_url }}). Those three pages tell
you what the project does today, what it claims to do, and which claims are
actually validated.

## The First Loop

1. Build the PS1 disc image.
2. Boot it in DuckStation.
3. Pick `FISHING 1`, because it is the reference bar.
4. Toggle night, tide, holiday, raft, and captions from the pause menu.
5. Run the [scripted input harness]({{ '/docs/scripted-input/' | relative_url }}) once so the menu route becomes screenshots instead of memory.
6. Read the per-scene page for [FISHING 1]({{ '/scenes/fishing1/' | relative_url }}).
7. Read the [regtest docs]({{ '/docs/regtest/' | relative_url }}) and understand why "it looked fine once" does not count.

That loop is small enough to finish and rich enough to teach the whole shape
of the project. You will touch the renderer, the input path, foreground pack
replay, audio cues, overlays, captions, and the runtime options surface.

## What to Read Next

The project's polished explanation is [About / Method]({{ '/about/method/' | relative_url }}).
The raw archive is the [source library]({{ '/source/' | relative_url }}), which
wraps every Markdown file outside the website. The files worth reading early:

- [PS1 current status]({{ '/source/docs/ps1/current-status/' | relative_url }})
- [PS1 scene pipeline status]({{ '/source/docs/ps1/scene-status/' | relative_url }})
- [PS1 development workflow]({{ '/source/docs/ps1/development-workflow/' | relative_url }})
- [PS1 regression test harness]({{ '/source/docs/ps1/regtest-harness/' | relative_url }})
- [PS1 scripted input harness]({{ '/source/docs/ps1/scripted-input-harness/' | relative_url }})
- [Website master plan]({{ '/source/docs/ps1/website-plan/' | relative_url }})

Read the old research too, but read it as archaeology. The project has
several false summits preserved on purpose, including the 63/63 host-harness
moment that did not mean what it first seemed to mean.

## What Not to Do First

Do not begin by changing scene routing. Do not begin by rewriting the pack
format. Do not begin by cleaning up style. The first useful contribution to
your own understanding is a local run that you can compare against a known
state. Once you can reproduce `FISHING 1`, the rest of the codebase has a
scale.
