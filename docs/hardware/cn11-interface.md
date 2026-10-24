# CN11: Hardware-Schnittstelle zur GICAR-Hauptplatine

Status: **V** (Mitschnitt/Live beim Autor), **U** (unverifiziert, an eigener Maschine messen), **W** (Widerspruch). Quelle: LMC `HARDWARE_INTERFACE.md`, `docs/WIRING.md`, `CLAUDE.md`, `gicar.h`/`gicar.cpp`-Kommentare; Foto-Pinout `reference/lm_mini/photos/` (nicht übernommen).

## Stecker
CN11 (4 Pins) auf der GICAR-Platine (Bezeichnung laut Quelle: E.1.059). Hier hängt das Original-IoT-Modul.

| Pin | Signal laut Aufdruck | Richtung | Pegel | Status |
|---|---|---|---|---|
| 1 | GND | – | 0 V | V |
| 2 | RXD (Maschine empfängt) | Gateway → Maschine | ca. 2,5 V LVCMOS | V (Autor), U (eigene Messung) |
| 3 | TXD (Maschine sendet) | Maschine → Gateway | ca. 2,5 V LVCMOS | V (Autor), U (eigene Messung) |
| 4 | +12 V | Maschine → Gateway | 12 V | V (Autor) |

## Steckverbinder
- **Angabe des Nutzers:** Buchse und Wanne (Stiftleiste mit Kragen) auf den Platinen sind „sehr wahrscheinlich“ **TE Connectivity AMPMODU MOD II** (U, nicht von mir geprüft; keine meiner Quellen nennt den Typ). Dieser Typ hat nach meinem Wissen 2,54 mm Raster und 0,64 mm Vierkantstifte (U, Maße im TE-Datenblatt und am Original nachmessen).
- **Das vorhandene Kabel hat an beiden Enden dieselbe Buchse** (Angabe des Nutzers). Für die Tests steckt die Buchse auf eine **4er-Stiftleiste auf dem Breadboard**; für Dupont-Verbindungen gilt dasselbe Raster (U). Prüfe, ob Buchse und einzelne Dupont-Gehäuse sauber und fest sitzen (Kragen der Wanne).
- **Belegung am Modul-Ende (CN4 am Original):** Pin 1 (Aufdruck „1“) = **GND** (vom Nutzer gemessen), Pin 4 (gegenüber, quadratisch umrandet) = **+12 V** (aus Durchgang zur Eingangsdiode abgeleitet, U), Pins 2 und 3 = Signale (Zuordnung offen). Das entspricht der CN11-Nummerierung oben; das Kabel ist vermutlich 1:1 verdrahtet (U).
- **Pin-Zuordnung des Kabels** (gerade oder gekreuzt) durchmessen und festhalten. Für das Modul-Ende gilt die dort gemessene Nummerierung, nicht die CN11-Nummerierung oben.
- **Vorsicht, Pin 4 führt +12 V** und liegt direkt neben TXD (Pin 3). Ein verrutschter Dupont-Stecker kann 12 V auf einen ESP-Eingang legen und das Board zerstören. Für Sniffer und lesende Tests wird Pin 4 **nicht angeschlossen**: Nur GND, TXD und RXD verbinden, Pin 4 frei lassen oder abisolieren.
- **Immer Pin 1 durchmessen**, bevor du etwas verbindest: Durchgang zwischen GND der Platine und dem Pin, den du für GND hältst (Maschine aus, Netzstecker gezogen). Die Beschriftung kann vertauscht sein, und eine Kodierung des Steckers ist nicht gesichert.
- Dupont-Verbindungen können wackeln und dadurch Bitfehler erzeugen. Für längere Aufnahmen fixieren (Schrumpfschlauch, Kabelbinder, Zugentlastung).
- **Dauerhaft (Entscheidung des Nutzers):** Die Trägerplatine bekommt eine **AMPMODU-MOD-II-Wanne**, in die das vorhandene Kabel gesteckt wird. Die Belegung der Wanne muss der des Original-Moduls entsprechen (am Modul-Ende des Kabels gemessen). So bleibt das Original-Modul als Rückfall durch Umstecken des Kabels jederzeit nutzbar. Details in `carrier-board.md`.

## Elektrische Eigenschaften
- Aufdruck/Doku sprechen von „RS-232“, gemessen sind aber 2,5-V-LVCMOS-Pegel (V, Autor). **Kein RS-232-Pegelwandler** (±12 V) anschließen: schädigt die Platine.
- Leitungen sind **idle-low**; die UART muss rx und tx **invertieren** (V: ohne Inversion nur Rauschen, mit Inversion saubere Frames).
- 9600 8N1 (V). Siehe `docs/protocol/overview.md`.

