# Ueberarbeitete Implementierung der ns-3 Simulationen

## Struktur der Implementierung

Die Simulationen sind als ns-3 Scratch-Programme umgesetzt. Dieser Ansatz
erlaubt eine schnelle Iteration, ohne ein eigenes ns-3 Modul anzulegen. Die
Programme liegen im Verzeichnis `scratch/`, die begleitenden
Experiment- und Auswertungsskripte im Verzeichnis `scripts/`.

Die wichtigsten Simulationsprogramme sind:

- `uav-wifi-aoi.cc` fuer Wi-Fi-Ad-hoc-Broadcast im freien Szenario,
- `uav-mesh-olsr-aoi.cc` fuer den OLSR-basierten Mesh-Ansatz,
- `uav-lte-infrastructure-aoi.cc` fuer die LTE/EPC-Infrastruktur,
- `uav-urban-wifi-aoi.cc` fuer die urbane Wi-Fi-Ad-hoc-Variante,
- `uav-urban-mesh-olsr-aoi.cc` fuer die urbane OLSR-Mesh-Variante,
- `uav-urban-lte-infrastructure-aoi.cc` fuer die urbane LTE-Variante.

Damit existieren nicht sechs verschiedene Architekturen, sondern drei
Architekturen mit freien und urbanen Szenarioauspraegungen.

## Gemeinsames UAV-Modell

Alle Programme modellieren UAVs als ns-3 Nodes mit statischer Positionierung.
Die Beweglichkeit wird ueber das Mobility-Modul bereitgestellt, aktuell aber
als konstante Position verwendet. Dadurch lassen sich Kommunikationsverhalten,
Reichweite, Routing und Infrastrukturwirkung isoliert untersuchen, ohne dass
Bewegungsdynamik die Interpretation der ersten Ergebnisse ueberlagert.

Die UAVs senden periodisch Zustandsupdates. Ein Update enthaelt mindestens
die Senderkennung, eine Sequenznummer und den Sendezeitpunkt. Aus diesen
Informationen werden Zustellung, Latenz und Age of Information berechnet.

## Wi-Fi-Ad-hoc-Implementierung

Die direkte Referenzarchitektur verwendet Wi-Fi im Ad-hoc-Modus. Jedes UAV
sendet periodische UDP-Broadcasts. Empfaenger werten eingehende Pakete direkt
aus. Eine Weiterleitung ueber andere UAVs findet nicht statt.

Diese Implementierung misst die Staerke direkter Nachbarschaftskommunikation.
Sie ist bewusst einfach gehalten und eignet sich als vereinfachte
ADS-L-inspirierte Referenz, weil sie ohne Infrastruktur und ohne Routing
arbeitet. Ihre Einschraenkung ist ebenfalls direkt sichtbar: Entfernte oder
ausbreitungstechnisch unguenstige UAV-Paare erhalten keine Updates.

## OLSR-Mesh-Implementierung

Der Mesh-Ansatz verwendet ebenfalls Wi-Fi als Funktechnologie, ergaenzt
diese aber um IP-Routing mit OLSR. Nach der Initialisierung kann ein UAV
Informationen an andere UAVs senden, auch wenn diese nicht in direkter
Funkreichweite liegen, solange eine Route ueber Zwischenknoten existiert.

Die Anwendung verwendet UDP-Unicast-Kommunikation zwischen UAV-Paaren. Fuer
empfangene Pakete wird die Hop-Anzahl aus der logischen Differenz der
IP-TTL-Werte abgeleitet. Dadurch kann der Mehrstufencharakter des Meshes in
der Auswertung sichtbar gemacht werden.

OLSR benoetigt eine Einschwingphase, damit Routingtabellen aufgebaut werden
koennen. Die Experimentauswertung beruecksichtigt dies, indem der
Steady-State-Anteil separat ausgewertet wird.

## LTE/EPC-Implementierung

Die LTE-Architektur verwendet das ns-3 LTE/EPC-Modell. UAVs werden als
UE-Knoten modelliert und kommunizieren ueber eine eNodeB und das EPC. Damit
entsteht eine infrastrukturbasierte Kommunikationsform, bei der UAVs nicht
direkt miteinander verbunden sein muessen.

Die Hop-Anzahl wird fuer diese Architektur als logische Zwei-Hop-Struktur
interpretiert: UAV zur Infrastruktur und Infrastruktur zum Ziel-UAV. Die
gemessene Latenz enthaelt damit die durch das LTE/EPC-Modell verursachte
Scheduling- und Infrastrukturwirkung.

## Urbane Varianten

Die urbanen Varianten verwenden eine synthetische Stadtgeometrie mit
Gebaeuden und Strassenkorridoren. Fuer Wi-Fi und OLSR wird das
`HybridBuildingsPropagationLossModel` ueber den Yans-Wi-Fi-Kanal genutzt.
Da die Wi-Fi-Konfiguration 802.11b mit `DsssRate11Mbps` verwendet, wurde die
Modellfrequenz in den urbanen Wi-Fi- und OLSR-Programmen auf 2.4 GHz
gesetzt. Damit ist die Frequenz mit dem verwendeten Wi-Fi-Standard
konsistent.

