---
layout: page
title: Retired scripts
eyebrow: Archaeology
subtitle: Fourteen shell and Python scripts from the binary-library regression era, kept here so the era's tooling is findable without commit archaeology.
description: Fourteen retired scripts from the Johnny Castaway PS1 fan port's binary-library regression era — what each one did, why it was useful, and why it was retired when the project pivoted away from history-bisection.
---

The `retired-scripts/` directory holds fourteen scripts that ran during
the 2026-03-29 to 2026-04-11 binary-library regression era. They built
the [binary library]({{ '/archaeology/binary-library/' | relative_url }}),
swept through it looking for regressions in FISHING 1, and reported on
its coverage gaps. None of them are invoked by the current operator
workflow. They are preserved here per the cleanup plan's `phase_04`
archive decision so that "what existed during the binary-library era"
is one directory listing rather than a commit archaeology exercise.

The scripts' own README documents the inventory in tabular form. What
follows is the narrative version — what each one was for, what made it
the right call at the time, and what made retiring it the right call
once the project changed shape.

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

## The library builders

`build-binary-library.sh` (17 KB) was the workhorse. Given a commit
range, it walked every code-changing commit, built the PS1 exe and the
disc image at that commit using the existing `build-ps1.sh` and
`make-cd-image.sh`, captured a `metadata.json` recording the build
result, and dropped everything into a sequence-numbered directory
under `binary-library/`. It was last touched on 2026-04-12 in commit
`0c6387a9`. Over the course of the era it produced approximately
118 GB of artefacts indexed by 6,902 entries.

`regtest-binary-library-scene.sh` (26 KB) was the consumer. Given a
specific scene tag and a binary-library entry, it spun up DuckStation
against the entry's exe and disc image, captured frames, and ran the
regtest comparison logic. This was how FISHING 1 — and only FISHING 1,
in practice — got swept across history. Last touched 2026-04-12,
commit `fa5be68e`.

Both scripts assumed a working `binary-library/` tree on disk. With the
payloads deleted in 2026-04-22, neither script will run end-to-end
without rebuilding the tree first.

## The FISHING-1 onset hunters

This is a family of five scripts, all dealing with the same question
in different framings: *when in history did FISHING 1 first do thing X?*

`find-fishing1-startup-onset.sh` (20 KB) hunted for the commit where
the scene first reached the launched-and-running state. The "onset" was
the boundary between a build that crashed before the title and a build
that booted into the scene. Last touched 2026-04-11, commit `8d2e6d90`.

`find-fishing1-visibility-onset.sh` (16 KB) hunted for the commit where
the scene first drew anything visible — sprites on the screen rather
than just a background. The two onsets were not the same; a scene
could "start" without drawing.

`continue-fishing1-startup-onset.sh` (9 KB) was the resume-from-where-
you-left-off variant for when the startup-onset sweep was interrupted.
The sweep took hours; if a workstation rebooted partway through, this
script found the last successful library entry and continued from
there.

`refine-fishing1-startup-onset.sh` (3 KB) and
`refine-fishing1-visibility-onset.sh` (4 KB) were the narrow-the-range
variants. After a coarse sweep had bracketed an onset to between two
commits, these scripts ran the same comparison at higher resolution —
every commit in the bracket — to pin the boundary down to a single
commit.

`scan-fishing1-binary-regression.sh` (11 KB) was the same idea applied
to regressions: given a known-good earlier commit and a known-broken
later commit, sweep the library for the boundary commit where the
regression entered. Last touched 2026-04-11, commit `8d2e6d90`.

`find-fishing1-regression-boundary.py` (7 KB) was the binary-search
implementation underneath the bash sweep — given a comparison function,
do log-N library lookups to find the boundary commit instead of
walking linearly. This was the right shape for a fast bisect, but only
worked when the failure mode was monotonic (a regression that, once
introduced, stayed broken).

These five scripts were the right tool for one specific class of
question. If the question is *which commit broke this scene*, you want
a binary library and a sweep. The questions stopped being that shape
once the foreground playback pivot landed, because the answer became
*the host build at HEAD has the truth and the PS1 build matches it
through pack data*. Bisecting history was no longer the route to
fixing scenes.

## The library reporters

Three Python scripts read the index and produced summary reports.

`report-binary-library-epochs.py` (4 KB) divided the library into
epochs — broadly contiguous stretches of similar build behaviour — and
reported how many entries each epoch contained. Useful when looking at
the library as a whole and asking *how many builds came before the
graphics rewrite vs. after*. Last touched 2026-04-11, commit
`5d374a46`.

`report-binary-library-gaps.py` (3 KB) reported coverage gaps —
sequences of commits where the build failed and produced no entry, or
where the sequence numbering jumped. Reading this output was how the
operator confirmed the library had built cleanly across a range,
rather than skipping over a string of broken commits.

`report-binary-library-sequence-resets.py` (4 KB) detected
sequence-counter resets — points where `build-binary-library.sh` had
been started fresh and the counter went back to 1. The library went
through several reset boundaries as the build script itself was
adjusted; finding them required scanning the index for the
discontinuities.

These three are still readable as stand-alone Python and would
nominally still run against the surviving index files, but their
output ceases to be useful once the binaries themselves are gone — the
reports are about a corpus that no longer exists.

## The audit utilities

`verify-fishing1-head-clean.sh` (6 KB) checked HEAD's PS1 build against
the binary-library baseline for FISHING 1. It was a fast pre-flight: if
HEAD's build didn't agree with the most recent library entry, no point
in running deeper sweeps. Last touched 2026-04-11, commit `047c1685`.

`analyze-scenes.sh` (1 KB) is a small shim that ran the host-side scene
analysis against a single scene, used during the host-script review
era to keep the per-scene inputs into the manifest in sync. Last
touched 2026-04-04 in commit `15c8e08e`.

## Why archived, not deleted

Three reasons, taken straight from the directory's own README:

- Git history keeps the source, but having the era's scripts grouped
  in one directory makes them findable without commit archaeology.
- They reference a workflow that the current scene-playback path has
  replaced; re-running them against today's HEAD will not work
  cleanly.
- They are concrete record of what the binary-library era's tooling
  looked like — the philosophical predecessor of every `regtest-*`
  script the project still uses.

If a future contributor wants to use one of them again, the README
spells out the steps: copy out (do not symlink, the scripts expect to
live under `scripts/`), confirm the Docker image still builds the
target commit range, and confirm the helper paths still exist. None of
those preconditions hold today out of the box; all are recoverable.

## Cross-links

- [The binary library]({{ '/archaeology/binary-library/' | relative_url }}) —
  the corpus these scripts consumed.
- [Era timeline]({{ '/archaeology/timeline/' | relative_url }}) —
  the dates around when these scripts were active and when they were
  retired.
- [Primary sources]({{ '/archaeology/data/' | relative_url }}) — the
  `tools.yaml` catalogue records last-touched commits for every script
  the project has ever shipped.

## Source on GitHub

[docs/ps1/archaeology/retired-scripts/]({{ site.github_url }}/tree/main/docs/ps1/archaeology/retired-scripts/) —
fourteen scripts plus README.
