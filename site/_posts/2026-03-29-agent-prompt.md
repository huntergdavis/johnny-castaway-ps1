---
layout: post
title: "PS1 Scene Validation And Debug Prompt"
date: 2026-03-29
editor_note: "The mission brief handed to a debugging agent the day the validation push for all 63 scenes started. Plain instructions, no rhetoric."
source_path: docs/ps1/research/AGENT_PROMPT_2026-03-29.md
description: "Mission brief for the PS1 scene-validation debugging agent: validate 63 PS1 scenes against the canonical Linux reference, with auditable artifacts."
---

You are working in the `jc_reborn` repository on the PS1 runtime and test harness.

Your mission is not to produce plausible-looking runs. Your mission is to get to `63` actually validated PS1 story scenes against the canonical Linux/reference output, with trustworthy comparison artifacts a human can review.

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

## Primary Goal

Reach a state where all `63` target PS1 story scenes can be:

1. booted deterministically
2. captured reproducibly
3. compared against the canonical reference set
4. reviewed visually in generated HTML
5. marked validated only when they actually match scene content, not just because they share black frames, title frames, or empty/ocean fallback frames

Do not count:
- black-screen agreement
- title-screen agreement
- empty-ocean agreement
- “close enough” when no scene content launched

The standard is validated scene content.

## Source Of Truth

The baseline source of truth is already present in this repo:

- reference root:
  - `repo:/regtest-references`

This contains the canonical Linux/reference outputs for the scene set. Treat that as authoritative.

Important related files:

- reference scene list:
  - `repo:/config/ps1/regtest-scenes.txt`
- single-scene runner:
  - `repo:/scripts/regtest-scene.sh`
- batch compare runner:
  - `repo:/scripts/compare-reference-batch.sh`
- compare renderer:
  - `repo:/scripts/render-compare-timeline.py`
- single-run review renderer:
  - `repo:/scripts/render-regtest-run.py`

## Current Expectations

There is still work to do on both:

- the harness
- the actual PS1 scene runs

Assume the current harness is useful but not fully solved.

In particular, there is an active suspicion that the harness may be stopping too early relative to the real PS1 boot/title lead-in.

Working hypothesis to test:

- scene timing windows may need to be increased significantly
- a good first hypothesis is that effective scene timing may need roughly `+35 seconds` of extra boot allowance before valid scene comparison begins

Do not assume that hypothesis is true. Test it.

## Comparison Requirements

For every serious run, produce artifacts that a human can inspect.

At minimum, each scene run should leave:

1. `result.json`
2. `review.html`
3. raw captured frames
4. if reference-compared, `compare.json`
5. if reference-compared, `compare.html`

The human must be able to open the HTML and confirm whether the run contains real scene content.

## How To Compare Correctly

### Single PS1 run review

Use:

- `repo:/scripts/regtest-scene.sh`

This now emits:

- `result.json`
- `review.html`

The review page is for raw PS1 inspection only. It is not proof of correctness by itself.

### PS1 vs reference comparison

Use:

- `repo:/scripts/compare-reference-batch.sh`

This should be the main path for actual validation sweeps against the canonical reference set.

Expected comparison artifacts per scene:

- `compare.json`
- `compare.html`

If compare alignment fails, that is not a pass. It means the harness or the run still needs work.

### Validation rule

A scene is only validated when:

- the PS1 run clearly reaches the intended scene
- the scene content aligns against the reference in a defensible way
- the generated HTML artifacts support that claim

## Current Harness Concerns To Investigate

You should actively test and improve these:

1. Boot lead-in may be under-budgeted.
   - Current scene windows may begin too early.
   - Test longer frame budgets and later alignment windows.

2. Some current comparisons may overweight title/black/ocean contamination.
   - The harness must reject invalid anchors instead of fabricating confidence.

3. HTML output must always exist for human review.
   - Keep that invariant.

4. The “best frame” or “state hash” is not enough.
   - Validation must be scene-content driven.

5. If the compare path falls back, make that obvious.
   - Fallback review pages are useful, but they are not validated compare pages.

## Current PS1 Runtime Debug Context

The active deep bug work has been concentrated on `ACTIVITY 1`.

Current strongest runtime read:

- the live seam is likely in `ps1PilotLoadPackIndex(...)` in:
  - `repo:/cdrom_ps1.c`
- it appears stack-layout-sensitive
- exact local ordering of:
  - `cdPath`
  - `cdfile`
  matters
- tiny caller-frame changes around `CdSearchFile(...)` move the runtime between stable bad branches

This means:

- do not assume a clean logic bug
- suspect UB, overwrite, or stack-sensitive corruption around the pack-file lookup path

That said, do not get trapped in only one scene forever if the better immediate win is harness confidence or a different scene that is closer to validated.

## Recommended Work Sequence

1. Verify the harness outputs are always reviewable.
   - `review.html` for single runs
   - `compare.html` for reference comparisons

2. Audit timing assumptions.
   - Test whether boot grace, scene-entry windows, or total run length need to move later
   - Specifically test the “add roughly 35 seconds” hypothesis

3. Run targeted scene comparisons against the reference set.
   - Prefer one scene at a time when debugging
   - Prefer broader sweeps when ranking “closest to validated”

4. Identify the easiest real win.
   - Choose scenes that already show actual content and are closest to aligned reference output

5. Only then return to deeper runtime surgery when needed.

## Logging Requirements

Keep a concise progress log in:

- `repo:/docs/ps1/research/HARNESS_WORKLOG_2026-03-28.md`

For each significant step, record:

- what was changed
- what run was executed
- where the result artifacts are
- what conclusion is justified
- what the next target is

Do not write vague summaries. Record the actual result and the actual conclusion.

## Standards

Be skeptical of false positives.

If a scene:

- never launches
- only shows water
- only shows black
- only shows title
- or fails to align cleanly against reference

then it is not validated.

The end goal is:

- `63` scenes
- all human-reviewable
- all compared against canonical reference
- all validated on real scene content
