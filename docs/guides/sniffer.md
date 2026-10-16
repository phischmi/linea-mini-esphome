# Anleitung: Sniffer bauen und benutzen

> **Zwei Wege:** (A) mit einem **Logikanalysator und PulseView** (Abschnitt „Variante mit Logikanalysator“ am Ende, vom Nutzer gewählt), (B) mit einem **ESP32** (der Rest dieser Anleitung). Sicherheit (Abschnitt 0), Pegelmessung (3) und die Mitschnitt-Liste (6) gelten für beide.

Ziel: Den Datenverkehr zwischen dem **Original-IoT-Modul** und der GICAR-Platine an CN11 mitschneiden, **ohne** ihn zu stören. Damit schließen wir die offenen Fragen in [`open-questions.md`](../protocol/open-questions.md), vor allem O13 (Gateway-Firmware v7.4.2) und O1–O11.

Der Sniffer **hört nur zu**. Sein ESP32 sendet nie auf den Bus (kein TX-Pin ist konfiguriert und keiner wird verdrahtet).

Status der Angaben: **V** verifiziert, **U** unverifiziert/selbst zu messen. Die Konfiguration `examples/sniffer.yaml` ist für das XIAO ESP32-S3 mit ESPHome 2026.9.1 per `esphome config` geprüft; die Firmware baut für dieses Board fehlerfrei. An der Maschine ist sie **noch nicht getestet** (U).

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

**Abgriff an CN11:** Nach Angabe des Nutzers sind Buchsen und Wannen sehr wahrscheinlich **TE Connectivity AMPMODU MOD II** (2,54 mm Raster, U), und das vorhandene Kabel hat **an beiden Enden dieselbe Buchse**. Diese Buchse passt auf eine **4er-Stiftleiste auf dem Breadboard** (Angabe des Nutzers, vor dem Aufbau kurz probieren). Siehe auch `docs/hardware/cn11-interface.md`, Abschnitt „Steckverbinder“.

**Vor dem Aufbau (Maschine aus, Netzstecker gezogen):**
1. **Pin-Zuordnung des Kabels durchmessen.** Welcher Pin am Modul-Ende des Kabels hat Durchgang zu welchem Pin am CN11-Ende? Das Kabel kann gerade (1:1) oder gekreuzt verdrahtet sein. Das könnte auch die beim Autor der Quelle „vertauschten“ TXD/RXD-Beschriftungen erklären (Vermutung, U). Notiere die Zuordnung in `captures/README.md`.
2. **Orientierung markieren:** Ist die Buchse nicht kodiert, lässt sie sich verdreht aufstecken. Markiere Pin 1 an Kabel und Breadboard.
3. **Pin 4 (+12 V)** bleibt bei Ersatz-Tests unbeschaltet (siehe unten).

**Aufbau A: Ersatz-Test mit dem XIAO (Phase 2, ohne Original-Modul)**
```
Maschine CN11 ── Kabel ── Buchse ──► 4er-Stiftleiste auf dem Breadboard
      GND  ──────────────────────────► XIAO GND
      Modulseite RXD (Maschine empfängt) ◄── 300 Ω ◄── XIAO D6 (GPIO43)
      Modulseite TXD (Maschine sendet)   ──► XIAO D7 (GPIO44)   (Pegel: siehe Schritt 3)
      +12 V ── nicht anschließen. XIAO bleibt am USB.
```
Das Original-Modul ist dabei abgesteckt.