## Beschaltung des Autors (U für mich, nicht nachgemessen)
- 300 Ω in Serie auf RX- und TX-Leitung (Strombegrenzung ca. 10 mA bei Kurzschluss).
- 10 kΩ Pull-up nach 3,3 V an der ESP-RX-Leitung, damit der 2,5-V-High-Pegel über der VIH-Schwelle des ESP32 liegt (laut Quelle ~2,47 V beim S3). Der Pull-up hebt nur den Pegel, er invertiert nicht.
- Versorgung: Buck-Wandler von CN11-+12 V auf 3,3 V (ESP direkt an 3V3, bypasst Onboard-Regler; USB nicht gleichzeitig) oder auf 5 V (je nach Board).
- Das ist **nicht** geprüft: Idle-Pegel, High-Pegel und Treiberart (Push-Pull oder Open-Drain?) an der eigenen Maschine messen, bevor die Schaltung final aufgebaut wird (siehe O11).

**W in der Quelle:** `HARDWARE_INTERFACE.md` schreibt „CN11 TXD idle high bei 2,5 V“, `WIRING.md` und der Code „idle-low“. Mit dem nötigen `invert=true` ist idle-low plausibel; die Aussage „idle high“ ist vermutlich falsch (U, messen).

## Vertauschte TXD/RXD-Beschriftung
Beim Autor waren die TXD/RXD-Labels am CN11-Stecker **vertauscht** (V, per Loopback am Stecker bestätigt). Außerdem hat ein passiver Sniffer zunächst die eigene Poll-Leitung als „TX“ gedeutet. Regel: Nicht den Labels trauen, sondern dekodierten Frames.

**Loopback-Test (vor dem Anschließen an die Maschine):**
1. Stecker von der Maschine getrennt lassen. Beide Signaladern am Maschinenende der Leitung brücken.
2. ESP sendet z. B. `R40000023DB`, soll exakt dasselbe zurückempfangen.
3. Danach anschließen. Erscheinen innerhalb weniger Sekunden saubere R-Frames (81 Zeichen), stimmt die Belegung; sonst beide Signalleitungen tauschen. Wegen der Serienwiderstände ist Vertauschen ungefährlich (laut Quelle).

## Versorgung
- +12 V von CN11 Pin 4. Strombedarf des ESP32 mit WLAN bis ~500 mA, Buck ≥ 0,8 A (Quelle nennt 0,8 A als Mindestgröße). Belastbarkeit des CN11-Zweigs: **U**, unbekannt (Platine/Sicherung nicht dokumentiert).
- Buck vor dem Anschluss des ESP auf Sollspannung trimmen, unter Last (WLAN aktiv) nachmessen.
- Ausfall von Platinen-Pins (Beobachtung des Autors): Auf manchen GPIO-Paaren kam nichts an (GPIO16/17 am C6, GPIO43 am S3). Pins daher nur mit Loopback prüfen, bevor die Beschaltung verlötet wird; GPIO-Wahl steht noch aus.

## Board und Pins (Entscheidung des Nutzers: Seeed XIAO ESP32-S3)
| Funktion | XIAO-Pin | GPIO | Anmerkung |
|---|---|---|---|
| ESP-TX → CN11 RXD (Maschine empfängt) | D6 | 43 | über 300 Ω in Serie |
| ESP-RX ← CN11 TXD (Maschine sendet) | D7 | 44 | Pegelanpassung siehe oben |
| Sniffer Kanal B / M (nur Eingänge) | D6 / D7 | 43 / 44 | dieselben Leitungen, im Sniffer nur gelesen |
| GND | GND | – | gemeinsame Masse mit CN11 Pin 1 |

- Die Zuordnung D0–D10 → GPIO stammt aus der ESPHome-Boarddefinition (`seeed_xiao_esp32s3`: D6 = 43, D7 = 44). V gegenüber ESPHome, U gegenüber dem Seeed-Datenblatt (nicht abgeglichen).
- GPIO43/44 sind die Standard-UART0-Pins. Deshalb steht in den Beispielen `logger: hardware_uart: USB_SERIAL_JTAG`; das Log läuft über den USB-C-Anschluss.
- Zu meiden: D2 (GPIO3, Strapping), BOOT (GPIO0), USB-Pins GPIO19/20 (nicht herausgeführt), Octal-PSRAM-Pins GPIO35–37 (nicht herausgeführt, U).
- Das Board hat nur einen nativen USB-Anschluss (keine UART-Brücke) und einen Anschluss für eine externe Antenne (U.FL, U).
- Loopback-Test (siehe oben) vor dem Anschluss an die Maschine: D6 mit D7 verbinden, Firmware sendet `R40000023DB`, muss dasselbe zurückempfangen.

## Sicherheit
Hinter dem Stecker liegt Netzspannungstechnik. Nur bei gezogenem Netzstecker arbeiten, Leitungen weg von Heizelementen und Netzverdrahtung führen. Das Original-Modul bleibt als Rückfall erhalten (Stecker umstecken).
