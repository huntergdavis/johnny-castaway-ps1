# Holiday Expansion: 4 → 35 US Holidays

## Context

The PS1 build of Johnny Castaway has 4 holiday decorations (Halloween, St Patrick's, Christmas, New Year) baked into `HOLIDAY.PSB` and switched on at scene start by `islandInitHoliday()` in `src/island.c:248`. The owner wants to expand this to **35 US-centric holidays** distributed across the calendar year, using a design doc already produced (`docs/ps1/holidays-expansion-design.md`) with date rules, visual concepts, and color palettes for each.

The expansion has to handle:
- **Variable-size sprites per holiday** (creative latitude)
- **Movable feasts** (Easter, Mardi Gras, Thanksgiving, etc.) computed by **pure algorithm**, no expiring date tables — must work for 100+ years
- **AI-generated pixel art** in the same Sierra-game style as the originals
- **Existing 4 sprites preserved** — keep their dialed-in pixel art as-is
- **Owner review at each milestone** — HTML sprite sheets for visual sign-off before locking each batch

## Decisions locked

| # | Choice |
|---|---|
| 1 | Sprite sizes: variable per-holiday, authored alongside concept |
| 2 | Original 4 sprites: untouched |
| 3 | Movable feasts: pure-algorithm (Meeus Easter; Nth-weekday math) |
| 4 | Art generation: AI (Claude sub-agents) authoring pixel data |
| 5 | Per-holiday config: Python `holidays.yml` / `.py` data file → codegen → C struct + sprite sources |

## Phase plan

### Phase A — Per-holiday config + date-algorithm core (~2 hrs, no art yet)

1. **Author `holidays.yml`** — one entry per holiday from the design doc, with:
   - `id` (1-35), `name`, `description`
   - `date_rule` (one of: `fixed:MM-DD`, `nth_weekday:N,DOW,month`, `easter+offset:N`, `solstice:winter|summer`, `equinox:vernal|autumnal`)
   - `sprite_size` (W, H) chosen per concept
   - `island_xy` (anchor on screen)
   - `palette` (3-color hint from design doc, expanded to 16-entry CLUT later)
   - `existing_sprite` (yes for the original 4: index 0/1/2/3 in `HOLIDAY.PSB`)

2. **Codegen Python script** `scripts/holidays-codegen.py`:
   - Reads `holidays.yml`
   - Generates `src/holidays.c` — static const struct array of holiday metadata
   - Generates `src/holidays.h` — public API
   - Generates `scripts/holidays-art-spec.json` — input for the art-generation agent

3. **Date-algorithm core** in `src/holidays.c`:
   - Day-of-week from `(month, day, year)` via Zeller's congruence
   - `nth_weekday_of_month(n, dow, month, year)` → date
   - `easter_sunday(year)` via Meeus/Jones/Butcher (works 1583–4099)
   - `solstice/equinox` from astronomical formula (within ±1 day, fine for visuals)
   - `holiday_for_date(month, day, year)` → returns holiday id (0 = none)

4. **Wire into runtime** — extend `src/utils.c` `ps1HolidayFromDate(month, day)` to delegate to the new core. Drop the hardcoded 4-case switch.

### Phase B — Art generation pipeline (~3 hrs setup + iteration)

5. **Style reference extraction** — Python script reads existing `HOLIDAY.BMP`, dumps the 4 sprites as PNGs alongside their CLUTs. This becomes the **style reference** for the AI art agent. Document the pixel art aesthetic in `docs/ps1/holidays-style-guide.md` (saturation, line weight, palette discipline, level of detail per pixel-budget).

6. **AI art agent loop** (Claude sub-agents):
   - Input: style guide, design doc concept for one holiday, target sprite size, palette hint
   - Output: PNG sprite at target size, 4-bit indexed using a 16-color CLUT
   - Approach: agent authors a Python+PIL script that draws the sprite using primitive shapes + per-pixel data, in the Sierra style. Iterative — agent can be re-run with feedback until owner approves.
   - Batch the 31 new holidays in 4-5 review batches (say 7-8 per batch) to keep review tractable.

7. **HTML preview generator** `scripts/holidays-preview.py`:
   - Reads all generated PNGs
   - Emits a single HTML page: 35-cell grid, each cell shows holiday name + date rule + sprite at 4x zoom + concept text
   - Output: `scratch/holidays-preview.html` for in-browser review

### Phase C — Owner review + sign-off (per batch)

8. After each art batch:
   - Generate the HTML preview
   - Owner reviews, gives go/iterate/redo per sprite
   - Iterate any rejects with feedback, regenerate
   - Approve → freeze that batch's PNGs

9. After all 31 are approved → final preview with all 35 (4 originals + 31 new) for sign-off.

### Phase D — Asset packaging (~1 hr)

10. **PSB build** — extend the existing pipeline:
    - All 35 sprites packed into a single `HOLIDAY.PSB` (or split if VRAM/budget says so) via `scripts/transcode-bmp-ps1.py`
    - Re-generate the source `HOLIDAY.BMP` from the 35 PNGs (Python composes them into a single Sierra-format 4-bit BMP with a shared CLUT and a frame table)
    - Update `config/ps1/cd_layout.xml` if file size changed significantly

11. **Runtime wiring** — extend `src/island.c` `islandInitHoliday()`:
    - Replace the 4-case switch with a lookup into `gHolidays[]` (the codegen array)
    - Each entry has its own sprite index + island_xy → `grDrawSprite(layer, slot, x, y, sprite_idx, 0)`

### Phase E — Pause-menu integration (~30 min)

12. **Holiday menu cycling** — currently the Holiday menu item cycles through 6 fixed states (AUTO + 5). Refactor to cycle through 36 (AUTO + 35) using the `gHolidays[]` array. Wrap the on-screen label so it doesn't overflow the panel.

### Phase F — Smoke-test gauntlet (~1 hr)

13. **Force-each-holiday script**:
    - Headless DuckStation runs through all 35 holidays via boot tokens
    - Captures one screenshot per holiday into `scratch/holidays-smoke/`
    - Generates a final HTML contact sheet — every sprite rendered ON the actual island background, not just the bare BMP

14. Commit each phase separately. Push to main once Phase F passes.

## Files to be created or modified

**New:**
- `holidays.yml` (or `.json`) — per-holiday metadata authored by hand from design doc
- `scripts/holidays-codegen.py`
- `scripts/holidays-art-agent.py` — orchestrates the Claude sub-agent loop
- `scripts/holidays-preview.py`
- `scripts/extract-original-holiday-sprites.py` — style reference dump
- `src/holidays.c` / `.h` — generated from yaml
- `docs/ps1/holidays-style-guide.md` — style reference for AI

**Modified:**
- `src/island.c` — `islandInitHoliday` replaced with table-driven lookup
- `src/utils.c` — `ps1HolidayFromDate` delegates to the new core
- `src/pause_menu.c` — Holiday cycling expanded to 36 states
- `jc_resources/extracted/bmp/HOLIDAY.BMP` — regenerated to hold 35 sprites
- `jc_resources/transcoded/HOLIDAY.PSB` — re-transcoded
- `config/ps1/cd_layout.xml` — file-size update if needed

**Reused (no changes):**
- `scripts/transcode-bmp-ps1.py` (already converts BMP → PSB)
- `scripts/make-cd-image.sh` (already packages the disc)
- `src/psb_format.h` (PSB on-disc format, untouched)
- `src/graphics_ps1.c` `grLoadBmp` / `grDrawSprite` (already render sprites)

## Verification

- **Phase A:** `holiday_for_date(2026, 12, 25)` returns Christmas; `holiday_for_date(2026, 4, 5)` returns Easter (Meeus); `holiday_for_date(2050, 11, 24)` returns Thanksgiving (4th Thu Nov, computed). Unit tests if present, otherwise a Python test harness comparing against a known-correct date library.
- **Phase B:** HTML preview renders without missing assets. Generated PNGs use only the assigned palette.
- **Phase D:** `HOLIDAY.PSB` has 35 frames and all sizes match `holidays.yml`. CD image builds clean.
- **Phase F:** All 35 holidays render in DuckStation without crashes / VRAM corruption. Contact sheet looks right.

## Risks

- Each AI-generated sprite may need 2-3 iterations to look right → batch review is critical to avoid 35× rework
- Variable sizes mean each holiday's `island_xy` needs human eyeballs — could be tedious. Mitigated by reusing palm-tree anchor coords as a default and only adjusting when needed
- Original 4 sprites use specific palette entries — when we merge into a single 16-color CLUT for `HOLIDAY.PSB`, we may have to expand to 32 colors via per-frame CLUT (PSB supports this) or accept that some holidays share palette space
