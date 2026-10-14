# Abläufe

Zeiten in ms relativ zum jeweiligen Mitschnitt (CAP, selbst ausgewertet = M) oder laut Live-Beobachtung des Autors (L). Status: **V** / **U** / **W**.

## 1. Boot-Handshake (Maschine wird bestromt) — V (M, CAP 13)
Die Maschine braucht nach Netz-Ein ca. **26 s**, bis sie auf die Schnittstelle antwortet. Davor sendet sie bei Power-on ein Rausch-Byte (`0xFC`), und das Original-Gateway sendet zwei `0x00`-Bytes (bei ~14,5 s und ~16,4 s; Bedeutung U).

| t (ms) | Richtung | Frame | Bedeutung |
|---|---|---|---|
| 26358 | → | `X00000000D8` | Probe „bist du im Anwendungsmodus?“ |
| 26377 | ← | `X00000001D9` | Antwort nach 19 ms |
| 26407 / 26428 | → / ← | `R40100001D8` / `R401000010038` | Discovery 0x4010 |
| 26457 / 26477 | → / ← | `R40000001D7` / `R40000001033A` | Maschinentyp `03` |
| 26507 / 26531 | → / ← | `R4003000CEC` / `R4003000C…` | Seriennummer |
| 26587 | → | `R40010001D8` | 0x4001 |
| ~26700 | → | Burst: `R0000 0020`, `R0020004C`, `R0310001D`, `R03000007`, `R00E20004` | Config-Lesung, ca. 12 ms Abstand |
| danach | → | `R40000023DB` alle 760 ms | normales Polling |
| ca. 5 s nach Polling-Start | → | `W00020001003A`, `R00000020D4`, `W03100001003C`, `W0300000724…` (7-Byte-Block zurückschreiben) | „Phase 3“ Config-Sync |

Anforderung für ESPHome: **nicht blockierend** (Zustandsautomat), X-Probe wiederholen, bis die Antwort kommt (Boot kann 26 s+ dauern). Phase 3 und das Zurückschreiben von 0x0300 sind **U** und werden erst einmal nicht nachgebaut (read-only-Default); ob die Maschine das Polling auch ohne sie mag, ist offen (live von LMC bestätigt: „kein H-Handshake nötig, Poll hält Stream am Laufen“, L).

## 2. Normalbetrieb — V (M)
- Poll `R40000023DB` alle 760 ms (V).
- Alle ca. 12–16 s: Config-Burst (`R0000…`, `R0020…`, `R0310…`, `R0300…`, `R00E20004`), siehe CAP 04–12 (V).
- Setpoint-Änderung durch die App: `W0007…` plus unmittelbar `R00000020D4` zur Kontrolle (CAP 04, V).

## 3. Standby / Wake — V (M, CAP 11/12; L)
Zwei Zustände (V, CAP-Analyse): *Energiesparen* durch Auto-Timer (Dampfboiler aus, Nutzbyte 27 = `60`, ohne Gateway-Kommando) und *Vollstandby* per `W0000=00` (Nutzbyte 27 `00`, Zwischenwert `20`).

CAP 12 (Zeiten ms):
| t | Frame | Anmerkung |
|---|---|---|
| 3580 | `W000000010038` | Standby; direkt danach `R400F0001ED` (+130 ms) |
| 5952 | `W00E100018157` | Dampf `81` erneut (Sync) |
| 7286–7338 | `W0400…`, `W0401…`, `W0402…`, `W0406…` (je `00`) | Sync-Burst ~3,7 s nach Standby; ACKs in ~90-ms-Takt |
| 17750 | `W000000010139` | Wake, danach `R400F0001ED` |
| 20916 | `W00E100018157` | Dampf wieder `81`, ~3,2 s nach Wake |
| 23260 | `W000000010038` | zweites Standby |
| 24317–24369 | Sync-Burst | ~1 s nach Standby |

Aus der Quelle (L, Code-Kommentare): Beim Wake den Dampfzustand neu setzen (die Maschine löscht ihn bei Standby); die vier Sync-Register folgen auch auf den Wake. Letzteres ist im Mitschnitt **nicht** zu sehen → **U**.
Auto-Standby-Zeit und Zeitpläne erzeugen **keine** Gicar-Kommandos (CAP 09/10, V): müssen im ESP selbst laufen.

