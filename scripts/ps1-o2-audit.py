#!/usr/bin/env python3
"""Audit PS1 -O2/-Os state for the performance experiment queue."""

from __future__ import annotations

import argparse
import csv
import datetime as dt
import json
import os
import re
import shlex
import subprocess
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_COMPILE_COMMANDS = REPO_ROOT / "build-ps1" / "compile_commands.json"
DEFAULT_MAP = REPO_ROOT / "build-ps1" / "johnnycastawayps1.map"
DEFAULT_EXE = REPO_ROOT / "build-ps1" / "johnnycastawayps1.exe"
DEFAULT_ELF = REPO_ROOT / "build-ps1" / "johnnycastawayps1.elf"
DEFAULT_MD = REPO_ROOT / "docs" / "ps1" / "performance-o2-audit.md"
DEFAULT_CSV = REPO_ROOT / "docs" / "ps1" / "performance-o2-audit.csv"

OPT_RE = re.compile(r"^-O(?:0|1|2|3|s|g|fast|z)$")
ATTR_RE = re.compile(r'__attribute__\s*\(\(\s*optimize\s*\(\s*"([^"]+)"\s*\)\s*\)\)')
FUNC_NAME_RE = re.compile(r"([A-Za-z_][A-Za-z0-9_]*)\s*\(")

HOT_TU_ORDER = [
    "src/foreground_pilot/foreground_pilot.c",
    "src/jc_reborn.c",
    "src/resource/resource.c",
    "src/platform/ps1/sound_ps1.c",
    "src/platform/ps1/events_ps1.c",
]

COLD_TU_ORDER = [
    "src/pause_menu/pause_menu.c",
    "src/platform/ps1/ps1_captions.c",
    "src/platform/ps1/memcard.c",
    "src/scene/holidays.c",
    "src/platform/ps1/ps1_debug.c",
    "src/platform/ps1/ps1_stubs.c",
    "src/core/utils.c",
    "src/scene/island.c",
]

GRAPHICS_HELPERS = {
    "grUpdateDisplay",
    "grDrawBackground",
    "grRestoreBgFromRects",
    "grCompositePacked4SpansToBackground",
}

CD_HELPERS = {
    "ps1_streamReadFromCdFile",
    "ps1_streamReadAlignedFromCdFileInto",
}

REJECTED_DEFAULT_O2_FUNCTIONS = {
    "grCompositePacked4CompactTemporalResidualToBackground": (
        "Keep scoped -Os; current five-yellow default-O2 retest rejected",
        "Removing the scoped Os override stayed flat on VISITOR3-low but regressed B2-high and W1-high blocking/refill and converted small W1-low/V3-high loop wins into hidden refill/blocking debt.",
    ),
    "grDrawBackground": (
        "Keep scoped -Os; current v0.7.2 default-O2 retest rejected",
        "The current retest grew the upload helper by 504 bytes and did not improve loop timing.",
    ),
    "grUpdateDisplay": (
        "Keep scoped -Os; current v0.7.2 default-O2 retest rejected",
        "The current retest grew the display wrapper by 40 bytes and did not improve key timing.",
    ),
    "ps1_streamReadAlignedFromCdFileInto": (
        "Keep scoped -Os; current v0.7.2 default-O2 retest rejected",
        "The current retest grew the active aligned-read helper by 524 bytes and stayed key-flat.",
    ),
    "ps1_streamReadFromCdFile": (
        "Keep scoped -Os; current v0.7.2 default-O2 retest rejected",
        "The current retest grew the setup/unbuffered stream helper by 68 bytes and regressed FISHING1.",
    ),
    "ps1PerfEndScene": (
        "Keep scoped -Os; current four-yellow code-headroom canary accepted",
        "Scoped Os plus default JCPERF2-only reporting shrank scene-end perf code while keeping the PS-EXE bucket, pack LBAs, and all four under-green rows exact-flat.",
    ),
}

