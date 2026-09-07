# Freie Skalierungserweiterung mit 40 UAVs und 200 m

Dieses Dokument beschreibt den Ergebnislauf `v21`. Der Lauf erweitert die
freie Standardauswertung aus `v20` um groessere Schwarm- und
Abstandsparameter. Ziel ist es, die Skalierbarkeit der drei
Kommunikationsarchitekturen in einem freien Gitter ohne Gebaeudemodell
genauer zu untersuchen.

## Motivation

Die freien Standardfaelle aus `v20` zeigten bei 5 und 20 UAVs sowie 60 m und
100 m Gitterabstand nach der 2.4-GHz-Friis-Korrektur kaum Unterschiede in der
Zuverlaessigkeit: alle Architekturen erreichten dort praktisch vollstaendige
Zustellung. Fuer einen belastbaren Vergleich reicht das allein nicht aus,
weil die Architekturen erst unter groesserer Netzdichte oder groesserer
Distanz unterschiedlich belastet werden.

Deshalb wurden zwei Erweiterungen ergaenzt:

- `40` UAVs als zusaetzliche Schwarmgroesse,
- `200 m` als zusaetzlicher Gitterabstand.

Im freien Szenario beschreibt `spacing` weiterhin den Abstand benachbarter
Gitterpunkte. Diese Interpretation gilt nur fuer die freien Gitterfaelle; in
urbanen Szenarien ist `spacing` dagegen das Platzierungsintervall entlang der
Strassenkorridore.

## Implementierung

Die Experimentsteuerung wurde in `scripts/uav-run-experiments.py` um das
Profil `free-scale-extension` erweitert. Dieses Profil erzeugt bewusst keine
vollstaendige Kreuzproduktmatrix aus allen bisherigen und neuen Werten,
sondern nur die zusaetzlich relevanten Skalierungspunkte:

```text
5 UAVs, 200 m
20 UAVs, 200 m
40 UAVs, 60 m
40 UAVs, 100 m
40 UAVs, 200 m
```

Damit bleibt der Lauf methodisch anschlussfaehig an `v20`, ohne bereits
vorhandene Standardfaelle erneut zu rechnen. Fuer jede Kombination wurden
die drei Architekturen mit jeweils fuenf RNG-Wiederholungen simuliert:

```text
3 Architekturen x 5 Szenariopunkte x 5 rngRun = 75 Runs
```

Zusaetzlich wurde das Experimentskript um `--resume-existing` erweitert. Bei
langen Laufserien kann ein abgebrochener Ergebnisordner damit fortgesetzt
werden. Vollstaendige vorhandene Logdateien werden erneut geparst und in die
neue `summary.csv` uebernommen; fehlende oder unvollstaendige Runs werden
neu ausgefuehrt. Diese Funktion wurde fuer `v21` genutzt, weil der erste
Durchlauf bereits 68 von 75 Einzellaeufen erzeugt hatte.

## Versuchsaufbau

Alle Laeufe nutzen dieselben Grundparameter wie die freie Standardauswertung:

- Simulationsdauer: 30 s,
- Updateintervall: 1 s,
- AoI-Abtastintervall: 0.2 s,
- fuenf RNG-Wiederholungen je Konfiguration,
- freie Ausbreitung mit Friis-Pfadverlust,
- Wi-Fi-basierte Architekturen mit expliziter 2.4-GHz-Parametrisierung.

Die drei verglichenen Architekturen sind:

- Wi-Fi-Ad-hoc-Broadcast als vereinfachte ADS-L-inspirierte Referenz,
- Wi-Fi/802.11 OLSR-Mesh als mehrstufiger Ansatz,
- LTE/EPC-Infrastruktur als zentrale Referenzarchitektur.

## Ergebnisse

Die folgende Tabelle zeigt Mittelwert +/- Standardabweichung ueber fuenf
Wiederholungen. Ausgewertet wird der eingeschwungene Zustand, damit
Start- und Konvergenzphasen die Werte weniger verzerren.

