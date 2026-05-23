#!/usr/bin/env python3
"""Run W1-high PS1 perf candidates in linked clones.

This is the tracked version of the scratch tournament runner used during the
last under-99 optimization pass. It keeps candidate runs isolated, builds each
candidate in its own linked clone, and gates each result against a baseline
summary with --require-improvement.

Default usage:

    ./scripts/ps1-w1high-parallel-swing.py --parallel 32

If --baseline is omitted or points at a missing file, the script first builds
the current tree and captures a fresh W1-high baseline under the output dir.
"""

from __future__ import annotations

import argparse
import concurrent.futures
import datetime as _dt
import json
import os
import shutil
import subprocess
from pathlib import Path
from typing import Any


SCRIPT_DIR = Path(__file__).resolve().parent
ROOT = SCRIPT_DIR.parent
DEFAULT_CASE = (
    "walkstuf1-high::fgpilot walkstuf1 lowtide 0 night 1 holiday 0 "
    "raft-stage 4 island-pos -154 54 perf-log noloop seed 1"
)
DEFAULT_BASELINE = ROOT / (
    "scratch/ps1-perf-iterate/"
    "w1high-frame0-consume-phase4-under-yellow-canary-20260523/"
    "20260523-065754-4032041/summary.json"
)
MUTABLE_PATHS = (
    "src/foreground_pilot.c",
    "config/ps1/bootmode_embedded.h",
    "config/ps1/padscript_embedded.h",
)
EXCLUDES = (
    ".git",
    "scratch",
    "build-host",
    "build-ps1",
    "host-results",
    "regtest-results",
    "site",
    "docs",
)

READ_ANCHOR = "    {344, 360, 0},\n"
DIRECT_STAGE_BLOCK = (
    "    if (!islandState.lowTide &&\n"
    "        gFgRuntimeSceneId == FG_SCENE_WALKSTUF1 &&\n"
    "        frameIndex >= 185 &&\n"
    "        frameIndex <= 191 &&\n"
    "        slackVBlanks > minSlack)\n"
    "        return 1;\n\n"
)
CAN_PREPARE_MACRO = (
    "#define fgRuntimeCanPrepareStagedFrame() \\\n"
    "    (gFgRuntime.active && \\\n"
    "     gFgRuntime.mode == FG_RUNTIME_SCENE_PACK && \\\n"
    "     gFgRuntime.frameRendered && \\\n"
    "     gFgRuntime.stagedFrameValid && \\\n"
    "     !gFgRuntime.preparedFrameValid && \\\n"
    "     gFgRuntime.stagedFrameIndex == (uint16)(gFgRuntime.frameIndex + 1) && \\\n"
    "     ((fgRuntimeHeldSlackBeforeWait() == FG_PREPARE_PRESENT_MIN_SLACK_VBLANKS) || \\\n"
    "      (!islandState.lowTide && \\\n"
    "       gFgRuntimeSceneId == FG_SCENE_WALKSTUF1 && \\\n"
    "       fgRuntimeHeldSlackBeforeWait() == 2 && \\\n"
    "       gFgRuntime.frameIndex >= 128 && \\\n"
    "       gFgRuntime.frameIndex <= 191)))"
)
CATCHUP_LINE = "            uint16 catchupThreshold = gFgRuntime.setupWindowPrimed ? 4 : 5;"
HIGH_GUARD = (
    "    if (gFgRuntimeSceneId == FG_SCENE_WALKSTUF1 &&\n"
    "        slackVBlanks <= (uint16)(islandState.lowTide ?\n"
    "            FG_PREFETCH_WINDOW_MIN_SLACK_VBLANKS :\n"
    "            FG_PREFETCH_WINDOW_MIN_SLACK_VBLANKS + 1)) {\n"
)


def run(cmd: list[str], cwd: Path, **kwargs: Any) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        cmd,
        cwd=cwd,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        **kwargs,
    )


def remove_path(path: Path) -> None:
    if not path.exists() and not path.is_symlink():
        return
    subprocess.run(
        ["chmod", "-R", "u+rwX", str(path)],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )
    if path.is_symlink():
        path.unlink()
    else:
        shutil.rmtree(path, ignore_errors=True)


def break_hardlink(path: Path) -> None:
    if not path.exists() or path.is_symlink():
        return
    try:
        if path.stat().st_nlink <= 1:
            return
    except FileNotFoundError:
        return
    tmp = path.with_name(f"{path.name}.unlink-{os.getpid()}")
    shutil.copy2(path, tmp)
    os.replace(tmp, path)


