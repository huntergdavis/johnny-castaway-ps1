---
title: Docs
eyebrow: Reference manuals
subtitle: Build, captions, holidays, regtest, API mapping — the technical surface.
---

The docs index lands here in P2 as a small grid pointing at:

- **/docs/build/** — `build-system.md` + `toolchain-setup.md`,
  merged. How to spin up a clean build of the PS1 disc image,
  including PSn00bSDK + DuckStation prerequisites.
- **/docs/captions/** — caption corpus + audit YAML, rendered
  with the ADS+tag → caption mapping legible.
- **/docs/holidays/** — the four holidays-* files merged with
  anchors, including the date-algorithm core (movable feasts
  via Meeus / Zeller, no expiring date tables).
- **/docs/pause-menu/** — current pause-menu design and state
  machine, including the Options sub-screen, Set Time / Island /
  Seed editors, and the Credits page.
- **/docs/freeplay/** — freeplay-mode design and the boot-token
  CLI surface used by the regtest harness.
- **/docs/regtest/** — `regtest-harness.md` + `regtest-quickstart.md`,
  merged. The headless DuckStation rig and how to add a scene to it.
- **/docs/api/** — `api-mapping.md`. Every SDL2 symbol the
  upstream engine called, mapped to its PSn00bSDK replacement.
- **/docs/dev-workflow/** — the author's own per-scene loop.
  Not a contributor onboarding — just the runbook.
- **/docs/archive/** — flat index of older performance and
  optimization docs that aren't promoted but are kept for the
  archaeology.

For now, the source documents live under
[`docs/ps1/`]({{ site.github_url }}/tree/main/docs/ps1) in the
repository.