## 4. Brew-Stop über Standby-Toggle — V (L), nicht im Mitschnitt
Kein Fern-Stop-Register vorhanden (0x8000 wirkungslos, L). Einziger funktionierender Weg laut Quelle:
1. `W0000=00` senden (Standby). Die Maschine stoppt die Pumpe sofort, **queued** das Standby aber, bis der Hebel auf „Stop“ geht.
2. Während des Brühens streamt die Maschine ~600 B/s und verwirft führende Writes einer Burst-Folge. Deshalb ACK-gesteuert arbeiten: erst nur `W0000=00`, bei fehlendem ACK nach ~300 ms erneut (bis 6×), erst nach dem ACK die vier Sync-Register senden.
3. Als zweite Bestätigung den Pumpenbit (Nutzbyte 27, `0x02`) in zwei aufeinanderfolgenden Poll-R-Frames als aus lesen; sonst nach ~2,6 s die ganze Burst-Folge wiederholen (max. 3×).
4. ~500 ms nach bestätigtem Stop `W0000=01` (Wake) plus Sync-Register; **zusätzlich** ~500 ms nachdem der Hebel nach „Stop“ zurückgekehrt ist (Brew-Flag 1 s lang aus), weil das erste Wake vom Queue-Standby ignoriert wird.
5. Garantierter Wake auch bei Aufgabe der Versuche (halb eingeschlafene Maschine ist schlechter als ein Durchläufer).
Zeitkonstanten stammen aus Autor-Beobachtungen (L). **Eigener Hardware-Test nötig**, bevor irgendetwas davon schreibt.

## 5. Reinigung / Backflush — V (M, CAP 07; L)
**Korrektur zum Vorwissen:** Nicht „1 Hz bis zum Ende der Brühphase“, sondern:
1. Hebel unten („Stop“-Seite), dann **genau 6× `W00E100018258` im 1-s-Abstand** (CAP 07: t = 5246, 6246, 7258, 8257, 9258, 10259 ms). Ein einzelner Puls startet nichts (L).
2. Hebel erst **nach** dem Burst auf „Brew“. Ein Puls während des laufenden Zyklus bricht ihn nach der ersten Pumpphase ab (L).
3. Danach **still sein**: Poll auf einen pro ~10 s drosseln (CAP 07: nächster Read bei ~101 s). Normales 760-ms-Polling oder weitere Pulse brechen den Zyklus an einer Phasengrenze ab (L).
4. Maschine läuft allein: ~80 s, ca. 10 Pumpphasen à ~8 s. Das Z-Shot-Flag (`0x10`) bleibt über den **gesamten** Zyklus gesetzt; die fallende Flanke markiert das Ende (L). CAP 07 enthält 889 Z-Frames und 62 R-Frames (V, M).
5. Danach normales Polling (760 ms) wiederherstellen. Das Fenster in der Quelle ist 135 s (nicht 140 s, nicht 130 s); Vorschlag: Ende an der Z-Flanke festmachen, 150 s als Timeout.
Wichtig für „disconnected“-Erkennung: Während der Reinigung Frame-Timeout nicht auslösen (L).

## 6. Post-Command-Init-Sequenz — V (M) Existenz, U Zweck
Die Register-Map beschreibt „nach `W00E1` liest das Gateway 0x0000 → 0x0020 → 0x0310 → 0x0300 → 0x00E2 und schreibt 0x0300 unverändert zurück (Dampf aus: übersprungen)“. Die Mitschnitte zeigen dieselben fünf Reads aber als **periodischen** Burst alle ~12–16 s, auch ohne vorheriges Kommando (CAP 01–13, V, M). Ein Zusammenhang mit Kommandos ist daher **U**; ein Zurückschreiben von 0x0300 nach Mode-Writes ist im Mitschnitt nicht zu sehen (nur beim Boot).

## 7. Dampf — V (M, CAP 08)
`W00E10001014F` (aus) → Nutzbyte 27 `70`→`60`; `W00E100018157` (an) → `60`→`70`. Dampfboiler hat nur an/aus (Pressostat), kein Sollwert (U/L, Quelle).
