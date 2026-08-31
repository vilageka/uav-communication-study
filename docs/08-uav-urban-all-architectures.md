# Version 08: Urbane Szenarien fuer alle Architekturen

## Ziel

Diese Version uebertraegt die urbane Gebaeude- und Strassengeometrie aus
Version 07 auf alle bisher betrachteten Architekturklassen:

```text
scratch/uav-urban-wifi-aoi.cc
scratch/uav-urban-mesh-olsr-aoi.cc
scratch/uav-urban-lte-infrastructure-aoi.cc
```

Damit koennen Wi-Fi-Ad-hoc-Broadcast, OLSR-Mesh und LTE-Infrastruktur nicht nur
im freien Feld, sondern auch in einem gemeinsamen urbanen Benchmark verglichen
werden.

## Neue Dateien

`uav-urban-mesh-olsr-aoi.cc` basiert auf der OLSR-Mesh-Version. Die UAVs stehen
nun auf Strassenachsen zwischen Gebaeudebloecken und der Wi-Fi-Kanal nutzt
`HybridBuildingsPropagationLossModel`.

`uav-urban-lte-infrastructure-aoi.cc` basiert auf der LTE-Version. UAVs und
eNodeB werden in dieselbe Block-Stadt gesetzt. Die eNodeB steht auf einer
zentralen Strassenachse und standardmaessig knapp ueber Dachhoehe. Auch der
LTE-Helper nutzt `HybridBuildingsPropagationLossModel`.

Alle urbanen Dateien koennen die erzeugte Gebaeudegeometrie als CSV ausgeben.

## Szenariotypen

Das Experiment-Skript kennt nun zusaetzlich konkrete urbane Formen:

| Szenario | Gebaeude | Blockgroesse | Strassenbreite | Hoehe | Interpretation |
| --- | ---: | ---: | ---: | ---: | --- |
| `urban-open` | 3 x 3 | 70 m x 70 m | 60 m | 20 m | offene Stadtstruktur |
| `urban-baseline` | 3 x 3 | 80 m x 80 m | 40 m | 35 m | mittlerer Urban-Benchmark |
| `urban-canyon` | 4 x 4 | 90 m x 90 m | 25 m | 60 m | dichter Urban-Canyon |

Der normale Standardlauf nutzt weiterhin den mittleren Urban-Benchmark.

## Startbefehle

Alle urbanen Architekturen im Standardraster:

```bash
scripts/uav-run-experiments.py --profile standard \
  --only urban-wifi-adhoc \
  --only urban-olsr-mesh \
  --only urban-lte-infra \
  --results-dir results/uav-urban-all-v08
```

Vergleich der drei urbanen Formen bei 20 UAVs und 100 m Abstand:

```bash
scripts/uav-run-experiments.py --profile urban-forms \
  --only urban-wifi-adhoc \
  --only urban-olsr-mesh \
  --only urban-lte-infra \
  --results-dir results/uav-urban-forms-v08
```

## Ergebnisuebersicht: urbaner Standardbenchmark

| Architektur | UAVs | Abstand | PDR | Avg. Latenz | Max. Latenz | Avg. Hops | Unknown AoI |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Urban Wi-Fi | 5 | 60 m | 0.900 | 0.301 ms | 0.303 ms | n/a | 0.254 |
| Urban Wi-Fi | 5 | 100 m | 0.800 | 0.302 ms | 0.303 ms | n/a | 0.337 |
| Urban Wi-Fi | 20 | 60 m | 0.811 | 0.303 ms | 0.305 ms | n/a | 0.328 |
| Urban Wi-Fi | 20 | 100 m | 0.771 | 0.303 ms | 0.305 ms | n/a | 0.361 |
| Urban OLSR | 5 | 60 m | 1.000 | 2.839 ms | 27.283 ms | 1.100 | 0.433 |
| Urban OLSR | 5 | 100 m | 1.000 | 44.677 ms | 1020.460 ms | 1.300 | 0.454 |
| Urban OLSR | 20 | 60 m | 0.966 | 58.332 ms | 2012.060 ms | 1.333 | 0.479 |
| Urban OLSR | 20 | 100 m | 0.950 | 81.217 ms | 3001.800 ms | 1.358 | 0.488 |
| Urban LTE | 5 | 60 m | 1.000 | 14.700 ms | 18.000 ms | 2.000 | 0.150 |
| Urban LTE | 5 | 100 m | 1.000 | 14.700 ms | 18.000 ms | 2.000 | 0.150 |
| Urban LTE | 20 | 60 m | 1.000 | 14.550 ms | 33.000 ms | 2.000 | 0.164 |
| Urban LTE | 20 | 100 m | 1.000 | 14.550 ms | 33.000 ms | 2.000 | 0.164 |

