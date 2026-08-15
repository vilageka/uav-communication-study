# Results

Dieses Verzeichnis ist fuer ausgewaehlte Ergebnisdateien oder spaetere
Auswertungsgrafiken vorgesehen.

Generierte CSV-Dateien werden standardmaessig durch `.gitignore` ignoriert,
damit das Repository nicht versehentlich mit grossen Rohdaten gefuellt wird.
Wenn ein bestimmtes Ergebnis dauerhaft dokumentiert werden soll, sollte dazu
ein kurzes Auswertungsdokument oder eine bewusst ausgewaehlte kleine Datei
committed werden.

Beispiel fuer lokale Rohdaten:

```bash
./ns3 run "uav-wifi-aoi --numUavs=20 --spacing=100 \
  --updateMetricsFile=../uav-communication-study/results/uav-aoi-updates-20.csv \
  --aoiMetricsFile=../uav-communication-study/results/uav-aoi-samples-20.csv"
```

Wenn solche Rohdaten versioniert werden sollen, die `.gitignore`-Regel fuer
die konkrete Datei gezielt mit `git add -f` uebersteuern.