def make_linked_clone(wt: Path) -> subprocess.CompletedProcess[str]:
    remove_path(wt)
    wt.parent.mkdir(parents=True, exist_ok=True)
    cmd = ["rsync", "-a", "--delete", f"--link-dest={ROOT}"]
    for exclude in EXCLUDES:
        cmd.append(f"--exclude={exclude}")
    cmd.extend([f"{ROOT}/", f"{wt}/"])
    result = run(cmd, ROOT)
    if result.returncode == 0:
        for rel in MUTABLE_PATHS:
            break_hardlink(wt / rel)
    return result


def direct_stage_block(ranges: list[list[int]], extra_slack: int = 0) -> str:
    threshold = "minSlack" if extra_slack == 0 else f"(uint16)(minSlack + {extra_slack})"
    clauses = " ||\n        ".join(
        f"(frameIndex >= {int(start)} && frameIndex <= {int(end)})"
        for start, end in ranges
    )
    return (
        "    if (!islandState.lowTide &&\n"
        "        gFgRuntimeSceneId == FG_SCENE_WALKSTUF1 &&\n"
        f"        ({clauses}) &&\n"
        f"        slackVBlanks > {threshold})\n"
        "        return 1;\n\n"
    )


def prep_macro(windows: list[list[int]]) -> str:
    parts = []
    for start, end, min_slack, max_slack in windows:
        parts.append(
            "       (fgRuntimeHeldSlackBeforeWait() >= "
            f"{int(min_slack)} && \\\n"
            "        fgRuntimeHeldSlackBeforeWait() <= "
            f"{int(max_slack)} && \\\n"
            "        gFgRuntime.frameIndex >= "
            f"{int(start)} && \\\n"
            "        gFgRuntime.frameIndex <= "
            f"{int(end)})"
        )
    joined = " || \\\n".join(parts)
    return (
        "#define fgRuntimeCanPrepareStagedFrame() \\\n"
        "    (gFgRuntime.active && \\\n"
        "     gFgRuntime.mode == FG_RUNTIME_SCENE_PACK && \\\n"
        "     gFgRuntime.frameRendered && \\\n"
        "     gFgRuntime.stagedFrameValid && \\\n"
        "     !gFgRuntime.preparedFrameValid && \\\n"
        "     gFgRuntime.stagedFrameIndex == (uint16)(gFgRuntime.frameIndex + 1) && \\\n"
        "     ((fgRuntimeHeldSlackBeforeWait() == FG_PREPARE_PRESENT_MIN_SLACK_VBLANKS) || \\\n"
        "      (!islandState.lowTide && \\\n"
        "       gFgRuntimeSceneId == FG_SCENE_WALKSTUF1 && \\\n"
        f"       ({joined}))))"
    )


def replace_define(text: str, old_tail: str, new_tail: str) -> str:
    old = f"#define {old_tail}"
    new = f"#define {new_tail}"
    if old not in text:
        raise RuntimeError(f"missing define anchor: {old}")
    return text.replace(old, new, 1)


