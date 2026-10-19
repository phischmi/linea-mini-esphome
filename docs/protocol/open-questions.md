# Offene Fragen und Widersprüche

Jeder Eintrag: Stand, Beleg, **Vorschlag zur Klärung an der echten Maschine**. „Sniffer-Log“ = passiver ESP32 parallel an CN11 (nur lauschen), Format wie in CAP: eine Zeile pro Byte mit Kanal und Zeitstempel.

## Gelöst durch eigene Auswertung der Mitschnitte
| # | Frage | Ergebnis |
|---|---|---|
| G1 | Z-Frame-Länge 54 vs. 55 | **55 Zeichen** (161/161 in CAP 14, 889/889 in CAP 07). „54“ ist ein Zählfehler der Quelle. |
| G2 | Z-Temperatur /10 vs. /160 | **/10**, Feld bei Zeichen `[45..48]`. /160 ergibt sich aus einer um 1 Zeichen verschobenen Lesung. Gegenprobe über R-Frame im selben Moment (CAP 14, `0366`). |
| G3 | X-Antwort 11 vs. 15 Zeichen | **11** (`X00000001D9`), CAP 13. |
| G4 | Z-Prüfsumme ohne `Z[N]`? | Nein, **inklusive** `Z6` (161/161). Letztes Zeichenpaar = Prüfsumme. |
| G5 | Standby-Register 0x0000 vs. 0x0406 | **0x0000** (CAP 11/12). 0x0406 wird nur als Sync mit `00` geschrieben. |
| G6 | `0x8000` Brew-Stop | **Nicht verwendbar** (Werkstest-Relais; Live-Test: wirkungslos). Gesperrt. |
| G7 | Poll-Rate 100 ms vs. 760 ms | **760 ms** (CAP 01; 100 ms stört laut Live-Test). |
| G8 | W-ACK-Schreibweise (`w` vs. `W`) | **Großes `W`**, 13 Zeichen, `OK`. |
| G9 | Reinigung „1 Hz bis Ende“ | **6 Pulse, dann Ruhe** (CAP 07, L). Siehe `sequences.md` §5. |

