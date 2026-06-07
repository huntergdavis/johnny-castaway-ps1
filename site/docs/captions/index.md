---
layout: page
title: Closed captions
eyebrow: Reference
subtitle: Caption corpus, ADS-tag map, and the dark-band overlay that draws them.
description: How closed captions work in the Johnny Castaway PS1 port — caption corpus, ADS-tag mapping, the 2026-04-26 audit, and the SPRT-based render path.
---

A labor of love by Hunter Davis. Closed captions are a PS1-only addition,
not a feature of the original Sierra screensaver. The text was authored
fresh for this port — it is **not** lifted from any prior caption corpus —
and is intended for accessibility: a viewer who cannot hear the SFX or who
finds the silent comedy ambiguous gets a short subtitle describing what
Johnny is doing.

If you paid for this, you were cheated. Open source and free.

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

## How it works

Three things plug together:

1. **The caption corpus** (`captions[]` in `src/platform/ps1/ps1_captions.c`) holds the
   actual subtitle strings. There are a handful of "special" captions
   (`intro`, `christmas`, `halloween`, `night`, `lowtide`, `fadeout`, etc.)
   plus 64 numbered scene captions (`scene00` … `scene63`).
2. **The scene-to-ADS map** (`captionSceneMap[]`, also in
   `src/platform/ps1/ps1_captions.c`) routes an `(ads_name, ads_tag)` pair to a caption
   id. When a scene starts, the runtime calls
   `captionsOnAdsStart("FISHING", 3)` and the mapper picks the right
   `sceneNN` string.
3. **The render hook** (`captionsRender()` called from `grUpdateDisplay`)
   draws the current caption inside a translucent dark band at the bottom
   of the framebuffer using the same 8x8 ASCII font the pause menu loads
   into VRAM.

The captions module shares VRAM with the [pause menu]({{ '/docs/pause-menu/' | relative_url }}):
on first use it calls `pauseMenuEnsureFontUploaded()` so the captions don't
need to wait for the user to open the pause menu before subtitles can render.
The font lives at VRAM `(640, 256)` with a CLUT at `(640, 360)` — those
constants are exported from [`pause_menu.h`]({{ site.github_url }}/blob/main/src/pause_menu/pause_menu.h).

When captions are disabled or no scene is active, `captionsRender()` returns
immediately. It is zero-cost when off.

## Public API

```c
void  captionsSetEnabled(int enabled);
int   captionsGetEnabled(void);

/* Look up by scene id (e.g. "scene05"). */
void  captionsOnSceneStart(const char *sceneId);

/* Look up by ADS name + tag (the runtime dispatch path). */
void  captionsOnAdsStart(const char *adsName, uint16 adsTag);

/* Currently visible caption text, or NULL. */
const char *captionsGetCurrent(void);

/* Per-frame draw. No-op when disabled. */
void  captionsRender(void);
```

The toggle is reachable from the pause menu's Accessibility sub-screen
(`Captions: ON / OFF`).

## The 2026-04-26 audit

The original `captionSceneMap[]` had a structural bug: it assumed
`caption_index == story_index` in a strictly sequential sweep. That
collides at every ADS group boundary — for instance `scene10`, which is
about lilliputians and yacht photos, was being routed into a `BUILDING`
sand-castle slot. The cascade error pushed every `FISHING` entry off by
one and misaligned the `JOHNNY` and `MARY` day-arcs.

The audit re-derived the map content-first, anchoring captions to the ADS
group they describe and then to `SPOT` / `HDG` / `FIRST` / `FINAL` /
`day_no` flags from `story-scenes.txt`. The result is recorded in
[`docs/ps1/caption-audit-2026-04-26.yaml`]({{ site.github_url }}/blob/main/docs/ps1/caption-audit-2026-04-26.yaml).

Each entry carries a confidence rating:

| Rating     | Meaning |
|------------|---------|
| `HIGH`     | Caption text contains an unambiguous keyword for this ADS+tag (e.g. "catches a starfish, throws it back"). |
| `MED`      | Plausible match, but other captions could also fit. |
| `LOW`      | Generic short text — typically `STAND` idle stances — placed by best-guess `SPOT` / `HDG` hints. |
| `NO_MATCH` | No caption in the corpus describes this ADS+tag well. The audit flags candidates but leaves them assigned. |

The 63-entry audit shakes out as **30 HIGH / 21 MED / 12 LOW / 0 NO_MATCH**.
HIGH is dominated by the story arcs (every `FISHING`, `JOHNNY`, and
`MISCGAG` entry is HIGH) and LOW is dominated by `STAND` idles, which are
genuinely interchangeable from the corpus.

### Confidence by ADS file