REJECTED_DEFAULT_O2_TRANSLATION_UNITS = {
    "src/foreground_pilot/foreground_pilot.c": (
        "Keep whole TU at -Os; historical default-O2 retest rejected",
        "Whole-TU -O2 grew foregroundPilotPlay and failed structurally before scene-end metrics.",
    ),
    "src/jc_reborn.c": (
        "Keep whole TU at -Os; historical default-O2 retest rejected",
        "Whole-TU -O2 stayed exact-flat while growing ELF and shifting hot symbols.",
    ),
    "src/resource/resource.c": (
        "Keep whole TU at -Os; historical default-O2 retest rejected",
        "Whole-TU -O2 stayed exact-flat while growing ELF and shifting foreground symbols.",
    ),
    "src/platform/ps1/sound_ps1.c": (
        "Keep whole TU at -Os; historical default-O2 retest rejected",
        "Whole-TU -O2 stayed exact-flat while growing ELF and shifting CD helper symbols.",
    ),
    "src/platform/ps1/events_ps1.c": (
        "Keep whole TU at -Os; historical default-O2 retest rejected",
        "Whole-TU -O2 stayed exact-flat while growing ELF and shifting CD helper symbols.",
    ),
    "src/platform/ps1/ps1_stubs.c": (
        "Keep whole TU at -Os; current v0.7.2 default-O2 retest rejected",
        "Whole-TU -O2 stayed exact-flat while growing ELF and the stubs object.",
    ),
    "src/pause_menu/pause_menu.c": (
        "Keep whole TU at -Os; current v0.7.2 default-O2 retest rejected",
        "Whole-TU -O2 grew the PS-EXE bucket, shifted foreground LBAs, and regressed canaries.",
    ),
    "src/platform/ps1/ps1_captions.c": (
        "Keep whole TU at -Os; current v0.7.2 default-O2 retest rejected",
        "Whole-TU -O2 stayed FISHING1-flat but grew ELF and the captions object.",
    ),
    "src/platform/ps1/memcard.c": (
        "Keep whole TU at -Os; current v0.7.2 default-O2 retest rejected",
        "Whole-TU -O2 regressed FISHING1 visible CD pressure while growing ELF and the memcard object.",
    ),
    "src/scene/holidays.c": (
        "Keep whole TU at -Os; current v0.7.2 default-O2 retest rejected",
        "Whole-TU -O2 regressed FISHING1 visible CD pressure while growing ELF and the holidays object.",
    ),
    "src/platform/ps1/ps1_debug.c": (
        "Keep whole TU at -Os; current v0.8.0 default-O2 retest rejected",
        "Whole-TU -O2 stayed FISHING1-flat against the refreshed v0.8.0 baseline but grew ELF with no work or speed win.",
    ),
    "src/core/utils.c": (
        "Keep whole TU at -Os; current v0.8.0 default-O2 retest rejected",
        "Whole-TU -O2 stayed FISHING1-flat but grew ELF and shifted tracked hot symbols by 20 bytes with no work or speed win.",
    ),
    "src/scene/island.c": (
        "Keep whole TU at -Os; current v0.8.0 default-O2 retest rejected",
        "Whole-TU -O2 stayed FISHING1-flat but grew ELF and shifted tracked graphics/CD symbols by 48 bytes with no work or speed win.",
    ),
    "src/platform/ps1/ps1_pad_script.c": (
        "Keep whole TU at -Os; current v0.8.0 default-O2 retest rejected",
        "Whole-TU -O2 stayed FISHING1-flat but grew ELF and shifted tracked CD helper symbols by 36 bytes with no work or speed win.",
    ),
    "src/scene_freeplay/scene_freeplay.c": (
        "Keep whole TU at -Os; current v0.8.0 default-O2 retest rejected",
        "Whole-TU -O2 grew the PS-EXE bucket, shifted foreground LBAs, and gave no active-scene timing win.",
    ),
    "src/scene/scene_picker.c": (
        "Keep whole TU at -Os; current v0.8.0 default-O2 retest rejected",
        "Whole-TU -O2 stayed FISHING1-flat with fixed tracked hot symbols but grew ELF with no work or speed win.",
    ),
}


def relpath(path_text: str) -> str:
    if not path_text:
        return ""
    path_text = path_text.split(":", 1)[0]
    if path_text.startswith("/project/"):
        return path_text[len("/project/") :]
    path = Path(path_text)
    try:
        return path.resolve().relative_to(REPO_ROOT).as_posix()
    except (OSError, ValueError):
        return path_text


