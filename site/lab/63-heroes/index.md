---
layout: page
title: 63 heroes
eyebrow: Lab · Retrospective
subtitle: How every per-scene page on the site got its own captured-on-PS1 hero image — and the cross-link clusters that emerged from writing one figcaption at a time.
description: A retrospective on the per-scene hero-image rollout — how the v0.8.4 chapter-select-grind captures became 62 of 63 figured-and-figcaptioned scene pages, what frame-selection patterns repeated, what cross-link clusters emerged, and how the missing scene was filled in headlessly.
date: 2026-05-09
---

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

Every per-scene page at [`/scenes/<slug>/`]({{ '/scenes/' | relative_url }}) on this site now carries a captured-on-PS1 hero image at the top, a per-page Open Graph override pointing at it, and a figcaption underneath that names the gag and any engineering retro the scene happens to belong to. All sixty-three.

That number is not interesting on its own. The interesting thing is that nobody planned the structure of the figcaptions or the cross-link clusters they made. They came out of writing one scene's hero at a time, in the order DuckStation had captured them, and noticing what wanted a link.

## What the rollout actually was

Per scene, mechanically:

1. Pick a representative frame from the v0.8.4-ps1 [chapter-select grind]({{ '/lab/chapter-select-grind/' | relative_url }})'s capture dump (DuckStation writes 961×720 PNGs into `~/.var/app/.../duckstation/screenshots/` whenever you take a screenshot, and the grind took a named one for almost every scene).
2. Promote it to `site/assets/img/<slug>-ps1-<short>.png`.
3. Add `image:` / `image_alt:` / `image_width:` / `image_height:` to the page's frontmatter so the [head template]({{ '/lab/the-site-itself/' | relative_url }}) flips the per-page OG override on.
4. Add a `<figure class="scene-hero">` block above the body with a figcaption that says what the gag is in one or two sentences.

That four-step pattern was set in [`1adac73b5`]({{ site.github_url }}/commit/1adac73b5) when FISHING 2 got the first per-scene hero. The next sixty-two ships followed it.

## Frame selection rules of thumb

The frame-selection step seemed obvious until it wasn't. A few patterns kept repeating:

- **Compose-the-whole-joke**. Pick the frame where both halves of the gag are visible at the same time. VISITOR 1's hero shows the speedboat zipping past *and* Johnny standing at the front of the island looking the wrong way — same frame. ACTIVITY 11 has the bird perched in the palm leaves with the stolen clothes and a clothes-less Johnny standing in the water — same frame. If the joke is "X happens to Y," the picture should have both.
- **Antagonist-in-frame beats Johnny-alone**. FISHING 4 had two captures: Johnny holding a fishing rod, and a frame with Johnny + a shark fin and a fishing line stretching across the open water. The fin frame won. VISITOR 5 had three captures; the one where the biplane is in-frame won. At OG-thumbnail size the antagonist makes the gag legible; without it, the frame just looks like Johnny standing on an island.
- **Title-naming beat over generic-looking-around**. JOHNNY 2's two captures were a "thinking of home" thought-bubble and an "SOS-note" thought-bubble. The page title is "puts an SOS note inside" — so the SOS bubble won.

None of these rules were written down before the rollout started. They got noticed around the fifth or sixth ship and applied retroactively where a previous frame still looked OK with hindsight.

## Cross-link clusters that wrote themselves

Every figcaption is short. Most of them have one or two outbound links. The links accumulated into clusters that nobody planned.

**Variant pairs** are the simplest case. Two scenes are obvious gag-twins varying one axis:

