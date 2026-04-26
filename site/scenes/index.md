---
title: Scenes
eyebrow: Ledger
subtitle: 63 scenes total. The honest count of what's validated, what's pending, and what each one is.
---

A real scene table lands here in P2. It will read from
`docs/ps1/scene-status.md` (parsed into `_data/scenes.yml` at
build time) and render every scene as a row — sortable by name,
ADS tag, validation status, and last-verified date.

For now: **{{ site.release.scenes_validated }} of {{ site.release.scenes_total }}
scenes** are confirmed working in the {{ site.release.tag }} build,
with FISHING 1 (Johnny casts and reels), FISHING 2 (a life raft
drifts past), and FISHING 3 (an octopus interlude) as the
reference scenes.

## What "validated" means

A scene is validated when:

1. The host capture produces a clean FG2 pack with no missing frames.
2. The PS1 build plays the pack through every variant the original
   game randomizes between.
3. A second pass at native resolution shows no VRAM corruption,
   no off-island sprites, no audio dropouts.

That's a higher bar than "it ran once and didn't crash" — which is
why the count is so low.

## What scenes look like in the corpus

The corpus is named the way Sierra named it:

| ADS file       | Scenes per file |
|----------------|-----------------|
| `FISHING.ADS`  | 8               |
| `JOHNNY.ADS`   | 14              |
| `MARY.ADS`     | 6               |
| `BUILDING.ADS` | 6               |
| `VISITOR.ADS`  | 5               |
| ...            | ...             |

The complete index — including which scenes have closed captions,
which holidays apply, and which packs they correspond to — is
what gets rendered onto this page in P2.

## Per-scene case studies

Every scene gets its own page at `/scenes/<slug>/`. Validated ones
get a hero screenshot, the variant gallery, and the captioning
entry. Unvalidated ones just say "Not yet validated" — there's no
"claim this scene" button. This is information, not a recruitment
funnel.
