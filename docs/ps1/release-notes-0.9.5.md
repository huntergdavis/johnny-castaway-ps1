# Johnny Castaway PS1 — v0.9.5

A stability + polish point release on top of v0.9.4. The headline is memory
robustness: three distinct deep-soak CACHE-exhaustion BSOD classes are now
fixed or made non-fatal, the product is renamed, and a full 126-variant visual
audit confirmed the scene set is clean save for the small fixes below.

## Highlights

### Renamed → `johnnycastawayps1`
The host artifacts (`.bin`/`.cue`, build target, ELF/EXE) are now
`johnnycastawayps1` so the emulator title bar reads correctly. The on-disc
8.3 boot executable is `JCAST.EXE` and the ISO volume is `JOHNNY_CASTAWAY`.

### Memory stability (no more deep-soak blue screens)
- **visitor3** — the reactive withhold-rebuild was re-stranding because the
  scene reloaded its own SCR backdrop into the defragged hole; the retry now
  keeps the full-screen SCR cache disabled so the contiguous hole survives.
- **Black-scene wipe ("the end" / johnny6)** — transitioning into an all-black
  scene could leave the previous island on screen. A forced full-frame upload
  was being silently cleared by a residual-clean present path; the forced
  upload is now latched and honored unconditionally.
- **Frame-buffer strand (johnny6, ~15.5 h soak)** — johnny6's ~116 KB grow-only
  frame buffer could strand on deep-soak fragmentation and hit a fatal halt
  (it lacked the no-halt protection the clean-rect path has). It now recovers
  through the same withhold-rebuild retry.
- **Graceful clean-rect decline** — a ceiling scene whose clean-rect snapshot
  cannot be placed even after the withhold-rebuild now declines staging and
  renders crash-free instead of blue-screening.

### Memory card v7
Saves now persist audio + closed-caption settings, tide/raft overrides,
performance level, scene set, and island position (range-validated on load).

### Scene picker + menus
- The Sequential picker walks the explorer catalog in order (1, 2, 3, 4…).
- The orphaned **Scene Info** and **Credits** screens are wired into the
  System menu.

### Visual fixes (from the 126-variant audit)
- **activity10** — fixed the "ghost Johnny" silhouette via a frame-wide keyed
  re-export.
- **johnny3** — restored the thought-bubble connector dots (base-diff lane carve).
- **stand10** — the scene no longer opens on a blank island for 3 frames;
  Johnny is present from frame 0.

## Under the hood
- A reusable **scene visual-audit harness** (`scripts/audit-montage.sh` +
  `.claude/workflows/scene-visual-audit.mjs`) inspects every variant's host
  capture against a red-teamed defect checklist, reading each scene's validated
  description so intended drama is not flagged.
- The **memory simulator** now models each scene's grow-only frame buffer (it
  was previously blind to it, modeling frames as fixed pinned blocks). A new
  `build-mem-fuzz.sh` gate proves the shipping frame buffers survive while an
  enlarged-frame pack regression is caught.

## Validation
Production-like build (no special boot flags). Soaking for a day or two to
confirm the deep-soak fixes hold across a long run.
