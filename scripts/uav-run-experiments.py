#!/usr/bin/env python3
"""
Run a reproducible UAV communication experiment matrix for the ns-3 study.

The individual ns-3 scratch programs each simulate one communication
architecture.  This script is the layer above them: it starts the same scenario
matrix for every architecture, stores the raw CSV files, and extracts the
important summary metrics into one machine-readable CSV file.

The script intentionally keeps the experiment design explicit.  For a thesis,
this is more valuable than a hidden "clever" generator because the chosen UAV
counts, distances, and timing assumptions can be explained and repeated later.
"""

from __future__ import annotations

import argparse
import csv
import datetime as dt
import importlib.util
import pathlib
import re
import subprocess
import sys
from dataclasses import dataclass
from typing import Iterable


@dataclass(frozen=True)
class Architecture:
    """Configuration that is specific to one implemented architecture."""

    key: str
    program: str
    label: str
    app_start: float | None
    has_building_metrics: bool = False


@dataclass(frozen=True)
class Scenario:
    """One point in the experiment matrix."""

    name: str
    uavs: int
    spacing: int
    sim_time: float
    update_interval: float
    aoi_sample_interval: float
    altitude: float | None = None
    urban_args: dict[str, str] | None = None


# The current thesis comparison contains one direct broadcast reference, one
# routed multi-hop mesh approach, and one infrastructure-based LTE approach.
ARCHITECTURES = (
    Architecture(
        key="wifi-adhoc",
        program="uav-wifi-aoi",
        label="Wi-Fi ad hoc broadcast",
        app_start=None,
    ),
    Architecture(
        key="olsr-mesh",
        program="uav-mesh-olsr-aoi",
        label="Wi-Fi/802.11 OLSR mesh",
        app_start=5.0,
    ),
    Architecture(
        key="lte-infra",
        program="uav-lte-infrastructure-aoi",
        label="LTE/EPC infrastructure",
        app_start=1.0,
    ),
    Architecture(
        key="urban-wifi-adhoc",
        program="uav-urban-wifi-aoi",
        label="Urban Wi-Fi ad hoc broadcast",
        app_start=None,
        has_building_metrics=True,
    ),
    Architecture(
        key="urban-olsr-mesh",
        program="uav-urban-mesh-olsr-aoi",
        label="Urban Wi-Fi/802.11 OLSR mesh",
        app_start=5.0,
        has_building_metrics=True,
    ),
    Architecture(
        key="urban-lte-infra",
        program="uav-urban-lte-infrastructure-aoi",
        label="Urban LTE/EPC infrastructure",
        app_start=1.0,
        has_building_metrics=True,
    ),
)


# The standard matrix is intentionally moderate: 5 UAVs form a small reference
# swarm and 20 UAVs are the main comparison point.  Larger all-to-all LTE
# scenarios are useful, but can take long enough that they belong in "full".
# The two distances separate a dense case from a more demanding sparse case.
STANDARD_UAV_COUNTS = (5, 20)
STANDARD_SPACINGS = (60, 100)

# The full matrix adds an intermediate swarm size and a sparse 160 m case.  It
# is useful for later thesis plots, but can take noticeably longer.
FULL_UAV_COUNTS = (5, 10, 20, 40)
FULL_SPACINGS = (60, 100, 160)

URBAN_FORMS = {
    "urban-open": {
        "blocksX": "3",
        "blocksY": "3",
        "buildingLengthX": "70",
        "buildingLengthY": "70",
        "streetWidth": "60",
        "buildingHeight": "20",
    },
    "urban-baseline": {
        "blocksX": "3",
        "blocksY": "3",
        "buildingLengthX": "80",
        "buildingLengthY": "80",
        "streetWidth": "40",
        "buildingHeight": "35",
    },
    "urban-canyon": {
        "blocksX": "4",
        "blocksY": "4",
        "buildingLengthX": "90",
        "buildingLengthY": "90",
        "streetWidth": "25",
        "buildingHeight": "60",
    },
}

URBAN_HEIGHT_MODES = (
    ("inside", 0.75),
    ("near-roof", 1.0),
    ("above-roof", None),
)


