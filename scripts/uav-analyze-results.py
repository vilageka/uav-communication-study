#!/usr/bin/env python3
"""
Post-process UAV experiment results.

The ns-3 scratch programs already print a complete-run summary.  For the
thesis, however, a second view is important: steady-state behaviour after the
initial discovery/attachment/routing phase.  This script reads an existing
experiment directory and writes an additional CSV file with metrics that start
after a configurable warmup period.

The script does not rerun ns-3.  It only reads summary.csv plus the generated
*_updates.csv and *_aoi.csv files.  That makes it useful for documenting and
rechecking already completed experiment runs.
"""

from __future__ import annotations

import argparse
import csv
import pathlib
from dataclasses import dataclass


BROADCAST_ARCHITECTURES = {"wifi-adhoc", "urban-wifi-adhoc"}


@dataclass(frozen=True)
class AnalysisWindow:
    """Time interval used for steady-state analysis."""

    start_s: float
    end_s: float


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Analyze UAV experiment CSV results.")
    parser.add_argument(
        "results_dir",
        help="Experiment directory containing summary.csv and raw CSV files.",
    )
    parser.add_argument(
        "--output",
        default=None,
        help="Output CSV path. Defaults to <results_dir>/steady-state-summary.csv.",
    )
    parser.add_argument(
        "--warmup-intervals",
        type=float,
        default=1.5,
        help="Number of update intervals added after traffic start for steady-state start.",
    )
    return parser.parse_args()


def as_float(value: str, default: float = 0.0) -> float:
    if value == "":
        return default
    return float(value)


def as_int(value: str, default: int = 0) -> int:
    if value == "":
        return default
    return int(float(value))


def traffic_start_for(row: dict[str, str]) -> float:
    """Return the first planned application send time for an architecture."""

    if row.get("app_start_s"):
        return float(row["app_start_s"])

    # The broadcast scratch programs start the first sender at t=1.0 s.  This
    # value is not printed as app_start_s because there is no routing attach
    # phase, so the analyzer records it explicitly here.
    if row["architecture"] in BROADCAST_ARCHITECTURES:
        return 1.0

    return 0.0


def analysis_window(row: dict[str, str], warmup_intervals: float) -> AnalysisWindow:
    traffic_start_s = traffic_start_for(row)
    update_interval_s = as_float(row["update_interval_s"])
    sim_time_s = as_float(row["sim_time_s"])
    start_s = traffic_start_s + warmup_intervals * update_interval_s
    end_s = traffic_start_s + sim_time_s
    return AnalysisWindow(start_s=start_s, end_s=end_s)


