# Version 02: Wi-Fi Ad-hoc Broadcast Baseline

## Programm

`scratch/uav-wifi-baseline.cc`

## Ziel

Diese Version fuehrt eine erste Kommunikationsarchitektur ein. Alle UAVs
nutzen Wi-Fi Ad-hoc und senden periodisch Positionsupdates per UDP-Broadcast.
Damit entsteht eine einfache Ein-Hop-Broadcast-Baseline fuer spaetere
Vergleiche mit Mesh und LTE.

## Modell

- UAVs als ns-3 Nodes
- statische Positionen auf einem 2D-Gitter mit konstanter Hoehe
- Wi-Fi Ad-hoc mit `AdhocWifiMac`
- IPv4-Stack auf allen UAVs
- UDP-Broadcast an `255.255.255.255`
- textbasierte Positionsupdates mit Sender-ID, Sequenznummer, Sendezeit und
  Position

## Parameter

Wichtige Kommandozeilenparameter:

```text
--numUavs          Anzahl der UAVs
--simTime          Simulationsdauer in Sekunden
--updateInterval   Abstand zwischen Positionsupdates pro UAV
--spacing          Gitterabstand zwischen UAVs in Metern
--altitude         Flughoehe in Metern
--txPower          Wi-Fi-Sendeleistung in dBm
--metricsFile      CSV-Datei fuer empfangene Updates
```

## Startbeispiele

```bash
./ns3 run "uav-wifi-baseline --numUavs=5 --simTime=5 --updateInterval=1 --spacing=100"
./ns3 run "uav-wifi-baseline --numUavs=20 --simTime=5 --updateInterval=1 --spacing=60"
```

## CSV-Ausgabe

Die Datei `uav-wifi-baseline-metrics.csv` enthaelt pro empfangenem Update:

```csv
time_s,sender_id,receiver_id,sequence,latency_ms,x_m,y_m,z_m
```

## Erste Messergebnisse

Die folgenden Ergebnisse wurden mit `simTime=5` und `updateInterval=1`
erzeugt. Jedes UAV sendet also 5 Positionsupdates.

| Szenario | Gesendet | Empfangen | Erwartet | Delivery Ratio | Durchschnittliche Latenz |
| --- | ---: | ---: | ---: | ---: | ---: |
| 5 UAVs, 100 m Abstand | 25 | 100 | 100 | 1.000 | 0.301 ms |
| 10 UAVs, 100 m Abstand | 50 | 450 | 450 | 1.000 | 0.302 ms |
| 20 UAVs, 100 m Abstand | 100 | 1700 | 1900 | 0.895 | 0.303 ms |
| 20 UAVs, 80 m Abstand | 100 | 1880 | 1900 | 0.989 | 0.303 ms |
| 20 UAVs, 60 m Abstand | 100 | 1900 | 1900 | 1.000 | 0.303 ms |

## Auswertung

Bei 5 und 10 UAVs mit 100 m Abstand erreichen alle Broadcast-Updates alle
anderen UAVs. Die Zustellrate betraegt 100 Prozent und die gemessene Latenz
liegt bei ungefaehr 0.3 ms.

Bei 20 UAVs und 100 m Abstand sinkt die Delivery Ratio auf etwa 89.5 Prozent.
Die CSV-Auswertung zeigte, dass nicht zufaellige Einzelpakete fehlen, sondern
ganze Sender-Empfaenger-Paare. Das deutet auf Reichweiten- und
Topologieeffekte hin: einige UAVs sind im Ein-Hop-Broadcast nicht direkt
erreichbar.

Der Kontrollvergleich mit 80 m und 60 m Abstand stuetzt diese Deutung. Bei
kleinerem Abstand steigt die Zustellrate wieder deutlich und erreicht bei 60 m
100 Prozent.

## Bedeutung fuer die Arbeit

Wi-Fi Ad-hoc Broadcast ist fuer kleine oder dichte UAV-Schwaerme sehr schnell
und einfach. Sobald aber nicht alle UAVs in direkter Funkreichweite liegen,
entstehen Informationsluecken. Diese Luecken sind ein wichtiger Vergleichspunkt
fuer Mesh-Routing, bei dem Zwischen-UAVs Pakete ueber mehrere Hops weiterleiten
koennen.
