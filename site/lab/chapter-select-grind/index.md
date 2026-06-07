---
layout: page
title: The chapter-select grind
eyebrow: Lab · Retrospective
subtitle: Walking all 63 packs on hardware again to fix the thumbnails — and the surprising number of caption-mismaps it caught.
description: A retrospective on the v0.8.4 chapter-select-thumbnail loop — what it took to ship a custom on-PS1 thumbnail for every scene and reconcile the website against the packs that actually play.
date: 2026-05-08
image: /assets/img/scene-explorer-fishing5.png
image_alt: The PS1 v0.8.4 Scene Explorer mid-grind — the in-pause-menu chapter-select grid showing live thumbnails captured on hardware, the loop the essay walks. Captured headlessly via the validation harness.
image_width: 640
image_height: 448
---

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

`v0.8.4-ps1` is the smallest-feeling release the project has shipped in
months, and the only release that took watching every scene play on
real hardware to ship. Before this loop, the in-game [Scene Explorer]({{ '/docs/glossary/#scene-explorer' | relative_url }})'s
chapter-select grid showed an auto-generated frame for each pack —
whatever the build pipeline picked at thumbnail-bake time. After this
loop, every grid slot is a frame the human author chose while the pack
played on PS1, encoded as a 320×240 [RGB555]({{ '/docs/glossary/#rgb555' | relative_url }}) SCR, and bundled onto the
disc.

The surprising thing wasn't the thumbnails. It was how many of the
existing scene titles, page bodies, and ledger notes turned out to be
wrong once the discs were watched.

## The finite list, again

Sixty-three scenes. The same finite list the
[63-scene grind]({{ '/lab/the-63-scene-grind/' | relative_url }})
walked through for visual signoff. The chapter-select grind walked it
again, but for a different reason: not "does this scene render
pixel-perfect," which has been true since
[`v0.7.0-ps1`]({{ '/releases/#v070-ps1--63-scene-completeness' | relative_url }})
— but "does the in-game menu and the website actually describe what
the scene plays."

Every scene already had a page at `/scenes/<slug>/`, a row in the live
ledger at [/scenes/]({{ '/scenes/' | relative_url }}), a Notes column
in `docs/ps1/scene-status.md`, and a thumbnail file. Nothing about the
loop *added* surfaces — it reconciled five existing ones in lockstep.
Per scene:

1. Per-page `index.md` — frontmatter `title:`, `description:`, and
   the "What this scene is" body section.
2. `site/_data/scenes.yml` — `notes:` field.
3. `docs/ps1/scene-status.md` — Notes column.
4. `jc_resources/extracted/scr/SX<abbrev><tag>.SCR` — the chapter-
   select thumbnail.
5. [`src/pause_menu/scene_explorer_data.h`]({{ site.github_url }}/blob/main/src/pause_menu/scene_explorer_data.h) — the in-game Scene Explorer header,
   regenerated from sources 1–3.

## What the on-PS1 view caught

Most prior scene titles came from a caption-mapping audit done early
in the project — a best-effort guess at which pack played which gag
based on the original game's on-screen captions and the pack's name.
Several were just wrong, and the auto-generated thumbnails had been
masking that. Highlights from the loop:

- **`FISHING 2` doesn't catch a boot.** The pack reels in a Titanic-
  stenciled life preserver. The audit's "He catches a boot" caption
  belongs to `MARY 2` — Mary the mermaid swims up while Johnny is
  fishing, he mistakes her for a fish on the line, and after the
  confusion clears he reels in a boot.
- **`FISHING 3` isn't the crab-snaps-his-nose gag.** It's an octopus
  that comes up on the line and walks off with the fish.
- **`VISITOR 4` isn't the coconut-plane-hit.** It's just a coconut
  rolling off the island into the ocean. The plane gag is `VISITOR 5`.
- **`WALKSTUF 1` isn't "jogs around the island".** That's `WALKSTUF 3`.
  `WALKSTUF 1` is a yacht-party-and-pass-out beat.

Half a dozen more had the right family but a sharper specific gag —
`ACTIVITY 5` is "rain dance, struck by lightning" not "climb / look /
dive"; `ACTIVITY 7` is "reads a book upside-down" not "bathes /
seagull steals clothes"; `BUILDING 4` is the Gulliver-style lilliputian
tie-up not a generic sandcastle scene; `MARY 4` is specifically Mary's
feelings hurt by Johnny working on the raft, not just "heartbroken at
the raft."

Each correction was made by watching the pack play on hardware —
DuckStation booting a per-scene ISO with `BOOTMODE.TXT` set to
`fgpilot <slug>` — capturing a representative frame, and updating all
five surfaces in lockstep. The scene-page wording changed from "What
this scene probably is (Guess.) ... Caption mapping confidence: MED"
to "What this scene is" with the on-PS1 confirmation noted.

## The five-surface helper