| UAVs | Abstand | Architektur | PDR | Unknown AoI | Bekannter AoI [s] | Latenz [ms] | Hops | App-Bytes |
| ---: | ---: | --- | ---: | ---: | ---: | ---: | ---: | ---: |
| 5 | 200 m | Wi-Fi Ad-hoc | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.581 | 0.303 | n/a | 2665 |
| 5 | 200 m | OLSR-Mesh | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.592 | 0.318 | 1.000 | 12840 |
| 5 | 200 m | LTE | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.592 | 14.700 | 2.000 | 12760 |
| 20 | 200 m | Wi-Fi Ad-hoc | 0.895 +/- 0.000 | 0.105 +/- 0.000 | 0.506 | 0.305 | n/a | 11860 |
| 20 | 200 m | OLSR-Mesh | 0.997 +/- 0.002 | 0.002 +/- 0.002 | 0.506 | 2.210 | 1.122 | 267810 |
| 20 | 200 m | LTE | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.516 | 14.550 | 2.000 | 266290 |
| 40 | 60 m | Wi-Fi Ad-hoc | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.505 | 0.304 | n/a | 23990 |
| 40 | 60 m | OLSR-Mesh | 0.821 +/- 0.036 | 0.060 +/- 0.004 | 1.010 | 215.953 | 1.101 | 1119765 |
| 40 | 60 m | LTE | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.515 | 15.205 | 2.000 | 1113525 |
| 40 | 100 m | Wi-Fi Ad-hoc | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.505 | 0.305 | n/a | 24380 |
| 40 | 100 m | OLSR-Mesh | 0.834 +/- 0.035 | 0.060 +/- 0.004 | 0.970 | 207.262 | 1.088 | 1134975 |
| 40 | 100 m | LTE | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.515 | 15.205 | 2.000 | 1128735 |
| 40 | 200 m | Wi-Fi Ad-hoc | 0.614 +/- 0.000 | 0.386 +/- 0.000 | 0.503 | 0.306 | n/a | 24830 |
| 40 | 200 m | OLSR-Mesh | 0.388 +/- 0.004 | 0.171 +/- 0.008 | 3.611 | 327.225 | 1.675 | 1152525 |
| 40 | 200 m | LTE | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.515 | 15.205 | 2.000 | 1146285 |

## Deutung

Bei 5 UAVs und 200 m bleiben alle Architekturen vollstaendig verbunden. Die
freie Reichweite reicht hier aus, um den kleinen Schwarm auch bei groesserem
Gitterabstand ohne erkennbare Paketverluste abzudecken. Die Unterschiede
liegen deshalb vor allem in Latenz und Aufwand: Wi-Fi-Ad-hoc ist mit rund
0.3 ms am schnellsten und erzeugt die geringste Anwendungslast. OLSR und LTE
erzeugen wegen der paarweisen Kommunikation deutlich mehr Anwendungsbytes.
LTE zeigt zudem die typische konstante Infrastrukturlatenz von rund 14.7 ms.

Bei 20 UAVs und 200 m wird der Unterschied zwischen direkter Kommunikation
und vermittelter Architektur sichtbar. Wi-Fi-Ad-hoc erreicht nur noch eine
PDR von 0.895 und besitzt einen unknown-AoI-Anteil von 0.105. Das bedeutet,
dass ein Teil der UAV-Paare bei direktem Broadcast keine aktuelle Information
uebereinander besitzt. OLSR kann diese Luecke fast vollstaendig schliessen:
die PDR steigt auf 0.997 und der unknown-AoI-Anteil sinkt auf 0.002. Der Preis
dabei ist eine hoehere Latenz von rund 2.21 ms und eine deutlich hoehere
Anwendungslast. LTE erreicht weiterhin vollstaendige Zustellung, bleibt aber
bei der hoeheren Infrastrukturlatenz.

