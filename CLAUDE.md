# CLAUDE.md

Kurzfassung für künftige Sessions. Sprache: Gespräch Deutsch (du-Form); Code, Kommentare, Commits, Dateinamen Englisch; Doku-Texte Deutsch.

## Projekt
ESPHome External Component für die La Marzocco Linea Mini. ESP32 ersetzt das IoT-Modul an CN11 der GICAR-Platine. Details: `README.md`, `docs/`.

## Regeln
1. **Read-only ist Default.** Schreibzugriffe auf die Maschine nur nach ausdrücklicher Freigabe des Nutzers.
2. **Keine unverifizierten Register schreiben.** Nur Register mit Status V in `docs/protocol/registers.md`. Die **Sperrliste** (0x80xx, 0x00FD, `X…02`, `H`, `S`, Reboot-Register, 0x0300/0x0310/0x0410 schreiben) ist tabu.
3. **Jede Protokollaussage trägt einen Status** (verifiziert / unverifiziert / Widerspruch) und eine Quelle.
4. **Kein Code und keine längeren Textpassagen aus unlizenzierten Quellen.** `Sc00bs110/linea-mini-controller` hat keine LICENSE: nur Fakten in eigenen Worten. `antondlr/gicar-serial` ist Apache-2.0 (Attribution beachten).
5. **Lizenz: GPL-3.0** (Entscheidung des Nutzers, `LICENSE`). Lizenz nicht ändern, ohne zu fragen. Fremdcode nur, wenn er mit GPLv3 vereinbar ist.
6. Push nur auf den vom Nutzer genannten Branch/Remote (`origin`), nie auf andere Branches.

## Protokollfakten (alle V, Details in `docs/protocol/`)
- 9600 8N1, **invertiert**; ASCII-Hex; Prüfsumme = Summe aller ASCII-Zeichen davor mod 256 (auch bei Z-Frames inkl. `Z6`).
- Poll `R40000023DB` alle **760 ms** (schneller stört). R-Antwort 81 Zeichen, Z-Frame 55, X-Antwort 11, W-ACK 13.
- Temperatur /10 (R: Nutzbyte 28–29; Z: Zeichen 45–48). Brühflag = Nutzbyte 17 (folgt dem Hebel), Boiler-Flags = Nutzbyte 27.
- Standby/Wake = Register 0x0000. Brew-Stop nur per Standby-Toggle. Reinigung = 6× `W00E1=82` im 1-s-Abstand, dann ruhig (Poll ~10 s).
- Maschine laut Quelle: Typ `03` (LINEA), `NEWMINIL`, ATmega32, 1 Gruppe, kein Flowmeter, Dampf per Pressostat.

## Maschine des Nutzers
IoT-Modul-Gateway-Firmware **v7.4.2** (vom Nutzer genannt), Hauptplatinen-Firmware v2.12. Die Mitschnitte der Quelle stammen von einem anderen Exemplar; Abgleich mit eigenem Sniffer-Log steht aus (O8, O13). Board: **Seeed XIAO ESP32-S3** (D6/GPIO43 = TX, D7/GPIO44 = RX, Log über USB-C), später auf eigener Trägerplatine in der Maschine. CN11-Gegenstecker fehlt noch. Mosquitto-Broker läuft in Home Assistant, MQTT ist nur Rückfall (ESPHome-API ist der Weg). Waage: Acaia Lunar. Langziel Brew-by-Weight: erst über die HA-Integration, dann direkte BLE-Kopplung im ESP32 (Roadmap Phase 5).

## Quellenstatus
Ausgewertet: LMC-Repo (Commit `13db6b0`), Mitschnitte (selbst nachgerechnet), `gicar-serial` (nur Hintergrund). Fixtures in `tests/fixtures/`, Prüfung: `python3 -I tools/verify_fixtures.py`. Host-Tests: `cmake -S tests -B build/tests && cmake --build build/tests && build/tests/test_gicar`. Protokollkonstanten nur in `components/linea_mini/protocol_profile.h` ändern.

## Offen
Siehe `docs/protocol/open-questions.md`.