## Offen
| # | Frage / Widerspruch | Quellenlage | Vorschlag zur Klärung |
|---|---|---|---|
| O1 | **Pre-Infusion-Register/Einheit**: `0x00E2`=`14` (App-Wert, Bereich 2–20) vs. `0x0101`/`0x0111` (Firmware-Strings, Default 0x10/0x25). Einheit unbekannt (s? 0,1 s?). Wert `0x01`/`0x02` für 0x000B nie gesehen. | CAP 05/06 vs. Register-Map | Sniffer-Log mit **App-Änderung** mehrerer Werte (z. B. 2, 5, 10, 20 s) und je einem Shot, **nur im Festwasserbetrieb möglich** (Angabe des Nutzers); Pre-Infusion-Dauer per Z-Temperaturplateau/Shot-Flag stoppen. Danach nur per Freigabe 0x00E2 mit kleinem Wert schreiben. |
| O2 | Bedeutung 0x0406 | Nur als Sync-Schreibwert `00` gesehen. | Sniffer: App-Standby mit unterschiedlichen Zuständen (Steam an/aus). Ob die Werte je ≠ `00` sind. |
| O3 | Wake: folgen die vier Sync-Writes? | Mitschnitt zeigt sie nach Standby, nach Wake nur `W00E1=81`. Code der Quelle sendet sie auch nach Wake. | Sniffer-Log einer Original-App-Wake-Sequenz, 30 s lang. |
| O4 | Bedeutung von `0x20` in Nutzbyte 27 (Standby-Übergang, „Lüfter“) | nur Spekulation | Sniffer beim Standby-Eintritt, parallel Beobachtung der Maschine (Lüfter/Relais hörbar). |
| O5 | Z-Frame-Felder außer Shot-Flag, Counter, Temp | unbekannt | Mehrere Shots (mit Puck, mit Pre-Infusion) loggen; Differenzen auswerten. |
| O6 | Brew-Flag: 1 s Debounce? Warum fehlen R-Antworten beim Brühen (Z-Frames dominieren)? | CAP 14: 1 R-Antwort in 15 s | Messung der Poll-Antwort-Rate beim Brühen mit 760 ms und mit langsamerem Poll. |
| O7 | `0x00FD`: Testmodus-Tor oder Dampf-Offset? 0x8002/0x8003 Zuordnung | widersprüchlich in LMC | **Nichts tun** (Sperrliste). Klärung nur durch Dokumente/Disassembly, nicht durch Schreibtest. |
| O8 | Firmware-Abhängigkeit: Mitschnitte stammen von einer Maschine mit Board-Firmware v2.12, DIP-Schalter 1/2 = OFF. Eigene Maschine: Hauptplatine v2.12, IoT-Modul-Gateway-Firmware **v7.4.2** (Angabe des Nutzers, nicht gegengeprüft) | Gateway-Version der Quelle ist nicht dokumentiert (U): Ob v7.4.2 dieselben Frames/Boot-Schritte nutzt, ist offen | Eigene Maschine mit Sniffer bestätigen: Typ-Byte `03`, Seriennummer, Frames identisch zu Fixtures? |
| O9 | Boot-Verhalten ohne Original-Modul: Genügt Poll ohne X-Probe? Wird Phase 3 (Writes auf 0x0002/0x0310/0x0300) benötigt? | L: Poll hält Stream am Laufen; Phase 3 ungetestet | Sniffer beim Boot **mit** Original-Modul (Referenz), dann ESP nur lesend (X-Probe + Polls) testen. |
| O10 | Was liefern `R4001`/`R4010`/`R400F`? | nur Existenz | Antworten im Sniffer-Log dekodieren; reine Lesung, harmlos. |
| O11 | Pegel an CN11 und Verhalten der 300 Ω / 10 kΩ-Beschaltung | Autor-Angabe | Oszilloskop/Multimeter: Idle-Pegel (erwartet ~0 V, invertiert), High ~2,5 V, nach Pull-up am ESP-Eingang ≤ 3,3 V. |
| O12 | Reboot-Register 0x0058/0x001F sowie `H`/`S`-Kommandos | nur Disassembly der Quelle | Nicht prüfen, Sperrliste. |
| O13 | Gateway-Firmware v7.4.2 des Nutzers: gleiche Boot-Sequenz (X-Probe, Discovery-Reads, Phase 3) und gleiche Poll-/Config-Burst-Takte wie in CAP 13? | Quelle nennt keine Gateway-Version | Sniffer-Log mit **Original-Modul** (Boot + 5 min Normalbetrieb) aufnehmen und mit `sequences.md` §1–2 vergleichen, bevor der ESP das Original-Modul ersetzt. |
| O14 | Acaia-BLE-Protokoll der Acaia Lunar des Nutzers für die direkte ESP32-Kopplung | nicht recherchiert, hier nichts dokumentiert | Erst in Phase 5b: Firmware der Waage klären, öffentliche Dokumentation oder eigenen BLE-Mitschnitt (nRF Connect) auswerten; keine Übernahme aus unlizenziertem Code. |
| O15 | **Wasserquelle (Tank / Festwasser)**: Ändert sich im Protokoll etwas, und wo ist der Betriebsmodus sichtbar? Pre-Infusion ist laut Nutzer nur im Festwasserbetrieb nutzbar, die Referenz-Mitschnitte nennen die Wasserquelle nicht (U). Möglich sind Unterschiede im Statusframe, in den Konfigurationsblöcken (0x0020/0x0300/0x0310) oder bei Schreibbefehlen (0x000B/0x00E2), und im Verhalten von 0x4010 (Nachfüllsonde laut Quelle, U). | – | Kaltstart (13) und Leerlauf (01) in beiden Betriebsarten aufnehmen und die Frames vergleichen (`tools/parse_sniffer_log.py`, Fixtures). Die Wasserquelle bei jedem Mitschnitt notieren. Pre-Infusion-Writes erst nach dieser Klärung planen. |
| O16 | **Signalanpassung des Original-Moduls zwischen CN4 und ESP32** (Transistoren „702“, drei SOT-23-6-ICs): Wie wird der Pegel der Maschine (ca. 2,5 V, idle-low) aufbereitet, und liegt am ESP32 invertiert oder nicht invertiert an? | Fotos (`docs/hardware/original-module.md`), Aufdrucke nicht lesbar | Makrofotos der ICs bei hellem, schrägem Licht (beide Seiten); dann Bauteile identifizieren. Bei **ausgeschaltetem** Gerät Durchgang und Widerstandswerte messen. Mit dem Analysator die Leitung **hinter** der Anpassung (am ESP32-Pin des Moduls) gegen CN4 vergleichen: Pegel und Polarität. Ergebnis fließt in den Eingangsstufen-Entwurf der Trägerplatine. |
| O17 | Funktion von **CN3** (10 Pole) und von **P1–P3**, **IC9** und den sechs Pads auf der Unterseite des Moduls | Fotos | Prüfen, ob in der Maschine ein Kabel an CN3 steckt und wohin es führt. Pins gegen GND und 3,3 V durchmessen. Nichts anschließen oder programmieren, bevor die Belegung klar ist. |