## Ergebnisuebersicht: urbane Formen

Diese Matrix betrachtet 20 UAVs, 100 m Abstand und 6 s Traffic-Dauer.

| Architektur | Szenario | PDR | Avg. Latenz | Avg. Hops | Unknown AoI | Gebaeude |
| --- | --- | ---: | ---: | ---: | ---: | ---: |
| Urban Wi-Fi | urban-open | 0.729 | 0.304 ms | n/a | 0.396 | 9 |
| Urban Wi-Fi | urban-baseline | 0.771 | 0.303 ms | n/a | 0.361 | 9 |
| Urban Wi-Fi | urban-canyon | 0.732 | 0.307 ms | n/a | 0.394 | 16 |
| Urban OLSR | urban-open | 0.946 | 79.110 ms | 1.457 | 0.488 | 9 |
| Urban OLSR | urban-baseline | 0.950 | 81.217 ms | 1.358 | 0.488 | 9 |
| Urban OLSR | urban-canyon | 0.934 | 78.816 ms | 1.428 | 0.492 | 16 |
| Urban LTE | urban-open | 1.000 | 14.550 ms | 2.000 | 0.164 | 9 |
| Urban LTE | urban-baseline | 1.000 | 14.550 ms | 2.000 | 0.164 | 9 |
| Urban LTE | urban-canyon | 1.000 | 14.550 ms | 2.000 | 0.164 | 16 |

## Deutung

Urban Wi-Fi Broadcast bleibt bei erfolgreich empfangenen Paketen extrem schnell,
verliert aber viele Updates. Bei 20 UAVs und 100 m Abstand erreicht es nur noch
eine PDR von etwa 0.771. Das bestaetigt: Direkter Broadcast ist als einfache
Referenz nuetzlich, aber in urbanen Umgebungen empfindlich gegen Reichweiten-
und Abschattungseffekte.

Urban OLSR-Mesh schliesst einen Teil dieser Luecken. Im 20-UAV/100-m-Szenario
steigt die PDR von 0.771 bei Broadcast auf 0.950. Gleichzeitig steigt die
mittlere Latenz auf etwa 81 ms und einzelne Pakete koennen mehrere Sekunden
spaet ankommen. Die Hop-Metrik zeigt, dass die urbanen Szenarien tatsaechlich
Multi-Hop-Pfade erzeugen.

Urban LTE bleibt in diesen vereinfachten Ein-Zellen-Szenarien sehr stabil. Die
PDR liegt bei 1.0 und die mittlere Latenz bei etwa 14.55 ms. Das ist als
Infrastruktur-Baseline hilfreich, muss aber vorsichtig interpretiert werden:
Die aktuelle LTE-Version nutzt eine zentrale eNodeB, keine Zelluebergaben, keine
Interferenz durch Nachbarzellen und noch keine realistische Netzlast ausserhalb
der UAV-Anwendung.

## Wichtige Einschraenkungen

Die urbanen Formen sind derzeit synthetische Benchmark-Szenarien. Sie bilden
keine konkrete Stadtkarte ab. Fuer die Bachelorarbeit ist das aber ein Vorteil,
solange die Forschungsfrage Architekturvergleich heisst: Die Parameter sind
kontrollierbar und alle Architekturen sehen dieselbe Umgebung.

Die AoI-Werte enthalten weiterhin die Startphase. Besonders OLSR wird dadurch
benachteiligt, weil es mit `appStart=5` eine Routing-Konvergenzphase hat.
Spaeter sollte das Experiment-Skript zusaetzlich AoI nach Einschwingzeit
auswerten.

Der Kommunikationsaufwand enthaelt weiterhin vor allem Anwendungstraffic.
Routing- und LTE-Kontrolltraffic sind noch nicht vollstaendig in der
Vergleichsmetrik enthalten.

## Naechste Schritte

Als naechstes sollte die Auswertung um eine steady-state Sicht erweitert
werden. Danach koennen wir die urbanen Szenarien mit 40 UAVs und groesseren
Abstaenden laufen lassen und daraus Skalierungsdiagramme fuer die Arbeit
ableiten.