def git_value(*args: str) -> str:
    try:
        return subprocess.check_output(
            ["git", *args], cwd=REPO_ROOT, text=True, stderr=subprocess.DEVNULL
        ).strip()
    except (OSError, subprocess.CalledProcessError):
        return "unknown"


def file_size(path: Path) -> int:
    return path.stat().st_size if path.is_file() else 0


def parse_compile_commands(path: Path) -> list[dict[str, object]]:
    data = json.loads(path.read_text(encoding="utf-8"))
    rows: list[dict[str, object]] = []
    for entry in data:
        tokens = shlex.split(entry.get("command", ""))
        opt_flags = [token for token in tokens if OPT_RE.match(token)]
        final_opt = opt_flags[-1] if opt_flags else ""
        output = ""
        if "-o" in tokens:
            output_index = tokens.index("-o")
            if output_index + 1 < len(tokens):
                output = tokens[output_index + 1]
        source = relpath(entry.get("file", ""))
        object_path = Path(output)
        if not object_path.is_absolute():
            object_path = REPO_ROOT / "build-ps1" / object_path
        rows.append(
            {
                "source": source,
                "final_opt": final_opt,
                "opt_flags": " ".join(opt_flags),
                "object": relpath(object_path.as_posix()),
                "object_bytes": file_size(object_path),
            }
        )
    return rows


def parse_function_attributes(source_root: Path) -> list[dict[str, object]]:
    rows: list[dict[str, object]] = []
    for path in sorted((source_root / "src").glob("*.c")):
        lines = path.read_text(encoding="utf-8", errors="ignore").splitlines()
        for index, line in enumerate(lines):
            match = ATTR_RE.search(line)
            if not match:
                continue
            signature = line[match.end() :].strip()
            for lookahead in range(index + 1, min(index + 8, len(lines))):
                signature += " " + lines[lookahead].strip()
                if "{" in lines[lookahead]:
                    break
            prefix = signature.split("{", 1)[0]
            names = FUNC_NAME_RE.findall(prefix)
            function_name = names[-1] if names else "unknown"
            rows.append(
                {
                    "source": path.relative_to(source_root).as_posix(),
                    "line": index + 1,
                    "function": function_name,
                    "optimize": match.group(1),
                }
            )
    return rows


def parse_map(path: Path) -> tuple[dict[str, dict[str, object]], dict[str, int]]:
    symbols: dict[str, dict[str, object]] = {}
    source_text_sizes: dict[str, int] = {}
    if not path.is_file():
        return symbols, source_text_sizes
    for raw in path.read_text(encoding="utf-8", errors="ignore").splitlines():
        parts = raw.split(None, 4)
        if len(parts) < 4:
            continue
        name, sym_type, address_text, size_text = parts[:4]
        try:
            address = int(address_text, 16) & 0xFFFFFFFF
            size = int(size_text, 16)
        except ValueError:
            continue
        source = relpath(parts[4]) if len(parts) >= 5 else ""
        symbols[name] = {
            "type": sym_type,
            "address": address,
            "size": size,
            "source": source,
        }
        if source and sym_type.lower() == "t":
            source_text_sizes[source] = source_text_sizes.get(source, 0) + size
    return symbols, source_text_sizes


def classify_tu(source: str) -> str:
    if source in HOT_TU_ORDER:
        return "hot"
    if source in COLD_TU_ORDER:
        return "cold"
    if source in {
        "src/graphics_ps1/graphics_ps1.c",
        "src/cdrom_ps1.c",
        "src/platform/ps1/ps1_perf.c",
        "src/walk/walk_render.c",
    }:
        return "hot-default-o2"
    return "default-o2"


def priority_for_function(function: str, source: str) -> tuple[int, str, str]:
    if function in REJECTED_DEFAULT_O2_FUNCTIONS:
        action, reason = REJECTED_DEFAULT_O2_FUNCTIONS[function]
        return (90, action, reason)
    if function in GRAPHICS_HELPERS:
        return (
            10,
            "Test scoped helper at default -O2",
            "Graphics helpers are the first free-speed candidate before C rewrites.",
        )
    if function in CD_HELPERS:
        return (
            40,
            "Retest only after graphics helper sweep",
            "CD helper codegen is phase-sensitive; keep behind graphics scoped probes.",
        )
    return (
        90,
        "Review manually",
        f"Function-scoped optimize attribute in {source} needs explicit owner review.",
    )


