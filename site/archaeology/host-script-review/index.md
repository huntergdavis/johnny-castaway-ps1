---
layout: page
title: Host-script review
eyebrow: Archaeology
subtitle: A bundled snapshot of the host-side identification pipeline as it stood when the bespoke-scene methodology took over.
description: The host-script review bundle preserves the cross-scene identification, capture-regression, and expectation reports for FISHING 1 and MARY 1 from the host validation pipeline that ran before the bespoke-scene loop.
---

The `host-script-review/` directory is a frozen snapshot of the host-side
review pipeline as it ran on 2026-04-04, captured against two scenes —
FISHING 1 and MARY 1 — and indexed into a self-contained set of HTML
viewers, JSON baselines, and regression reports. Twenty-six files total,
roughly 285 KB on disk. None of it is invoked by the current operator
workflow. It is preserved here so the methodology that produced the
[regtest reference set]({{ '/archaeology/regtest-references/' | relative_url }})
is documented in concrete terms rather than only described in the
[era timeline]({{ '/archaeology/timeline/' | relative_url }}).

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

## What's in there

The bundle indexes itself through `manifest.json`, which lists two
scenes (`fishing1`, `mary1`) and a set of "extras" — the cross-scene
HTML viewers and the semantic-truth payload. The
`verification-summary.txt` is a single line of key=value pairs that
records the full provenance contract: 41 paths, 34 file inputs, the
SHA256 of the input set, the depth distribution of the directory tree,
and a `status=PASS` flag. This was meant to be machine-checkable —
another script could read the line and confirm the bundle had not been
altered.

The two scenes have the same shape:

- `fishing1/frames/frame_NNNNN.bmp` — five sampled host frames at
  indices 0, 20, 40, 60, 80.
- `fishing1/frame-meta/frame_NNNNN.json` — per-frame metadata with the
  actor draws, BMP names, and visible-unique counts.
- `fishing1/review.html` — the per-scene viewer.

`mary1/` is the same with six frames at indices 0, 50, 100, 150, 200,
250. The frame at index 80 in FISHING 1 is the first one where
`actor_summary` records `johnny=1` and `bmp_names=["JOHNWALK.BMP"]`.
Before that the host pipeline saw zero actor draws — the johnny sprite
had not yet been emitted into the frame buffer. The MARY 1 capture
shows johnny first appearing at frame 100.

The cross-scene viewers (the "extras") are the meaty part:

- **`identification-review.html`** (51 KB) — the full identification
  report, walking each frame's top-1 match against the reference bank.
- **`capture-regression-review.html`** (4 KB) — the capture-regression
  comparison page.
- **`host-truth-compare.html`** (3 KB) — host-truth comparison output.
- **`expectation-report.html`** (3 KB) — the expectation-vs-actual
  report. For FISHING 1, frame 80's expectation says
  `expect_any_actor: true, expect_entities: ["johnny"], required_actor_bmps: ["JOHNWALK.BMP"]`;
  the report confirms the host pipeline observed exactly that.
- **`repro-compare.html`** (6 KB) — reproducibility comparison: did a
  re-run produce the same outputs as the baseline?
- **`index.html`** (5 KB) — the cover page that links the rest together.

Behind those HTML viewers are the JSON baselines and reports that
generated them: `frame-image-regression-baseline.json`,
`frame-meta-regression-baseline.json`,
`semantic-regression-baseline.json`, and the matching `*-report.json`
files. The `identification-selfcheck.json` (28 KB),
`identification-temporal.json` (5 KB), `identification-eval.json`,
`identification-partials.json`, `identification-challenges.json`, and
`identification-regression-floors.json` together describe the
reference-bank match quality at the point the bundle was captured.

## Why these were reviewed and saved

The host pipeline at this point was attempting to validate scene
identification in two senses at once. First: *given a captured frame,
can the system identify which scene it came from* by matching against a
reference bank? Second: *given an expected scene structure, do the
captured frames meet the expected actor presence at the expected frame
indices*?

The first question was tractable. The vision-classifier era's reference
bank contained 13,128 frames across all 63 scenes, and the
identification self-check recorded each scene's top-1 hit rate. FISHING
1's hit rate was 47% — confused with FISHING 2 about 40% of the time —
which was one of the lowest in the set. That low number is part of why
the project moved away from broad identification toward bespoke
per-scene capture. The
[vision-classifier artifacts]({{ '/archaeology/vision-artifacts/' | relative_url }})
preserve the broader identification record.

The second question — actor presence at expected indices — was the
useful part of the pipeline. It said, in writing, "at frame 80 the host
should have drawn JOHNWALK.BMP." That kind of frame-and-entity
expectation is what the regtest reference set still inherits, in its
state-hash-and-frame-count form. The host-script review is the more
elaborate ancestor.

Saving the bundle preserves both shapes — the broad identification
attempt and the narrow expectation check — at the moment when the
narrow form was about to win. Anyone reading the bundle six months
later sees the methodology halfway through its evolution.

## What it teaches about earlier methodology

The methodology was: *automate truth at scale*. Capture every frame
from every scene, build a reference bank, ask whether each fresh frame
matches the bank, ask whether the bank itself is internally consistent
(self-check), ask whether each scene's expected entities appear at the
expected frames. If all of those hold, the scene is verified.

The bundle shows that approach working in narrow scope — the two
sample scenes pass — and also reveals where it strained. A 41-path
manifest with twelve named path-map categories is a system trying to
keep many disjoint truth surfaces consistent. The
`verification-summary.txt` line is itself 8 KB long, summarising every
field of every category with hashes. That's a lot of bookkeeping to
keep eight HTML viewers and twenty-four JSON files coherent.

The path the project took instead was: per-scene capture, per-scene
metadata, per-scene comparison. Three files per scene. Sixty-two
scenes. The
[regtest reference set]({{ '/archaeology/regtest-references/' | relative_url }})
is what won, and the host-script review is what it replaced.

## Status: retired

These files are not regenerated. The scripts that produced them are not
on the current operator path. A future contributor wanting to re-run
the pipeline would need to find the host-side identification code at
the commit it last ran cleanly (around 2026-04-04) and walk it
forward — there is no shortcut. The bundle is preserved as a complete,
self-contained example of one validation philosophy, not as a tool
that's expected to keep running.

The documents are still readable in a browser and self-explanatory if
opened in order: start with `index.html`, follow into
`identification-review.html` for the full match report, then into
`expectation-report.html` for the per-frame entity check.

## Cross-links

- [Regtest documentation]({{ '/docs/regtest/' | relative_url }}) — the
  successor harness that replaced this style of validation.
- [Vision-classifier artifacts]({{ '/archaeology/vision-artifacts/' | relative_url }}) —
  the larger reference-bank context this bundle's identification
  outputs were measured against.
- [Era timeline]({{ '/archaeology/timeline/' | relative_url }}) —
  for the dates around when the host-script review approach was
  active and when it gave way to the bespoke per-scene line.

## Source on GitHub

[docs/ps1/archaeology/host-script-review/]({{ site.github_url }}/tree/main/docs/ps1/archaeology/host-script-review/)