Updating five files per scene, sixty-three times, by hand wasn't
viable. The loop's tooling investment was a single helper:
[`scripts/apply-scene-correction.py`]({{ site.github_url }}/blob/main/scripts/apply-scene-correction.py).
One command per scene:

```text
python3 scripts/apply-scene-correction.py <slug> \
    --title "..."  --short "..."  --description "..."  --body "..." \
    [--png /path/to/screenshot.png]
```

The script does five things in order — page edit, scenes.yml notes
prepend, scene-status.md row prepend, optional PNG-to-SCR encode,
progress-tracker flip — and uses exact-string Edits everywhere. That
last detail was deliberate: re-running the script on an already-
corrected scene fails noisily because the old strings aren't there to
match. A re-run can't silently clobber a prior fix.

Two bugs surfaced inside the loop that the helper had to learn from:

- **Body-section regex over-match.** The first version's lookahead
  was `\n## ` (h2-or-end), but the per-page body sits above an H3
  ("How this scene gets validated"). With no later H2 in the file,
  the non-greedy match ran to end-of-file and ate the validation
  trailer. Tightened the lookahead to `\n#{2,} `.
- **Scene-status row regex non-greedy column miscount.** The
  scene-status table has eight columns; notes is the eighth. The
  first version's non-greedy `.*?\| ` skip was supposed to walk past
  four columns before reaching notes, but non-greedy stops at the
  first separator — so the prepended short-description landed in the
  `sfx` cell instead of `notes`. Replaced the skip with an explicit
  `(?:[^|]+\| ){4}` so each intermediate cell consumes its full
  content.

Both were caught on the first real scene run, fixed in
[`6c76aa316`]({{ site.github_url }}/commit/6c76aa316), and the
remaining 62 scenes ran cleanly.

## The manifest gap

After all 63 scenes were corrected and the CD rebuilt, a quick walk
through Scene Explorer reported 11 broken slots: `STAND 2-5` plus the
last six grid positions. The SCR files existed on disk at the right
sizes; the in-game loader was returning "file not on disc."

The bug was in [`config/ps1/cd_layout.xml`]({{ site.github_url }}/blob/main/config/ps1/cd_layout.xml). The manifest only listed
42 of the 63 thumbnail SCRs — the 21 newly-created ones (`SXBL2-7`,
`SXMG1-2`, `SXST1-5`, `SXSU1-2`, `SXVI1`, `SXVI3-7`) were on disk but
never made it onto the CD because nothing referenced them. Files exist
on the host filesystem; nothing puts them on the disc unless the
manifest names them.

The user-facing report was a partial set ("STAND 2-5, BUILDING 2-7,
positions 58-63") because those were the slots the user happened to
navigate to before reporting. The actual broken set was 21 — they
just hadn't all been clicked yet. Fix was a 21-line manifest patch
(`3d673b48f`); a `comm -23` between the filesystem and the manifest
verifies parity going forward.

## The loop is the work

Same observation as the
[63-scene grind essay]({{ '/lab/the-63-scene-grind/' | relative_url }}):
the work isn't an algorithm. It's a finite list, walked with care.

What the chapter-select grind added on top of that observation is
that the validation bar can stop being honest about what each scene
*shows* even while remaining honest about whether each scene
*renders*. The host-vs-PS1 frame diff that defines the
[FISHING 1 bar]({{ '/docs/glossary/#fishing1-bar' | relative_url }})
catches pixel drift, not caption drift. Twenty-three months of
"validated 63/63" was true for the visuals and silent on the metadata.

The chapter-select loop was the first time anyone had a reason to
read every scene's prose against every scene's pack at the same time.
The audit-guess hedging on dozens of pages had been quietly outdated
for months; nothing in the build pipeline could surface that without
a human watching.

## Cross-links

- [/releases/#v084-ps1--custom-chapter-select-thumbnails-for-all-63-scenes]({{ '/releases/#v084-ps1--custom-chapter-select-thumbnails-for-all-63-scenes' | relative_url }})
  — the release notes entry that ships this loop.
- [/lab/the-63-scene-grind/]({{ '/lab/the-63-scene-grind/' | relative_url }})
  — the prior validation grind retrospective; the chapter-select loop
  is the metadata follow-on.
- [/scenes/]({{ '/scenes/' | relative_url }})
  — the live ledger now reflects the corrected per-scene leads.
- [/docs/glossary/#fishing1-bar]({{ '/docs/glossary/#fishing1-bar' | relative_url }})
  — the validation bar this loop didn't change; it changed the prose
  about what the validated scenes contain.
- [`scripts/apply-scene-correction.py`]({{ site.github_url }}/blob/main/scripts/apply-scene-correction.py)
  — the 5-surface helper.
- [`config/ps1/cd_layout.xml`]({{ site.github_url }}/blob/main/config/ps1/cd_layout.xml)
  — the CD manifest the missing-thumbnails bug lived in.
