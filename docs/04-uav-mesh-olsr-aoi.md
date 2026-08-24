# Version 04: Wi-Fi Mesh With OLSR and AoI

## Programm

`scratch/uav-mesh-olsr-aoi.cc`

## Ziel

Diese Version implementiert die zweite Kommunikationsarchitektur der Arbeit:
einen mehrstufigen Mesh-Ansatz auf Basis von 802.11/Wi-Fi Ad-hoc und OLSR.

Der wichtigste Unterschied zur ADS-L-inspirierten Broadcast-Referenz ist die
Weiterleitung ueber Zwischen-UAVs. Wenn Sender und Empfaenger nicht direkt in
Funkreichweite sind, kann OLSR eine Route ueber andere UAVs bereitstellen.

## Modell

- UAVs als ns-3 Nodes
- statische Positionen auf einem 2D-Gitter mit konstanter Hoehe
- Wi-Fi Ad-hoc als Funkbasis
- OLSR als proaktives IPv4-Routing-Protokoll
- UDP-Unicast von jedem UAV an jedes andere UAV
- periodische Positionsupdates
- Age-of-Information-Sampling fuer alle gerichteten Sender-Empfaenger-Paare

## Vergleich zur Broadcast-Referenz

Die Broadcast-Referenz `uav-wifi-aoi.cc` sendet pro UAV und Intervall ein
einziges Paket an die lokale Broadcast-Adresse. Das ist effizient, aber auf
direkte Funkreichweite begrenzt.

Die OLSR-Version sendet pro UAV und Intervall ein eigenes Unicast-Paket an
jedes andere UAV. Fuer `N` UAVs entstehen pro Update-Runde also:

```text
N * (N - 1)
```

Anwendungspakete. Dieser Ansatz erzeugt deutlich mehr Kommunikationsaufwand,
kann dafuer aber Multi-Hop-Routen nutzen.

## Metriken

Diese Version misst:

- Packet Delivery Ratio
- Ende-zu-Ende-Latenz
- Age of Information
- Anwendungspakete und Anwendungsbytes als erster Kommunikationsaufwand
- Hop-Anzahl ueber IPv4-TTL-Tags

Die Hop-Anzahl wird als Naeherung berechnet:

```text
hopCount = initialTtl - remainingTtl + 1
```

Ein direkt empfangenes Paket hat damit `hopCount=1`; ein Paket ueber einen
Zwischen-UAV hat `hopCount=2`.

## Startbeispiele

```bash
./ns3 run "uav-mesh-olsr-aoi --numUavs=5 --simTime=3 --appStart=3"
./ns3 run "uav-mesh-olsr-aoi --numUavs=20 --simTime=5 --appStart=5 --spacing=100"
```

`appStart` gibt OLSR vor dem ersten Anwendungspaket Zeit, Routen aufzubauen.
Das ist wichtig, weil OLSR ein proaktives Routing-Protokoll ist und seine
Nachbarschafts- und Topologieinformationen erst austauschen muss.

## CSV-Ausgaben

Empfangene Updates:

```csv
receive_time_s,send_time_s,sender_id,receiver_id,sequence,latency_ms,hop_count,payload_bytes,x_m,y_m,z_m
```

AoI-Samples:

```csv
time_s,receiver_id,sender_id,known,aoi_s
```

## Erste Messergebnisse

### 5 UAVs, 100 m Abstand

Startbefehl:

```bash
./ns3 run "uav-mesh-olsr-aoi --numUavs=5 --simTime=3 --appStart=3 --updateInterval=1 --aoiSampleInterval=0.2 --spacing=100"
```

Ergebnis:

| Metrik | Wert |
| --- | ---: |
| Gesendete Anwendungspakete | 60 |
| Empfangene Anwendungspakete | 60 / 60 |
| Delivery Ratio | 1.000 |
| Anwendungsbytes gesendet | 1194 |
| Durchschnittliche Latenz | 3.116 ms |
| Max Latenz | 20.706 ms |
| Durchschnittliche Hop-Anzahl | 1.000 |
| Max Hop-Anzahl | 1 |
| Durchschnittliche bekannte AoI | 0.777 s |
| Max bekannte AoI | 1.799 s |

