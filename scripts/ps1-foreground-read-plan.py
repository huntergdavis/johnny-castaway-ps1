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


def select_case(summary: dict[str, Any], case_label: str | None) -> dict[str, Any]:
    cases = summary.get("cases")
    if isinstance(cases, list) and cases:
        if case_label is not None:
            for case in cases:
                if case.get("label") == case_label:
                    return case
            labels = ", ".join(str(case.get("label", "<unnamed>")) for case in cases)
            raise SystemExit(f"--case-label {case_label!r} not found; available: {labels}")
        return cases[0]
    if case_label is not None and summary.get("label") != case_label:
        raise SystemExit(f"--case-label {case_label!r} requested but summary is not a matching multi-case run")
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


def gap_slack_class(gap_s: float | None) -> str:
    if gap_s is None:
        return "unknown"
    if gap_s < 0.20:
        return "tight"
    if gap_s < 0.50:
        return "short"
    if gap_s < 1.00:
        return "medium"
    return "long"


def read_seek_direction(reads: list[dict[str, Any]]) -> str:
    lbas = [
        int(segment["lba"])
        for segment in reads
        if isinstance(segment.get("lba"), int)
    ]
    if len(lbas) < 2:
        return "single-or-unknown"

    deltas = [later - earlier for earlier, later in zip(lbas, lbas[1:])]
    if all(delta >= 0 for delta in deltas):
        return "forward"
    if all(delta <= 0 for delta in deltas):
        return "reverse"
    return "mixed"


def visible_cd_cost_class(saved_reads: int,
                          append_start_fireable: bool,
                          first_gap_class: str,
                          internal_gap_class: str,
                          partial_touch_count: int,
                          overread_sectors: int,
                          seek_direction: str) -> str:
    if saved_reads <= 0:
        return "reject:no-saved-read"
    if not append_start_fireable:
        return "reject:no-observed-append-start"
    if first_gap_class == "tight" or internal_gap_class == "tight":
        return "unsafe:tight-visible-gap"
    if seek_direction in ("reverse", "mixed"):
        return "risky:seek-direction"
    if partial_touch_count >= 2:
        return "risky:multi-partial-overlap"
    if overread_sectors > 4:
        return "risky:overread"
    if first_gap_class == "short" or internal_gap_class == "short":
        return "risky:short-visible-gap"
    if partial_touch_count > 0 or overread_sectors > 0:
        return "balanced:validate-overlap"
    if first_gap_class == "long" and internal_gap_class in ("long", "medium", "unknown"):
        return "safe:long-visible-gap"
    return "balanced:medium-visible-gap"