def apply_candidate(wt: Path, candidate: dict[str, Any]) -> None:
    path = wt / "src/foreground_pilot.c"
    break_hardlink(path)
    text = path.read_text()

    if "phase" in candidate:
        text = replace_define(
            text,
            "FG_WALKSTUF1_HIGH_PHASE_VBLANKS 4",
            f"FG_WALKSTUF1_HIGH_PHASE_VBLANKS {int(candidate['phase'])}",
        )

    if "catchup_threshold" in candidate:
        value = int(candidate["catchup_threshold"])
        new_line = (
            "            uint16 catchupThreshold = (!islandState.lowTide &&\n"
            "                gFgRuntimeSceneId == FG_SCENE_WALKSTUF1) ?\n"
            f"                {value} : (gFgRuntime.setupWindowPrimed ? 4 : 5);"
        )
        if CATCHUP_LINE not in text:
            raise RuntimeError("missing catchup threshold anchor")
        text = text.replace(CATCHUP_LINE, new_line, 1)

    if "high_guard_delta" in candidate:
        delta = int(candidate["high_guard_delta"])
        high_expr = (
            "FG_PREFETCH_WINDOW_MIN_SLACK_VBLANKS"
            if delta <= 0
            else f"FG_PREFETCH_WINDOW_MIN_SLACK_VBLANKS + {delta}"
        )
        if HIGH_GUARD not in text:
            raise RuntimeError("missing W1 high guard anchor")
        text = text.replace(
            HIGH_GUARD,
            HIGH_GUARD.replace(
                "FG_PREFETCH_WINDOW_MIN_SLACK_VBLANKS + 1))",
                f"{high_expr}))",
            ),
            1,
        )

    for old_tail, new_tail in candidate.get("replace", {}).items():
        text = replace_define(text, old_tail, new_tail)

    for old, new in candidate.get("replace_text", {}).items():
        if old not in text:
            raise RuntimeError(f"missing text anchor: {old!r}")
        text = text.replace(old, new, 1)

    if candidate.get("prep_windows"):
        if CAN_PREPARE_MACRO not in text:
            raise RuntimeError("missing prep macro anchor")
        text = text.replace(CAN_PREPARE_MACRO, prep_macro(candidate["prep_windows"]), 1)

    if candidate.get("read_add"):
        rows = "".join(
            f"    {{{int(s)}, {int(e)}, {int(slack)}}},\n"
            for s, e, slack in candidate["read_add"]
        )
        if READ_ANCHOR not in text:
            raise RuntimeError("missing read group anchor")
        text = text.replace(READ_ANCHOR, rows + READ_ANCHOR, 1)

    for start, end, slack in candidate.get("read_slack", []):
        old = f"    {{{int(start)}, {int(end)}, 0}},\n"
        new = f"    {{{int(start)}, {int(end)}, {int(slack)}}},\n"
        if old not in text:
            raise RuntimeError(f"missing read-slack row: {old.strip()}")
        text = text.replace(old, new, 1)

    for start, end in candidate.get("read_remove", []):
        old = f"    {{{int(start)}, {int(end)}, 0}},\n"
        if old not in text:
            raise RuntimeError(f"missing read-remove row: {old.strip()}")
        text = text.replace(old, "", 1)

    if candidate.get("direct_ranges"):
        if DIRECT_STAGE_BLOCK not in text:
            raise RuntimeError("missing direct stage anchor")
        text = text.replace(
            DIRECT_STAGE_BLOCK,
            direct_stage_block(
                candidate["direct_ranges"],
                int(candidate.get("direct_extra_slack", 0)),
            ),
            1,
        )

    path.write_text(text)


def summarize(summary: Path) -> dict[str, Any]:
    if not summary.exists():
        return {"gate_pass": False, "error": "missing summary"}
    data = json.loads(summary.read_text())
    case = data.get("cases", [{}])[0] if data.get("cases") else {}
    sections = case.get("sections", {})
    timing = sections.get("timing", {})
    cd = sections.get("cd", {})
    prefetch = sections.get("prefetch", {})
    scene = sections.get("scene", {})
    failures = []
    for failure in data.get("overall_failures", []):
        if failure.get("label") == case.get("label"):
            failures = failure.get("failures", [])
            break
    return {
        "gate_pass": data.get("overall_pass", False),
        "failures": failures,
        "scene": timing.get("scene_vb"),
        "loop": timing.get("loop_vb"),
        "target": timing.get("target_vb"),
        "over": timing.get("overrun_vb"),
        "blocking": cd.get("blocking_vb"),
        "refill": prefetch.get("overrun_vb"),
        "reads": cd.get("loop_reads"),
        "due": prefetch.get("due_misses"),
        "pack_lba": scene.get("pack_lba"),
        "ps_exe": sections.get("exe", {}).get("sector_bucket_bytes"),
        "summary": str(summary),
    }


def latest_summary(out_dir: Path) -> Path | None:
    summaries = sorted(out_dir.glob("*/summary.json"))
    return summaries[-1] if summaries else None


def ensure_baseline(args: argparse.Namespace, out: Path) -> Path:
    baseline = Path(args.baseline).expanduser() if args.baseline else DEFAULT_BASELINE
    if not baseline.is_absolute():
        baseline = ROOT / baseline
    if baseline.exists():
        return baseline

    baseline_out = out / "baseline"
    baseline_out.mkdir(parents=True, exist_ok=True)
    print(f"baseline missing; capturing current W1-high baseline under {baseline_out}", flush=True)
    build = run(["./scripts/build-ps1.sh"], ROOT, timeout=args.build_timeout)
    (out / "baseline.build.log").write_text(build.stdout)
    if build.returncode != 0:
        raise RuntimeError(f"baseline build failed: {out / 'baseline.build.log'}")
    perf = run(
        [
            "./scripts/ps1-perf-iterate.sh",
            "--case-local-cd",
            "--timeout",
            str(args.timeout),
            "--output",
            str(baseline_out),
            "--case",
            args.case,
        ],
        ROOT,
        timeout=args.perf_timeout,
    )
    (out / "baseline.perf.log").write_text(perf.stdout)
    summary = latest_summary(baseline_out)
    if perf.returncode != 0 or summary is None:
        raise RuntimeError(f"baseline perf failed: {out / 'baseline.perf.log'}")
    return summary


