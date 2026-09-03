#!/usr/bin/env python3
"""
Aggregate repeated UAV steady-state experiment runs.

Input is a `steady-state-summary.csv` produced by `uav-analyze-results.py`.
Rows with the same architecture, scenario, UAV count, spacing, and urban
parameters are treated as repetitions.  The script writes one row per group and
metric with mean, sample standard deviation, and a 95 percent confidence
interval half width.

This is intentionally separate from the Markdown report: the report is for
reading, this CSV is for plots and thesis tables.
"""

from __future__ import annotations

import argparse
import csv
import math
import pathlib
import statistics
from collections import defaultdict


GROUP_KEYS = (
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
    "urban_args",
)

METRICS = (
    "steady_delivery_ratio",
    "steady_avg_latency_ms",
    "steady_unknown_aoi_share",
    "steady_avg_known_aoi_s",
    "steady_max_known_aoi_s",
    "steady_avg_hops",
    "steady_app_bytes_received",
    "delivery_ratio",
    "avg_latency_ms",
    "app_bytes_sent",
    "app_bytes_received",
    "min_pair_distance_m",
    "avg_pair_distance_m",
    "avg_nearest_neighbor_distance_m",
    "max_pair_distance_m",
)

T_CRITICAL_95 = {
    1: 12.706,
    2: 4.303,
    3: 3.182,
    4: 2.776,
    5: 2.571,
    6: 2.447,
    7: 2.365,
    8: 2.306,
    9: 2.262,
    10: 2.228,
    11: 2.201,
    12: 2.179,
    13: 2.160,
    14: 2.145,
    15: 2.131,
    16: 2.120,
    17: 2.110,
    18: 2.101,
    19: 2.093,
    20: 2.086,
    21: 2.080,
    22: 2.074,
    23: 2.069,
    24: 2.064,
    25: 2.060,
    26: 2.056,
    27: 2.052,
    28: 2.048,
    29: 2.045,
    30: 2.042,
}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Aggregate repeated UAV result rows.")
    parser.add_argument("input", help="steady-state-summary.csv input file.")
    parser.add_argument(
        "--output",
        default=None,
        help="Output CSV. Defaults to <input-dir>/aggregate-summary.csv.",
    )
    return parser.parse_args()


def numeric_values(rows: list[dict[str, str]], metric: str) -> list[float]:
    values = []
    for row in rows:
        value = row.get(metric, "")
        if value != "":
            values.append(float(value))
    return values


def group_key(row: dict[str, str]) -> tuple[str, ...]:
    return tuple(row.get(key, "") for key in GROUP_KEYS)


def aggregate_metric(rows: list[dict[str, str]], metric: str) -> dict[str, str] | None:
    values = numeric_values(rows, metric)
    if not values:
        return None

    mean = statistics.fmean(values)
    stddev = statistics.stdev(values) if len(values) > 1 else 0.0
    degrees_of_freedom = len(values) - 1
    t_critical = T_CRITICAL_95.get(degrees_of_freedom, 1.96)
    ci95 = t_critical * stddev / math.sqrt(len(values)) if len(values) > 1 else 0.0

    return {
        "metric": metric,
        "n": str(len(values)),
        "mean": f"{mean:.9f}",
        "stddev": f"{stddev:.9f}",
        "ci95_half_width": f"{ci95:.9f}",
        "ci95_method": "student_t" if len(values) > 1 else "",
        "min": f"{min(values):.9f}",
        "max": f"{max(values):.9f}",
    }


def main() -> int:
    args = parse_args()
    input_path = pathlib.Path(args.input)
    output_path = pathlib.Path(args.output) if args.output else input_path.parent / "aggregate-summary.csv"

    with input_path.open(newline="", encoding="utf-8") as input_file:
        rows = list(csv.DictReader(input_file))

    grouped: dict[tuple[str, ...], list[dict[str, str]]] = defaultdict(list)
    for row in rows:
        grouped[group_key(row)].append(row)

    output_rows = []
    for key, group_rows in sorted(grouped.items()):
        base = dict(zip(GROUP_KEYS, key))
        for metric in METRICS:
            aggregate = aggregate_metric(group_rows, metric)
            if aggregate is None:
                continue
            output_rows.append(base | aggregate)

    output_path.parent.mkdir(parents=True, exist_ok=True)
    fieldnames = list(GROUP_KEYS) + [
        "metric",
        "n",
        "mean",
        "stddev",
        "ci95_half_width",
        "ci95_method",
        "min",
        "max",
    ]
    with output_path.open("w", newline="", encoding="utf-8") as output_file:
        writer = csv.DictWriter(output_file, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(output_rows)

    print(f"Aggregate CSV: {output_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