| ADS file   | Entries | HIGH | MED | LOW | Notes |
|------------|--------:|-----:|----:|----:|-------|
| `ACTIVITY` | 10      | 0    | 9   | 1   | Generic island gags; tag-to-caption order can't be proven without runtime evidence. |
| `BUILDING` | 7       | 5    | 1   | 1   | LOW slot is `BUILDING 6` — flagged as a `NO_MATCH` candidate. |
| `FISHING`  | 8       | 8    | 0   | 0   | All anchored on caught-object keywords. |
| `JOHNNY`   | 6       | 6    | 0   | 0   | Pinned by `day_no` and `FIRST` / `FINAL` flags. |
| `MARY`     | 5       | 4    | 1   | 0   | The MED entry overlaps `BUILDING 5` content (raft + heartbroken mermaid). |
| `MISCGAG`  | 2       | 2    | 0   | 0   | Both gag captions are unambiguous. |
| `STAND`    | 14      | 0    | 6   | 8   | Idle stances; SPOT hints help, runtime evidence would help more. |
| `VISITOR`  | 6       | 3    | 2   | 1   | LOW slot is `VISITOR 5` (LEFT_ISLAND, FINAL) — `NO_MATCH` candidate. |
| `WALKSTUF` | 3       | 0    | 1   | 2   | Pure walk transitions; both LOW slots are `NO_MATCH` candidates. |
| `SUZY`     | 2       | 2    | 0   | 0   | Counted in the audit notes; both HIGH. |
| **Total**  | **63**  | **30** | **21** | **12** | Total story scenes confirmed at 63 (re-counted; task said 62). |

The four entries currently flagged as `NO_MATCH` candidates — `BUILDING 6`,
`VISITOR 5`, `WALKSTUF 2`, `WALKSTUF 3` — got assignments anyway, on the
"least bad" principle. They're the most likely sites for future blanking
when runtime evidence makes a better mapping possible.

## Post-validation runtime corrections (v0.8.4-ps1)

The audit was a structural fix — the off-by-one cascade is gone, every
ADS+tag has *some* mapping, and the corpus + render path work as
designed. What the audit could not resolve from text alone was whether
the caption it picked actually described the gag the pack plays.

That second-order check arrived in `v0.8.4-ps1`, when the
[chapter-select grind]({{ '/lab/chapter-select-grind/' | relative_url }})
walked all 63 packs on PS1 hardware and reconciled every scene-page
title and body against the disc. The runtime evidence overturned a
substantial fraction of the audit's confidence ratings:

- **Four flat mismaps named.** `FISHING 2`'s pack reels in a Titanic
  life preserver, not a boot — the boot is at `MARY 2`. `FISHING 3`
  is the octopus-steals-fish beat, not a crab snapping a nose.
  `VISITOR 4` is just a coconut rolling into the ocean — the
  coconut-plane-hit is `VISITOR 5`. `WALKSTUF 1` is a yacht-party-
  and-pass-out beat — the jog is `WALKSTUF 3`.
- **Two HIGH-confidence FISHING entries were exact text-pack
  mismatches.** `FISHING 2` and `FISHING 3` were both rated HIGH
  because the audit anchored on caught-object keywords ("boot",
  "crab"). The keyword *was* in the corpus; the pack just doesn't
  play it. The structural strength of HIGH (unambiguous keyword
  match) didn't survive contact with the runtime.
- **Multiple ACTIVITY / BUILDING / VISITOR MED entries flipped.**
  ACTIVITY 5 (rain dance, not climb/look/dive), ACTIVITY 7 (book
  upside-down, not bathes/seagull), ACTIVITY 11 (bird-steals-
  clothes, not rain dance), ACTIVITY 12 (bird-on-head, not belly-
  flop), BUILDING 2 (lilliputian airport, not boot-roast), BUILDING 5
  (builds fire, not raft/mermaid), BUILDING 7 (grills fish, not
  raft-build), VISITOR 1 (misses speedboat, not lilliputian arrival),
  VISITOR 3 (perspective gag, not yacht/photos).
- **One audit `NO_MATCH` candidate turned out to be a clear gag.**
  `VISITOR 5` was on the "future blanking" list above; on PS1 it's
  the unambiguous coconut-plane-hit beat that's been miscredited to
  `VISITOR 4` since the audit shipped.
- **JOHNNY 3 / 4 were swapped.** The audit had `JOHNNY 3` as "his
  own SOS returns" and `JOHNNY 4` as "writes a fresh SOS". On PS1
  it's the other way around: JOHNNY 3 writes the letter to Suzy, and
  the SOS returns at JOHNNY 4.
- **STAND idles read better than the audit could prove.** All 14
  STAND entries got concrete two-axis labels (position × pose:
  edge-of-island / front / right / left × adjusts-pants / adjusts-
  hat / looks-around / scratches-head / spyglass) once watched on
  hardware. The 8 LOW-rated rows are now distinguishable from each
  other in a way the corpus alone could not support.

The corpus and on-screen captions themselves did not change in
`v0.8.4-ps1`. What changed is the website's *description* of which
caption belongs to which scene — the per-scene `index.md` titles +
bodies, the [`scenes.yml`]({{ site.github_url }}/blob/main/site/_data/scenes.yml) notes, the [`scene-status.md`]({{ site.github_url }}/blob/main/docs/ps1/scene-status.md) Notes column,
and the in-game [Scene Explorer]({{ '/docs/glossary/#scene-explorer' | relative_url }})'s display strings (regenerated from
those sources). The `captionSceneMap[]` array in
[`src/platform/ps1/ps1_captions.c`]({{ site.github_url }}/blob/main/src/platform/ps1/ps1_captions.c)
was not updated as part of `v0.8.4-ps1`; the runtime continues to
play whatever caption the audit's mapping picked. Repointing the
`captionSceneMap[]` rows where the caption text contradicts the
runtime gag (notably `FISHING 2` → MARY's boot, `FISHING 3` → the
crab caption belongs elsewhere or stays as `NO_MATCH`) is open work.

