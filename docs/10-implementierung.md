# Version 10: Implementierung der ns-3 Simulationen

## Ueberblick

Die Implementierung ist bewusst in mehrere Scratch-Dateien aufgeteilt. Jede
groessere Version bleibt als eigene Datei erhalten, damit der Fortschritt der
Bachelorarbeit nachvollziehbar dokumentiert werden kann.

Die wichtigsten Dateien sind:

```text
scratch/uav-wifi-aoi.cc
scratch/uav-mesh-olsr-aoi.cc
scratch/uav-lte-infrastructure-aoi.cc
scratch/uav-urban-wifi-aoi.cc
scratch/uav-urban-mesh-olsr-aoi.cc
scratch/uav-urban-lte-infrastructure-aoi.cc
scripts/uav-run-experiments.py
```

## Gemeinsame Grundstruktur

Alle Simulationsdateien folgen demselben Muster:

1. Parameter ueber `CommandLine` einlesen.
2. UAV-Nodes erzeugen.
3. Mobility-Modell und Positionen installieren.
4. Kommunikationsarchitektur installieren.
5. UDP-Sockets fuer Positionsupdates erzeugen.
6. Periodische Sendungen planen.
7. AoI-Sampling planen.
8. Simulation starten.
9. CSV-Dateien und Konsolensummary schreiben.

Diese gemeinsame Struktur ist wichtig, damit die Ergebnisse zwischen
Architekturen vergleichbar bleiben.

## Positionsupdates

Jedes Positionsupdate wird als einfache Textnachricht kodiert. Das ist weniger
effizient als ein eigener ns-3 Header, aber in dieser Phase deutlich
nachvollziehbarer.

Bei Broadcast:

```text
senderId sequence sendTimeSeconds x y z
```

Bei Unicast-Architekturen:

```text
senderId receiverId sequence sendTimeSeconds x y z
```

Der Zeitstempel `sendTimeSeconds` ist die Grundlage fuer Latenz und AoI.

## Wi-Fi Ad-hoc Broadcast

Dateien:

```text
scratch/uav-wifi-aoi.cc
scratch/uav-urban-wifi-aoi.cc
```

Technik:

- `WifiHelper`
- `YansWifiChannelHelper`
- `AdhocWifiMac`
- `UdpSocketFactory`
- Broadcast an `255.255.255.255`

Jedes UAV sendet pro Update-Intervall genau ein Broadcast-Paket. Alle anderen
UAVs, die das Paket empfangen, aktualisieren ihre Information ueber den Sender.

Die erwartete Empfangszahl berechnet sich als:

```text
packetsSent * (numUavs - 1)
```

Das ist wichtig, weil ein Broadcast logisch mehrere Empfaenger haben kann.

## OLSR-Mesh

Dateien:

```text
scratch/uav-mesh-olsr-aoi.cc
scratch/uav-urban-mesh-olsr-aoi.cc
```

Technik:

- Wi-Fi Ad-hoc als Funkbasis
- `OlsrHelper` als Routing-Protokoll
- UDP-Unicast von jedem UAV an jedes andere UAV

Im Gegensatz zu Broadcast erzeugt OLSR pro Sender und Intervall ein Paket pro
Ziel-UAV. Bei `n` UAVs entstehen pro Update-Runde:

```text
n * (n - 1)
```

Anwendungspakete.

OLSR bekommt eine Startzeit `appStart`. In unseren Experimenten liegt diese
standardmaessig bei 5 s, damit das Routing vor dem Anwendungstraffic
konvergieren kann.

## LTE-Infrastruktur

Dateien:

```text
scratch/uav-lte-infrastructure-aoi.cc
scratch/uav-urban-lte-infrastructure-aoi.cc
```

Technik:

- UAVs als LTE UEs
- zentrale eNodeB
- `PointToPointEpcHelper` fuer EPC/IP-Kern
- statische Default-Route zum UE-Gateway
- UDP-Unicast all-to-all

Die LTE-Version ist eine Infrastruktur-Baseline. Sie beantwortet nicht die
Frage, ob UAVs ohne Infrastruktur kommunizieren koennen, sondern wie sich eine
zentrale Infrastruktur unter denselben Anwendungsmustern verhaelt.

In der urbanen LTE-Version wird die eNodeB auf eine zentrale Strassenachse
gesetzt. Die Standardhoehe ist 45 m, also knapp ueber der aktuellen
Gebaeudehoehe von 35 m im Baseline-Szenario.

## Urbane Szenarien

Dateien:

```text
scratch/uav-urban-wifi-aoi.cc
scratch/uav-urban-mesh-olsr-aoi.cc
scratch/uav-urban-lte-infrastructure-aoi.cc
```

Die urbanen Dateien enthalten dieselbe Szenarioidee:

- rechteckige Gebaeudebloecke
- orthogonale Strassenkorridore
- deterministische UAV-Positionen auf Strassenachsen
- CSV-Ausgabe der Gebaeudegeometrie

Die Gebaeude werden mit `Building`-Objekten erzeugt:

```text
Box(xMin, xMax, yMin, yMax, 0.0, buildingHeight)
```

