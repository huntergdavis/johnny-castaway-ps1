---
layout: page
title: Era Timeline
eyebrow: Six months in five eras
subtitle: How the PS1 branch changed jobs without changing standards.
description: A vertical era strip covering the PS1 branch from embedded bootstrap (Oct 2025) through the foreground playback pivot (April 2026) that produced the first validated scene.
---

The PS1 branch changed shape several times between October 2025 and
April 2026. Each era had its own standards, its own tooling, and its
own definition of *verified*. This page walks them in order. The raw
source for the dates and dispositions is
[timeline.yaml]({{ site.github_url }}/blob/main/docs/ps1/archaeology/timeline.yaml).

The five eras overlap at their edges. That's accurate to the work —
the harness era didn't end at midnight on April 11, it tapered off as
the foreground pilot started looking like the answer.

<section class="chapter" markdown="1">
<span class="chapter-num">2025-10-18 → 2026-03-16</span>
<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

## Embedded port bootstrap and runtime survival

This is the era of "make the console boot." Cross-compilation through
PSn00bSDK 0.24 inside a Docker image, the CD layout XML, a CMake
configuration that targets MIPS, and the slow grind of CD-ROM I/O.
The graphics layer started at 700 lines and grew past 3,000 as
primitive ordering bugs, sprite alignment quirks, and heap stability
problems were tracked down one at a time. The acceptance bar was
modest: the game runs at native 640x480, scenes load from disc,
nothing crashes on long playback. Most of the foundational decisions
that the rest of the project still rests on — the resource hash
table, the LRU sprite cache, the dirty-rect bookkeeping — were made
during this stretch. It's preserved in the cleanup plan as
*foundational origin story*.
</section>

<section class="chapter" markdown="1">
<span class="chapter-num">2026-03-17 → 2026-03-28</span>
## Restore packs and the rollout surface

By March, the runtime worked but scene continuity didn't. Sierra's
original engine relies on *replay continuity* — playing prior scenes
to establish state for the current one. On the PS1, with CD seek
latency and 2 MB of main RAM, that wasn't viable. The response was an
offline pipeline: analyze every scene, write a *restore spec* that
declared which resources and dirty regions the scene needed, cluster
shared specs into 34 contracts, emit a 1000-line auto-generated
header (`ps1_restore_pilots.h`). The pipeline was real engineering
and it produced **25 of 63** scenes claimed verified by March 21.
Read carefully later, "verified" here meant "rendered without
crashing in a headless harness." That's not the same as "looks right
to a human." This era is preserved as historical research; the
authoring scripts still exist, the contracts file is still in the
tree, but it's no longer the active path.
</section>

<section class="chapter" markdown="1">
<span class="chapter-num">2026-03-29 → 2026-04-11</span>
## Validation and instrumentation maximalism

This is the period when the project tried to solve correctness with
tooling. HTML review surfaces, semantic-truth manifests, classifier
packs, OpenVINO-backed VLM review, headless regtest sweeps, agent
prompts written explicitly to *distrust* what the harness reported.
The validation log on **2026-03-29** claimed **63 of 63** scenes
verified. That number is now flagged in the project's own status
docs as a *false summit* — the harness was counting black frames and
ocean-only frames as valid. The skepticism was right; the
infrastructure built to express that skepticism is what survived.
The takeaway, hard-won: automation supports archaeology, not
certification. A scene is validated when a human watches it run and
listens to the SFX. Everything else is a hint.
</section>

<section class="chapter" markdown="1">
<span class="chapter-num">2026-04-04 → 2026-04-11</span>
## Binary library and regression archaeology

Overlapping with the late maximalism era, this stretch built a large
historical binary corpus and the bisection tooling to query it. The
question was: *when did `FISHING 1` start working, and when did it
stop?* The branch had broken and recovered multiple times in the
weeks before, and the binary library — every PS-EXE and CD image
worth keeping, indexed by commit and date — was the way to find the
exact onset commit for any given visual symptom. It was effective
debugging. It is also why the repo accumulated several gigabytes of
binaries that have since been pruned to manifests-only. The corpus
is preserved as searchable history, archived out of the primary
branch surface.
</section>

<section class="chapter" markdown="1">
<span class="chapter-num">2026-04-12 → 2026-04-22</span>
## Foreground playback pivot to validated baseline

