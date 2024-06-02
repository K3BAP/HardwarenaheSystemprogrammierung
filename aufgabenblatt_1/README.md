# Übungsblatt 1 - Ergebnisbericht

## Aufgabe 1
### Implementierung
Für die Implementierung habe ich mich für Python entschieden. Folgende Libraries wurden für die Implementierung verwendet:

- **numpy** für die Datenverwaltung und die Bereitstellung des FFT Algorithmus
- **scipi** für das Lesen der WAV-Datei
- **matplotlib** für das Erstellen von Diagrammen aus den Ergebnissen.

Ich habe mich entschieden, eine maximale Anzahl an blöcken festzulegen, die verarbeitet werden sollen, um die Laufzeit des Programms zu begrenzen. Für die Darstellung nutze ich ein Spektrogramm.

Die Variablen (einzulesende Datei, Block-Größe und maximale Samples) können im Skript angepasst werden. Die Schrittweite beträgt 1.

### Ergebnis
In der Datei `example.png` ist ein beispielhaftes Ergebnis des Algorithmus dargestellt. Man erkennt hier einen deutlichen Ausschlag bei ca. 90 hz, zwei leichtere Ausschläge bei etwa 490 hz und 4000 hz, sowie einen ganz leichten Ausschlag bei knapp 900 hz. Für die Generierung des Beispiels wurden die ersten 100000 Samples mit einer Block-Größe von 1024 und Schrittweite 1 betrachtet.

## Aufgabe 2
Für die Bestimmung des Speicherverbrauchs wurde die Bibliothek `memory_profiler` verwendet, mit 10 Samples pro Sekunde den Speicherbedarf des Python-Programms aufzeichnet und bei bedarf auch plottet. Für den Testlauf wurden die ersten 500000 Samples mit einer Block-Größe von 1024 betrachtet. Der maximale Speicherbedarf lag bei 4350 MiB. Es wurde Python 3.12 unter Arch Linux verwendet, als CPU kam ein AMD Ryzen 5 4500U (6C/6T) mit einer TDP von 15 Watt zum Einsatz. Es standen 8GB DDR4-Arbeitsspeicher und ein 16GB großer SWAP-Speicher zur Verfügung.

## Aufgabe 3
### Variante 1: R
Wenn große Datenmengen verarbeitet werden müssen, greifen Statistiker gerne auf die Programmiersprache R zurück. Mal sehen, ob sich das auch in unserem Fall bewähren kann. Ich schreibe diese Zeilen noch vor dem Memory Profiling, habe jedoch das Programm schon mal laufen lassen. Allein die Laufzeit ist gefühlt um ein Vielfaches schneller als die des Python-Programms. Ich verwende R in der Version 4.4.0.

Für das Profiling habe ich Rprof mit der Option `memory.profiler = TRUE` verwendet. Die Gesamtspeichernutzung der Main-Funktion lag laut Rprof bei knap 88 MB. Die Konfigurationsoptionen sowie die Hardware und das Betriebssystem sind die selben wie in Aufgabe 2 gewesen.

### Variante 2: JavaScript
An welche Programmiersprache denkt man als letztes, wenn es um effiziente und hardwarenahe Programme geht? Richtig: JavaScript. Drum habe ich mich entschieden, Node.js auch mal eine Chance zu geben, sein Potenzial zu demonstrieren. Die Chrome Dev Tools können verwendet werden, um ein Memory Profile einer Node.js Applikation durchzuführen.

*Anmerkung: Ich warte gerade darauf, dass das Programm endlich fertig wird... Also Laufzeitkönig wird Node.js sicherlich nicht. Ich beobachte aber die Heap Size über die Chrome Dev Tools, das scheint zumindest dahingehend nicht so schlimm zu sein wie Python.*

Eine Ewigkeit später kann ich nun verkünden, dass der Speicherbedarf bei etwa 147 MB lag. Die Konfigurationsoptionen sowie die Hardware und das Betriebssystem sind wieder die selben wie in Aufgabe 2 gewesen.