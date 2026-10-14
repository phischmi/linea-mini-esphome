# Tests

Geplanter Ansatz:
1. **Fixtures** (`tests/fixtures/*.txt`): kurze, echte Frames aus Mitschnitten, je mit Szenario und Quelle. Prüfen: `python3 -I tools/verify_fixtures.py` (Prüfsumme, Länge).
2. **Parser hostseitig**: Der Frame-Parser wird als reines C++ ohne ESPHome-Header gebaut, damit er mit einem normalen Compiler (z. B. Catch2/doctest, CMake) gegen die Fixtures läuft: Frame-Erkennung, Längen, Prüfsumme, Felder (Temp /10, Brühflag, Boiler-Flags, Z-Shot-Flag), Resync nach Datenmüll, getrennte Bytes.
3. **Zustandsautomat**: Zeitgesteuert mit simulierter Uhr, ohne Hardware (Boot-Probe, Polling, Timeouts).
4. **Hardware-Tests** (manuell, siehe `docs/roadmap.md`): Loopback, Sniffer, später freigegebene Writes.

Fixture-Format: `<frame> | <Beschreibung>`, Zeilen mit `#` sind Kommentare.
