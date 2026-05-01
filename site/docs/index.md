---
layout: page
title: Docs
eyebrow: Reference manuals
subtitle: Build, captions, holidays, regtest, API mapping. The technical surface of the PS1 port.
description: Reference manuals for the Johnny Castaway PS1 port — build, captions, holidays, pause menu, freeplay, regtest, API mapping, and the per-scene workflow.
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
    <p>State machine, compact sub-screens, Freeplay entry/exit, World Options, Sound Test, Accessibility, System, and the shared font atlas used by captions.</p>
  </li>
  <li>
    <a href="{{ '/docs/freeplay/' | relative_url }}">Freeplay mode</a>
    <p>The runtime-driven scene where the player drives Johnny instead of watching him: controls, gag/visitor debug catalogs, world toggles, frog loading transitions, and long-run memory rules.</p>
  </li>
  <li>
    <a href="{{ '/docs/regtest/' | relative_url }}">Regression testing</a>
    <p>Headless DuckStation in Docker. Frame PNGs every <em>N</em> frames, SHA256 state hashes, telemetry overlay decode, and the per-scene wrapper.</p>
  </li>
  <li>
    <a href="{{ '/docs/api/' | relative_url }}">API mapping (SDL2 → PSn00bSDK)</a>
    <p>Every SDL2 symbol the upstream engine called, mapped to the PSn00bSDK call that replaced it. The bridge between the host-capture build and the PS1 build.</p>
  </li>
  <li>
    <a href="{{ '/docs/dev-workflow/' | relative_url }}">Development workflow</a>
    <p>The author's per-scene runbook: capture on host, encode to <code>.FG2</code>, replay on PS1, take a screenshot, validate. Honest about how long it takes and how often it breaks.</p>
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
