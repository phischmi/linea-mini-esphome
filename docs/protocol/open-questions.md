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
| O1 | **Pre-Infusion-Register/Einheit**: `0x00E2`=`14` (App-Wert, Bereich 2–20) vs. `0x0101`/`0x0111` (Firmware-Strings, Default 0x10/0x25). Einheit unbekannt (s? 0,1 s?). Wert `0x01`/`0x02` für 0x000B nie gesehen. | CAP 05/06 vs. Register-Map | Sniffer-Log mit **App-Änderung** mehrerer Werte (z. B. 2, 5, 10, 20 s) und je einem Shot; Pre-Infusion-Dauer per Z-Temperaturplateau/Shot-Flag stoppen. Danach nur per Freigabe 0x00E2 mit kleinem Wert schreiben. |
| O2 | Bedeutung 0x0406 | Nur als Sync-Schreibwert `00` gesehen. | Sniffer: App-Standby mit unterschiedlichen Zuständen (Steam an/aus). Ob die Werte je ≠ `00` sind. |
| O3 | Wake: folgen die vier Sync-Writes? | Mitschnitt zeigt sie nach Standby, nach Wake nur `W00E1=81`. Code der Quelle sendet sie auch nach Wake. | Sniffer-Log einer Original-App-Wake-Sequenz, 30 s lang. |
| O4 | Bedeutung von `0x20` in Nutzbyte 27 (Standby-Übergang, „Lüfter“) | nur Spekulation | Sniffer beim Standby-Eintritt, parallel Beobachtung der Maschine (Lüfter/Relais hörbar). |
| O5 | Z-Frame-Felder außer Shot-Flag, Counter, Temp | unbekannt | Mehrere Shots (mit Puck, mit Pre-Infusion) loggen; Differenzen auswerten. |
| O6 | Brew-Flag: 1 s Debounce? Warum fehlen R-Antworten beim Brühen (Z-Frames dominieren)? | CAP 14: 1 R-Antwort in 15 s | Messung der Poll-Antwort-Rate beim Brühen mit 760 ms und mit langsamerem Poll. |
| O7 | `0x00FD`: Testmodus-Tor oder Dampf-Offset? 0x8002/0x8003 Zuordnung | widersprüchlich in LMC | **Nichts tun** (Sperrliste). Klärung nur durch Dokumente/Disassembly, nicht durch Schreibtest. |
| O8 | Firmware-Abhängigkeit: Mitschnitte stammen von einer Maschine mit Board-Firmware v2.12, DIP-Schalter 1/2 = OFF | – | Eigene Maschine mit Sniffer bestätigen: Typ-Byte `03`, Seriennummer, Frames identisch zu Fixtures? |
| O9 | Boot-Verhalten ohne Original-Modul: Genügt Poll ohne X-Probe? Wird Phase 3 (Writes auf 0x0002/0x0310/0x0300) benötigt? | L: Poll hält Stream am Laufen; Phase 3 ungetestet | Sniffer beim Boot **mit** Original-Modul (Referenz), dann ESP nur lesend (X-Probe + Polls) testen. |
| O10 | Was liefern `R4001`/`R4010`/`R400F`? | nur Existenz | Antworten im Sniffer-Log dekodieren; reine Lesung, harmlos. |
| O11 | Pegel an CN11 und Verhalten der 300 Ω / 10 kΩ-Beschaltung | Autor-Angabe | Oszilloskop/Multimeter: Idle-Pegel (erwartet ~0 V, invertiert), High ~2,5 V, nach Pull-up am ESP-Eingang ≤ 3,3 V. |
| O12 | Reboot-Register 0x0058/0x001F sowie `H`/`S`-Kommandos | nur Disassembly der Quelle | Nicht prüfen, Sperrliste. |
