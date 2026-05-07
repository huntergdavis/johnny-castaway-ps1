---
layout: post
title: "Vision Classifier Usage — March 29, 2026"
date: 2026-03-29
editor_note: "Operator's reference for the vision-classifier pipeline as it stood on the day it landed. One-command runs, knobs, what to expect."
source_path: docs/ps1/research/VISION_CLASSIFIER_USAGE_2026-03-29.md
description: "Operator reference for running the PS1 vision-classifier captioning pipeline end-to-end."
---

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

Worktree:
- `/tmp/jc_reborn_ps1_debug`

## One-Command Reference Pipeline

Run:

```bash
/tmp/jc_reborn_ps1_debug/scripts/run-vision-reference-pipeline.sh
```

Optional custom output root:

```bash
/tmp/jc_reborn_ps1_debug/scripts/run-vision-reference-pipeline.sh \
  repo:/vision-artifacts/custom-reference-pipeline
```

## Primary Outputs

Top-level published entry:

- `repo:/vision-artifacts/vision-reference-pipeline-current/index.html`

Reference bank:

- `/tmp/jc_reborn_ps1_debug/artifacts/vision-reference-bank-20260329/index.html`

Latest self-check:

- `repo:/vision-artifacts/vision-reference-selfcheck-20260329-v4/index.html`

## Useful Reports

- quality: `repo:/vision-artifacts/vision-reference-selfcheck-20260329-v4/quality-report.html`
- confusion: `repo:/vision-artifacts/vision-reference-selfcheck-20260329-v4/confusion-report.html`
- family: `repo:/vision-artifacts/vision-reference-selfcheck-20260329-v4/family-report.html`
- inventory: `repo:/vision-artifacts/vision-reference-pipeline-current/scene-inventory.html`

## Core CLI

Build bank:

```bash
python3 /tmp/jc_reborn_ps1_debug/scripts/vision_classifier.py \
  build-reference-bank \
  --refdir repo:/regtest-references \
  --outdir /tmp/jc_reborn_ps1_debug/artifacts/vision-reference-bank-20260329
```

Analyze one run:

```bash
python3 /tmp/jc_reborn_ps1_debug/scripts/vision_classifier.py \
  analyze-run \
  --scene-dir repo:/regtest-references/ACTIVITY-1 \
  --bank-dir /tmp/jc_reborn_ps1_debug/artifacts/vision-reference-bank-20260329 \
  --outdir repo:/vision-artifacts/example-run \
  --expected-scene ACTIVITY-1
```

Analyze all reference scenes against the bank:

```bash
python3 /tmp/jc_reborn_ps1_debug/scripts/vision_classifier.py \
  analyze-reference-set \
  --refdir repo:/regtest-references \
  --bank-dir /tmp/jc_reborn_ps1_debug/artifacts/vision-reference-bank-20260329 \
  --outdir repo:/vision-artifacts/vision-reference-selfcheck-20260329-v4
```

Publish top-level entry pages:

```bash
python3 /tmp/jc_reborn_ps1_debug/scripts/publish-vision-pipeline.py
```

## Important Note

This pipeline is complete on the reference side.
The next meaningful use is PS1 analysis against the built reference bank.
