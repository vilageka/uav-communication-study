# Version 06: Experimentmatrix fuer Architekturvergleiche

## Ziel

Diese Version fuehrt ein separates Experiment-Skript ein:

```text
scripts/uav-run-experiments.py
```

Die bisherigen Scratch-Programme simulieren jeweils eine konkrete
Kommunikationsarchitektur. Das neue Skript startet diese Programme systematisch
mit mehreren UAV-Anzahlen und Abstaenden. Dadurch entsteht ein einheitlicher
Versuchsaufbau, mit dem Wi-Fi Ad-hoc, OLSR-Mesh und LTE-Infrastruktur besser
vergleichbar werden.

## Warum ein separates Skript?

Fuer die Bachelorarbeit ist wichtig, dass Experimente nicht nur einmal manuell
gestartet werden, sondern nachvollziehbar wiederholbar sind. Das Skript
dokumentiert daher direkt im Code, welche Szenarien zum Standardlauf gehoeren
und welche Parameter fuer alle Architekturen gleich gehalten werden.

Der Standardlauf ist bewusst moderat gewaehlt:

- 5 UAVs als kleines Referenzszenario.
- 20 UAVs als erstes groesseres Schwarm-Szenario.
- 60 m Abstand als dichteres Szenario.
- 100 m Abstand als anspruchsvolleres Szenario.
- 6 s Traffic-Dauer.
- 1 s Update-Intervall.
- 0.2 s AoI-Sampling.

Der Full-Modus enthaelt zusaetzlich 10 und 40 UAVs sowie 160 m Abstand. Dieser
Modus ist fuer spaetere Skalierungsdiagramme gedacht, kann aber deutlich
laenger laufen.

## Startbefehle

Standardlauf:

```bash
scripts/uav-run-experiments.py --profile standard
```

Groesserer Lauf:

```bash
scripts/uav-run-experiments.py --profile full
```

Schneller Funktionstest:

```bash
scripts/uav-run-experiments.py --profile smoke
```

Der Lauf erzeugt einen Ergebnisordner unter `results/`. Darin liegen pro
Szenario die Rohdaten der Scratch-Programme sowie eine zusammengefuehrte
`summary.csv`.

## Ausgefuehrter Standardlauf

Fuer diese Version wurde folgender Lauf verwendet:

```bash
scripts/uav-run-experiments.py --profile standard --results-dir results/uav-experiments-standard-v06
```

Die generierten CSV-Dateien werden nicht ins Git-Repository aufgenommen, damit
das Repository klein bleibt. Die wichtigsten Ergebnisse sind hier festgehalten.

## Ergebnisuebersicht

| Architektur | UAVs | Abstand | PDR | Avg. Latenz | Max. Latenz | Avg. Hops | Unknown AoI | Avg. Known AoI |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Wi-Fi Ad-hoc | 5 | 60 m | 1.000 | 0.301 ms | 0.301 ms | n/a | 0.171 | 0.566 s |
| Wi-Fi Ad-hoc | 5 | 100 m | 1.000 | 0.301 ms | 0.303 ms | n/a | 0.171 | 0.566 s |
| Wi-Fi Ad-hoc | 20 | 60 m | 1.000 | 0.303 ms | 0.305 ms | n/a | 0.171 | 0.491 s |
| Wi-Fi Ad-hoc | 20 | 100 m | 0.895 | 0.303 ms | 0.305 ms | n/a | 0.259 | 0.491 s |
| OLSR-Mesh | 5 | 60 m | 1.000 | 18.112 ms | 1001.490 ms | 1.000 | 0.442 | 0.695 s |
| OLSR-Mesh | 5 | 100 m | 1.000 | 18.128 ms | 1001.490 ms | 1.000 | 0.442 | 0.695 s |
| OLSR-Mesh | 20 | 60 m | 1.000 | 58.096 ms | 2001.520 ms | 1.000 | 0.468 | 0.603 s |
| OLSR-Mesh | 20 | 100 m | 0.982 | 71.029 ms | 2986.700 ms | 1.094 | 0.478 | 0.611 s |
| LTE-Infrastruktur | 5 | 60 m | 1.000 | 14.700 ms | 18.000 ms | 2.000 | 0.150 | 0.694 s |
| LTE-Infrastruktur | 5 | 100 m | 1.000 | 14.700 ms | 18.000 ms | 2.000 | 0.150 | 0.694 s |
| LTE-Infrastruktur | 20 | 60 m | 1.000 | 14.550 ms | 33.000 ms | 2.000 | 0.164 | 0.601 s |
| LTE-Infrastruktur | 20 | 100 m | 1.000 | 14.550 ms | 33.000 ms | 2.000 | 0.164 | 0.601 s |