def candidate_visible_cost(start: int, end: int,
                           fully_covered_reads: list[dict[str, Any]],
                           touched_reads: list[dict[str, Any]],
                           append_start_fireable: bool) -> dict[str, Any]:
    read_count = len(fully_covered_reads)
    saved_reads = max(0, read_count - 1)
    touched_count = len(touched_reads)
    partial_touch_count = max(0, touched_count - read_count)
    read_gaps = numeric_values([
        segment.get("prev_time_delta_s")
        for segment in fully_covered_reads
    ])
    internal_gaps = read_gaps[1:] if len(read_gaps) > 1 else []
    first_gap = read_gaps[0] if read_gaps else None
    min_gap = min(read_gaps) if read_gaps else None
    max_gap = max(read_gaps) if read_gaps else None
    min_internal_gap = min(internal_gaps) if internal_gaps else None
    max_internal_gap = max(internal_gaps) if internal_gaps else None
    observed_sectors = sum(
        max(0, int(segment.get("file_sector_end", 0)) -
             int(segment.get("file_sector_start", 0)))
        for segment in fully_covered_reads
    )
    group_sectors = end - start
    overread_sectors = max(0, group_sectors - observed_sectors)
    first_gap_class = gap_slack_class(first_gap)
    internal_gap_class = gap_slack_class(min_internal_gap)
    seek_direction = read_seek_direction(fully_covered_reads)
    visible_cd_class = visible_cd_cost_class(
        saved_reads,
        append_start_fireable,
        first_gap_class,
        internal_gap_class,
        partial_touch_count,
        overread_sectors,
        seek_direction,
    )

    risk = 0
    reasons: list[str] = []
    if saved_reads <= 0:
        risk += 120
        reasons.append("no-saved-read")
    if not append_start_fireable:
        risk += 180
        reasons.append("no-observed-append-start")
    if partial_touch_count > 0:
        risk += partial_touch_count * 35
        reasons.append("partial-read-overlap")
    if overread_sectors > 0:
        risk += min(40, overread_sectors * 4)
        reasons.append("group-overread")
    if first_gap is not None:
        if first_gap < 0.20:
            risk += 55
            reasons.append("tight-first-gap")
        elif first_gap < 0.50:
            risk += 25
            reasons.append("short-first-gap")
    if min_internal_gap is not None:
        if min_internal_gap < 0.20:
            risk += 45
            reasons.append("tight-internal-gap")
        elif min_internal_gap < 0.50:
            risk += 20
            reasons.append("short-internal-gap")

    safety_score = (
        saved_reads * 1000 -
        risk * 10 +
        int(round((first_gap or 0.0) * 60.0)) +
        min(120, int(round((max_gap or 0.0) * 10.0))) -
        partial_touch_count * 25 -
        overread_sectors * 4
    )

    if not append_start_fireable:
        hint = "reject:no-observed-append-start"
    elif saved_reads <= 0:
        hint = "reject:no-saved-read"
    elif risk >= 100:
        hint = "high-risk:visible-cadence"
    elif risk >= 50:
        hint = "medium-risk:validate-phase"
    else:
        hint = "candidate:low-visible-risk"

    return {
        "visible_safety_score": safety_score,
        "visible_risk_score": risk,
        "visible_risk_hint": hint,
        "visible_cd_cost_class": visible_cd_class,
        "first_gap_slack_class": first_gap_class,
        "internal_gap_slack_class": internal_gap_class,
        "read_seek_direction": seek_direction,
        "visible_risk_reasons": reasons,
        "partial_touch_count": partial_touch_count,
        "observed_read_sectors": observed_sectors,
        "group_overread_sectors": overread_sectors,
        "first_prev_gap_s": round(first_gap, 4) if first_gap is not None else None,
        "min_internal_gap_s": round(min_internal_gap, 4) if min_internal_gap is not None else None,
        "max_internal_gap_s": round(max_internal_gap, 4) if max_internal_gap is not None else None,
    }


def runtime_group_metadata(start: int, end: int,
                           saved_reads: int,
                           append_start_fireable: bool,
                           runtime_group_capacity_sectors: int) -> dict[str, Any]:
    sectors = end - start
    capacity = max(0, runtime_group_capacity_sectors)
    fits_current_group_path = (
        saved_reads > 0 and
        append_start_fireable and
        capacity > 0 and
        sectors <= capacity
    )
    if saved_reads <= 0:
        metadata_class = "reject:no-saved-read"
    elif not append_start_fireable:
        metadata_class = "needs-new-append-start"
    elif capacity <= 0:
        metadata_class = "needs-runtime-group-capacity"
    elif sectors > capacity:
        metadata_class = "needs-scheduler-or-larger-group-window"
    else:
        metadata_class = "current-read-group-compatible"
    return {
        "runtime_group_capacity_sectors": capacity,
        "runtime_current_group_fit": fits_current_group_path,
        "runtime_metadata_class": metadata_class,
    }


