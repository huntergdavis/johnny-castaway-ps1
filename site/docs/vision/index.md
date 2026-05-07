---
layout: page
title: Vision-classifier work
eyebrow: Reference
subtitle: A host-side semantic layer that compares PS1 frames to a reference bank — what it caught, what it missed, where it stands.
description: The vision-classifier validation layer for the Johnny Castaway PS1 fan port — reference-first nearest-neighbor retrieval on the host, with optional VLM captioning, used to triage scene rendering bugs.
---

A labor of love by Hunter Davis. This page describes the vision-classifier
work done on the host validation side of the project: what problem it
solves, what model it uses, how it gets called, and where it currently
stands. If you paid for this, you were cheated. Open source and free.

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

## Why a vision classifier

The validation problem on this project is "did this scene render
correctly?" The acceptance bar is human visual + audible signoff, but
human review does not scale to an iterative CD-image rebuild loop.
Something has to triage: which scenes look broken, which look fine,
which look subtly wrong in a way that warrants a closer human pass.

Pixel-by-pixel diff against a host-captured reference frame works for
some scenes. It breaks for others, in predictable ways:

- **Palette mismatches.** Index 5 is true black on the host, mapped to
  PS1 0x0001 because 0x0000 is the transparency semaphore. Direct
  pixel comparison flags every black pixel as different.
- **Tile-edge artifacts.** The PS1 compositor draws on 320x240 tile
  boundaries inside the 640x480 framebuffer. Sub-pixel stitching at
  tile edges produces small differences that pixel diff treats as
  failures.
- **Randomized variants.** The island position randomizes per scene
  (xPos / yPos offsets within fixed ranges). A captured reference
  frame at xPos=-200 cannot pixel-diff against a PS1 frame at
  xPos=-114; the same scene is at a different screen position.
- **Telemetry overlay contamination.** When telemetry is on (it is
  by default in dev builds), the top-left 100&times;243 region of the
  framebuffer is colored diagnostic bars. Pixel diff against a
  telemetry-free reference flags every bar pixel as broken.

A per-scene visual-difference *metric* — "how similar are these two
frames in the way that matters?" — is much more useful than raw pixel
diff. The vision classifier is that metric.

## The local classifier

The classifier is reference-first, not training-first. Source of truth
is the canonical reference capture set at
`regtest-references/`, produced by running the host build through every
scene and saving frame PNGs. The PS1 evaluation compares against that
bank rather than inferring scene content from scratch.

**Where it runs:** Host build only. The PS1 has no business running a
classifier. Output is written to host disk and consumed by the regtest
harness and the scene-promotion workflow.

**What model:** The current operational pipeline is non-neural — it
uses Pillow + NumPy and deterministic visual features (color
histograms, water/dark/edge ratios, telemetry presence) plus
lightweight per-frame feature vectors and nearest-neighbor retrieval.
The decision to start there was deliberate: avoid heavyweight runtime
dependencies (`torch`, `transformers`, `sklearn`) so the pipeline runs
cleanly on the author's ~8 GB RAM dev machine without GPU.

**The original plan** was a layered approach: deterministic triage
first, then CLIP ViT-B/32 embeddings for nearest-neighbor retrieval,
then optional Florence-2 or SmolVLM captioning on sampled mismatch
frames. The deterministic and feature-vector retrieval layers are
implemented and operational. The CLIP and VLM layers were specced but
the working pipeline reached usable triage quality without them, so
they remain on the shelf.

