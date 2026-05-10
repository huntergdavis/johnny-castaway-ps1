---
layout: scene
title: WALKSTUF 1 — Parties on a yacht, comes back drunk, passes out
ads: WALKSTUF
tag: 1
slug: walkstuf1
status: validated
last_verified: "2026-05-05"
description: "WALKSTUF.ADS scene 1: Johnny parties on a passing yacht, returns to the island drunk, and passes out. Validated 2026-05-08."
image: /assets/img/walkstuf1-ps1-yacht-party.png
image_alt: "WALKSTUF 1 on PS1 at night: a white yacht has pulled up next to the island with party-goers visible on its deck, the moon overhead and Johnny's island and palm tree on the right."
image_width: 961
image_height: 720
---

<figure class="scene-hero">
  <img src="{{ '/assets/img/walkstuf1-ps1-yacht-party.png' | relative_url }}"
       width="961" height="720"
       fetchpriority="high"
       decoding="async"
       alt="WALKSTUF 1 on PS1 at night: a white yacht has pulled up next to the island with party-goers visible on its deck, the moon overhead and Johnny's island and palm tree on the right." />
  <figcaption>
    WALKSTUF 1 on PS1, captured during the v0.8.4-ps1
    <a href="{{ '/lab/chapter-select-grind/' | relative_url }}">chapter-select grind</a>.
    The yacht-party setup beat: a yacht has pulled up beside the
    island with the party crowd visible on deck, the moment before
    Johnny boards and joins them. (The next beat returns him drunk
    to the island, then he passes out on the sand. The on-PS1 loop
    overturned the original
    <a href="{{ '/docs/captions/' | relative_url }}">caption audit</a>'s
    "jogs around the island" guess for this scene — that gag is
    <a href="{{ '/scenes/walkstuf3/' | relative_url }}">WALKSTUF 3</a>.)
    Engineering retro: WALKSTUF 1 is the orange-band perf scene
    on <a href="{{ '/perf/' | relative_url }}">/perf/</a>
    (95.6% / 95.8% target speed) — both packs were converted to
    compact FGP3/v4 restore-minus-current in
    <a href="{{ '/releases/#v083-ps1--walkstuf1-compact-foreground-performance' | relative_url }}"><code>v0.8.3-ps1</code></a>
    while preserving pack footprints, LBAs, and the PS-EXE
    bucket; high blocking dropped from 275 → 85 VBlanks and low
    from 270 → 86. The high/low packs also use a range-gated
    Johnny-bbox hold (frames 63–165, glitch threshold 1000) so
    Johnny stays drawn in his rest position while the boat scene
    plays out.
  </figcaption>
</figure>

Validated 2026-05-05. Visual + audible signoff after re-exporting
high/low packs with -500/+300 stitch positions and a range-gated
Johnny-bbox hold (frames 63-165, glitch threshold 1000) so Johnny
stays drawn in his rest position while the boat + mermaid scene plays
out and the foreground-only diff drops him.

## Pack identifiers

- ADS dispatch: <code>WALKSTUF.ADS scene 1</code>
- Slug: <code>walkstuf1</code>

## What this scene is

A yacht pulls up; Johnny boards and parties with the crowd on board. He comes back to the island visibly drunk, staggers a few steps, and passes out on the sand. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; the earlier "jogs around the island" caption-mapping guess was wrong (no jogging in the on-PS1 pack).

### How this scene gets validated

The same way every scene does: under the FISHING 1 bar.

A host-side Johnny Reborn capture/export pass produces a
base-diff `.FG2` foreground pack and a JSONL of sound events. The PS1
build replays that pack at native resolution through every variant the
original game randomized between (night, low-tide, holiday overlays,
raft-stage progress where applicable). The
[regtest harness]({{ '/docs/regtest/' | relative_url }}) checks that the
visuals come out frame-identical and the SFX cues land on the same
ticks. Once that holds across all applicable variants, the scene moves
to `validated` and a row turns green in the
[ledger]({{ '/scenes/' | relative_url }}).
