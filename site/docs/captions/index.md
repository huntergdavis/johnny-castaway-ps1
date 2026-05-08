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

1. **The caption corpus** (`captions[]` in `src/ps1_captions.c`) holds the
   actual subtitle strings. There are a handful of "special" captions
   (`intro`, `christmas`, `halloween`, `night`, `lowtide`, `fadeout`, etc.)
   plus 64 numbered scene captions (`scene00` … `scene63`).
2. **The scene-to-ADS map** (`captionSceneMap[]`, also in
   `src/ps1_captions.c`) routes an `(ads_name, ads_tag)` pair to a caption
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
constants are exported from `pause_menu.h`.

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

## How to add a caption

The pipeline is two arrays in one file. Both live in
[`src/ps1_captions.c`]({{ site.github_url }}/blob/main/src/ps1_captions.c).

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
- **Slot N-3** — one `SPRT` per glyph, sampled from the pause-menu font
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
- [Voice guide]({{ '/about/voice/' | relative_url }}) — the editorial
  standard the caption text is held to (short lines, plainspoken,
  describes-the-gag-not-the-frame).

## View source on GitHub

- [`docs/ps1/caption-audit-2026-04-26.yaml`]({{ site.github_url }}/blob/main/docs/ps1/caption-audit-2026-04-26.yaml)
- [`src/ps1_captions.c`]({{ site.github_url }}/blob/main/src/ps1_captions.c)
- [`src/ps1_captions.h`]({{ site.github_url }}/blob/main/src/ps1_captions.h)
