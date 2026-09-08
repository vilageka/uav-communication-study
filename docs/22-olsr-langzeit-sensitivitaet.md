# OLSR-Langzeitlauf fuer 40 UAVs und 200 m

Dieses Dokument beschreibt den Ergebnislauf `v22`. Der Lauf dient nicht dazu,
die PDR-Aussage aus `v21` grundlegend neu zu bestimmen. Stattdessen
untersucht er, wie sich Age of Information und unknown-AoI-Zustaende im
kritischen OLSR-Fall ueber ein laengeres Beobachtungsfenster verhalten.

## Fragestellung

In `v21` brach OLSR bei 40 UAVs und 200 m Gitterabstand deutlich ein. Der
30-s-Lauf zeigte eine niedrige PDR und gleichzeitig einen erhoehten Anteil
unbekannter AoI-Zustaende. Fuer die Interpretation ist jedoch wichtig, ob
dieser unknown-Anteil dauerhaft bestehen bleibt oder vor allem durch
Spaetkonvergenz und die kurze Beobachtungsdauer verursacht wird.

Der neue Lauf beantwortet deshalb folgende Frage:

```text
Bleibt der OLSR-Einbruch bei 40 UAVs und 200 m auch ueber 120 s
Anwendungstraffic sichtbar, und wie veraendert sich die AoI-Auswertung,
wenn fruehe Transienten ausgeschlossen werden?
```

## Implementierung

Das Experimentskript `scripts/uav-run-experiments.py` wurde um das Profil
`olsr-long-sensitivity` erweitert. Dieses Profil erzeugt genau einen
Szenariopunkt:

```text
OLSR-Mesh, 40 UAVs, 200 m Gitterabstand
```

Dadurch wird verhindert, dass der lange 120-s-Lauf unbeabsichtigt fuer alle
Architekturen oder alle Skalierungspunkte gestartet wird. Die Architektur-
Auswahl wird im Profil automatisch auf `olsr-mesh` begrenzt.

Die Sendezeit-Staffelung wurde nicht veraendert. Im OLSR-Programm wird der
erste Versand fuer jedes Sender-Empfaenger-Paar weiterhin deterministisch aus
Sender- und Empfaenger-ID abgeleitet. Damit bleibt die Laststruktur
methodisch vergleichbar mit `v21`.

## Versuchsaufbau

Der Lauf wurde mit folgenden Parametern ausgefuehrt:

```text
Architektur: OLSR-Mesh
UAVs: 40
Gitterabstand: 200 m
Anwendungstraffic: 120 s
Anwendungsstart: 5 s
Updateintervall: 1 s
AoI-Abtastintervall: 0.2 s
RNG-Wiederholungen: 1 bis 5
```

Der verwendete Startbefehl war:

```bash
python3 scripts/uav-run-experiments.py \
    --profile olsr-long-sensitivity \
    --runs 5 \
    --sim-time 120 \
    --update-interval 1 \
    --aoi-sample-interval 0.2 \
    --results-dir results/uav-olsr-long-40-200-v22 \
    --timeout 1800
```

Im OLSR-Scratch-Programm bedeutet `simTime` die Dauer des Anwendungstraffics.
Mit `appStart=5` laeuft der Anwendungsverkehr daher von 5 s bis 125 s. Die
Simulation selbst endet kurz danach.

## Auswertungsfenster

Die Rohdaten wurden viermal ausgewertet. Dabei wurde jeweils derselbe
Rohdatensatz verwendet, aber der Beginn des Analysefensters verschoben:

| Datei | Analysefenster |
| --- | --- |
| `steady-from-10s.csv` | 10 s bis 125 s |
| `steady-from-20s.csv` | 20 s bis 125 s |
| `steady-from-40s.csv` | 40 s bis 125 s |
| `steady-from-60s.csv` | 60 s bis 125 s |

Technisch wurde dies ueber `--warmup-intervals` umgesetzt. Da das
Updateintervall 1 s betraegt und der Anwendungstraffic bei 5 s startet,
entsprechen die Warmup-Werte 5, 15, 35 und 55 den Analysebeginnen 10, 20,
40 und 60 s.

## Ergebnisse

Die Tabelle zeigt Mittelwert +/- Standardabweichung ueber fuenf
RNG-Wiederholungen. Zum Vergleich ist auch der kurze `v21`-Lauf aufgefuehrt.