Interpretation: Bei 5 UAVs sind alle betrachteten Paare direkt erreichbar.
OLSR wird zwar verwendet, aber es entstehen keine Multi-Hop-Pfade. Die
Latenz ist hoeher als beim reinen Broadcast, weil hier Unicast und Routing
genutzt werden.

### 20 UAVs, 100 m Abstand

Startbefehl:

```bash
./ns3 run "uav-mesh-olsr-aoi --numUavs=20 --simTime=5 --appStart=5 --updateInterval=1 --aoiSampleInterval=0.1 --spacing=100"
```

Ergebnis:

| Metrik | Wert |
| --- | ---: |
| Gesendete Anwendungspakete | 1900 |
| Empfangene Anwendungspakete | 1862 / 1900 |
| Delivery Ratio | 0.980 |
| Anwendungsbytes gesendet | 41785 |
| Anwendungsbytes empfangen | 40965 |
| Durchschnittliche Latenz | 85.341 ms |
| Max Latenz | 2986.699 ms |
| Durchschnittliche Hop-Anzahl | 1.092 |
| Min Hop-Anzahl | 1 |
| Max Hop-Anzahl | 2 |
| Durchschnittliche bekannte AoI | 0.640 s |
| Max bekannte AoI | 2.999 s |

Hop-Verteilung:

| Hop-Anzahl | Empfangene Pakete |
| --- | ---: |
| 1 | 1690 |
| 2 | 172 |

AoI nach der Startphase:

| Betrachtung | Wert |
| --- | ---: |
| Unknown Share ab `t >= 6.5s` | 0.015 |

## Auswertung

Der 20-UAV-Fall zeigt den eigentlichen Nutzen des Mesh-Ansatzes. In der
Broadcast-AoI-Baseline bei 20 UAVs und 100 m Abstand lag die Delivery Ratio
bei etwa 0.895, und nach der Startphase blieben etwa 10.5 Prozent der
gerichteten Informationsbeziehungen unbekannt.

Mit OLSR steigt die Delivery Ratio im gleichen Positionsszenario auf 0.980.
Nach der Startphase sinkt der Unknown Share auf etwa 1.5 Prozent. Gleichzeitig
zeigen die TTL-basierten Hop-Daten, dass ein Teil der Pakete tatsaechlich ueber
zwei Hops empfangen wurde.

Der Gewinn hat aber Kosten:

- Die Anwendung sendet bei 20 UAVs 1900 Unicast-Pakete statt 100 Broadcast-
  Pakete.
- Die durchschnittliche Latenz steigt von Sub-Millisekunden-Bereich bei
  direktem Broadcast auf etwa 85 ms.
- Einzelne Pakete erfahren sehr hohe Latenzen bis knapp 3 s, vermutlich durch
  Routing-/MAC-Wartezeiten oder Routenstabilisierung.

## Bedeutung fuer die Arbeit

OLSR-Mesh ist ein sinnvoller mehrstufiger Vergleich zur ADS-L-inspirierten
Broadcast-Referenz. Es verbessert Reichweite und Informationsabdeckung, kostet
aber deutlich mehr Kommunikationsaufwand und Latenz.

Damit stuetzt diese Version die zentrale Vergleichslogik:

```text
Broadcast:
  wenig Aufwand, sehr niedrige Latenz fuer direkte Nachbarn,
  aber Reichweitenluecken.

OLSR Mesh:
  bessere Abdeckung durch Multi-Hop,
  aber mehr Unicast-Traffic und hoehere Latenz.
```

Als naechster Schritt sollte der Kommunikationsaufwand genauer gemessen werden,
z.B. inklusive OLSR-Kontrollpaketen auf IP- oder MAC-Ebene.
