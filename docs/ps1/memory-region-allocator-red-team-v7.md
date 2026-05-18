# Memory region allocator — red team v7 (panel re-review of v8)

Panel reconvenes for loop 5. v8 closed 8 of 8 v6 findings (3 material +
5 hygiene). This pass: are we converged?

---

## Reviewer 1 — Pat, Embedded Systems Veteran

### Closed in v8

- **P20 (ps1IsMainContext bit-pattern + unit test):** Phase 1 sub-task
  with three explicit test cases. Done.
- **P21 (`__builtin_unreachable` dropped):** both branches. Done.
- **P22 (nm fallback removed):** Done.

### New concerns

**None.** Plan looks correct on PSX-specifics. The unit-test
approach for ps1IsMainContext (EnterCriticalSection + VBlank
callback) is exactly what I'd write.

---

## Reviewer 2 — Sarah, SRE / Operations

### Closed in v8

- **S17 (Python fixtures):** 5 cases + make target. Done.

### New concerns

**None new.** The Python script + fixture pattern is the right
shape for this CI gate.

---

## Reviewer 3 — Mateo, Future Maintainer

### Closed in v8

- **M17 (#ifdef clarification):** in the decision tree. Done.

### New concerns

#### 🟡 M18. The plan now spans 9 documents — orientation may be hard

Counting:
- `memory-region-allocator-plan.md` (the plan itself, v1-v8 trail)
- `memory-region-allocator-red-team{,-v2,-v3,-v4,-v5,-v6,-v7}.md`
  (this doc)
- `adding-new-scenes-memory.md`
- `mem-region-decision-tree.md`
- `mem-region-phase-1-checklist.md`

A new maintainer arriving cold has a lot to scan. A one-page
**README** at `docs/ps1/mem-region-README.md` summarizing what each
doc is for and the recommended reading order would help. Not a
blocker — it's literally just "here's the index." 5 minutes to
write.

---

## Reviewer 4 — Priya, Performance Engineer

### Closed in v8

- **A28 (pin-count delta cost):** in perf table. Done.

### New concerns

**None.** Performance accounting is clean.

---

## Reviewer 5 — Alex, Adversarial Tester

### Closed in v8

- **A26 (incomplete-enum approach):** committed. Done.
- **A27 (pickInProgress flag):** state machine + transitions
  specified. Done.

### New concerns

#### 🟡 A29. BSS-size math should be explicit

The plan adds a 1.2 MB region buffer to BSS. Current BSS is ~500 KB
(per the linker map showing `_end - exe_size ≈ 500 KB`). Adding the
region pushes BSS to ~1.7 MB. Plus exe (~129 KB) + stack (64 KB) =
~1.9 MB, fits within 1.92 MB usable.

But: the plan **doesn't subtract the existing dynamic buffers being
consolidated into the region**. `gWalkCleanBuf` (149 KB), frame
buffers (~150 KB max), surface pool (4 × ~50 KB) — all currently
malloc'd at runtime, not in BSS. Moving them into the region adds
their footprint to BSS. **The math is fine** (they were going to be
in RAM anyway, just shifting from heap to BSS), but the plan should
note this so a future maintainer doesn't try to reclaim "BSS budget"
that's not actually free.

Add a one-paragraph note in the budget section: "The region buffer is
new BSS; the per-region budget includes allocations that were
previously heap. Net delta-to-BSS is the full 1.2 MB."

---

## Synthesis — loop 5 panel verdict

**Closed from red-team v6:** 8 of 8 (100%).

**New blockers:** **0**.
**New material:** **0**.
**New hygiene:** **2** (M18 README index, A29 BSS-math note).

## Recommendation

**Plan is converged.** Two trivial hygiene items remain — both
documentation polish, neither blocks Phase 1 implementation. Address
them in v9 if you want a literally zero-finding pass, or accept v8 as
implementation-ready.

Panel's collective view: **v8 is ready to start Phase 1.** The two
🟡s are notes-to-self, not gates.
