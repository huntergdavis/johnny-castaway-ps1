---
layout: page
title: Binary formats reference
eyebrow: Reference
subtitle: Every on-disc and on-wire format the project defines, in one place.
description: Reference manuals for the project-internal binary and JSON formats that bridge the host capture build and the PS1 build of Johnny Castaway — pack payload, pack manifest, dirty-region template, transition prefetch schema, and the SDL compat lite contract.
---

A labor of love by Hunter Davis. The pages below describe the file formats the
project itself defines — not Sierra's original `RESOURCE.MAP` /
`RESOURCE.001`, which have their own archaeology elsewhere, but the formats
the host capture build *writes* and the PS1 build *reads*. They exist because
the PS1 cannot run Sierra's ADS bytecode at native speed, and the renderer
cannot afford to recompute scene state every frame from a 2x CD drive. So a
host-side tool runs each scene once, captures the result, and bakes it into a
pre-decided binary the console can replay deterministically. Every format on
this page is part of that bridge.

There are five of them. Two are on-disc binary (the FG2 pack payload, the
dirty-region template). Two are JSON sidecars used by the build pipeline (the
pack manifest, the transition prefetch schema). One is a written contract,
not a file format — the SDL compat lite specification, which describes the
narrow graphics surface gameplay code is allowed to depend on regardless of
which build it lives in. They are documented together because they all
answer the same question from different angles: *what does the host build
hand the PS1 build, and in what shape.*

These docs exist because reading any of these files in a hex editor at 3 AM
is a real activity on this project. If a regtest run fails because a pack's
sound-event table has the wrong offset, the only way to find out is to open
the `.PAK`, count bytes, and compare them to what the spec says. Without a
spec, you guess. With a spec, you check. Most of the entries below were
written after a bug.

If you paid for this, you were cheated. Open source and free.

## What's in /docs/formats/

<ul class="doc-grid">
  <li>
    <a href="{{ '/docs/formats/pack-payload/' | relative_url }}">FG2 pack payload</a>
    <p>The on-disc binary form of one captured ADS scene. A 40-byte header, a palette, an entry table, and a stream of base-and-diff frames, padded to a 2048-byte CD sector. Read at runtime by <a href="{{ site.github_url }}/blob/main/src/foreground_pilot/foreground_pilot.c"><code>foreground_pilot.c</code></a>.</p>
  </li>
  <li>
    <a href="{{ '/docs/formats/pack-manifest/' | relative_url }}">FG2 pack manifest</a>
    <p>The JSON sidecar that travels with each pack. Lists scene indices, ADS names, resource memberships, peak memory, and prefetch hints. Consumed by the CD image builder and by the regtest harness for sanity checks.</p>
  </li>
  <li>
    <a href="{{ '/docs/formats/dirty-region/' | relative_url }}">Dirty region template</a>
    <p>The renderer's bookkeeping cheat-sheet. For each TTM, which rectangles get touched, which need a clean-background restore, and which are clear-screen heavy. Computed offline from TTM bytecode; consumed by the PS1 dirty-row renderer.</p>
  </li>
  <li>
    <a href="{{ '/docs/formats/transition-prefetch/' | relative_url }}">Transition prefetch schema</a>
    <p>The post-processed planner that says <em>which pack to start streaming next</em>, given the current pack and the analyzer's story-order graph. Ranks edges by added bytes and working set so the foreground pilot has somewhere to start.</p>
  </li>
  <li>
    <a href="{{ '/docs/formats/sdl-compat-lite/' | relative_url }}">SDL compat lite</a>
    <p>Not a file format — a written contract. The narrow set of <code>gr*</code> functions gameplay code is allowed to call. Both the host SDL2 build and the PS1 PSn00bSDK build implement this surface. Everything else is implementation detail.</p>
  </li>
</ul>

## How they fit together

The pipeline runs in one direction: host build, then PS1 build, then disc.

1. The host capture build runs a scene through Sierra's ADS interpreter (with
   real SDL2 graphics) and records every frame's pixel delta into an FG2
   payload. It writes the manifest beside it. It optionally derives a
   dirty-region template from the TTM bytecode for the same scene.
2. The pack manifest and the prefetch schema are read by post-processing
   tools that decide pack granularity and likely pack-to-pack transitions.
3. [`mkpsxiso`]({{ '/docs/glossary/#mkpsxiso' | relative_url }}) reads [`cd_layout.xml`]({{ site.github_url }}/blob/main/config/ps1/cd_layout.xml) and stamps the chosen FG2 packs onto
   `jcreborn.bin` at deterministic sector offsets.
4. At runtime, `foreground_pilot.c` calls `CdSearchFile` on `FG\\<NAME>.FG2`,
   reads the 40-byte header, walks the entry table, and presents one diff
   frame per VBlank window. Sound events fire by `sourceFrame`. Dirty-region
   restore happens off the offline template when one exists.

The formats describe the boundary of every step. None of them is final — the
manifest schema is on `schema_version: 1` and explicitly says "draft". The
pack payload is "the live on-disc pilot format ... even though it is still
not the final long-term binary format." Both files read like that on
purpose. Final shapes get written when the runtime stops finding new
constraints.

## Where the source documents live

These pages wrap the canonical schema files under
[`docs/ps1/research/`]({{ site.github_url }}/tree/main/docs/ps1/research).
Each sub-page links to the file or files it was derived from. If something on
a page disagrees with the source doc, the source doc wins — these pages are
re-narrations, not the spec.

## Related references

- [Build & toolchain]({{ '/docs/build/' | relative_url }}) — where the disc
  image and the FG2 packs end up linked together.
- [Regression testing]({{ '/docs/regtest/' | relative_url }}) — the harness
  that consumes the pack manifest for cross-check verification.
- [API mapping]({{ '/docs/api/' | relative_url }}) — the SDL2 ↔ PSn00bSDK
  table the SDL compat lite contract sits on top of.
