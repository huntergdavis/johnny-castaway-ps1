# v0.9.7-ps1 — Final Pre-Release Red-Team Pass

**Date:** 2026-06-25
**Release under audit:** v0.9.7-ps1 (tag `v0.9.7-ps1`, `main` @ `f8022df580`, VERSION 0.9.7, published to GitHub)
**Method:** 8-dimension adversarial red-team (14 agents — one finder per dimension + per-finding skeptic verifiers). Static analysis + reads + quick shell; no rebuilds/soaks.

## Verdict: ⛔ NOT ship-ready as-is — one P0 blocks distribution

The **disc image, code, and stability are sound** (24h/12,512-scene zero-crash soak; clean adversarial code review; fuzz gate green; no visual regressions; clean debug hygiene). **But the public website's Download buttons 404** — the site still points at the old `jcreborn.*` asset names while the release ships `johnnycastawayps1.*`. The fix is website-only (no ISO rebuild, no re-tag).

## Findings (confirmed, by severity)

| # | Sev | Dimension | Finding | Fix |
|---|-----|-----------|---------|-----|
| 1 | **P0** | website | `/play/` Download .bin/.cue buttons link to `jcreborn.bin/.cue`; release ships `johnnycastawayps1.bin/.cue` → **live HTTP 404**. Asset names changed at the v0.9.6 rename; site never updated. | Update `site/play/index.md` (download hrefs, Quickstart, JSON-LD, verify table, sha256sum cmd) + `site/_config.yml:42-43` comment: `jcreborn` → `johnnycastawayps1`. Rebuild `docs/`, push. |
| 2 | **P1** | website | Verify-download SHA-256 table + `sha256sum` command name `jcreborn.*` (hashes are *correct*, only filenames wrong) — verify instructions reference nonexistent files. | Same edit as #1 (filenames only; leave hashes). |
| 3 | P2 | docs | `README.md` "Current release" badge + release history stale at **v0.9.3** (4 releases behind). | Bump README:44/51 to v0.9.7-ps1; add v0.9.4–0.9.7 history bullets. |
| 4 | P2 | website | Public `/releases/` changelog newest entry v0.9.3 but headline says v0.9.7. | Add v0.9.4–0.9.7 entries to `site/releases/index.md`. |
| 5 | P2 | release-integrity | `tests/mem_path_replay` (40 KB x86-64 ELF build artifact) committed in the release tag tree (`.gitignore` missed it; siblings are ignored). Tree also dirty (`mem_path_replay`, `mem_region_fuzz_corpus.txt`). | `git rm --cached tests/mem_path_replay` + add to `.gitignore`; revert/commit the dirty files. Does **not** affect the ISO. |
| 6 | P2 | code | Decline render loop re-loads MRAFT (CACHE alloc/free + SPU DMA + sprite expand) **every frame** while a raft is afloat — worst case on an already-pressured heap. Path hit **0×** in soak. | Cache the expanded raft for the scene's lifetime, or draw it once. Acceptable to ship (unreachable in practice). |
| 7 | P2 | memory | `grBlitToFramebuffer` per-frame ~70 KB temp uses a **halting** `memAlloc` with no no-halt fallback and is unmodeled by the sim. Pre-existing (not a 0.9.7 regression); survived the soak (freed same-call, coalescing makes the hole likely). | Arm `memSetCacheAllocNoHalt` + skip-blit on NULL, or pre-reserve scratch; add a sim model. |
| 8 | P2 | docs | Release notes say walk path is "never a crash," but `fgWalkRender` still has a reachable `JC_BSOD` if both SPU load *and* BMP fallback yield 0 sprites. | Soften wording to "degrades to a teleport when the BMP fallback succeeds," or reconcile code. |
| 9 | P2 | untested | Walk/raft no-halt "teleport" fallback and single-RNG-seed coverage — primary crash-fix path never triggered in the deterministic soak. | Forced-fragmentation verification build; 2–3 extra short soaks on different seeds. |

## Notable downgrades / refutations (adversarial verify earned its keep)

- **REFUTED — "walk-clean restore disabled the entire soak"** (finder: P1). The boot `clean-buf alloc failed` line is the **unused legacy/libc fallback**; the shipping config uses SPU-stage walk-clean (`foregroundPilotSetSpuStage(1)`), which re-validates per scene and ran fine all 24h. Not a real issue (P3 curiosity at most).
- **Downgraded P1→P3 — "decline-render path never exercised."** Real (0 declines in soak, all 26 resets `rewound=1`), but the path is a self-labeled "should be unreachable" safety net, crash-safe by construction (every draw is NULL/numSprites-guarded). Code-reviewed-only is acceptable; force-exercising it is a follow-up hardening item.

## Clean dimensions (no findings)

- **Visual** — `fgBackdropDrawIslandSprites` factored verbatim (identical sprite indices/positions); stand-family re-export verified by FG2 binary decode (old packs had empty frame 0, new packs have a real Johnny sprite); decline re-composite correct. Known false-positive traps correctly not flagged.
- **Debug hygiene** — the 5 new printfs are all rare reactive-rung diagnostics (dozens/day), not hot-loop; the per-frame decline block has zero printf; `PS1_FORCE_VISITOR3_FULLRESET` default-0 and fully inert; no new TODO/temp files.
- **Code review** — no UAF/double-free/NULL-deref/leak; bounded `goto` recursion (one-shot flags); non-PS1 build safe; abandoned wipe left no dead code.
- **Memory** — fuzz gate green; escalation ladder well-formed; `memCacheRewindIfEmpty` correctly refuses to rewind when `g_cacheUsed != 0` (no dangling-pointer risk).

## P3 / follow-ups (non-blocking)

`PS1_FORCE_VISITOR3_FULLRESET` undocumented outside source · `release-notes-0.9.4.md` missing for an existing tag · stand10 `START_FRAME` in export script but pack not re-exported (shipped pack fixed upstream — reconcile to avoid a future-re-export trap) · mem_sim doesn't model the `fgFullCacheReset` rung · no CD-error/memcard/real-hardware coverage.

## Recommended actions, in order

1. **(P0/P1, before announcing) Fix the site asset names** `jcreborn → johnnycastawayps1` in `site/play/index.md` + `_config.yml`, rebuild `docs/`, commit, push. Then re-probe the live Download URL → 200/302.
2. **(P2)** README + `/releases/` changelog → v0.9.7; `git rm --cached tests/mem_path_replay` + gitignore; clean the dirty tree.
3. **(P2/P3, post-release)** harden `grBlitToFramebuffer` no-halt + sim model; force-exercise the decline branch; soften the walk "never a crash" wording; reconcile stand10.

**Bottom line:** the *game* is release-quality; the *distribution page* is broken. Item 1 is the only true blocker and is a quick site-only fix.
