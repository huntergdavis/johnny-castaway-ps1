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
import struct
from pathlib import Path
from typing import Any


ANSI_RE = re.compile(r"\x1b\[[0-9;?]*[A-Za-z]")
TIME_RE = re.compile(r"\[\s*([0-9]+(?:\.[0-9]+)?)\]")
SETLOC_RE = re.compile(r"CDROM:\s+Setloc\s+(\d+):(\d+):(\d+)")
READN_RE = re.compile(r"CDROM:\s+ReadN\s+(\d+):(\d+):(\d+)")
LOCATED_RE = re.compile(r"Located file at LBA\s+(\d+)")
DATA_SECTOR_RE = re.compile(r"CDROM:\s+DataSector\s+\d+:\d+:\d+\s+LBA=(\d+)")
FG2_HEADER = "<4sHHHHHHHHHHIIIHH"
FG2_HEADER_SIZE = struct.calcsize(FG2_HEADER)
FG2_ENTRY_SIZE = 20


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


def parse_fg2_pack(path: Path | None) -> dict[str, Any]:
    if path is None:
        return {}
    payload = path.read_bytes()
    if len(payload) < FG2_HEADER_SIZE:
        raise SystemExit(f"FG pack too small: {path}")

    (
        magic,
        version,
        frame_count,
        display_vblanks,
        flags,
        screen_width,
        screen_height,
        union_x,
        union_y,
        union_width,
        union_height,
        table_offset,
        data_offset,
        sound_events_offset,
        sound_event_count,
        reserved1,
    ) = struct.unpack_from(FG2_HEADER, payload, 0)

    if magic not in (b"FGP2", b"FGP3"):
        raise SystemExit(f"not an FG2/FGP3 pack: {path}")
    if table_offset + frame_count * FG2_ENTRY_SIZE > len(payload):
        raise SystemExit(f"FG entry table extends beyond pack: {path}")

    entries: list[dict[str, Any]] = []
    payload_entries: list[dict[str, Any]] = []
    for index in range(frame_count):
        (
            source_frame,
            x,
            y,
            width,
            height,
            hold_vblanks,
            data_offset_entry,
            data_size,
        ) = struct.unpack_from("<HhhHHHII", payload, table_offset + index * FG2_ENTRY_SIZE)
        item = {
            "index": index,
            "source_frame": source_frame,
            "x": x,
            "y": y,
            "width": width,
            "height": height,
            "hold_vblanks": hold_vblanks,
            "data_offset": data_offset_entry,
            "data_size": data_size,
            "sector_start": data_offset_entry // 2048,
            "sector_end": (data_offset_entry + data_size + 2047) // 2048,
        }
        entries.append(item)
        if data_size > 0 and width > 0 and height > 0:
            payload_entries.append(item)

    return {
        "path": str(path.resolve()),
        "bytes": len(payload),
        "magic": magic.decode("ascii", errors="replace"),
        "header": {
            "version": version,
            "frame_count": frame_count,
            "display_vblanks": display_vblanks,
            "flags": flags,
            "screen_width": screen_width,
            "screen_height": screen_height,
            "union_x": union_x,
            "union_y": union_y,
            "union_width": union_width,
            "union_height": union_height,
            "table_offset": table_offset,
            "data_offset": data_offset,
            "sound_events_offset": sound_events_offset,
            "sound_event_count": sound_event_count,
            "reserved1": reserved1,
        },
        "payload_entry_count": len(payload_entries),
        "max_payload_entry": max(payload_entries, key=lambda item: item["data_size"], default=None),
        "entries": entries,
    }