def scheduler_retry_metadata(item: dict[str, Any]) -> dict[str, Any]:
    saved_reads = int(item.get("estimated_saved_reads") or 0)
    if saved_reads <= 0:
        retry_class = "reject:no-saved-read"
    elif not item.get("append_start_fireable"):
        retry_class = "reject:no-observed-append-start"
    elif item.get("runtime_metadata_class") != "current-read-group-compatible":
        retry_class = "needs-generated-or-larger-group-window"
    else:
        cd_class = str(item.get("visible_cd_cost_class") or "")
        risk_reasons = set(item.get("visible_risk_reasons") or [])
        hidden_refill_reasons = {
            "tight-internal-gap",
            "short-internal-gap",
            "group-overread",
            "partial-read-overlap",
        }
        if cd_class.startswith("safe:"):
            retry_class = "standalone-candidate"
        elif cd_class.startswith("balanced:") or cd_class == "risky:short-visible-gap":
            retry_class = "scheduler-owned-candidate"
        else:
            retry_class = "high-risk:scheduler-only"

    notes = {
        "standalone-candidate": "current table may promote under strict no-regression gate",
        "scheduler-owned-candidate": "visible timing may improve, but refill/overlap risk should be scheduler-owned",
        "high-risk:scheduler-only": "do not test as a standalone table",
        "needs-generated-or-larger-group-window": "requires metadata or a larger retained window before runtime can fire",
        "reject:no-observed-append-start": "runtime will not fire from the current append start",
        "reject:no-saved-read": "no expected read-count win",
    }
    return {
        "scheduler_retry_class": retry_class,
        "scheduler_retry_note": notes.get(retry_class, ""),
    }


def read_segment_summary(segment: dict[str, Any]) -> dict[str, Any]:
    """Keep the generated read-plan JSON self-contained for runtime metadata."""
    return {
        "read_index": segment.get("index"),
        "line": segment.get("line"),
        "lba": segment.get("lba"),
        "sectors": segment.get("sectors"),
        "file_sector_start": segment.get("file_sector_start"),
        "file_sector_end": segment.get("file_sector_end"),
        "inferred_sectors": segment.get("inferred_sectors"),
        "prev_time_delta_s": segment.get("prev_time_delta_s"),
    }


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


def split_top_level_commas(text: str) -> list[str]:
    parts: list[str] = []
    start = 0
    paren_depth = 0
    brace_depth = 0
    for index, char in enumerate(text):
        if char == "(":
            paren_depth += 1
        elif char == ")" and paren_depth > 0:
            paren_depth -= 1
        elif char == "{":
            brace_depth += 1
        elif char == "}" and brace_depth > 0:
            brace_depth -= 1
        elif char == "," and paren_depth == 0 and brace_depth == 0:
            parts.append(text[start:index].strip())
            start = index + 1
    tail = text[start:].strip()
    if tail:
        parts.append(tail)
    return parts


def parse_initializer_entries(source: str, table_name: str) -> list[str]:
    match = re.search(rf"\b{re.escape(table_name)}\[\]\s*=\s*\{{", source)
    if match is None:
        return []

    table_start = source.find("{", match.end() - 1)
    if table_start < 0:
        return []

    table_body_start = table_start + 1
    depth = 1
    table_end = table_body_start
    while table_end < len(source):
        char = source[table_end]
        if char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                break
        table_end += 1
    if depth != 0:
        return []

    body = source[table_body_start:table_end]
    entries: list[str] = []
    index = 0
    while index < len(body):
        if body[index] != "{":
            index += 1
            continue
        start = index + 1
        depth = 1
        index = start
        while index < len(body):
            char = body[index]
            if char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    entries.append(body[start:index].strip())
                    break
            index += 1
        index += 1
    return entries


