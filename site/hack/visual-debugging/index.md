---
layout: page
title: Visual Debugging
eyebrow: Curious hacker path
subtitle: When text lies, draw pixels.
description: Visual-debugging guide for the Johnny Castaway PS1 port, covering screenshots, overlays, frame diffs, and review HTML.
---

For a long stretch of this project, text logging was not trustworthy on the
PS1 path. The runtime could destabilize under enough unbounded TTY output. The
solution was blunt and effective: draw the debug information into the frame,
capture the frame, and compare the pixels.

That choice shaped the whole project. The site exists partly because so many
debugging artifacts became historical evidence.

## The Tools

The visual stack has several layers:

- A telemetry overlay drawn by the runtime for frame counters, pack state, resource state, and perf flags.
- Headless DuckStation captures driven by the [regtest harness]({{ '/docs/regtest/' | relative_url }}).
- Frozen host references under [regtest reference cases]({{ '/archaeology/regtest-references/cases/' | relative_url }}).
- Side-by-side review HTML that makes frame drift reviewable by a human.
- The [scene ledger]({{ '/scenes/' | relative_url }}), where each scene's status is explained in public.

The [vision docs]({{ '/docs/vision/' | relative_url }}) cover the screenshot
detection side. The [vision artifacts page]({{ '/archaeology/vision-artifacts/' | relative_url }})
preserves the older detection work.

## The Rule

If a bug is visual, make it visible in the artifact. Do not hide it behind a
single pass/fail line. The project learned this the hard way during the
restore-pilot era, when automated checks could say "green" while the frame on
screen was still not Johnny Castaway. A test that measures the wrong thing is
not dishonest; it is worse than that, because it is useful-looking.

## The Workflow

Run the scene. Capture frames. Diff the frames. Open the review page. Decide
what changed. Only then change code.

That workflow is slower than "try a fix and squint." It is also how the project
found dirty-rectangle errors, tide-state mistakes, SPU timing offsets, and
holiday overlay placement errors that would have blended into the island if
all anyone had was a log line.

For the essay version, read
[Regression as a lifestyle]({{ '/lab/regression-as-lifestyle/' | relative_url }})
and [The 63-scene grind]({{ '/lab/the-63-scene-grind/' | relative_url }}).