def parse_log(log_path: Path) -> tuple[list[dict[str, Any]], list[dict[str, Any]]]:
    reads: list[dict[str, Any]] = []
    located: list[dict[str, Any]] = []
    pending_setloc: dict[str, Any] | None = None
    active_read: dict[str, Any] | None = None

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
            active_read = item
            continue

        match = LOCATED_RE.search(line)
        if match:
            located.append({
                "line": line_number,
                "time_s": timestamp,
                "lba": int(match.group(1)),
            })
            continue

        match = DATA_SECTOR_RE.search(line)
        if match and active_read is not None:
            # DuckStation prints DataSector LBA in physical MSF space, while
            # Setloc/ReadN and ISO file locations here are logical LBAs.
            sector_lba = int(match.group(1)) - 150
            sectors = active_read.setdefault("data_sector_lbas", [])
            times = active_read.setdefault("data_sector_times_s", [])
            sectors.append(sector_lba)
            times.append(timestamp)

    for index, item in enumerate(reads):
        previous_item = reads[index - 1] if index > 0 else None
        next_item = reads[index + 1] if index + 1 < len(reads) else None
        data_sector_lbas = item.pop("data_sector_lbas", [])
        data_sector_times = item.pop("data_sector_times_s", [])
        if data_sector_lbas:
            item["data_sector_count"] = len(data_sector_lbas)
            item["data_lba_first"] = data_sector_lbas[0]
            item["data_lba_last"] = data_sector_lbas[-1]
            item["data_lba_min"] = min(data_sector_lbas)
            item["data_lba_max"] = max(data_sector_lbas)
            item["data_contiguous"] = (
                data_sector_lbas == list(range(data_sector_lbas[0],
                                               data_sector_lbas[0] + len(data_sector_lbas)))
            )
            if data_sector_times and data_sector_times[0] is not None:
                item["data_time_first_s"] = data_sector_times[0]
            if data_sector_times and data_sector_times[-1] is not None:
                item["data_time_last_s"] = data_sector_times[-1]
            if (isinstance(item.get("data_time_first_s"), (int, float)) and
                    isinstance(item.get("data_time_last_s"), (int, float))):
                item["data_time_span_s"] = round(
                    item["data_time_last_s"] - item["data_time_first_s"],
                    6,
                )
        else:
            item["data_sector_count"] = 0
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
    delivered_sectors: list[int] = []
    largest_gap_candidates: list[dict[str, Any]] = []

    for index, item in enumerate(reads):
        delivered_count = item.get("data_sector_count")
        if isinstance(delivered_count, int):
            delivered_sectors.append(delivered_count)
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
        "delivered_sector_sum": sum(delivered_sectors),
        "max_delivered_sectors": max(delivered_sectors, default=0),
        "largest_prev_time_delta_s": max(time_deltas, default=0),
        "largest_time_gaps": largest_time_gaps,
    }


def infer_read_segments(reads: list[dict[str, Any]], pack_lba: int | None,
                        pack_sectors: int | None) -> list[dict[str, Any]]:
    if pack_lba is None or pack_sectors is None:
        return []

    pack_end_lba = pack_lba + pack_sectors
    segments: list[dict[str, Any]] = []
    for index, item in enumerate(reads):
        start_lba = item["lba"]
        data_lba_last = item.get("data_lba_last")
        if isinstance(data_lba_last, int) and data_lba_last >= start_lba:
            end_lba = data_lba_last + 1
        else:
            next_lba = reads[index + 1]["lba"] if index + 1 < len(reads) else pack_end_lba
            end_lba = next_lba if next_lba > start_lba else min(start_lba + 1, pack_end_lba)
        if end_lba > pack_end_lba:
            end_lba = pack_end_lba
        if end_lba <= start_lba:
            continue
        segment = dict(item)
        segment["file_sector_start"] = start_lba - pack_lba
        segment["file_sector_end"] = end_lba - pack_lba
        segment["inferred_sectors"] = end_lba - start_lba
        segments.append(segment)
    return segments


def entries_covered_by_group(entries: list[dict[str, Any]], start_sector: int,
                             end_sector: int) -> list[int]:
    covered: list[int] = []
    for entry in entries:
        if entry["data_size"] <= 0 or entry["width"] <= 0 or entry["height"] <= 0:
            continue
        if entry["sector_start"] >= start_sector and entry["sector_end"] <= end_sector:
            covered.append(entry["index"])
    return covered


def plan_read_groups(segments: list[dict[str, Any]], pack: dict[str, Any],
                     max_group_sectors: int) -> dict[str, Any]:
    groups: list[dict[str, Any]] = []
    entries = pack.get("entries", []) if isinstance(pack.get("entries"), list) else []
    current: dict[str, Any] | None = None

    for segment in segments:
        segment_start = segment["file_sector_start"]
        segment_end = segment["file_sector_end"]
        if current is None:
            current = {
                "file_sector_start": segment_start,
                "file_sector_end": segment_end,
                "source_read_indices": [segment["index"]],
            }
            continue

        proposed_end = max(current["file_sector_end"], segment_end)
        proposed_sectors = proposed_end - current["file_sector_start"]
        contiguous = segment_start <= current["file_sector_end"]
        if contiguous and proposed_sectors <= max_group_sectors:
            current["file_sector_end"] = proposed_end
            current["source_read_indices"].append(segment["index"])
        else:
            groups.append(current)
            current = {
                "file_sector_start": segment_start,
                "file_sector_end": segment_end,
                "source_read_indices": [segment["index"]],
            }

    if current is not None:
        groups.append(current)

    for index, group in enumerate(groups):
        group["index"] = index
        group["sectors"] = group["file_sector_end"] - group["file_sector_start"]
        group["replaces_read_count"] = len(group["source_read_indices"])
        group["covered_entry_indices"] = entries_covered_by_group(
            entries,
            group["file_sector_start"],
            group["file_sector_end"],
        )
        if group["covered_entry_indices"]:
            group["first_entry"] = group["covered_entry_indices"][0]
            group["last_entry"] = group["covered_entry_indices"][-1]

    original_sector_count = sum(segment["inferred_sectors"] for segment in segments)
    planned_sector_count = sum(group["sectors"] for group in groups)
    return {
        "max_group_sectors": max_group_sectors,
        "source_read_count": len(segments),
        "planned_read_count": len(groups),
        "read_count_delta": len(groups) - len(segments),
        "source_inferred_sectors": original_sector_count,
        "planned_sectors": planned_sector_count,
        "sector_delta": planned_sector_count - original_sector_count,
        "groups": groups,
    }