def parse_runtime_setup_prime_table(source: str, symbols: dict[str, int]) -> dict[str, dict[str, int | None]]:
    policies: dict[str, dict[str, int | None]] = {}
    for entry in parse_initializer_entries(source, "kFgRuntimeSetupPrimePolicies"):
        fields = split_top_level_commas(entry)
        if len(fields) < 3:
            continue
        scene_match = re.fullmatch(r'"([^"]+)"', fields[0].strip())
        if scene_match is None:
            continue
        policies[scene_match.group(1)] = {
            "high": eval_c_int_expr(fields[1], symbols),
            "low": eval_c_int_expr(fields[2], symbols),
        }
    return policies


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

    policy: dict[str, Any] = {
        "symbols": symbols,
        "runtime_setup_prime": parse_runtime_setup_prime_table(source, symbols),
    }
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
    if "FG_SETUP_PRIME_MAX_RESIDENT_BYTES" in symbols:
        policy["setup_prime_max_resident_bytes"] = symbols[
            "FG_SETUP_PRIME_MAX_RESIDENT_BYTES"
        ]
    if "FG_PREFETCH_DEFAULT_WINDOW_BYTES" in symbols:
        policy["default_window_bytes"] = symbols[
            "FG_PREFETCH_DEFAULT_WINDOW_BYTES"
        ]
    if "FG_VISITOR3_SETUP_PRIME_MAX_RESIDENT_BYTES" in symbols:
        policy["visitor3_setup_prime_max_resident_bytes"] = symbols[
            "FG_VISITOR3_SETUP_PRIME_MAX_RESIDENT_BYTES"
        ]
    if "FG_WALKSTUF1_HIGH_RESIDUAL_WINDOW_BYTES" in symbols:
        policy["walkstuf1_high_window_bytes"] = symbols[
            "FG_WALKSTUF1_HIGH_RESIDUAL_WINDOW_BYTES"
        ]
    if "FG_WALKSTUF1_LOW_RESIDUAL_WINDOW_BYTES" in symbols:
        policy["walkstuf1_low_window_bytes"] = symbols[
            "FG_WALKSTUF1_LOW_RESIDUAL_WINDOW_BYTES"
        ]
    if "FG_WALKSTUF1_SETUP_PRIME_BASE_BYTES" in symbols:
        policy["walkstuf1_setup_prime_base_bytes"] = symbols[
            "FG_WALKSTUF1_SETUP_PRIME_BASE_BYTES"
        ]
    if "FG_WALKSTUF1_HIGH_SETUP_PRIME_TRIM_BYTES" in symbols:
        policy["walkstuf1_high_setup_prime_trim_bytes"] = symbols[
            "FG_WALKSTUF1_HIGH_SETUP_PRIME_TRIM_BYTES"
        ]
    if "FG_WALKSTUF1_HIGH_SETUP_PRIME_MAX_RESIDENT_BYTES" in symbols:
        policy["walkstuf1_high_setup_prime_max_resident_bytes"] = symbols[
            "FG_WALKSTUF1_HIGH_SETUP_PRIME_MAX_RESIDENT_BYTES"
        ]
    if "FG_WALKSTUF1_LOW_SETUP_PRIME_MAX_RESIDENT_BYTES" in symbols:
        policy["walkstuf1_low_setup_prime_max_resident_bytes"] = symbols[
            "FG_WALKSTUF1_LOW_SETUP_PRIME_MAX_RESIDENT_BYTES"
        ]
    if "FG_ACTIVITY12_HIGH_SETUP_PRIME_WINDOW_BYTES" in symbols:
        policy["activity12_high_prime_bytes"] = symbols[
            "FG_ACTIVITY12_HIGH_SETUP_PRIME_WINDOW_BYTES"
        ]
    if "FG_VISITOR3_HIGH_SETUP_PRIME_WINDOW_BYTES" in symbols:
        policy["visitor3_high_prime_bytes"] = symbols[
            "FG_VISITOR3_HIGH_SETUP_PRIME_WINDOW_BYTES"
        ]
    if "FG_VISITOR3_LOW_SETUP_PRIME_WINDOW_BYTES" in symbols:
        policy["visitor3_low_prime_bytes"] = symbols[
            "FG_VISITOR3_LOW_SETUP_PRIME_WINDOW_BYTES"
        ]
    if "FG_VISITOR1_HIGH_SETUP_PRIME_WINDOW_BYTES" in symbols:
        policy["visitor1_high_prime_bytes"] = symbols[
            "FG_VISITOR1_HIGH_SETUP_PRIME_WINDOW_BYTES"
        ]
    if "FG_VISITOR7_HIGH_SETUP_PRIME_WINDOW_BYTES" in symbols:
        policy["visitor7_high_prime_bytes"] = symbols[
            "FG_VISITOR7_HIGH_SETUP_PRIME_WINDOW_BYTES"
        ]
    if "FG_FISHING6_HIGH_SETUP_PRIME_WINDOW_BYTES" in symbols:
        policy["fishing6_high_prime_bytes"] = symbols[
            "FG_FISHING6_HIGH_SETUP_PRIME_WINDOW_BYTES"
        ]
    if "FG_FISHING7_HIGH_SETUP_PRIME_WINDOW_BYTES" in symbols:
        policy["fishing7_high_prime_bytes"] = symbols[
            "FG_FISHING7_HIGH_SETUP_PRIME_WINDOW_BYTES"
        ]
    if "FG_JOHNNY3_HIGH_SETUP_PRIME_WINDOW_BYTES" in symbols:
        policy["johnny3_high_prime_bytes"] = symbols[
            "FG_JOHNNY3_HIGH_SETUP_PRIME_WINDOW_BYTES"
        ]

    segmented_scenes = ("FISHING3", "BUILDING2", "BUILDING4", "WALKSTUF1", "VISITOR3")
    for scene_key in segmented_scenes:
        scene_policy_key = scene_key.lower()
        for tide in ("HIGH", "LOW"):
            segments = []
            for suffix in ("", "2", "3", "4"):
                start = symbols.get(f"FG_{scene_key}_{tide}_SETUP_SEGMENT{suffix}_START")
                size = symbols.get(f"FG_{scene_key}_{tide}_SETUP_SEGMENT{suffix}_BYTES")
                if start is not None and size is not None and size > 0:
                    segments.append(
                        (
                            start // SECTOR_SIZE,
                            int(math.ceil((start + size) / SECTOR_SIZE)),
                        )
                    )
            if segments:
                policy[f"{scene_policy_key}_{tide.lower()}_segments"] = segments

    return policy


