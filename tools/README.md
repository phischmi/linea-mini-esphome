# tools

- `verify_fixtures.py`: prüft Prüfsumme und Länge aller Frames in `tests/fixtures/` (`python3 -I tools/verify_fixtures.py`).
- `parse_sniffer_log.py`: wertet Logs von `examples/sniffer.yaml` aus (Frames, Prüfsummen, Befehle, vertauschte Kanäle); siehe `docs/guides/sniffer.md`.
- `vcd_to_sniffer_log.py`: wandelt eine Logikanalysator-Aufnahme (VCD aus PulseView/sigrok-cli) in dasselbe Logformat; dekodiert 9600 8N1 invertiert aus den Flanken.
- Geplant: Prüfsummen-Helfer zum Bauen von Kommandos.
