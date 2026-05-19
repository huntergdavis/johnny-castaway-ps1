#!/usr/bin/env python3
"""Summarize under-target PS1 perf rows and their current read-plan candidates.

This is host-side evidence tooling. It does not run DuckStation or modify packs;
it combines a ps1-perf-iterate summary with each case's foreground-read-plan
JSON so the next optimization starts from measured cadence/read-plan data.
"""

from __future__ import annotations

import argparse
import csv
import json
import sys
from pathlib import Path
from typing import Any


def load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8"))


def summary_path(value: str) -> Path:
    path = Path(value)
    if path.is_dir():
        path = path / "summary.json"
    if not path.exists():
        raise SystemExit(f"summary not found: {path}")
    return path


def target_speed_pct(loop_vb: Any, target_vb: Any) -> float | None:
    if not isinstance(loop_vb, (int, float)) or not isinstance(target_vb, (int, float)):
        return None
    if loop_vb <= 0:
        return None
    return (float(target_vb) / float(loop_vb)) * 100.0


def fmt_float(value: Any, digits: int = 3) -> str:
    if isinstance(value, (int, float)):
        return f"{float(value):.{digits}f}"
    return ""


def read_case_plan(case_dir: Path) -> dict[str, Any]:
    path = case_dir / "foreground-read-plan.json"
    if not path.exists():
        return {}
    return load_json(path)


def candidate_rows(plan: dict[str, Any]) -> list[tuple[int, dict[str, Any]]]:
    rows: list[tuple[int, dict[str, Any]]] = []
    visible_sets = plan.get("visible_candidate_sets")
    if not isinstance(visible_sets, dict):
        return rows
    for size_key, candidates in visible_sets.items():
        if not isinstance(candidates, list):
            continue
        try:
            size = int(size_key)
        except ValueError:
            size = 0
        for item in candidates:
            if not isinstance(item, dict):
                continue
            saved = item.get("estimated_saved_reads")
            fully_covered = item.get("fully_covered_read_count")
            if not isinstance(saved, (int, float)) or saved <= 0:
                if not isinstance(fully_covered, int) or fully_covered <= 1:
                    continue
            rows.append((size, item))
    return sorted(
        rows,
        key=lambda pair: (
            pair[1].get("visible_safety_score", -999999),
            pair[1].get("estimated_saved_reads", 0),
            pair[1].get("estimated_read_vblanks", 0),
        ),
        reverse=True,
    )


def scene_base_row(case: dict[str, Any], threshold_pct: float) -> dict[str, str]:
    sections = case.get("sections", {})
    timing = sections.get("timing", {}) if isinstance(sections, dict) else {}
    cd = sections.get("cd", {}) if isinstance(sections, dict) else {}
    prefetch = sections.get("prefetch", {}) if isinstance(sections, dict) else {}
    gfx = sections.get("gfx", {}) if isinstance(sections, dict) else {}
    frame = sections.get("frame", {}) if isinstance(sections, dict) else {}

    loop_vb = timing.get("loop_vb")
    target_vb = timing.get("target_vb")
    speed = target_speed_pct(loop_vb, target_vb)
    return {
        "label": str(case.get("label", "")),
        "target_speed_pct": fmt_float(speed),
        "threshold_pct": fmt_float(threshold_pct),
        "loop_vb": str(loop_vb or ""),
        "target_vb": str(target_vb or ""),
        "overrun_vb": str(timing.get("overrun_vb", "")),
        "blocking_vb": str(cd.get("blocking_vb", "")),
        "prefetch_overrun_vb": str(prefetch.get("overrun_vb", "")),
        "loop_reads": str(cd.get("loop_reads", "")),
        "due_misses": str(prefetch.get("due_misses", "")),
        "upload_bytes": str(gfx.get("upload_bytes", "")),
        "upload_rects": str(gfx.get("upload_rects", "")),
        "restore_bytes": str(gfx.get("restore_bytes", "")),
        "max_payload_idx": str(frame.get("max_payload_idx", "")),
        "max_payload": str(frame.get("max_payload", "")),
        "prefetch_policy": str(prefetch.get("policy", "")),
    }


