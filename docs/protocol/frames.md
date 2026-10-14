# Frames: Längen, Felder, Beispiele

Status-Legende: **V** verifiziert, **U** unverifiziert, **W** Widerspruch. Alle Beispiele stammen aus Mitschnitten (CAP, selbst zusammengesetzt = EIGEN) und liegen als Testdaten in `tests/fixtures/`.

## 1. R-Antwort auf den Poll (81 Zeichen) — V
Antwort auf `R40000023DB`. Aufbau: `R` + `4000` + `0023` + 35 Nutzbytes (70 Hex-Zeichen) + Prüfsumme (2) = 1+4+4+70+2 = **81**.
Nutzbyte *n* beginnt bei Zeichenindex `9 + 2n`.

| Nutzbyte | Inhalt | Status | Beleg |
|---|---|---|---|
| 0 | `03` (Maschinentyp LINEA) | V | CAP 13: Read `R40000001` liefert `03` |
| 3–14 | ASCII-Seriennummer (`Sn…`, 12 Zeichen) | V | EIGEN |
| 17 | Brühflag: `01` = Hebel auf „Brew“, sonst `00`; folgt dem **Hebel**, nicht der Pumpe | V | CAP 14 (`…0100 01…`), LMC `CLAUDE.md` (Live 2026-07-14) |
| 27 | Boiler-Flags (Bitmaske, siehe unten) | V | CAP 01/03/12/14 |
| 28–29 | Kaffeeboiler-Temperatur, uint16 big-endian, **/10 °C** | V | CAP 01 (`03A2` = 93,0), CAP 14 (`0366` = 87,0 beim Brühen, Temperaturabfall) |
| übrige | unbekannt | U | – |

Bits von Nutzbyte 27 (V, CAP 01/03/12/14):
`0x01` Heizelement Kaffee an · `0x02` Pumpe läuft · `0x08` Magnetventil offen · `0x10` Dampfboiler aktiv · Bits `0x60` im Normalbetrieb konstant gesetzt. Beobachtete Werte: `60/61` Idle, `70/71` Dampf an, `7B` Brühen, `20` Übergang, `00` Standby.
`0x20`-Bedeutung („Lüfter“) ist **U** (Spekulation der Quelle).

Beispiel Idle (`tests/fixtures/r_frames.txt`):
`R4000002303D400536E323031313031303535310100000000000000010100006103A200000100058D`

## 2. R-Antwort Config-Block (75 Zeichen) — V
Antwort auf `R00000020D4` (32 Bytes ab 0x0000). Länge 1+4+4+64+2 = 75. Nutzbyte 7–8 = **aktive Solltemperatur** (uint16 BE, /10). In CAP 13 `03B6` = 95,0 °C, passt zur gesetzten Temperatur (V). Laut LMC läuft dieser Wert beim Aufheizen als „Soft-Start“ hoch (85 → 91 → 93) und ist erst danach der Zielwert (V, Live laut Autor, LMC `gicar.cpp`). Übrige Bytes: U.
Weitere Blöcke, die der Original-Gateway periodisch (~12–16 s) liest: `R0020004C` (76 B), `R0310001D` (29 B), `R03000007` (7 B), `R00E20004` (4 B, Antwort `14000000` = Pre-Infusion-Wert 0x14 + 3 unbekannte Bytes). V für Existenz (CAP 04–08), Inhalt außer 0x00E2: U.

## 3. Z-Frame (autonom, 55 Zeichen) — V (Länge/Prüfsumme/Felder), W (Skalierung der Quellen)
Beispiel: `Z60000016536E3230313130313035353101000009000003A2000098`

| Zeichenindex | Inhalt | Status |
|---|---|---|
| 0–1 | `Z6` (Kennung; `6` = 0x36 = 54, **nicht** die Framelänge 55 und kein Zähler) | V (alle 889+162 Z-Starts in CAP 07/14 sind `Z6`; auch CAP 02/15), Bedeutung U |
| 2–5 | `0000` | V |
| 6–7 | `01` konstant | V |
| 8–9 | `65` konstant | V |
| 10–33 | enthält Seriennummer (Hex-ASCII `53 6E …`) und Konstanten | V |
| 34–35 | **Shot-Flag**: `10` = Brühen, `00` = Shot beendet | V (CAP 14/15, Backflush: ein durchgehender Shot über den ganzen Zyklus) |
| 40–41 | Rolling Counter (inkrementiert, wiederholt sich gelegentlich) | V (Frame 20/21 in CAP 14 identisch) |
| 45–48 | **Kaffeetemperatur**, uint16 BE, **/10 °C** (`03A2` = 93,0) | V, siehe unten |
| 53–54 | Prüfsumme über alle 53 Zeichen davor (inkl. `Z6`) | V (161/161) |

**Länge 55** ist V (EIGEN: 161/161 Frames in CAP 14, 889 in CAP 07 haben 55 Zeichen). Die „54“ in `gicar_protocol_analysis.md` / Register-Map ist falsch gezählt (W, gelöst: 55). Das letzte Zeichenpaar ist eine normale 2-stellige Prüfsumme, kein „1-Byte-Terminator“.

**Skalierung (W, gelöst):** Die LMC-Auswertung liest `[46..49]` als uint16 und teilt durch 160. Das ist eine um **ein Zeichen verschobene** Lesung: Das Feld liegt bei `[45..48]` (`03A2`); `[46..49]` ergibt `3A20` = 14880 = 930·16 → /160 = 93,0 liefert nur zufällig dasselbe, weil das Folgezeichen `0` ist. Mit korrekter Ausrichtung gilt **/10**, identisch zur R-Frame-Skalierung. Gegenprobe: R-Frame (`0366`) und der direkt benachbarte Z-Frame (`0366`) in CAP 14 sind gleich. Das erklärt auch das „immer Vielfaches von 32“ der Quelle. (EIGEN, V.)

## 4. X-Antwort (11 Zeichen) — V
Probe: `X00000000D8` → Antwort `X00000001D9` (Zeichen 1–8 = `00000001`, danach Prüfsumme). Die „15 Zeichen mit `Z[seq]` und 2 Sensorbytes“ aus `gicar_protocol_analysis.md` ist durch CAP 13 widerlegt (W, gelöst: 11 Zeichen). Bedeutung von `…0001`: „läuft im Anwendungsmodus“ (U, aus Disassembly der Quelle).

## 5. W-ACK (13 Zeichen) — V
Aufbau: `W` + Adresse (4) + Länge (4) + `OK` + Prüfsumme (2). **Großes `W`** (V, EIGEN; die Ascaso-Familie antwortet kleingeschrieben, hier nicht). Beispiele: `W00000001OK72` (Reg 0x0000), `W00E10001OK88`, `W00070002OK7A`, `W04000001OK76`.
Das ACK trägt weder Wert noch Fehlerdetail. Fehlerform `KO` statt `OK`: U (nur Dokumentation der Quelle, nie gesehen).
Hinweis für den Parser: ACKs von Register 0x0000 und den vier Sync-Registern kommen in einer Burst-Sequenz direkt hintereinander.

## 6. Prüfsummen-Beispiele (V, EIGEN)
| Kommando | Bedeutung |
|---|---|
| `R40000023DB` | Poll |
| `R00000020D4` | Config lesen |
| `W0007000203ADC8` / `W0007000203B6BB` | Solltemperatur 94,1 / 95,0 °C (CAP 04) |
| `W00E100018157` / `W00E10001014F` / `W00E100018258` | Dampf an / aus / Reinigungspuls |
| `W000000010038` / `W000000010139` | Standby / Wake |