SUMMARY_PATTERNS = {
    "sent_packets": (
        r"Sent application packets:\s+([0-9]+)",
        r"Sent packets:\s+([0-9]+)",
    ),
    "received_packets": (
        r"Received application packets:\s+([0-9]+)\s+/",
        r"Received packets:\s+([0-9]+)\s+/",
    ),
    "expected_received_packets": (
        r"Received application packets:\s+[0-9]+\s+/\s+([0-9]+)",
        r"Received packets:\s+[0-9]+\s+/\s+([0-9]+)",
    ),
    "delivery_ratio": (r"Delivery ratio:\s+([0-9.eE+-]+)",),
    "app_bytes_sent": (r"Application bytes sent:\s+([0-9]+)",),
    "app_bytes_received": (r"Application bytes received:\s+([0-9]+)",),
    "avg_latency_ms": (r"Average latency:\s+([0-9.eE+-]+)\s+ms",),
    "min_latency_ms": (r"Min latency:\s+([0-9.eE+-]+)\s+ms",),
    "max_latency_ms": (r"Max latency:\s+([0-9.eE+-]+)\s+ms",),
    "avg_hops": (
        r"Average hop count:\s+([0-9.eE+-]+)",
        r"Average infrastructure hop estimate:\s+([0-9.eE+-]+)",
    ),
    "min_hops": (
        r"Min hop count:\s+([0-9.eE+-]+)",
        r"Min infrastructure hop estimate:\s+([0-9.eE+-]+)",
    ),
    "max_hops": (
        r"Max hop count:\s+([0-9.eE+-]+)",
        r"Max infrastructure hop estimate:\s+([0-9.eE+-]+)",
    ),
    "known_aoi_samples": (r"Known AoI samples:\s+([0-9]+)",),
    "unknown_aoi_samples": (r"Unknown AoI samples:\s+([0-9]+)",),
    "unknown_aoi_share": (r"Unknown AoI share:\s+([0-9.eE+-]+)",),
    "avg_known_aoi_s": (r"Average known AoI:\s+([0-9.eE+-]+)\s+s",),
    "max_known_aoi_s": (r"Max known AoI:\s+([0-9.eE+-]+)\s+s",),
    "buildings": (r"Buildings:\s+([0-9]+)",),
    "min_pair_distance_m": (r"Min pair distance:\s+([0-9.eE+-]+)\s+m",),
    "avg_pair_distance_m": (r"Average pair distance:\s+([0-9.eE+-]+)\s+m",),
    "avg_nearest_neighbor_distance_m": (
        r"Average nearest-neighbor distance:\s+([0-9.eE+-]+)\s+m",
    ),
    "max_pair_distance_m": (r"Max pair distance:\s+([0-9.eE+-]+)\s+m",),
}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Run UAV communication architecture experiments."
    )
    parser.add_argument(
        "--profile",
        choices=("standard", "full", "smoke", "urban-forms", "urban-heights"),
        default="standard",
        help="Scenario matrix size. 'smoke' is only for a quick script check.",
    )
    parser.add_argument(
        "--results-dir",
        default=None,
        help="Directory for generated CSV/log files. Defaults to results/uav-experiments-<timestamp>.",
    )
    parser.add_argument(
        "--sim-time",
        type=float,
        default=6.0,
        help="Traffic duration in seconds for every scenario.",
    )
    parser.add_argument(
        "--update-interval",
        type=float,
        default=1.0,
        help="Seconds between two generated position updates.",
    )
    parser.add_argument(
        "--aoi-sample-interval",
        type=float,
        default=0.2,
        help="Seconds between two AoI samples.",
    )
    parser.add_argument(
        "--runs",
        type=int,
        default=1,
        help="Number of independent ns-3 RNG runs per architecture/scenario point.",
    )
    parser.add_argument(
        "--only",
        choices=tuple(arch.key for arch in ARCHITECTURES),
        action="append",
        help="Restrict the run to one architecture. Can be used multiple times.",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Print the ns-3 commands without executing them.",
    )
    parser.add_argument(
        "--timeout",
        type=float,
        default=600.0,
        help="Maximum wall-clock seconds for one ns-3 run.",
    )
    parser.add_argument(
        "--skip-steady-state",
        action="store_true",
        help="Do not create steady-state-summary.csv after the experiment run.",
    )
    return parser.parse_args()


