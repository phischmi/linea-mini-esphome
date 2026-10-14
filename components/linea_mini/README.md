# components/linea_mini

ESPHome External Component für die GICAR-Platine der Linea Mini (CN11). Stand: rein lesende Komponente. Parser und Zustandsautomat sind am Rechner getestet, die ESPHome-Anbindung ist konfigurationsgeprüft (`esphome config`) und als Firmware für `esp32dev` (ESP-IDF, ESPHome 2026.9.1) kompiliert; die Komponente ist **noch nicht an der Maschine getestet**.

## Dateien
| Datei | Inhalt |
|---|---|
| `protocol_profile.h` | **Alle** Protokollkonstanten an einer Stelle (Offsets, Längen, Bitmasken, Zeiten, Adressen), je mit Status V/U. Weicht eine Maschine ab, wird nur das Profil geändert (`kDefaultProfile` = Referenzmaschine, Board-Firmware v2.12). |
| `gicar_protocol.h/.cpp` | Prüfsumme, Befehls-Builder (nur Lesen und Boot-Probe), byteweiser Frame-Assembler, Decoder für Poll-, Config-, Z- und ACK-Frames. Kein Schreib-Builder. |
| `controller.h/.cpp` | Nicht blockierender, **rein lesender** Zustandsautomat: Boot-Probe, Poll alle 760 ms, Config-Block, `MachineState`, Verbindungsüberwachung. Schnittstellen `Transport` (UART) und `Listener` (Logging). |
| `__init__.py` | Hub-Schema (`uart_id`, `send_probe`), prüft 9600 8N1 und verlangt RX und TX am UART. |
| `sensor.py`, `binary_sensor.py`, `text_sensor.py` | Entitäten (siehe unten). |
| `linea_mini.h/.cpp` | `LineaMiniComponent`: liest UART-Bytes, ruft den Controller, veröffentlicht Zustände nur bei Änderung, loggt defekte Frames (WARN), undecodierte (DEBUG), alle (VERBOSE). |

Alles ohne ESPHome-Header, damit es mit einem normalen Compiler testbar ist.

## Entitäten (alle optional)
| Plattform | Schlüssel | Bedeutung |
|---|---|---|
| sensor | `coffee_temperature`, `setpoint_temperature` | °C; `NaN`, solange keine Verbindung besteht |
| sensor | `frames_ok`, `frames_bad` | Diagnose-Zähler |
| binary_sensor | `connected`, `brewing`, `pump`, `heating`, `steam`, `standby` | ohne Verbindung (außer `connected`) immer aus |
| text_sensor | `serial_number` | Seriennummer aus der Poll-Antwort (Diagnose) |

Beispiel: `examples/linea_mini_readonly.yaml`. Wichtig: `inverted: true` auf `rx_pin` **und** `tx_pin` des UART setzen (CN11 ist idle-low); das kann die Komponente nicht prüfen.

## Entwurfsentscheidungen
- **Profil statt Konstanten im Code.** Abweichungen deiner Maschine (Sniffer-Log) lassen sich durch ein zweites `Profile` abbilden; `Controller` und `FrameParser` bekommen das Profil übergeben.
- **Read-only erzwungen.** Der Controller kann nur `R…`-Lesebefehle und die Boot-Probe `X00000000D8` senden. Ein Test (`test_controller_is_read_only`) prüft das über einen langen simulierten Lauf.
- **Boot-Probe abschaltbar/überspringbar.** Antwortet die Maschine nicht auf die Probe, wird nach `probe_give_up_ms` trotzdem gepollt (offene Frage O9). `send_probe=false` im Profil überspringt sie ganz.
- **Brühzustand** (`brew_active`, Status U): Beim Brühen kommen kaum R-Antworten, die Z-Frames dominieren. Deshalb gilt Brühen, wenn ein Z-Frame mit gesetztem Shot-Flag jünger als `z_recent_ms` ist **oder** der Hebel-Flag der letzten R-Antwort jünger als `r_stale_ms` ist. Die Zeiten sind aus den Mitschnitten abgeleitet, nicht gemessen.
- **Standby** = Boiler-Flags `0x00` (V, CAP 12); `0x20` ist ein Übergang und zählt nicht als Standby.
- **Unbekannte/defekte Frames** werden nicht verworfen, sondern dem `Listener` gemeldet (Logging), damit Abweichungen im Sniffer-Log sofort auffallen.

## Tests
`tests/README.md`. Kurz: `cmake -S tests -B build/tests && cmake --build build/tests && build/tests/test_gicar`.

## TODO
- Test an der Maschine nach dem Sniffer-Abgleich (O13).
- Optional später: Standby-Entität als `switch` erst mit Schreibfreigabe (Phase 3).
- Writes (Phase 3) erst nach Freigabe je Register.
