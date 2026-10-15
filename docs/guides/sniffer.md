# Anleitung: Sniffer bauen und benutzen

Ziel: Den Datenverkehr zwischen dem **Original-IoT-Modul** und der GICAR-Platine an CN11 mitschneiden, **ohne** ihn zu stören. Damit schließen wir die offenen Fragen in [`open-questions.md`](../protocol/open-questions.md), vor allem O13 (Gateway-Firmware v7.4.2) und O1–O11.

Der Sniffer **hört nur zu**. Sein ESP32 sendet nie auf den Bus (kein TX-Pin ist konfiguriert und keiner wird verdrahtet).

Status der Angaben: **V** verifiziert, **U** unverifiziert/selbst zu messen. Die Konfiguration `examples/sniffer.yaml` ist für das XIAO ESP32-S3 mit ESPHome 2026.9.1 per `esphome config` geprüft; der Firmware-Build für dieses Board ist noch nicht bestätigt (für `esp32dev` war er fehlerfrei). An der Maschine ist sie **noch nicht getestet** (U).

## 0. Sicherheit zuerst
- Hinter dem Stecker arbeitet Netzspannungstechnik. **Netzstecker ziehen**, bevor du das Gehäuse öffnest oder etwas ansteckst. Verkabeln nur stromlos.
- Nur lauschen: keine Verbindung vom ESP32 zu CN11 Pin 4 (+12 V), keine Verbindung eines ESP32-Ausgangs zu den Signalleitungen.
- **Masse (GND):** Der Sniffer braucht GND von CN11 Pin 1 als Bezugspunkt. Ob diese Masse galvanisch von Netz und Schutzleiter getrennt ist, ist **nicht dokumentiert (U)**. Vor dem ersten Anschluss: Multimeter zwischen CN11-GND und Schutzleiter (z. B. Metallgehäuse) messen, bei Maschine aus und an, DC und AC. Zeigt sich mehr als wenige Volt oder AC-Spannung, **nicht anschließen**; dann nur mit USB-Isolator oder Laptop im Akkubetrieb. Im Zweifel nachfragen.
- Der Rückbau bleibt trivial: Sniffer ab, Maschine zu.

## 1. Material
| Teil | Zweck |
|---|---|
| **Seeed XIAO ESP32-S3** (die normale Variante genügt; die „Sense“-Variante mit Kamera ist nicht nötig) mit externer Antenne am U.FL-Anschluss (nach meiner Erinnerung im Lieferumfang, bitte beim Kauf prüfen; Antenne vor dem ersten Einschalten anstecken) | Sniffer; versorgt über USB-C vom Laptop/Rechner |
| Je 1 kΩ Widerstand, 2 Stück | Strombegrenzung in den beiden Signalleitungen (Schutz, kein Einfluss auf 9600 Baud) |
| Optional 2 × 10 kΩ | Pull-up nach 3,3 V, nur falls der Pegel zu niedrig ist (Schritt 3) |
| Dupont-Kabel, Messklemmen oder Mini-Prüfklemmen | Abgriff an CN11 |
| Multimeter | Pegel- und Masse-Messung |
| Optional USB-Isolator | falls die Masse-Messung es verlangt |

**Abgriff an CN11:** Welcher Steckertyp CN11 ist, steht in keiner meiner Quellen (U). Er ist unter dem Original-Modul zu sehen. Mögliche Wege, vom besten zum schlechtesten:
1. **Y-Adapter**, sobald du zwei passende Gegenstecker hast (du brauchst ohnehin einen für den späteren Dauerbetrieb): ein Stecker zur Platine, einer zum Modul, Abgriff in der Mitte.
2. Die vier Adern zwischen Modul und Platine **am Kabel abgreifen** (Prüfklemmen/Abisolieren nur bei gezogenem Netzstecker), ohne den Kontakt zu unterbrechen.
3. Am Modul-Stecker die Pins von oben abgreifen, falls zugänglich.
Nicht mit Nadeln durch die Isolierung stechen, wenn dadurch Kurzschlüsse zwischen Adern möglich sind.

## 2. Schaltung
CN11-Pins: **1 GND, 2 RXD, 3 TXD, 4 +12 V**. Die Beschriftung kann bei dir vertauscht sein (beim Autor der Quelle war sie es, V). Deshalb heißen die Kanäle im Sniffer nach der Datenrichtung, nicht nach dem Pin:

