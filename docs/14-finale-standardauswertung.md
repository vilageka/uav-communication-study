# Version 14: Standardauswertung mit Mehrfachlaeufen

Diese Version enthaelt den ersten echten Ergebnislauf nach der methodischen
Stabilisierung aus Version 13. Im Unterschied zum Smoke-Test wurden hier alle
Standard-Szenariopunkte mit mehreren RNG-Wiederholungen simuliert.

## Versuchsaufbau

Ausgefuehrter Befehl:

```bash
python3 scripts/uav-run-experiments.py --profile standard --runs 5 --sim-time 30 --aoi-sample-interval 0.2 --results-dir results/uav-final-standard-v14 --timeout 900
```

Die Matrix besteht aus:

- 6 Architekturen:
  - Wi-Fi Ad-hoc Broadcast
  - OLSR-Mesh
  - LTE-Infrastruktur
  - Urban Wi-Fi Ad-hoc Broadcast
  - Urban OLSR-Mesh
  - Urban LTE-Infrastruktur
- 2 UAV-Zahlen: 5 und 20
- 2 Abstaende: 60 m und 100 m
- 5 RNG-Wiederholungen je Szenariopunkt

Insgesamt wurden damit 120 ns-3-Einzelruns ausgefuehrt.

Die wichtigsten Ergebnisdateien sind:

```text
results/uav-final-standard-v14/summary.csv
results/uav-final-standard-v14/steady-state-summary.csv
results/uav-final-standard-v14/aggregate-summary.csv
doc/uav-communication-study/14-finale-standardauswertung-report.md
```

## Aggregierte Kernergebnisse

Die folgenden Werte sind Mittelwert +/- Standardabweichung ueber 5
Wiederholungen. Ausgewertet wird der Steady-State-Bereich.

### 5 UAVs, 60 m

| Architektur | PDR | Unknown AoI | Avg AoI | Latenz | Hops | App-Bytes sent |
| --- | --- | --- | --- | --- | --- | --- |
| Wi-Fi | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.581 s | 0.302 ms | n/a | 2545 |
| OLSR | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.592 s | 0.316 ms | 1.000 | 12360 |
| LTE | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.592 s | 14.700 ms | 2.000 | 12280 |
| Urban Wi-Fi | 0.940 +/- 0.055 | 0.060 +/- 0.055 | 0.582 s | 0.303 ms | n/a | 2755 |
| Urban OLSR | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.592 s | 0.402 ms | 1.080 | 13200 |
| Urban LTE | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.592 s | 14.700 ms | 2.000 | 13120 |

### 5 UAVs, 100 m

| Architektur | PDR | Unknown AoI | Avg AoI | Latenz | Hops | App-Bytes sent |
| --- | --- | --- | --- | --- | --- | --- |
| Wi-Fi | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.581 s | 0.302 ms | n/a | 2665 |
| OLSR | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.592 s | 0.317 ms | 1.000 | 12840 |
| LTE | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.592 s | 14.700 ms | 2.000 | 12760 |
| Urban Wi-Fi | 0.830 +/- 0.027 | 0.170 +/- 0.027 | 0.581 s | 0.303 ms | n/a | 2785 |
| Urban OLSR | 1.000 +/- 0.000 | 0.000 +/- 0.001 | 0.593 s | 0.654 ms | 1.300 | 13320 |
| Urban LTE | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.592 s | 14.700 ms | 2.000 | 13240 |

### 20 UAVs, 60 m

| Architektur | PDR | Unknown AoI | Avg AoI | Latenz | Hops | App-Bytes sent |
| --- | --- | --- | --- | --- | --- | --- |
| Wi-Fi | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.506 s | 0.304 ms | n/a | 11590 |
| OLSR | 1.000 +/- 0.000 | 0.001 +/- 0.000 | 0.503 s | 0.371 ms | 1.000 | 262680 |
| LTE | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.516 s | 14.550 ms | 2.000 | 261160 |
| Urban Wi-Fi | 0.826 +/- 0.027 | 0.174 +/- 0.027 | 0.506 s | 0.304 ms | n/a | 12010 |
| Urban OLSR | 0.994 +/- 0.005 | 0.004 +/- 0.004 | 0.516 s | 6.912 ms | 1.353 | 270660 |
| Urban LTE | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.516 s | 14.550 ms | 2.000 | 269140 |