def numeric_delta(current: Any, baseline: Any) -> int | float | None:
    if isinstance(current, (int, float)) and isinstance(baseline, (int, float)):
        return current - baseline
    return None


def map_segments_by_sector(payload: dict[str, Any]) -> dict[int, dict[str, Any]]:
    result: dict[int, dict[str, Any]] = {}
    segments = payload.get("after_last_pack_locate_segments", [])
    if not isinstance(segments, list):
        return result
    for segment in segments:
        sector = segment.get("file_sector_start")
        if isinstance(sector, int):
            result[sector] = segment
    return result


def segment_elapsed_from_first(segment: dict[str, Any],
                               first_time_s: float | None) -> float | None:
    time_s = segment.get("time_s")
    if not isinstance(time_s, (int, float)) or first_time_s is None:
        return None
    return round(float(time_s) - first_time_s, 6)


def first_segment_time(payload: dict[str, Any]) -> float | None:
    segments = payload.get("after_last_pack_locate_segments", [])
    if not isinstance(segments, list) or not segments:
        return None
    time_s = segments[0].get("time_s")
    if isinstance(time_s, (int, float)):
        return float(time_s)
    return None


def covered_entries_for_sector(payload: dict[str, Any], start_sector: int,
                               end_sector: int) -> list[int]:
    pack_file = payload.get("pack_file")
    if not isinstance(pack_file, dict):
        return []
    entries = payload.get("pack_entries")
    if not isinstance(entries, list):
        return []
    return entries_covered_by_group(entries, start_sector, end_sector)


