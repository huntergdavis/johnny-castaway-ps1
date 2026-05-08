---
layout: page
title: Docs
eyebrow: Reference manuals
subtitle: Build, captions, holidays, pause menu, freeplay, story-loop walks, regtest, scripted input, performance, hardware, audio, infrastructure, file formats, AI sub-agents, vision-classifier, the SDL2 → PSn00bSDK API mapping, dev workflow, and a glossary. The technical surface of the PS1 port.
description: Reference manuals for the Johnny Castaway PS1 port — build, captions, holidays, pause menu, freeplay, story-loop walks, regtest, scripted input, performance, hardware, audio, infrastructure, file formats, AI sub-agents, vision-classifier, the SDL2 → PSn00bSDK API mapping, the per-scene workflow, and a glossary of project vocabulary.
---

A labor of love by Hunter Davis. The pages below are reference manuals for the
PlayStation 1 build of *Johnny Castaway*. They describe how the executable is
compiled, what each subsystem does, and what the author types when bringing up
a scene. They are not tutorials. They are not a community forum. There is no
contributor onboarding here — see the [FAQ]({{ '/faq/' | relative_url }}) for
the project's stance on contributions.

If you paid for this, you were cheated. Open source and free.

## What's in /docs/

<ul class="doc-grid">
  <li>
    <a href="{{ '/docs/build/' | relative_url }}">Build &amp; toolchain</a>
    <p>PSn00bSDK 0.24, the Docker dev image, the cmake invocation, and the mkpsxiso step that produces <code>jcreborn.bin</code> + <code>jcreborn.cue</code>. Includes the exact docker commands the author uses.</p>
  </li>
  <li>
    <a href="{{ '/docs/captions/' | relative_url }}">Closed captions</a>
    <p>How the caption corpus, the ADS-tag map, and the on-screen text band fit together. Includes the 30 HIGH / 21 MED / 12 LOW confidence breakdown from the 2026-04-26 audit.</p>
  </li>
  <li>
    <a href="{{ '/docs/holidays/' | relative_url }}">Holidays</a>
    <p>From four shipped holidays to thirty-six. Movable feasts via Meeus / Zeller, no expiring tables. Codegen pipeline from <code>holidays.yml</code> to a C struct array.</p>
  </li>
  <li>
    <a href="{{ '/docs/pause-menu/' | relative_url }}">Pause menu</a>
    <p>State machine, eleven sub-screens (Scene Set, Freeplay Options, Controls, World Options, Holidays, Set Island Position, Accessibility, Sound Test, System, Set Time/Date, Set RNG Seed), Freeplay entry/exit, and the shared font atlas used by captions.</p>
  </li>
  <li>
    <a href="{{ '/docs/freeplay/' | relative_url }}">Freeplay mode</a>
    <p>The runtime-driven scene where the player drives Johnny instead of watching him: controls, gag/visitor debug catalogs, world toggles, frog loading transitions, and long-run memory rules.</p>
  </li>
  <li>
    <a href="{{ '/docs/walks/' | relative_url }}">Story-loop walks</a>
    <p>How Johnny stops teleporting between scenes — the walk_pilot/walk_render split, Sierra's six-spot route table, the persistent clean buffer, and the runtime invariants (waves, palm-tree occlusion, holiday overlay re-stamping).</p>
  </li>
  <li>
    <a href="{{ '/docs/regtest/' | relative_url }}">Regression testing</a>
    <p>Headless DuckStation in Docker. Frame PNGs every <em>N</em> frames, SHA256 state hashes, telemetry overlay decode, and the per-scene wrapper.</p>
  </li>
  <li>
    <a href="{{ '/docs/scripted-input/' | relative_url }}">Scripted input harness</a>
    <p>Pad scripts embedded into the PS1 build: deterministic menu routes, Freeplay repros, marker-aligned screenshots, and the generated menu help guide.</p>
  </li>
  <li>
    <a href="{{ '/docs/api/' | relative_url }}">API mapping (SDL2 → PSn00bSDK)</a>
    <p>Every SDL2 symbol the upstream engine called, mapped to the PSn00bSDK call that replaced it. The bridge between the host-capture build and the PS1 build.</p>
  </li>
  <li>
    <a href="{{ '/docs/dev-workflow/' | relative_url }}">Development workflow</a>
    <p>The author's per-scene runbook: capture on host, encode to <code>.FG2</code>, replay on PS1, take a screenshot, validate. Honest about how long it takes and how often it breaks.</p>
  </li>
  <li>
    <a href="{{ '/docs/performance/' | relative_url }}">Performance</a>
    <p>Reference manual for the headless-perf battle card: how <code>loop_vb</code> / <code>target_vb</code> are measured, what each column means, the column-by-column glossary, and the experiment-log discipline.</p>
  </li>
  <li>
    <a href="{{ '/docs/hardware/' | relative_url }}">Hardware</a>
    <p>The PS1 envelope this port runs inside — 33.8688 MHz MIPS, 2 MB RAM, 1 MB VRAM, 512 KB SPU, no FPU, 2× CD drive at 300 KB/s. Why every constraint shows up in the build.</p>
  </li>
  <li>
    <a href="{{ '/docs/audio/' | relative_url }}">Audio</a>
    <p>SPU layout, VAG sample bank, the round-robin voice allocator, the captured <code>0xC051 PLAY_SAMPLE</code> events, and the ocean-ambience loop on its dedicated voice.</p>
  </li>
  <li>
    <a href="{{ '/docs/formats/' | relative_url }}">File formats</a>
    <p>The five spec pages behind the build chain: the FG2 / FGP3 pack payload, the pack-manifest sidecar, the dirty-region template, the transition-prefetch schema, and the SDL-compat-lite shim spec.</p>
  </li>
  <li>
    <a href="{{ '/docs/infrastructure/' | relative_url }}">Infrastructure</a>
    <p>The Docker dev image, the regtest container, the release script, the build-farm pattern, and what's still done by hand on every release.</p>
  </li>
  <li>
    <a href="{{ '/docs/agents/' | relative_url }}">AI sub-agents</a>
    <p>The honest accounting of where LLM sub-agents helped on this project — caption corpus drafts, holiday-emblem sprite primitives, long-form prose drafting — and what they explicitly didn't do.</p>
  </li>
  <li>
    <a href="{{ '/docs/vision/' | relative_url }}">Vision-classifier</a>
    <p>The pixel-vs-reference comparison layer that runs on top of the regtest harness: per-frame diffs, scene-state classification, and how it feeds the visual signoff bar.</p>
  </li>
  <li>
    <a href="{{ '/docs/glossary/' | relative_url }}">Glossary</a>
    <p>The specific technical terms the rest of the docs use without scaffolding — FG2 packs, ADS, TTM, dirty-rect bookkeeping, FntFlush, the FISHING 1 bar, drawCredits. Grouped by area, not alphabetical.</p>
  </li>
