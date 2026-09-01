# Version 13: Methodische Stabilisierung der Auswertung

Diese Version setzt drei Korrekturen um, die vor finalen Ergebnislaeufen
wichtig sind. Ziel ist nicht, eine neue Kommunikationsarchitektur einzufuehren,
sondern die bestehende Messkette belastbarer und besser auswertbar zu machen.

## 1. AoI-Schutz gegen veraltete Pakete

Age of Information beschreibt den aktuellsten Informationsstand, den ein
Empfaenger ueber einen Sender besitzt. Deshalb darf ein spaeter eintreffendes
aelteres Paket diesen Zustand nicht ueberschreiben.

Vor dieser Version wurde bei jedem empfangenen Paket direkt gesetzt:

```text
lastGenerationTime[receiver][sender] = sendTime
knownInformation[receiver][sender] = true
```

Das ist in einfachen Faellen unproblematisch, kann bei Queueing, Routing oder
unterschiedlichen Pfaden aber theoretisch einen Fehler erzeugen: Ein neueres
Update kann zuerst eintreffen, ein aelteres danach. Dann wuerde AoI kuenstlich
schlechter, obwohl der Empfaenger bereits frischere Information besitzt.

Die Scratch-Programme nutzen deshalb nun sinngemaess:

```text
accept = information_unknown || sendTime > lastGenerationTime[receiver][sender]
```

Wichtig: Das Paket zaehlt weiterhin fuer PDR, Latenz und Byte-Metriken. Nur der
gespeicherte AoI-Zustand wird nicht auf einen aelteren Zeitstempel
zurueckgesetzt.

Betroffene Dateien:

- `scratch/uav-wifi-aoi.cc`
- `scratch/uav-mesh-olsr-aoi.cc`
- `scratch/uav-lte-infrastructure-aoi.cc`
- `scratch/uav-urban-wifi-aoi.cc`
- `scratch/uav-urban-mesh-olsr-aoi.cc`
- `scratch/uav-urban-lte-infrastructure-aoi.cc`

## 2. Reproduzierbare Mehrfachlaeufe

Alle aktuellen Simulationsprogramme besitzen nun den Parameter:

```bash
--rngRun=<zahl>
```

Im C++-Programm wird damit `RngSeedManager::SetRun(rngRun)` gesetzt. Der Seed
bleibt konstant, der Run wird variiert. Das entspricht der ueblichen ns-3-Idee
fuer wiederholbare, aber statistisch getrennte Laeufe.

Das Experiment-Skript besitzt zusaetzlich:

```bash
--runs <anzahl>
```

Beispiel fuer zehn Wiederholungen der Standardmatrix:

```bash
python3 scripts/uav-run-experiments.py --profile standard --runs 10 --sim-time 30
```

Die Rohdaten bekommen den Run im Dateinamen, z.B.:

```text
wifi-adhoc_n20_d100_r3_updates.csv
```

Die Spalte `rng_run` steht ausserdem in `summary.csv` und wird von der
Steady-State-Auswertung in `steady-state-summary.csv` uebernommen.

## 3. Vergleichbare Broadcast-Byte-Metrik

OLSR und LTE hatten bereits `appBytesSent`, `appBytesReceived` und die
CSV-Spalte `payload_bytes`. Die Broadcast-Varianten hatten diese Werte noch
nicht vollstaendig.

Das ist nun vereinheitlicht:

- Wi-Fi-Broadcast zaehlt gesendete und empfangene Nutzlastbytes.
- Urban-Wi-Fi-Broadcast zaehlt gesendete und empfangene Nutzlastbytes.
- Die Update-CSV enthaelt bei Broadcast nun ebenfalls `payload_bytes`.
- Die Konsolensummary gibt `Application bytes sent` und
  `Application bytes received` aus.

Damit kann Kommunikationsaufwand auf Anwendungsebene fuer alle drei
Architekturklassen zumindest auf Paket- und Nutzdatenebene verglichen werden.
Kontrolltraffic, z.B. OLSR-Routingnachrichten oder LTE-internes Signaling, ist
damit weiterhin noch nicht vollstaendig erfasst und muss in der Arbeit als
Einschraenkung genannt werden.

## 4. Report mit Mittelwert und Standardabweichung

`scripts/uav-build-report.py` gruppiert Wiederholungen nun nach Architektur,
Szenario, UAV-Zahl und Abstand. Wenn mehrere `rngRun`-Wiederholungen vorhanden
sind, werden die Tabellen als Mittelwert +/- Standardabweichung ausgegeben.

Dadurch eignen sich die Reports besser fuer die spaetere Bachelorarbeit, weil
nicht nur Einzelwerte, sondern auch die Streuung zwischen Wiederholungen
sichtbar wird.

Zusaetzlich gibt es `scripts/uav-aggregate-results.py`. Dieses Skript erzeugt
aus einer `steady-state-summary.csv` eine maschinenlesbare
`aggregate-summary.csv` mit:

- Anzahl der Wiederholungen `n`
- Mittelwert
- Standardabweichung
- 95-Prozent-Konfidenzintervall als halbe Intervallbreite
- Minimum und Maximum

## 5. Verifikation

Ausgefuehrte technische Pruefungen:

```bash
python3 -m py_compile scripts/uav-run-experiments.py scripts/uav-analyze-results.py scripts/uav-build-report.py scripts/uav-aggregate-results.py
./ns3 run "uav-wifi-aoi --numUavs=3 --simTime=2 --updateInterval=1 --aoiSampleInterval=0.5 --rngRun=2 --updateMetricsFile=/tmp/uav-wifi-aoi-rng-smoke-updates.csv --aoiMetricsFile=/tmp/uav-wifi-aoi-rng-smoke-aoi.csv"
python3 scripts/uav-run-experiments.py --profile smoke --runs 2 --sim-time 4 --aoi-sample-interval 0.5 --results-dir results/uav-method-smoke-v13b --timeout 600
python3 scripts/uav-aggregate-results.py results/uav-method-smoke-v13b/steady-state-summary.csv
python3 scripts/uav-build-report.py --source "Methoden-Smoke v13" results/uav-method-smoke-v13b/steady-state-summary.csv --output doc/uav-communication-study/13-methodische-stabilisierung-report.md
```

Der Smoke-Run besteht aus 12 Einzelruns:

- 6 Architekturen
- 1 Szenario mit 5 UAVs und 100 m Abstand
- 2 RNG-Wiederholungen

Die Ergebnisse liegen in:

```text
results/uav-method-smoke-v13b/
doc/uav-communication-study/13-methodische-stabilisierung-report.md
```

## 6. Interpretation des Smoke-Runs

Der Smoke-Run ist keine finale Auswertung, sondern eine Plausibilitaetspruefung
der neuen Methodik. Trotzdem zeigt er erwartbare Tendenzen:

- Freifeld-Wi-Fi, OLSR und LTE erreichen bei 5 UAVs und 100 m jeweils PDR 1.0.
- Urban-Wi-Fi faellt im Smoke auf etwa 0.825 PDR im Mittel, weil Broadcast ohne
  Routing in der Gebaeudeumgebung empfindlicher ist.
- Urban-OLSR erreicht im Steady-State PDR 1.0, hat aber mehr
  Kommunikationsaufwand als Broadcast.
- LTE bleibt in diesem kleinen Test stabil, zeigt aber eine deutlich hoehere
  Latenz als direkter Wi-Fi-Empfang.

Fuer die finalen Ergebnislaeufe sollte die Simulationsdauer wieder deutlich
laenger gewaehlt werden, z.B. 30 s oder mehr, und `--runs=10` oder hoeher
verwendet werden.