- [ACTIVITY 1]({{ '/scenes/activity1/' | relative_url }}) belly-flop dive vs. [ACTIVITY 4]({{ '/scenes/activity4/' | relative_url }}) clean dive (impact axis).
- [BUILDING 4]({{ '/scenes/building4/' | relative_url }}) Gulliver-tie-down with bird vs. [BUILDING 6]({{ '/scenes/building6/' | relative_url }}) without bird.
- [BUILDING 5]({{ '/scenes/building5/' | relative_url }}) just a fire vs. [BUILDING 7]({{ '/scenes/building7/' | relative_url }}) fire + grill + eat.
- [FISHING 1]({{ '/scenes/fishing1/' | relative_url }}) left-side starfish vs. [FISHING 7]({{ '/scenes/fishing7/' | relative_url }}) right-side starfish vs. [FISHING 8]({{ '/scenes/fishing8/' | relative_url }}) right-side caught-fish.
- [STAND 3]({{ '/scenes/stand3/' | relative_url }}) edge-of-island hat-adjust vs. [STAND 4]({{ '/scenes/stand4/' | relative_url }}) front-of-island hat-adjust vs. [STAND 7]({{ '/scenes/stand7/' | relative_url }}) look-right-hat-lift vs. [STAND 12]({{ '/scenes/stand12/' | relative_url }}) forward-hat-adjust — that's a hat-pose **quartet**.
- [STAND 2]({{ '/scenes/stand2/' | relative_url }}) edge-pants-adjust vs. [STAND 9]({{ '/scenes/stand9/' | relative_url }}) tree-side-pants-adjust.
- [STAND 6]({{ '/scenes/stand6/' | relative_url }}) front-scratch-head vs. [STAND 8]({{ '/scenes/stand8/' | relative_url }}) right-side-scratch-head.
- [STAND 15]({{ '/scenes/stand15/' | relative_url }}) left-spyglass vs. [STAND 16]({{ '/scenes/stand16/' | relative_url }}) right-spyglass.

**Story-arc pairs and sagas** were more interesting because they walked across families. JOHNNY 2 (recycle a found bottle, write SOS) → JOHNNY 5 (write SOS in a fresh bottle, send) → JOHNNY 4 (one of his bottles eventually washes back). Three scenes, mutually cross-linked. Then SUZY 1 (Suzy back home receives a Johnny letter) ↔ JOHNNY 3 (Johnny writes a letter to Suzy). And then the longest arc in the game: WALKSTUF 2 (raft-build) → STAND 10 (admire raft) → MARY 5 (pack the raft, sail off) → SUZY 2 (drift in to the home beach). Add the SOS bottles in transit and that's an **eight-scene** mutually-linked cluster: walkstuf2 + mary5 + johnny2 + johnny5 + johnny4 + suzy1 + suzy2 + stand10. None of this was visible from the per-scene markdown bodies on their own — it only crystallized once the figcaptions started linking.

**Theme clusters** appeared in single ADS families:

- The **coconut quartet** in VISITOR. [VISITOR 4]({{ '/scenes/visitor4/' | relative_url }}) loses one (rolls off into the ocean), [VISITOR 5]({{ '/scenes/visitor5/' | relative_url }}) weaponizes one (throws at a plane, brings it down), [VISITOR 6]({{ '/scenes/visitor6/' | relative_url }}) shake-crack-eats one, [VISITOR 7]({{ '/scenes/visitor7/' | relative_url }}) just-crack-eats one. Same prop, four different fates.
- The **shark thread** crosses families. [FISHING 4]({{ '/scenes/fishing4/' | relative_url }}) drags Johnny like a water-skier; [FISHING 5]({{ '/scenes/fishing5/' | relative_url }}) eats him whole; [MISCGAG 2]({{ '/scenes/miscgag2/' | relative_url }}) scares him out of his bath.
- The **hard cluster**. [/about/]({{ '/about/' | relative_url }}) names it: foreground-only multi-view scenes ([MISCGAG 1]({{ '/scenes/miscgag1/' | relative_url }}), [MISCGAG 2]({{ '/scenes/miscgag2/' | relative_url }}), [STAND 1]({{ '/scenes/stand1/' | relative_url }}), the wide LILLIPUTIAN arrival) that all needed the generic normal / far-left / far-right host stitch before their packs replayed cleanly.

