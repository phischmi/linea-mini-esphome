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

## Sicherheit
Hinter dem Stecker liegt Netzspannungstechnik. Nur bei gezogenem Netzstecker arbeiten, Leitungen weg von Heizelementen und Netzverdrahtung führen. Das Original-Modul bleibt als Rückfall erhalten (Stecker umstecken).
