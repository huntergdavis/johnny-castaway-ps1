# Memory region allocator — documentation index

Quick index of every doc related to the PS1 memory-region allocator,
in recommended reading order.

## If you're new to this work

Read in order:

1. **[memory-region-allocator-plan.md](./memory-region-allocator-plan.md)**
   — the current plan (v8). Start with the "What changed in v8" + Core
   Invariant + Goals/non-goals sections; skim the rest until you find
   the part relevant to your task. The plan has accumulated revisions
   from 6 rounds of panel review; the most current state is at the top.
2. **[mem-region-decision-tree.md](./mem-region-decision-tree.md)** — if
   you're about to write a `memAlloc` call, this is the canonical
   "which region?" reference.
3. **[adding-new-scenes-memory.md](./adding-new-scenes-memory.md)** — if
   you're adding a scene (base or holiday variant) that might affect
   memory budgets.
4. **[mem-region-phase-1-checklist.md](./mem-region-phase-1-checklist.md)**
   — if you're implementing Phase 1; tick items as you go.

## If you want the design rationale

Read the **red-team progression** to understand why each design
decision is where it is. Each red-team round closed the issues from
the prior round and surfaced new ones:

- **[red-team v1](./memory-region-allocator-red-team.md)** — initial
  technical pass on the v1 plan; found 3 architectural showstoppers.
- **[red-team v2](./memory-region-allocator-red-team-v2.md)** — first
  5-personality principal-engineer panel on the v3 plan; found 12
  blockers across 5 reviewers.
- **[red-team v3](./memory-region-allocator-red-team-v3.md)** — panel
  re-review of v4 (post BSOD-framework integration); 6 new blockers.
- **[red-team v4](./memory-region-allocator-red-team-v4.md)** — panel
  re-review of v5; 3 new blockers around memHalt / ps1DebugInit /
  fgLoopGetLastScene.
- **[red-team v5](./memory-region-allocator-red-team-v5.md)** — panel
  re-review of v6; 2 new blockers (variant timing + diagnostic
  continuity).
- **[red-team v6](./memory-region-allocator-red-team-v6.md)** — panel
  re-review of v7; first clean blocker pass.
- **[red-team v7](./memory-region-allocator-red-team-v7.md)** — panel
  re-review of v8; convergence verdict.

## File responsibilities at a glance

| File | Audience | Stable? |
|------|----------|---------|
| `memory-region-allocator-plan.md` | Implementer + reviewer | Stable once Phase 1 lands |
| `mem-region-decision-tree.md` | Any contributor writing memAlloc | Stable, update when new region added |
| `adding-new-scenes-memory.md` | Anyone adding a scene/variant | Stable |
| `mem-region-phase-1-checklist.md` | Phase 1 implementer | Discardable after Phase 1 ships |
| `mem-region-README.md` (this doc) | Anyone new to the work | Stable |
| `memory-region-allocator-red-team-*.md` | Historical / rationale | Append-only history |

## Status

Plan v8 is implementation-ready. Red-team v7 closed with zero blockers
and zero material risks. Phase 1 can begin.