```
CN11 Pin 1  GND  ───────────────────────────────┬── ESP32 GND
CN11 Pin 3  "TXD" (Maschine → Modul) ── 1 kΩ ───┼── XIAO D7 = GPIO44  (Kanal M, rx_pin)
CN11 Pin 2  "RXD" (Modul → Maschine) ── 1 kΩ ───┴── XIAO D6 = GPIO43  (Kanal B, rx_pin)
CN11 Pin 4  +12 V  ── NICHT verbinden
ESP32 versorgt über USB.   Das Original-Modul bleibt angeschlossen.
```
- **M** = Maschine → Modul (Antworten, Z-Frames), **B** = Modul → Maschine (Polls, Befehle).
- Die Pins D6/D7 (GPIO43/44) sind die beschrifteten TX/RX-Pins des XIAO ESP32-S3. Es sind die Standard-UART0-Pins, deshalb läuft das Log über den USB-C-Anschluss. Dieselben beiden Leitungen bleiben später im Dauerbetrieb dieselben (D6 = ESP-TX → CN11 RXD, D7 = ESP-RX ← CN11 TXD). Pins anderer Boards: `pin_m`/`pin_b` in der YAML ändern und keine Strapping-, Flash-, PSRAM- oder USB-Pins nehmen. Die Pin-Belegung des XIAO stammt aus der ESPHome-Boarddefinition; prüfe sie zusätzlich im Seeed-Pinout.
- Ob die Pin-Zuordnung stimmt, zeigt später der Log (Schritt 6); Vertauschen ist harmlos.

## 3. Pegel messen (vor dem ersten Anschluss an den ESP)
Mit Multimeter (DC) gegen GND, **Maschine an, Original-Modul steckt**:
- Beide Signalleitungen sollten im Ruhezustand nahe **0 V** liegen (idle-low, V laut Quelle); beim Datenverkehr springt der Wert auf **ca. 2,5 V** (Mittelwert am Multimeter, deshalb nicht exakt).
- Zeigt eine Leitung im Ruhezustand dauerhaft ~2,5 V oder 3,3 V oder 12 V, **stoppen** und mir die Werte nennen. Dann stimmt etwas an meinen Annahmen nicht.
- **Hoher Pegel zu niedrig für den ESP32?** Der ESP32 erwartet laut Datenblatt für „High“ etwa 75 % von 3,3 V ≈ 2,5 V; die Maschine liefert ca. 2,5 V. Das ist knapp (U). Funktioniert es (Schritt 6), ist alles gut. Kommen kaputte oder keine Frames, lege pro Kanal einen 10-kΩ-Pull-up von der ESP-Eingangsleitung nach 3,3 V (der Autor der Quelle nutzt diesen Pull-up in seiner Schaltung, V; ob er am parallelen Abgriff das Original-Modul stört, ist **U**: bei Problemen sofort wieder entfernen).

## 4. Software einrichten
Zwei Wege, beide mit derselben Datei `examples/sniffer.yaml`:

**A) Home Assistant ESPHome-Builder (du hast HA):**
1. ESPHome-Add-on öffnen → „New device“ → leer anlegen → Inhalt der `sniffer.yaml` einfügen (Pins/Board anpassen).
2. „Install“ → „Plug into this computer“ (Chrome/Edge, WebSerial), das XIAO per USB-C einstecken. Wird es nicht erkannt: **BOOT-Taste gedrückt halten**, USB einstecken, Taste loslassen (Bootloader-Modus, laut Seeed-Anleitung). Beim ersten Mal ist kein WLAN nötig.
3. Logs: im Dialog „Logs“ → „Download logs“ oder Text kopieren und als `sniffer.log` speichern.

**B) Kommandozeile (Linux/macOS/Windows):**
```
pip install esphome
cd examples
esphome run sniffer.yaml --device /dev/ttyACM0 | tee ../sniffer.log
```
(Das XIAO meldet sich als nativer USB-Anschluss, unter Linux meist `/dev/ttyACM0`, unter Windows `COMx`. Nach jedem Reset meldet sich der Anschluss neu; die Ausgabe erscheint erst, wenn ein Programm die Schnittstelle geöffnet hat.) `esphome logs sniffer.yaml --device …` zeigt später wieder nur den Log.

Der Sniffer nutzt kein WLAN und keine API. Die Konfiguration protokolliert Datenblöcke, die durch eine Pause von ≥ 15 ms getrennt sind (Frames kommen am Stück, dazwischen liegt eine Lücke). Zeitstempel sind `millis()` des ESP32 in Millisekunden, beide Kanäle auf derselben Uhr.

