---
title: The dev environment
eyebrow: Photographed · 2026-05-06
subtitle: One screenshot of the workflow that built the post-validation performance baseline. Dunking Bird, the fresh editor, two LLM sub-agents, DuckStation, and the bottom-monitor telemetry — all on KDE Plasma.
description: A photograph of the development environment behind the Johnny Castaway PS1 fan port — Hunter's own Dunking Bird auto-poker, the fresh editor, Claude and Codex sub-agents on separate worktrees, DuckStation running the latest build, and bottom-monitor system telemetry on KDE Neon (Debian backend).
image: /assets/img/dev-environment-2026-05-06-w1600.jpg
image_alt: "Multi-monitor PS1 development environment on KDE Plasma. Dunking Bird task list, the fresh editor on C source, two AI agent prompt windows (Claude and Codex), DuckStation running Johnny Castaway, and a bottom-monitor system telemetry panel."
image_width: 1600
image_height: 1086
---

A photograph of the actual workflow as of `{{ site.release.tag }}`. Click the image for the full-resolution capture.

<a href="{{ '/assets/img/dev-environment-2026-05-06.webp' | relative_url }}"
   title="Click for full-resolution (3816×2592 lossless WebP, ~4.1 MB; was 8.5 MB PNG)">
  <img src="{{ '/assets/img/dev-environment-2026-05-06-w1600.jpg' | relative_url }}"
       width="1600" height="1086"
       fetchpriority="high"
       decoding="async"
       alt="Multi-monitor PS1 development environment with Dunking Bird, the fresh editor, two AI agent windows (Claude and Codex), DuckStation, and bottom-monitor system telemetry on KDE Plasma." />
</a>

## What's in the frame

Six things, in roughly the order the eye walks the screen:

- **Dunking Bird.** Hunter's own program. The task list shows two agent slots queued — a *double-dunk*. Whenever either model goes idle, the bird taps a key to keep the perf-iteration loop moving instead of stalling on attention. The methodology essay is at [/lab/dunking-bird/]({{ '/lab/dunking-bird/' | relative_url }}).
- **The fresh editor.** Where the C source gets hand-shaped. The frame catches a chunk of foreground-pilot code, the runtime that owns FG2/FGP3 replay. Most LLM-drafted patches land in scripts and YAML; fresh stays for the runtime.
- **AI agent #1 — Claude.** A prompt window taking input against its own working tree. Drafts patches and prose, both of which get graded by the matrix, not by who wrote them.
- **AI agent #2 — Codex.** A second prompt window on a separate branch. The double-dunk only helps when the two agents aren't waiting on the same human.
- **DuckStation.** Running the latest build off `jcreborn.bin` / `jcreborn.cue`. The frame caught FISHING 1 mid-cast — the canary scene whose `loop_vb` vs `target_vb` ratio is the first sanity check after any matrix-wide change.
- **Bottom monitor.** A `btop`-style telemetry panel — CPU / memory / network, plus the [build-farm]({{ '/lab/build-farm/' | relative_url }}) Docker runs feeding the perf experiment log. Bottom monitor exists so the laptop fan and the experiment cadence stay in the same field of view.

The whole desktop is **KDE Plasma** on **KDE Neon** (Debian backend). Window tiling, virtual desktops, and the same six positions for every session — the workflow only works if the windows are where the muscle memory expects them.

## Why all of it on one screen

Every window above maps to one of the bars the project actually measures.

- **Dunking Bird** keeps the agents productive when attention drifts — a hardware solution to the soft "are the agents still going" problem.
- **Claude + Codex** each draft against the [experiment log]({{ '/docs/glossary/#experiment-log' | relative_url }}); their output gets graded by the matrix and the [promotion rule]({{ '/docs/glossary/#promotion-rule' | relative_url }}), not by which model wrote it.
- **DuckStation** is the visual signoff bar — pixel-perfect against host capture, the human-review gate that no LLM signs off in this project.
- **Fresh + terminals** are the manual override path. When a patch needs to touch the runtime carefully, the human takes the keyboard.
- **Bottom monitor** is an honest look at whether the laptop is melting.

The two-ledger discipline ([visual signoff and headless perf stay separate]({{ '/about/method/#why-hybrid-won' | relative_url }})) shows up in the layout: the right side of the screen is what the player will see, the middle and left are what makes it true, the bottom is the cost of keeping it true.

This is what most of the [post-validation performance loop]({{ '/lab/from-87-to-99-5/' | relative_url }}) looked like. Not a methodology diagram — a desk.

## Related pages

- [Lab: dunking-bird]({{ '/lab/dunking-bird/' | relative_url }})
  — the methodology essay behind the auto-poker visible in
  the screenshot's task list.
- [Lab: build-farm]({{ '/lab/build-farm/' | relative_url }})
  — the 24/7 Docker-runner machinery the bottom-monitor
  panel watches.
- [Lab: the LLM pass]({{ '/lab/llm-pass/' | relative_url }})
  — methodology for the two LLM sub-agent windows in the
  middle of the frame.
- [Lab: from 87 to 99.5]({{ '/lab/from-87-to-99-5/' | relative_url }})
  — the post-validation performance retrospective this
  workflow drove.
- [Docs: AI sub-agents]({{ '/docs/agents/' | relative_url }})
  — the honest accounting of where the agents helped and
  where they didn't.
- [About: Method]({{ '/about/method/' | relative_url }})
  — the two-ledger discipline (visual signoff + headless
  perf) the screen layout above maps to.
