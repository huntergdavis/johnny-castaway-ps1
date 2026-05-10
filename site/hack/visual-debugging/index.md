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

- A [telemetry overlay]({{ '/docs/glossary/#telemetry-overlay' | relative_url }}) drawn by the runtime for frame counters, pack state, resource state, and perf flags.
- Headless DuckStation captures driven by the [regtest harness]({{ '/docs/regtest/' | relative_url }}).
- Scripted PS1 controller routes driven by the [scripted input harness]({{ '/docs/scripted-input/' | relative_url }}), which can open menus, enter Freeplay, and drop screenshot markers without a human at the keyboard.
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

For input bugs, replace "run the scene" with "write the controller route."
`PADSCRIPT.TXT` can say `wait 30s`, `tap START`, `tap DOWN`, `tap CROSS`, and
`shot freeplay-options 30`. The PS1 runtime presses those buttons through its
real pad path, DuckStation captures frames, and the reporter pulls the first
PNG at or after each [`JCPADSHOT`]({{ '/docs/glossary/#jcpadshot' | relative_url }}) marker. That is how the menu help page is
made, and it is also how a flaky "I pressed this and it crashed" report turns
into a repeatable artifact.

That workflow is slower than "try a fix and squint." It is also how the project
found dirty-rectangle errors, tide-state mistakes, SPU timing offsets, and
holiday overlay placement errors that would have blended into the island if
all anyone had was a log line.

For the essay version, read
[Regression as a lifestyle]({{ '/lab/regression-as-lifestyle/' | relative_url }})
and [The 63-scene grind]({{ '/lab/the-63-scene-grind/' | relative_url }}).

## Related pages

- [Hack: start here]({{ '/hack/start-here/' | relative_url }})
  — first-day flow if you haven't already done the build and
  boot loop.
- [Hack: learn C from Johnny]({{ '/hack/learn-c/' | relative_url }})
  — "make state visible" applied to the source: small structs,
  explicit ownership, telemetry overlays.
- [Hack: memory wars]({{ '/hack/memory-wars/' | relative_url }})
  — visual debugging mattered most when the bug was a stale
  buffer or fragmented heap that text logs couldn't surface.
- [Hack: performance loop]({{ '/hack/performance-loop/' | relative_url }})
  — the headless iteration that consumes the screenshots and
  state hashes the visual stack produces.
- [Hack: port to a new platform]({{ '/hack/port-to-a-new-platform/' | relative_url }})
  — the visual debugging discipline travels even when the
  toolchain doesn't.
- [Regression testing]({{ '/docs/regtest/' | relative_url }}) —
  the headless DuckStation harness that drives most of the
  screenshot capture above.
- [Scripted input harness]({{ '/docs/scripted-input/' | relative_url }})
  — the `PADSCRIPT.TXT` example in the workflow section.
- [Vision-classifier]({{ '/docs/vision/' | relative_url }}) —
  the per-frame detection layer that runs on top of the
  screenshots.
- [Lab: regression as a lifestyle]({{ '/lab/regression-as-lifestyle/' | relative_url }})
  — the magazine treatment of the screenshot-diff regtest
  practice this page is the practical-loop chapter for. Cited
  in the body above; surfaced here as a Related entry too.
- [Lab: the 63-scene grind]({{ '/lab/the-63-scene-grind/' | relative_url }})
  — the visual-signoff loop applied 63 times. The visual-debug
  loop on this page is the tooling that makes that signoff
  reproducible.
- [Glossary]({{ '/docs/glossary/' | relative_url }}) —
  vocabulary anchor for `BOOTMODE`, `PADSCRIPT.TXT`, `regtest`,
  `JCPADSHOT`, and the rest of the harness vocabulary used
  above.