def run_candidate(
    candidate: dict[str, Any],
    args: argparse.Namespace,
    out: Path,
    wtroot: Path,
    baseline: Path,
) -> dict[str, Any]:
    name = candidate["name"]
    wt = wtroot / name
    log_dir = out / "logs"
    log_dir.mkdir(parents=True, exist_ok=True)
    clone = make_linked_clone(wt)
    if clone.returncode != 0:
        return {"name": name, "setup_error": clone.stdout}
    try:
        apply_candidate(wt, candidate)
        build = run(["./scripts/build-ps1.sh"], wt, timeout=args.build_timeout)
        (log_dir / f"{name}.build.log").write_text(build.stdout)
        if build.returncode != 0:
            return {
                "name": name,
                "build_failed": True,
                "build_log": str(log_dir / f"{name}.build.log"),
            }
        perf_out = wt / "scratch/ps1-perf-iterate" / name
        perf = run(
            [
                "./scripts/ps1-perf-iterate.sh",
                "--case-local-cd",
                "--timeout",
                str(args.timeout),
                "--baseline",
                str(baseline),
                "--require-improvement",
                "--output",
                str(perf_out),
                "--case",
                args.case,
            ],
            wt,
            timeout=args.perf_timeout,
        )
        (log_dir / f"{name}.perf.log").write_text(perf.stdout)
        summary = latest_summary(perf_out) or (perf_out / "missing-summary.json")
        result = summarize(summary)
        result.update(
            {
                "name": name,
                "returncode": perf.returncode,
                "perf_log": str(log_dir / f"{name}.perf.log"),
                "worktree": str(wt),
            }
        )
        return result
    except subprocess.TimeoutExpired as exc:
        return {"name": name, "timeout": True, "output": exc.stdout}
    except Exception as exc:  # noqa: BLE001 - tournament output should keep moving.
        return {"name": name, "exception": repr(exc), "worktree": str(wt)}
    finally:
        if not args.keep_worktrees:
            remove_path(wt)


def load_candidates(path: Path | None) -> list[dict[str, Any]]:
    if path is not None:
        return json.loads(path.read_text())
    return [
        {"name": "prep2_hot35_61", "prep_windows": [[35, 61, 2, 2], [128, 191, 2, 2]]},
        {"name": "prep2_hot38_49", "prep_windows": [[38, 49, 2, 2], [128, 191, 2, 2]]},
        {"name": "prep2_hot43_57", "prep_windows": [[43, 57, 2, 2], [128, 191, 2, 2]]},
        {"name": "prep2_mid82_92", "prep_windows": [[82, 92, 2, 2], [128, 191, 2, 2]]},
        {"name": "prep23_hot35_61", "prep_windows": [[35, 61, 2, 3], [128, 191, 2, 2]]},
        {"name": "direct183_s2", "direct_ranges": [[183, 191]], "direct_extra_slack": 1},
        {"name": "direct183_s2_rg80_92", "direct_ranges": [[183, 191]], "direct_extra_slack": 1, "read_add": [[80, 92, 4]]},
        {"name": "direct183_s2_rg74_98", "direct_ranges": [[183, 191]], "direct_extra_slack": 1, "read_add": [[74, 98, 4]]},
        {"name": "direct183_s2_rg365_381", "direct_ranges": [[183, 191]], "direct_extra_slack": 1, "read_add": [[365, 381, 4]]},
        {"name": "rg80_92_s4", "read_add": [[80, 92, 4]]},
        {"name": "rg74_98_s4", "read_add": [[74, 98, 4]]},
        {"name": "rg253_265_s4", "read_add": [[253, 265, 4]]},
        {"name": "rg246_258_s4", "read_add": [[246, 258, 4]]},
        {"name": "rg365_381_s4", "read_add": [[365, 381, 4]]},
        {"name": "rg377_383_s4", "read_add": [[377, 383, 4]]},
        {"name": "rg358_382_s4", "read_add": [[358, 382, 4]]},
        {"name": "window52", "replace": {"FG_WALKSTUF1_HIGH_RESIDUAL_WINDOW_BYTES (54UL * 1024UL)": "FG_WALKSTUF1_HIGH_RESIDUAL_WINDOW_BYTES (52UL * 1024UL)"}},
        {"name": "window56", "replace": {"FG_WALKSTUF1_HIGH_RESIDUAL_WINDOW_BYTES (54UL * 1024UL)": "FG_WALKSTUF1_HIGH_RESIDUAL_WINDOW_BYTES (56UL * 1024UL)"}},
        {"name": "guard_high3", "high_guard_delta": 0},
        {"name": "guard_high5", "high_guard_delta": 2},
        {"name": "catchup7", "catchup_threshold": 7},
        {"name": "catchup7_direct183_s2", "catchup_threshold": 7, "direct_ranges": [[183, 191]], "direct_extra_slack": 1},
    ]