The decisive era. On **2026-04-12**, two commits — `1c7481ca` ("Add
host foreground scene export pilot") and `77276e50` ("Add PS1
foreground playback pilot scaffold") — started the work that became
the project's primary methodology. The host runs the original
bytecode; the PS1 plays back the captured pixels and audio events.
Over the next ten days the pilot graduated from a single-scene
experiment to a working pipeline: full-frame `FISHING 1` capture
(`e24c2c23`, **04-13**), generic foreground runtime proven on
`FISHING 2` (`de1e0b89`, **04-14**), ocean base restored under the
foreground runtime (`44cec0a6`, **04-18**), scene-relative packs
(`5ea67b35`, **04-19**), animated shore waves (`74bb0c24`,
**04-21**), captured `DRAW_LINE` and `DRAW_PIXEL` events
(`8670d2a5`, **04-22**), holiday overlays wired through
(`67822af1`), night-mode and host CLI overrides (`33263ced`).
**`v0.3.5-ps1`** (`9448d49f`, **04-22**) declared `FISHING 1`
pixel-perfect across all variants. **`v0.3.6-ps1`** (`f2737253`,
later that same day) added the full SFX pipeline — host capture, VAG
encoding, PS1 playback, key-on alignment. That release is the first
real validated scene under the current bar. The methodology is
preserved as *authoritative current* and is what every subsequent
scene gets brought up under.
</section>

<section class="chapter" markdown="1">
<span class="chapter-num">2026-04-23 → 2026-05-06</span>
## Feature build-out, validation grind, performance baseline

After `v0.3.6-ps1` the work split into three overlapping arcs.
The first was feature build-out: the holidays
[codegen pipeline]({{ '/lab/35-holidays-codegen/' | relative_url }})
expanding 4 to 36, the pause menu reachable with Start, the
[SPI pad-poll fix]({{ '/lab/two-day-spi-bug/' | relative_url }}),
[story-loop walking]({{ '/releases/#v0420-ps1--story-loop-walking' | relative_url }})
that ended Johnny's between-scene teleport in **`v0.4.20-ps1`**,
[freeplay and the debug catalogs]({{ '/releases/#v050-ps1--freeplay-and-debug-mode' | relative_url }})
in **`v0.5.0-ps1`**, and the
[ocean-ambience SPU loop]({{ '/releases/#v060-ps1--ocean-ambience' | relative_url }})
in **`v0.6.0-ps1`**. The second was the
[63-scene grind]({{ '/lab/the-63-scene-grind/' | relative_url }})
through the v0.6.x bring-up cluster:
**`v0.7.0-ps1`** (**2026-05-05**) declared the [scene
ledger]({{ '/scenes/' | relative_url }}) complete — 63 of 63 scenes
pixel-perfect under the
[FISHING 1 bar]({{ '/docs/glossary/#fishing1-bar' | relative_url }})
across every applicable variant. The third was the post-validation
[performance loop]({{ '/lab/from-87-to-99-5/' | relative_url }})
that closed the headless matrix gap from `+17.4%` over target /
`87.1%` target speed at the v0.7.0 baseline to `+0.9%` over target /
`99.5%` target speed at **`v0.8.0-ps1`**, plus the
[stability follow-on]({{ '/lab/v081-mary4-freeze/' | relative_url }})
in **`v0.8.1-ps1`** that fixed a clean-rect pressure freeze the
per-commit matrix never reached. **`v0.8.2-ps1`** and
**`v0.8.3-ps1`** (**2026-05-07** / **2026-05-08**) closed the
VISITOR3 and WALKSTUF1 outliers — the matrix mean now sits at
`{{ site.release.perf_target_speed_pct }}%` target speed,
slightly under target across all 126 timing-bearing rows after the
missing-scene timing refresh.
The lab essays on each arc are the deep dives; this entry is
the chronological anchor.
</section>

---

The dated worklogs for every commit between these eras live at
[/devlog/]({{ '/devlog/' | relative_url }}). The full narrative
walkthrough is at [/archaeology/]({{ '/archaeology/' | relative_url }}).

## Related pages

- [About: History]({{ '/about/history/' | relative_url }}) —
  the narrative-prose companion to this dated chronology.
- [Releases]({{ '/releases/' | relative_url }}) — short
  notes on every tagged version named above.
- [Scene ledger]({{ '/scenes/' | relative_url }}) — the
  visual-signoff bar the v0.7.0 milestone certified.
- [Performance battle card]({{ '/perf/' | relative_url }}) —
  the second ledger that opened after v0.7.0.
- [Lab: the 63-scene grind]({{ '/lab/the-63-scene-grind/' | relative_url }})
  — retrospective on the bring-up era between v0.4 and v0.7.
- [Lab: from 87 to 99.5]({{ '/lab/from-87-to-99-5/' | relative_url }})
  — retrospective on the v0.8.0 performance arc.
- [Lab: v0.8.1 MARY 4 freeze]({{ '/lab/v081-mary4-freeze/' | relative_url }})
  — stability follow-on; the soak loop catching what the
  per-commit matrix didn't.
