# Urbane Hoehenvariation: Auswertung und Deutung

Dieses Dokument beschreibt den Ergebnislauf `v18`. Der Lauf untersucht, wie
sich die relative Flughoehe der UAVs zur Gebaeudehoehe auf die drei
Kommunikationsarchitekturen auswirkt. Damit wird die wichtigste offene Frage
aus der urbanen Formen-Auswertung aufgegriffen: Die vorherigen urbanen
Szenarien verwendeten 80 m UAV-Hoehe, wodurch alle UAVs oberhalb der
Gebaeude lagen.

## Untersuchungsziel

Die Hoehenvariation soll nicht eine neue Architektur einfuehren, sondern den
Einfluss der vertikalen Positionierung innerhalb derselben urbanen
Stadtprofile sichtbar machen. Fuer jedes urbane Profil wurden drei
Hoehenmodi simuliert:

- `inside`: UAV-Hoehe liegt unterhalb der Dachhoehe, bei 75 Prozent der
  Gebaeudehoehe.
- `near-roof`: UAV-Hoehe liegt knapp oberhalb der Dachhoehe, konkret
  Gebaeudehoehe plus 5 m.
- `above-roof`: UAV-Hoehe entspricht der bisherigen Referenz von 80 m.

Die UAVs befinden sich auch im `inside`-Modus auf Outdoor-Positionen entlang
der Strassenkorridore. Der Begriff bedeutet also nicht, dass UAVs innerhalb
von Gebaeuden platziert werden, sondern dass ihre Flughoehe unterhalb der
Dachlinie des synthetischen Stadtprofils liegt.

## Versuchsaufbau

Der Lauf umfasst drei Architekturen, drei urbane Formen, drei Hoehenmodi und
fuenf Wiederholungen je Konfiguration. Insgesamt wurden damit 135
Einzelsimulationen ausgefuehrt:

```text
3 Architekturen x 3 urbane Formen x 3 Hoehenmodi x 5 rngRun = 135 Runs
```

Die Simulationsdauer betraegt 30 s, das Updateintervall 1 s und das
AoI-Abtastintervall 0.2 s. Die horizontale Platzierung bleibt innerhalb
eines urbanen Profils konstant. Dadurch laesst sich die Wirkung der
Flughoehe besser isolieren als bei einer gleichzeitigen Variation von
UAV-Anzahl, Platzierungsintervall und Stadtform.

Die urbanen Profile verwenden folgende Gebaeudehoehen und daraus abgeleitete
Flughoehen:

| Urbane Form | Gebaeudehoehe | inside | near-roof | above-roof |
| --- | ---: | ---: | ---: | ---: |
| urban-open | 20 m | 15.0 m | 25.0 m | 80.0 m |
| urban-baseline | 35 m | 26.25 m | 40.0 m | 80.0 m |
| urban-canyon | 60 m | 45.0 m | 65.0 m | 80.0 m |

Wie in der vorherigen urbanen Auswertung gilt: Das verwendete
`HybridBuildingsPropagationLossModel` ist kein Raytracing-Modell. Die
Ergebnisse zeigen parametrisierte urbane Ausbreitungsbedingungen in
synthetischer Stadtgeometrie, nicht die konkrete Abschattung einzelner
Gebaeudekanten.

## Ergebnisse

### Urban Open

| Hoehenmodus | Architektur | PDR | Unknown AoI | Bekannter AoI [s] | Latenz [ms] | Hops | App-Bytes |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: |
| inside, 15.0 m | Wi-Fi Ad-hoc | 0.271 +/- 0.005 | 0.729 +/- 0.005 | 0.506 | 0.304 | n/a | 12130 |
| inside, 15.0 m | OLSR-Mesh | 0.929 +/- 0.011 | 0.065 +/- 0.004 | 0.595 | 52.937 | 2.285 | 272940 |
| inside, 15.0 m | LTE | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.516 | 14.552 | 2.000 | 271420 |
| near-roof, 25.0 m | Wi-Fi Ad-hoc | 0.911 +/- 0.012 | 0.089 +/- 0.012 | 0.506 | 0.305 | n/a | 12130 |
| near-roof, 25.0 m | OLSR-Mesh | 0.996 +/- 0.005 | 0.003 +/- 0.005 | 0.508 | 2.661 | 1.191 | 272940 |
| near-roof, 25.0 m | LTE | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.516 | 14.550 | 2.000 | 271420 |
| above-roof, 80.0 m | Wi-Fi Ad-hoc | 0.914 +/- 0.013 | 0.086 +/- 0.013 | 0.506 | 0.305 | n/a | 12130 |
| above-roof, 80.0 m | OLSR-Mesh | 0.999 +/- 0.001 | 0.002 +/- 0.001 | 0.506 | 1.928 | 1.197 | 272940 |
| above-roof, 80.0 m | LTE | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.516 | 14.550 | 2.000 | 271420 |

