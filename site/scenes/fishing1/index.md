---
layout: scene
title: FISHING 1 — Catches a starfish, throws it back
ads: FISHING
tag: 1
slug: fishing1
status: validated
description: "FISHING.ADS scene 1: Johnny fishes; the line hooks a starfish, which he throws back. Validated under the FISHING 1 bar."
image: /assets/img/fishing1-ps1-cast.png
image_alt: "FISHING 1 on PS1: Johnny casts a fishing line off the island, sun overhead, palm tree in frame."
image_width: 1127
image_height: 677
---

<figure class="scene-hero">
  <img src="{{ '/assets/img/fishing1-ps1-cast.png' | relative_url }}" width="1127" height="677" loading="lazy" decoding="async" alt="FISHING 1 running on PS1 hardware: Johnny mid-cast." />
  <figcaption>FISHING 1 on PS1 hardware. The reference scene.</figcaption>
</figure>
## What happens

Johnny walks down to the water with a fishing rod, casts the line, and after a beat reels in a starfish. He looks at it, shrugs, and tosses it back. Then he packs up and walks back inland.

This is the reference scene for the whole pipeline. If FISHING 1 looks right and sounds right across all four variant flags, the host capture pipeline and the PS1 playback engine are working as designed. Every other scene gets validated against this bar.

## Reference frames

These are the Sierra original — captured from the host build running the
real ADS/TTM bytecode against the original Sierra data files. The
host-vs-PS1 frame diff is the project's whole signoff loop: a scene
"clears the bar" when the PS1 frames match these references pixel-for-
pixel across every applicable variant.

<figure>
  <img src="{{ '/assets/img/fishing1-reference-arrival.png' | relative_url }}"
       width="640" height="480" loading="lazy" decoding="async"
       alt="FISHING 1 reference, host capture: Johnny arrives at the shoreline carrying a coiled fishing line." />
  <figcaption>Reference · arrival. Johnny walks onto the shore with the rod tucked, line coiled. From the Sierra original via host capture.</figcaption>
</figure>

<figure>
  <img src="{{ '/assets/img/fishing1-reference-cast.png' | relative_url }}"
       width="640" height="480" loading="lazy" decoding="async"
       alt="FISHING 1 reference, host capture: Johnny mid-cast with the rod arched and the line trailing into the ocean." />
  <figcaption>Reference · cast. Rod arched, line out. The PS1 build's hero shot at the top of this page is the same beat, replayed from the captured FG2 pack.</figcaption>
</figure>

<figure>
  <img src="{{ '/assets/img/fishing1-reference-catch.png' | relative_url }}"
       width="640" height="480" loading="lazy" decoding="async"
       alt="FISHING 1 reference, host capture: Johnny reels in a small red starfish that flies off the line back into the ocean." />
  <figcaption>Reference · catch. The red starfish in flight — the line that gives the scene its caption ("He catches a starfish. He throws it back.").</figcaption>
</figure>

## Validation

Validated as of `v0.3.6-ps1`. Variants exercised: night, low-tide, holiday, raft-stage.
This scene clears the [FISHING 1 bar]({{ '/about/method/' | relative_url }}) — pixel-perfect visuals plus synced SFX across every applicable variant.

## Pack identifiers

- ADS dispatch: <code>FISHING.ADS scene 1</code>
- Slug: <code>fishing1</code>
- High-tide pack: <code>FG/FISHING1.FG2</code>
- Low-tide pack: <code>FG/FISH1LOW.FG2</code>

## Variants

- **night** — Dusk/night palette swap (BOOTMODE `night 1`).
- **low-tide** — Tide state variant; different shoreline geometry (BOOTMODE `lowtide 1`).
- **holiday** — Holiday overlay variants — christmas, halloween, etc. (BOOTMODE `holiday N`).
- **raft-stage** — Cumulative raft-build state; raft sprite gains parts as the player progresses (BOOTMODE `raft-stage N`).

<figure>
  <img src="{{ '/assets/img/fishing1-ps1-night.png' | relative_url }}"
       width="1127" height="677" loading="lazy" decoding="async" alt="FISHING 1 running on PS1 at night: full moon, dark ocean, Johnny mid-cast at the moonlit shore." />
  <figcaption>FISHING 1 · night variant. Full moon, dark sky and water, the same cast pose against the moonlit shore.</figcaption>
</figure>

<figure>
  <img src="{{ '/assets/img/fishing1-ps1-raft.png' | relative_url }}"
       width="1127" height="677" loading="lazy" decoding="async" alt="FISHING 1 on PS1 with the raft built: Johnny stands next to a completed wood raft on the island shore." />
  <figcaption>FISHING 1 · raft-stage variant. Same scene with the cumulative raft-build state at completion.</figcaption>
</figure>

## Caption

This scene has on-screen caption text. Confidence: HIGH in the [caption audit]({{ '/docs/captions/' | relative_url }}).

<blockquote class="scene-caption">
Johnny goes fishing.<br />
He catches a starfish.<br />
He throws it back.
</blockquote>

## Notable runtime history

`FISHING 1` high-tide is the
[canary scene]({{ '/docs/glossary/#canary' | relative_url }}) for
the entire headless-perf matrix. Re-measured on every release; its
`loop_vb` vs `target_vb` ratio is the load-bearing reference frame
for "did this matrix-wide change just regress the easiest path."
The latest rollup at `{{ site.release.tag }}` lands at
`1068 / 1074 VBlanks`, `0.0%` public over target, `100.0%` public target
speed, `blocking_vb=2` — the raw signed CSV row is `-0.6%` / `100.6%`. That is
what "the FISHING 1 bar" means as a *timing* claim alongside the visual one.

The
[perf battle card]({{ '/perf/' | relative_url }}) shows the
per-variant rows; the
[perf retrospective]({{ '/lab/from-87-to-99-5/' | relative_url }})
walks through how this scene's stability paid for the rest of the
matrix's accepted optimizations — the
[promotion rule]({{ '/docs/glossary/#promotion-rule' | relative_url }})
requires that the canary doesn't move backward.
