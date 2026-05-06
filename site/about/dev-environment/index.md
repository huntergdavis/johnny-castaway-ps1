---
title: The dev environment
eyebrow: Photographed · 2026-05-06
subtitle: One screenshot of the workflow that built the post-validation performance baseline. Two LLM agents, one auto-dunker, DuckStation, the editor, the build farm, and bottom-monitor system telemetry.
description: A photograph of the development environment behind the Johnny Castaway PS1 fan port — Claude and Codex sub-agents, a fresh worktree, the dunking bird auto-poker, DuckStation running the latest build, the editor/terminal column, and bottom-monitor system telemetry.
image: /assets/img/dev-environment-2026-05-06-w1600.jpg
image_alt: "Multi-monitor PS1 development environment. Top row: dunking bird task list, two LLM agent prompt windows, DuckStation running Johnny Castaway. Middle row: vim editing C source, terminal with git diff and build output. Bottom row: system telemetry."
---

A photograph of the actual workflow as of `{{ site.release.tag }}`. Click the image for the full-resolution capture.

<a href="{{ '/assets/img/dev-environment-2026-05-06.png' | relative_url }}">
  <img src="{{ '/assets/img/dev-environment-2026-05-06-w1600.jpg' | relative_url }}"
       width="1600" height="1086"
       loading="lazy"
       decoding="async"
       alt="Multi-monitor PS1 development environment with dunking bird, two LLM agent windows, DuckStation, editor, terminal, and system telemetry." />
</a>

## What's in the frame

Reading the screenshot the way the work actually flows on this machine:

- **Top-left.** The [dunking bird]({{ '/lab/dunking-bird/' | relative_url }}) task list, mid-double-dunk. Two agent slots are queued; the bird taps a key whenever either model goes idle so the perf-iteration loop keeps moving instead of stalling on attention. The lab essay covers the why.
- **Top-middle.** Two LLM sub-agents accepting input. **Claude** on the left, **Codex** on the right. Each has its own working tree against its own branch — they don't share commits. Promotion goes through the same headless-perf gate every other change does ([promotion rule]({{ '/docs/glossary/#promotion-rule' | relative_url }})), no matter who drafted the patch.
- **Top-right.** **DuckStation** running the latest build off `jcreborn.bin` / `jcreborn.cue`. The frame caught FISHING 1 mid-cast — the canary scene whose `loop_vb` vs `target_vb` ratio is the first sanity check after any matrix-wide change.
- **Middle-left.** The editor. C source in `src/` — that frame is foreground-pilot code, the runtime that owns FG2/FGP3 replay. Not where most LLM-drafted patches land; the agents prefer scripts and YAML, the editor stays for hand-shaping the runtime.
- **Middle-right.** Terminal with `git diff` and `rebuild-and-let-run.sh` output. This is the loop that produces a new disc image, points DuckStation at it, and watches for a clean boot. About 90 seconds end-to-end for an unchanged build; longer when packs rebuild.
- **Bottom row.** The "fresh" worktree and the [build-farm]({{ '/lab/build-farm/' | relative_url }}) terminal — Docker matrix runs feeding the perf experiment log — plus bottom-monitor system telemetry (`btop`-style CPU / memory / network panels). The build farm runs unattended overnight; bottom monitor exists so the laptop fan and the experiment cadence stay in the same field of view.

## Why all of it on one screen

Every window above maps to one of the bars the project actually measures.

- The **dunking bird** keeps the agents productive when attention drifts — it's a hardware solution to the soft "are the agents still going" problem.
- **Claude + Codex** draft against the [experiment log]({{ '/docs/glossary/#experiment-log' | relative_url }}); their output gets graded by the matrix, not by which model wrote it.
- **DuckStation** is the visual signoff bar — pixel-perfect against host capture, the human-review gate that no LLM signs off in this project.
- **Editor + terminal** are the manual override path. When a patch needs to touch the runtime carefully, the human takes the keyboard.
- **Bottom monitor** is an honest look at whether the laptop is melting.

The two-ledger discipline ([visual signoff and headless perf stay separate]({{ '/about/method/#why-hybrid-won' | relative_url }})) shows up in the layout: the right side of the screen is what the player will see, the middle and left are what made it true, the bottom is the cost of keeping it true.

This is what most of the [post-validation performance loop]({{ '/lab/from-87-to-99-5/' | relative_url }}) looked like. Not a methodology diagram — a desk.
