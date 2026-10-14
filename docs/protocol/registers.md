# Register

Status: **V** verifiziert (Mitschnitt = M, Live-Test laut Autor = L), **U** unverifiziert, **W** Widerspruch. Länge in Bytes. Alle Werte hexadezimal, Daten big-endian.
„Schreiben erlaubt?“ bezieht sich auf die Projektregel: nur Status **V** und nur nach meiner Freigabe (siehe `CLAUDE.md`).

## A. Beobachtete Register (Mitschnitt)
| Adresse | Len | R/W | Funktion | Werte | Status | Schreiben erlaubt? | Quelle |
|---|---|---|---|---|---|---|---|
| 0x4000 | 35 | R | Maschinenzustand (Poll, 81-Zeichen-Antwort) | siehe `frames.md` | V (M) | – | CAP 01, EIGEN |
| 0x4000 | 1 | R | Maschinentyp beim Boot | `03` = LINEA | V (M) | – | CAP 13 |
| 0x4003 | 12 | R | Seriennummer (ASCII) | `Sn…` | V (M) | – | CAP 13 |
| 0x4001 / 0x4010 | 1 | R | Boot-Discovery | Bedeutung unklar | V Existenz / U Inhalt | – | CAP 13 |
| 0x400F | 1 | R | Statusabfrage nach Standby/Wake | Inhalt nicht dekodiert | V Existenz / U Inhalt | – | CAP 12 |
| 0x0000 | 32 | R | Config-Block; Nutzbyte 7–8 = aktive Solltemp (/10) | – | V (M, L) | – | CAP 13, LMC `gicar.cpp` |
| 0x0000 | 1 | W | **Standby/Wake**: `00` Standby, `01` Wake | | V (M, L) | nach Freigabe | CAP 11/12, LMC `machine.cpp` |
| 0x0007 | 2 | W | Kaffeeboiler-Solltemperatur, uint16 BE, ×10 °C | `03B6` = 95,0 | V (M) | nach Freigabe | CAP 04 |
| 0x00E1 | 1 | W | Modus: `81` Dampf an, `01` Dampf aus, `82` Reinigungspuls | | V (M) | nach Freigabe | CAP 07/08 |
| 0x000B | 1 | W | Pre-Infusion aus (`00`); ein = `01` vermutet | `00` | V (M) für `00` / U für `01`, `02` | nur `00` nach Freigabe | CAP 06 |
| 0x00E2 | 1 | W | Pre-Infusion-Dauer, App-Bereich 2–20, Beispiel `14`; **Einheit unbekannt** | `14` | V (M) Write / U Einheit | nach Freigabe, Einheit erst klären | CAP 05 |
| 0x00E2 | 4 | R | Pre-Infusion-Block (erstes Byte = Dauer) | `14000000` | V (M) | – | CAP 04–13 |
| 0x0400, 0x0401, 0x0402, 0x0406 | 1 | W | Config-Sync, vom Original-Gateway nach Standby mit `00` geschrieben | `00` | V (M) | nach Freigabe, nur als Standby-Sequenz | CAP 12 |
| 0x0020 | 76 | R | Konfig-Block, periodisch gelesen | | V Existenz / U Inhalt | – | CAP 04–12 |
| 0x0310 | 29 | R | Block, periodisch gelesen | | V Existenz / U Inhalt | – | CAP 04–12 |
| 0x0300 | 7 | R | Block, periodisch gelesen; Boot-Phase 3 schreibt ihn zurück | Beispielantwort `07000001010101` | V Existenz / U Inhalt | **nein** (Inhalt unbekannt) | CAP 13 |
| 0x0002 | 1 | W | Boot-Phase 3: `00` | | V (M) Beobachtung / U Bedeutung | nein | CAP 13 |

### 0x00E1-Werte
`81` Dampfboiler an · `01` Dampfboiler aus · `82` Reinigungs-/Backflush-Puls (V, M). Der Original-Gateway schreibt `81` auch als „Sync“ nach Wake und bei App-Reconnect (V, M, CAP 10/12). Bitfeld-Deutung der Quelle (bit7=aktiv, bit1=Reinigung, bit0=Dampf) ist **U**.

## B. Aus Firmware/Disassembly der Quelle, nicht im Mitschnitt gesehen (alle U)
| Adresse | Len | Behauptete Funktion | Anmerkung |
|---|---|---|---|
| 0x0101 / 0x0111 | 16 / 37 (Block) oder 1 | Pre-Infusion Ein-/Aus-Zeiten Gruppe 1 | **W** gegen 0x00E2 als beobachtete Dauer-Quelle, siehe open-questions |
| 0x0136 / 0x0150 | 26 / 37 | Gruppe 2, bei Mini ohne Funktion | U |
| 0x0100 | 1 | Pre-Infusion-Modusflag | U |
| 0x0406 | 1 | „Standby on/off“ laut Register-Map | **W**: im Mitschnitt wird 0x0406 immer mit `00` als Sync nach beiden Standby-Übergängen geschrieben, kein Toggle (siehe open-questions) |
| 0x0009, 0x0059 | 2 | Boiler 2 / unbekannt | U, bei Mini vermutlich ohne Funktion |
| 0x4002, 0x4003 (12 B), 0x401C | – | Heizstatus, Shot-Timer/Telemetrie | U |

## C. Sperrliste (nie schreiben/senden)
| Ziel | Warum | Quelle |
|---|---|---|
| `0x8000`–`0x8007` | Werkstest-Relais (Pumpe, Heizelemente, Ventile, LED). Wirkt nur im Testmodus; im Normalbetrieb „abgelehnt“. Im Testmodus könnten Heizungen oder Pumpe ohne Regelung schalten. Zuordnung einzelner Adressen ist **W** (0x8002: „Pumpe“ vs. „Kaffeeheizung“; 0x8003: „EROG-Magnetventil“ vs. „Dampfheizung“). | LMC `gicar_protocol_analysis.md` §3A, CAP-Analyse, LMC `CLAUDE.md` |
| `0x00FD` | **W**: „Testmodus-Gate (0x01 = rein)“ vs. „Dampf-Offset (Kalibrierung)“. Beide U. Möglicherweise Tor zu 0x80xx. | CAP-Analyse vs. `gicar_protocol_analysis.md` |
| `X00000002…` | Schickt die Platine in den Bootloader (Firmware-Flash). Mit falscher Folge ist die Maschinenplatine nicht mehr ansprechbar. | `gicar_protocol_analysis.md` §5B (U, Disassembly) |
| `H…`-Frames, `S`-Kommando | Bootloader-Sync bzw. Selbsttest. Für den Betrieb nicht nötig (kein H-Handshake erforderlich, L). | `gicar_protocol_analysis.md` §2, LMC `machine.cpp` |
| 0x0058, 0x001F, 0x0400-`07`-Muster | „Reboot-Sequenz“ der Maschine, nur U. | Register-Map |
| 0x0300, 0x0310, 0x0410 Schreiben | Inhalt unbekannt, kalibrierungsnah. | Register-Map |
| 0x8000 als „Brew stop“ | **Widerspruch gelöst**: wirkungslos (Live-Test 2026-07-14: Pumpe lief weiter) und Teil der Werkstest-Relais. Nicht verwenden. | LMC `CLAUDE.md`, `machine.cpp`; Register-Map nennt es „unbestätigt“ |
