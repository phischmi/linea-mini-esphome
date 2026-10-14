# Protokoll-Überblick (GICAR, La Marzocco Linea Mini)

Status-Legende: **V** = verifiziert (Mitschnitt oder Live-Test), **U** = unverifiziert, **W** = Widerspruch (siehe `open-questions.md`).
Quellen-Kürzel: siehe `sources.md`.

## Transport
| Aussage | Status | Quelle |
|---|---|---|
| UART 9600 Baud, 8N1 | V | CAP-Index („9600 8N1 inverted“), LMC `gicar.cpp`-Kopfkommentar (läuft live) |
| Signale auf CN11 sind idle-low, daher in der UART invertiert zu lesen/senden (`inverted`) | V | LMC `docs/WIRING.md` §2; Live: nicht invertiert dekodiert nur Rauschen, invertiert saubere Frames (LMC `gicar.cpp`, Abschnitt UART-Setup) |
| Pegel ca. 2,5 V LVCMOS, kein RS-232-Pegel | V (Autor-Messung) | LMC `HARDWARE_INTERFACE.md`; Nachmessen an eigener Maschine bleibt Pflicht |
| Master/Slave: Gateway fragt, Maschine antwortet; zusätzlich sendet die Maschine **autonom** Z-Frames beim Brühen/Rückspülen | V | CAP-Analyse „Active-Brew Autonomous Stream“, EIGEN (161 Z-Frames in CAP 14) |
| Alle Frames sind druckbares ASCII, Nutzdaten als Hex-Paare (Großbuchstaben) | V | EIGEN (alle Fixtures) |

## Frameformat Gateway → Maschine (kein Z-Präfix)
```
Read : R AAAA LLLL CC
Write: W AAAA LLLL DD…  CC
Boot : X 00000000 CC          (Probe, einmalig nach Maschinenstart)
```
`AAAA` = Registeradresse (4 Hex-Zeichen), `LLLL` = Länge in **Bytes** (4 Hex-Zeichen), `DD…` = Daten (2 Hex-Zeichen je Byte), `CC` = Prüfsumme. Status **V** (EIGEN: alle Kommandos der Captures, Prüfsumme stimmt).

## Prüfsumme
`CC` = (Summe der ASCII-Werte **aller** Zeichen vor `CC`) mod 256, als 2 Hex-Zeichen.
- Gilt für Gateway-Kommandos: **V** (EIGEN).
- Gilt für Maschinen-R-, W-ACK- und X-Antworten: **V** (EIGEN).
- Gilt für **Z-Frames inklusive** der Zeichen `Z` und `6` am Anfang: **V** (EIGEN: 161/161 Frames in CAP 14 stimmen nur mit Start bei Index 0; Start bei Index 1 oder 2: 0 Treffer). Die LMC-Register-Map behauptet, `Z[N]` sei ausgenommen: **W, widerlegt**.

## Richtung und Timing
| Aussage | Status | Quelle |
|---|---|---|
| Poll `R40000023DB` alle **760 ms** (Abstand in CAP 01: 439, 1199, 1959 … ms) | V | EIGEN, CAP-Index 01 |
| Schnelleres Polling (100 ms) stört: Busauslastung ~96 %, kollidiert mit Z-Stream, Frame-Korruption | V (Live laut Autor) | LMC `gicar.cpp`-Kommentar zu `POLL_PERIOD_MS`; LMC `gicar_protocol_analysis.md` nennt dagegen 100 ms → **W** |
| Bei Brühen ~600 B/s Maschine→Gateway (161 Z-Frames in ~15 s füllen ~99 % der Maschinenbytes) | V | CAP 14, EIGEN |
| UART-RX-Puffer ≥ 1024–2048 B nötig (ESPHome-Default ist 256) | V (Defaults laut ESPH) / U (Mindestgröße ist Erfahrungswert aus LMC, dort 2048) | ESPH, LMC `gicar.cpp` |
| Maschine antwortet auf X-Probe nach ~19 ms | V | CAP 13 (X bei 26358 ms, Antwort bei 26377 ms) |
| Antwort auf Write (ACK) folgt typisch binnen ~80–100 ms; unter Last (Brühen) gehen führende Writes einer Burst-Sequenz verloren | V (Live laut Autor) | LMC `machine.cpp`-Kommentare zum Brew-Stop; EIGEN: ACK-Abstand in CAP 12 ≈ 80–95 ms |

## Frame-Typen Maschine → Gateway (Kurzüberblick, Details in `frames.md`)
| Typ | Start | Länge | Anlass |
|---|---|---|---|
| R-Antwort Poll | `R40000023…` | 81 | Antwort auf `R40000023DB` |
| R-Antwort Config | `R00000020…` | 75 | Antwort auf `R00000020D4` |
| Z-Stream | `Z6…` | 55 | autonom beim Brühen/Rückspülen, ~11 Hz |
| X-Antwort | `X00000001…` | 11 | Antwort auf X-Probe |
| W-ACK | `W<Adr><Len>OK` | 13 | Antwort auf jeden Write |

## Gefahrenhinweis
Die Gateway-Firmware kennt Kommandos, die die Maschinen-Platine in den Bootloader schicken oder Testrelais schalten (`X…02`, `H…`, Register `0x80xx`, `0x00FD`). **Nie senden**, siehe `registers.md` Abschnitt „Sperrliste“.
