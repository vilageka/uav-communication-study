# UAV Steady-state Vergleichsreport

Dieser Report wird aus `steady-state-summary.csv` Dateien erzeugt.
Er nutzt die eingeschwungene Auswertung, damit Start- und
Konvergenzphasen den Architekturvergleich weniger verzerren.

Die Rangfolge innerhalb eines Szenarios ist bewusst einfach:
zuerst hohe Packet Delivery Ratio, dann wenig unknown AoI, dann
niedrige Latenz fuer erfolgreich empfangene Pakete.
Wenn mehrere `rngRun`-Wiederholungen vorhanden sind, zeigt die
Tabelle Mittelwert +/- Standardabweichung.

## Freie Skalierungserweiterung v21

Quelle: `results/uav-free-scale-extension-v21/steady-state-summary.csv`

### grid, 5 UAVs, 200 m

- Beste PDR: Wi-Fi
- Wenigste unbekannte AoI-Zustaende: Wi-Fi
- Niedrigste Latenz empfangener Pakete: Wi-Fi

| Architektur | Runs | Steady PDR | Steady unknown AoI | Steady Avg AoI | Steady Latenz | Steady Hops | Gesamt App-Bytes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Wi-Fi | 5 | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.581 +/- 0.000 s | 0.303 +/- 0.000 ms | n/a | 2665 +/- 0 |
| OLSR | 5 | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.592 +/- 0.000 s | 0.318 +/- 0.003 ms | 1.000 +/- 0.000 | 12840 +/- 0 |
| LTE | 5 | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.592 +/- 0.000 s | 14.700 +/- 0.000 ms | 2.000 +/- 0.000 | 12760 +/- 0 |

### grid, 20 UAVs, 200 m

- Beste PDR: LTE
- Wenigste unbekannte AoI-Zustaende: LTE
- Niedrigste Latenz empfangener Pakete: Wi-Fi

| Architektur | Runs | Steady PDR | Steady unknown AoI | Steady Avg AoI | Steady Latenz | Steady Hops | Gesamt App-Bytes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| LTE | 5 | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.516 +/- 0.000 s | 14.550 +/- 0.000 ms | 2.000 +/- 0.000 | 266290 +/- 0 |
| OLSR | 5 | 0.997 +/- 0.002 | 0.002 +/- 0.002 | 0.506 +/- 0.003 s | 2.210 +/- 0.930 ms | 1.122 +/- 0.010 | 267810 +/- 0 |
| Wi-Fi | 5 | 0.895 +/- 0.000 | 0.105 +/- 0.000 | 0.506 +/- 0.000 s | 0.305 +/- 0.000 ms | n/a | 11860 +/- 0 |

### grid, 40 UAVs, 60 m

- Beste PDR: Wi-Fi
- Wenigste unbekannte AoI-Zustaende: Wi-Fi
- Niedrigste Latenz empfangener Pakete: Wi-Fi

| Architektur | Runs | Steady PDR | Steady unknown AoI | Steady Avg AoI | Steady Latenz | Steady Hops | Gesamt App-Bytes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Wi-Fi | 5 | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.505 +/- 0.000 s | 0.304 +/- 0.000 ms | n/a | 23990 +/- 0 |
| LTE | 5 | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.515 +/- 0.000 s | 15.205 +/- 0.011 ms | 2.000 +/- 0.000 | 1113525 +/- 0 |
| OLSR | 5 | 0.821 +/- 0.036 | 0.060 +/- 0.004 | 1.010 +/- 0.119 s | 215.953 +/- 27.335 ms | 1.101 +/- 0.036 | 1119765 +/- 0 |

### grid, 40 UAVs, 100 m

- Beste PDR: Wi-Fi
- Wenigste unbekannte AoI-Zustaende: Wi-Fi
- Niedrigste Latenz empfangener Pakete: Wi-Fi

| Architektur | Runs | Steady PDR | Steady unknown AoI | Steady Avg AoI | Steady Latenz | Steady Hops | Gesamt App-Bytes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Wi-Fi | 5 | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.505 +/- 0.000 s | 0.305 +/- 0.000 ms | n/a | 24380 +/- 0 |
| LTE | 5 | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.515 +/- 0.000 s | 15.205 +/- 0.011 ms | 2.000 +/- 0.000 | 1128735 +/- 0 |
| OLSR | 5 | 0.834 +/- 0.035 | 0.060 +/- 0.004 | 0.970 +/- 0.115 s | 207.262 +/- 19.645 ms | 1.088 +/- 0.023 | 1134975 +/- 0 |

### grid, 40 UAVs, 200 m

- Beste PDR: LTE
- Wenigste unbekannte AoI-Zustaende: LTE
- Niedrigste Latenz empfangener Pakete: Wi-Fi

| Architektur | Runs | Steady PDR | Steady unknown AoI | Steady Avg AoI | Steady Latenz | Steady Hops | Gesamt App-Bytes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| LTE | 5 | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.515 +/- 0.000 s | 15.205 +/- 0.011 ms | 2.000 +/- 0.000 | 1146285 +/- 0 |
| Wi-Fi | 5 | 0.614 +/- 0.000 | 0.386 +/- 0.000 | 0.503 +/- 0.000 s | 0.306 +/- 0.000 ms | n/a | 24830 +/- 0 |
| OLSR | 5 | 0.388 +/- 0.004 | 0.171 +/- 0.008 | 3.611 +/- 0.088 s | 327.225 +/- 9.575 ms | 1.675 +/- 0.049 | 1152525 +/- 0 |

## Hinweise zur Interpretation

- Niedrige Latenz allein reicht nicht aus, weil verlorene Pakete keine Latenz haben.
- Unknown AoI zeigt, ob ein UAV ueber andere UAVs gar keine aktuelle Information besitzt.
- App-Bytes sind nur Anwendungslast. Kontrolltraffic ist noch nicht vollstaendig enthalten.
- LTE-Hops sind Infrastruktur-Hops und nicht direkt mit OLSR-Mesh-Hops gleichzusetzen.
