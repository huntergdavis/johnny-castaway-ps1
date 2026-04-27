---
layout: page
title: Vision-classifier artifacts
eyebrow: Archaeology
subtitle: Three subdirectories of frozen output from the visual-detection-spec era — the reference bank's self-check, the pipeline manifest, and a captured FISHING 1 regtest run.
description: The frozen vision-classifier outputs from the Johnny Castaway PS1 fan port — reference-bank self-check across 63 scenes, pipeline manifest with strongest and weakest scenes, and a full FISHING 1 annotation review.
---

The `vision-artifacts/` directory holds three subdirectories of output
from the project's vision-classifier era — the stretch when the
validation theory was *match each captured PS1 frame against a
reference bank built from host captures, and confidence in the match
proves the scene is rendering correctly*. The era ran roughly from
2026-03-29 (when the v4 reference bank was built) through 2026-04-12
(when the foreground playback pivot displaced the approach). The
artifacts are kept here because they document, at scale, both what the
classifier was good at and where it was weak — and the weak places are
part of why the project moved on.

The classifier itself is not in this directory; the
[vision documentation]({{ '/docs/vision/' | relative_url }}) describes the
current pipeline. What sits here are the *outputs* of the pipeline at
specific points in time, frozen so a reader can see what the
identification report looked like, what the per-scene match quality
was, and what a full regtest run of FISHING 1 produced when the
classifier was the active validation surface.

## What's preserved

Three subdirectories, each with a different scope.

### vision-reference-pipeline-current

This is the *index over the indexes*. Thirteen files including an
`index.html` cover page, a `pipeline-manifest.json` that points at the
reference bank and the self-check, an `artifact-catalog.json` that
enumerates every output the pipeline produces, an
`artifact-checksums.json` with SHA256 over the catalog (so the bundle
is checkable), the per-scene inventory in three formats
(`scene-inventory.html`, `.json`, `.csv`), the `family-summary.csv`
that aggregates by ADS family (10 ACTIVITY, 7 BUILDING, 8 FISHING, 6
JOHNNY, 5 MARY, 2 MISCGAG, 14 STAND, 2 SUZY, 6 VISITOR, 3 WALKSTUF —
sixty-three scenes total), and three diagnostic JSON files:
`strongest-scenes.json`, `weakest-scenes.json`, and
`top-confusion-pairs.json`.

The strongest scenes — the ones where the classifier reached or
approached 100% top-1 against the reference bank — were FISHING-6,
JOHNNY-1, JOHNNY-6, STAND-1, SUZY-1, SUZY-2, BUILDING-6, and BUILDING-4.
The weakest were FISHING-1 (47.6% top-1, confused with FISHING-2 about
40% of the time), FISHING-2 (49.3%), FISHING-7 (57.8%), BUILDING-5
(63.4%, confused with BUILDING-7), and VISITOR-7 (66.1%, confused with
VISITOR-6). Those numbers are not editorial framing; they are the
actual pipeline output, reproduced verbatim in
`weakest-scenes.json`.

The `validation-report.json` is short and clear: twelve checks, twelve
passed. Each check confirms one of the artifacts the pipeline was
supposed to produce did exist at the expected path. It is the
pipeline's "I built every output I was supposed to" receipt.

### vision-reference-selfcheck-20260329-v4

This is the meaty one. Subdirectory name encodes the date the
reference bank was built (2026-03-29) and the pipeline version (v4).
Contents: an `index.html`/`index.json` that catalogues all 63 scenes
with their top-1 ratios; `quality-report.html`/`.json`,
`confusion-report.html`/`.json`, and `family-report.html`/`.json` for
the three diagnostic angles (per-scene quality, scene-to-scene
confusion, per-family aggregation); and a `scenes/` directory with one
subdirectory per scene, each holding a `review.html` and
`vision-analysis.json`.

