# Version 07: Urbanes Wi-Fi-Ad-hoc-Szenario mit Gebaeuden

## Ziel

Diese Version beginnt mit konkreteren Szenarien fuer die Bachelorarbeit. Bisher
standen die UAVs in freien Gitter-Szenarien. Das ist gut fuer Basisvergleiche,
aber zu idealisiert fuer urbane Umgebungen. In dieser Version wird daher eine
erste parametrisierbare Block-Stadt eingefuehrt.

Die neue Scratch-Datei heisst:

```text
scratch/uav-urban-wifi-aoi.cc
```

Sie basiert auf `uav-wifi-aoi.cc`, behaelt also die Wi-Fi-Ad-hoc-Broadcast-
Kommunikation und die bisherigen Metriken bei. Neu ist vor allem das
Szenariomodell mit Gebaeuden und Strassenkorridoren.

## Modellannahmen

Das Szenario besteht aus rechteckigen Gebaeudebloecken und orthogonalen
Strassen. UAVs werden deterministisch auf Strassenachsen positioniert. Dadurch
bleiben die Experimente reproduzierbar, waehrend Gebaeudehoehe,
Strassenbreite, Blockgroesse, UAV-Anzahl und UAV-Abstand variiert werden
koennen.

Der Funkkanal nutzt:

```text
ns3::HybridBuildingsPropagationLossModel
```

Das ist kein Raytracing-Modell und keine exakte Nachbildung einer konkreten
Stadt. Es ist ein bewusst einfacher Urban-Benchmark: Die Simulation kennt
Gebaeude, Rooftop-Level und urbanen Pfadverlust. Damit koennen wir testen, wie
stark die bisher sehr gute Wi-Fi-Ad-hoc-Referenz leidet, wenn die Umgebung
weniger ideal ist.

## Wichtige Parameter

```text
--numUavs              Anzahl der UAVs
--spacing              Abstand zwischen UAV-Positionen entlang der Strassen
--altitude             UAV-Flughoehe
--blocksX              Anzahl Gebaeudebloecke in x-Richtung
--blocksY              Anzahl Gebaeudebloecke in y-Richtung
--buildingLengthX      Gebaeudelaenge in x-Richtung
--buildingLengthY      Gebaeudelaenge in y-Richtung
--streetWidth          Strassenbreite
--buildingHeight       Gebaeudehoehe
--frequency            Frequenz fuer das urbane Ausbreitungsmodell
--buildingMetricsFile  CSV-Datei mit den erzeugten Gebaeudebloecken
```

## Startbefehle

Ein einzelner Lauf:

```bash
./ns3 run "uav-urban-wifi-aoi --numUavs=20 --spacing=100 --simTime=6"
```

Urbaner Standardlauf ueber das Experiment-Skript:

```bash
scripts/uav-run-experiments.py --profile standard --only urban-wifi-adhoc --results-dir results/uav-urban-wifi-v07
```

## CSV-Dateien

Die Datei fuer empfangene Updates nutzt dasselbe Format wie `uav-wifi-aoi.cc`:

```text
receive_time_s,send_time_s,sender_id,receiver_id,sequence,latency_ms,x_m,y_m,z_m
```

Die AoI-Datei nutzt ebenfalls das bekannte Format:

```text
time_s,receiver_id,sender_id,known,aoi_s
```

Neu ist die Buildings-Datei:

```text
building_id,x_min_m,x_max_m,y_min_m,y_max_m,z_min_m,z_max_m
```

Damit ist dokumentiert, welche Gebaeudegeometrie fuer einen Lauf erzeugt
wurde.

## Ausgefuehrter Lauf

Fuer diese Version wurde folgende Urban-Matrix ausgefuehrt:

```bash
scripts/uav-run-experiments.py --profile standard --only urban-wifi-adhoc --results-dir results/uav-urban-wifi-v07
```

Der Standardaufbau nutzt 9 Gebaeude:

```text
3 x 3 Gebaeudebloecke
80 m x 80 m Grundflaeche pro Gebaeude
40 m Strassenbreite
35 m Gebaeudehoehe
80 m UAV-Flughoehe
```

## Ergebnisuebersicht

| Szenario | PDR | Avg. Latenz | Max. Latenz | Unknown AoI | Avg. Known AoI |
| --- | ---: | ---: | ---: | ---: | ---: |
| 5 UAVs, 60 m | 0.900 | 0.301 ms | 0.303 ms | 0.254 | 0.567 s |
| 5 UAVs, 100 m | 0.800 | 0.302 ms | 0.303 ms | 0.337 | 0.565 s |
| 20 UAVs, 60 m | 0.811 | 0.303 ms | 0.305 ms | 0.328 | 0.491 s |
| 20 UAVs, 100 m | 0.771 | 0.303 ms | 0.305 ms | 0.361 | 0.492 s |

## Vergleich mit freiem Wi-Fi-Ad-hoc-Szenario

Im freien Wi-Fi-Ad-hoc-Szenario aus Version 06 ergaben sich:

| Szenario | Freies Wi-Fi PDR | Urbanes Wi-Fi PDR |
| --- | ---: | ---: |
| 5 UAVs, 60 m | 1.000 | 0.900 |
| 5 UAVs, 100 m | 1.000 | 0.800 |
| 20 UAVs, 60 m | 1.000 | 0.811 |
| 20 UAVs, 100 m | 0.895 | 0.771 |

Die Latenz der erfolgreich empfangenen Pakete bleibt fast unveraendert bei etwa
0.3 ms. Das ist typisch fuer Broadcast-Auswertung: Wenn ein Paket direkt
ankommt, kommt es sehr schnell an. Die schlechtere Umgebung zeigt sich nicht
primaer in hoeherer Latenz, sondern in weniger empfangenen Paketen und in mehr
unbekannten AoI-Zustaenden.

## Deutung

Das urbane Szenario bestaetigt eine wichtige These fuer die Arbeit:
Direkter Broadcast ist nur dann stark, wenn die Funknachbarschaft ausreichend
dicht und wenig gestoert ist. Sobald Gebaeudebloecke und groessere Abstaende
hinzukommen, sinkt die Packet Delivery Ratio deutlich. Bei 20 UAVs und 100 m
Abstand faellt sie von etwa 0.895 im freien Szenario auf etwa 0.771 im urbanen
Szenario.

Fuer Age of Information bedeutet das: Nicht empfangene Updates erzeugen keine
hohe Paketlatenz, sondern fehlende oder veraltete Information. Deshalb steigt
der Anteil unbekannter AoI-Zustaende im urbanen 20-UAV/100-m-Szenario auf etwa
0.361. Diese Metrik macht sichtbar, was reine Latenz nicht zeigt.

## Bedeutung fuer die naechsten Architekturen

Die urbane Wi-Fi-Ad-hoc-Version ist als Referenz gedacht. Als naechstes sollten
wir dieselbe Gebaeude- und Positionslogik auf OLSR-Mesh uebertragen. Dann kann
geprueft werden, ob Multi-Hop-Routing die urbanen Empfangsluecken schliessen
kann und welchen Preis das bei Latenz, Hop-Anzahl und Kommunikationsaufwand hat.