**Aufbau B: Mitschneiden bei angeschlossenem Original-Modul (Durchschleifen)**
```
Maschine CN11 ── Kabel ── Buchse ──► Stiftleiste P1 auf dem Breadboard
Breadboard-Reihen:  GND, TXD, RXD (je eine Reihe), +12 V in eigener Reihe mit Abstand
Dupont (Buchse) von diesen Reihen ──► Stifte der Wanne am Original-Modul
Analysator: Klemmen auf die TXD- und RXD-Reihe, GND-Klemme auf die GND-Reihe
```
- Der Verkehr läuft über die Breadboard-Reihen; der Analysator hängt nur parallel und treibt nichts.
- **12 V durchschleifen:** Das Modul bezieht seine Spannung über Pin 4. Führe sie über eine eigene Reihe und eine Dupont-Ader zum Modul, mit Abstand zu den Signalreihen und mit einer **rückstellbaren Sicherung** davor. Den Wert wählst du nach einer Strommessung des Moduls (A-Bereich des Multimeters, U); der Stromverbrauch des Moduls ist mir nicht bekannt.
- Ein verrutschter Dupont-Stecker an Pin 4 neben TXD wäre das Hauptrisiko. Isoliere die 12-V-Ader und sichere die Verbindungen.
- Passen die Dupont-Buchsen nicht sauber in die Wanne des Moduls (Kragen, Sitz), wackelt der Aufbau und erzeugt Bitfehler. Dann auf ein zweites Kabel oder eine Durchschleif-Platine ausweichen.
- Alternativ ohne Eingriff: Mini-Prüfklemmen an den Lötstellen der Wanne auf der Rückseite der Platine (nur bei gezogenem Netzstecker).

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
| 05/06 | **Nur im Festwasserbetrieb:** Pre-Infusion in der App auf **mehrere Werte** (z. B. 2, 5, 10, 20) und aus/an | 90 s | O1 (Register, Einheit) |
| 15 | **Nur im Festwasserbetrieb:** Leerbezug mit Pre-Infusion (z. B. 3 s), Hebel auf Brew | 30 s | O1, O5 |
| 14 | Leerbezug ohne Pre-Infusion | 30 s | O5, O6 |
| 02 | Normaler Shot mit Puck | 45 s | O5, O6 |
| 03/08 | Dampf an/aus in der App, Dampfhahn kurz öffnen | 60 s | Flags |
| 11/12 | Standby über die App, nach ~20 s Wake, mehrfach | 90 s | O3, O4, O2 |
| 07 | **Backflush** mit Blindsieb über die App | ~150 s | Reinigungsablauf |
| 09/10 | Auto-Standby-Zeit und Zeitplan in der App ändern | 60 s | bestätigt: keine Gicar-Kommandos |

**Wasserquelle:** Die Pre-Infusion lässt sich nur im **Festwasserbetrieb** testen (Angabe des Nutzers). Für die Tests 05, 06 und 15 muss die Maschine also am Festwasseranschluss hängen. Notiere bei **jedem** Mitschnitt, ob Tank oder Festwasser aktiv war, denn ob sich Frames oder Register je nach Wasserquelle unterscheiden, ist unbekannt (U, siehe O15). Wenn es sich einrichten lässt, nimm Kaltstart (13) und Leerlauf (01) in **beiden** Betriebsarten auf.

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

## Variante mit Logikanalysator (PulseView und sigrok-cli)

Der Analysator (8 Kanäle, FX2-Nachbau eines Saleae Logic) ersetzt den ESP32 als Sniffer. Er hört nur zu; die Auswertung bleibt dieselbe. Diese Anleitung geht von **PulseView 0.5.0 (Eigenbau) auf einem Apple-Silicon-Mac** aus.

Status der Befehle: Die `sigrok-cli`-Befehle habe ich mit sigrok-cli 0.7.2 und dem `demo`-Treiber ausprobiert (Aufnahme, Umwandlung in VCD, UART-Dekodierung, Auswertung). Den echten FX2-Treiber (`fx2lafw`), PulseView 0.5.0 und macOS konnte ich nicht testen; dort ist alles mit **U** markiert.

### 1. Software und Gerät prüfen
1. **Gleiche Bibliotheken:** PulseView und `sigrok-cli` sollten auf dieselbe libsigrok und libsigrokdecode zeigen. Hast du PulseView selbst gebaut, bau oder installiere `sigrok-cli` aus denselben Quellen (sonst können Treiber- und Dekoder-Versionen abweichen). Ohne eigene `sigrok-cli`: `brew install sigrok-cli` (U, nicht geprüft).
2. **Firmware:** Der FX2-Nachbau bekommt beim Anstecken die Firmware `fx2lafw` aus dem Paket *sigrok-firmware-fx2lafw* in den Chip geladen. Sie muss in einem Suchpfad der libsigrok liegen (bei Eigenbau meist unter `<prefix>/share/sigrok-firmware`, U).
3. **Test:** Analysator direkt am Mac anstecken (kein Hub), dann
   ```
   sigrok-cli --scan
   ```
   Erwartet: eine Zeile mit `fx2lafw` und den Kanälen `D0 … D7`. Fehlt sie, mit `sigrok-cli -l 5 --scan` ausführlich loggen; dort steht, welche Pfade nach Firmware durchsucht werden und ob sie fehlt.
