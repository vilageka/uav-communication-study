# Version 03: Wi-Fi Broadcast With Age of Information

## Programm

`scratch/uav-wifi-aoi.cc`

## Ziel

Diese Version erweitert die Wi-Fi-Broadcast-Baseline um Age of Information
(AoI). AoI misst, wie alt die aktuell verfuegbare Positionsinformation eines
Senders bei einem bestimmten Empfaenger ist.

## Motivation

Reine Latenz betrachtet nur erfolgreich empfangene Pakete. Fuer einen
UAV-Schwarm ist aber wichtig, ob ein UAV ueber andere UAVs aktuelle
Informationen besitzt. Wenn Pakete verloren gehen oder ein Sender nie
erreichbar ist, kann die Latenz der empfangenen Pakete weiterhin niedrig sein,
waehrend die Informationsaktualitaet schlecht ist.

AoI beschreibt diesen Zustand besser:

```text
AoI(receiver, sender, t) = t - generationTime(lastReceivedUpdate(sender))
```

## Modell

Die Kommunikationsbasis entspricht Version 02:

- Wi-Fi Ad-hoc
- IPv4
- UDP-Broadcast
- statische UAV-Positionen auf einem Gitter

Zusaetzlich speichert jeder Empfaenger fuer jeden Sender:

- ob ueberhaupt schon ein Update bekannt ist
- die Erzeugungszeit des letzten bekannten Updates

Die Simulation sampelt periodisch die AoI fuer alle gerichteten
Sender-Empfaenger-Paare.

## Parameter

Zusaetzlich zu Version 02 gibt es:

```text
--aoiSampleInterval  Abstand zwischen AoI-Samples in Sekunden
--updateMetricsFile  CSV-Datei fuer empfangene Positionsupdates
--aoiMetricsFile     CSV-Datei fuer AoI-Zeitreihen
```

## Startbeispiele

```bash
./ns3 run "uav-wifi-aoi --numUavs=5 --simTime=5 --updateInterval=1 --aoiSampleInterval=0.1"
./ns3 run "uav-wifi-aoi --numUavs=20 --spacing=100 --aoiSampleInterval=0.1"
```

## CSV-Ausgaben

Empfangene Updates:

```csv
receive_time_s,send_time_s,sender_id,receiver_id,sequence,latency_ms,x_m,y_m,z_m
```

AoI-Samples:

```csv
time_s,receiver_id,sender_id,known,aoi_s
```

`known=0` und `aoi_s=-1` bedeutet, dass der Empfaenger bis zu diesem Zeitpunkt
noch keine Positionsinformation ueber diesen Sender besitzt.

## Erste Messergebnisse

Alle folgenden Ergebnisse wurden mit `simTime=5`, `updateInterval=1` und
`aoiSampleInterval=0.1` erzeugt.

| Szenario | Delivery Ratio | Durchschnittliche Latenz | Unknown AoI Share | Durchschnittliche bekannte AoI | Max bekannte AoI |
| --- | ---: | ---: | ---: | ---: | ---: |
| 5 UAVs, 100 m Abstand | 1.000 | 0.301 ms | 0.183 | 0.521 s | 1.000 s |
| 20 UAVs, 100 m Abstand | 0.895 | 0.303 ms | 0.277 | 0.492 s | 1.000 s |
| 20 UAVs, 60 m Abstand | 1.000 | 0.303 ms | 0.192 | 0.492 s | 1.000 s |

Die Unknown AoI Share enthaelt auch die Startphase vor dem ersten
Positionsupdate. Betrachtet man nur Samples ab `t >= 1.5s`, ergibt sich:

| Szenario | Unknown Share nach Startphase |
| --- | ---: |
| 20 UAVs, 100 m Abstand | 0.105 |
| 20 UAVs, 60 m Abstand | 0.000 |

## Auswertung

AoI macht sichtbar, was reine Latenz nicht zeigt. Im 20-UAV-Szenario mit
100 m Abstand bleiben einige Sender-Empfaenger-Paare dauerhaft unbekannt.
Die empfangenen Pakete haben zwar weiterhin sehr geringe Latenz, aber manche
Empfaenger besitzen gar keine Positionsinformation ueber bestimmte UAVs.

Bei 60 m Abstand verschwinden diese unbekannten Paare nach der Startphase.
Die Delivery Ratio erreicht 100 Prozent und alle UAVs haben aktuelle
Informationen ueber alle anderen UAVs.

## Bedeutung fuer die Arbeit

AoI ist eine zentrale Metrik fuer die Forschungsfrage nach der Aktualitaet von
Positionsinformationen. Sie sollte in den spaeteren Architekturvergleichen
beibehalten werden. Fuer Mesh-Routing ist zu erwarten, dass entfernte Paare
weniger haeufig unbekannt bleiben, dafuer aber Latenz, Kommunikationslast und
Hop-Anzahl steigen.