The `2026-04-26` audit's confidence ratings should now be read as
"how well the caption corpus maps to the audit's notion of what each
scene should be," not "how well the caption text describes what the
scene actually plays." The audit was a necessary structural fix; the
chapter-select grind was the runtime cross-check the audit had
explicitly anticipated.

## How to add a caption

The pipeline is two arrays in one file. Both live in
[`src/platform/ps1/ps1_captions.c`]({{ site.github_url }}/blob/main/src/platform/ps1/ps1_captions.c).

1. Add the caption text to the `captions[]` array.

   ```c
   {"sceneNN",
       "Short opening line.\n"
       "Second line, ~35 chars max.\n"
       "Optional third line."},
   ```

   Lines wrap at `\n`. Three lines fit in the dark band at the rendered
   font size; a fourth line clamps the band but is supported. Keep each
   line at or under ~35 characters at 16px draw width to avoid edge
   clipping at 320 logical width.

2. Route an ADS tag to it via `captionSceneMap[]`.

   ```c
   { "sceneNN", "FISHING", 9 },
   ```

   The first match wins, so if you add a more specific entry above the
   sequential default, that wins.

3. Rebuild. There is no codegen — both arrays are hand-edited C.

If a scene legitimately has no fitting caption, leave the corresponding
row out of `captionSceneMap[]` and the renderer will simply show no
subtitle for that scene. That's preferable to a wrong caption.

## The render path

`captionsRender()` is called from `grUpdateDisplay` after the scene
composite + `LoadImage` and before `VSync`. The render uses three OT slots
in a small dedicated ordering table (`capOt[]`):

- **Slot N-1** — a `DR_TPAGE` setting the texture page for the font region
  with `abr=1` (semi-trans 50% blend).
- **Slot N-2** — a `POLY_F4` flat quad at full transparent black, sized to
  the actual line count (1–4 lines), positioned along the bottom of the
  640x480 frame. The semi-trans bit lets the underlying scene show through.
- **Slot N-3** — one [`SPRT`]({{ '/docs/glossary/#sprt' | relative_url }}) per glyph, sampled from the pause-menu font
  atlas at draw size 16x16. Lines are centered horizontally inside the
  band.

Glyphs come from the embedded 8x8 ASCII font that `pauseMenuEnsureFontUploaded()`
uploaded once at startup. `PAUSE_GLYPH_FIRST = 0x20` and
`PAUSE_GLYPH_COUNT = 96` cover the printable ASCII range. Anything outside
that range renders as a blank advance.

On the host build (`#ifdef PS1_BUILD` is false), `captionsRender()` is a
no-op stub — captions are a PS1-only feature.

## Related pages

- [Pause menu]({{ '/docs/pause-menu/' | relative_url }}) — owns the shared
  font atlas and the `Captions: ON / OFF` toggle.
- [Development workflow]({{ '/docs/dev-workflow/' | relative_url }}) — how
  the per-scene loop validates that captions fire on the right ADS tags.
- [AI sub-agents]({{ '/docs/agents/' | relative_url }}) — the caption
  corpus and the 2026-04-26 audit's confidence ratings were drafted
  by an LLM sub-agent and then human-edited; this page records what
  that did and didn't cover.
- [Lab: the LLM pass]({{ '/lab/llm-pass/' | relative_url }})
  — magazine treatment of the same disclosure, with the caption
  corpus named as one of the canonical agent-drafted artifacts.
- [Lab: the chapter-select grind]({{ '/lab/chapter-select-grind/' | relative_url }})
  — the v0.8.4-ps1 retrospective on walking all 63 packs on
  hardware to ship the on-PS1 chapter-select thumbnails, which
  also surfaced the [caption-mismap]({{ '/docs/glossary/#caption-mismap' | relative_url }})
  set this page's post-validation runtime corrections section
  enumerates. The pixel-validation bar guards pixel drift, not
  caption drift; that loop reconciled the prose against the discs.
- [Voice guide]({{ '/about/voice/' | relative_url }}) — the editorial
  standard the caption text is held to (short lines, plainspoken,
  describes-the-gag-not-the-frame).

## View source on GitHub

- [`docs/ps1/caption-audit-2026-04-26.yaml`]({{ site.github_url }}/blob/main/docs/ps1/caption-audit-2026-04-26.yaml)
- [`src/platform/ps1/ps1_captions.c`]({{ site.github_url }}/blob/main/src/platform/ps1/ps1_captions.c)
- [`src/platform/ps1/ps1_captions.h`]({{ site.github_url }}/blob/main/src/platform/ps1/ps1_captions.h)