def clamp_setup_prime_bytes(
    source_policy: dict[str, Any],
    requested: int | None,
    cap_override: int | None = None,
) -> int:
    if requested is None or requested <= 0:
        return 0
    cap = cap_override if cap_override is not None else source_policy.get("setup_prime_max_resident_bytes")
    if isinstance(cap, int) and cap > 0 and requested > cap:
        return cap
    return int(requested)


def runtime_setup_prime_bytes(source_policy: dict[str, Any], scene_name: str | None,
                              lowtide: bool) -> int | None:
    if scene_name is None:
        return None
    table = source_policy.get("runtime_setup_prime")
    if not isinstance(table, dict):
        return None
    entry = table.get(scene_name)
    if not isinstance(entry, dict):
        return None
    key = "low" if lowtide else "high"
    value = entry.get(key)
    return value if isinstance(value, int) else None


def default_setup_policy(case: dict[str, Any]) -> tuple[int, list[tuple[int, int]], str]:
    scene = case.get("sections", {}).get("scene", {})
    scene_name = scene.get("scene")
    lowtide = scene.get("lowtide") == 1
    source_policy = parse_source_setup_policy()
    if scene_name == "fishing1":
        prime = runtime_setup_prime_bytes(source_policy, scene_name, lowtide)
        if prime is None:
            prime = source_policy.get("fishing1_prime_bytes") or 320 * 1024
        return clamp_setup_prime_bytes(source_policy, prime), [], "auto:fishing1"
    if scene_name == "fishing2":
        if lowtide:
            prime = runtime_setup_prime_bytes(source_policy, scene_name, lowtide)
            if prime is None:
                prime = source_policy.get("fishing2_low_prime_bytes")
            return clamp_setup_prime_bytes(source_policy, prime or 256 * 1024), [], "auto:fishing2-low"
        prime = runtime_setup_prime_bytes(source_policy, scene_name, lowtide)
        if prime is None:
            prime = source_policy.get("fishing2_high_prime_bytes")
        return clamp_setup_prime_bytes(source_policy, prime or 352 * 1024), [], "auto:fishing2-high"
    if scene_name == "building2":
        prime = runtime_setup_prime_bytes(source_policy, scene_name, lowtide)
        if prime is not None:
            policy_name = "auto:building2-low" if lowtide else "auto:building2-high"
            segments = source_policy.get(
                "building2_low_segments" if lowtide else "building2_high_segments"
            ) or []
            return clamp_setup_prime_bytes(source_policy, prime), list(segments), policy_name
    if scene_name == "activity12" and not lowtide:
        prime = runtime_setup_prime_bytes(source_policy, scene_name, lowtide)
        if prime is None:
            prime = source_policy.get("activity12_high_prime_bytes")
        return clamp_setup_prime_bytes(source_policy, prime or 328 * 1024), [], "auto:activity12-high"
    if scene_name == "fishing3":
        if lowtide:
            prime = runtime_setup_prime_bytes(source_policy, scene_name, lowtide)
            if prime is None:
                prime = source_policy.get("fishing3_low_prime_bytes")
            segments = source_policy.get("fishing3_low_segments") or [(146, 152)]
            return clamp_setup_prime_bytes(source_policy, prime or 288 * 1024), list(segments), "auto:fishing3-low"
        prime = runtime_setup_prime_bytes(source_policy, scene_name, lowtide)
        if prime is None:
            prime = source_policy.get("fishing3_high_prime_bytes")
        segments = source_policy.get("fishing3_high_segments") or [(67, 73)]
        return clamp_setup_prime_bytes(source_policy, prime or 128 * 1024), list(segments), "auto:fishing3-high"
    if scene_name == "visitor3":
        if lowtide:
            segments = source_policy.get("visitor3_low_segments") or []
            return (
                0,
                list(segments),
                "auto:visitor3-low-clean-relief",
            )
        segments = source_policy.get("visitor3_high_segments") or []
        return (
            0,
            list(segments),
            "auto:visitor3-high-clean-relief",
        )
    if scene_name == "walkstuf1":
        if scene.get("fmt") == "fgp3_indexed8_residual":
            normal = source_policy.get(
                "walkstuf1_low_window_bytes" if lowtide else "walkstuf1_high_window_bytes"
            )
        else:
            normal = source_policy.get("default_window_bytes")
        base = source_policy.get("walkstuf1_setup_prime_base_bytes")
        trim = 0 if lowtide else source_policy.get("walkstuf1_high_setup_prime_trim_bytes")
        if isinstance(normal, int) and isinstance(base, int) and isinstance(trim, int):
            prime = (normal << 2) + base - trim
            cap = source_policy.get(
                "walkstuf1_low_setup_prime_max_resident_bytes" if lowtide
                else "walkstuf1_high_setup_prime_max_resident_bytes"
            )
            cap_override = cap if isinstance(cap, int) and cap > 0 else None
            segments = source_policy.get(
                "walkstuf1_low_segments" if lowtide else "walkstuf1_high_segments"
            ) or []
            return clamp_setup_prime_bytes(source_policy, prime, cap_override), list(segments), (
                "auto:walkstuf1-low" if lowtide else "auto:walkstuf1-high"
            )
    if scene_name == "visitor1" and not lowtide:
        prime = runtime_setup_prime_bytes(source_policy, scene_name, lowtide)
        if prime is None:
            prime = source_policy.get("visitor1_high_prime_bytes")
        return clamp_setup_prime_bytes(source_policy, prime or 296 * 1024), [], "auto:visitor1-high"
    if scene_name == "visitor7" and not lowtide:
        prime = runtime_setup_prime_bytes(source_policy, scene_name, lowtide)
        if prime is None:
            prime = source_policy.get("visitor7_high_prime_bytes")
        return clamp_setup_prime_bytes(source_policy, prime or 368 * 1024), [], "auto:visitor7-high"
    if scene_name == "fishing6" and not lowtide:
        prime = runtime_setup_prime_bytes(source_policy, scene_name, lowtide)
        if prime is None:
            prime = source_policy.get("fishing6_high_prime_bytes")
        return clamp_setup_prime_bytes(source_policy, prime or 312 * 1024), [], "auto:fishing6-high"
    if scene_name == "fishing7" and not lowtide:
        prime = runtime_setup_prime_bytes(source_policy, scene_name, lowtide)
        if prime is None:
            prime = source_policy.get("fishing7_high_prime_bytes")
        return clamp_setup_prime_bytes(source_policy, prime or 328 * 1024), [], "auto:fishing7-high"
    if scene_name == "johnny3" and not lowtide:
        prime = runtime_setup_prime_bytes(source_policy, scene_name, lowtide)
        if prime is None:
            prime = source_policy.get("johnny3_high_prime_bytes")
        return clamp_setup_prime_bytes(source_policy, prime or 312 * 1024), [], "auto:johnny3-high"
    return 0, [], "none"


