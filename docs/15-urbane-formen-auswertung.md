# Urbane Formen: Auswertung und Deutung des korrigierten Ergebnislaufs

Dieses Dokument fasst den korrigierten urbanen Ergebnislauf `v16` zusammen.
Der Lauf ersetzt die vorherige urbane Auswertung `v15`, weil dort ein
methodischer Fehler in der Parametrisierung sichtbar wurde: Die urbanen
Wi-Fi- und OLSR-Simulationen verwendeten weiterhin 802.11b mit
`DsssRate11Mbps`, waehrend das urbane Pfadverlustmodell mit 5 GHz
parametrisiert war. In `v16` wurde die Frequenz fuer diese beiden
Architekturen auf 2.4 GHz gesetzt. Damit sind Wi-Fi-Standard, Datenmodus und
Ausbreitungsmodell konsistent.

## Untersuchungsziel

Der Lauf untersucht drei Kommunikationsarchitekturen in drei synthetischen
urbanen Formen:

- Wi-Fi Ad-hoc Broadcast als vereinfachte ADS-L-inspirierte Referenz.
- Wi-Fi/802.11 mit OLSR als mehrstufiger Mesh-Ansatz.
- LTE/EPC als infrastrukturbasierte Referenzarchitektur.

Die drei urbanen Formen sind keine drei weiteren Architekturen, sondern
Szenariovarianten. Damit werden in diesem Abschnitt drei Architekturen unter
unterschiedlichen urbanen Ausbreitungsbedingungen verglichen.

## Methodische Einordnung der urbanen Umgebung

Die in ns-3 erzeugten Gebaeude duerfen nicht als Raytracing-Hindernisse
interpretiert werden. Das verwendete `HybridBuildingsPropagationLossModel`
berechnet den Pfadverlust anhand parametrisierter Ausbreitungsmodelle und
Gebaeudeeigenschaften. Es prueft fuer Outdoor-zu-Outdoor-Verbindungen nicht
geometrisch, ob ein einzelner Gebaeudeblock die direkte Sichtlinie zwischen
zwei UAVs schneidet.

Die korrekte Interpretation lautet daher: Das Szenario bildet urbane
Ausbreitungsbedingungen mithilfe eines parametrisierten Pfadverlustmodells und
einer synthetischen Stadtgeometrie ab. Es bildet keine konkrete
Gebaeudeabschattung im Sinne eines Raytracing- oder Sichtlinienmodells ab.

Zusaetzlich ist zu beachten, dass die UAVs in diesem Lauf auf 80 m Hoehe
positioniert sind. Die Gebaeudehoehen betragen 20 m fuer `urban-open`, 35 m
fuer `urban-baseline` und 60 m fuer `urban-canyon`. Die UAVs befinden sich
damit in allen drei Profilen oberhalb der Daecher. Der erwartete
Stadtschluchteneffekt wird dadurch abgeschwaecht. Die Ergebnisse eignen sich
deshalb fuer den Vergleich urban parametrisierter Ausbreitungsbedingungen,
nicht aber fuer eine starke Aussage ueber blockierte Strassenschluchten.

## Versuchsaufbau

Fuer jede Kombination aus Architektur und urbanem Profil wurden fuenf
unabhaengige Wiederholungen mit unterschiedlichem `rngRun` ausgefuehrt. Die
Simulationsdauer betraegt 30 s. Die Anwendung startet nach der jeweiligen
architekturspezifischen Einschwingphase, und die Auswertung verwendet den
Steady-State-Anteil der Messungen.

Der Parameter `spacing` ist in den urbanen Szenarien kein allgemeiner Abstand
zwischen allen UAVs. Er beschreibt das Platzierungsintervall entlang der
synthetisch erzeugten Strassenkorridore. Deshalb werden zusaetzlich reale
Abstandsmetriken der erzeugten UAV-Positionen ausgegeben:

- minimaler Paarabstand,
- mittlerer Paarabstand,
- mittlerer Abstand zum naechsten Nachbarn,
- maximaler Paarabstand.

Die Aggregation berichtet Mittelwert und Standardabweichung. Die
95-Prozent-Intervalle in der CSV werden fuer die fuenf Wiederholungen mit
einem Student-t-Faktor berechnet. Aufgrund der kleinen Stichprobe sollten die
Intervalle trotzdem als Orientierung und nicht als endgueltiger statistischer
Nachweis verstanden werden.

## Ergebnisse

### Urban Open

| Architektur | PDR | Unbekannter AoI-Anteil | Bekannter AoI [s] | Latenz [ms] | Hops | Gesendete App-Bytes |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Wi-Fi Ad-hoc | 0.914 +/- 0.013 | 0.086 +/- 0.013 | 0.506 +/- 0.000 | 0.305 +/- 0.000 | n/a | 12130 |
| OLSR-Mesh | 0.999 +/- 0.001 | 0.002 +/- 0.001 | 0.506 +/- 0.002 | 1.928 +/- 1.142 | 1.197 +/- 0.054 | 272940 |
| LTE-Infrastruktur | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.516 +/- 0.000 | 14.550 +/- 0.000 | 2.000 +/- 0.000 | 271420 |

Positionsmetriken: minimaler Paarabstand 85.0 m, mittlerer Paarabstand
262.5 m, mittlerer Abstand zum naechsten Nachbarn 92.5 m und maximaler
Paarabstand 538.1 m.

### Urban Baseline

| Architektur | PDR | Unbekannter AoI-Anteil | Bekannter AoI [s] | Latenz [ms] | Hops | Gesendete App-Bytes |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Wi-Fi Ad-hoc | 0.935 +/- 0.015 | 0.065 +/- 0.015 | 0.507 +/- 0.001 | 0.304 +/- 0.000 | n/a | 12040 |
| OLSR-Mesh | 0.998 +/- 0.003 | 0.001 +/- 0.000 | 0.507 +/- 0.006 | 1.932 +/- 1.584 | 1.150 +/- 0.020 | 271230 |
| LTE-Infrastruktur | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.516 +/- 0.000 | 14.550 +/- 0.000 | 2.000 +/- 0.000 | 269710 |

