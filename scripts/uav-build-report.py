#!/usr/bin/env python3
"""
Build a compact Markdown comparison report from UAV steady-state summaries.

This script is deliberately small and transparent.  It does not try to hide the
evaluation behind a complicated score.  Instead, it groups comparable rows and
prints the key metrics that are relevant for the thesis: reliability,
freshness, latency, hops, and application-level effort.
"""

from __future__ import annotations

import argparse
import csv
import pathlib
import statistics
from collections import defaultdict
from dataclasses import dataclass


@dataclass(frozen=True)
class ReportSource:
    """One input CSV and a human-readable title for the report section."""

    title: str
    path: pathlib.Path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Build a UAV comparison Markdown report.")
    parser.add_argument(
        "--source",
        action="append",
        nargs=2,
        metavar=("TITLE", "CSV"),
        required=True,
        help="Report section title and steady-state-summary.csv path.",
    )
    parser.add_argument(
        "--output",
        required=True,
        help="Markdown output file.",
    )
    return parser.parse_args()


def read_rows(path: pathlib.Path) -> list[dict[str, str]]:
    with path.open(newline="", encoding="utf-8") as input_file:
        return list(csv.DictReader(input_file))


def float_value(row: dict[str, str], key: str, default: float = 0.0) -> float:
    value = row.get(key, "")
    if value == "":
        return default
    return float(value)


def display_float(row: dict[str, str], key: str, digits: int = 3, suffix: str = "") -> str:
    value = row.get(key, "")
    if value == "":
        return "n/a"
    return f"{float(value):.{digits}f}{suffix}"


def metric_values(rows: list[dict[str, str]], key: str) -> list[float]:
    """Return all numeric values for one metric across repeated RNG runs."""

    values = []
    for row in rows:
        value = row.get(key, "")
        if value != "":
            values.append(float(value))
    return values


def display_metric_mean(rows: list[dict[str, str]], key: str, digits: int = 3, suffix: str = "") -> str:
    """Display a metric as mean, and as mean +/- stddev when repetitions exist."""

    values = metric_values(rows, key)
    if not values:
        return "n/a"

    mean = statistics.fmean(values)
    if len(values) == 1:
        return f"{mean:.{digits}f}{suffix}"

    stddev = statistics.stdev(values)
    return f"{mean:.{digits}f} +/- {stddev:.{digits}f}{suffix}"


def architecture_name(key: str) -> str:
    names = {
        "wifi-adhoc": "Wi-Fi",
        "olsr-mesh": "OLSR",
        "lte-infra": "LTE",
        "urban-wifi-adhoc": "Urban Wi-Fi",
        "urban-olsr-mesh": "Urban OLSR",
        "urban-lte-infra": "Urban LTE",
    }
    return names.get(key, key)


def comparable_key(row: dict[str, str]) -> tuple[str, str, str]:
    scenario = row.get("scenario") or "grid"
    return (scenario, row.get("num_uavs", ""), row.get("spacing_m", ""))


def sort_key(rows: list[dict[str, str]]) -> tuple[float, float, float]:
    """Rank rows by reliability, freshness, then latency."""

    pdr_values = metric_values(rows, "steady_delivery_ratio")
    unknown_values = metric_values(rows, "steady_unknown_aoi_share")
    latency_values = metric_values(rows, "steady_avg_latency_ms")

    pdr = statistics.fmean(pdr_values) if pdr_values else 0.0
    unknown = statistics.fmean(unknown_values) if unknown_values else 1.0
    latency = statistics.fmean(latency_values) if latency_values else 1e9
    return (-pdr, unknown, latency)


def markdown_table(rows: list[list[str]]) -> list[str]:
    if not rows:
        return []

    header = rows[0]
    separator = ["---"] * len(header)
    lines = [
        "| " + " | ".join(header) + " |",
        "| " + " | ".join(separator) + " |",
    ]
    for row in rows[1:]:
        lines.append("| " + " | ".join(row) + " |")
    return lines