def summarize_cd_comparison(current: dict[str, Any],
                            baseline: dict[str, Any]) -> dict[str, Any]:
    current_jcperf = current.get("jcperf2", {})
    baseline_jcperf = baseline.get("jcperf2", {})
    metrics = [
        "total_reads",
        "setup_reads",
        "loop_reads",
        "blocking_reads",
        "blocking_vb",
        "prefetch_overrun_vb",
    ]
    jcperf_deltas = {
        metric: {
            "baseline": baseline_jcperf.get(metric),
            "current": current_jcperf.get(metric),
            "delta": numeric_delta(current_jcperf.get(metric), baseline_jcperf.get(metric)),
        }
        for metric in metrics
    }

    baseline_segments = map_segments_by_sector(baseline)
    current_segments = map_segments_by_sector(current)
    common_sectors = sorted(set(baseline_segments) & set(current_segments))
    missing_sectors = sorted(set(baseline_segments) - set(current_segments))
    new_sectors = sorted(set(current_segments) - set(baseline_segments))
    baseline_first_time = first_segment_time(baseline)
    current_first_time = first_segment_time(current)

    candidates: list[dict[str, Any]] = []
    for sector in common_sectors:
        base_segment = baseline_segments[sector]
        cur_segment = current_segments[sector]
        base_prev = base_segment.get("prev_time_delta_s")
        cur_prev = cur_segment.get("prev_time_delta_s")
        prev_delta = numeric_delta(cur_prev, base_prev)
        base_elapsed = segment_elapsed_from_first(base_segment, baseline_first_time)
        cur_elapsed = segment_elapsed_from_first(cur_segment, current_first_time)
        elapsed_delta = numeric_delta(cur_elapsed, base_elapsed)
        start_sector = cur_segment.get("file_sector_start", sector)
        end_sector = cur_segment.get("file_sector_end", sector + 1)
        if not isinstance(end_sector, int):
            end_sector = sector + 1
        if not isinstance(start_sector, int):
            start_sector = sector
        score = max(abs(prev_delta or 0), abs(elapsed_delta or 0))
        if score <= 0:
            continue
        candidates.append({
            "file_sector_start": sector,
            "file_sector_end": end_sector,
            "current_lba": cur_segment.get("lba"),
            "baseline_lba": base_segment.get("lba"),
            "current_read_index": cur_segment.get("index"),
            "baseline_read_index": base_segment.get("index"),
            "current_prev_time_delta_s": cur_prev,
            "baseline_prev_time_delta_s": base_prev,
            "prev_time_delta_s": round(prev_delta, 6) if isinstance(prev_delta, float) else prev_delta,
            "current_elapsed_from_first_s": cur_elapsed,
            "baseline_elapsed_from_first_s": base_elapsed,
            "elapsed_from_first_delta_s": (
                round(elapsed_delta, 6) if isinstance(elapsed_delta, float) else elapsed_delta
            ),
            "estimated_vblank_delta": (
                round(float(prev_delta) * 60.0, 2)
                if isinstance(prev_delta, (int, float)) else None
            ),
            "inferred_sectors": cur_segment.get("inferred_sectors"),
            "covered_entries": covered_entries_for_sector(current, start_sector, end_sector),
        })

    timing_shifts = sorted(
        candidates,
        key=lambda item: (
            abs(item.get("estimated_vblank_delta") or 0),
            abs(item.get("elapsed_from_first_delta_s") or 0),
        ),
        reverse=True,
    )
    regression_candidates = sorted(
        [
            item for item in candidates
            if (item.get("prev_time_delta_s") or 0) > 0
            or (item.get("elapsed_from_first_delta_s") or 0) > 0
        ],
        key=lambda item: (
            item.get("estimated_vblank_delta") or 0,
            item.get("elapsed_from_first_delta_s") or 0,
        ),
        reverse=True,
    )

    blocking_delta = jcperf_deltas["blocking_reads"]["delta"]
    return {
        "baseline_summary_file": baseline.get("summary_file"),
        "baseline_log_file": baseline.get("log_file"),
        "jcperf2_deltas": jcperf_deltas,
        "read_sequence": {
            "baseline_segments": len(baseline_segments),
            "current_segments": len(current_segments),
            "common_file_sectors": len(common_sectors),
            "missing_file_sectors": missing_sectors[:20],
            "new_file_sectors": new_sectors[:20],
        },
        "extra_visible_read": {
            "blocking_read_delta": blocking_delta,
            "likely_regressed": (
                isinstance(blocking_delta, (int, float)) and blocking_delta > 0
            ),
            "candidate_basis": (
                "file-sector-normalized host read timing deltas; use as a locator, "
                "not as PS1-side proof"
            ),
            "top_regression_candidates": regression_candidates[:12],
            "largest_timing_shifts": timing_shifts[:12],
        },
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("log", type=Path, help="DuckStation headless log")
    parser.add_argument("--summary", type=Path, help="ps1-perf-iterate summary JSON")
    parser.add_argument("--pack-lba", type=int, help="Scene pack LBA override")
    parser.add_argument("--pack-sectors", type=int, help="Scene pack sector count override")
    parser.add_argument("--pack-file", type=Path, help="FG2 pack file for entry-aware group planning")
    parser.add_argument("--compare", type=Path, help="Baseline CD summary JSON from this script")
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
    pack = parse_fg2_pack(args.pack_file)
    pack_reads: list[dict[str, Any]] = []
    after_last_pack_locate: list[dict[str, Any]] = []
    after_last_segments: list[dict[str, Any]] = []
    group_plans: list[dict[str, Any]] = []

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
            after_last_segments = infer_read_segments(
                after_last_pack_locate,
                pack_lba,
                pack_sectors,
            )
            if pack:
                group_plans = [
                    plan_read_groups(after_last_segments, pack, max_group_sectors)
                    for max_group_sectors in (12, 16, 24)
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
        "after_last_pack_locate_segments": after_last_segments,
        "pack_file": {
            key: value for key, value in pack.items()
            if key != "entries"
        } if pack else None,
        "pack_entries": pack.get("entries") if pack else None,
        "group_plans": group_plans,
        "located_lbas": located,
        "reads": {
            "scene_pack": pack_reads,
            "after_last_pack_locate": after_last_pack_locate,
        },
    }
    if args.compare:
        compare_payload = json.loads(args.compare.read_text(encoding="utf-8"))
        payload["comparison"] = summarize_cd_comparison(payload, compare_payload)

    text = json.dumps(payload, indent=2) + "\n"
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(text, encoding="utf-8")
    else:
        print(text, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
