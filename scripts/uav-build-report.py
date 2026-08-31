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


def sort_key(row: dict[str, str]) -> tuple[float, float, float]:
    """Rank rows by reliability, freshness, then latency."""

    pdr = float_value(row, "steady_delivery_ratio")
    unknown = float_value(row, "steady_unknown_aoi_share", default=1.0)
    latency = float_value(row, "steady_avg_latency_ms", default=1e9)
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
    table = [[
        "Architektur",
        "Steady PDR",
        "Steady unknown AoI",
        "Steady Avg AoI",
        "Steady Latenz",
        "Steady Hops",
        "Gesamt App-Bytes",
    ]]

    for row in sorted(rows, key=sort_key):
        table.append([
            architecture_name(row["architecture"]),
            display_float(row, "steady_delivery_ratio"),
            display_float(row, "steady_unknown_aoi_share"),
            display_float(row, "steady_avg_known_aoi_s", suffix=" s"),
            display_float(row, "steady_avg_latency_ms", suffix=" ms"),
            display_float(row, "steady_avg_hops"),
            row.get("app_bytes_sent") or "n/a",
        ])

    return markdown_table(table)


def best_label(rows: list[dict[str, str]], metric: str, higher_is_better: bool) -> str:
    valid = [row for row in rows if row.get(metric, "") != ""]
    if not valid:
        return "n/a"

    best = max(valid, key=lambda row: float_value(row, metric)) if higher_is_better else min(
        valid,
        key=lambda row: float_value(row, metric),
    )
    return architecture_name(best["architecture"])


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