## 5. Testen
1. Maschine **aus**, Sniffer verkabelt, ESP per USB an. Im Log erscheinen keine oder nur vereinzelt Zeilen (schwebende Eingänge können Rauschen liefern; das ist normal).
2. Maschine **ein**. Nach rund 26 s Bootzeit (V, CAP 13) sollten Zeilen erscheinen:
   - `B … X00000000D8` (Probe), `M … X00000001D9` (Antwort),
   - danach `B … R40000023DB` alle ca. 760 ms und `M … R4000002303D4…` (81 Zeichen).
3. Auswerten:
   ```
   python3 -I tools/parse_sniffer_log.py sniffer.log
   ```
   Das Skript zählt die Frame-Typen, prüft jede Prüfsumme, listet alle Befehle des Moduls mit Zeitstempel und warnt, wenn der Poll auf Kanal **M** statt **B** erscheint (dann Pins/Adern tauschen).
   Erwartung: kaum fehlerhafte Prüfsummen. Viele Fehler deuten auf Pegelprobleme (Schritt 3), falsche Polarität oder zu langsames Log hin.

## 6. Mitschnitte, die ich brauche
Je Szenario: Sniffer-Start vor der Aktion, Aktion mit ungefährer Uhrzeit notieren, danach Log speichern (`NN_name.log`) und mir schicken (als Datei oder ins Repo unter `captures/`, falls du das willst). Das Original-Modul bleibt dabei aktiv, Steuerung über die La-Marzocco-App.

| Nr. | Szenario | Dauer | Klärt |
|---|---|---|---|
| 13 | **Kalter Start**: Sniffer läuft, *dann* Maschine einschalten | bis ~60 s nach Boot | O13, O9, O10 (Boot-Ablauf, Gateway v7.4.2) |
| 01 | Idle, nichts tun | 60 s | Takt, Config-Bursts, O13 |
| 04 | Solltemperatur in der App ändern (z. B. 93 → 94 → 93 °C) | 60 s | Schrittweite, Register 0x0007 |
| 05/06 | Pre-Infusion in der App auf **mehrere Werte** (z. B. 2, 5, 10, 20) und aus/an | 90 s | O1 (Register, Einheit) |
| 15 | Leerbezug mit Pre-Infusion (z. B. 3 s), Hebel auf Brew | 30 s | O1, O5 |
| 14 | Leerbezug ohne Pre-Infusion | 30 s | O5, O6 |
| 02 | Normaler Shot mit Puck | 45 s | O5, O6 |
| 03/08 | Dampf an/aus in der App, Dampfhahn kurz öffnen | 60 s | Flags |
| 11/12 | Standby über die App, nach ~20 s Wake, mehrfach | 90 s | O3, O4, O2 |
| 07 | **Backflush** mit Blindsieb über die App | ~150 s | Reinigungsablauf |
| 09/10 | Auto-Standby-Zeit und Zeitplan in der App ändern | 60 s | bestätigt: keine Gicar-Kommandos |

Bitte Mitschnitte von **deiner** Maschine, nicht aus dem Referenzrepo. Der Abgleich mit den Fixtures (`tests/fixtures/`) zeigt, ob deine Maschine identisch spricht (O8).

## 7. Fehlersuche
| Symptom | Ursache / Maßnahme |
|---|---|
| Gar keine Zeilen, auch bei laufender Maschine | GND fehlt oder falsche Pins; Spannung nach Schritt 3 prüfen; `inverted: true` nicht entfernen |
| Nur Müll (`.`, Zufallszeichen) | Polarität/Pegel; Pull-up probieren; GND-Verbindung prüfen |
| Poll `R40000023DB` auf Kanal M | Adern/Pins tauschen (oder `pin_m`/`pin_b` in der YAML) |
| Frames fehlen beim Brühen | Log-Datenrate; die Kanal-M-Zeilen sind dann lang; sicherstellen, dass kein WLAN/API aktiv ist; 115200 Baud ist ausreichend (V, geschätzt ~1,5 kB/s) |
| Prüfsummenfehler gehäuft | Pegel zu knapp (Schritt 3), Kabel zu lang, fehlende Masse |
| Original-Modul verhält sich anders, seit der Sniffer hängt | Sofort abstecken. Sniffer belastet die Leitung nur mit 1 kΩ + ESP-Eingang; Ursache wäre ein Pull-up oder ein Fehlkontakt |

## 8. Danach
- Logs in `docs/` auswerten, Fixtures ergänzen (`tests/fixtures/`, `python3 -I tools/verify_fixtures.py`), offene Fragen aktualisieren.
- Erst wenn der Boot-Ablauf mit dem Original-Modul verstanden ist (O13), ersetzen wir das Modul durch den ESP und testen **read-only** (Roadmap Phase 2).
