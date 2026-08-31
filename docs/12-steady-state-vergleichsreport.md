# UAV Steady-state Vergleichsreport

Dieser Report wird aus `steady-state-summary.csv` Dateien erzeugt.
Er nutzt die eingeschwungene Auswertung, damit Start- und
Konvergenzphasen den Architekturvergleich weniger verzerren.

Die Rangfolge innerhalb eines Szenarios ist bewusst einfach:
zuerst hohe Packet Delivery Ratio, dann wenig unknown AoI, dann
niedrige Latenz fuer erfolgreich empfangene Pakete.

## Freier Standardlauf

Quelle: `results/uav-experiments-standard-v06/steady-state-summary.csv`

### grid, 5 UAVs, 60 m

- Beste PDR: Wi-Fi
- Wenigste unbekannte AoI-Zustaende: Wi-Fi
- Niedrigste Latenz empfangener Pakete: Wi-Fi

| Architektur | Steady PDR | Steady unknown AoI | Steady Avg AoI | Steady Latenz | Steady Hops | Gesamt App-Bytes |
| --- | --- | --- | --- | --- | --- | --- |
| Wi-Fi | 1.000 | 0.000 | 0.589 s | 0.301 ms | n/a | n/a |
| OLSR | 1.000 | 0.000 | 0.614 s | 0.312 ms | 1.000 | 2312 |
| LTE | 1.000 | 0.000 | 0.614 s | 14.700 ms | 2.000 | 2292 |

### grid, 5 UAVs, 100 m

- Beste PDR: Wi-Fi
- Wenigste unbekannte AoI-Zustaende: Wi-Fi
- Niedrigste Latenz empfangener Pakete: Wi-Fi

| Architektur | Steady PDR | Steady unknown AoI | Steady Avg AoI | Steady Latenz | Steady Hops | Gesamt App-Bytes |
| --- | --- | --- | --- | --- | --- | --- |
| Wi-Fi | 1.000 | 0.000 | 0.589 s | 0.301 ms | n/a | n/a |
| OLSR | 1.000 | 0.000 | 0.614 s | 0.312 ms | 1.000 | 2408 |
| LTE | 1.000 | 0.000 | 0.614 s | 14.700 ms | 2.000 | 2388 |

### grid, 20 UAVs, 60 m

- Beste PDR: Wi-Fi
- Wenigste unbekannte AoI-Zustaende: Wi-Fi
- Niedrigste Latenz empfangener Pakete: Wi-Fi

| Architektur | Steady PDR | Steady unknown AoI | Steady Avg AoI | Steady Latenz | Steady Hops | Gesamt App-Bytes |
| --- | --- | --- | --- | --- | --- | --- |
| Wi-Fi | 1.000 | 0.000 | 0.514 s | 0.303 ms | n/a | n/a |
| LTE | 1.000 | 0.000 | 0.525 s | 14.550 ms | 2.000 | 49116 |
| OLSR | 1.000 | 0.004 | 0.513 s | 0.483 ms | 1.000 | 49496 |

### grid, 20 UAVs, 100 m

- Beste PDR: LTE
- Wenigste unbekannte AoI-Zustaende: LTE
- Niedrigste Latenz empfangener Pakete: Wi-Fi

| Architektur | Steady PDR | Steady unknown AoI | Steady Avg AoI | Steady Latenz | Steady Hops | Gesamt App-Bytes |
| --- | --- | --- | --- | --- | --- | --- |
| LTE | 1.000 | 0.000 | 0.525 s | 14.550 ms | 2.000 | 50142 |
| OLSR | 0.994 | 0.015 | 0.517 s | 1.963 ms | 1.103 | 50522 |
| Wi-Fi | 0.895 | 0.105 | 0.514 s | 0.303 ms | n/a | n/a |

## Urbaner Standardbenchmark

Quelle: `results/uav-urban-all-v08/steady-state-summary.csv`

### grid, 5 UAVs, 60 m

- Beste PDR: Urban OLSR
- Wenigste unbekannte AoI-Zustaende: Urban OLSR
- Niedrigste Latenz empfangener Pakete: Urban Wi-Fi

| Architektur | Steady PDR | Steady unknown AoI | Steady Avg AoI | Steady Latenz | Steady Hops | Gesamt App-Bytes |
| --- | --- | --- | --- | --- | --- | --- |
| Urban OLSR | 1.000 | 0.000 | 0.614 s | 0.423 ms | 1.100 | 2480 |
| Urban LTE | 1.000 | 0.000 | 0.614 s | 14.700 ms | 2.000 | 2460 |
| Urban Wi-Fi | 0.900 | 0.100 | 0.590 s | 0.301 ms | n/a | n/a |

### grid, 5 UAVs, 100 m

- Beste PDR: Urban OLSR
- Wenigste unbekannte AoI-Zustaende: Urban OLSR
- Niedrigste Latenz empfangener Pakete: Urban Wi-Fi