Bei 40 UAVs zeigt sich ein zweiter Effekt: Nicht nur Reichweite, sondern auch
Netzlast und Skalierung werden relevant. Wi-Fi-Ad-hoc bleibt bei 60 m und
100 m stabil und liefert alle Pakete im eingeschwungenen Zustand aus. OLSR
faellt in diesen dichten 40-UAV-Szenarien dagegen auf eine PDR von etwa
0.82 bis 0.83 zurueck und zeigt Latenzen oberhalb von 200 ms. Die Hop-Zahl
liegt zwar nur knapp ueber 1, trotzdem erzeugt die paarweise Kommunikation
zwischen allen UAVs eine hohe Last. Das deutet darauf hin, dass der
Mehrhop-Ansatz in dieser Konfiguration weniger durch fehlende Reichweite als
durch Protokoll- und Lastverhalten begrenzt wird.

Der Fall 40 UAVs und 200 m ist der schaerfste freie Skalierungstest. Direkter
Wi-Fi-Broadcast faellt auf eine PDR von 0.614 und einen unknown-AoI-Anteil
von 0.386. Viele UAV-Paare sind also nicht direkt erreichbar. OLSR sollte
theoretisch solche Luecken ueber Mehrhop-Pfade schliessen, erreicht in diesem
konkreten Lauf aber nur eine PDR von 0.388. Gleichzeitig steigen bekannte AoI
und Latenz stark an. Die mittlere Hop-Zahl von 1.675 zeigt, dass tatsaechlich
mehr Mehrhop-Kommunikation genutzt wird, diese aber unter der Kombination aus
groesserer Distanz, 40 Knoten und hoher paarweiser Last nicht stabil genug
ist. LTE bleibt in diesem freien Ein-Zellen-Szenario dagegen vollstaendig
zuverlaessig.

## Bedeutung fuer den Architekturvergleich

Die Erweiterung veraendert die Aussage der freien Szenarien deutlich. In
`v20` sahen die freien Gitterfaelle noch so aus, als seien alle drei
Architekturen hinsichtlich Zuverlaessigkeit weitgehend gleichwertig. Mit
200 m Abstand und 40 UAVs entstehen nun klare Trennlinien:

- Wi-Fi-Ad-hoc ist bei kleinen und mittleren freien Topologien sehr schnell
  und effizient, verliert aber bei grossem Abstand direkte Erreichbarkeit.
- OLSR-Mesh verbessert bei 20 UAVs und 200 m die Erreichbarkeit deutlich,
  skaliert bei 40 UAVs in der aktuellen Vollvermaschungs-Last aber schlecht.
- LTE ist in den freien Ein-Zellen-Szenarien am robustesten gegen Schwarm-
  und Abstandsskalierung, besitzt aber eine konstant hoehere Latenz und setzt
  Infrastruktur voraus.

Fuer die Bachelorarbeit ergibt sich daraus eine wichtige methodische
Konsequenz: Die freien Szenarien sollten nicht nur als einfache Baseline mit
kleinen Abstaenden betrachtet werden. Erst die Kombination aus groesserer
Topologie und 200 m Gitterabstand macht sichtbar, ob eine Architektur direkte
Nachbarschaft voraussetzt, Mehrhop-Pfade nutzen kann oder auf eine zentrale
Infrastruktur angewiesen ist.

## Einordnung und Grenzen

Die Ergebnisse gelten fuer das implementierte freie Gittermodell und die
aktuelle Anwendungslast. Besonders bei OLSR ist zu beachten, dass alle
UAV-Paare periodisch Informationen austauschen. Dadurch waechst die
Anwendungslast stark mit der Schwarmgroesse. Die ausgewiesenen App-Bytes
enthalten Anwendungspakete, aber noch nicht den vollstaendigen MAC- und
Routing-Kontrollaufwand. Fuer eine noch feinere Skalierbarkeitsanalyse sollte
der Kommunikationsaufwand spaeter um technologie- und protokollspezifische
Kontrollpakete erweitert werden.

Trotz dieser Einschraenkung ist `v21` als Vergleichsbaustein sinnvoll: Der
Lauf zeigt, dass die Architekturunterschiede im freien Raum nicht bei 60 m
oder 100 m, sondern erst bei groesserer Topologie und groesserem Abstand
deutlich hervortreten.