def candidate_base_row(rank: int, size: int, item: dict[str, Any]) -> dict[str, str]:
    first_entry = item.get("first_entry")
    last_entry = item.get("last_entry")
    return {
        "candidate_rank": str(rank),
        "candidate_size": str(size or ""),
        "candidate_start": str(item.get("start_sector", "")),
        "candidate_end": str(item.get("end_sector", "")),
        "candidate_saved_reads": str(item.get("estimated_saved_reads", "")),
        "candidate_read_count": str(item.get("fully_covered_read_count", "")),
        "candidate_touched_reads": str(item.get("touched_read_count", "")),
        "candidate_est_read_vb": fmt_float(item.get("estimated_read_vblanks"), 2),
        "candidate_score": str(item.get("visible_safety_score", "")),
        "candidate_risk": str(item.get("visible_risk_hint", "")),
        "candidate_cd_class": str(item.get("visible_cd_cost_class", "")),
        "candidate_scheduler_class": str(item.get("scheduler_class", "")),
        "candidate_entries": (
            f"{first_entry}..{last_entry}"
            if first_entry is not None and last_entry is not None else ""
        ),
        "candidate_payload_bytes": str(item.get("covered_payload_bytes", "")),
        "candidate_first_gap_s": fmt_float(item.get("min_prev_gap_s"), 4),
        "candidate_internal_gap": str(item.get("internal_gap_slack_class", "")),
        "candidate_fireable": str(item.get("append_start_fireable", "")),
    }


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Emit CSV rows for under-green PS1 perf cases and read-plan candidates.",
    )
    parser.add_argument("summary", help="ps1-perf-iterate summary.json or run directory")
    parser.add_argument("--threshold-pct", type=float, default=99.0)
    parser.add_argument("--top", type=int, default=5, help="candidate rows per scene")
    parser.add_argument("--all", action="store_true", help="include rows at or above threshold")
    args = parser.parse_args()

    summary = load_json(summary_path(args.summary))
    cases = summary.get("cases")
    if not isinstance(cases, list):
        raise SystemExit("summary has no cases array")

    fieldnames = [
        "label",
        "target_speed_pct",
        "threshold_pct",
        "loop_vb",
        "target_vb",
        "overrun_vb",
        "blocking_vb",
        "prefetch_overrun_vb",
        "loop_reads",
        "due_misses",
        "upload_bytes",
        "upload_rects",
        "restore_bytes",
        "max_payload_idx",
        "max_payload",
        "prefetch_policy",
        "candidate_rank",
        "candidate_size",
        "candidate_start",
        "candidate_end",
        "candidate_saved_reads",
        "candidate_read_count",
        "candidate_touched_reads",
        "candidate_est_read_vb",
        "candidate_score",
        "candidate_risk",
        "candidate_cd_class",
        "candidate_scheduler_class",
        "candidate_entries",
        "candidate_payload_bytes",
        "candidate_first_gap_s",
        "candidate_internal_gap",
        "candidate_fireable",
    ]
    writer = csv.DictWriter(sys.stdout, fieldnames=fieldnames)
    writer.writeheader()

    for case in cases:
        sections = case.get("sections", {})
        timing = sections.get("timing", {}) if isinstance(sections, dict) else {}
        speed = target_speed_pct(timing.get("loop_vb"), timing.get("target_vb"))
        if not args.all and (speed is None or speed >= args.threshold_pct):
            continue

        base = scene_base_row(case, args.threshold_pct)
        case_dir_value = case.get("case_dir")
        candidates: list[tuple[int, dict[str, Any]]] = []
        if isinstance(case_dir_value, str):
            candidates = candidate_rows(read_case_plan(Path(case_dir_value)))
        if not candidates:
            writer.writerow(base)
            continue
        for rank, (size, item) in enumerate(candidates[:max(1, args.top)], 1):
            row = dict(base)
            row.update(candidate_base_row(rank, size, item))
            writer.writerow(row)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
