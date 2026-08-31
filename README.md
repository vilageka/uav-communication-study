# UAV Communication Study

Dieses Repository enthaelt die eigenen ns-3 Scratch-Simulationen und
Auswertungsdokumente fuer die Bachelorarbeit:

> Simulation und Vergleich von Kommunikationsarchitekturen fuer kooperative
> UAV-Schwaerme in urbanen Umgebungen

Das komplette ns-3 Projekt ist bewusst **nicht** Teil dieses Repositories.
Stattdessen wird eine externe ns-3 Installation verwendet. Dadurch bleibt
dieses Repo klein, uebersichtlich und gut teilbar.

## Inhalt

```text
scratch/
  uav-test.cc             Erste Mobility-Probe
  uav-wifi-baseline.cc    Wi-Fi Ad-hoc Broadcast mit Latenzmetriken
  uav-wifi-aoi.cc         Wi-Fi Ad-hoc Broadcast mit Age of Information
  uav-mesh-olsr-aoi.cc    Wi-Fi/802.11 Mesh mit OLSR, AoI und Hop-Anzahl
  uav-lte-infrastructure-aoi.cc LTE/EPC-Infrastruktur mit AoI

docs/
  README.md               Uebersicht der Versionen
  01-uav-test.md          Auswertung Version 01
  02-uav-wifi-baseline.md Auswertung Version 02
  03-uav-wifi-aoi.md      Auswertung Version 03
  04-uav-mesh-olsr-aoi.md Auswertung Version 04
  05-uav-lte-infrastructure-aoi.md Auswertung Version 05
  06-uav-experiment-matrix.md Auswertung Version 06

scripts/
  install-to-ns3.sh       Kopiert Scratch-Dateien in ein ns-3 Checkout
  uav-run-experiments.py  Startet reproduzierbare Experimentmatrizen

results/
  README.md               Hinweise fuer erzeugte CSV-Ergebnisse
```

## Voraussetzungen

Dieses Projekt wurde mit einem `ns-3-dev` Checkout entwickelt. Zum Ausfuehren
wird ns-3 benoetigt:

```bash
git clone https://gitlab.com/nsnam/ns-3-dev.git
cd ns-3-dev
./ns3 configure --enable-examples --enable-tests
```

Die genutzte lokale ns-3 Basis beim ersten Projektcommit war:

```text
2eeb26c52218ea30156bfbfc28e7b705e30c9cc1
```

Dieser Commit ist der Commit des urspruenglichen Arbeitsrepos mit den
UAV-Dateien. Fuer reproduzierbare Arbeiten sollte spaeter zusaetzlich ein
konkreter upstream ns-3 Commit dokumentiert werden.

## Installation in ns-3

Dieses Repo neben einem ns-3 Checkout klonen:

```bash
git clone https://github.com/vilageka/uav-communication-study.git
```

Dann die Scratch-Dateien in dein ns-3 Checkout kopieren:

```bash
cd uav-communication-study
./scripts/install-to-ns3.sh /pfad/zu/ns-3-dev
```

Beispiel fuer die aktuelle lokale Struktur:

```bash
./scripts/install-to-ns3.sh /home/vinni/ns3/ns-3-dev
```

Danach im ns-3 Verzeichnis starten:

```bash
cd /home/vinni/ns3/ns-3-dev
./ns3 run uav-test
./ns3 run "uav-wifi-baseline --numUavs=20 --spacing=100"
./ns3 run "uav-wifi-aoi --numUavs=20 --spacing=100 --aoiSampleInterval=0.1"
./ns3 run "uav-mesh-olsr-aoi --numUavs=20 --spacing=100 --appStart=5"
./ns3 run "uav-lte-infrastructure-aoi --numUavs=20 --spacing=100 --appStart=1"
scripts/uav-run-experiments.py --profile standard
```

## Wichtige Parameter

Die Wi-Fi Programme unterstuetzen u.a.:

```text
--numUavs              Anzahl der UAVs
--simTime              Simulationsdauer in Sekunden
--updateInterval       Abstand zwischen Positionsupdates pro UAV
--spacing              Gitterabstand zwischen UAVs in Metern
--altitude             Flughoehe in Metern
--txPower              Wi-Fi-Sendeleistung in dBm
--enablePcap           PCAP-Traces erzeugen
```

`uav-wifi-aoi.cc` und `uav-mesh-olsr-aoi.cc` ergaenzen:

```text
--aoiSampleInterval    Abstand zwischen AoI-Samples in Sekunden
--updateMetricsFile    CSV-Datei fuer empfangene Positionsupdates
--aoiMetricsFile       CSV-Datei fuer AoI-Samples
```

`uav-mesh-olsr-aoi.cc` ergaenzt ausserdem:

```text
--appStart             Vorlaufzeit fuer OLSR-Konvergenz
--initialTtl           Initiale IPv4-TTL fuer Hop-Count-Schaetzung
```

`uav-lte-infrastructure-aoi.cc` nutzt ebenfalls `--appStart` und
`--initialTtl`, interpretiert die Hop-Spalte aber als Infrastruktur-Hop-
Schaetzung. Zusaetzlich gibt es:

```text
--enbHeight            Hoehe der zentralen eNodeB
--enableLteTraces      LTE PHY/MAC/RLC/PDCP-Traces erzeugen
```

## Dokumentationsprinzip

Zu jeder groesseren Simulationsversion wird ein eigenes Dokument in `docs/`
angelegt. Darin stehen:

- Ziel der Version
- Modellannahmen
- Startbefehle
- CSV-Format
- erste Messergebnisse
- Interpretation fuer die Bachelorarbeit

So bleibt nachvollziehbar, welche Version welche Forschungsfrage vorbereitet.

## Aktueller Stand

Die aktuelle Implementierung enthaelt drei Architekturklassen:

1. Wi-Fi Ad-hoc Broadcast als vereinfachte ADS-L-inspirierte Referenz.
2. Wi-Fi/802.11 Mesh mit OLSR als mehrstufiger Ansatz.
3. LTE/EPC-Infrastruktur als zentrale Architektur.

Die naechsten sinnvollen Schritte sind:

1. AoI-Auswertung nach der Einschwingphase ergaenzen.
2. Kommunikationsaufwand inklusive Kontrollpaketen genauer messen.
3. Optional AODV als reaktiven Mesh-Vergleich ergaenzen.
4. Vergleich aller Architekturen anhand von Latenz, Delivery Ratio, AoI,
   Hop-Anzahl und
   Kommunikationslast.
