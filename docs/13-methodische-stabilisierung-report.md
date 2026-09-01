# UAV Steady-state Vergleichsreport

Dieser Report wird aus `steady-state-summary.csv` Dateien erzeugt.
Er nutzt die eingeschwungene Auswertung, damit Start- und
Konvergenzphasen den Architekturvergleich weniger verzerren.

Die Rangfolge innerhalb eines Szenarios ist bewusst einfach:
zuerst hohe Packet Delivery Ratio, dann wenig unknown AoI, dann
niedrige Latenz fuer erfolgreich empfangene Pakete.
Wenn mehrere `rngRun`-Wiederholungen vorhanden sind, zeigt die
Tabelle Mittelwert +/- Standardabweichung.

## Methoden-Smoke v13

Quelle: `results/uav-method-smoke-v13b/steady-state-summary.csv`

### grid, 5 UAVs, 100 m

- Beste PDR: Wi-Fi
- Wenigste unbekannte AoI-Zustaende: Wi-Fi
- Niedrigste Latenz empfangener Pakete: Wi-Fi

| Architektur | Runs | Steady PDR | Steady unknown AoI | Steady Avg AoI | Steady Latenz | Steady Hops | Gesamt App-Bytes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Wi-Fi | 2 | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.680 +/- 0.000 s | 0.301 +/- 0.000 ms | n/a | 328 +/- 0 |
| OLSR | 2 | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.738 +/- 0.000 s | 0.309 +/- 0.000 ms | 1.000 +/- 0.000 | 1592 +/- 0 |
| LTE | 2 | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.738 +/- 0.000 s | 14.700 +/- 0.000 ms | 2.000 +/- 0.000 | 1592 +/- 0 |
| Urban LTE | 2 | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.738 +/- 0.000 s | 14.700 +/- 0.000 ms | 2.000 +/- 0.000 | 1656 +/- 0 |
| Urban OLSR | 2 | 1.000 +/- 0.000 | 0.017 +/- 0.024 | 0.747 +/- 0.012 s | 0.811 +/- 0.156 ms | 1.300 +/- 0.000 | 1656 +/- 0 |
| Urban Wi-Fi | 2 | 0.825 +/- 0.035 | 0.175 +/- 0.035 | 0.680 +/- 0.001 s | 0.302 +/- 0.000 ms | n/a | 344 +/- 0 |

## Hinweise zur Interpretation

- Niedrige Latenz allein reicht nicht aus, weil verlorene Pakete keine Latenz haben.
- Unknown AoI zeigt, ob ein UAV ueber andere UAVs gar keine aktuelle Information besitzt.
- App-Bytes sind nur Anwendungslast. Kontrolltraffic ist noch nicht vollstaendig enthalten.
- LTE-Hops sind Infrastruktur-Hops und nicht direkt mit OLSR-Mesh-Hops gleichzusetzen.