| Architektur | Steady PDR | Steady unknown AoI | Steady Avg AoI | Steady Latenz | Steady Hops | Gesamt App-Bytes |
| --- | --- | --- | --- | --- | --- | --- |
| Urban OLSR | 1.000 | 0.000 | 0.614 s | 0.683 ms | 1.300 | 2504 |
| Urban LTE | 1.000 | 0.000 | 0.614 s | 14.700 ms | 2.000 | 2484 |
| Urban Wi-Fi | 0.800 | 0.200 | 0.588 s | 0.302 ms | n/a | n/a |

### grid, 20 UAVs, 60 m

- Beste PDR: Urban LTE
- Wenigste unbekannte AoI-Zustaende: Urban LTE
- Niedrigste Latenz empfangener Pakete: Urban Wi-Fi

| Architektur | Steady PDR | Steady unknown AoI | Steady Avg AoI | Steady Latenz | Steady Hops | Gesamt App-Bytes |
| --- | --- | --- | --- | --- | --- | --- |
| Urban LTE | 1.000 | 0.000 | 0.525 s | 14.550 ms | 2.000 | 50712 |
| Urban OLSR | 0.997 | 0.011 | 0.522 s | 3.547 ms | 1.352 | 51092 |
| Urban Wi-Fi | 0.811 | 0.189 | 0.514 s | 0.303 ms | n/a | n/a |

### grid, 20 UAVs, 100 m

- Beste PDR: Urban LTE
- Wenigste unbekannte AoI-Zustaende: Urban LTE
- Niedrigste Latenz empfangener Pakete: Urban Wi-Fi

| Architektur | Steady PDR | Steady unknown AoI | Steady Avg AoI | Steady Latenz | Steady Hops | Gesamt App-Bytes |
| --- | --- | --- | --- | --- | --- | --- |
| Urban LTE | 1.000 | 0.000 | 0.525 s | 14.550 ms | 2.000 | 50826 |
| Urban OLSR | 0.989 | 0.024 | 0.534 s | 9.115 ms | 1.378 | 51206 |
| Urban Wi-Fi | 0.771 | 0.229 | 0.515 s | 0.303 ms | n/a | n/a |

## Urbane Formen

Quelle: `results/uav-urban-forms-v08/steady-state-summary.csv`

### urban-baseline, 20 UAVs, 100 m

- Beste PDR: Urban LTE
- Wenigste unbekannte AoI-Zustaende: Urban LTE
- Niedrigste Latenz empfangener Pakete: Urban Wi-Fi

| Architektur | Steady PDR | Steady unknown AoI | Steady Avg AoI | Steady Latenz | Steady Hops | Gesamt App-Bytes |
| --- | --- | --- | --- | --- | --- | --- |
| Urban LTE | 1.000 | 0.000 | 0.525 s | 14.550 ms | 2.000 | 50826 |
| Urban OLSR | 0.989 | 0.024 | 0.534 s | 9.115 ms | 1.378 | 51206 |
| Urban Wi-Fi | 0.771 | 0.229 | 0.515 s | 0.303 ms | n/a | n/a |

### urban-canyon, 20 UAVs, 100 m

- Beste PDR: Urban LTE
- Wenigste unbekannte AoI-Zustaende: Urban LTE
- Niedrigste Latenz empfangener Pakete: Urban Wi-Fi

| Architektur | Steady PDR | Steady unknown AoI | Steady Avg AoI | Steady Latenz | Steady Hops | Gesamt App-Bytes |
| --- | --- | --- | --- | --- | --- | --- |
| Urban LTE | 1.000 | 0.000 | 0.525 s | 14.550 ms | 2.000 | 62112 |
| Urban OLSR | 0.970 | 0.038 | 0.545 s | 20.369 ms | 1.448 | 62492 |
| Urban Wi-Fi | 0.732 | 0.268 | 0.514 s | 0.307 ms | n/a | n/a |

### urban-open, 20 UAVs, 100 m

- Beste PDR: Urban LTE
- Wenigste unbekannte AoI-Zustaende: Urban LTE
- Niedrigste Latenz empfangener Pakete: Urban Wi-Fi

| Architektur | Steady PDR | Steady unknown AoI | Steady Avg AoI | Steady Latenz | Steady Hops | Gesamt App-Bytes |
| --- | --- | --- | --- | --- | --- | --- |
| Urban LTE | 1.000 | 0.000 | 0.525 s | 14.550 ms | 2.000 | 51168 |
| Urban OLSR | 0.981 | 0.031 | 0.540 s | 16.131 ms | 1.482 | 51548 |
| Urban Wi-Fi | 0.729 | 0.271 | 0.514 s | 0.304 ms | n/a | n/a |

## Hinweise zur Interpretation

- Niedrige Latenz allein reicht nicht aus, weil verlorene Pakete keine Latenz haben.
- Unknown AoI zeigt, ob ein UAV ueber andere UAVs gar keine aktuelle Information besitzt.
- App-Bytes sind nur Anwendungslast. Kontrolltraffic ist noch nicht vollstaendig enthalten.
- LTE-Hops sind Infrastruktur-Hops und nicht direkt mit OLSR-Mesh-Hops gleichzusetzen.