**A separate VLM track** spec'd at
[`docs/ps1/research/VLM_CLASSIFIER_PLAN_2026-03-29.md`]({{ site.github_url }}/blob/main/docs/ps1/research/VLM_CLASSIFIER_PLAN_2026-03-29.md)
points at OpenVINO GenAI with `llmware/Qwen2.5-VL-3B-Instruct-ov-int4`
for structured semantic captioning ("Johnny is fishing off the right
side of the dock"). That track is in research status — runtime is
installed, smoke tests are run, but the operational pipeline does not
depend on it.

## How it gets called

The pipeline lives at
[`scripts/vision_classifier.py`]({{ site.github_url }}/blob/main/scripts/vision_classifier.py)
with a one-command runner at
`scripts/run-vision-reference-pipeline.sh`. Three primary subcommands:

```bash
# Build the reference bank (one-time, or after reference recapture)
python3 scripts/vision_classifier.py build-reference-bank \
    --refdir regtest-references \
    --outdir artifacts/vision-reference-bank-20260329

# Analyze one PS1 run against the bank
python3 scripts/vision_classifier.py analyze-run \
    --scene-dir regtest-results/<runId>/ACTIVITY-1 \
    --bank-dir  artifacts/vision-reference-bank-20260329 \
    --outdir    vision-artifacts/<runId>/ACTIVITY-1 \
    --expected-scene ACTIVITY-1

# Self-check: analyze every reference scene against the bank
python3 scripts/vision_classifier.py analyze-reference-set \
    --refdir    regtest-references \
    --bank-dir  artifacts/vision-reference-bank-20260329 \
    --outdir    vision-artifacts/vision-reference-selfcheck-...
```

Output for each analysis is structured JSON plus a per-scene HTML
review page. The harness ingests the JSON and the human reads the
HTML when triaging a regression. A top-level published index lives at
`vision-artifacts/vision-reference-pipeline-current/index.html`.

The reference bank as last built indexed **13,128 frames across 63
scenes**. Per-frame data:

```json
{
  "scene_id": "ACTIVITY-1",
  "frame": "frame_06000.png",
  "metrics": {
    "mean_rgb": [34, 88, 120],
    "water_ratio": 0.41,
    "dark_ratio": 0.03,
    "edge_density": 0.19,
    "telemetry_present": true
  },
  "labels": {
    "screen_type": "scene",
    "scene_family_guess": "ACTIVITY",
    "sprites_visible": true,
    "johnny_present": true,
    "mary_present": false
  }
}
```

A PS1 run analysis emits both a `best_expected_match` (within the
target scene) and a `best_global_match` (across all reference scenes).
The distinction is what makes the output useful: an expected scene of
`ACTIVITY-1` with global match of `FISHING-3` is a different kind of
failure than `ACTIVITY-1` with global match of `ACTIVITY-1` but low
sprite-visible ratio. The first is a wrong-family failure; the second
is a sprite-rendering failure.

## What it caught that pixel-diff missed

The retrieval-plus-labels approach has flagged failure modes that
pixel-diff couldn't:

- **Background-correct, sprite-missing.** The PS1 frame matches the
  expected reference background (ocean, island, sky) but the sprite
  compositing region has no characteristic non-background palette
  colors. Visually: "Johnny didn't get drawn." Pixel-diff would
  report a high diff and not say why.
- **Wrong-family reroute.** Expected `ACTIVITY-1` but global-match
  closest scene is `FISHING-3`. The runtime took the wrong ADS
  family branch — usually a story-state bug, not a rendering bug.
- **Title persistence.** PS1 frame is dominated by black + bright
  cyan in `INTRO.SCR`-typical proportions when the scene should
  already have started. Indicates a startup hang or a load failure.
- **Ocean-only when island expected.** Detects the absence of
  bright yellow (palette index 14) in the scene region. Pixel diff
  would flag the whole frame as different; the classifier flags
  *why*: island content didn't render.

## What it didn't help with

The classifier is a visual layer. It does not see audio, time, or
input. Things it does not catch:

- **Audio sync.** A frame can render correctly but have the SFX play
  on the wrong beat. The vision classifier sees the frame, not the
  sound.
- **Controller flow.** Pause-menu and input-handling regressions
  produce identical frames to a working build until the player tries
  to interact. The classifier never tries.
- **Scene timing.** Two runs of the same scene can render the same
  frames at different rates. The classifier looks at output, not
  cadence; the regtest harness's `loop_vb` counters do that.
- **First-time-on-hardware bugs.** Everything described here runs
  against DuckStation captures. Real-hardware-only divergences
  (rare but exist) need real-hardware capture, which the
  classifier doesn't drive.

## Where it stands

The pipeline is operational on the reference side. **63 reference
scenes processed, 13,128 frames indexed**, full self-check artifact
set published. Quality reports, confusion reports, family reports,
and inventory CSVs are all generated.

Semantic precision is acknowledged as v1, not final. Known weak
areas, from the worklog:

- Similar `FISHING` scenes cross-match each other.
- Several `STAND` scenes remain hard to separate.
- Some scene families need stronger actor/sprite semantics than the
  current deterministic features expose.

The pipeline's status today is **in active use for reference-side
self-check, in standby for PS1-run analysis.** The next planned
practical step is running the same pipeline against PS1 result
directories to produce per-scene PS1 quality summaries — that is the
specific use case that would directly help fix PS1 scene bugs. It is
ready to run; it is not yet wired into the bring-up loop on every
scene.

The VLM track (Qwen2.5-VL-3B in OpenVINO) remains in research status.
If actor identification gets stuck on the deterministic feature
ceiling, that's the path to revisit. As of {{ site.release.tag }} the
deterministic + retrieval pipeline is doing enough that the VLM has
not been pulled forward.

## Related pages

- [Visual detection spec]({{ site.github_url }}/blob/main/docs/ps1/visual-detection-spec.md) —
  the underlying palette and sprite analysis the classifier rests on.
- [Regression testing]({{ '/docs/regtest/' | relative_url }}) — the
  harness that produces the inputs to this pipeline.
- [Performance work]({{ '/docs/performance/' | relative_url }}) — the
  other validation surface, for runtime cadence rather than pixel
  content.
- [AI sub-agents]({{ '/docs/agents/' | relative_url }}) — vision
  classification is one place AI gets used at process boundaries.
- [Method]({{ '/about/method/' | relative_url }}) — how the
  validation work fits the project's overall acceptance model.
- [Lab: regression as a lifestyle]({{ '/lab/regression-as-lifestyle/' | relative_url }})
  — the magazine treatment of why frame-by-frame regtest +
  vision classification are how this project gets work done,
  not features bolted on. Reciprocal of the link from there to
  here.

## View source on GitHub

- [`docs/ps1/visual-detection-spec.md`]({{ site.github_url }}/blob/main/docs/ps1/visual-detection-spec.md)
- [`docs/ps1/research/LOCAL_VISION_CLASSIFIER_PLAN_2026-03-29.md`]({{ site.github_url }}/blob/main/docs/ps1/research/LOCAL_VISION_CLASSIFIER_PLAN_2026-03-29.md)
- [`docs/ps1/research/VISION_CLASSIFIER_WORKLOG_2026-03-29.md`]({{ site.github_url }}/blob/main/docs/ps1/research/VISION_CLASSIFIER_WORKLOG_2026-03-29.md)
- [`docs/ps1/research/VISION_CLASSIFIER_USAGE_2026-03-29.md`]({{ site.github_url }}/blob/main/docs/ps1/research/VISION_CLASSIFIER_USAGE_2026-03-29.md)
- [`docs/ps1/research/VLM_CLASSIFIER_PLAN_2026-03-29.md`]({{ site.github_url }}/blob/main/docs/ps1/research/VLM_CLASSIFIER_PLAN_2026-03-29.md)
- [`scripts/vision_classifier.py`]({{ site.github_url }}/blob/main/scripts/vision_classifier.py)
- [`scripts/run-vision-reference-pipeline.sh`]({{ site.github_url }}/blob/main/scripts/run-vision-reference-pipeline.sh)