Positionsmetriken: minimaler Paarabstand 30.0 m, mittlerer Paarabstand
225.6 m, mittlerer Abstand zum naechsten Nachbarn 70.5 m und maximaler
Paarabstand 453.4 m.

### Urban Canyon

| Architektur | PDR | Unbekannter AoI-Anteil | Bekannter AoI [s] | Latenz [ms] | Hops | Gesendete App-Bytes |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Wi-Fi Ad-hoc | 0.918 +/- 0.015 | 0.082 +/- 0.015 | 0.506 +/- 0.000 | 0.308 +/- 0.000 | n/a | 15010 |
| OLSR-Mesh | 0.999 +/- 0.002 | 0.002 +/- 0.001 | 0.505 +/- 0.004 | 1.442 +/- 1.358 | 1.174 +/- 0.042 | 327660 |
| LTE-Infrastruktur | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.516 +/- 0.000 | 14.550 +/- 0.000 | 2.000 +/- 0.000 | 326140 |

Positionsmetriken: minimaler Paarabstand 100.0 m, mittlerer Paarabstand
257.7 m, mittlerer Abstand zum naechsten Nachbarn 100.0 m und maximaler
Paarabstand 532.3 m.

## Deutung

Die korrigierten urbanen Ergebnisse zeigen ein klares Muster. Die
infrastrukturbasierte LTE-Architektur erreicht in allen urbanen Formen eine
vollstaendige Zustellrate. Auch der Anteil unbekannter AoI-Zustaende ist null.
Das ist im Ein-Zellen-Szenario erwartbar, weil alle UAVs ueber die
Infrastruktur kommunizieren und nicht auf direkte Nachbarschaftsbeziehungen
angewiesen sind. Die mittlere Ende-zu-Ende-Latenz liegt konstant bei etwa
14.55 ms. Die Hop-Zahl wird fuer LTE als logische Zwei-Hop-Kommunikation ueber
die Infrastruktur modelliert.

Der OLSR-Mesh-Ansatz erreicht ebenfalls nahezu vollstaendige Zustellung. In
allen drei urbanen Formen liegt die PDR bei mindestens 0.998. Gegenueber
direktem Broadcast verbessert OLSR also vor allem die Erreichbarkeit und
reduziert unbekannte AoI-Zustaende fast vollstaendig. Der Preis dafuer ist ein
deutlich hoeherer Kommunikationsaufwand. Die gesendeten Anwendungbytes liegen
bei OLSR um mehr als eine Groessenordnung ueber Wi-Fi Ad-hoc. Zudem entstehen
mehrere logische Weiterleitungen, wodurch die Latenz hoeher als beim direkten
Broadcast ist. Sie bleibt in diesem Lauf aber klar unterhalb der
LTE-Latenz.

Wi-Fi Ad-hoc Broadcast besitzt die niedrigste Latenz, solange Pakete
ankommen. Die gemessene Latenz liegt in allen urbanen Formen bei etwa
0.3 ms. Gleichzeitig erreicht diese Architektur nicht alle UAV-Paare. Die PDR
liegt je nach urbanem Profil zwischen 0.914 und 0.935. Der unbekannte
AoI-Anteil liegt entsprechend zwischen 6.5 Prozent und 8.6 Prozent. Diese
Architektur ist dadurch sehr effizient und schnell, aber fuer groessere oder
raeumlich gestreckte Schwaerme weniger robust.

Die Unterschiede zwischen den drei urbanen Formen duerfen nicht als reine
Gebaeudeabschattung gelesen werden. Sie entstehen aus einer Kombination aus
Positionierung, Gebaeudeprofil, Ausbreitungsparametern und tatsaechlicher
Schwarmgeometrie. Besonders auffaellig ist, dass `urban-baseline` trotz
hoeherer Gebaeude im Wi-Fi-Ad-hoc-Fall die beste PDR erreicht. Das ist durch
die dichtere tatsaechliche Platzierung erklaerbar: Der mittlere Paarabstand
ist mit 225.6 m geringer als in `urban-open` und `urban-canyon`, und der
mittlere naechste Nachbar liegt nur bei 70.5 m. Die reale Geometrie des
Schwarms ist damit fuer die Interpretation mindestens so wichtig wie der Name
des urbanen Profils.

## Schlussfolgerung fuer den Architekturvergleich

Fuer die betrachteten urbanen Formen ergibt sich folgende Einordnung:

- Wi-Fi Ad-hoc ist die leichtgewichtigste und schnellste Architektur, verliert
  aber bei nicht direkt erreichbaren UAV-Paaren Aktualitaet und Zustellung.
- OLSR-Mesh ist ein robuster Kompromiss: sehr hohe PDR und sehr niedriger
  unbekannter AoI-Anteil, aber deutlich hoeherer Kommunikationsaufwand.
- LTE ist im Ein-Zellen-Modell am zuverlaessigsten, zeigt konstante
  Infrastruktur-Latenz und ist konzeptionell von Netzabdeckung und
  Infrastrukturverfuegbarkeit abhaengig.

Die naechste sinnvolle Vertiefung besteht darin, die Flughoehe relativ zur
Gebaeudehoehe systematisch zu variieren. Gerade fuer urbane UAV-Kommunikation
ist nicht nur die horizontale Verteilung relevant, sondern auch die Frage, ob
die UAVs oberhalb, knapp ueber oder innerhalb der Hoehe der Gebaeudestruktur
operieren.