### Urban Baseline

| Hoehenmodus | Architektur | PDR | Unknown AoI | Bekannter AoI [s] | Latenz [ms] | Hops | App-Bytes |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: |
| inside, 26.25 m | Wi-Fi Ad-hoc | 0.359 +/- 0.004 | 0.641 +/- 0.004 | 0.507 | 0.306 | n/a | 13840 |
| inside, 26.25 m | OLSR-Mesh | 0.931 +/- 0.016 | 0.052 +/- 0.014 | 0.598 | 34.455 | 1.979 | 305430 |
| inside, 26.25 m | LTE | 0.978 +/- 0.030 | 0.020 +/- 0.027 | 0.518 | 14.494 | 2.000 | 303910 |
| near-roof, 40.0 m | Wi-Fi Ad-hoc | 0.934 +/- 0.013 | 0.066 +/- 0.013 | 0.507 | 0.304 | n/a | 12040 |
| near-roof, 40.0 m | OLSR-Mesh | 0.998 +/- 0.002 | 0.003 +/- 0.002 | 0.505 | 1.336 | 1.141 | 271230 |
| near-roof, 40.0 m | LTE | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.516 | 14.550 | 2.000 | 269710 |
| above-roof, 80.0 m | Wi-Fi Ad-hoc | 0.935 +/- 0.015 | 0.065 +/- 0.015 | 0.507 | 0.304 | n/a | 12040 |
| above-roof, 80.0 m | OLSR-Mesh | 0.998 +/- 0.003 | 0.001 +/- 0.000 | 0.507 | 1.932 | 1.150 | 271230 |
| above-roof, 80.0 m | LTE | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.516 | 14.550 | 2.000 | 269710 |

### Urban Canyon

| Hoehenmodus | Architektur | PDR | Unknown AoI | Bekannter AoI [s] | Latenz [ms] | Hops | App-Bytes |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: |
| inside, 45.0 m | Wi-Fi Ad-hoc | 0.291 +/- 0.014 | 0.709 +/- 0.014 | 0.509 | 0.308 | n/a | 15010 |
| inside, 45.0 m | OLSR-Mesh | 0.875 +/- 0.047 | 0.092 +/- 0.039 | 0.689 | 59.806 | 2.274 | 327660 |
| inside, 45.0 m | LTE | 0.728 +/- 0.048 | 0.261 +/- 0.029 | 0.736 | 21.550 | 2.000 | 326140 |
| near-roof, 65.0 m | Wi-Fi Ad-hoc | 0.917 +/- 0.015 | 0.083 +/- 0.015 | 0.506 | 0.308 | n/a | 15010 |
| near-roof, 65.0 m | OLSR-Mesh | 0.998 +/- 0.003 | 0.002 +/- 0.001 | 0.509 | 2.292 | 1.186 | 327660 |
| near-roof, 65.0 m | LTE | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.516 | 14.551 | 2.000 | 326140 |
| above-roof, 80.0 m | Wi-Fi Ad-hoc | 0.918 +/- 0.015 | 0.082 +/- 0.015 | 0.506 | 0.308 | n/a | 15010 |
| above-roof, 80.0 m | OLSR-Mesh | 0.999 +/- 0.002 | 0.002 +/- 0.001 | 0.505 | 1.442 | 1.174 | 327660 |
| above-roof, 80.0 m | LTE | 1.000 +/- 0.000 | 0.000 +/- 0.000 | 0.516 | 14.550 | 2.000 | 326140 |

## Deutung

Die Hoehenvariation bestaetigt, dass die relative Flughoehe ein zentraler
Parameter fuer urbane UAV-Kommunikation ist. Der Unterschied zwischen
`inside` und `near-roof` ist deutlich groesser als der Unterschied zwischen
`near-roof` und `above-roof`. Sobald die UAVs knapp oberhalb der Dachlinie
liegen, naehern sich die Ergebnisse stark der 80-m-Referenz an.