def urban_height_altitude(building_height: float, mode: str, factor: float | None) -> float:
    """Return an UAV altitude for one height mode.

    The height study keeps the horizontal placement and urban form fixed while
    moving UAVs vertically relative to the generated buildings:

    * inside: below the roofline, but still on street-corridor positions.
    * near-roof: slightly above the roofline.
    * above-roof: the previous 80 m reference altitude.
    """

    if mode == "above-roof":
        return 80.0
    if mode == "near-roof":
        return building_height + 5.0
    if factor is None:
        raise ValueError(f"height mode {mode} needs a factor")
    return max(5.0, building_height * factor)


def scenario_matrix(profile: str, sim_time: float, update_interval: float, aoi_sample_interval: float) -> list[Scenario]:
    """Return the scenario list for the selected experiment profile."""

    if profile == "smoke":
        uav_counts = (5,)
        spacings = (100,)
        scenario_names = ("grid",)
    elif profile == "full":
        uav_counts = FULL_UAV_COUNTS
        spacings = FULL_SPACINGS
        scenario_names = ("grid",)
    elif profile == "urban-forms":
        uav_counts = (20,)
        spacings = (100,)
        scenario_names = tuple(URBAN_FORMS.keys())
    elif profile == "urban-heights":
        scenarios: list[Scenario] = []
        for urban_name, urban_args in URBAN_FORMS.items():
            building_height = float(urban_args["buildingHeight"])
            for mode, factor in URBAN_HEIGHT_MODES:
                altitude = urban_height_altitude(building_height, mode, factor)
                scenarios.append(
                    Scenario(
                        name=f"{urban_name}-{mode}",
                        uavs=20,
                        spacing=100,
                        sim_time=sim_time,
                        update_interval=update_interval,
                        aoi_sample_interval=aoi_sample_interval,
                        altitude=altitude,
                        urban_args=urban_args,
                    )
                )
        return scenarios
    else:
        uav_counts = STANDARD_UAV_COUNTS
        spacings = STANDARD_SPACINGS
        scenario_names = ("grid",)

    return [
        Scenario(
            name=scenario_name,
            uavs=uavs,
            spacing=spacing,
            sim_time=sim_time,
            update_interval=update_interval,
            aoi_sample_interval=aoi_sample_interval,
            altitude=None,
            urban_args=URBAN_FORMS.get(scenario_name),
        )
        for scenario_name in scenario_names
        for uavs in uav_counts
        for spacing in spacings
    ]


def ns3_root_from_script() -> pathlib.Path:
    """Resolve the ns-3 root directory from this script location."""

    return pathlib.Path(__file__).resolve().parents[1]


def load_analyzer_module():
    """Load the sibling analyzer script while keeping its CLI-friendly filename."""

    analyzer_path = pathlib.Path(__file__).resolve().with_name("uav-analyze-results.py")
    spec = importlib.util.spec_from_file_location("uav_analyze_results", analyzer_path)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"Could not load analyzer script: {analyzer_path}")

    module = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    return module


def build_command(
    ns3_root: pathlib.Path,
    architecture: Architecture,
    scenario: Scenario,
    results_dir: pathlib.Path,
    rng_run: int,
) -> tuple[list[str], pathlib.Path, pathlib.Path, pathlib.Path, pathlib.Path | None]:
    """Build the ./ns3 run command and the output file paths for one run."""

    run_id = f"{architecture.key}_n{scenario.uavs}_d{scenario.spacing}_r{rng_run}"
    if scenario.name != "grid":
        run_id = f"{architecture.key}_{scenario.name}_n{scenario.uavs}_d{scenario.spacing}_r{rng_run}"
    update_csv = results_dir / f"{run_id}_updates.csv"
    aoi_csv = results_dir / f"{run_id}_aoi.csv"
    building_csv = results_dir / f"{run_id}_buildings.csv"
    stdout_log = results_dir / f"{run_id}.log"

    program_parts = [
        architecture.program,
        f"--numUavs={scenario.uavs}",
        f"--spacing={scenario.spacing}",
        f"--simTime={scenario.sim_time}",
        f"--updateInterval={scenario.update_interval}",
        f"--aoiSampleInterval={scenario.aoi_sample_interval}",
        f"--rngRun={rng_run}",
        f"--updateMetricsFile={update_csv}",
        f"--aoiMetricsFile={aoi_csv}",
    ]

    if scenario.altitude is not None:
        program_parts.append(f"--altitude={scenario.altitude}")

    if architecture.app_start is not None:
        program_parts.append(f"--appStart={architecture.app_start}")

    if architecture.has_building_metrics:
        program_parts.append(f"--buildingMetricsFile={building_csv}")
        for key, value in (scenario.urban_args or URBAN_FORMS["urban-baseline"]).items():
            program_parts.append(f"--{key}={value}")

    # Passing the complete scratch-program invocation as one argument is the
    # most reliable way to use the ns-3 wrapper from Python.
    return (
        [str(ns3_root / "ns3"), "run", " ".join(program_parts)],
        update_csv,
        aoi_csv,
        building_csv if architecture.has_building_metrics else None,
        stdout_log,
    )


