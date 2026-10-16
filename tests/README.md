# Tests

Geplanter Ansatz:
1. **Fixtures** (`tests/fixtures/*.txt`): kurze, echte Frames aus Mitschnitten, je mit Szenario und Quelle. Prüfen: `python3 -I tools/verify_fixtures.py` (Prüfsumme, Länge).
0. **Ausführen:** `cmake -S tests -B build/tests && cmake --build build/tests && build/tests/test_gicar` (oder mit ctest). Zusätzlich lohnt sich ein Lauf mit `-fsanitize=address,undefined`.
2. **Parser hostseitig** (umgesetzt in `tests/test_gicar.cpp`): Der Frame-Parser wird als reines C++ ohne ESPHome-Header gebaut, damit er mit einem normalen Compiler (z. B. Catch2/doctest, CMake) gegen die Fixtures läuft: Frame-Erkennung, Längen, Prüfsumme, Felder (Temp /10, Brühflag, Boiler-Flags, Z-Shot-Flag), Resync nach Datenmüll, getrennte Bytes.
3. **Zustandsautomat** (umgesetzt): Zeitgesteuert mit simulierter Uhr und Fake-Transport (Boot-Probe, Polling-Takt, Verbindungs-Timeout, Config-Refresh, Brühzustand, Read-only-Garantie).
3b. **Sniffer-Log-Parser und VCD-Konverter**: `python3 -I tests/test_vcd_converter.py` (synthetische 2-Kanal-Aufnahme `tests/vcd/synthetic_2ch_1MHz.vcd`, erzeugt aus den Fixtures mit `sigrok-cli`; kein echter Mitschnitt).
4. **Hardware-Tests** (manuell, siehe `docs/roadmap.md`): Loopback, Sniffer, später freigegebene Writes.

Fixture-Format: `<frame> | <Beschreibung>`, Zeilen mit `#` sind Kommentare.
