# Ueberarbeitete Konzeption des Architekturvergleichs

## Zielsetzung

Ziel der Simulation ist der vergleichende Entwurf und die Bewertung
unterschiedlicher Kommunikationsarchitekturen fuer kooperative UAV-Schwaerme.
Im Mittelpunkt steht nicht die detaillierte Nachbildung eines konkreten
Stadtgebiets, sondern der methodisch nachvollziehbare Vergleich mehrerer
Kommunikationsansaetze unter kontrollierten und reproduzierbaren Bedingungen.

Verglichen werden drei Architekturen:

- eine direkte Wi-Fi-Ad-hoc-Kommunikation als vereinfachte
  ADS-L-inspirierte Referenz,
- ein mehrstufiger Wi-Fi/802.11-Mesh-Ansatz mit OLSR-Routing,
- eine LTE/EPC-basierte Infrastrukturarchitektur.

Diese drei Architekturen werden in zwei Umgebungsklassen betrachtet:
freiraumartige Gitterpositionierung und synthetische urbane Szenarien. Die
urbanen Varianten sind dabei keine zusaetzlichen Architekturen, sondern
Umgebungs- und Positionierungsvarianten derselben drei Architekturen.

## Kommunikationsarchitekturen

Die Wi-Fi-Ad-hoc-Referenz modelliert einen sehr einfachen
Broadcast-basierten Informationsaustausch. Jedes UAV sendet periodisch
Zustandsinformationen an die direkte Funknachbarschaft. Diese Architektur ist
leichtgewichtig, benoetigt keine Infrastruktur und erzeugt nur geringen
Anwendungstraffic. Ihre wesentliche Einschraenkung besteht darin, dass
Informationen nur dann ankommen, wenn Sender und Empfaenger direkt erreichbar
sind.

Der OLSR-Mesh-Ansatz erweitert das Wi-Fi-basierte Szenario um
mehrstufige Weiterleitung. UAVs koennen Informationen ueber andere UAVs
erreichen, sofern eine Route im Mesh existiert. Dadurch kann die
Erreichbarkeit gegenueber direktem Broadcast steigen. Gleichzeitig entstehen
Routing-Overhead, zusaetzliche Weiterleitungen und potenziell hoehere
Latenzen.

Die LTE/EPC-Architektur verwendet eine zentrale Infrastruktur. Die UAVs
kommunizieren ueber eine Basisstation und das EPC-Kernnetz. Diese Architektur
ist besonders geeignet, um eine infrastrukturbasierte Referenz mit hoher
Zuverlaessigkeit abzubilden. Sie ist jedoch konzeptionell von Netzabdeckung,
Infrastrukturverfuegbarkeit und Scheduling-Mechanismen abhaengig.

## Szenarioebenen

Die freiraumartigen Szenarien positionieren UAVs auf einem regelmaessigen
Gitter. Dort beschreibt der Parameter `spacing` tatsaechlich den Abstand
benachbarter Gitterpunkte. Diese Szenarien eignen sich besonders gut, um
Skalierungseffekte mit steigender UAV-Anzahl und wachsender raeumlicher
Ausdehnung zu untersuchen.

Die urbanen Szenarien verwenden eine synthetische Stadtgeometrie mit
Gebaeuden und Strassenkorridoren. Dabei existieren drei Formen:
`urban-open`, `urban-baseline` und `urban-canyon`. Sie unterscheiden sich in
Gebaeudehoehe, Strassenbreite und Blockstruktur. In diesen Szenarien beschreibt
`spacing` nicht den allgemeinen Abstand zwischen den UAVs, sondern das
Platzierungsintervall entlang der Strassenkorridore. Aus diesem Grund werden
fuer jeden Lauf zusaetzlich reale Abstandsmetriken berechnet: minimaler
Paarabstand, mittlerer Paarabstand, mittlerer Abstand zum naechsten Nachbarn
und maximaler Paarabstand.

## Urbane Ausbreitungsmodellierung

Die urbanen Szenarien verwenden in ns-3 ein parametrisiertes
Gebaeude-Ausbreitungsmodell. Die erzeugte Stadtgeometrie ist deshalb nicht
als exaktes Hindernismodell zu verstehen. Insbesondere wird fuer
Outdoor-zu-Outdoor-Verbindungen nicht fuer jedes UAV-Paar geometrisch
geprueft, ob ein Gebaeude die direkte Sichtlinie schneidet. Stattdessen
werden urbane Ausbreitungsbedingungen ueber empirische beziehungsweise
parametrisierte Pfadverlustmodelle angenaehert.