def first_regex_match(text: str, patterns: Iterable[str]) -> str:
    """Return the first captured value from the provided regex list."""

    for pattern in patterns:
        match = re.search(pattern, text)
        if match:
            return match.group(1)
    return ""


def parse_summary(stdout: str) -> dict[str, str]:
    """Extract the metrics printed by the scratch programs."""

    return {
        metric_name: first_regex_match(stdout, patterns)
        for metric_name, patterns in SUMMARY_PATTERNS.items()
    }


def write_manifest(
    results_dir: pathlib.Path,
    profile: str,
    scenarios: list[Scenario],
    architectures: list[Architecture],
    runs: int,
) -> None:
    """Write a small text file describing what this experiment directory means."""

    manifest = results_dir / "manifest.txt"
    manifest.write_text(
        "\n".join(
            [
                "UAV communication experiment matrix",
                f"created_at={dt.datetime.now().isoformat(timespec='seconds')}",
                f"profile={profile}",
                f"rng_runs=1..{runs}",
                "architectures=" + ",".join(architecture.key for architecture in architectures),
                "uav_counts=" + ",".join(str(value) for value in sorted({scenario.uavs for scenario in scenarios})),
                "spacings=" + ",".join(str(value) for value in sorted({scenario.spacing for scenario in scenarios})),
                "altitudes="
                + ",".join(
                    str(value)
                    for value in sorted(
                        {scenario.altitude for scenario in scenarios if scenario.altitude is not None}
                    )
                ),
                "",
                "Each *_updates.csv and *_aoi.csv file is produced by one ns-3 scratch program.",
                "summary.csv contains the parsed headline metrics for quick comparison.",
                "",
            ]
        ),
        encoding="utf-8",
    )


