# UAV Steady-state Vergleichsreport

Dieser Report wird aus `steady-state-summary.csv` Dateien erzeugt.
Er nutzt die eingeschwungene Auswertung, damit Start- und
Konvergenzphasen den Architekturvergleich weniger verzerren.

Die Rangfolge innerhalb eines Szenarios ist bewusst einfach:
zuerst hohe Packet Delivery Ratio, dann wenig unknown AoI, dann
niedrige Latenz fuer erfolgreich empfangene Pakete.
Wenn mehrere `rngRun`-Wiederholungen vorhanden sind, zeigt die
Tabelle Mittelwert +/- Standardabweichung.

## Urbane Hoehenvariation v18

Quelle: `results/uav-urban-heights-v18/steady-state-summary.csv`

### urban-baseline-above-roof, 20 UAVs, 100 m, altitude 80.0 m

- Beste PDR: Urban LTE
- Wenigste unbekannte AoI-Zustaende: Urban LTE
- Niedrigste Latenz empfangener Pakete: Urban Wi-Fi

| Architektur | Runs | Steady PDR | Steady unknown AoI | Steady Avg AoI | Steady Latenz | Steady Hops | Gesamt App-Bytes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Urban LTE | 5 | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.516 +/- 0.000 s | 14.550 +/- 0.000 ms | 2.000 +/- 0.000 | 269710 +/- 0 |
| Urban OLSR | 5 | 0.998 +/- 0.003 | 0.001 +/- 0.000 | 0.507 +/- 0.006 s | 1.932 +/- 1.584 ms | 1.150 +/- 0.020 | 271230 +/- 0 |
| Urban Wi-Fi | 5 | 0.935 +/- 0.015 | 0.065 +/- 0.015 | 0.507 +/- 0.001 s | 0.304 +/- 0.000 ms | n/a | 12040 +/- 0 |

### urban-baseline-inside, 20 UAVs, 100 m, altitude 26.2 m

- Beste PDR: Urban LTE
- Wenigste unbekannte AoI-Zustaende: Urban LTE
- Niedrigste Latenz empfangener Pakete: Urban Wi-Fi

| Architektur | Runs | Steady PDR | Steady unknown AoI | Steady Avg AoI | Steady Latenz | Steady Hops | Gesamt App-Bytes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Urban LTE | 5 | 0.978 +/- 0.030 | 0.020 +/- 0.027 | 0.518 +/- 0.003 s | 14.494 +/- 0.465 ms | 2.000 +/- 0.000 | 303910 +/- 0 |
| Urban OLSR | 5 | 0.931 +/- 0.016 | 0.052 +/- 0.014 | 0.598 +/- 0.015 s | 34.455 +/- 10.292 ms | 1.979 +/- 0.056 | 305430 +/- 0 |
| Urban Wi-Fi | 5 | 0.359 +/- 0.004 | 0.641 +/- 0.004 | 0.507 +/- 0.001 s | 0.306 +/- 0.000 ms | n/a | 13840 +/- 0 |

### urban-baseline-near-roof, 20 UAVs, 100 m, altitude 40.0 m

- Beste PDR: Urban LTE
- Wenigste unbekannte AoI-Zustaende: Urban LTE
- Niedrigste Latenz empfangener Pakete: Urban Wi-Fi

| Architektur | Runs | Steady PDR | Steady unknown AoI | Steady Avg AoI | Steady Latenz | Steady Hops | Gesamt App-Bytes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Urban LTE | 5 | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.516 +/- 0.000 s | 14.550 +/- 0.000 ms | 2.000 +/- 0.000 | 269710 +/- 0 |
| Urban OLSR | 5 | 0.997 +/- 0.002 | 0.003 +/- 0.002 | 0.505 +/- 0.004 s | 1.336 +/- 1.165 ms | 1.141 +/- 0.015 | 271230 +/- 0 |
| Urban Wi-Fi | 5 | 0.934 +/- 0.013 | 0.066 +/- 0.013 | 0.507 +/- 0.001 s | 0.304 +/- 0.000 ms | n/a | 12040 +/- 0 |

### urban-canyon-above-roof, 20 UAVs, 100 m, altitude 80.0 m

- Beste PDR: Urban LTE
- Wenigste unbekannte AoI-Zustaende: Urban LTE
- Niedrigste Latenz empfangener Pakete: Urban Wi-Fi

| Architektur | Runs | Steady PDR | Steady unknown AoI | Steady Avg AoI | Steady Latenz | Steady Hops | Gesamt App-Bytes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Urban LTE | 5 | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.516 +/- 0.000 s | 14.550 +/- 0.000 ms | 2.000 +/- 0.000 | 326140 +/- 0 |
| Urban OLSR | 5 | 0.999 +/- 0.002 | 0.002 +/- 0.001 | 0.505 +/- 0.004 s | 1.442 +/- 1.358 ms | 1.174 +/- 0.042 | 327660 +/- 0 |
| Urban Wi-Fi | 5 | 0.918 +/- 0.015 | 0.082 +/- 0.015 | 0.506 +/- 0.000 s | 0.308 +/- 0.000 ms | n/a | 15010 +/- 0 |

### urban-canyon-inside, 20 UAVs, 100 m, altitude 45.0 m

- Beste PDR: Urban OLSR
- Wenigste unbekannte AoI-Zustaende: Urban OLSR
- Niedrigste Latenz empfangener Pakete: Urban Wi-Fi