def print_table(results: list[dict[str, Any]]) -> None:
    print("\nname,pass,scene,loop,target,over,blocking,refill,reads,due,summary")
    for r in results:
        print(
            "{name},{gate_pass},{scene},{loop},{target},{over},{blocking},{refill},{reads},{due},{summary}".format(
                name=r.get("name"),
                gate_pass=r.get("gate_pass", False),
                scene=r.get("scene", ""),
                loop=r.get("loop", ""),
                target=r.get("target", ""),
                over=r.get("over", ""),
                blocking=r.get("blocking", ""),
                refill=r.get("refill", ""),
                reads=r.get("reads", ""),
                due=r.get("due", ""),
                summary=r.get("summary", ""),
            )
        )


def parse_args() -> argparse.Namespace:
    today = _dt.datetime.now().strftime("%Y%m%d-%H%M%S")
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--baseline", default=str(DEFAULT_BASELINE), help="baseline summary.json; captured automatically if missing")
    parser.add_argument("--case", default=DEFAULT_CASE, help="LABEL::BOOT case string")
    parser.add_argument("--out", default=str(ROOT / f"scratch/w1high-parallel-swing-{today}"), help="output directory")
    parser.add_argument("--worktree-root", default=str(ROOT.parent / "jc_reborn_parallel_worktrees" / f"w1high_{os.getpid()}"))
    parser.add_argument("--parallel", type=int, default=int(os.environ.get("W1H_PARALLEL", "20")))
    parser.add_argument("--timeout", type=int, default=300, help="per-case ps1-perf-iterate timeout argument")
    parser.add_argument("--build-timeout", type=int, default=240, help="wall-clock seconds for each build")
    parser.add_argument("--perf-timeout", type=int, default=420, help="wall-clock seconds for each perf run")
    parser.add_argument("--candidates", type=Path, help="optional JSON list of candidate dicts")
    parser.add_argument("--only", help="comma-separated candidate names to run")
    parser.add_argument("--list", action="store_true", help="list candidates and exit")
    parser.add_argument("--keep-worktrees", action="store_true", help="leave candidate clones for inspection")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    out = Path(args.out).resolve()
    wtroot = Path(args.worktree_root).resolve()
    out.mkdir(parents=True, exist_ok=True)
    wtroot.mkdir(parents=True, exist_ok=True)

    candidates = load_candidates(args.candidates)
    if args.only:
        allow = {name.strip() for name in args.only.split(",") if name.strip()}
        candidates = [c for c in candidates if c["name"] in allow]
    if args.list:
        for c in candidates:
            print(c["name"])
        return 0
    if not candidates:
        raise SystemExit("no candidates selected")

    baseline = ensure_baseline(args, out)
    print(f"baseline={baseline}")
    print(f"out={out}")
    print(f"worktree_root={wtroot}")
    print(f"candidates={len(candidates)} parallel={args.parallel}")

    results: list[dict[str, Any]] = []
    with concurrent.futures.ThreadPoolExecutor(max_workers=args.parallel) as pool:
        futures = [
            pool.submit(run_candidate, candidate, args, out, wtroot, baseline)
            for candidate in candidates
        ]
        for future in concurrent.futures.as_completed(futures):
            result = future.result()
            results.append(result)
            print(json.dumps(result, sort_keys=True), flush=True)

    results.sort(
        key=lambda r: (
            not r.get("gate_pass", False),
            r.get("loop") if isinstance(r.get("loop"), int) else 99999,
            r.get("blocking") if isinstance(r.get("blocking"), int) else 99999,
            r.get("refill") if isinstance(r.get("refill"), int) else 99999,
        )
    )
    (out / "results.json").write_text(json.dumps(results, indent=2) + "\n")
    print_table(results)
    if not args.keep_worktrees:
        remove_path(wtroot)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