def main() -> int:
    args = parse_args()
    if args.runs < 1:
        raise SystemExit("--runs must be at least 1")

    ns3_root = ns3_root_from_script()
    analyzer_module = None if args.skip_steady_state or args.dry_run else load_analyzer_module()

    # Urban profiles should only run the urban variants.  The free-grid
    # programs would accept the same UAV counts and altitudes, but not the
    # building parameters, and their results would not describe the intended
    # urban comparison.
    selected_architectures = [
        architecture
        for architecture in ARCHITECTURES
        if (
            args.only is not None
            and architecture.key in args.only
            or args.only is None
            and (not args.profile.startswith("urban-") or architecture.has_building_metrics)
        )
    ]
    scenarios = scenario_matrix(
        args.profile,
        args.sim_time,
        args.update_interval,
        args.aoi_sample_interval,
    )

    if args.results_dir:
        results_dir = pathlib.Path(args.results_dir).resolve()
    else:
        timestamp = dt.datetime.now().strftime("%Y%m%d-%H%M%S")
        results_dir = ns3_root / "results" / f"uav-experiments-{timestamp}"

    if not args.dry_run:
        results_dir.mkdir(parents=True, exist_ok=True)
        write_manifest(results_dir, args.profile, scenarios, selected_architectures, args.runs)

    summary_rows: list[dict[str, str]] = []
    total_runs = len(selected_architectures) * len(scenarios) * args.runs
    absolute_run_number = 0

    for architecture in selected_architectures:
        for scenario in scenarios:
            for rng_run in range(1, args.runs + 1):
                absolute_run_number += 1

                command, update_csv, aoi_csv, building_csv, stdout_log = build_command(
                    ns3_root,
                    architecture,
                    scenario,
                    results_dir,
                    rng_run,
                )
                printable_command = " ".join(command)

                print(
                    f"[{absolute_run_number}/{total_runs}] "
                    f"{architecture.key}: {scenario.uavs} UAVs, {scenario.spacing} m, rngRun {rng_run}",
                    flush=True,
                )
                print(f"  {printable_command}", flush=True)

                if args.dry_run:
                    continue

                try:
                    completed = subprocess.run(
                        command,
                        cwd=ns3_root,
                        check=False,
                        capture_output=True,
                        text=True,
                        timeout=args.timeout,
                    )
                except subprocess.TimeoutExpired as error:
                    stdout_log.write_text(
                        (error.stdout or "") + (error.stderr or ""),
                        encoding="utf-8",
                    )
                    print(f"  timed out after {args.timeout} seconds", file=sys.stderr)
                    print(f"  see {stdout_log}", file=sys.stderr)
                    return 124
                stdout_log.write_text(
                    completed.stdout + completed.stderr,
                    encoding="utf-8",
                )

                if completed.returncode != 0:
                    print(f"  failed with exit code {completed.returncode}", file=sys.stderr)
                    print(f"  see {stdout_log}", file=sys.stderr)
                    return completed.returncode

                row = {
                    "architecture": architecture.key,
                    "architecture_label": architecture.label,
                    "program": architecture.program,
                    "scenario": scenario.name,
                    "num_uavs": str(scenario.uavs),
                    "spacing_m": str(scenario.spacing),
                    "sim_time_s": str(scenario.sim_time),
                    "app_start_s": "" if architecture.app_start is None else str(architecture.app_start),
                    "update_interval_s": str(scenario.update_interval),
                    "aoi_sample_interval_s": str(scenario.aoi_sample_interval),
                    "altitude_m": "" if scenario.altitude is None else str(scenario.altitude),
                    "rng_run": str(rng_run),
                    "urban_args": "" if scenario.urban_args is None else ";".join(
                        f"{key}={value}" for key, value in scenario.urban_args.items()
                    ),
                    "updates_csv": str(update_csv),
                    "aoi_csv": str(aoi_csv),
                    "building_csv": "" if building_csv is None else str(building_csv),
                    "stdout_log": str(stdout_log),
                }
                row.update(parse_summary(completed.stdout))
                summary_rows.append(row)

    if not args.dry_run:
        summary_csv = results_dir / "summary.csv"
        fieldnames = [
            "architecture",
            "architecture_label",
            "program",
            "scenario",
            "num_uavs",
            "spacing_m",
            "sim_time_s",
            "app_start_s",
            "update_interval_s",
            "aoi_sample_interval_s",
            "altitude_m",
            "rng_run",
            "urban_args",
            "sent_packets",
            "received_packets",
            "expected_received_packets",
            "delivery_ratio",
            "app_bytes_sent",
            "app_bytes_received",
            "avg_latency_ms",
            "min_latency_ms",
            "max_latency_ms",
            "avg_hops",
            "min_hops",
            "max_hops",
            "known_aoi_samples",
            "unknown_aoi_samples",
            "unknown_aoi_share",
            "avg_known_aoi_s",
            "max_known_aoi_s",
            "buildings",
            "min_pair_distance_m",
            "avg_pair_distance_m",
            "avg_nearest_neighbor_distance_m",
            "max_pair_distance_m",
            "updates_csv",
            "aoi_csv",
            "building_csv",
            "stdout_log",
        ]
        with summary_csv.open("w", newline="", encoding="utf-8") as output:
            writer = csv.DictWriter(output, fieldnames=fieldnames)
            writer.writeheader()
            writer.writerows(summary_rows)

        if not args.skip_steady_state:
            steady_rows = [
                analyzer_module.analyze_row(row, warmup_intervals=1.5)
                for row in summary_rows
            ]
            steady_csv = results_dir / "steady-state-summary.csv"
            steady_fieldnames = list(steady_rows[0].keys()) if steady_rows else fieldnames
            with steady_csv.open("w", newline="", encoding="utf-8") as output:
                writer = csv.DictWriter(output, fieldnames=steady_fieldnames)
                writer.writeheader()
                writer.writerows(steady_rows)

        print()
        print(f"Experiment complete: {results_dir}", flush=True)
        print(f"Summary CSV: {summary_csv}", flush=True)
        if not args.skip_steady_state:
            print(f"Steady-state CSV: {steady_csv}", flush=True)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