def skips_auto_fgp3_setup_prime(scene_name: str | None) -> bool:
    return scene_name in {"building1", "visitor5"}


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
                   window_sectors: int, top: int,
                   runtime_group_capacity_sectors: int = 0) -> list[dict[str, Any]]:
    candidates: list[dict[str, Any]] = []
    uncovered_append_starts = sorted({
        int(segment["file_sector_start"])
        for segment in read_segments
        if not range_contains(coverage_ranges,
                              segment["file_sector_start"],
                              segment["file_sector_end"])
    })
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

        append_start_reads = [
            segment for segment in read_segments
            if not range_contains(coverage_ranges,
                                  segment["file_sector_start"],
                                  segment["file_sector_end"])
            and int(segment["file_sector_start"]) == start
        ]
        nearest_append_start = None
        nearest_append_delta = None
        if uncovered_append_starts:
            nearest_append_start = min(uncovered_append_starts,
                                       key=lambda value: abs(value - start))
            nearest_append_delta = nearest_append_start - start

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
        visible_cost = candidate_visible_cost(
            start,
            end,
            fully_covered_reads,
            touched_reads,
            bool(append_start_reads),
        )
        saved_reads = max(0, len(fully_covered_reads) - 1)
        runtime_metadata = runtime_group_metadata(
            start,
            end,
            saved_reads,
            bool(append_start_reads),
            runtime_group_capacity_sectors,
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
            "estimated_saved_reads": saved_reads,
            "touched_read_count": len(touched_reads),
            "estimated_read_vblanks": round(read_vblank_estimate, 2),
            "min_prev_gap_s": round(min(read_gaps), 4) if read_gaps else None,
            "max_prev_gap_s": round(max(read_gaps), 4) if read_gaps else None,
            "avg_prev_gap_s": round(sum(read_gaps) / len(read_gaps), 4) if read_gaps else None,
            "phase_risk_hint": phase_risk_hint(fully_covered_reads),
            "source_read_indices": [segment.get("index") for segment in fully_covered_reads],
            "source_read_segments": [
                read_segment_summary(segment)
                for segment in fully_covered_reads
            ],
            "touched_read_segments": [
                read_segment_summary(segment)
                for segment in touched_reads
            ],
            "append_start_fireable": bool(append_start_reads),
            "append_start_read_indices": [segment.get("index") for segment in append_start_reads],
            "append_start_read_segments": [
                read_segment_summary(segment)
                for segment in append_start_reads
            ],
            "nearest_observed_append_start_sector": nearest_append_start,
            "nearest_observed_append_delta_sectors": nearest_append_delta,
            **visible_cost,
            **runtime_metadata,
        })
        candidates[-1].update(scheduler_retry_metadata(candidates[-1]))

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