Direkter Wi-Fi-Ad-hoc-Broadcast reagiert am staerksten auf den Wechsel unter
die Dachhoehe. In `urban-open-inside` sinkt die PDR auf 0.271, in
`urban-baseline-inside` auf 0.359 und in `urban-canyon-inside` auf 0.291.
Gleichzeitig steigt der Anteil unbekannter AoI-Zustaende auf 64 bis
73 Prozent. Die Latenz der empfangenen Pakete bleibt weiterhin sehr niedrig,
weil nur erfolgreich empfangene Direktpakete in diese Latenz eingehen. Gerade
hier zeigt sich der Nutzen von AoI: Die niedrige Latenz allein wuerde die
schlechte Informationsvollstaendigkeit verbergen.

OLSR-Mesh kann den Einbruch deutlich abfedern, aber nicht ohne Kosten.
Innerhalb der Gebaeudehoehe erreicht OLSR PDR-Werte zwischen 0.875 und
0.931. Das ist wesentlich besser als direkter Broadcast, aber klar schlechter
als die nahezu vollstaendige Zustellung oberhalb der Daecher. Gleichzeitig
steigen Latenz und Hop-Anzahl stark: In `urban-open-inside` liegt die mittlere
Latenz bei 52.937 ms und in `urban-canyon-inside` bei 59.806 ms. Die mittlere
Hop-Zahl steigt dort auf etwa 2.3. Das spricht dafuer, dass das Mesh bei
unguenstiger Hoehe laengere oder instabilere Wege nutzen muss.

LTE bleibt in `urban-open-inside` vollstaendig stabil und zeigt in
`urban-baseline-inside` nur eine moderate Verschlechterung auf eine PDR von
0.978. Im strengsten Szenario `urban-canyon-inside` faellt LTE jedoch auf
eine PDR von 0.728 und einen unbekannten AoI-Anteil von 0.261. Damit zeigt
auch die Infrastrukturarchitektur eine klare Abhaengigkeit von der relativen
Hoehe, wenn die Kombination aus hoher Gebaeudestruktur und niedriger
UAV-Position besonders unguenstig wird.

Der Vergleich zwischen `near-roof` und `above-roof` zeigt dagegen nur geringe
Unterschiede. Wi-Fi-Ad-hoc erreicht knapp oberhalb der Daecher bereits wieder
PDR-Werte um 0.91 bis 0.93. OLSR liegt wieder bei etwa 0.996 bis 0.998, LTE
bei 1.0. Fuer die betrachteten Modelle ist der Schritt von unterhalb der
Dachlinie zu knapp oberhalb der Dachlinie daher wesentlich wichtiger als der
Schritt von knapp oberhalb auf 80 m.

## Schlussfolgerung

Die Hoehenvariation liefert eine wichtige Ergaenzung zur urbanen
Formen-Auswertung. Die Stadtform allein erklaert die Ergebnisse nicht
ausreichend. Entscheidend ist, ob die UAVs relativ zur Gebaeudestruktur
unterhalb oder oberhalb der Dachlinie operieren.

Fuer den Architekturvergleich ergibt sich:

- Wi-Fi-Ad-hoc ist fuer niedrige urbane Flughoehen nicht robust genug, wenn
  vollstaendige Schwarmaktualitaet gefordert ist.
- OLSR-Mesh verbessert die Erreichbarkeit innerhalb der Gebaeudehoehe
  deutlich, bezahlt dies aber mit hoher Latenz und mehr Hops.
- LTE ist bei offeneren und mittleren Profilen sehr robust, kann aber im
  niedrigen Canyon-Fall ebenfalls deutlich einbrechen.
- AoI ist fuer die Bewertung unverzichtbar, weil geringe Paketlatenz bei
  Wi-Fi nicht bedeutet, dass der Schwarm insgesamt gut informiert ist.

Methodisch sollte die Bachelorarbeit die Hoehe relativ zur Gebaeudehoehe als
eigenstaendigen Szenarioparameter behandeln. Fuer die finalen Diagramme ist
eine Darstellung nach Hoehenmodus sinnvoll, beispielsweise PDR und unknown
AoI je Architektur fuer `inside`, `near-roof` und `above-roof`.
