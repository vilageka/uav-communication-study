# Freie Friis-Szenarien: Frequenzkorrektur auf 2.4 GHz

Dieses Dokument beschreibt eine methodische Korrektur an den freien
Wi-Fi-basierten Szenarien. Die urbanen Wi-Fi- und OLSR-Varianten wurden
bereits in Version 16 auf eine konsistente Modellfrequenz von 2.4 GHz
gesetzt. Bei der anschliessenden Pruefung fiel auf, dass die freien
Friis-Szenarien diese Frequenz bisher nicht explizit gesetzt hatten.

## Problem

Die freien Wi-Fi- und OLSR-Szenarien verwenden:

- `WIFI_STANDARD_80211b`,
- `DsssRate11Mbps`,
- `FriisPropagationLossModel`.

Der verwendete Wi-Fi-Standard und der DSSS-Datenmodus gehoeren zur
2.4-GHz-Wi-Fi-Konfiguration. Das `FriisPropagationLossModel` besitzt jedoch
ein eigenes Attribut `Frequency`. In ns-3 liegt dessen Default bei 5.15 GHz.
Wird die Frequenz nicht explizit gesetzt, ist die Freiraumdaempfung deshalb
nicht konsistent zur verwendeten 802.11b-Konfiguration.

## Korrektur

Folgende Dateien setzen nun explizit `channelFrequencyHz = 2.4e9` und reichen
diesen Wert an das Friis-Modell weiter:

- `scratch/uav-wifi-baseline.cc`,
- `scratch/uav-wifi-aoi.cc`,
- `scratch/uav-mesh-olsr-aoi.cc`.

Zusaetzlich wurde ein Kommandozeilenparameter `--frequency` ergaenzt. Damit
kann die Frequenz fuer Sensitivitaetsanalysen spaeter gezielt ueberschrieben
werden, ohne den C++-Code zu aendern.

## Konsequenz fuer vorhandene Ergebnisse

Die bereits erzeugten urbanen Ergebnisse ab `v16` und die Hoehenvariation
`v18` sind von dieser Korrektur nicht betroffen, weil die urbanen
Wi-Fi-basierten Programme die Frequenz bereits explizit auf 2.4 GHz setzen.

Die freien Standardergebnisse aus `v14` sollten dagegen neu erzeugt werden,
bevor sie als finale Zahlen in der Bachelorarbeit verwendet werden. Sie
wurden mit dem alten freien Friis-Default erstellt und sind daher fuer einen
finalen Architekturvergleich methodisch nicht mehr die bevorzugte
Ergebnisbasis.

## Verifikation

Nach der Korrektur wurden kurze Smoke-Tests fuer die drei betroffenen
Programme ausgefuehrt:

```bash
./ns3 run "uav-wifi-aoi --numUavs=5 --simTime=2 --spacing=100 --frequency=2.4e9"
./ns3 run "uav-mesh-olsr-aoi --numUavs=5 --simTime=2 --appStart=2 --spacing=100 --frequency=2.4e9"
./ns3 run "uav-wifi-baseline --numUavs=5 --simTime=2 --spacing=100 --frequency=2.4e9"
```

Alle drei Programme bauen und starten erfolgreich. Der OLSR-Smoke-Test ist
nicht als Ergebnislauf zu interpretieren, weil die Einschwing- und
Simulationszeit absichtlich sehr kurz gewaehlt wurden.