| Auswertung | PDR | Unknown AoI | Avg AoI [s] | Max AoI [s] | Latenz [ms] | Hops | RX App-Bytes |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `v21`, ab 6.5 s, 30 s Traffic | 0.388 +/- 0.004 | 0.171 +/- 0.008 | 3.611 +/- 0.088 | 29.954 +/- 0.038 | 327.225 +/- 9.575 | 1.675 +/- 0.049 | 423848 +/- 5188 |
| `v22`, ab 10 s | 0.350 +/- 0.006 | 0.042 +/- 0.004 | 6.600 +/- 0.065 | 116.615 +/- 4.723 | 368.783 +/- 10.257 | 1.927 +/- 0.026 | 1594808 +/- 26970 |
| `v22`, ab 20 s | 0.348 +/- 0.006 | 0.025 +/- 0.003 | 6.902 +/- 0.064 | 116.615 +/- 4.723 | 373.030 +/- 10.873 | 1.954 +/- 0.030 | 1450949 +/- 26377 |
| `v22`, ab 40 s | 0.343 +/- 0.007 | 0.013 +/- 0.002 | 7.357 +/- 0.108 | 116.615 +/- 4.723 | 380.838 +/- 12.115 | 1.985 +/- 0.049 | 1162324 +/- 25601 |
| `v22`, ab 60 s | 0.343 +/- 0.010 | 0.008 +/- 0.002 | 7.611 +/- 0.216 | 116.615 +/- 4.723 | 382.111 +/- 15.123 | 1.989 +/- 0.067 | 895331 +/- 25306 |

## Deutung

Der Langzeitlauf bestaetigt die PDR-Aussage aus `v21`: OLSR bleibt bei
40 UAVs und 200 m auch nach laengerer Laufzeit deutlich eingeschraenkt. Die
PDR liegt im 120-s-Lauf je nach Auswertungsfenster bei etwa 0.343 bis 0.350
und damit sogar etwas niedriger als im 30-s-Lauf. Die reduzierte Zustellrate
ist also kein reines Start- oder Konvergenzartefakt.

Anders verhaelt sich der unknown-AoI-Anteil. Im kurzen `v21`-Fenster lag er
bei 0.171. Im langen Lauf sinkt er bereits bei Auswertung ab 10 s auf 0.042
und bei Auswertung ab 60 s auf 0.008. Das bedeutet: Viele UAV-Paare erhalten
im Verlauf der laengeren Simulation irgendwann mindestens eine Information,
auch wenn anschliessend nicht alle periodischen Updates erfolgreich
zugestellt werden. Unknown AoI misst daher eher, ob ueberhaupt jemals ein
aktueller Zustand bekannt wurde, waehrend PDR die kontinuierliche
Zustellleistung beschreibt.

Gleichzeitig steigt der bekannte AoI deutlich an. Der mittlere bekannte AoI
liegt im Langzeitlauf zwischen 6.6 s und 7.6 s; der maximale bekannte AoI
liegt bei rund 116.6 s. Diese Kombination aus niedrigem unknown-Anteil und
hohem bekannten AoI ist fuer die Interpretation wichtig: Das Netz ist nicht
vollstaendig blind, aber viele Informationen sind veraltet. Fuer kooperative
UAV-Anwendungen ist das kritisch, weil eine formal bekannte Position nicht
automatisch operativ brauchbar ist.

Auch die Latenz bestaetigt die Belastung des Mesh-Ansatzes. Die mittlere
Latenz empfangener Pakete steigt von etwa 327 ms im kurzen Lauf auf etwa
369 bis 382 ms im langen Lauf. Die mittlere Hop-Zahl steigt ebenfalls auf
knapp 2. Das zeigt, dass im laengeren Beobachtungsfenster mehr Mehrhop-Pfade
auftreten, diese aber die Zustellprobleme nicht vollstaendig kompensieren.

## Konsequenz fuer die Bachelorarbeit

Fuer die PDR-Aussage reicht `v21` weiterhin aus: Der Einbruch bei OLSR fuer
40 UAVs und 200 m ist robust. `v22` sollte ergaenzend verwendet werden, um
die AoI-Aussage differenzierter zu formulieren:

- Der hohe unknown-AoI-Anteil aus dem kurzen Lauf ist teilweise transient.
- Die schlechte PDR bleibt auch im Langzeitlauf bestehen.
- Der bekannte AoI wird im Langzeitlauf deutlich groesser.
- Die Architektur liefert also nicht einfach gar keine Informationen, sondern
  haeufig spaete und veraltete Informationen.

Damit ist `v22` besonders geeignet, um den Unterschied zwischen
Erreichbarkeit, kontinuierlicher Zustellung und Informationsfrische zu
erklaeren.