def build_metric_table(rows: list[dict[str, str]]) -> list[str]:
    architecture_rows: dict[str, list[dict[str, str]]] = defaultdict(list)
    for row in rows:
        architecture_rows[row["architecture"]].append(row)

    table = [[
        "Architektur",
        "Runs",
        "Steady PDR",
        "Steady unknown AoI",
        "Steady Avg AoI",
        "Steady Latenz",
        "Steady Hops",
        "Gesamt App-Bytes",
    ]]

    for architecture, arch_rows in sorted(architecture_rows.items(), key=lambda item: sort_key(item[1])):
        table.append([
            architecture_name(architecture),
            str(len(arch_rows)),
            display_metric_mean(arch_rows, "steady_delivery_ratio"),
            display_metric_mean(arch_rows, "steady_unknown_aoi_share"),
            display_metric_mean(arch_rows, "steady_avg_known_aoi_s", suffix=" s"),
            display_metric_mean(arch_rows, "steady_avg_latency_ms", suffix=" ms"),
            display_metric_mean(arch_rows, "steady_avg_hops"),
            display_metric_mean(arch_rows, "app_bytes_sent", digits=0),
        ])

    return markdown_table(table)


def best_label(rows: list[dict[str, str]], metric: str, higher_is_better: bool) -> str:
    architecture_rows: dict[str, list[dict[str, str]]] = defaultdict(list)
    for row in rows:
        architecture_rows[row["architecture"]].append(row)

    means = []
    for architecture, arch_rows in architecture_rows.items():
        values = metric_values(arch_rows, metric)
        if values:
            means.append((architecture, statistics.fmean(values)))

    if not means:
        return "n/a"

    best = max(means, key=lambda item: item[1]) if higher_is_better else min(means, key=lambda item: item[1])
    return architecture_name(best[0])


def build_group_summary(rows: list[dict[str, str]]) -> list[str]:
    reliability = best_label(rows, "steady_delivery_ratio", higher_is_better=True)
    freshness = best_label(rows, "steady_unknown_aoi_share", higher_is_better=False)
    latency = best_label(rows, "steady_avg_latency_ms", higher_is_better=False)

    return [
        f"- Beste PDR: {reliability}",
        f"- Wenigste unbekannte AoI-Zustaende: {freshness}",
        f"- Niedrigste Latenz empfangener Pakete: {latency}",
    ]


def build_section(source: ReportSource, rows: list[dict[str, str]]) -> list[str]:
    grouped: dict[tuple[str, str, str], list[dict[str, str]]] = defaultdict(list)
    for row in rows:
        grouped[comparable_key(row)].append(row)

    lines = [
        f"## {source.title}",
        "",
        f"Quelle: `{source.path}`",
        "",
    ]

    for key in sorted(grouped.keys(), key=lambda item: (item[0], int(item[1]), int(item[2]))):
        scenario, num_uavs, spacing_m = key
        rows_for_key = grouped[key]
        lines.extend(
            [
                f"### {scenario}, {num_uavs} UAVs, {spacing_m} m",
                "",
            ]
        )
        lines.extend(build_group_summary(rows_for_key))
        lines.append("")
        lines.extend(build_metric_table(rows_for_key))
        lines.append("")

    return lines


def build_report(sources: list[ReportSource]) -> str:
    lines = [
        "# UAV Steady-state Vergleichsreport",
        "",
        "Dieser Report wird aus `steady-state-summary.csv` Dateien erzeugt.",
        "Er nutzt die eingeschwungene Auswertung, damit Start- und",
        "Konvergenzphasen den Architekturvergleich weniger verzerren.",
        "",
        "Die Rangfolge innerhalb eines Szenarios ist bewusst einfach:",
        "zuerst hohe Packet Delivery Ratio, dann wenig unknown AoI, dann",
        "niedrige Latenz fuer erfolgreich empfangene Pakete.",
        "Wenn mehrere `rngRun`-Wiederholungen vorhanden sind, zeigt die",
        "Tabelle Mittelwert +/- Standardabweichung.",
        "",
    ]

    for source in sources:
        lines.extend(build_section(source, read_rows(source.path)))

    lines.extend(
        [
            "## Hinweise zur Interpretation",
            "",
            "- Niedrige Latenz allein reicht nicht aus, weil verlorene Pakete keine Latenz haben.",
            "- Unknown AoI zeigt, ob ein UAV ueber andere UAVs gar keine aktuelle Information besitzt.",
            "- App-Bytes sind nur Anwendungslast. Kontrolltraffic ist noch nicht vollstaendig enthalten.",
            "- LTE-Hops sind Infrastruktur-Hops und nicht direkt mit OLSR-Mesh-Hops gleichzusetzen.",
            "",
        ]
    )
    return "\n".join(lines)


def main() -> int:
    args = parse_args()
    sources = [
        ReportSource(title=title, path=pathlib.Path(path))
        for title, path in args.source
    ]
    output = pathlib.Path(args.output).resolve()
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(build_report(sources), encoding="utf-8")
    print(f"Report written: {output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