</ul>

## The Other Maps

The reference manuals are not the whole archive. The site now has four
neighboring maps that make the rest of the work findable:

<ul class="doc-grid">
  <li>
    <a href="{{ '/source/' | relative_url }}">Source library</a>
    <p>Every Markdown file outside the website tree gets a public page: active manuals, generated research, old plans, status notes, and archaeology.</p>
  </li>
  <li>
    <a href="{{ '/resources/' | relative_url }}">Resource catalog</a>
    <p>The asset shelf: BMP, ADS, TTM, SCR, VAG, PSB, and FG2 files indexed by type with source links.</p>
  </li>
  <li>
    <a href="{{ '/archaeology/regtest-references/cases/' | relative_url }}">Regtest case shelf</a>
    <p>Sixty-three host baselines, one page per captured reference, with boot string, frame count, state hash, and source artifacts.</p>
  </li>
  <li>
    <a href="{{ '/hack/' | relative_url }}">Curious hacker guide</a>
    <p>A learning path for reading the C, porting to another machine, using visual debugging, and understanding the memory budget.</p>
  </li>
</ul>

## Where this content comes from

These pages are derived from the source documents under
[`docs/ps1/`]({{ site.github_url }}/tree/main/docs/ps1) in the repository.
Each sub-page links directly to the file or files it was built from. If you
want to view-source any reference manual, the GitHub tree is the canonical
copy; `/docs/` is just a re-organized read of it.

For the high-level "what this project is" narrative see
[About]({{ '/about/' | relative_url }}). For per-scene status see the
[scene ledger]({{ '/scenes/' | relative_url }}). For day-by-day worklog see
the [devlog]({{ '/devlog/' | relative_url }}). For methodology essays see
the [Lab]({{ '/lab/' | relative_url }}).
