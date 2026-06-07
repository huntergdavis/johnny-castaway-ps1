# Memory region allocator — red team v6 (panel re-review of v7)

Same panel reconvenes for loop 4. v7 closed 12 of 12 v5 findings.
Looking for new issues from v7's resolutions. Convergence likely
this loop or next.

---

## Reviewer 1 — Pat, Embedded Systems Veteran

### Closed in v7

- **P17 (ps1IsMainContext):** implementation specified. *Mostly* done —
  see below for the bit-pattern caveat.
- **P18 (toolchain compat):** verified for PsnoobSDK GCC 12+. Done.
- **P19 (drop `__builtin_unreachable`):** partially done — see below.

### New concerns

#### 🟠 P20. `ps1IsMainContext()` bit-pattern needs verification against R3000A docs

v7 says:
```c
return ((cause & 0x7C) == 0) && (sr & 0x1);
```

R3000A's CAUSE register's `ExcCode` field is in bits 2-6 (mask 0x7C
matches). But SR layout on R3000A is *not* the same as MIPS32:

- Bit 0 of SR is `IEc` (current interrupt enable) on both — OK.
- But the proper "are we in an exception handler" check on R3000A is
  whether we're in the *current* mode (KUc=0 → kernel, KUc=1 → user)
  or whether `IEc` is restored from `IEp`/`IEo` after `RFE` returns.
- A safer check is: read `EPC` register; if `(SR & 0x2)` indicates
  "interrupts disabled by exception entry," we're inside an
  exception. On R3000A this involves the `BD` bit and the IE-stack
  shuffling.

The check as written is *probably* correct for the common case (main
flow has SR.IEc=1) but Phase 1 must verify against PSX docs and add a
unit test that exercises the function from inside a synthetic VBlank
ISR (to prove it returns 0 in that context).

#### 🟠 P21. `__builtin_unreachable` after `ps1Bsod` is also redundant

P19 dropped the annotation after `while(1)` but kept it after
`ps1Bsod`. **`ps1Bsod` is `__attribute__((noreturn))`** — the compiler
already infers unreachable after it. Drop both annotations for
consistency.

#### 🟡 P22. `-Wglobal-constructors` fallback via `nm` is flaky

Plan's fallback: `nm jcreborn.elf | grep -E "_init|__init"`. MIPS
toolchains emit constructors as `.init_array` entries, not as named
symbols. The grep returns nothing whether or not constructors exist.

Cleanest fix: drop the fallback. PsyQ/PsnoobSDK is on GCC 12; the
flag works. If the toolchain ever loses support, fix it then.

---

## Reviewer 2 — Sarah, SRE / Operations

### Closed in v7

- **S14 (Python AST script):** committed to. Done.
- **S15 (gates re-categorized):** moved to Phase 1 milestones. Done.

### New concerns

#### 🟡 S17. Python script needs its own test coverage

`scripts/check-mem-region-rationale.py` (~30 LOC) is a new CI gate.
If the script itself has a bug, CI either false-passes (bad calls
ship) or false-fails (good PRs blocked). Phase 1 needs a small fixture
directory with:
- Valid example (RATIONALE comment present, well-formed memAlloc)
- Invalid: missing comment
- Invalid: comment too far above (>3 lines)
- Invalid: macro wrapping memAlloc
- Valid: multi-line memAlloc with comment on the right line

Run the script against each fixture in CI. ~1 hour of Phase 1 work;
not a blocker but trivial to do right.

---

## Reviewer 3 — Mateo, Future Maintainer

### Closed in v7

- **M15 (variant decision tree):** three-way split. Done.
- **M16 (writing burden noted):** in decision tree. Done.

### New concerns

#### 🟡 M17. "No conditional region choice" — what about `#ifdef PS1_BUILD`?

Decision tree's "Prohibitions" section says:
> No conditional region choice. A given allocation site must always go
> to the same region.

But a PC vs PS1 branch is fine — different platforms, different
constraints. Clarify: prohibition is on **runtime** conditional
region choice, not build-time `#ifdef` branches that yield distinct
textual call sites per platform.

---

