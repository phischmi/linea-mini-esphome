# Quellen und Lizenzstatus

Stand: 2026-10-14. Alle Aussagen in `docs/` nennen ihre Quelle über die Kürzel unten.

| Kürzel | Quelle | Lizenz | Nutzung hier |
|---|---|---|---|
| **LMC** | <https://github.com/Sc00bs110/linea-mini-controller> (Commit `13db6b0`), Dateien unter `reference/lm_mini/`, `src/`, `docs/`, `CLAUDE.md` | **Keine LICENSE-Datei** (geprüft: weder im Root noch im Unterbaum). Damit gilt: alle Rechte beim Autor. | Nur Wissensquelle. Fakten in eigenen Worten, kein Code, keine längeren Textpassagen. |
| **CAP** | Mitschnitte `reference/captures/*.docx` im LMC-Repo (Sniffer-Logs, ein Byte pro Zeile, Kanal B = Gateway→Maschine, M = Maschine→Gateway) | wie LMC (keine Lizenz) | Nur Fakten. Aus den Byte-Strömen habe ich Frames selbst zusammengesetzt und Prüfsummen nachgerechnet (Kürzel **EIGEN**). Kurze Einzel-Frames liegen als Testdaten in `tests/fixtures/`, die Dokumente selbst nicht. |
| **GS** | <https://github.com/antondlr/gicar-serial> | Apache-2.0 (LICENSE vorhanden) | Nur Hintergrund zur Gicar-Familie (Ascaso Baby T, anderes Frameformat). Bei Übernahme wären Apache-Pflichten (Lizenztext, Hinweise) zu beachten. Bisher nichts übernommen. |
| **EIGEN** | Eigene Auswertung dieser Session: Frames aus CAP neu zusammengesetzt, Prüfsummen und Feldpositionen per Skript geprüft (`tools/verify_fixtures.py`). | – | Belegt „verifiziert (Mitschnitt)“. |
| **ESPH** | <https://esphome.io/components/uart/>, ESPHome-`LICENSE` | MIT (Python/Rest), GPLv3 (C/C++-Runtime) | Doku nur zitiert/zusammengefasst. |

## Was nicht kopiert werden darf
- Von LMC/CAP: kein Quellcode (`gicar.cpp`, `machine.cpp`, …), keine Registertabellen-Texte, keine Kommentarblöcke, keine Diagramme/Fotos, keine `.docx`-Dateien.
- Zulässig: Fakten (Adressen, Offsets, Prüfsummenregel, Zeiten) in eigener Formulierung, kurze Roh-Frames als Testdaten.
- Der Ablauf des Brew-Stops und der Reinigung ist hier selbst aus CAP und den Beobachtungen der Quelle beschrieben, nicht deren Code nachgebaut.

## Vertrauensstufen der Quellen (wichtig fürs Lesen)
- LMC-Dokumente sind teils KI-gestützt entstanden und widersprechen sich untereinander und dem eigenen Code (Details: `open-questions.md`). Bei Konflikt gilt: **Mitschnitt (EIGEN) > Live-Test laut Autor-Kommentar im Code > Firmware-Disassembly > Spekulation**.