Die korrekte Formulierung fuer die Arbeit lautet daher: Das Szenario bildet
urbane Ausbreitungsbedingungen mithilfe eines parametrisierten
Pfadverlustmodells und einer synthetischen Stadtgeometrie ab. Die Aussage,
ein einzelnes Gebaeude schatte eine konkrete Verbindung ab, waere fuer dieses
Modell zu stark.

Ein weiterer wichtiger Parameter ist die relative Hoehe der UAVs zur
Gebaeudestruktur. In den aktuellen urbanen Laeufen fliegen die UAVs auf
80 m. Die Gebaeudehoehen liegen bei 20 m, 35 m und 60 m. Damit operieren die
UAVs in allen drei urbanen Formen oberhalb der Daecher. Das ist fuer UAVs
plausibel, reduziert aber den Einfluss klassischer Stadtschluchten. Fuer eine
spaetere Vertiefung ist daher eine systematische Variation der Flughoehe
gegenueber der Gebaeudehoehe sinnvoll.

## Bewertungsmetriken

Die Architekturen werden anhand folgender Metriken verglichen:

- Packet Delivery Ratio als Anteil erfolgreich zugestellter
  Informationsbeziehungen,
- Ende-zu-Ende-Latenz als Zeit zwischen Sendung und Empfang eines Updates,
- Age of Information als Aktualitaetsmass fuer bekannte UAV-Zustaende,
- unbekannter AoI-Anteil als Anteil der Sender-Empfaenger-Beziehungen ohne
  aktuelle Information,
- Kommunikationsaufwand ueber gesendete und empfangene Anwendungsbytes,
- Hop-Anzahl fuer mehrstufige beziehungsweise infrastrukturbasierte
  Kommunikation,
- Skalierbarkeit durch Variation von UAV-Anzahl, raeumlicher Ausdehnung und
  Szenarioform.

Age of Information wird dabei nicht nur als Latenzersatz verwendet. Die
Metrik beschreibt, wie alt die zuletzt bekannte Information ueber ein anderes
UAV ist. Eine Architektur kann geringe Latenz fuer empfangene Pakete
besitzen und trotzdem einen schlechten AoI-Wert zeigen, wenn viele
Sender-Empfaenger-Paare nie oder selten erreicht werden.

## Methodische Grenzen

Die aktuelle Simulationsbasis ist bewusst vergleichend und kontrolliert
aufgebaut. Daraus ergeben sich mehrere Grenzen, die in der Arbeit transparent
genannt werden muessen.

Erstens bilden die urbanen Gebaeude keine exakte geometrische Abschattung ab.
Zweitens fliegen die UAVs in den aktuellen urbanen Profilen oberhalb der
Gebaeude, wodurch starke Canyon-Effekte abgeschwaecht werden. Drittens ist
die Nutzlast aktuell textkodiert und zwischen Broadcast- und
Unicast-Varianten strukturell nicht vollkommen identisch. Die gemessenen
Bytewerte sind daher als architekturspezifischer Anwendungstraffic dieser
Implementierung zu verstehen und nicht als isolierter Vergleich eines
standardisierten Nutzdatenblocks.

Viertens bestehen die aktuellen urbanen Auswertungen aus fuenf
Wiederholungen pro Konfiguration. Das reicht fuer eine erste stabile
Tendenz, ersetzt aber keine umfangreiche statistische Kampagne. Deshalb
werden Mittelwert und Standardabweichung berichtet; die zusaetzlichen
95-Prozent-Intervalle dienen als Orientierung.

## Erwartete Vergleichsaussagen

Auf Basis der Konzeption werden folgende qualitativen Erwartungen geprueft:

- Direkter Wi-Fi-Ad-hoc-Broadcast ist schnell und effizient, aber bei
  wachsender raeumlicher Ausdehnung weniger vollstaendig.
- OLSR-Mesh verbessert die Erreichbarkeit und den AoI-Zustand, verursacht
  aber mehr Kommunikationsaufwand und hoehere Latenz.
- LTE erreicht im Ein-Zellen-Modell eine sehr hohe Zustellrate, zeigt aber
  eine infrastrukturseitig gepraegte Latenz und ist von Netzabdeckung
  abhaengig.
- Urbane Szenarioformen beeinflussen die Ergebnisse nicht isoliert ueber
  Gebaeudehoehen, sondern durch das Zusammenspiel von Platzierung,
  tatsaechlichen Paarabstaenden, Modellparametern und Ausbreitungsmodell.
