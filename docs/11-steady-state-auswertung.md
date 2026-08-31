# Version 11: Steady-state Auswertung

## Ziel

Diese Version verbessert die Auswertung, ohne die eigentlichen ns-3
Simulationen zu veraendern. Bisher wurden AoI, Latenz und PDR ueber den
gesamten Simulationszeitraum zusammengefasst. Das ist fuer einen ersten Blick
nuetzlich, aber methodisch nicht ideal.

Der Grund: Einige Architekturen haben eine Einschwingphase. Besonders OLSR
benoetigt Zeit, um Routinginformationen aufzubauen. LTE benoetigt eine
Attach-Phase. Wi-Fi Broadcast startet ebenfalls erst nach einer kurzen
Anlaufzeit. Wenn diese Anfangsphase voll in die AoI-Auswertung eingeht, werden
unknown-AoI-Zustaende stark mitgezaehlt, obwohl sie nicht unbedingt den
eingeschwungenen Betrieb beschreiben.

## Neue Datei

```text
scripts/uav-analyze-results.py
```

Das Skript liest einen vorhandenen Ergebnisordner mit `summary.csv`,
`*_updates.csv` und `*_aoi.csv`. Es startet keine Simulation neu, sondern
erzeugt eine zusaetzliche Datei:

```text
steady-state-summary.csv
```

## Analysefenster

Standardmaessig beginnt die steady-state Auswertung bei:

```text
trafficStart + 1.5 * updateInterval
```

Bei den bisherigen Standardlaeufen bedeutet das:

| Architektur | trafficStart | updateInterval | steady-state Start |
| --- | ---: | ---: | ---: |
| Wi-Fi Broadcast | 1.0 s | 1.0 s | 2.5 s |
| OLSR-Mesh | 5.0 s | 1.0 s | 6.5 s |
| LTE | 1.0 s | 1.0 s | 2.5 s |

Die 1.5 Update-Intervalle sind ein pragmatischer Kompromiss: Die erste
Positionsrunde ist nicht mehr im Fokus, aber das Analysefenster bleibt bei den
kurzen Testlaeufen noch gross genug.

## Startbefehle

Nachtraegliche Auswertung eines vorhandenen Ordners:

```bash
scripts/uav-analyze-results.py results/uav-urban-all-v08
```

Neue Experimentlaeufe erzeugen die steady-state CSV automatisch:

```bash
scripts/uav-run-experiments.py --profile standard
```

Bei Bedarf kann die automatische steady-state Auswertung abgeschaltet werden:

```bash
scripts/uav-run-experiments.py --profile standard --skip-steady-state
```

## Berechnete steady-state Metriken

Das Analyse-Skript ergaenzt unter anderem:

```text
steady_start_s
steady_end_s
steady_expected_received_packets
steady_received_packets
steady_delivery_ratio
steady_avg_latency_ms
steady_min_latency_ms
steady_max_latency_ms
steady_avg_hops
steady_unknown_aoi_share
steady_avg_known_aoi_s
steady_max_known_aoi_s
```

Die Metriken werden aus den Roh-CSV-Dateien berechnet. Fuer erwartete Pakete
rekonstruiert das Skript die deterministischen Sendezeitpunkte der jeweiligen
Scratch-Programme.

## Ergebnis: freier Standardlauf

Quelle:

```text
results/uav-experiments-standard-v06/steady-state-summary.csv
```

| Architektur | UAVs | Abstand | Gesamt-PDR | Steady-PDR | Gesamt-Latenz | Steady-Latenz | Gesamt unknown AoI | Steady unknown AoI |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Wi-Fi | 5 | 60 m | 1.000 | 1.000 | 0.301 ms | 0.301 ms | 0.171 | 0.000 |
| Wi-Fi | 5 | 100 m | 1.000 | 1.000 | 0.301 ms | 0.301 ms | 0.171 | 0.000 |
| Wi-Fi | 20 | 60 m | 1.000 | 1.000 | 0.303 ms | 0.303 ms | 0.171 | 0.000 |
| Wi-Fi | 20 | 100 m | 0.895 | 0.895 | 0.303 ms | 0.303 ms | 0.259 | 0.105 |
| OLSR | 5 | 60 m | 1.000 | 1.000 | 18.112 ms | 0.312 ms | 0.442 | 0.000 |
| OLSR | 5 | 100 m | 1.000 | 1.000 | 18.128 ms | 0.312 ms | 0.442 | 0.000 |
| OLSR | 20 | 60 m | 1.000 | 1.000 | 58.096 ms | 0.483 ms | 0.468 | 0.004 |
| OLSR | 20 | 100 m | 0.982 | 0.994 | 71.029 ms | 1.963 ms | 0.478 | 0.015 |
| LTE | 5 | 60 m | 1.000 | 1.000 | 14.700 ms | 14.700 ms | 0.150 | 0.000 |
| LTE | 5 | 100 m | 1.000 | 1.000 | 14.700 ms | 14.700 ms | 0.150 | 0.000 |
| LTE | 20 | 60 m | 1.000 | 1.000 | 14.550 ms | 14.550 ms | 0.164 | 0.000 |
| LTE | 20 | 100 m | 1.000 | 1.000 | 14.550 ms | 14.550 ms | 0.164 | 0.000 |

## Ergebnis: urbaner Standardbenchmark

Quelle:

```text
results/uav-urban-all-v08/steady-state-summary.csv
```