Fuer LTE wird das Gebaeude-Ausbreitungsmodell ueber den LTE-Helper in die
Funkstrecke eingebunden. Die LTE-Frequenz wird dabei separat durch die
LTE-Konfiguration bestimmt und nicht an die 802.11b-Parametrisierung der
Wi-Fi-Programme gekoppelt.

Wichtig ist die methodische Interpretation: Die Gebaeude werden nicht als
exaktes Raytracing-Hindernismodell verwendet. Die Simulation bildet urbane
Ausbreitungsbedingungen parametrisiert ab. Deshalb vermeidet die Auswertung
Formulierungen, die einzelne Gebaeude als konkrete Abschattung einer
bestimmten Verbindung beschreiben.

## Positions- und Abstandsausgabe

In den freien Gitter-Szenarien ist `spacing` der Abstand benachbarter
Gitterpunkte. In den urbanen Szenarien erzeugt derselbe Parameter dagegen
moegliche Platzierungen entlang der Strassenkorridore. Dadurch koennen reale
Paarabstaende entstehen, die deutlich vom eingestellten Wert abweichen.

Zur besseren Nachvollziehbarkeit berechnen die urbanen Programme deshalb
zusaetzlich vier Positionsmetriken:

- minimaler Paarabstand,
- mittlerer Paarabstand,
- mittlerer Abstand zum naechsten Nachbarn,
- maximaler Paarabstand.

Diese Werte werden am Ende jeder Simulation ausgegeben und durch das
Experiment-Skript in die Ergebnis-CSV uebernommen. Damit kann die Auswertung
unterscheiden, ob ein Ergebnis durch das urbane Profil selbst oder durch eine
dichtere beziehungsweise weiter gestreckte tatsaechliche UAV-Verteilung
erklaert werden muss.

## AoI-Messung

Age of Information wird pro Sender-Empfaenger-Beziehung berechnet. Sobald ein
Empfaenger ein gueltiges Update eines anderen UAVs erhalten hat, speichert er
den Sendezeitpunkt dieses Updates. Zu festgelegten Abtastzeitpunkten wird
berechnet, wie alt die zuletzt bekannte Information ist.

Die Implementierung schuetzt gegen veraltete oder aus der Reihenfolge
geratene Updates. Ein empfangenes Paket aktualisiert den AoI-Zustand nur dann,
wenn es neuer ist als der bisher gespeicherte Zustand derselben
Sender-Empfaenger-Beziehung. Dadurch wird verhindert, dass spaeter
eintreffende alte Pakete die Informationsqualitaet kuenstlich verschlechtern.

Zusaetzlich wird der Anteil unbekannter AoI-Zustaende berichtet. Dieser Wert
ist besonders wichtig, weil er zeigt, fuer welche UAV-Paare noch nie ein
aktuelles Update empfangen wurde. Eine geringe Latenz allein waere ohne
diese Metrik irrefuehrend, wenn nur ein Teil der Beziehungen ueberhaupt
Pakete empfaengt.

## Experimentautomatisierung

Das Skript `scripts/uav-run-experiments.py` fuehrt reproduzierbare
Experimentmatrizen aus. Es variiert Architektur, Szenario, UAV-Anzahl,
Platzierungsparameter und Zufallslauf. Jeder Lauf wird mit einem eigenen
`rngRun` gestartet, sodass mehrere Wiederholungen derselben Konfiguration
statistisch aggregiert werden koennen.

Die Rohzusammenfassungen werden in `summary.csv` geschrieben. Anschliessend
erzeugt `scripts/uav-analyze-results.py` beziehungsweise die integrierte
Steady-State-Auswertung eine bereinigte Sicht auf den relevanten
Messzeitraum. `scripts/uav-aggregate-results.py` aggregiert mehrere
Wiederholungen zu Mittelwert, Standardabweichung und einem 95-Prozent-Intervall.
Fuer kleine Stichproben wird dabei ein Student-t-Faktor verwendet.

Mit `scripts/uav-build-report.py` koennen aus den CSV-Dateien
Markdown-Berichte erzeugt werden. Diese Berichte dienen der schnellen
Kontrolle der Ergebnisse. Die wissenschaftliche Deutung erfolgt in separaten
Auswertungsdokumenten, damit Annahmen, Grenzen und Schlussfolgerungen nicht
mit den automatisch erzeugten Tabellen vermischt werden.

## Reproduzierbarkeit

Ein korrigierter urbaner Ergebnislauf wurde mit folgendem Befehl erzeugt:

```bash
python3 scripts/uav-run-experiments.py --profile urban-forms --runs 5 --sim-time 30 --aoi-sample-interval 0.2 --results-dir results/uav-urban-forms-v16 --timeout 900 --only urban-wifi-adhoc --only urban-olsr-mesh --only urban-lte-infra
```

Der Lauf umfasst 45 Einzelruns: drei Architekturen, drei urbane Formen und
fuenf Zufallswiederholungen. Die aggregierten Ergebnisse liegen in
`results/uav-urban-forms-v16/aggregate-summary.csv`. Da Ergebnisordner nicht
Teil des schlanken Git-Repositories sind, werden die interpretierenden
Auswertungsdokumente versioniert und die Ergebnisse dort zusammengefasst.