def planned_send_times(row: dict[str, str]) -> list[float]:
    """Reconstruct deterministic send times from the scratch-program schedule."""

    architecture = row["architecture"]
    num_uavs = as_int(row["num_uavs"])
    sim_time_s = as_float(row["sim_time_s"])
    update_interval_s = as_float(row["update_interval_s"])
    updates_per_sender = int(sim_time_s // update_interval_s)
    traffic_start_s = traffic_start_for(row)
    times: list[float] = []

    if architecture in BROADCAST_ARCHITECTURES:
        for sender_id in range(num_uavs):
            first_send_s = traffic_start_s + sender_id * 0.01
            for update_index in range(updates_per_sender):
                times.append(first_send_s + update_index * update_interval_s)
    else:
        for sender_id in range(num_uavs):
            for receiver_id in range(num_uavs):
                if sender_id == receiver_id:
                    continue
                first_send_s = traffic_start_s + (sender_id * num_uavs + receiver_id) * 0.001
                for update_index in range(updates_per_sender):
                    times.append(first_send_s + update_index * update_interval_s)

    return times


def expected_receives_in_window(row: dict[str, str], window: AnalysisWindow) -> int:
    """Calculate how many packets should be received in the steady-state window."""

    num_uavs = as_int(row["num_uavs"])
    multiplier = num_uavs - 1 if row["architecture"] in BROADCAST_ARCHITECTURES else 1
    sends = [
        send_time_s
        for send_time_s in planned_send_times(row)
        if window.start_s <= send_time_s <= window.end_s
    ]
    return len(sends) * multiplier


def analyze_updates(row: dict[str, str], window: AnalysisWindow) -> dict[str, str]:
    """Analyze received update packets that belong to the steady-state window."""

    update_path = pathlib.Path(row["updates_csv"])
    if not update_path.exists():
        return {}

    received = 0
    latency_sum = 0.0
    latency_min: float | None = None
    latency_max: float | None = None
    payload_bytes_received = 0
    hop_sum = 0.0
    hop_min: float | None = None
    hop_max: float | None = None
    hop_samples = 0

    with update_path.open(newline="", encoding="utf-8") as input_file:
        reader = csv.DictReader(input_file)
        for update in reader:
            send_time_s = as_float(update["send_time_s"])
            receive_time_s = as_float(update["receive_time_s"])
            if send_time_s < window.start_s or receive_time_s < window.start_s:
                continue
            if send_time_s > window.end_s:
                continue

            latency_ms = as_float(update["latency_ms"])
            received += 1
            latency_sum += latency_ms
            latency_min = latency_ms if latency_min is None else min(latency_min, latency_ms)
            latency_max = latency_ms if latency_max is None else max(latency_max, latency_ms)

            if "payload_bytes" in update and update["payload_bytes"]:
                payload_bytes_received += as_int(update["payload_bytes"])

            if "hop_count" in update and update["hop_count"]:
                hop_count = as_float(update["hop_count"])
                if hop_count > 0:
                    hop_samples += 1
                    hop_sum += hop_count
                    hop_min = hop_count if hop_min is None else min(hop_min, hop_count)
                    hop_max = hop_count if hop_max is None else max(hop_max, hop_count)

    expected = expected_receives_in_window(row, window)
    delivery_ratio = received / expected if expected else 0.0
    avg_latency = latency_sum / received if received else 0.0
    avg_hops = hop_sum / hop_samples if hop_samples else 0.0

    return {
        "steady_expected_received_packets": str(expected),
        "steady_received_packets": str(received),
        "steady_delivery_ratio": f"{delivery_ratio:.6f}",
        "steady_avg_latency_ms": f"{avg_latency:.6f}",
        "steady_min_latency_ms": "" if latency_min is None else f"{latency_min:.6f}",
        "steady_max_latency_ms": "" if latency_max is None else f"{latency_max:.6f}",
        "steady_app_bytes_received": str(payload_bytes_received),
        "steady_avg_hops": "" if hop_samples == 0 else f"{avg_hops:.6f}",
        "steady_min_hops": "" if hop_min is None else f"{hop_min:.6f}",
        "steady_max_hops": "" if hop_max is None else f"{hop_max:.6f}",
    }


def analyze_aoi(row: dict[str, str], window: AnalysisWindow) -> dict[str, str]:
    """Analyze AoI samples inside the steady-state window."""

    aoi_path = pathlib.Path(row["aoi_csv"])
    if not aoi_path.exists():
        return {}

    known_samples = 0
    unknown_samples = 0
    aoi_sum = 0.0
    aoi_max = 0.0

    with aoi_path.open(newline="", encoding="utf-8") as input_file:
        reader = csv.DictReader(input_file)
        for sample in reader:
            time_s = as_float(sample["time_s"])
            if time_s < window.start_s or time_s > window.end_s:
                continue

            if sample["known"] == "1":
                aoi_s = as_float(sample["aoi_s"])
                known_samples += 1
                aoi_sum += aoi_s
                aoi_max = max(aoi_max, aoi_s)
            else:
                unknown_samples += 1

    total_samples = known_samples + unknown_samples
    unknown_share = unknown_samples / total_samples if total_samples else 0.0
    avg_known_aoi = aoi_sum / known_samples if known_samples else 0.0

    return {
        "steady_known_aoi_samples": str(known_samples),
        "steady_unknown_aoi_samples": str(unknown_samples),
        "steady_unknown_aoi_share": f"{unknown_share:.6f}",
        "steady_avg_known_aoi_s": f"{avg_known_aoi:.6f}",
        "steady_max_known_aoi_s": f"{aoi_max:.6f}",
    }


def analyze_row(row: dict[str, str], warmup_intervals: float) -> dict[str, str]:
    window = analysis_window(row, warmup_intervals)
    analyzed = dict(row)
    analyzed["steady_start_s"] = f"{window.start_s:.6f}"
    analyzed["steady_end_s"] = f"{window.end_s:.6f}"
    analyzed.update(analyze_updates(row, window))
    analyzed.update(analyze_aoi(row, window))
    return analyzed


def main() -> int:
    args = parse_args()
    results_dir = pathlib.Path(args.results_dir).resolve()
    summary_csv = results_dir / "summary.csv"
    output_csv = pathlib.Path(args.output).resolve() if args.output else results_dir / "steady-state-summary.csv"

    if not summary_csv.exists():
        raise SystemExit(f"Missing summary CSV: {summary_csv}")

    with summary_csv.open(newline="", encoding="utf-8") as input_file:
        rows = list(csv.DictReader(input_file))
        original_fieldnames = list(rows[0].keys()) if rows else []

    analyzed_rows = [analyze_row(row, args.warmup_intervals) for row in rows]

    added_fieldnames = [
        "steady_start_s",
        "steady_end_s",
        "steady_expected_received_packets",
        "steady_received_packets",
        "steady_delivery_ratio",
        "steady_avg_latency_ms",
        "steady_min_latency_ms",
        "steady_max_latency_ms",
        "steady_app_bytes_received",
        "steady_avg_hops",
        "steady_min_hops",
        "steady_max_hops",
        "steady_known_aoi_samples",
        "steady_unknown_aoi_samples",
        "steady_unknown_aoi_share",
        "steady_avg_known_aoi_s",
        "steady_max_known_aoi_s",
    ]
    fieldnames = original_fieldnames + [name for name in added_fieldnames if name not in original_fieldnames]

    with output_csv.open("w", newline="", encoding="utf-8") as output_file:
        writer = csv.DictWriter(output_file, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(analyzed_rows)

    print(f"Steady-state summary: {output_csv}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