def visible_candidate_rows(entries: list[dict[str, Any]], read_segments: list[dict[str, Any]],
                           coverage_ranges: list[tuple[int, int]], pack_sectors: int,
                           window_sectors: int, top: int,
                           runtime_group_capacity_sectors: int = 0) -> list[dict[str, Any]]:
    candidates = candidate_rows(
        entries,
        read_segments,
        coverage_ranges,
        pack_sectors,
        window_sectors,
        top=max(top * 8, top),
        runtime_group_capacity_sectors=runtime_group_capacity_sectors,
    )
    candidates.sort(
        key=lambda item: (
            item["visible_safety_score"],
            item["estimated_saved_reads"],
            -item["visible_risk_score"],
            item["estimated_read_vblanks"],
        ),
        reverse=True,
    )
    return candidates[:top]


def build_report(args: argparse.Namespace) -> dict[str, Any]:
    cdlog = load_cdlog_module()
    summary = load_summary(args.summary)
    case = select_case(summary, args.case_label)
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
    runtime_group_capacity_sectors = 0
    group_bytes = source_policy.get("symbols", {}).get("FG_PREFETCH_GROUP_WINDOW_BYTES")
    if isinstance(group_bytes, int) and group_bytes > 0:
        runtime_group_capacity_sectors = int(math.ceil(group_bytes / SECTOR_SIZE))
    data_offset = int(header.get("data_offset", 0))

    auto_prime_bytes, auto_segments, auto_policy = default_setup_policy(case)
    scene = case.get("sections", {}).get("scene", {})
    scene_name = scene.get("scene")
    if pack.get("magic") == "FGP3" and not skips_auto_fgp3_setup_prime(scene_name):
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
                auto_prime_bytes = clamp_setup_prime_bytes(source_policy, window_bytes)
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
                                    pack_sectors, window, args.top,
                                    runtime_group_capacity_sectors=runtime_group_capacity_sectors)
        for window in args.candidate_sectors
    }
    visible_candidate_sets = {
        str(window): visible_candidate_rows(entries, read_segments, coverage_ranges,
                                            pack_sectors, window, args.top,
                                            runtime_group_capacity_sectors=runtime_group_capacity_sectors)
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
        "runtime_grouping": {
            "current_group_capacity_sectors": runtime_group_capacity_sectors,
            "current_group_capacity_bytes": runtime_group_capacity_sectors * SECTOR_SIZE,
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
        "visible_candidate_sets": visible_candidate_sets,
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
                f"fire={'yes' if item['append_start_fireable'] else 'no'} "
                f"entries={item['covered_entry_count']}({entry_span}) "
                f"bytes={item['covered_payload_bytes']}"
            )
    for sectors, candidates in report.get("visible_candidate_sets", {}).items():
        print(f"\nTop visible-cost {sectors}-sector candidates:")
        if not candidates:
            print("  none")
            continue
        for item in candidates:
            first_gap = (
                f"{item['first_prev_gap_s']}s"
                if item["first_prev_gap_s"] is not None else "-"
            )
            internal_min = (
                f"{item['min_internal_gap_s']}s"
                if item["min_internal_gap_s"] is not None else "-"
            )
            print(
                "  "
                f"{item['start_sector']}:{item['end_sector']} "
                f"score={item['visible_safety_score']} "
                f"hint={item['visible_risk_hint']} "
                f"cd={item.get('visible_cd_cost_class')} "
                f"saved={item['estimated_saved_reads']} "
                f"risk={item['visible_risk_score']} "
                f"partial={item['partial_touch_count']} "
                f"overread={item['group_overread_sectors']} "
                f"first_gap={first_gap} "
                f"internal_min={internal_min} "
                f"seek={item.get('read_seek_direction')} "
                f"fire={'yes' if item['append_start_fireable'] else 'no'} "
                f"fit={'yes' if item.get('runtime_current_group_fit') else 'no'} "
                f"class={item.get('runtime_metadata_class')} "
                f"scheduler={item.get('scheduler_retry_class')} "
                f"reads={item['source_read_indices']}"
            )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--summary", type=Path, help="ps1-perf-iterate summary JSON")
    parser.add_argument("--case-label", help="case label to select from a multi-case summary JSON")
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