The self-check is what told the project the reference bank was
internally consistent at the bank-build moment. *If you classify a
frame from FISHING-6 against the bank, does the bank report
FISHING-6 as the top match?* For most scenes the answer was yes. For
the FISHING family the answer was usually-yes-but. The `family-report`
records FISHING's average top-1 at 0.77, the lowest of any family;
SUZY's average was 0.996, the highest. That spread is partly about
sprite visibility (SUZY scenes are dense with sprites; FISHING scenes
have long stretches of ocean-only frames) and partly about the
similarity between scenes within a family.

The 13,128 frames the bank held were drawn from the host-side captures
of all 63 scenes; the per-scene `vision-analysis.json` records the
matchings frame-by-frame. The `review.html` in each scene's directory
is a side-by-side viewer linking each captured frame to the reference
that ranked highest for it.

### fishing1-full-annotation-review

This is one entry from one regtest run, kept whole. The directory is
`regtest-run/20260410-073839/` — the regtest output captured on
2026-04-10 at 07:38:39 — and inside it: a `frames/jcreborn/`
subdirectory holding the BMPs the run captured, a 2,639-line
`regtest.log` with the full DuckStation log (boot, BIOS, CD-ROM,
disc detection, scene start, frame capture), and the run's
`duckstation.log`.

This run is preserved because FISHING 1 was the project's anchor scene
during this stretch — the scene that everything else's tooling was
built around. Saving the full run, including the DuckStation log, makes
the classifier era's working data concrete: someone reading the log can
see what the host saw, in what order, with what timing. The disc image
hashes (`Hash for 'JCREBORN.EXE' - 085F74AB419128F6`), the BIOS used
(`SCPH-1001, 5003, DTL-H1201, H3001 (v2.2 12-04-95 A)`), and the
fast-boot patch are all recorded.

## Why these artifacts are kept

Two reasons. The first is recordkeeping: when the project decided the
broad-classifier approach wasn't going to be the validation surface,
the natural temptation was to delete the outputs along with the
methodology. Keeping them documents what the methodology produced
before it was retired — a rebuttable form of "we tried it." Anyone
arguing in the future that classifier-based identification *would have
worked* can read these reports and see the actual confusion rates.

The second is informational. The strongest/weakest scene lists in
`vision-reference-pipeline-current/` are still informative. The
[regtest reference set]({{ '/archaeology/regtest-references/' | relative_url }})
that replaced this approach uses state-hash equality, which doesn't
care about classifier confusion rates — but the per-scene difficulty
distribution is real, and it's the same distribution that affects
bringup ordering. The scenes that confused the classifier are the
scenes whose frames look most alike in pixel space; those are the
scenes hardest to get right one at a time.

## What's NOT preserved

The reference bank itself — `features.npy` (the embedding matrix) and
`metadata.json` and `index.json` — was at
`/tmp/jc_reborn_ps1_debug/artifacts/vision-reference-bank-20260329/`,
which is a tmpfs path and does not survive reboot. The bank can be
rebuilt from the per-scene review HTMLs and the source frames if
needed, but the rebuilt bank's bytes will not match the original.

The host-side reference frames — the 13,128 BMPs the bank was built
from — are in the gitignored host-capture tree, not in archaeology.
They remain regenerable from the host engine.

## Cross-links

- [Vision documentation]({{ '/docs/vision/' | relative_url }}) — the
  current vision pipeline. The artifacts here are its predecessor.
- [Regtest reference set]({{ '/archaeology/regtest-references/' | relative_url }}) —
  the validation surface that replaced classifier-based identification.
- [Era timeline]({{ '/archaeology/timeline/' | relative_url }}) —
  the dates around when classifier-based identification was the
  primary truth surface.
- [The 63/63 chapter in the era timeline]({{ '/archaeology/' | relative_url }}) —
  context for why this era's confidence eventually came down.

## Source on GitHub

[docs/ps1/archaeology/vision-artifacts/]({{ site.github_url }}/tree/main/docs/ps1/archaeology/vision-artifacts/) —
three subdirectories: `vision-reference-pipeline-current/`,
`vision-reference-selfcheck-20260329-v4/`, and
`fishing1-full-annotation-review/`.