4. Auf macOS ist kein Zadig-Treiber nötig (libusb, U). Beim Anstecken meldet sich der Chip zuerst mit der Cypress-Kennung und nach dem Firmware-Upload neu als `fx2lafw`; das ist normal.

### 2. Anschluss
- **GND-Leitung** an CN11 Pin 1, **D0** an die Leitung CN11 TXD (Maschine → Modul, Kanal M), **D1** an CN11 RXD (Modul → Maschine, Kanal B). Welche Ader welche ist, entscheidet der Poll `R40000023DB` (kommt vom Modul, also auf B). Vertauschst du sie, merkt das die Auswertung.
- Schritt 0 gilt unverändert: **Masse gegen Schutzleiter messen**, im Zweifel USB-Isolator oder Laptop im Akkubetrieb. Der Analysator ist nicht galvanisch getrennt (bei Nachbauten nicht anzunehmen, U).
- Schritt 3 gilt auch: **Pegel mit dem Multimeter messen**. Der Analysator zeigt nur 0 oder 1. Billig-Nachbauten haben oft eine Eingangsschwelle um 2 V (U); die ca. 2,5 V der Maschine liegen darüber, aber knapp. Erscheint auf einer Leitung nichts oder ständig dasselbe, ist das ein Hinweis auf zu schwaches Signal oder die falsche Ader, nicht auf eine tote Maschine.

### 3. Aufnahme mit sigrok-cli (empfohlen, reproduzierbar)
```
sigrok-cli --driver fx2lafw --config samplerate=1m --channels D0,D1 --time 90s -o 13_power_on.sr
```
- `samplerate=1m` heißt hier **1 MHz** (nicht 1 mHz). Das reicht: Ein Bit dauert bei 9600 Baud rund 104 µs, das sind etwa 100 Abtastwerte pro Bit.
- `--time 90s` legt die Dauer fest (auch `--time 90000` in Millisekunden oder `--samples 100m`). **Nicht mit Ctrl-C abbrechen:** In meinem Test (Demo-Treiber, sigrok-cli 0.7.2, Dauermodus) blieb nach dem Abbruch keine Datei übrig. Wähle die Dauer lieber etwas länger als nötig.
- Die Aufnahme beginnt sofort. Beim Kaltstart also erst den Befehl starten und **dann** die Maschine einschalten (~26 s Bootzeit, V laut Mitschnitt der Quelle), Dauer 90 s.
- Größe: 1 Byte pro Abtastwert, also rund 1 MB pro Sekunde (90 s ≈ 90 MB als `.sr` je nach Kompression deutlich weniger).
- Die Kanalnamen `D0,D1` bleiben in der Datei und im VCD erhalten.

### 4. Aufnahme in PulseView 0.5.0 (Alternative)
1. Gerät wählen (Treiber `fx2lafw`), **1 MHz**, Abtastwerte so groß, dass die Aufnahme passt (bei 1 MHz sind 100 M Abtastwerte ≈ 100 s). PulseView hält die Aufnahme im Speicher.
2. Nur D0 und D1 aktiv lassen.
3. Zur Kontrolle einen **UART-Dekoder** pro Kanal hinzufügen (Decoder-Auswahl oben): RX = D0 bzw. D1, 9600 Baud, 8 Datenbits, keine Parität, 1 Stoppbit, **„Invert RX“ an**, Datenformat ASCII. Dann erscheinen `R4000…`-Frames im Klartext. Das ist nur eine Anzeige; die Auswertung unten braucht den Dekoder nicht.
4. Run, nach der Aktion Stop, dann **File → Save As** als `.sr`. Menünamen können in 0.5.0 abweichen (U).
5. Den VCD-Export brauchst du in PulseView nicht: Die Umwandlung geht mit `sigrok-cli` (nächster Abschnitt).

