# UAV Communication Study

Dieses Verzeichnis dokumentiert die Scratch-Simulationen fuer die Bachelorarbeit
"Simulation und Vergleich von Kommunikationsarchitekturen fuer kooperative
UAV-Schwaerme in urbanen Umgebungen".

Die eigentlichen ns-3 Programme liegen in `scratch/`. Zu jeder groesseren
Version gibt es hier ein Auswertungsdokument, damit der Fortschritt spaeter
nachvollziehbar bleibt.

## Versionen

| Version | Programm | Zweck |
| --- | --- | --- |
| 01 | `scratch/uav-test.cc` | Erste Mobility-Probe mit statischen UAV-Positionen |
| 02 | `scratch/uav-wifi-baseline.cc` | Wi-Fi Ad-hoc Broadcast mit Paket- und Latenzmetriken |
| 03 | `scratch/uav-wifi-aoi.cc` | Erweiterung um Age of Information |

## Grundidee

Die Simulationen modellieren UAVs als ns-3 Nodes. Die aktuellen Versionen
nutzen statische Positionen auf einem Gitter und Wi-Fi Ad-hoc Kommunikation.
Jedes UAV sendet periodisch Positionsupdates per UDP-Broadcast. Empfaenger
werten Latenz, Zustellrate und ab Version 03 Age of Information aus.

## Wichtige Startbefehle

```bash
./ns3 run uav-test
./ns3 run "uav-wifi-baseline --numUavs=20 --spacing=100"
./ns3 run "uav-wifi-aoi --numUavs=20 --spacing=100 --aoiSampleInterval=0.1"
```

Die Scratch-Ziele werden ohne `scratch/` und ohne `.cc` gestartet.

## Hinweise zur Interpretation

Die aktuellen Ergebnisse sind Baseline-Ergebnisse. Sie zeigen, ob die
Messkette funktioniert und wie sich ein einfacher Ein-Hop-Broadcast verhaelt.
Sie sind noch kein Mesh- oder LTE-Vergleich. Insbesondere bei groesseren
Abstaenden kann Broadcast scheitern, weil entfernte UAVs nicht direkt in
Funkreichweite sind. Genau dieser Effekt ist spaeter ein sinnvoller
Vergleichspunkt fuer Mesh-Routing mit OLSR oder AODV.
