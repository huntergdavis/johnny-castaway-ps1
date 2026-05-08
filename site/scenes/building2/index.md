---
layout: scene
title: BUILDING 2 — Lilliputians take over the sandcastle and launch airplanes
ads: BUILDING
tag: 2
slug: building2
status: validated
description: "BUILDING.ADS scene 2: Johnny builds a sandcastle; tiny lilliputians take it over and launch airplanes from it. Validated 2026-05-08."
---

Validated on 2026-05-05 under the [FISHING 1 bar]({{ '/docs/glossary/#fishing1-bar' | relative_url }}). The high/low packs were
regenerated through the generic normal/far-left/far-right foreground-only
multi-view stitch, with the persistent full-host sandcastle injected so the
red flag and planes originate from the correct base. The pack stays in FGP3
residual form so disappeared Lilliputian, plane, sand, and splash pixels clean
up explicitly.

## Pack identifiers

- ADS dispatch: <code>BUILDING.ADS scene 2</code>
- Slug: <code>building2</code>

## What this scene is

Johnny builds a sandcastle and tiny lilliputians take it over, turning it into a runway and launching airplanes from it. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; the earlier "roasts a boot" caption-mapping guess was wrong.

### How this scene gets validated

The same way every scene does: under the
[FISHING 1 bar]({{ '/docs/glossary/#fishing1-bar' | relative_url }}).

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

## Notable runtime history

`BUILDING 2` straddles the
[perf battle card]({{ '/perf/' | relative_url }})'s orange and yellow
bands — high tide at orange (`97.6%`) and low tide at yellow
(`94.3%`), close to but not yet at native [target speed]({{ '/docs/glossary/#target-speed' | relative_url }}). Its
[clean-rect]({{ '/docs/glossary/#clean-rect' | relative_url }})-heavy
Lilliputian-sandcastle frames are explicitly named in the
[post-validation perf retrospective]({{ '/lab/from-87-to-99-5/' | relative_url }})
as part of the wide-action surface still finishing its
[prefetch-relief]({{ '/docs/glossary/#prefetch-window' | relative_url }})
and [stream-window]({{ '/docs/glossary/#stream-window' | relative_url }})
work. The full-host sandcastle injection that the validated pack
relies on (red flag + planes originating from the correct base)
also forces a larger residual cleanup table, which is the underlying
reason the rows haven't graduated to green yet.