| Architektur | UAVs | Abstand | Gesamt-PDR | Steady-PDR | Gesamt-Latenz | Steady-Latenz | Gesamt unknown AoI | Steady unknown AoI |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Urban Wi-Fi | 5 | 60 m | 0.900 | 0.900 | 0.301 ms | 0.301 ms | 0.254 | 0.100 |
| Urban Wi-Fi | 5 | 100 m | 0.800 | 0.800 | 0.302 ms | 0.302 ms | 0.337 | 0.200 |
| Urban Wi-Fi | 20 | 60 m | 0.811 | 0.811 | 0.303 ms | 0.303 ms | 0.328 | 0.189 |
| Urban Wi-Fi | 20 | 100 m | 0.771 | 0.771 | 0.303 ms | 0.303 ms | 0.361 | 0.229 |
| Urban OLSR | 5 | 60 m | 1.000 | 1.000 | 2.839 ms | 0.423 ms | 0.433 | 0.000 |
| Urban OLSR | 5 | 100 m | 1.000 | 1.000 | 44.677 ms | 0.683 ms | 0.454 | 0.000 |
| Urban OLSR | 20 | 60 m | 0.966 | 0.997 | 58.332 ms | 3.547 ms | 0.479 | 0.011 |
| Urban OLSR | 20 | 100 m | 0.950 | 0.989 | 81.217 ms | 9.115 ms | 0.488 | 0.024 |
| Urban LTE | 5 | 60 m | 1.000 | 1.000 | 14.700 ms | 14.700 ms | 0.150 | 0.000 |
| Urban LTE | 5 | 100 m | 1.000 | 1.000 | 14.700 ms | 14.700 ms | 0.150 | 0.000 |
| Urban LTE | 20 | 60 m | 1.000 | 1.000 | 14.550 ms | 14.550 ms | 0.164 | 0.000 |
| Urban LTE | 20 | 100 m | 1.000 | 1.000 | 14.550 ms | 14.550 ms | 0.164 | 0.000 |

## Ergebnis: urbane Formen

Quelle:

```text
results/uav-urban-forms-v08/steady-state-summary.csv
```

Alle Werte beziehen sich auf 20 UAVs und 100 m Abstand.

| Architektur | Szenario | Gesamt-PDR | Steady-PDR | Gesamt-Latenz | Steady-Latenz | Steady unknown AoI | Steady Hops |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Urban Wi-Fi | urban-open | 0.729 | 0.729 | 0.304 ms | 0.304 ms | 0.271 | n/a |
| Urban Wi-Fi | urban-baseline | 0.771 | 0.771 | 0.303 ms | 0.303 ms | 0.229 | n/a |
| Urban Wi-Fi | urban-canyon | 0.732 | 0.732 | 0.307 ms | 0.307 ms | 0.268 | n/a |
| Urban OLSR | urban-open | 0.946 | 0.981 | 79.110 ms | 16.131 ms | 0.031 | 1.482 |
| Urban OLSR | urban-baseline | 0.950 | 0.989 | 81.217 ms | 9.115 ms | 0.024 | 1.378 |
| Urban OLSR | urban-canyon | 0.934 | 0.970 | 78.816 ms | 20.369 ms | 0.038 | 1.448 |
| Urban LTE | urban-open | 1.000 | 1.000 | 14.550 ms | 14.550 ms | 0.000 | 2.000 |
| Urban LTE | urban-baseline | 1.000 | 1.000 | 14.550 ms | 14.550 ms | 0.000 | 2.000 |
| Urban LTE | urban-canyon | 1.000 | 1.000 | 14.550 ms | 14.550 ms | 0.000 | 2.000 |

## Deutung

Die steady-state Auswertung veraendert vor allem die Interpretation von OLSR.
In der Gesamtbetrachtung wirken OLSR-Szenarien teilweise sehr langsam, weil
einzelne Pakete in der Anfangsphase stark verzoegert werden. Nach der
Einschwingzeit sinkt die mittlere Latenz deutlich. Im urbanen 20-UAV/100-m-Fall
faellt sie von etwa 81 ms auf etwa 9 ms.

Auch die unknown-AoI-Werte werden methodisch klarer. LTE hat im steady state in
den bisherigen Szenarien keine unbekannten AoI-Zustaende mehr. OLSR liegt im
urbanen 20-UAV/100-m-Fall bei etwa 0.024 unknown AoI share. Urban Wi-Fi bleibt
dagegen bei etwa 0.229, weil direkte Broadcast-Reichweite auch im
eingeschwungenen Zustand nicht alle UAV-Paare erreicht.

Damit wird der Architekturvergleich schaerfer:

- Wi-Fi Broadcast ist schnell, aber nicht immer vollstaendig.
- OLSR verbessert Erreichbarkeit deutlich, braucht aber Routing und hat mehr
  Latenz als direkter Broadcast.
- LTE ist im aktuellen Modell stabil, sollte aber spaeter mit realistischeren
  Last- und Interferenzannahmen getestet werden.

## Bedeutung fuer die Arbeit

Fuer die Bachelorarbeit sollten Gesamtwerte und steady-state Werte getrennt
diskutiert werden. Die Gesamtwerte zeigen Einschwingverhalten und erste
Informationsverfuegbarkeit. Die steady-state Werte beschreiben den laufenden
Betrieb nach der Anfangsphase.

Diese Trennung ist besonders wichtig, weil AoI sonst zu stark von Startzeiten
und Routing-Konvergenz beeinflusst wird.