## Reviewer 4 — Priya, Performance Engineer

### Closed in v7

- **PR14 (variant lookup):** fgLoopApplyVariant returns effective name.
  Done.
- **PR15 (MEM_REQUIRE on memFree):** documented + quantified. Done.

### New concerns

None new. v7 perf is well-quantified.

---

## Reviewer 5 — Alex, Adversarial Tester

### Closed in v7

- **A22 (fgLoopGetLastScene target-vs-played):** two-slot state. Done.
- **A23 (forward-decl drift):** mem_region_extern.h. Mostly done —
  see below.
- **A24 (macros wrapping memAlloc):** prohibited. Done.
- **A25 (pin-count delta logging):** in step 26. Done.

### New concerns

#### 🟠 A26. `mem_region_extern.h` header layering still ambiguous

v7 says: "`src/mem_region/mem_region_extern.h` holds forward declarations
consumed by both `mem_region.c` and `ps1_debug.c`."

But the forward decls use `MemRegion` (the enum):
```c
extern size_t memRegionUsed(MemRegion);
```

`MemRegion` is defined in `mem_region.h`. So `mem_region_extern.h`
must either (a) include `mem_region.h`, (b) forward-declare the enum,
or (c) use a typedef'd integer alias.

Pick one in v8:
- **A:** circular-include risk if `mem_region.h` ever includes
  `mem_region_extern.h` for its own definitions.
- **B:** `enum MemRegion;` as an incomplete type — works for function
  declarations (the compiler doesn't need the size for declaration),
  but doesn't let callers use the enum constants.
- **C:** `typedef int MemRegionId;` in mem_region_extern.h. Callers
  use `int` casting. Less clean but unambiguous.

Recommendation: **B**, with a comment explaining the incomplete-enum
trick. Compilers handle it correctly per C99/C11.

#### 🟠 A27. `fgLoopGetLastScene` needs a "pick in progress" flag

Two-slot state (`lastTarget`, `lastPlayed`) doesn't tell
`fgLoopGetLastScene` *which* to return. v7 says "returns target if a
pick is in progress, else last played." There's no flag for that.

Implementation:
```c
static int pickInProgress = 0;
static const char *lastTarget = NULL;
static const char *lastPlayed = NULL;

const char *fgLoopGetLastScene(void) {
    return pickInProgress ? lastTarget : lastPlayed;
}

/* In fgLoopNextScene: */
pickInProgress = 1;
lastTarget = chosenSlug;

/* In jc_reborn.c main loop, AFTER foregroundPilotPlay returns: */
lastPlayed = lastTarget;
pickInProgress = 0;
```

Three-state machine: idle → picking (pickInProgress=1) → playing →
idle. Add to Phase 1 step 19 explicitly so the implementer doesn't
miss the flag.

#### 🟡 A28. Pin-count delta sum cost

Step 26: at every scene transition, sum all 200+ resource
`pinCount` fields and compare to a stored previous sum. ~600 cycles
per transition (200 × 3 cycles for read + compare). Negligible but
worth a one-liner in the perf table.

---

## Synthesis — loop 4 panel verdict

**Closed from red-team v5:** 12 of 12. Clean.

**New blockers:** **0**. This is the first clean pass on blockers.

**New material risks (3):**

- P20: ps1IsMainContext bit-pattern needs verification + a unit test
  (one Phase 1 sub-task)
- A26: mem_region_extern.h must specify how to handle the MemRegion
  enum (one-line plan refinement)
- A27: fgLoopGetLastScene needs a `pickInProgress` flag (~5 lines in
  Phase 1 step 19)

**New hygiene (4):**

- P21: drop `__builtin_unreachable` after ps1Bsod too (consistency)
- P22: drop the `nm`-based -Wglobal-constructors fallback
- S17: Python script needs CI fixtures
- M17: clarify "no conditional region" excludes `#ifdef` platform branches
- A28: note pin-count delta sum cost in perf table

## Recommendation

**Architectural convergence achieved.** Zero new blockers. All open
items are small implementation refinements that fit in a one-day v8
revision. Confidence is high that v8 + one final red-team will land
at zero findings.
