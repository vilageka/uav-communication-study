# Version 09: Konzeption des Architektur- und Szenariovergleichs

## Einordnung

Der aktuelle Projektstand enthaelt genug Szenarien fuer einen ersten
sinnvollen Architekturvergleich. Er ist noch nicht der finale Umfang der
Bachelorarbeit, aber die methodische Basis ist jetzt breit genug, um Ergebnisse
systematisch zu interpretieren:

- Es gibt mehrere Kommunikationsarchitekturen.
- Es gibt freie und urbane Umgebungen.
- Es gibt Variation in UAV-Anzahl und Abstand.
- Es gibt mehrere Metriken statt nur Paketverlust oder Latenz.

Damit koennen erste belastbare Aussagen entstehen. Weitere Szenarien sollten ab
jetzt gezielt aus konkreten Forschungsfragen entstehen, nicht mehr nur als
blinde Erweiterung.

## Forschungsziel

Ziel der Simulation ist der Vergleich von Kommunikationsarchitekturen fuer
kooperative UAVs. Die zentrale Frage lautet:

```text
Welche Kommunikationsarchitektur liefert unter welchen Szenariobedingungen
zuverlaessige, aktuelle und skalierbare Positionsinformationen?
```

Positionsinformationen sind hier beispielhaft fuer kooperative
Situational-Awareness-Daten. Jedes UAV sendet periodisch seinen Zustand, und
andere UAVs sollen moeglichst viele dieser Informationen moeglichst aktuell
kennen.

## Betrachtete Architekturen

### Wi-Fi Ad-hoc Broadcast

Wi-Fi Ad-hoc Broadcast dient als einfache ADS-L-inspirierte Referenz. Jedes UAV
sendet ein Positionsupdate als Broadcast. Alle direkten Funknachbarn koennen
dieses Update empfangen.

Staerken:

- sehr geringe Latenz fuer erfolgreich empfangene Pakete
- geringer Anwendungstraffic, weil pro Sender nur ein Broadcast noetig ist
- einfaches Modell und gut als Referenz geeignet

Schwaechen:

- keine Weiterleitung ueber mehrere Hops
- Empfang haengt stark von direkter Funkreichweite ab
- in urbanen oder duenneren Szenarien entstehen schnell Informationsluecken

### Wi-Fi/802.11 OLSR-Mesh

Das OLSR-Mesh nutzt weiterhin Wi-Fi Ad-hoc als Funkbasis, fuegt aber proaktives
Routing hinzu. Positionsupdates werden nicht gebroadcastet, sondern als
UDP-Unicast von jedem UAV an jedes andere UAV gesendet.

Staerken:

- kann entfernte UAVs ueber Zwischenknoten erreichen
- Hop-Anzahl ist als Mesh-Metrik auswertbar
- geeignet, um Multi-Hop-Kommunikation gegen einfachen Broadcast zu vergleichen

Schwaechen:

- mehr Anwendungstraffic, weil jedes gerichtete UAV-Paar ein eigenes Update
  bekommt
- Routing erzeugt zusaetzliche Kontrolllast
- hoehere Latenzen, besonders bei Routenaufbau, Routenwechseln oder vielen
  Flows

### LTE/EPC-Infrastruktur

Die LTE-Architektur modelliert UAVs als LTE User Equipment. Eine zentrale
eNodeB versorgt den Schwarm, und das EPC stellt die IP-Infrastruktur bereit.

Staerken:

- hohe Erreichbarkeit im vereinfachten Ein-Zellen-Modell
- stabile mittlere Latenz in den bisherigen Szenarien
- weniger abhaengig von direkter UAV-zu-UAV-Nachbarschaft

Schwaechen:

- Infrastruktur ist Voraussetzung
- aktuelle Simulation betrachtet noch keine Zelluebergaben
- Interferenz, Netzlast und Kontrollsignalisierung sind noch vereinfacht

## Szenarioebenen

### Freies Gitter

Die ersten Szenarien verwenden statische UAV-Positionen auf einem Gitter. Diese
Szenarien sind bewusst einfach. Sie dienen als Kontrollgruppe, weil dort der
Einfluss von Gebaeuden und Stadtstruktur fehlt.

Nutzen:

- Basisvergleich der reinen Architektur
- einfache Variation von UAV-Anzahl und Abstand
- schnelle Reproduzierbarkeit

### Urbaner Benchmark

Die urbanen Szenarien verwenden rechteckige Gebaeudebloecke und
Strassenkorridore. UAVs stehen deterministisch auf Strassenachsen. Der
Funkkanal nutzt `HybridBuildingsPropagationLossModel`.

Nutzen:

- urbaner Pfadverlust und Gebaeudeinformationen gehen in die Simulation ein
- alle Architekturen sehen dieselbe Stadtgeometrie
- Gebaeudeparameter sind kontrollierbar

### Urbane Formen

Das Profil `urban-forms` vergleicht drei Stadttypen:

| Szenario | Idee |
| --- | --- |
| `urban-open` | offene Stadtstruktur mit breiteren Strassen und niedrigeren Gebaeuden |
| `urban-baseline` | mittleres Referenzszenario |
| `urban-canyon` | engere Strassen, hoehere Gebaeude, mehr Bloecke |

Diese Formen sind sinnvoll, weil sie nicht nur UAV-Abstand variieren, sondern
auch den Charakter der Umgebung.

## Metriken

### Packet Delivery Ratio

