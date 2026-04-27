#!/usr/bin/env python3
"""Summarize DuckStation CD read logs for PS1 perf experiments.

This is host-side only. It parses DuckStation's developer-level CDROM log lines
and correlates them with a ps1-perf-iterate summary so CD read cadence can be
studied without adding hot-path PS1 instrumentation.
"""

from __future__ import annotations

import argparse
import json
import re
from pathlib import Path
from typing import Any


ANSI_RE = re.compile(r"\x1b\[[0-9;?]*[A-Za-z]")
TIME_RE = re.compile(r"\[\s*([0-9]+(?:\.[0-9]+)?)\]")
SETLOC_RE = re.compile(r"CDROM:\s+Setloc\s+(\d+):(\d+):(\d+)")
READN_RE = re.compile(r"CDROM:\s+ReadN\s+(\d+):(\d+):(\d+)")
LOCATED_RE = re.compile(r"Located file at LBA\s+(\d+)")


def msf_to_lba(minutes: str, seconds: str, frames: str) -> int:
    return int(minutes) * 60 * 75 + int(seconds) * 75 + int(frames) - 150


def line_time(line: str) -> float | None:
    match = TIME_RE.search(line)
    return float(match.group(1)) if match else None


def load_summary(path: Path | None) -> dict[str, Any]:
    if path is None:
        return {}
    return json.loads(path.read_text(encoding="utf-8"))


def first_case(summary: dict[str, Any]) -> dict[str, Any]:
    cases = summary.get("cases")
    if isinstance(cases, list) and cases:
        return cases[0]
    return summary


def parse_log(log_path: Path) -> tuple[list[dict[str, Any]], list[dict[str, Any]]]:
    reads: list[dict[str, Any]] = []
    located: list[dict[str, Any]] = []
    pending_setloc: dict[str, Any] | None = None

    for line_number, raw in enumerate(
        log_path.read_text(encoding="utf-8", errors="ignore").splitlines(),
        1,
    ):
        line = ANSI_RE.sub("", raw)
        timestamp = line_time(line)

        match = SETLOC_RE.search(line)
        if match:
            pending_setloc = {
                "line": line_number,
                "time_s": timestamp,
                "lba": msf_to_lba(*match.groups()),
                "msf": ":".join(match.groups()),
            }
            continue

        match = READN_RE.search(line)
        if match:
            read_lba = msf_to_lba(*match.groups())
            item = {
                "index": len(reads),
                "line": line_number,
                "time_s": timestamp,
                "lba": read_lba,
                "msf": ":".join(match.groups()),
            }
            if pending_setloc is not None:
                item["setloc_lba"] = pending_setloc["lba"]
                item["setloc_line"] = pending_setloc["line"]
                item["setloc_time_s"] = pending_setloc["time_s"]
            reads.append(item)
            continue

        match = LOCATED_RE.search(line)
        if match:
            located.append({
                "line": line_number,
                "time_s": timestamp,
                "lba": int(match.group(1)),
            })

    for index, item in enumerate(reads):
        previous_item = reads[index - 1] if index > 0 else None
        next_item = reads[index + 1] if index + 1 < len(reads) else None
        if previous_item is not None:
            item["prev_lba_delta"] = item["lba"] - previous_item["lba"]
            if item.get("time_s") is not None and previous_item.get("time_s") is not None:
                item["prev_time_delta_s"] = round(item["time_s"] - previous_item["time_s"], 6)
        if next_item is not None:
            next_delta = next_item["lba"] - item["lba"]
            item["next_lba_delta"] = next_delta
            if next_delta > 0:
                item["inferred_forward_sectors"] = next_delta
            if item.get("time_s") is not None and next_item.get("time_s") is not None:
                item["next_time_delta_s"] = round(next_item["time_s"] - item["time_s"], 6)

    return reads, located


