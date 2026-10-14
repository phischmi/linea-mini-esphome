# linea-mini-esphome

ESPHome-Integration für die La Marzocco Linea Mini (2022, mit IoT-Modul, Hauptplatinen-Firmware v2.12). Ein eigener ESP32 ersetzt das IoT-Modul am Stecker **CN11** der GICAR-Hauptplatine. Kein Cloud-Zwang, Steuerung über die native ESPHome-API und Home Assistant, optional MQTT. Kein Display.

**Status: Phase 0 (Dokumentation und Gerüst).** Es gibt noch keinen lauffähigen Komponenten-Code. Die Protokollbasis steht in [`docs/protocol/`](docs/protocol/overview.md), jede Aussage mit Quelle und Status (verifiziert / unverifiziert / Widerspruch). Offene Punkte: [`open-questions.md`](docs/protocol/open-questions.md). Fahrplan: [`docs/roadmap.md`](docs/roadmap.md).

## Sicherheitshinweis
- Du arbeitest an einer netzspannungsführenden Maschine. Netzstecker ziehen, bevor du Gehäuse öffnest.
- CN11 ist **kein** RS-232. Keinen Pegelwandler anschließen. Pegel messen, bevor du verbindest ([`docs/hardware/cn11-interface.md`](docs/hardware/cn11-interface.md)).
- Das Projekt ist standardmäßig **read-only**. Schreibzugriffe auf die Maschine nur nach bewusster Freigabe, nie auf Register aus der Sperrliste ([`docs/protocol/registers.md`](docs/protocol/registers.md), Abschnitt C).
- Keine Gewähr. Eingriffe können die Garantie berühren und die Maschine beschädigen.

## Rückbau
Das Original-IoT-Modul bleibt aufbewahrt. Zum Zurückrüsten: ESP-Adapter von CN11 abziehen, Original-Modul auf CN11 stecken. Keine Änderungen an der Hauptplatine nötig, solange nur CN11 benutzt wird.

## Struktur
`docs/` Wissensbasis · `components/linea_mini/` External Component (Platzhalter) · `tests/` Fixtures und Testkonzept · `examples/` ESPHome-Beispiele (Platzhalter) · `tools/` Hilfsskripte.

Lizenz: noch nicht festgelegt (Entscheidung ausstehend).