def priority_for_translation_unit(source: str, source_class: str, order_index: dict[str, int]) -> tuple[int, str, str]:
    if source in REJECTED_DEFAULT_O2_TRANSLATION_UNITS:
        action, reason = REJECTED_DEFAULT_O2_TRANSLATION_UNITS[source]
        return (90, action, reason)
    if source_class == "hot":
        return (
            20 + order_index.get(source, 99),
            "Test whole TU at default -O2",
            "Hot or semi-hot TU is currently forced to -Os; speed may beat size.",
        )
    if source_class == "cold":
        return (
            60 + order_index.get(source, 99),
            "Test whole TU at default -O2 after hot sweep",
            "Cold/default-off TU may perturb phase; record outcome after hot targets.",
        )
    return (
        80,
        "Review TU optimization flag",
        "Source is size-optimized but not classified in the current sweep order.",
    )


def build_candidates(
    compile_rows: list[dict[str, object]],
    attr_rows: list[dict[str, object]],
    symbols: dict[str, dict[str, object]],
    source_text_sizes: dict[str, int],
) -> list[dict[str, object]]:
    candidates: list[dict[str, object]] = []
    for attr in attr_rows:
        function = str(attr["function"])
        source = str(attr["source"])
        priority, action, reason = priority_for_function(function, source)
        symbol = symbols.get(function, {})
        candidates.append(
            {
                "priority": priority,
                "target_type": "function",
                "target": function,
                "source": source,
                "current_opt": f'optimize("{attr["optimize"]}")',
                "symbol_address": f'0x{int(symbol.get("address", 0)):08x}' if symbol else "",
                "symbol_size_bytes": int(symbol.get("size", 0)) if symbol else 0,
                "source_text_bytes": source_text_sizes.get(source, 0),
                "next_action": action,
                "reason": reason,
            }
        )

    order_index = {source: i for i, source in enumerate(HOT_TU_ORDER + COLD_TU_ORDER)}
    for row in compile_rows:
        source = str(row["source"])
        final_opt = str(row["final_opt"])
        if final_opt != "-Os":
            continue
        source_class = classify_tu(source)
        priority, action, reason = priority_for_translation_unit(source, source_class, order_index)
        candidates.append(
            {
                "priority": priority,
                "target_type": "translation_unit",
                "target": source,
                "source": source,
                "current_opt": final_opt,
                "symbol_address": "",
                "symbol_size_bytes": 0,
                "source_text_bytes": source_text_sizes.get(source, 0),
                "next_action": action,
                "reason": reason,
            }
        )
    return sorted(candidates, key=lambda item: (int(item["priority"]), str(item["target"])))


def markdown_table(headers: list[str], rows: list[list[object]]) -> str:
    out = [
        "| " + " | ".join(headers) + " |",
        "| " + " | ".join("---" for _ in headers) + " |",
    ]
    for row in rows:
        out.append("| " + " | ".join(str(value) for value in row) + " |")
    return "\n".join(out)


