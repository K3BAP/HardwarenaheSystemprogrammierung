# Übungsblatt 2 - Ergebnisbericht

## Implementierung
### Aufgabe 1
In anbetracht der folgenden Aufgaben, habe ich mich diesmal bei der Implementierung für C++ entschieden, da die OpenCL-API, die später zum Einsatz kommt, in erster Linie für C und C++ entwickelt wurde. Ich habe mir also eine einfache Implementierung des Cooley-Tukey FFT-Algorithmus mithilfe von ChatGPT 4o generieren lassen, die den Anforderungen der Aufgabe entspricht. Das Programm läuft ohne Parallelisierung.

Die Inputs werden dem Programm als Parameter übergeben: ./fourier_base <file_name> <block_size> <step_width> <amplitude_threshold>