Anschliessend wird fuer die Nodes `BuildingsHelper::Install(...)` aufgerufen.
Danach wird `MobilityBuildingInfo::MakeConsistent(...)` verwendet, damit jedes
MobilityModel korrekt als indoor oder outdoor eingeordnet wird.

Der urbane Wi-Fi-Kanal nutzt:

```text
HybridBuildingsPropagationLossModel
```

Auch die urbane LTE-Version setzt dieses Modell ueber den `LteHelper`.

## Age of Information

Die AoI-Implementierung nutzt zwei Matrizen:

```text
lastGenerationTime[receiver][sender]
knownInformation[receiver][sender]
```

`lastGenerationTime` speichert den Erzeugungszeitpunkt der neuesten bekannten
Information. `knownInformation` verhindert, dass der Wert 0.0 falsch
interpretiert wird. Ohne diese zweite Matrix waere unklar, ob ein Empfaenger
noch keine Information besitzt oder ob die letzte Information wirklich bei
Zeitpunkt 0 erzeugt wurde.

Bei jedem AoI-Sample werden alle gerichteten Paare betrachtet:

```text
receiver != sender
```

Wenn Information bekannt ist:

```text
aoi = now - lastGenerationTime[receiver][sender]
```

Wenn noch keine Information bekannt ist, wird `known=0` und `aoi=-1`
geschrieben.

## Latenz

Die Latenz wird beim Empfang berechnet:

```text
latencyMs = (receiveTime - sendTime) * 1000
```

Sie wird als Minimum, Maximum und Durchschnitt ausgegeben. Wichtig ist die
Interpretation: Verlorene Pakete haben keine Latenz. Deshalb muss Latenz immer
zusammen mit PDR und AoI gelesen werden.

## Hop-Anzahl

Bei OLSR und LTE wird am Sender eine initiale IPv4-TTL gesetzt. Am Empfaenger
wird die verbleibende TTL gelesen. Daraus folgt:

```text
hopCount = initialTtl - remainingTtl + 1
```

Bei OLSR beschreibt das die Mesh-Hop-Anzahl. Bei LTE ist es nur eine
Infrastruktur-Hop-Schaetzung und wird deshalb in der Doku bewusst getrennt
interpretiert.

## CSV-Ausgaben

Update-CSV bei Broadcast:

```text
receive_time_s,send_time_s,sender_id,receiver_id,sequence,latency_ms,x_m,y_m,z_m
```

Update-CSV bei OLSR/LTE:

```text
receive_time_s,send_time_s,sender_id,receiver_id,sequence,latency_ms,hop_count,payload_bytes,x_m,y_m,z_m
```

AoI-CSV:

```text
time_s,receiver_id,sender_id,known,aoi_s
```

Buildings-CSV:

```text
building_id,x_min_m,x_max_m,y_min_m,y_max_m,z_min_m,z_max_m
```

## Experiment-Skript

Das Skript:

```text
scripts/uav-run-experiments.py
```

startet die Scratch-Programme systematisch. Es definiert Architekturen,
Szenarien und Ergebnisordner zentral.

Wichtige Profile:

```text
smoke         kurzer Funktionstest
standard      5 und 20 UAVs, 60 m und 100 m
full          5, 10, 20, 40 UAVs sowie 60 m, 100 m, 160 m
urban-forms   urban-open, urban-baseline, urban-canyon
```

Das Skript schreibt pro Lauf:

- Update-CSV
- AoI-CSV
- optional Buildings-CSV
- Logdatei
- zusammengefuehrte `summary.csv`

Die `summary.csv` ist fuer schnelle Tabellen und Plots gedacht.

## Ergebnisordner

Generierte Ergebnisordner unter `results/` werden per `.gitignore`
ausgeschlossen. Die Rohdaten koennen lokal gross werden. Stattdessen werden die
wichtigsten Auswertungen in den versionierten Markdown-Dokumenten festgehalten.

## Warum mehrere Dateien statt eine grosse Simulation?

Die Aufteilung ist absichtlich:

- Jede Architektur bleibt separat verstaendlich.
- Grosse Aenderungen erzeugen neue Dateien und damit dokumentierbare Versionen.
- Fehler in einer Architektur zerlegen nicht den gesamten Vergleich.
- Die Doku kann genau auf die passende Datei verweisen.

Fuer spaetere finale Experimente koennte man gemeinsame Hilfsfunktionen in ein
kleines eigenes Modul auslagern. Fuer die Bachelorarbeitsphase ist die aktuelle
Explizitheit aber hilfreicher, weil jede Datei fuer sich lesbar bleibt.

## Offene technische Punkte

Die wichtigste offene Erweiterung ist eine steady-state Auswertung. Aktuell
enthaelt AoI auch die Startphase. Das sollte im Skript oder in einer separaten
Analysefunktion korrigiert werden.

Der Kommunikationsaufwand sollte um Kontrolltraffic erweitert werden. Besonders
OLSR-Kontrollpakete und LTE-Signalisierung sind fuer einen fairen Vergleich
wichtig.

Bewegung fehlt noch. Der naechste grosse Szenarioschritt waere daher eine
Version mit kontrollierter UAV-Mobilitaet entlang der Strassenkorridore.
