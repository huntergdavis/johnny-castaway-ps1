---
layout: page
title: The 63-scene grind
eyebrow: Lab · Retrospective
subtitle: The scene-by-scene validation ledger, from five signed off to the current grind.
description: A retrospective on the finite scene-by-scene validation grind behind Johnny Castaway PS1.
date: 2026-04-26
---

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

## The finite list

The port has 63 routed scenes in the current ledger.

That number is a gift. It means the project is finite. Not easy. Not short.
Finite.

Each scene has an ADS family and tag. Each scene has a foreground pack. Some
have tide variants. Some have raft variants. Some need holiday overlays to
avoid stomping pixels. Some are mostly standing animation and some are little
movies. They all eventually need the same thing: watch the PS1 output, compare
against reference, fix what is wrong, mark the row.

## Why the first five mattered

At the time this essay was first drafted, five scenes were validated:
`FISHING 1`, `FISHING 2`, `FISHING 3`, `FISHING 4`, and `FISHING 6`.
The live ledger has since moved through the rest; all 63 routed scenes
are validated as of `v0.7.0-ps1` and the headless-perf battle card is
now its own
[post-validation arc]({{ '/lab/from-87-to-99-5/' | relative_url }}).

Five out of sixty-three could sound small if you treat the scene count
like a progress bar. It is not the right read. The first validated
scene built the pipeline. The second proved the pipeline was not a
one-off accident. The next three proved the loop could survive
placement bugs, residual cleanup bugs, and real variant checks. The
remaining fifty-eight became work, but they were work inside a known
frame.

The difference between "nothing is known" and "the loop is known" is
enormous. The same difference shows up later in performance work: once
the first row of the matrix has a number, the rest are numbers in
context, not unknowns.

## The daily loop

The loop is:

1. Pick a scene.
2. Capture or refresh the host reference.
3. Generate or inspect the FG2 pack.
4. Wire the runtime if this scene needs a new path.
5. Build the disc.
6. Boot DuckStation.
7. Watch the scene.
8. Compare screenshots, logs, and timing.
9. Fix the obvious thing.
10. Repeat until the human review feels boring.

"Feels boring" is important. A validated scene should not feel lucky. It
should feel like you have watched it enough times that the next playthrough is
uneventful.

## The grind is the project

There was no grand final algorithm that validated the other fifty-eight.
There were tricks. There were pack improvements. There was more codegen.
But the work stayed one scene at a time, and the last cluster (the
foreground-only multi-view scenes) closed on `2026-05-05` with
`ACTIVITY 9` as the final row to flip green.

That is not disappointing. It is the nature of preservation work. The
only way to know a gag landed is to watch the gag — sixty-three times.

## Why the website mirrors it

The site has 63 scene pages for the same reason the code has 63 ledger rows.
The shape of the project is the shape of the source material. If a scene has a
status, it deserves a page. If a regtest reference exists, it deserves a page.
If a holiday has an emblem, it deserves a page.

The grind becomes navigable when every unit has a shelf.

## Cross-links

- [Scene ledger]({{ '/scenes/' | relative_url }}) — the live
  state of the 63 rows this essay is the back-story for.
- [Regtest reference cases]({{ '/archaeology/regtest-references/cases/' | relative_url }}) —
  the host-side baseline preserved per scene, captured before
  PS1 validation took over.
- [Regression as a lifestyle]({{ '/lab/regression-as-lifestyle/' | relative_url }}) —
  the methodology essay on why the daily loop runs at all.
- [Method]({{ '/about/method/' | relative_url }}) — the technical
  pipeline the grind grinds against.
- [`v0.7.0-ps1` release]({{ '/releases/#v070-ps1--complete-scene-validation' | relative_url }})
  — the milestone that capped this grind: 63 / 63 signed off
  under the [FISHING 1 bar]({{ '/docs/glossary/#fishing1-bar' | relative_url }}).
- [Performance battle card]({{ '/perf/' | relative_url }}) and
  [from-87-to-99-5 retrospective]({{ '/lab/from-87-to-99-5/' | relative_url }})
  — the second ledger that opened when this one closed.
- [v0.8.1: what the soak found that the matrix didn't]({{ '/lab/v081-mary4-freeze/' | relative_url }})
  — the kind of regression the validated-but-fragile post-grind
  era surfaces, and why the project added a soak loop alongside
  the per-commit matrix.
