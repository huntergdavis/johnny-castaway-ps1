---
title: About
eyebrow: What this is · How it works
subtitle: A fan port of Johnny Castaway, rebuilt for the PS1 in a hybrid host-and-replay pipeline.
---

## In one paragraph

Sierra's *Johnny Castaway* is a screensaver about a small man on a small
island. It originally ran on Windows 3.1, in the *After Dark* aesthetic
of its day. This project ports it to the original PlayStation, hardware
that has roughly the same headroom as a 1992 PC but a wholly different
graphics pipeline. It's not an emulator and it's not a re-creation. It's
a hybrid — a *host build* extracts the game's behavior into small
playback files, and the *PS1 build* renders those files using the
console's native GPU.

## The hybrid pipeline (one diagram, soon)

A method page with the actual pipeline diagram lands in P2 at
`/about/method/`. For now, the short version:

```
[ original Sierra engine ]   --plays a scene-->  [ host capture ]
                                                    |
                                                    v
                                             [ FG2 packs:  one
                                               per ADS+tag ]
                                                    |
                                                    v
                       [ CD-ROM image ]   <-bundles--+
                                                    |
                                                    v
                                            [ PS1 GPU replay ]
```

A *pack* is a small binary file that records every visible bitblit
in a scene — what was drawn, where, when, and against what
background. The PS1 build doesn't interpret Sierra's ADS / TTM
bytecode at runtime; it just plays the packs.

That's why the project counts scenes one at a time. Each scene
needs a verified host capture and a successful PS1 replay before
it joins the validated count.

## Status

A live ledger lives at [/scenes/]({{ '/scenes/' | relative_url }}).
The narrative status report — what's done at the *component* level
(rendering, audio, captions, holidays, pause menu) — lands at
`/about/status/` in P2.

## Project history

A 5-chapter timeline lands at `/archaeology/`. The dated worklogs
that drove each phase live at [/devlog/]({{ '/devlog/' | relative_url }})
in their original form.
