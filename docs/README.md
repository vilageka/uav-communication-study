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
| 04 | `scratch/uav-mesh-olsr-aoi.cc` | Wi-Fi/802.11 Mesh mit OLSR, AoI und Hop-Anzahl |
| 05 | `scratch/uav-lte-infrastructure-aoi.cc` | LTE/EPC-Infrastruktur mit AoI |
| 06 | `scripts/uav-run-experiments.py` | Reproduzierbare Experimentmatrix fuer Architekturvergleiche |
| 07 | `scratch/uav-urban-wifi-aoi.cc` | Urbanes Wi-Fi-Ad-hoc-Szenario mit Gebaeuden |
| 08 | `scratch/uav-urban-*.cc` | Urbane Varianten fuer Wi-Fi, OLSR-Mesh und LTE |
| 09 | `09-konzeption.md` | Konzeption des Architektur- und Szenariovergleichs |
| 10 | `10-implementierung.md` | Implementierungsaufbau der ns-3 Simulationen |
| 11 | `scripts/uav-analyze-results.py` | Steady-state Auswertung vorhandener Ergebnisordner |
| 12 | `scripts/uav-build-report.py` | Markdown-Vergleichsreport aus steady-state Ergebnissen |
| 13 | `13-methodische-stabilisierung.md` | AoI-Out-of-order-Schutz, RNG-Wiederholungen und Broadcast-Bytes |
| 13b | `scripts/uav-aggregate-results.py` | Aggregierte CSV-Auswertung mit Mittelwert, Standardabweichung und 95-Prozent-CI |
| 14 | `14-finale-standardauswertung.md` | Erster echter Standard-Ergebnislauf mit 120 Einzelruns |
| 15 | `15-urbane-formen-auswertung.md` | Korrigierte Auswertung der urbanen Formen mit konsistenter 2.4-GHz-Wi-Fi-Parametrisierung |
| 16 | `16-kapitel-konzeption-ueberarbeitet.md` | Ueberarbeitete Konzeption als Grundlage fuer das Bachelorarbeitskapitel |
| 17 | `17-kapitel-implementierung-ueberarbeitet.md` | Ueberarbeitete Implementierungsbeschreibung als Grundlage fuer das Bachelorarbeitskapitel |
| 18 | `18-urbane-hoehenvariation-auswertung.md` | Ergebnislauf zur Flughoehe relativ zur Gebaeudehoehe mit 135 Einzelruns |

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
./ns3 run "uav-urban-wifi-aoi --numUavs=20 --spacing=100"
./ns3 run "uav-urban-mesh-olsr-aoi --numUavs=20 --spacing=100 --appStart=5"
./ns3 run "uav-urban-lte-infrastructure-aoi --numUavs=20 --spacing=100 --appStart=1"
scripts/uav-run-experiments.py --profile standard
scripts/uav-run-experiments.py --profile standard --runs 10 --sim-time 30
scripts/uav-analyze-results.py results/uav-urban-all-v08
scripts/uav-aggregate-results.py results/uav-urban-all-v08/steady-state-summary.csv
```

Die Scratch-Ziele werden ohne `scratch/` und ohne `.cc` gestartet.

## Aktueller Stand fuer die Bachelorarbeit

Fuer die freien Standard-Szenarien liegt ein Ergebnislauf mit 120 Einzelruns
vor. Fuer die urbanen Formen liegt mit `v16` ein korrigierter Lauf mit
45 Einzelruns vor. Der vorherige urbane Diagnose-Lauf `v15` wird nicht als
finale Ergebnisbasis verwendet, weil Wi-Fi-Standard und Modellfrequenz dort
nicht konsistent parametrisiert waren.

Die ausformulierten Kapitelentwuerfe fuer die Bachelorarbeit sind:

- `16-kapitel-konzeption-ueberarbeitet.md`
- `17-kapitel-implementierung-ueberarbeitet.md`
- `15-urbane-formen-auswertung.md`
- `18-urbane-hoehenvariation-auswertung.md`

## Hinweise zur Interpretation

Die ersten Ergebnisse zeigen zwei Architekturklassen:

- Wi-Fi Ad-hoc Broadcast als vereinfachte ADS-L-inspirierte Referenz.
- Wi-Fi/802.11 Mesh mit OLSR als mehrstufiger Ansatz.
- LTE/EPC-Infrastruktur als zentrale Architektur.

Broadcast ist sehr schnell und erzeugt wenig Anwendungstraffic, scheitert aber
bei entfernten UAV-Paaren ohne direkte Funkreichweite. OLSR-Mesh kann einen
Teil dieser Luecken ueber mehrere Hops schliessen, erzeugt aber deutlich mehr
Unicast-Traffic und hoehere Latenz. LTE erreicht in den ersten vereinfachten
Ein-Zellen-Szenarien alle UAV-Paare zuverlaessig, ist aber von Infrastruktur
und Scheduling abhaengig.
