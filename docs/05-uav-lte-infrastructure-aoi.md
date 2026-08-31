# Version 05: LTE Infrastructure With AoI

## Programm

`scratch/uav-lte-infrastructure-aoi.cc`

## Ziel

Diese Version implementiert die LTE-basierte Infrastrukturarchitektur der
Arbeit. Im Gegensatz zur Wi-Fi-Broadcast-Referenz und zum OLSR-Mesh laeuft die
Kommunikation nicht direkt zwischen UAVs oder ueber UAV-Zwischenknoten, sondern
ueber eine zentrale LTE/EPC-Infrastruktur.

## Modell

- jedes UAV ist ein LTE User Equipment (UE)
- eine zentrale eNodeB versorgt den UAV-Schwarm
- das EPC stellt den IP-Kern bereit
- alle UAVs erhalten IPv4-Adressen ueber den EPC
- jedes UAV sendet UDP-Unicast-Positionsupdates an jedes andere UAV
- AoI wird fuer alle gerichteten Sender-Empfaenger-Paare gesampelt

Die eNodeB wird in die Mitte des UAV-Gitters gesetzt. Dadurch bleiben
`--numUavs` und `--spacing` direkt mit den bisherigen Wi-Fi-Szenarien
vergleichbar.

## Vergleichsidee

Die drei bisher relevanten Architekturen unterscheiden sich grundsaetzlich:

```text
Wi-Fi Broadcast:
  ein Paket pro UAV und Update-Runde, nur direkte Funkreichweite

OLSR Mesh:
  Unicast pro Ziel-UAV, Multi-Hop ueber andere UAVs

LTE Infrastruktur:
  Unicast pro Ziel-UAV, Weiterleitung ueber eNodeB/EPC statt UAV-Mesh
```

Damit ist LTE besonders wichtig als Gegenmodell zu dezentralen
Kommunikationsarchitekturen. Die UAVs muessen sich nicht gegenseitig direkt
erreichen, aber sie muessen LTE-Abdeckung haben.

## Parameter

Wichtige Parameter:

```text
--numUavs              Anzahl der UAVs
--simTime              Dauer des Anwendungstraffics in Sekunden
--appStart             Startzeit fuer Positionsupdates nach LTE-Attach
--updateInterval       Abstand zwischen Positionsupdates pro Sender/Ziel-Paar
--aoiSampleInterval    Abstand zwischen AoI-Samples
--spacing              Gitterabstand der UAVs in Metern
--altitude             UAV-Flughoehe in Metern
--enbHeight            Hoehe der zentralen eNodeB in Metern
--initialTtl           initiale IPv4-TTL fuer Hop-Schaetzung
--enableLteTraces      LTE PHY/MAC/RLC/PDCP-Traces aktivieren
```

## Startbeispiele

```bash
./ns3 run "uav-lte-infrastructure-aoi --numUavs=3 --simTime=2 --appStart=1"
./ns3 run "uav-lte-infrastructure-aoi --numUavs=20 --simTime=5 --appStart=1 --spacing=100"
./ns3 run "uav-lte-infrastructure-aoi --numUavs=40 --simTime=3 --appStart=1 --spacing=100"
```

## CSV-Ausgaben

Empfangene Updates:

```csv
receive_time_s,send_time_s,sender_id,receiver_id,sequence,latency_ms,hop_count,payload_bytes,x_m,y_m,z_m
```

AoI-Samples:

```csv
time_s,receiver_id,sender_id,known,aoi_s
```

Die Spalte `hop_count` ist bei LTE als Infrastruktur-Hop-Schaetzung zu
interpretieren, nicht als UAV-Mesh-Hop. In den ersten Tests ergibt sich
durchgehend der Wert 2.

## Erste Messergebnisse

### 3 UAVs, 100 m Abstand

Startbefehl:

```bash
./ns3 run "uav-lte-infrastructure-aoi --numUavs=3 --simTime=2 --appStart=1 --updateInterval=1 --aoiSampleInterval=0.2 --spacing=100"
```

Ergebnis:

| Metrik | Wert |
| --- | ---: |
| Gesendete Anwendungspakete | 12 |
| Empfangene Anwendungspakete | 12 / 12 |
| Delivery Ratio | 1.000 |
| Durchschnittliche Latenz | 14.833 ms |
| Min Latenz | 14.000 ms |
| Max Latenz | 16.000 ms |
| Durchschnittliche Infrastruktur-Hop-Schaetzung | 2.000 |
| Durchschnittliche bekannte AoI | 0.853 s |
| Max bekannte AoI | 1.799 s |

### 20 UAVs, 100 m Abstand

Startbefehl:

```bash
./ns3 run "uav-lte-infrastructure-aoi --numUavs=20 --simTime=5 --appStart=1 --updateInterval=1 --aoiSampleInterval=0.1 --spacing=100"
```

Ergebnis:

| Metrik | Wert |
| --- | ---: |
| Gesendete Anwendungspakete | 1900 |
| Empfangene Anwendungspakete | 1900 / 1900 |
| Delivery Ratio | 1.000 |
| Anwendungsbytes gesendet | 41785 |
| Durchschnittliche Latenz | 14.550 ms |
| Min Latenz | 8.000 ms |
| Max Latenz | 33.000 ms |
| Durchschnittliche Infrastruktur-Hop-Schaetzung | 2.000 |
| Durchschnittliche bekannte AoI | 0.627 s |
| Max bekannte AoI | 1.899 s |
| Unknown Share ab `t >= 2.5s` | 0.000 |

### 20 UAVs, 60 m Abstand

Das Ergebnis entspricht unter diesen vereinfachten Parametern praktisch dem
100-m-Fall:

| Metrik | Wert |
| --- | ---: |
| Delivery Ratio | 1.000 |
| Durchschnittliche Latenz | 14.550 ms |
| Max Latenz | 33.000 ms |
| Unknown Share ab `t >= 2.5s` | 0.000 |

### 40 UAVs, 100 m Abstand

Startbefehl:

```bash
./ns3 run "uav-lte-infrastructure-aoi --numUavs=40 --simTime=3 --appStart=1 --updateInterval=1 --aoiSampleInterval=0.2 --spacing=100"
```

Ergebnis:

| Metrik | Wert |
| --- | ---: |
| Gesendete Anwendungspakete | 4680 |
| Empfangene Anwendungspakete | 4680 / 4680 |
| Delivery Ratio | 1.000 |
| Anwendungsbytes gesendet | 106419 |
| Durchschnittliche Latenz | 14.978 ms |
| Min Latenz | 8.000 ms |
| Max Latenz | 53.000 ms |
| Durchschnittliche Infrastruktur-Hop-Schaetzung | 2.000 |
| Durchschnittliche bekannte AoI | 0.564 s |
| Max bekannte AoI | 1.799 s |
| Unknown Share ab `t >= 2.5s` | 0.001 |

## Auswertung

Die LTE-Infrastrukturversion erreicht in den getesteten Szenarien eine
Delivery Ratio von 100 Prozent. Anders als beim Wi-Fi-Broadcast brechen keine
Sender-Empfaenger-Paare durch fehlende direkte Funkreichweite weg. Das passt
zur Architekturannahme: Kommunikation erfolgt ueber eNodeB/EPC, nicht ueber
direkte UAV-Nachbarschaft.

Die Latenz liegt mit rund 15 ms deutlich hoeher als beim direkten Wi-Fi-
Broadcast, aber niedriger und stabiler als im getesteten OLSR-Mesh-Fall mit
20 UAVs und 100 m Abstand. Die maximale Latenz steigt bei 40 UAVs auf 53 ms,
was ein erstes Zeichen fuer Skalierungseffekte durch mehr gleichzeitige
UE-zu-UE-Fluesse sein kann.

Der Abstand zwischen UAVs hat in den 20-UAV-Tests kaum Einfluss, solange alle
UAVs von der zentralen eNodeB abgedeckt werden. Das unterscheidet LTE deutlich
von Wi-Fi-Broadcast und Mesh, wo die Nachbarschaftsstruktur und Reichweite der
UAVs direkt entscheidend sind.

## Bedeutung fuer die Arbeit

LTE ist in dieser vereinfachten Baseline die stabilste Architektur hinsichtlich
Erreichbarkeit und AoI-Abdeckung. Der Preis ist die Abhaengigkeit von
Infrastruktur und die gemeinsame Nutzung zentraler Funk- und Netzressourcen.

Fuer die spaetere Diskussion ergibt sich damit ein klarer Vergleich:

```text
Wi-Fi Broadcast:
  niedrigste Latenz fuer direkte Nachbarn, wenig Aufwand, aber Reichweitenluecken.

OLSR Mesh:
  bessere Abdeckung als Broadcast, aber mehr Anwendungstraffic und hoehere Latenz.

LTE Infrastruktur:
  sehr gute Abdeckung im Ein-Zellen-Modell, stabile Latenz, aber
  Infrastrukturabhaengigkeit und zentralisierte Ressourcennutzung.
```

Als naechster Schritt sollte ein gemeinsames Experiment-Skript entstehen, das
alle Architekturen systematisch mit mehreren UAV-Zahlen und Abstaenden startet.
