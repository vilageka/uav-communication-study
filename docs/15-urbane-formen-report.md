# UAV Steady-state Vergleichsreport

Dieser Report wird aus `steady-state-summary.csv` Dateien erzeugt.
Er nutzt die eingeschwungene Auswertung, damit Start- und
Konvergenzphasen den Architekturvergleich weniger verzerren.

Die Rangfolge innerhalb eines Szenarios ist bewusst einfach:
zuerst hohe Packet Delivery Ratio, dann wenig unknown AoI, dann
niedrige Latenz fuer erfolgreich empfangene Pakete.
Wenn mehrere `rngRun`-Wiederholungen vorhanden sind, zeigt die
Tabelle Mittelwert +/- Standardabweichung.

## Urbane Formen v16

Quelle: `results/uav-urban-forms-v16/steady-state-summary.csv`

### urban-baseline, 20 UAVs, 100 m

- Beste PDR: Urban LTE
- Wenigste unbekannte AoI-Zustaende: Urban LTE
- Niedrigste Latenz empfangener Pakete: Urban Wi-Fi

| Architektur | Runs | Steady PDR | Steady unknown AoI | Steady Avg AoI | Steady Latenz | Steady Hops | Gesamt App-Bytes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Urban LTE | 5 | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.516 +/- 0.000 s | 14.550 +/- 0.000 ms | 2.000 +/- 0.000 | 269710 +/- 0 |
| Urban OLSR | 5 | 0.998 +/- 0.003 | 0.001 +/- 0.000 | 0.507 +/- 0.006 s | 1.932 +/- 1.584 ms | 1.150 +/- 0.020 | 271230 +/- 0 |
| Urban Wi-Fi | 5 | 0.935 +/- 0.015 | 0.065 +/- 0.015 | 0.507 +/- 0.001 s | 0.304 +/- 0.000 ms | n/a | 12040 +/- 0 |

### urban-canyon, 20 UAVs, 100 m

- Beste PDR: Urban LTE
- Wenigste unbekannte AoI-Zustaende: Urban LTE
- Niedrigste Latenz empfangener Pakete: Urban Wi-Fi

| Architektur | Runs | Steady PDR | Steady unknown AoI | Steady Avg AoI | Steady Latenz | Steady Hops | Gesamt App-Bytes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Urban LTE | 5 | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.516 +/- 0.000 s | 14.550 +/- 0.000 ms | 2.000 +/- 0.000 | 326140 +/- 0 |
| Urban OLSR | 5 | 0.999 +/- 0.002 | 0.002 +/- 0.001 | 0.505 +/- 0.004 s | 1.442 +/- 1.358 ms | 1.174 +/- 0.042 | 327660 +/- 0 |
| Urban Wi-Fi | 5 | 0.918 +/- 0.015 | 0.082 +/- 0.015 | 0.506 +/- 0.000 s | 0.308 +/- 0.000 ms | n/a | 15010 +/- 0 |

### urban-open, 20 UAVs, 100 m

- Beste PDR: Urban LTE
- Wenigste unbekannte AoI-Zustaende: Urban LTE
- Niedrigste Latenz empfangener Pakete: Urban Wi-Fi

| Architektur | Runs | Steady PDR | Steady unknown AoI | Steady Avg AoI | Steady Latenz | Steady Hops | Gesamt App-Bytes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Urban LTE | 5 | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.516 +/- 0.000 s | 14.550 +/- 0.000 ms | 2.000 +/- 0.000 | 271420 +/- 0 |
| Urban OLSR | 5 | 0.999 +/- 0.001 | 0.002 +/- 0.001 | 0.506 +/- 0.002 s | 1.928 +/- 1.142 ms | 1.197 +/- 0.054 | 272940 +/- 0 |
| Urban Wi-Fi | 5 | 0.914 +/- 0.013 | 0.086 +/- 0.013 | 0.506 +/- 0.000 s | 0.305 +/- 0.000 ms | n/a | 12130 +/- 0 |

## Hinweise zur Interpretation

- Niedrige Latenz allein reicht nicht aus, weil verlorene Pakete keine Latenz haben.
- Unknown AoI zeigt, ob ein UAV ueber andere UAVs gar keine aktuelle Information besitzt.
- App-Bytes sind nur Anwendungslast. Kontrolltraffic ist noch nicht vollstaendig enthalten.
- LTE-Hops sind Infrastruktur-Hops und nicht direkt mit OLSR-Mesh-Hops gleichzusetzen.
