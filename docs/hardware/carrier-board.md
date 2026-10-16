# Trägerplatine (Entwurf, noch nicht begonnen)

Die Platine nimmt das **Seeed XIAO ESP32-S3** auf und sitzt in der Maschine am CN11. Dieses Dokument sammelt die Anforderungen; der Schaltplan entsteht erst nach dem Sniffer-Test und der Pegelmessung (siehe `docs/guides/sniffer.md`, `docs/hardware/cn11-interface.md`).
Status: **U** = nicht geprüft / noch zu klären.

## Entscheidungen
- Das XIAO wird aufgesteckt oder aufgelötet (Lötfahnen und Pins, laut Hersteller; **U**, Footprint und Maße im Seeed-Datenblatt prüfen, ggf. KiCad-Bibliothek von Seeed verwenden).
- Programmierung per USB-C am XIAO; Zugang von außen über ein Kabel oder OTA (offen).

## Anforderungen
1. **Eingang CN11, Versorgung 12 V:** Verpolschutz, rückstellende Sicherung, Abwärtsregler 12 V → 5 V mit Reserve für WLAN-Spitzen (≈ 500 mA). Die Belastbarkeit des CN11-Zweigs ist **nicht dokumentiert (U)**; messen oder konservativ auslegen.
2. **Speisung des XIAO:** über den 5-V-Anschluss des XIAO. Ob und wie das XIAO USB und externe 5 V entkoppelt, im Seeed-Schaltplan prüfen (**U**); sonst Schottky-Diode oder Lötbrücke vorsehen, damit nie zwei Quellen gleichzeitig einspeisen.
3. **Leitung Maschine → ESP (CN11 TXD → D7):** Pegel ca. 2,5 V, idle-low. Je nach Messung: direkt, mit Pull-up nach 3,3 V oder über einen Puffer (z. B. 74LVC1G17 an 3,3 V, nicht invertierend). Alle drei als Bestückungsoptionen (0-Ω-Brücken, unbestückte Plätze) vorsehen.
4. **Leitung ESP → Maschine (D6 → CN11 RXD):** 300 Ω in Serie.
5. **ESD-Schutz** (TVS) auf beiden Signalleitungen.
6. **Zwei Anschlüsse parallel** (Maschine / Original-Modul), damit Sniffer und Original-Modul gleichzeitig laufen; Original-Modul später abziehen.
7. **Antenne:** Die externe Antenne des XIAO nach außen aus dem Metallgehäuse führen oder an eine Kunststoffstelle setzen. Antennenseite frei von Kupfer und Bauteilen halten.
8. **Messpunkte** (GND, 3,3 V, 5 V, D6, D7), Status-LED optional (das XIAO hat eine eigene).
9. **Temperatur:** Einbauort neben dem Boiler messen; Betriebsbereich des XIAO und des Reglers im Datenblatt prüfen (**U**).

## Offene Punkte
- **Steckverbinder:** sehr wahrscheinlich TE AMPMODU MOD II, 2,54 mm (Angabe des Nutzers, U). Zwei Möglichkeiten: (a) die Trägerplatine bekommt denselben Steckverbinder wie das Original-Modul, dann passt das **vorhandene Kabel** und das Original-Modul bleibt als Rückfall steckbar; (b) ein eigener Gegenstecker zur CN11-Wanne. Dafür das Kabel an beiden Enden prüfen: Typ, Polzahl, Raster, Kodierung/Polarisation, Tiefe der Wanne. Teilenummern erst nach dem Nachmessen am Original bestimmen, nicht raten.
- Ergebnis der Pegelmessung (siehe Punkt 3).
- Einbauort und Platzbedarf in der Maschine, Befestigung.
- Ergebnis der WLAN-Messung am Einbauort (Sensor `wifi_signal` im Beispiel ergänzen).