## Deutung

Wi-Fi Ad-hoc Broadcast ist in diesen Szenarien mit Abstand am schnellsten. Die
mittlere Latenz liegt bei etwa 0.3 ms. Das liegt daran, dass jedes UAV ein Paket
einmal per Broadcast sendet und direkt erreichbare Nachbarn es ohne Routing
empfangen. Der Nachteil wird beim 20-UAV-Szenario mit 100 m Abstand sichtbar:
Die PDR sinkt auf etwa 0.895, weil nicht alle UAV-Paare direkt erreichbar sind.

OLSR-Mesh verbessert die Reichweite gegenueber reinem Broadcast. Beim
20-UAV-Szenario mit 100 m Abstand erreicht OLSR eine PDR von etwa 0.982 und
nutzt im Mittel mehr als einen Hop. Gleichzeitig steigen Latenz und
Kommunikationsaufwand deutlich. Das ist plausibel, weil jedes UAV
Positionsupdates an jedes andere UAV per Unicast sendet und Routing zusaetzliche
Protokollaktivitaet erzeugt. Die sehr hohen Maximallatenzen zeigen ausserdem,
dass einzelne Pakete waehrend Routenaufbau oder Routenanpassung deutlich spaeter
ankommen koennen.

LTE liefert in dieser vereinfachten Ein-Zellen-Infrastruktur fuer 5 und 20 UAVs
eine PDR von 1.0. Die mittlere Latenz liegt stabil bei etwa 14.5 bis 14.7 ms und
ist damit hoeher als direkter Wi-Fi-Broadcast, aber niedriger und stabiler als
OLSR in den groesseren Szenarien. Die Hop-Schaetzung ist konstant 2, weil die
Kommunikation ueber Infrastruktur modelliert wird: UAV zur eNodeB/EPC-Seite und
danach zur Ziel-UAV-Seite.

## Wichtige Einschraenkungen

Die aktuelle AoI-Auswertung enthaelt auch die Startphase. Das verzerrt besonders
OLSR, weil dort bewusst `appStart=5` verwendet wird, damit Routing vorher
konvergieren kann. Fuer die finale Arbeit sollte zusaetzlich eine AoI-Auswertung
nach der Einschwingphase ergaenzt werden.

Der Kommunikationsaufwand ist noch nicht vollstaendig vergleichbar. OLSR und LTE
geben Anwendungslast in Bytes aus, Wi-Fi Ad-hoc derzeit nur Paketanzahl und
Empfangszahl. Ausserdem fehlen bei OLSR noch die Routing-Kontrollpakete und bei
LTE die Kontrollsignalisierung. Die aktuelle Metrik ist daher eher
Anwendungsaufwand als vollstaendiger Kommunikationsaufwand.

## Naechste Schritte

Der naechste sinnvolle Schritt ist, die Auswertung nach der Einschwingphase in
das Experiment-Skript aufzunehmen. Danach kann der Kommunikationsaufwand
praeziser werden, indem nicht nur Anwendungspakete, sondern auch Kontrolltraffic
beruecksichtigt wird.
