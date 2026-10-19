# Trägerplatine (Entwurf, noch nicht begonnen)

Die Platine nimmt das **Seeed XIAO ESP32-S3** auf und sitzt in der Maschine am CN11. Dieses Dokument sammelt die Anforderungen; der Schaltplan entsteht erst nach dem Sniffer-Test und der Pegelmessung (siehe `docs/guides/sniffer.md`, `docs/hardware/cn11-interface.md`).
Status: **U** = nicht geprüft / noch zu klären.

## Entscheidungen
- **Drop-in-Gedanke (Vorschlag, aus den Fotos des Original-Moduls, siehe `original-module.md`):** Umriss, die vier Befestigungspunkte und die Lage des 4-poligen Anschlusses (CN4 am Original) des Original-Moduls übernehmen, damit die Platine an derselben Stelle sitzt. Maße mit dem Messschieber bestimmen.
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
- **Steckverbinder (entschieden):** Die Platine bekommt eine **TE AMPMODU MOD II Wanne** (Angabe des Nutzers, Typ U), in die das vorhandene Kabel passt; das Kabel hat an beiden Enden dieselbe Buchse. Die Belegung der Wanne wird so gewählt wie am Original-Modul. Offen: Teilenummer (Polzahl, Bauform stehend/liegend, Kodierung), Maße der Wanne am Original nachmessen und mit dem TE-Datenblatt abgleichen; keine Teilenummer raten.
- **Pin-Zuordnung:** Am Kabel durchmessen, ob die Adern gerade oder gekreuzt geführt sind; die Belegung der Platinenwanne entspricht dem, was am Modul-Ende des Kabels ankommt.
- Ergebnis der Pegelmessung (siehe Punkt 3).
- Einbauort und Platzbedarf in der Maschine, Befestigung.
- Ergebnis der WLAN-Messung am Einbauort (Sensor `wifi_signal` im Beispiel ergänzen).