| Architektur | Runs | Steady PDR | Steady unknown AoI | Steady Avg AoI | Steady Latenz | Steady Hops | Gesamt App-Bytes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Urban OLSR | 5 | 0.875 +/- 0.047 | 0.092 +/- 0.039 | 0.689 +/- 0.111 s | 59.806 +/- 8.140 ms | 2.274 +/- 0.069 | 327660 +/- 0 |
| Urban LTE | 5 | 0.728 +/- 0.048 | 0.261 +/- 0.029 | 0.736 +/- 0.480 s | 21.550 +/- 6.060 ms | 2.000 +/- 0.000 | 326140 +/- 0 |
| Urban Wi-Fi | 5 | 0.291 +/- 0.014 | 0.709 +/- 0.014 | 0.509 +/- 0.002 s | 0.308 +/- 0.000 ms | n/a | 15010 +/- 0 |

### urban-canyon-near-roof, 20 UAVs, 100 m, altitude 65.0 m

- Beste PDR: Urban LTE
- Wenigste unbekannte AoI-Zustaende: Urban LTE
- Niedrigste Latenz empfangener Pakete: Urban Wi-Fi

| Architektur | Runs | Steady PDR | Steady unknown AoI | Steady Avg AoI | Steady Latenz | Steady Hops | Gesamt App-Bytes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Urban LTE | 5 | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.516 +/- 0.000 s | 14.551 +/- 0.002 ms | 2.000 +/- 0.000 | 326140 +/- 0 |
| Urban OLSR | 5 | 0.998 +/- 0.003 | 0.002 +/- 0.001 | 0.509 +/- 0.012 s | 2.292 +/- 2.212 ms | 1.186 +/- 0.054 | 327660 +/- 0 |
| Urban Wi-Fi | 5 | 0.917 +/- 0.015 | 0.083 +/- 0.015 | 0.506 +/- 0.001 s | 0.308 +/- 0.000 ms | n/a | 15010 +/- 0 |

### urban-open-above-roof, 20 UAVs, 100 m, altitude 80.0 m

- Beste PDR: Urban LTE
- Wenigste unbekannte AoI-Zustaende: Urban LTE
- Niedrigste Latenz empfangener Pakete: Urban Wi-Fi

| Architektur | Runs | Steady PDR | Steady unknown AoI | Steady Avg AoI | Steady Latenz | Steady Hops | Gesamt App-Bytes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Urban LTE | 5 | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.516 +/- 0.000 s | 14.550 +/- 0.000 ms | 2.000 +/- 0.000 | 271420 +/- 0 |
| Urban OLSR | 5 | 0.999 +/- 0.001 | 0.002 +/- 0.001 | 0.506 +/- 0.002 s | 1.928 +/- 1.142 ms | 1.197 +/- 0.054 | 272940 +/- 0 |
| Urban Wi-Fi | 5 | 0.914 +/- 0.013 | 0.086 +/- 0.013 | 0.506 +/- 0.000 s | 0.305 +/- 0.000 ms | n/a | 12130 +/- 0 |

### urban-open-inside, 20 UAVs, 100 m, altitude 15.0 m

- Beste PDR: Urban LTE
- Wenigste unbekannte AoI-Zustaende: Urban LTE
- Niedrigste Latenz empfangener Pakete: Urban Wi-Fi

| Architektur | Runs | Steady PDR | Steady unknown AoI | Steady Avg AoI | Steady Latenz | Steady Hops | Gesamt App-Bytes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Urban LTE | 5 | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.516 +/- 0.000 s | 14.552 +/- 0.004 ms | 2.000 +/- 0.000 | 271420 +/- 0 |
| Urban OLSR | 5 | 0.929 +/- 0.011 | 0.065 +/- 0.004 | 0.595 +/- 0.035 s | 52.937 +/- 10.763 ms | 2.285 +/- 0.081 | 272940 +/- 0 |
| Urban Wi-Fi | 5 | 0.271 +/- 0.005 | 0.729 +/- 0.005 | 0.506 +/- 0.001 s | 0.304 +/- 0.000 ms | n/a | 12130 +/- 0 |

### urban-open-near-roof, 20 UAVs, 100 m, altitude 25.0 m

- Beste PDR: Urban LTE
- Wenigste unbekannte AoI-Zustaende: Urban LTE
- Niedrigste Latenz empfangener Pakete: Urban Wi-Fi

| Architektur | Runs | Steady PDR | Steady unknown AoI | Steady Avg AoI | Steady Latenz | Steady Hops | Gesamt App-Bytes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Urban LTE | 5 | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.516 +/- 0.000 s | 14.550 +/- 0.000 ms | 2.000 +/- 0.000 | 271420 +/- 0 |
| Urban OLSR | 5 | 0.996 +/- 0.005 | 0.003 +/- 0.005 | 0.508 +/- 0.005 s | 2.661 +/- 1.924 ms | 1.191 +/- 0.050 | 272940 +/- 0 |
| Urban Wi-Fi | 5 | 0.911 +/- 0.012 | 0.089 +/- 0.012 | 0.506 +/- 0.000 s | 0.305 +/- 0.000 ms | n/a | 12130 +/- 0 |

## Hinweise zur Interpretation

- Niedrige Latenz allein reicht nicht aus, weil verlorene Pakete keine Latenz haben.
- Unknown AoI zeigt, ob ein UAV ueber andere UAVs gar keine aktuelle Information besitzt.
- App-Bytes sind nur Anwendungslast. Kontrolltraffic ist noch nicht vollstaendig enthalten.
- LTE-Hops sind Infrastruktur-Hops und nicht direkt mit OLSR-Mesh-Hops gleichzusetzen.
