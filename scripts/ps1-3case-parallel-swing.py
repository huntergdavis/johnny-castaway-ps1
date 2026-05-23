#!/usr/bin/env python3
"""Run multi-case PS1 perf candidates in parallel linked clones.

Like `ps1-w1high-parallel-swing.py` but runs every candidate against all three
yellow under-99 cases (walkstuf1-high, visitor3-high, visitor3-low) and applies
a custom gate:

  - no case may regress key metrics beyond the allow_pct tolerance,
  - at least one targeted case must improve a key metric vs its baseline label,
  - non-target cases must be exact-flat or improved.

Candidates use the same JSON schema as the W1-high swing (the `replace_text`
keys are exercised; the structural anchors specific to W1-high are not).
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

DEFAULT_CASES = (
    "walkstuf1-high::fgpilot walkstuf1 lowtide 0 night 1 holiday 0 raft-stage 4 island-pos -154 54 perf-log noloop seed 1",
    "visitor3-high::fgpilot visitor3 lowtide 0 night 1 holiday 0 raft-stage 4 island-pos -154 54 perf-log noloop seed 1",
    "visitor3-low::fgpilot visitor3 lowtide 1 night 1 holiday 0 raft-stage 4 island-pos -154 54 perf-log noloop seed 1",
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

KEY_TIMING_FIELDS = (
    ("timing", "scene_vb"),
    ("timing", "loop_vb"),
    ("timing", "target_vb"),
    ("timing", "overrun_vb"),
    ("cd", "blocking_vb"),
    ("prefetch", "overrun_vb"),
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


def apply_candidate(wt: Path, candidate: dict[str, Any]) -> None:
    path = wt / "src/foreground_pilot.c"
    break_hardlink(path)
    text = path.read_text()
    for old, new in candidate.get("replace_text", {}).items():
        if old not in text:
            raise RuntimeError(f"missing text anchor: {old!r}")
        text = text.replace(old, new, 1)
    path.write_text(text)


def field(section_data: dict[str, Any], key: str) -> Any:
    return section_data.get(key) if isinstance(section_data, dict) else None


def case_metrics(case: dict[str, Any]) -> dict[str, int]:
    sections = case.get("sections", {})
    return {
        "scene_vb": field(sections.get("timing", {}), "scene_vb"),
        "loop_vb": field(sections.get("timing", {}), "loop_vb"),
        "target_vb": field(sections.get("timing", {}), "target_vb"),
        "overrun_vb": field(sections.get("timing", {}), "overrun_vb"),
        "blocking_vb": field(sections.get("cd", {}), "blocking_vb"),
        "refill_vb": field(sections.get("prefetch", {}), "overrun_vb"),
        "loop_reads": field(sections.get("cd", {}), "loop_reads"),
        "due_misses": field(sections.get("prefetch", {}), "due_misses"),
        "pack_lba": field(sections.get("scene", {}), "pack_lba"),
        "ps_exe_bytes": field(sections.get("exe", {}), "sector_bucket_bytes"),
    }


def parse_summary(summary_path: Path) -> dict[str, dict[str, int]]:
    if not summary_path.exists():
        return {}
    data = json.loads(summary_path.read_text())
    return {case.get("label"): case_metrics(case) for case in data.get("cases", [])}


def load_baseline(baseline_path: Path) -> dict[str, dict[str, int]]:
    data = json.loads(baseline_path.read_text())
    return {case.get("label"): case_metrics(case) for case in data.get("cases", [])}


def compare_case(current: dict[str, int], baseline: dict[str, int]) -> dict[str, Any]:
    deltas: dict[str, int] = {}
    improved = []
    regressed = []
    if not current or not baseline:
        return {"deltas": deltas, "improved": improved, "regressed": regressed}
    for key in ("scene_vb", "loop_vb", "target_vb", "overrun_vb", "blocking_vb", "refill_vb"):
        c = current.get(key)
        b = baseline.get(key)
        if c is None or b is None:
            continue
        delta = c - b
        deltas[key] = delta
        # for these key timing fields, smaller is better
        if delta < 0:
            improved.append(key)
        elif delta > 0:
            regressed.append(key)
    return {"deltas": deltas, "improved": improved, "regressed": regressed}


def evaluate(result: dict[str, dict[str, Any]], target_labels: set[str]) -> dict[str, Any]:
    summary = {"pass": True, "reasons": [], "improved_labels": [], "regressed_labels": []}
    target_improved = False
    for label, cmp in result.items():
        if cmp["regressed"]:
            summary["pass"] = False
            summary["regressed_labels"].append(label)
            summary["reasons"].append(f"{label} regressed {cmp['regressed']} (deltas={cmp['deltas']})")
        if cmp["improved"]:
            summary["improved_labels"].append(label)
            if label in target_labels:
                target_improved = True
    if not target_improved:
        summary["pass"] = False
        summary["reasons"].append("no target case improved a key metric")
    return summary


def latest_summary(out_dir: Path) -> Path | None:
    summaries = sorted(out_dir.glob("*/summary.json"))
    return summaries[-1] if summaries else None


def run_candidate(
    candidate: dict[str, Any],
    args: argparse.Namespace,
    out: Path,
    wtroot: Path,
    baseline_metrics: dict[str, dict[str, int]],
    cases: list[str],
    target_labels: set[str],
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
        perf_cmd = [
            "./scripts/ps1-perf-iterate.sh",
            "--case-local-cd",
            "--timeout",
            str(args.timeout),
            "--output",
            str(perf_out),
        ]
        for c in cases:
            perf_cmd.extend(["--case", c])
        perf = run(perf_cmd, wt, timeout=args.perf_timeout)
        (log_dir / f"{name}.perf.log").write_text(perf.stdout)
        summary = latest_summary(perf_out)
        if summary is None:
            return {"name": name, "no_summary": True, "perf_log": str(log_dir / f"{name}.perf.log")}
        per_case = parse_summary(summary)
        compare = {
            label: compare_case(per_case.get(label, {}), baseline_metrics.get(label, {}))
            for label in baseline_metrics
        }
        verdict = evaluate(compare, target_labels)
        return {
            "name": name,
            "summary": str(summary),
            "perf_log": str(log_dir / f"{name}.perf.log"),
            "metrics": per_case,
            "compare": compare,
            "verdict": verdict,
        }
    except subprocess.TimeoutExpired as exc:
        return {"name": name, "timeout": True, "output": exc.stdout}
    except Exception as exc:  # noqa: BLE001 — keep tournament moving on errors.
        return {"name": name, "exception": repr(exc), "worktree": str(wt)}
    finally:
        if not args.keep_worktrees:
            remove_path(wt)


def parse_args() -> argparse.Namespace:
    today = _dt.datetime.now().strftime("%Y%m%d-%H%M%S")
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--baseline", required=True, help="3-case baseline summary.json")
    parser.add_argument("--candidates", required=True, type=Path, help="candidates JSON list")
    parser.add_argument("--case", action="append", default=[], help="LABEL::BOOT (repeatable; defaults to walkstuf1-high + visitor3-high + visitor3-low)")
    parser.add_argument("--target", action="append", default=[], help="labels that must improve at least once (repeatable; default all cases)")
    parser.add_argument("--out", default=str(ROOT / f"scratch/3case-parallel-swing-{today}"))
    parser.add_argument("--worktree-root", default=str(ROOT / f"scratch/parallel-worktrees/3case_{today}"))
    parser.add_argument("--parallel", type=int, default=int(os.environ.get("PS1_PARALLEL", "20")))
    parser.add_argument("--timeout", type=int, default=300)
    parser.add_argument("--build-timeout", type=int, default=240)
    parser.add_argument("--perf-timeout", type=int, default=600)
    parser.add_argument("--keep-worktrees", action="store_true")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    out = Path(args.out).resolve()
    wtroot = Path(args.worktree_root).resolve()
    out.mkdir(parents=True, exist_ok=True)
    wtroot.mkdir(parents=True, exist_ok=True)

    cases = list(args.case) or list(DEFAULT_CASES)
    target_labels = set(args.target) if args.target else {c.split("::", 1)[0] for c in cases}

    baseline = Path(args.baseline).expanduser()
    if not baseline.is_absolute():
        baseline = ROOT / baseline
    if not baseline.exists():
        raise SystemExit(f"baseline missing: {baseline}")
    baseline_metrics = load_baseline(baseline)
    for label in target_labels:
        if label not in baseline_metrics:
            raise SystemExit(f"target {label} not in baseline labels: {sorted(baseline_metrics)}")

    candidates = json.loads(args.candidates.read_text())
    print(f"baseline={baseline}")
    print(f"out={out}")
    print(f"worktree_root={wtroot}")
    print(f"cases={[c.split('::',1)[0] for c in cases]} targets={sorted(target_labels)}")
    print(f"candidates={len(candidates)} parallel={args.parallel}")

    results: list[dict[str, Any]] = []
    with concurrent.futures.ThreadPoolExecutor(max_workers=args.parallel) as pool:
        futures = [
            pool.submit(run_candidate, c, args, out, wtroot, baseline_metrics, cases, target_labels)
            for c in candidates
        ]
        for future in concurrent.futures.as_completed(futures):
            result = future.result()
            results.append(result)
            print(json.dumps(result, sort_keys=True))

    (out / "results.json").write_text(json.dumps(results, indent=2))

    print("\nname,verdict,improved,regressed,w1high_o/b/r,v3high_o/b/r,v3low_o/b/r")
    for r in sorted(results, key=lambda x: x.get("name", "")):
        v = r.get("verdict", {})
        verdict = "PASS" if v.get("pass") else "fail"
        improved = ",".join(v.get("improved_labels", []))
        regressed = ",".join(v.get("regressed_labels", []))

        def fmt(label: str) -> str:
            m = r.get("metrics", {}).get(label, {}) or {}
            return f"{m.get('overrun_vb', '?')}/{m.get('blocking_vb', '?')}/{m.get('refill_vb', '?')}"

        print(
            f"{r.get('name')},{verdict},[{improved}],[{regressed}],{fmt('walkstuf1-high')},{fmt('visitor3-high')},{fmt('visitor3-low')}"
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