def write_csv(path: Path, candidates: list[dict[str, object]]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    fields = [
        "priority",
        "target_type",
        "target",
        "source",
        "current_opt",
        "symbol_address",
        "symbol_size_bytes",
        "source_text_bytes",
        "next_action",
        "reason",
    ]
    with path.open("w", encoding="utf-8", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=fields, lineterminator="\n")
        writer.writeheader()
        writer.writerows({field: row.get(field, "") for field in fields} for row in candidates)


def write_markdown(
    path: Path,
    compile_rows: list[dict[str, object]],
    attr_rows: list[dict[str, object]],
    candidates: list[dict[str, object]],
    exe: Path,
    elf: Path,
    map_path: Path,
    csv_path: Path,
) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    opt_counts: dict[str, int] = {}
    for row in compile_rows:
        opt = str(row["final_opt"] or "unknown")
        opt_counts[opt] = opt_counts.get(opt, 0) + 1

    ps_exe_bytes = file_size(exe)
    ps_exe_sectors = (ps_exe_bytes + 2047) // 2048 if ps_exe_bytes else 0
    ps_exe_bucket = ps_exe_sectors * 2048
    generated_at = dt.datetime.now(dt.timezone.utc).replace(microsecond=0).isoformat()
    commit = git_value("rev-parse", "--short", "HEAD")
    branch = git_value("branch", "--show-current")

    top_rows = [
        [
            row["priority"],
            row["target_type"],
            f'`{row["target"]}`',
            row["current_opt"],
            row["symbol_size_bytes"],
            row["source_text_bytes"],
            row["next_action"],
        ]
        for row in candidates[:16]
    ]

    compile_summary = [
        [
            f'`{row["source"]}`',
            row["final_opt"],
            row["object_bytes"],
            row["opt_flags"],
        ]
        for row in sorted(compile_rows, key=lambda item: str(item["source"]))
    ]

    attr_summary = [
        [
            f'`{row["source"]}:{row["line"]}`',
            f'`{row["function"]}`',
            f'`optimize("{row["optimize"]}")`',
        ]
        for row in attr_rows
    ]

    text = f"""# PS1 `-O2` Audit

> Generated by `scripts/ps1-o2-audit.py`. Re-run after any build-system,
> toolchain, or source-level optimization flag change.

| Field | Value |
|---|---|
| Generated at | `{generated_at}` |
| Branch | `{branch}` |
| Commit | `{commit}` |
| Compile database | `{DEFAULT_COMPILE_COMMANDS.relative_to(REPO_ROOT)}` |
| Map file | `{map_path.relative_to(REPO_ROOT)}` |
| PS-EXE bytes | `{ps_exe_bytes}` |
| PS-EXE sector bucket bytes | `{ps_exe_bucket}` |
| ELF bytes | `{file_size(elf)}` |
| Map bytes | `{file_size(map_path)}` |
| Translation units at `-O2` | `{opt_counts.get("-O2", 0)}` |
| Translation units at `-Os` | `{opt_counts.get("-Os", 0)}` |
| Function-scoped optimize attributes | `{len(attr_rows)}` |
| Candidate CSV | [`{csv_path.relative_to(REPO_ROOT)}`](performance-o2-audit.csv) |

## Immediate Probe Order

{markdown_table(["Priority", "Type", "Target", "Current", "Symbol bytes", "Source text bytes", "Next action"], top_rows)}

## Function-Scoped Flags

{markdown_table(["Location", "Function", "Current attribute"], attr_summary)}

## Translation Unit Flags

{markdown_table(["Source", "Final optimization", "Object bytes", "Optimization flags"], compile_summary)}

## Acceptance Notes

- Test one target per experiment and compare against a fresh baseline with the
  normal headless perf gate.
- Keep PS-EXE sector bucket, foreground LBA, work identity, visual output, and
  audio output fixed unless an experiment explicitly allows a layout change.
- Broad `-O3` remains out of scope; this audit is for reverting existing `-Os`
  overrides back to default `-O2`.
"""
    path.write_text(text, encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--compile-commands", type=Path, default=DEFAULT_COMPILE_COMMANDS)
    parser.add_argument("--map", type=Path, default=DEFAULT_MAP)
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    parser.add_argument("--elf", type=Path, default=DEFAULT_ELF)
    parser.add_argument("--md", type=Path, default=DEFAULT_MD)
    parser.add_argument("--csv", type=Path, default=DEFAULT_CSV)
    args = parser.parse_args()

    if not args.compile_commands.is_file():
        raise SystemExit(f"missing compile database: {args.compile_commands}")
    if not args.map.is_file():
        raise SystemExit(f"missing map file: {args.map}")

    compile_rows = parse_compile_commands(args.compile_commands)
    attr_rows = parse_function_attributes(REPO_ROOT)
    symbols, source_text_sizes = parse_map(args.map)
    candidates = build_candidates(compile_rows, attr_rows, symbols, source_text_sizes)

    write_csv(args.csv, candidates)
    write_markdown(
        args.md,
        compile_rows,
        attr_rows,
        candidates,
        args.exe,
        args.elf,
        args.map,
        args.csv,
    )
    print(f"Wrote {os.fspath(args.md)}")
    print(f"Wrote {os.fspath(args.csv)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
