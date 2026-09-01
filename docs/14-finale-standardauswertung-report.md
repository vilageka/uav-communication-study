# UAV Steady-state Vergleichsreport

Dieser Report wird aus `steady-state-summary.csv` Dateien erzeugt.
Er nutzt die eingeschwungene Auswertung, damit Start- und
Konvergenzphasen den Architekturvergleich weniger verzerren.

Die Rangfolge innerhalb eines Szenarios ist bewusst einfach:
zuerst hohe Packet Delivery Ratio, dann wenig unknown AoI, dann
niedrige Latenz fuer erfolgreich empfangene Pakete.
Wenn mehrere `rngRun`-Wiederholungen vorhanden sind, zeigt die
Tabelle Mittelwert +/- Standardabweichung.

## Finaler Standardlauf v14

Quelle: `results/uav-final-standard-v14/steady-state-summary.csv`

### grid, 5 UAVs, 60 m

- Beste PDR: Wi-Fi
- Wenigste unbekannte AoI-Zustaende: Wi-Fi
- Niedrigste Latenz empfangener Pakete: Wi-Fi

| Architektur | Runs | Steady PDR | Steady unknown AoI | Steady Avg AoI | Steady Latenz | Steady Hops | Gesamt App-Bytes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Wi-Fi | 5 | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.581 +/- 0.000 s | 0.302 +/- 0.000 ms | n/a | 2545 +/- 0 |
| OLSR | 5 | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.592 +/- 0.000 s | 0.316 +/- 0.002 ms | 1.000 +/- 0.000 | 12360 +/- 0 |
| Urban OLSR | 5 | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.592 +/- 0.000 s | 0.402 +/- 0.048 ms | 1.080 +/- 0.045 | 13200 +/- 0 |
| LTE | 5 | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.592 +/- 0.000 s | 14.700 +/- 0.000 ms | 2.000 +/- 0.000 | 12280 +/- 0 |
| Urban LTE | 5 | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.592 +/- 0.000 s | 14.700 +/- 0.000 ms | 2.000 +/- 0.000 | 13120 +/- 0 |
| Urban Wi-Fi | 5 | 0.940 +/- 0.055 | 0.060 +/- 0.055 | 0.582 +/- 0.000 s | 0.303 +/- 0.000 ms | n/a | 2755 +/- 0 |

### grid, 5 UAVs, 100 m

- Beste PDR: Wi-Fi
- Wenigste unbekannte AoI-Zustaende: Wi-Fi
- Niedrigste Latenz empfangener Pakete: Wi-Fi

| Architektur | Runs | Steady PDR | Steady unknown AoI | Steady Avg AoI | Steady Latenz | Steady Hops | Gesamt App-Bytes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Wi-Fi | 5 | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.581 +/- 0.000 s | 0.302 +/- 0.000 ms | n/a | 2665 +/- 0 |
| OLSR | 5 | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.592 +/- 0.000 s | 0.317 +/- 0.003 ms | 1.000 +/- 0.000 | 12840 +/- 0 |
| LTE | 5 | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.592 +/- 0.000 s | 14.700 +/- 0.000 ms | 2.000 +/- 0.000 | 12760 +/- 0 |
| Urban LTE | 5 | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.592 +/- 0.000 s | 14.700 +/- 0.000 ms | 2.000 +/- 0.000 | 13240 +/- 0 |
| Urban OLSR | 5 | 1.000 +/- 0.000 | 0.000 +/- 0.001 | 0.593 +/- 0.001 s | 0.654 +/- 0.100 ms | 1.300 +/- 0.071 | 13320 +/- 0 |
| Urban Wi-Fi | 5 | 0.830 +/- 0.027 | 0.170 +/- 0.027 | 0.581 +/- 0.001 s | 0.303 +/- 0.000 ms | n/a | 2785 +/- 0 |

### grid, 20 UAVs, 60 m

- Beste PDR: Wi-Fi
- Wenigste unbekannte AoI-Zustaende: Wi-Fi
- Niedrigste Latenz empfangener Pakete: Wi-Fi

| Architektur | Runs | Steady PDR | Steady unknown AoI | Steady Avg AoI | Steady Latenz | Steady Hops | Gesamt App-Bytes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Wi-Fi | 5 | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.506 +/- 0.000 s | 0.304 +/- 0.000 ms | n/a | 11590 +/- 0 |
| LTE | 5 | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.516 +/- 0.000 s | 14.550 +/- 0.000 ms | 2.000 +/- 0.000 | 261160 +/- 0 |
| Urban LTE | 5 | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.516 +/- 0.000 s | 14.550 +/- 0.000 ms | 2.000 +/- 0.000 | 269140 +/- 0 |
| OLSR | 5 | 1.000 +/- 0.000 | 0.001 +/- 0.000 | 0.503 +/- 0.000 s | 0.371 +/- 0.007 ms | 1.000 +/- 0.000 | 262680 +/- 0 |
| Urban OLSR | 5 | 0.994 +/- 0.005 | 0.004 +/- 0.004 | 0.516 +/- 0.010 s | 6.912 +/- 4.568 ms | 1.353 +/- 0.043 | 270660 +/- 0 |
| Urban Wi-Fi | 5 | 0.826 +/- 0.027 | 0.174 +/- 0.027 | 0.506 +/- 0.001 s | 0.304 +/- 0.000 ms | n/a | 12010 +/- 0 |

### grid, 20 UAVs, 100 m

- Beste PDR: LTE
- Wenigste unbekannte AoI-Zustaende: LTE
- Niedrigste Latenz empfangener Pakete: Wi-Fi

| Architektur | Runs | Steady PDR | Steady unknown AoI | Steady Avg AoI | Steady Latenz | Steady Hops | Gesamt App-Bytes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| LTE | 5 | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.516 +/- 0.000 s | 14.550 +/- 0.000 ms | 2.000 +/- 0.000 | 266290 +/- 0 |
| Urban LTE | 5 | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.516 +/- 0.000 s | 14.550 +/- 0.000 ms | 2.000 +/- 0.000 | 269710 +/- 0 |
| OLSR | 5 | 0.994 +/- 0.007 | 0.003 +/- 0.002 | 0.515 +/- 0.018 s | 3.279 +/- 3.664 ms | 1.133 +/- 0.029 | 267810 +/- 0 |
| Urban OLSR | 5 | 0.991 +/- 0.007 | 0.006 +/- 0.005 | 0.528 +/- 0.017 s | 11.540 +/- 6.376 ms | 1.405 +/- 0.046 | 271230 +/- 0 |
| Wi-Fi | 5 | 0.895 +/- 0.000 | 0.105 +/- 0.000 | 0.506 +/- 0.000 s | 0.304 +/- 0.000 ms | n/a | 11860 +/- 0 |
| Urban Wi-Fi | 5 | 0.784 +/- 0.030 | 0.216 +/- 0.030 | 0.507 +/- 0.002 s | 0.304 +/- 0.000 ms | n/a | 12040 +/- 0 |

## Hinweise zur Interpretation

- Niedrige Latenz allein reicht nicht aus, weil verlorene Pakete keine Latenz haben.
- Unknown AoI zeigt, ob ein UAV ueber andere UAVs gar keine aktuelle Information besitzt.
- App-Bytes sind nur Anwendungslast. Kontrolltraffic ist noch nicht vollstaendig enthalten.
- LTE-Hops sind Infrastruktur-Hops und nicht direkt mit OLSR-Mesh-Hops gleichzusetzen.