### 20 UAVs, 100 m

| Architektur | PDR | Unknown AoI | Avg AoI | Latenz | Hops | App-Bytes sent |
| --- | --- | --- | --- | --- | --- | --- |
| Wi-Fi | 0.895 +/- 0.000 | 0.105 +/- 0.000 | 0.506 s | 0.304 ms | n/a | 11860 |
| OLSR | 0.994 +/- 0.007 | 0.003 +/- 0.002 | 0.515 s | 3.279 ms | 1.133 | 267810 |
| LTE | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.516 s | 14.550 ms | 2.000 | 266290 |
| Urban Wi-Fi | 0.784 +/- 0.030 | 0.216 +/- 0.030 | 0.507 s | 0.304 ms | n/a | 12040 |
| Urban OLSR | 0.991 +/- 0.007 | 0.006 +/- 0.005 | 0.528 s | 11.540 ms | 1.405 | 271230 |
| Urban LTE | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.516 s | 14.550 ms | 2.000 | 269710 |

## Deutung

Wi-Fi-Broadcast ist in freien, kleinen und dichten Szenarien sehr stark:
PDR 1.0, sehr niedrige Latenz um 0.30 ms und deutlich weniger Anwendungslast.
Das passt zur ADS-L-inspirierten Referenzidee: wenige, direkte Broadcasts sind
effizient, solange alle relevanten Empfaenger direkt erreicht werden.

Der Nachteil wird sichtbar, sobald Gebaeude oder groessere Abstaende ins Spiel
kommen. Urban-Wi-Fi erreicht bei 20 UAVs und 100 m nur noch etwa 0.784 PDR und
hat 0.216 unknown AoI. Das bedeutet: Ein relevanter Anteil der
Sender-Empfaenger-Zustaende besitzt im Auswertungsfenster keine aktuelle
Information.

OLSR-Mesh reduziert diese Luecken deutlich. Bei 20 UAVs und 100 m steigt die
PDR von 0.895 bei Wi-Fi-Broadcast auf 0.994 im freien Mesh und von 0.784 bei
Urban-Wi-Fi auf 0.991 bei Urban-OLSR. Der Preis dafuer ist klar: Die
Anwendungslast steigt massiv, weil jedes UAV nicht nur ein Broadcast-Paket,
sondern ein Unicast-Paket pro Ziel-UAV sendet.

LTE liefert in diesen Szenarien durchgehend PDR 1.0 und unknown AoI 0.0. Das
ist die Staerke der Infrastrukturarchitektur: Die Erreichbarkeit haengt weniger
von direkter UAV-Nachbarschaft ab. Die Latenz ist aber mit etwa 14.55 ms bis
14.70 ms deutlich hoeher als bei direktem Wi-Fi oder gutem OLSR-Mesh.

Die Hop-Zahlen bestaetigen die Modellannahmen. OLSR liegt im Freifeld oft bei
1 Hop, in urbanen Szenarien steigt der Mittelwert auf etwa 1.08 bis 1.405. LTE
bleibt konstant bei 2 Infrastruktur-Hops; diese Zahl darf nicht als Mesh-Hop
interpretiert werden.

## Zwischenfazit

Fuer die Bachelorarbeit ergibt sich aus diesem Lauf eine klare qualitative
Rangfolge:

- Beste Effizienz und niedrigste Latenz bei direkter Erreichbarkeit:
  Wi-Fi-Broadcast.
- Bester Kompromiss bei schwierigerer Topologie:
  OLSR-Mesh, besonders wenn PDR/AoI wichtiger sind als Kommunikationsaufwand.
- Beste Zuverlaessigkeit in diesem vereinfachten Infrastrukturmodell:
  LTE, aber mit Infrastrukturabhaengigkeit und hoeherer Latenz.

Fuer die finale Thesis-Auswertung sollten diese Standardergebnisse noch durch
gezielte Zusatzszenarien ergaenzt werden, z.B. urbane Formen wie Open,
Baseline und Canyon sowie groessere Schwarmgroessen.
