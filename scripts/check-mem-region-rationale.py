#!/usr/bin/env python3
"""
check-mem-region-rationale.py — CI gate enforcing that every
memAlloc(REGION, ...) call site has a preceding MEM_REGION_RATIONALE
comment.

Per plan v9 step 20 / S14. Avoids the false-positive issues of a
naive grep:
- handles multi-line memAlloc calls (split across newlines)
- handles same-line comments (rationale on the same line)
- detects macros whose body contains memAlloc (prohibited per A24)

Exit codes:
- 0 if all call sites are properly annotated.
- 1 if any unannotated call site is found.
- 2 if a macro wrapping memAlloc is detected.

Usage:
    python3 scripts/check-mem-region-rationale.py [path...]

If no paths given, scans src/.
"""
from __future__ import annotations

import re
import sys
from pathlib import Path


RATIONALE_RE = re.compile(r"MEM_REGION_RATIONALE:")
ALLOC_RE     = re.compile(r"\bmemAlloc\s*\(")
MACRO_RE     = re.compile(r"^\s*#\s*define\s+\w+.*\bmemAlloc\s*\(", re.MULTILINE)

# Files whose memAlloc calls don't need RATIONALE comments (the
# allocator implementation itself + helper code that's already
# documented in its own way).
WHITELIST = {
    "src/mem_region.c",
    "src/mem_region/mem_region_extern.h",
    "src/mem_region/mem_region.h",
}


def check_file(path: Path) -> list[str]:
    """Returns a list of human-readable issue strings for one file."""
    rel = str(path).replace("\\", "/")
    if any(rel.endswith(w) for w in WHITELIST):
        return []
    try:
        src = path.read_text(errors="replace")
    except OSError:
        return []

    issues: list[str] = []

    # A24: detect macros that expand to a memAlloc call.
    for m in MACRO_RE.finditer(src):
        line_no = src.count("\n", 0, m.start()) + 1
        issues.append(
            f"{path}:{line_no}: A24 violation — macro definition expands to memAlloc; "
            "wrappers around memAlloc are prohibited"
        )

    # For each memAlloc(...) call site, look for a MEM_REGION_RATIONALE
    # comment in the 10 preceding lines OR on the same line.
    lines = src.splitlines()
    for i, line in enumerate(lines):
        if not ALLOC_RE.search(line):
            continue
        # Skip if the alloc is inside a comment line (basic heuristic).
        stripped = line.strip()
        if stripped.startswith("//") or stripped.startswith("*"):
            continue
        # Look for rationale on this line or up to 10 lines above.
        # 10 accommodates multi-line memAlloc call signatures plus a
        # multi-line rationale block above the previous call in the same
        # cluster (e.g., a pair of related allocations).
        found = False
        for j in range(max(0, i - 10), i + 1):
            if RATIONALE_RE.search(lines[j]):
                found = True
                break
        if not found:
            issues.append(
                f"{path}:{i + 1}: memAlloc call missing MEM_REGION_RATIONALE "
                f"comment within the preceding 10 lines"
            )

    return issues


def main(argv: list[str]) -> int:
    paths = [Path(p) for p in argv] if argv else [Path("src")]

    files: list[Path] = []
    for p in paths:
        if p.is_dir():
            files.extend(p.rglob("*.c"))
            files.extend(p.rglob("*.h"))
        elif p.is_file():
            files.append(p)

    all_issues: list[str] = []
    macro_violations = 0
    for f in files:
        issues = check_file(f)
        for issue in issues:
            if "A24" in issue:
                macro_violations += 1
            all_issues.append(issue)

    if all_issues:
        print("MEM_REGION_RATIONALE check failed:", file=sys.stderr)
        for issue in all_issues:
            print(f"  {issue}", file=sys.stderr)
        return 2 if macro_violations else 1

    print(f"OK: {len(files)} files scanned, all memAlloc sites annotated.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