def summarize_sequence(reads: list[dict[str, Any]]) -> dict[str, Any]:
    lba_deltas: list[int] = []
    time_deltas: list[float] = []
    forward_sectors: list[int] = []
    largest_gap_candidates: list[dict[str, Any]] = []

    for index, item in enumerate(reads):
        if index > 0:
            previous_item = reads[index - 1]
            lba_delta = item["lba"] - previous_item["lba"]
            lba_deltas.append(lba_delta)
            if item.get("time_s") is not None and previous_item.get("time_s") is not None:
                time_delta = round(item["time_s"] - previous_item["time_s"], 6)
                time_deltas.append(time_delta)
                largest_gap_candidates.append({
                    "index": item["index"],
                    "lba": item["lba"],
                    "prev_lba_delta": lba_delta,
                    "prev_time_delta_s": time_delta,
                })
        if index + 1 < len(reads):
            next_delta = reads[index + 1]["lba"] - item["lba"]
            if next_delta > 0:
                forward_sectors.append(next_delta)

    largest_time_gaps = sorted(
        largest_gap_candidates,
        key=lambda item: item["prev_time_delta_s"],
        reverse=True,
    )[:10]

    return {
        "read_count": len(reads),
        "first_lba": reads[0]["lba"] if reads else None,
        "last_lba": reads[-1]["lba"] if reads else None,
        "forward_step_count": sum(1 for delta in lba_deltas if delta > 0),
        "backward_step_count": sum(1 for delta in lba_deltas if delta < 0),
        "same_lba_step_count": sum(1 for delta in lba_deltas if delta == 0),
        "max_forward_step": max((delta for delta in lba_deltas if delta > 0), default=0),
        "max_backward_step": min((delta for delta in lba_deltas if delta < 0), default=0),
        "inferred_forward_sectors_sum": sum(forward_sectors),
        "largest_prev_time_delta_s": max(time_deltas, default=0),
        "largest_time_gaps": largest_time_gaps,
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("log", type=Path, help="DuckStation headless log")
    parser.add_argument("--summary", type=Path, help="ps1-perf-iterate summary JSON")
    parser.add_argument("--pack-lba", type=int, help="Scene pack LBA override")
    parser.add_argument("--pack-sectors", type=int, help="Scene pack sector count override")
    parser.add_argument("--output", type=Path, help="Write JSON summary to this path")
    args = parser.parse_args()

    summary = load_summary(args.summary)
    case = first_case(summary)
    scene = case.get("sections", {}).get("scene", {})
    cd = case.get("sections", {}).get("cd", {})
    prefetch = case.get("sections", {}).get("prefetch", {})

    pack_lba = args.pack_lba
    if pack_lba is None and isinstance(scene.get("pack_lba"), int):
        pack_lba = scene["pack_lba"]
    pack_sectors = args.pack_sectors
    if pack_sectors is None and isinstance(scene.get("pack_sectors"), int):
        pack_sectors = scene["pack_sectors"]

    reads, located = parse_log(args.log)
    pack_reads: list[dict[str, Any]] = []
    after_last_pack_locate: list[dict[str, Any]] = []

    if pack_lba is not None and pack_sectors is not None:
        pack_end = pack_lba + pack_sectors
        pack_reads = [
            item for item in reads
            if pack_lba <= item["lba"] < pack_end
        ]
        pack_locates = [item for item in located if item["lba"] == pack_lba]
        if pack_locates:
            last_locate_line = max(item["line"] for item in pack_locates)
            after_last_pack_locate = [
                item for item in reads
                if item["line"] > last_locate_line and pack_lba <= item["lba"] < pack_end
            ]

    payload = {
        "schema": "ps1-perf-cdlog-summary/v1",
        "log_file": str(args.log.resolve()),
        "summary_file": str(args.summary.resolve()) if args.summary else None,
        "scene": {
            "label": case.get("label"),
            "pack": scene.get("pack"),
            "pack_lba": pack_lba,
            "pack_sectors": pack_sectors,
        },
        "jcperf2": {
            "total_reads": cd.get("reads"),
            "setup_reads": cd.get("setup_reads"),
            "loop_reads": cd.get("loop_reads"),
            "blocking_reads": prefetch.get("blocking_reads"),
            "blocking_vb": cd.get("blocking_vb"),
            "prefetch_overrun_vb": prefetch.get("overrun_vb"),
        },
        "all_reads": summarize_sequence(reads),
        "scene_pack_reads": summarize_sequence(pack_reads),
        "after_last_pack_locate_reads": summarize_sequence(after_last_pack_locate),
        "located_lbas": located,
        "reads": {
            "scene_pack": pack_reads,
            "after_last_pack_locate": after_last_pack_locate,
        },
    }

    text = json.dumps(payload, indent=2) + "\n"
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(text, encoding="utf-8")
    else:
        print(text, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