**Engineering-quirk lineages** showed up too. Three STAND scenes (10, 11, 12) all had the same host pipeline quirk — `STAND.ADS:N` exits after only two frames, the no-stitch export collapses to a 92-byte empty pack. The previously-committed packs played cleanly on PS1, so they were signed off as-is. Paid pragmatism, called out by name. Three more STAND scenes (5, 6, 7, 8) all share the no-stitch fast-path lineage with the runtime FG2 wave-tick fix STAND 8 introduced.

## Engineering retros that didn't justify a doc

A repeated discovery during the rollout was that a figcaption is a useful surface for naming a concrete engineering detail that doesn't justify writing a whole doc but should be on the record somewhere. A reader looking at the [STAND 5]({{ '/scenes/stand5/' | relative_url }}) hero learns:

> Engineering footnote: STAND 5 is what surfaced the "no-stitch fast path fades Johnny's legs" bug — pure base-diff treated frame-0 static pixels as background and dropped them. The exporter fast path now keeps a single-position foreground-only overlay while still skipping far-left / far-right stitch captures for simple STAND scenes.

That sentence didn't exist anywhere on the site before the hero shipped. It would not, on its own, have justified a `/docs/` reference page or a devlog post — but it is a useful piece of the project's why. A reader on [STAND 8]({{ '/scenes/stand8/' | relative_url }}) finds the static-ocean bug retro the same way; on [VISITOR 7]({{ '/scenes/visitor7/' | relative_url }}), the dedupe-too-short-strike-rows fix; on [WALKSTUF 1]({{ '/scenes/walkstuf1/' | relative_url }}), the v0.8.3 compact-FGP3/v4 restore-minus-current pack pass and its before/after VBlank numbers.

## The one scene the chapter-select grind missed

Sixty-two of sixty-three. The capture dump from the v0.8.4 grind covered every scene except [JOHNNY 3]({{ '/scenes/johnny3/' | relative_url }}). The dump was named, frame-by-frame, and `johnny3` was simply absent — the sweep had moved past it without taking a screenshot.

Closing the rollout required generating a fresh capture. The same headless harness machinery that runs [`/help/menu/`]({{ '/help/menu/' | relative_url }}) and powered the [Scene Explorer screenshot]({{ '/docs/pause-menu/#scene-explorer' | relative_url }}) replacement on this site's homepage handled it: write a one-shot `BOOTMODE.TXT` (`fgpilot johnny3 night/raft-stage 4 island-pos -64 54 pad-script`), write a one-shot `PADSCRIPT.TXT` taking four shots across one full ~3.5s scene loop, rebuild the EXE + CD image, run [`./scripts/run-regtest.sh`]({{ site.github_url }}/blob/main/scripts/run-regtest.sh), grep `JCPADSHOT` markers out of the TTY log, pick the captured frame that best showed the gag, restore the originals.

The frame that won was the romantic thought-bubble — Johnny standing on the island writing the letter, picturing himself with Suzy back home embracing on a beach. Two of the four candidate frames showed only Johnny + the letter; one showed only the bubble at a too-early moment. The winning frame had both. Compose-the-whole-joke held up.

## Final shape

Sixty-three of sixty-three. Ten ADS families closed. Every per-scene page now carries a unique OG card. The cross-link cluster taxonomy that emerged was not designed up front; it accreted because every figcaption has to ask "what does this scene most want to link to right now?" and small consistent answers stack up.

If there's a generalizable thing here, it's that figcaptions are an under-rated documentation surface. They are constrained — short, page-local, attached to a picture — and the constraint is what makes them useful. A writer can fit one engineering-retro sentence and one gag-arc cross-link in a figcaption without making it feel padded. Sixty-three of those, taken together, build a connectivity graph that no individual essay had to draw on its own.

The full list of per-scene pages with their heroes is at [/scenes/]({{ '/scenes/' | relative_url }}). The matching commit history is on the project's [main branch]({{ site.github_url }}/commits/main) — every scene's hero is one focused commit, mergeable and revertable on its own.