### 5. Trockenlauf (vor jeder Messreihe)
Kurze Aufnahme von 10 s bei laufender Maschine im Leerlauf und ansehen:
```
sigrok-cli --driver fx2lafw --config samplerate=1m --channels D0,D1 --time 10s -o test.sr
sigrok-cli -i test.sr -P uart:rx=D1:baudrate=9600:invert_rx=yes:format=ascii -A uart=rx-data | sed 's/^uart-1: //' | tr -d '\n'; echo
```
Erwartet: auf D1 ein Strom `R40000023DBR40000023DB…`, rund 13 Polls in 10 s (alle 760 ms, V). Mit `rx=D0` siehst du die Antworten der Maschine (`R4000002303D4…`, 81 Zeichen). Kommt auf D0 der Poll statt auf D1, sind die Adern vertauscht.

### 6. Auswertung
```
sigrok-cli -i 13_power_on.sr -O vcd -o 13_power_on.vcd
python3 -I tools/vcd_to_sniffer_log.py 13_power_on.vcd --machine D0 --module D1 -o 13_power_on.log
python3 -I tools/parse_sniffer_log.py 13_power_on.log
```
- Der Konverter dekodiert 9600 8N1 **invertiert** selbst aus den Flanken, fasst Bytes mit Pausen unter 15 ms zu Blöcken zusammen und schreibt dasselbe Logformat wie der ESP32-Sniffer. Er nennt am Ende pro Kanal Bytes, Rahmenfehler und die aus den Pulsen geschätzte Baudrate (Erwartung ≈ 9600 bzw. 9615).
- **Warnung „check --baud and --no-invert“** bedeutet: Der Inhalt ist keine druckbare ASCII-Schrift. Dann stimmt die Polarität oder Baudrate nicht, oder der Pegel wird nicht sauber erkannt.
- Erscheint der Poll auf M, warnt `parse_sniffer_log.py`: `--machine` und `--module` tauschen.
- Die Zeitstempel sind Millisekunden ab Aufnahmebeginn. Die genauen Pausen zwischen den Bytes stehen im VCD (Flanken) und sind in PulseView direkt messbar.

### 7. Aufnahmen weitergeben
Ich kann nur lesen, was im Repo liegt. Lege die **Logs** (`NN_name.log`) unter `captures/` ab, committe und pushe sie. Rohdaten (`.sr`, `.vcd`) bleiben bei dir und sind per `.gitignore` ausgeschlossen. Notiere zu jeder Aufnahme in `captures/README.md`: Datum und Uhrzeit, Szenario, **Wasserquelle (Tank oder Festwasser)**, Firmware-Stände, Solltemperatur und die Aktionen mit ungefährer Uhrzeit.

### 8. Was der Analysator zusätzlich liefert
- Exakte Zeiten zwischen Poll und Antwort, zwischen Befehl und ACK und zwischen den Bytes (für die Zeitkonstanten in `docs/protocol/sequences.md`).
- Eine Aufnahme **der Leitungen der Trägerplatine** später (Pegel nach dem Puffer, TX des ESP32), mit demselben Werkzeug.

### 9. Fehlersuche (Analysator)
| Symptom | Ursache / Maßnahme |
|---|---|
| `--scan` findet nichts | Kabel/Hub; `-l 5` zeigt, ob die Firmware fehlt oder USB nicht ansprechbar ist |
| Aufnahme leer oder nur Nullen | GND fehlt, falsche Ader oder Pegel unter der Eingangsschwelle (Multimeter, Schritt 3) |
| Datei fehlt nach der Aufnahme | Mit Ctrl-C abgebrochen; feste Dauer mit `--time` verwenden |
| Viele Rahmenfehler | Polarität oder Baudrate falsch (Dekoder: invertiert, 9600) |
| Poll auf M statt B | Adern vertauscht, `--machine`/`--module` tauschen |