Die Packet Delivery Ratio misst, welcher Anteil der erwarteten Updates
tatsaechlich empfangen wurde. Sie beantwortet:

```text
Kommt die Information ueberhaupt an?
```

Bei Broadcast ist die erwartete Empfangszahl:

```text
gesendete Broadcasts * (numUavs - 1)
```

Bei OLSR und LTE ist jedes gesendete Unicast-Paket bereits fuer genau einen
Empfaenger bestimmt.

### Ende-zu-Ende-Latenz

Die Latenz misst die Zeit zwischen Erzeugung und Empfang eines erfolgreich
angekommenen Updates. Sie beantwortet:

```text
Wie schnell sind empfangene Informationen?
```

Wichtig ist: Latenz betrachtet nur empfangene Pakete. Verlorene Pakete tauchen
in dieser Metrik nicht als schlechte Werte auf.

### Age of Information

Age of Information misst, wie alt die aktuell bekannte Information ueber ein
anderes UAV ist:

```text
AoI(receiver, sender, t) = t - generationTime(lastUpdate)
```

Wenn ein Empfaenger noch nie ein Update eines Senders erhalten hat, wird der
Zustand als unknown gespeichert. Deshalb ist der Anteil unbekannter AoI-Zustaende
eine wichtige Zusatzmetrik.

AoI beantwortet:

```text
Wie frisch ist das Lagebild eines UAVs ueber den restlichen Schwarm?
```

### Kommunikationsaufwand

Der Kommunikationsaufwand wird derzeit auf Anwendungsebene ueber Pakete und
Bytes angenaehert. Bei OLSR und LTE werden Anwendungspakete und Payload-Bytes
explizit ausgegeben. Bei Wi-Fi Broadcast steht aktuell vor allem die Paketzahl
im Vordergrund.

Fuer die finale Arbeit sollte diese Metrik erweitert werden:

- OLSR-Kontrollpakete
- LTE-Kontrollsignalisierung
- eventuell MAC-/PHY-Overhead

### Hop-Anzahl

Bei OLSR wird die Hop-Anzahl ueber die empfangene IPv4-TTL geschaetzt. Ein
direkter Empfang hat Hop-Anzahl 1, eine Weiterleitung ueber einen Zwischenknoten
hat Hop-Anzahl 2.

Bei LTE ist die Hop-Anzahl eine Infrastruktur-Hop-Schaetzung und nicht direkt
mit Mesh-Hops gleichzusetzen.

### Skalierbarkeit

Skalierbarkeit wird ueber die Variation von UAV-Anzahl und Abstand betrachtet.
Aktuell sind besonders diese Punkte wichtig:

- 5 UAVs als kleines Referenzszenario
- 20 UAVs als erster groesserer Schwarm
- 40 UAVs im Full-Profil fuer spaetere Skalierungslaeufe
- 60 m als dichteres Szenario
- 100 m als mittleres Distanzszenario
- 160 m im Full-Profil als sparse case

## Warum der aktuelle Szenarioumfang reicht

Der aktuelle Stand deckt die wesentlichen Vergleichsdimensionen ab:

| Dimension | Abgedeckt durch |
| --- | --- |
| Architektur | Wi-Fi Broadcast, OLSR-Mesh, LTE-Infrastruktur |
| Umgebung | freies Gitter, urbaner Benchmark |
| Stadtform | open, baseline, canyon |
| Schwarmgroesse | 5, 20, optional 40 UAVs |
| Abstand | 60 m, 100 m, optional 160 m |
| Frische | AoI und unknown AoI share |
| Zuverlaessigkeit | PDR |
| Performance | Latenz |
| Topologie | Hop-Anzahl |

Damit ist die Basis fuer einen sinnvollen Vergleich vorhanden. Mehr Szenarien
waeren erst dann sinnvoll, wenn sie gezielt eine offene Frage beantworten, etwa:

- Wie verhalten sich die Architekturen bei Bewegung?
- Wie stark beeinflusst UAV-Flughoehe den urbanen Pfadverlust?
- Wie stark skaliert die Kommunikationslast bei 40 oder 60 UAVs?
- Wie veraendert ein zweiter LTE-Standort das Ergebnis?

## Aktuelle Limitationen

Die Simulationen enthalten noch keine UAV-Bewegung. Alle UAVs stehen statisch.
Das ist fuer den ersten Architekturvergleich akzeptabel, muss in der Arbeit
aber klar benannt werden.

Die urbanen Szenarien sind synthetisch. Sie bilden keine reale Stadtkarte ab,
sondern einen kontrollierbaren Benchmark.

AoI wird aktuell inklusive Startphase gemessen. Besonders OLSR wird dadurch
benachteiligt, weil es eine Konvergenzphase hat. Eine steady-state Auswertung
nach `appStart` sollte als naechster methodischer Schritt ergaenzt werden.

Der Kommunikationsaufwand ist noch nicht vollstaendig, weil Kontrolltraffic
noch nicht umfassend ausgewertet wird.

## Empfohlene weitere Arbeit

Nicht sofort mehr Szenarien bauen, sondern zuerst die Auswertung schaerfen:

1. Steady-state AoI nach Einschwingphase berechnen.
2. Kommunikationsaufwand inklusive Kontrolltraffic erweitern.
3. Full-Profil mit 40 UAVs und 160 m fuer ausgewaehlte Architekturen laufen
   lassen.
4. Danach Bewegungsmodelle als neue Szenariofamilie ergaenzen.
