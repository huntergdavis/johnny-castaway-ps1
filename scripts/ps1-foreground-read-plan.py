#!/usr/bin/env python3
"""Rank foreground pack read/setup candidates from existing perf artifacts.

This is host-side only. It consumes a ps1-perf-iterate summary, the matching
DuckStation log, and an FG2/FGP3 pack, then reports which file-sector ranges
are still read during the active loop after current setup-prime coverage.
"""

from __future__ import annotations

import argparse
import importlib.util
import json
import math
import re
from pathlib import Path
from typing import Any


SECTOR_SIZE = 2048
REPO_ROOT = Path(__file__).resolve().parents[1]


def load_cdlog_module() -> Any:
    module_path = Path(__file__).with_name("ps1-perf-cdlog-summary.py")
    spec = importlib.util.spec_from_file_location("_ps1_perf_cdlog_summary", module_path)
    if spec is None or spec.loader is None:
        raise SystemExit(f"unable to load {module_path}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def load_summary(path: Path | None) -> dict[str, Any]:
    if path is None:
        return {}
    return json.loads(path.read_text(encoding="utf-8"))


def first_case(summary: dict[str, Any]) -> dict[str, Any]:
    cases = summary.get("cases")
    if isinstance(cases, list) and cases:
        return cases[0]
    return summary


def int_from_case(case: dict[str, Any], section: str, key: str) -> int | None:
    value = case.get("sections", {}).get(section, {}).get(key)
    return value if isinstance(value, int) else None


def parse_sector_range(value: str) -> tuple[int, int]:
    if ":" not in value:
        raise argparse.ArgumentTypeError("range must be START:END")
    start_text, end_text = value.split(":", 1)
    try:
        start = int(start_text, 0)
        end = int(end_text, 0)
    except ValueError as exc:
        raise argparse.ArgumentTypeError(str(exc)) from exc
    if start < 0 or end <= start:
        raise argparse.ArgumentTypeError("range must be a positive half-open START:END")
    return start, end


def merge_ranges(ranges: list[tuple[int, int]]) -> list[tuple[int, int]]:
    if not ranges:
        return []
    merged: list[tuple[int, int]] = []
    cur_start, cur_end = sorted(ranges)[0]
    for start, end in sorted(ranges)[1:]:
        if start <= cur_end:
            cur_end = max(cur_end, end)
        else:
            merged.append((cur_start, cur_end))
            cur_start, cur_end = start, end
    merged.append((cur_start, cur_end))
    return merged


def range_contains(ranges: list[tuple[int, int]], start: int, end: int) -> bool:
    return any(start >= cover_start and end <= cover_end for cover_start, cover_end in ranges)


def range_overlaps(ranges: list[tuple[int, int]], start: int, end: int) -> bool:
    return any(start < cover_end and end > cover_start for cover_start, cover_end in ranges)


def numeric_values(values: list[Any]) -> list[float]:
    return [float(value) for value in values if isinstance(value, (int, float))]


def phase_risk_hint(fully_covered_reads: list[dict[str, Any]]) -> str:
    if len(fully_covered_reads) <= 1:
        return "single-read-or-padding"

    gaps = numeric_values([segment.get("prev_time_delta_s") for segment in fully_covered_reads])
    if not gaps:
        return "unknown-gap"

    min_gap = min(gaps)
    max_gap = max(gaps)
    if min_gap < 0.5:
        return "high-tight-cluster"
    if max_gap < 1.0:
        return "medium-no-long-gap"
    return "medium-validate-phase"


def eval_c_int_expr(expr: str, symbols: dict[str, int]) -> int | None:
    text = re.sub(r"/\*.*?\*/", "", expr)
    text = re.sub(r"//.*", "", text)
    text = re.sub(r"\b([0-9]+)[uUlL]*\b", r"\1", text)
    for name, value in sorted(symbols.items(), key=lambda item: len(item[0]), reverse=True):
        text = re.sub(rf"\b{re.escape(name)}\b", str(value), text)
    if re.search(r"[A-Za-z_]", text):
        return None
    if not re.fullmatch(r"[0-9\s+\-*/%()<>|&~]+", text):
        return None
    try:
        value = eval(text, {"__builtins__": {}}, {})
    except Exception:
        return None
    return int(value) if isinstance(value, int) and value >= 0 else None


def parse_source_setup_policy() -> dict[str, Any]:
    source_path = REPO_ROOT / "src" / "foreground_pilot.c"
    try:
        source = source_path.read_text(encoding="utf-8")
    except OSError:
        return {}

    symbols: dict[str, int] = {"FG_CD_SECTOR_SIZE": SECTOR_SIZE}
    define_pattern = re.compile(r"^#define\s+([A-Za-z_][A-Za-z0-9_]*)\s+(.+)$")
    for _ in range(4):
        changed = False
        for line in source.splitlines():
            match = define_pattern.match(line.strip())
            if match is None:
                continue
            name, expr = match.groups()
            if name in symbols or "(" in name or "?" in expr:
                continue
            value = eval_c_int_expr(expr, symbols)
            if value is not None:
                symbols[name] = value
                changed = True
        if not changed:
            break

    policy: dict[str, Any] = {"symbols": symbols}
    fishing3 = re.search(
        r'if\s*\(fgSceneEquals\(sceneName,\s*"fishing3"\)\)\s*'
        r"return\s+islandState\.lowTide\s*\?\s*(.*?)\s*:\s*(.*?);",
        source,
        re.S,
    )
    if fishing3 is not None:
        low_expr, high_expr = fishing3.groups()
        policy["fishing3_low_prime_bytes"] = eval_c_int_expr(low_expr, symbols)
        policy["fishing3_high_prime_bytes"] = eval_c_int_expr(high_expr, symbols)

    fishing2 = re.search(
        r'if\s*\(fgSceneEquals\(sceneName,\s*"fishing2"\)\)\s*'
        r"return\s+islandState\.lowTide\s*\?\s*(.*?)\s*:\s*(.*?);",
        source,
        re.S,
    )
    if fishing2 is not None:
        low_expr, high_expr = fishing2.groups()
        policy["fishing2_low_prime_bytes"] = eval_c_int_expr(low_expr, symbols)
        policy["fishing2_high_prime_bytes"] = eval_c_int_expr(high_expr, symbols)

    if "FG_SETUP_PRIME_WINDOW_BYTES" in symbols:
        policy["fishing1_prime_bytes"] = symbols["FG_SETUP_PRIME_WINDOW_BYTES"]
    if "FG_VISITOR1_HIGH_SETUP_PRIME_WINDOW_BYTES" in symbols:
        policy["visitor1_high_prime_bytes"] = symbols[
            "FG_VISITOR1_HIGH_SETUP_PRIME_WINDOW_BYTES"
        ]

    for tide in ("HIGH", "LOW"):
        start = symbols.get(f"FG_FISHING3_{tide}_SETUP_SEGMENT_START")
        size = symbols.get(f"FG_FISHING3_{tide}_SETUP_SEGMENT_BYTES")
        if start is not None and size is not None and size > 0:
            policy[f"fishing3_{tide.lower()}_segments"] = [
                (start // SECTOR_SIZE, int(math.ceil((start + size) / SECTOR_SIZE)))
            ]

    return policy


def default_setup_policy(case: dict[str, Any]) -> tuple[int, list[tuple[int, int]], str]:
    scene = case.get("sections", {}).get("scene", {})
    scene_name = scene.get("scene")
    lowtide = scene.get("lowtide") == 1
    source_policy = parse_source_setup_policy()
    if scene_name == "fishing1":
        return int(source_policy.get("fishing1_prime_bytes") or 320 * 1024), [], "auto:fishing1"
    if scene_name == "fishing2":
        if lowtide:
            prime = source_policy.get("fishing2_low_prime_bytes")
            return int(prime or 256 * 1024), [], "auto:fishing2-low"
        prime = source_policy.get("fishing2_high_prime_bytes")
        return int(prime or 352 * 1024), [], "auto:fishing2-high"
    if scene_name == "fishing3":
        if lowtide:
            prime = source_policy.get("fishing3_low_prime_bytes")
            segments = source_policy.get("fishing3_low_segments") or [(146, 152)]
            return int(prime or 288 * 1024), list(segments), "auto:fishing3-low"
        prime = source_policy.get("fishing3_high_prime_bytes")
        segments = source_policy.get("fishing3_high_segments") or [(67, 73)]
        return int(prime or 128 * 1024), list(segments), "auto:fishing3-high"
    if scene_name == "visitor1" and not lowtide:
        prime = source_policy.get("visitor1_high_prime_bytes")
        return int(prime or 296 * 1024), [], "auto:visitor1-high"
    return 0, [], "none"


def fg_pack_payload_end(header: dict[str, Any], entries: list[dict[str, Any]]) -> int:
    payload_end = 0
    for entry in entries:
        data_size = int(entry.get("data_size", 0) or 0)
        if data_size > 0:
            entry_end = int(entry.get("data_offset", 0) or 0) + data_size
            payload_end = max(payload_end, entry_end)
    sound_offset = int(header.get("sound_events_offset", 0) or 0)
    sound_count = int(header.get("sound_event_count", 0) or 0)
    if sound_offset and sound_count:
        payload_end = max(payload_end, sound_offset + sound_count * 4)
    return payload_end


def entry_is_payload(entry: dict[str, Any]) -> bool:
    return (
        entry.get("data_size", 0) > 0 and
        entry.get("width", 0) > 0 and
        entry.get("height", 0) > 0
    )


def extract_log_path(case: dict[str, Any], explicit_log: Path | None) -> Path | None:
    if explicit_log is not None:
        return explicit_log
    log_file = case.get("log_file")
    if isinstance(log_file, str) and log_file:
        return Path(log_file)
    return None


def read_segments_from_log(cdlog: Any, log_path: Path | None, pack_lba: int | None,
                           pack_sectors: int | None) -> list[dict[str, Any]]:
    if log_path is None or pack_lba is None or pack_sectors is None:
        return []
    reads, located = cdlog.parse_log(log_path)
    pack_locates = [item for item in located if item.get("lba") == pack_lba]
    pack_end = pack_lba + pack_sectors
    if pack_locates:
        last_locate_line = max(item["line"] for item in pack_locates)
        pack_reads = [
            item for item in reads
            if item["line"] > last_locate_line and pack_lba <= item["lba"] < pack_end
        ]
    else:
        pack_reads = [
            item for item in reads
            if pack_lba <= item["lba"] < pack_end
        ]
    return cdlog.infer_read_segments(pack_reads, pack_lba, pack_sectors)


def candidate_rows(entries: list[dict[str, Any]], read_segments: list[dict[str, Any]],
                   coverage_ranges: list[tuple[int, int]], pack_sectors: int,
                   window_sectors: int, top: int) -> list[dict[str, Any]]:
    candidates: list[dict[str, Any]] = []
    for start in range(0, max(0, pack_sectors - window_sectors) + 1):
        end = start + window_sectors
        if range_overlaps(coverage_ranges, start, end):
            continue

        covered_entries = [
            entry for entry in entries
            if entry_is_payload(entry)
            and not range_contains(coverage_ranges, entry["sector_start"], entry["sector_end"])
            and entry["sector_start"] >= start
            and entry["sector_end"] <= end
        ]
        fully_covered_reads = [
            segment for segment in read_segments
            if not range_contains(coverage_ranges,
                                  segment["file_sector_start"],
                                  segment["file_sector_end"])
            and segment["file_sector_start"] >= start
            and segment["file_sector_end"] <= end
        ]
        touched_reads = [
            segment for segment in read_segments
            if not range_contains(coverage_ranges,
                                  segment["file_sector_start"],
                                  segment["file_sector_end"])
            and segment["file_sector_start"] < end
            and segment["file_sector_end"] > start
        ]
        if not covered_entries and not fully_covered_reads and not touched_reads:
            continue

        payload_bytes = sum(int(entry["data_size"]) for entry in covered_entries)
        hold_vblanks = sum(int(entry.get("hold_vblanks", 0)) for entry in covered_entries)
        read_vblank_estimate = 0.0
        for segment in fully_covered_reads:
            prev_time = segment.get("prev_time_delta_s")
            if isinstance(prev_time, (int, float)):
                read_vblank_estimate += float(prev_time) * 60.0
        read_gaps = numeric_values([
            segment.get("prev_time_delta_s")
            for segment in fully_covered_reads
        ])

        score = (
            len(fully_covered_reads) * 1_000_000 +
            len(touched_reads) * 100_000 +
            int(round(read_vblank_estimate)) * 10_000 +
            len(covered_entries) * 1_000 +
            payload_bytes +
            hold_vblanks
        )
        candidates.append({
            "start_sector": start,
            "end_sector": end,
            "sectors": window_sectors,
            "score": score,
            "covered_entry_count": len(covered_entries),
            "covered_payload_bytes": payload_bytes,
            "covered_hold_vblanks": hold_vblanks,
            "first_entry": covered_entries[0]["index"] if covered_entries else None,
            "last_entry": covered_entries[-1]["index"] if covered_entries else None,
            "fully_covered_read_count": len(fully_covered_reads),
            "estimated_saved_reads": max(0, len(fully_covered_reads) - 1),
            "touched_read_count": len(touched_reads),
            "estimated_read_vblanks": round(read_vblank_estimate, 2),
            "min_prev_gap_s": round(min(read_gaps), 4) if read_gaps else None,
            "max_prev_gap_s": round(max(read_gaps), 4) if read_gaps else None,
            "avg_prev_gap_s": round(sum(read_gaps) / len(read_gaps), 4) if read_gaps else None,
            "phase_risk_hint": phase_risk_hint(fully_covered_reads),
            "source_read_indices": [segment.get("index") for segment in fully_covered_reads],
        })

    candidates.sort(
        key=lambda item: (
            item["fully_covered_read_count"],
            item["estimated_read_vblanks"],
            item["touched_read_count"],
            item["covered_entry_count"],
            item["covered_payload_bytes"],
        ),
        reverse=True,
    )
    return candidates[:top]


def build_report(args: argparse.Namespace) -> dict[str, Any]:
    cdlog = load_cdlog_module()
    summary = load_summary(args.summary)
    case = first_case(summary)
    pack = cdlog.parse_fg2_pack(args.pack)
    header = pack.get("header", {})
    entries = pack.get("entries", [])
    if not isinstance(entries, list):
        entries = []

    pack_lba = args.pack_lba
    if pack_lba is None:
        pack_lba = int_from_case(case, "scene", "pack_lba")
    pack_sectors = args.pack_sectors
    if pack_sectors is None:
        pack_sectors = int_from_case(case, "scene", "pack_sectors")
    if pack_sectors is None:
        pack_sectors = int(math.ceil(int(pack.get("bytes", 0)) / SECTOR_SIZE))

    source_policy = parse_source_setup_policy()
    data_offset = int(header.get("data_offset", 0))

    auto_prime_bytes, auto_segments, auto_policy = default_setup_policy(case)
    if pack.get("magic") == "FGP3":
        auto_pack_limit = int(
            source_policy.get("symbols", {}).get("FG_SETUP_PRIME_AUTO_PACK_BYTES") or
            source_policy.get("symbols", {}).get("FG_SETUP_PRIME_SMALL_PACK_BYTES") or
            64 * 1024
        )
        payload_end = fg_pack_payload_end(header, entries)
        window_start = (data_offset // SECTOR_SIZE) * SECTOR_SIZE
        if payload_end > window_start:
            window_bytes = int(math.ceil((payload_end - window_start) / SECTOR_SIZE)) * SECTOR_SIZE
            if 0 < window_bytes <= auto_pack_limit:
                auto_prime_bytes = window_bytes
                auto_policy = "auto:fgp3-resident-pack"
    setup_prime_bytes = args.setup_prime_bytes
    setup_policy = "explicit"
    if setup_prime_bytes is None:
        setup_prime_bytes = auto_prime_bytes
        setup_policy = auto_policy

    segment_ranges = list(auto_segments if args.use_default_segments else [])
    segment_ranges.extend(args.setup_segment or [])

    prime_start = data_offset // SECTOR_SIZE
    prime_end = prime_start
    if setup_prime_bytes > 0:
        prime_end = min(pack_sectors, prime_start + int(math.ceil(setup_prime_bytes / SECTOR_SIZE)))

    coverage_ranges: list[tuple[int, int]] = []
    if prime_end > prime_start:
        coverage_ranges.append((prime_start, prime_end))
    coverage_ranges.extend(segment_ranges)
    coverage_ranges = merge_ranges(coverage_ranges)

    log_path = extract_log_path(case, args.log)
    read_segments = read_segments_from_log(cdlog, log_path, pack_lba, pack_sectors)
    uncovered_read_segments = [
        segment for segment in read_segments
        if not range_contains(coverage_ranges,
                              segment["file_sector_start"],
                              segment["file_sector_end"])
    ]
    payload_entries = [entry for entry in entries if entry_is_payload(entry)]
    uncovered_entries = [
        entry for entry in payload_entries
        if not range_contains(coverage_ranges, entry["sector_start"], entry["sector_end"])
    ]

    candidate_sets = {
        str(window): candidate_rows(entries, read_segments, coverage_ranges,
                                    pack_sectors, window, args.top)
        for window in args.candidate_sectors
    }

    return {
        "schema": "ps1-foreground-read-plan/v1",
        "summary_file": str(args.summary.resolve()) if args.summary else None,
        "log_file": str(log_path.resolve()) if log_path else None,
        "pack_file": str(args.pack.resolve()),
        "scene": {
            "label": case.get("label"),
            "boot": case.get("boot"),
            "pack_lba": pack_lba,
            "pack_sectors": pack_sectors,
            "pack_bytes": pack.get("bytes"),
            "magic": pack.get("magic"),
            "frame_count": header.get("frame_count"),
            "data_start_sector": prime_start,
        },
        "current_metrics": {
            "loop_vb": int_from_case(case, "timing", "loop_vb"),
            "target_vb": int_from_case(case, "timing", "target_vb"),
            "overrun_vb": int_from_case(case, "timing", "overrun_vb"),
            "blocking_vb": int_from_case(case, "cd", "blocking_vb"),
            "loop_reads": int_from_case(case, "cd", "loop_reads"),
            "loop_read_vb": int_from_case(case, "cd", "loop_read_vb"),
            "prefetch_overrun_vb": int_from_case(case, "prefetch", "overrun_vb"),
            "due_misses": int_from_case(case, "prefetch", "due_misses"),
        },
        "setup_coverage": {
            "policy": setup_policy,
            "setup_prime_bytes": setup_prime_bytes,
            "prime_range": [prime_start, prime_end],
            "setup_segments": [[start, end] for start, end in segment_ranges],
            "merged_ranges": [[start, end] for start, end in coverage_ranges],
        },
        "observed_reads": {
            "after_pack_locate_segments": len(read_segments),
            "uncovered_segments": len(uncovered_read_segments),
            "uncovered_segment_sectors": sum(
                int(segment["file_sector_end"]) - int(segment["file_sector_start"])
                for segment in uncovered_read_segments
            ),
            "uncovered_entries": len(uncovered_entries),
            "uncovered_payload_bytes": sum(int(entry["data_size"]) for entry in uncovered_entries),
            "top_uncovered_segments": [
                {
                    "file_sector_start": segment.get("file_sector_start"),
                    "file_sector_end": segment.get("file_sector_end"),
                    "read_index": segment.get("index"),
                    "inferred_sectors": segment.get("inferred_sectors"),
                    "prev_time_delta_s": segment.get("prev_time_delta_s"),
                }
                for segment in uncovered_read_segments[:args.top]
            ],
        },
        "candidate_sets": candidate_sets,
    }


def print_human(report: dict[str, Any]) -> None:
    scene = report["scene"]
    metrics = report["current_metrics"]
    coverage = report["setup_coverage"]
    reads = report["observed_reads"]
    print(f"Scene: {scene.get('label')}  pack={Path(report['pack_file']).name}")
    print(
        "Metrics: "
        f"loop_vb={metrics.get('loop_vb')} "
        f"target_vb={metrics.get('target_vb')} "
        f"blocking_vb={metrics.get('blocking_vb')} "
        f"prefetch_overrun_vb={metrics.get('prefetch_overrun_vb')} "
        f"loop_reads={metrics.get('loop_reads')}"
    )
    print(
        "Setup coverage: "
        f"policy={coverage.get('policy')} "
        f"prime={coverage.get('prime_range')} "
        f"segments={coverage.get('setup_segments')} "
        f"merged={coverage.get('merged_ranges')}"
    )
    print(
        "Observed reads: "
        f"after_pack_locate={reads.get('after_pack_locate_segments')} "
        f"uncovered={reads.get('uncovered_segments')} "
        f"uncovered_sectors={reads.get('uncovered_segment_sectors')} "
        f"uncovered_entries={reads.get('uncovered_entries')}"
    )
    for sectors, candidates in report["candidate_sets"].items():
        print(f"\nTop {sectors}-sector candidates:")
        if not candidates:
            print("  none")
            continue
        for item in candidates:
            entry_span = "-"
            if item["first_entry"] is not None:
                entry_span = f"{item['first_entry']}..{item['last_entry']}"
            print(
                "  "
                f"{item['start_sector']}:{item['end_sector']} "
                f"reads={item['fully_covered_read_count']} "
                f"saved={item['estimated_saved_reads']} "
                f"touches={item['touched_read_count']} "
                f"est_vb={item['estimated_read_vblanks']} "
                f"gap={item['min_prev_gap_s']}..{item['max_prev_gap_s']}s "
                f"risk={item['phase_risk_hint']} "
                f"entries={item['covered_entry_count']}({entry_span}) "
                f"bytes={item['covered_payload_bytes']}"
            )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--summary", type=Path, help="ps1-perf-iterate summary JSON")
    parser.add_argument("--log", type=Path, help="DuckStation headless log; defaults to summary case log_file")
    parser.add_argument("--pack", type=Path, required=True, help="FG2/FGP3 pack file")
    parser.add_argument("--pack-lba", type=int, help="pack LBA override")
    parser.add_argument("--pack-sectors", type=int, help="pack sector count override")
    parser.add_argument("--setup-prime-bytes", type=int, help="setup-prime byte coverage override")
    parser.add_argument(
        "--setup-segment",
        action="append",
        type=parse_sector_range,
        help="extra half-open setup segment START:END; may be repeated",
    )
    parser.add_argument(
        "--no-default-segments",
        dest="use_default_segments",
        action="store_false",
        help="do not auto-load current known fishing3 setup segment ranges",
    )
    parser.set_defaults(use_default_segments=True)
    parser.add_argument(
        "--candidate-sectors",
        action="append",
        type=int,
        default=[],
        help="candidate segment size in sectors; may be repeated",
    )
    parser.add_argument("--top", type=int, default=10, help="candidates per segment size")
    parser.add_argument("--json", action="store_true", help="print full JSON instead of compact text")
    parser.add_argument("--output", type=Path, help="write full JSON report")
    args = parser.parse_args()

    if not args.candidate_sectors:
        args.candidate_sectors = [6, 12, 16, 24]
    for sectors in args.candidate_sectors:
        if sectors <= 0:
            raise SystemExit("--candidate-sectors values must be positive")

    report = build_report(args)
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    if args.json:
        print(json.dumps(report, indent=2))
    else:
        print_human(report)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
