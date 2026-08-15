# Version 01: Basic UAV Mobility Test

## Programm

`scratch/uav-test.cc`

## Ziel

Diese erste Version prueft, ob UAVs als ns-3 Nodes erzeugt und mit festen
3D-Positionen versehen werden koennen. Sie ist keine Kommunikationssimulation,
sondern ein Einstieg in das Mobility-Modell.

## Modell

- 5 UAVs als `NodeContainer`
- feste Positionen mit `ListPositionAllocator`
- statisches `ConstantPositionMobilityModel`
- keine NetDevices
- kein Internet-Stack
- keine IP-Adressen
- keine Paketuebertragung

## Startbefehl

```bash
./ns3 run uav-test
```

## Beispielausgabe

```text
Sim laeuft5
UAV 0 : (0,0,80)
UAV 1 : (100,0,80)
UAV 2 : (200,0,80)
UAV 3 : (300,0,80)
UAV 4 : (400,0,80)
```

## Auswertung

Diese Version bestaetigt, dass die grundlegende UAV-Erzeugung funktioniert.
Die Positionen werden korrekt gesetzt und aus dem `MobilityModel` gelesen.

Wissenschaftlich ist diese Version nur eine technische Vorstufe. Sie liefert
noch keine Aussagen ueber Kommunikationsarchitekturen, Latenz, Skalierbarkeit
oder Aktualitaet von Positionsinformationen.

## Naechster Schritt

Aufbau einer Kommunikationsbaseline mit Wi-Fi Ad-hoc, UDP-Broadcast,
IP-Adressen und messbaren Paketmetriken.
