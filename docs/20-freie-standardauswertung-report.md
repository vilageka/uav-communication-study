# UAV Steady-state Vergleichsreport

Dieser Report wird aus `steady-state-summary.csv` Dateien erzeugt.
Er nutzt die eingeschwungene Auswertung, damit Start- und
Konvergenzphasen den Architekturvergleich weniger verzerren.

Die Rangfolge innerhalb eines Szenarios ist bewusst einfach:
zuerst hohe Packet Delivery Ratio, dann wenig unknown AoI, dann
niedrige Latenz fuer erfolgreich empfangene Pakete.
Wenn mehrere `rngRun`-Wiederholungen vorhanden sind, zeigt die
Tabelle Mittelwert +/- Standardabweichung.

## Freie Standardauswertung v20

Quelle: `results/uav-final-free-standard-v20/steady-state-summary.csv`

### grid, 5 UAVs, 60 m

- Beste PDR: Wi-Fi
- Wenigste unbekannte AoI-Zustaende: Wi-Fi
- Niedrigste Latenz empfangener Pakete: Wi-Fi

| Architektur | Runs | Steady PDR | Steady unknown AoI | Steady Avg AoI | Steady Latenz | Steady Hops | Gesamt App-Bytes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Wi-Fi | 5 | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.581 +/- 0.000 s | 0.302 +/- 0.000 ms | n/a | 2545 +/- 0 |
| OLSR | 5 | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.592 +/- 0.000 s | 0.316 +/- 0.002 ms | 1.000 +/- 0.000 | 12360 +/- 0 |
| LTE | 5 | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.592 +/- 0.000 s | 14.700 +/- 0.000 ms | 2.000 +/- 0.000 | 12280 +/- 0 |

### grid, 5 UAVs, 100 m

- Beste PDR: Wi-Fi
- Wenigste unbekannte AoI-Zustaende: Wi-Fi
- Niedrigste Latenz empfangener Pakete: Wi-Fi

| Architektur | Runs | Steady PDR | Steady unknown AoI | Steady Avg AoI | Steady Latenz | Steady Hops | Gesamt App-Bytes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Wi-Fi | 5 | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.581 +/- 0.000 s | 0.302 +/- 0.000 ms | n/a | 2665 +/- 0 |
| OLSR | 5 | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.592 +/- 0.000 s | 0.317 +/- 0.003 ms | 1.000 +/- 0.000 | 12840 +/- 0 |
| LTE | 5 | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.592 +/- 0.000 s | 14.700 +/- 0.000 ms | 2.000 +/- 0.000 | 12760 +/- 0 |

### grid, 20 UAVs, 60 m

- Beste PDR: Wi-Fi
- Wenigste unbekannte AoI-Zustaende: Wi-Fi
- Niedrigste Latenz empfangener Pakete: Wi-Fi

| Architektur | Runs | Steady PDR | Steady unknown AoI | Steady Avg AoI | Steady Latenz | Steady Hops | Gesamt App-Bytes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Wi-Fi | 5 | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.506 +/- 0.000 s | 0.304 +/- 0.000 ms | n/a | 11590 +/- 0 |
| LTE | 5 | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.516 +/- 0.000 s | 14.550 +/- 0.000 ms | 2.000 +/- 0.000 | 261160 +/- 0 |
| OLSR | 5 | 1.000 +/- 0.000 | 0.001 +/- 0.000 | 0.503 +/- 0.000 s | 0.366 +/- 0.013 ms | 1.000 +/- 0.000 | 262680 +/- 0 |

### grid, 20 UAVs, 100 m

- Beste PDR: Wi-Fi
- Wenigste unbekannte AoI-Zustaende: Wi-Fi
- Niedrigste Latenz empfangener Pakete: Wi-Fi

| Architektur | Runs | Steady PDR | Steady unknown AoI | Steady Avg AoI | Steady Latenz | Steady Hops | Gesamt App-Bytes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Wi-Fi | 5 | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.506 +/- 0.000 s | 0.304 +/- 0.000 ms | n/a | 11860 +/- 0 |
| LTE | 5 | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.516 +/- 0.000 s | 14.550 +/- 0.000 ms | 2.000 +/- 0.000 | 266290 +/- 0 |
| OLSR | 5 | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.503 +/- 0.000 s | 0.461 +/- 0.219 ms | 1.002 +/- 0.005 | 267810 +/- 0 |

## Hinweise zur Interpretation

- Niedrige Latenz allein reicht nicht aus, weil verlorene Pakete keine Latenz haben.
- Unknown AoI zeigt, ob ein UAV ueber andere UAVs gar keine aktuelle Information besitzt.
- App-Bytes sind nur Anwendungslast. Kontrolltraffic ist noch nicht vollstaendig enthalten.
- LTE-Hops sind Infrastruktur-Hops und nicht direkt mit OLSR-Mesh-Hops gleichzusetzen.
