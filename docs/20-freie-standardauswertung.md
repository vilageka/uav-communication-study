# Freie Standardauswertung nach Friis-Frequenzkorrektur

Dieses Dokument beschreibt den Ergebnislauf `v20`. Der Lauf ersetzt die
freien Standardergebnisse aus `v14`, weil die freien Wi-Fi-basierten
Friis-Szenarien nach Version 19 explizit auf 2.4 GHz korrigiert wurden.
Die alten freien `v14`-Zahlen sind daher nicht mehr die finale Grundlage fuer
die Bachelorarbeit.

## Versuchsaufbau

Verglichen werden die drei freien Kommunikationsarchitekturen:

- Wi-Fi-Ad-hoc-Broadcast,
- Wi-Fi/802.11 OLSR-Mesh,
- LTE/EPC-Infrastruktur.

Die Matrix umfasst zwei Schwarmgroessen, zwei Gitterabstaende und fuenf
Wiederholungen je Konfiguration:

```text
3 Architekturen x 2 UAV-Anzahlen x 2 Gitterabstaende x 5 rngRun = 60 Runs
```

Die Simulationsdauer betraegt 30 s. Das Updateintervall liegt bei 1 s, das
AoI-Abtastintervall bei 0.2 s. Die Wi-Fi-basierten freien Programme verwenden
802.11b mit `DsssRate11Mbps` und setzen das `FriisPropagationLossModel`
explizit auf 2.4 GHz.

## Ergebnisse

### 5 UAVs

| Abstand | Architektur | PDR | Unknown AoI | Bekannter AoI [s] | Latenz [ms] | Hops | App-Bytes |
| ---: | --- | ---: | ---: | ---: | ---: | ---: | ---: |
| 60 m | Wi-Fi Ad-hoc | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.581 | 0.302 | n/a | 2545 |
| 60 m | OLSR-Mesh | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.592 | 0.316 | 1.000 | 12360 |
| 60 m | LTE | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.592 | 14.700 | 2.000 | 12280 |
| 100 m | Wi-Fi Ad-hoc | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.581 | 0.302 | n/a | 2665 |
| 100 m | OLSR-Mesh | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.592 | 0.317 | 1.000 | 12840 |
| 100 m | LTE | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.592 | 14.700 | 2.000 | 12760 |

### 20 UAVs

| Abstand | Architektur | PDR | Unknown AoI | Bekannter AoI [s] | Latenz [ms] | Hops | App-Bytes |
| ---: | --- | ---: | ---: | ---: | ---: | ---: | ---: |
| 60 m | Wi-Fi Ad-hoc | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.506 | 0.304 | n/a | 11590 |
| 60 m | OLSR-Mesh | 1.000 +/- 0.000 | 0.001 +/- 0.000 | 0.503 | 0.366 | 1.000 | 262680 |
| 60 m | LTE | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.516 | 14.550 | 2.000 | 261160 |
| 100 m | Wi-Fi Ad-hoc | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.506 | 0.304 | n/a | 11860 |
| 100 m | OLSR-Mesh | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.503 | 0.461 | 1.002 | 267810 |
| 100 m | LTE | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.516 | 14.550 | 2.000 | 266290 |

## Deutung

Die korrigierten freien Ergebnisse zeigen, dass die untersuchten freien
Gitter-Szenarien fuer 2.4 GHz nicht mehr reichweitenlimitiert sind. Alle drei
Architekturen erreichen in allen betrachteten Konfigurationen eine PDR von
1.0. Auch der unbekannte AoI-Anteil ist praktisch null.

Damit veraendert sich die Interpretation gegenueber `v14` deutlich. Die alte
Auswertung zeigte im freien 20-UAV/100-m-Fall noch eine reduzierte
Wi-Fi-Zustellrate. Diese Reduktion war jedoch nicht belastbar, weil das
Friis-Modell implizit mit 5.15 GHz statt mit 2.4 GHz lief. Nach der Korrektur
zeigt Wi-Fi-Ad-hoc in den freien 60-m- und 100-m-Szenarien vollstaendige
direkte Erreichbarkeit.

Der Architekturunterschied liegt im freien Szenario deshalb weniger in der
Zuverlaessigkeit als in Latenz, Aufwand und Abhaengigkeiten. Wi-Fi-Ad-hoc ist
am leichtgewichtigsten und erreicht die geringste Latenz von etwa 0.3 ms.
OLSR erreicht ebenfalls vollstaendige Zustellung, verursacht aber deutlich
mehr Anwendungstraffic, weil jedes Sender-Empfaenger-Paar adressiert wird.
Die Hop-Zahl bleibt im freien 60-m- und 100-m-Szenario fast immer bei 1,
was bedeutet, dass das Mesh in diesen Konfigurationen kaum echte
Mehrhop-Vorteile ausspielen muss.

LTE erreicht ebenfalls vollstaendige Zustellung, zeigt aber eine deutlich
hoehere und sehr konstante Latenz von etwa 14.55 ms bei 20 UAVs und 14.70 ms
bei 5 UAVs. Diese Latenz ist die infrastrukturseitige Referenz des
Ein-Zellen-Modells und nicht direkt mit der reinen direkten Wi-Fi-Latenz
gleichzusetzen.

## Konsequenz fuer den Gesamtvergleich

Fuer die Bachelorarbeit sollte `v20` als freie Standardbasis verwendet
werden. Die freien Szenarien zeigen nun:

- Bei 2.4 GHz und den betrachteten Abstaenden sind alle Architekturen
  hinsichtlich PDR und unknown AoI praktisch vollstaendig.
- Direkter Wi-Fi-Broadcast ist in freier Umgebung die effizienteste und
  schnellste Variante.
- OLSR bietet in diesen freien Szenarien noch keinen deutlichen
  Erreichbarkeitsvorteil, erzeugt aber mehr Aufwand.
- LTE liefert eine robuste infrastrukturbasierte Referenz mit hoeherer
  konstanter Latenz.

Die eigentliche Trennschaerfe zwischen den Architekturen entsteht damit nicht
im freien 60-m- oder 100-m-Gitter, sondern in anspruchsvolleren urbanen
Szenarien, bei niedriger relativer Flughoehe oder bei spaerlicheren
Topologien.
