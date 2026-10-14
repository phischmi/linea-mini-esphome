# Roadmap

Jede Phase endet mit einem Review durch den Nutzer. Writes erst ab Phase 2 und nur nach Freigabe.

## Phase 0: Wissensbasis und Gerüst
- [x] Protokoll- und Hardware-Doku mit Quellen/Status
- [x] Fixtures aus Mitschnitten, Prüfsummen geprüft
- [x] Repo-Gerüst
- [x] Lizenz: GPL-3.0
- [x] MQTT-Broker vorhanden (Mosquitto in Home Assistant)
- [ ] Offene Entscheidungen klären (Board: ESP32 vorhanden, Typ prüft der Nutzer zuhause; GPIO; CN11-Gegenstecker noch zu besorgen)

## Phase 1: Sniffer (passiv)
- [x] `examples/sniffer.yaml` (zwei Empfangs-UARTs parallel zu CN11, nur lauschen), Anleitung `docs/guides/sniffer.md`, Auswertung `tools/parse_sniffer_log.py`
- [ ] Sniffer aufbauen und an der Maschine testen (Pegel und Masse zuerst messen)
- [ ] Eigene Mitschnitte (zuerst mit Original-Modul, Gateway-FW v7.4.2): Boot, Idle, Brew, Steam, Standby/Wake, Backflush
- [ ] Offene Fragen O1–O11 mit echten Daten schließen
- [ ] Pegel an CN11 messen, Loopback-Test

## Phase 2: Komponente read-only
- [x] Hostseitig testbarer Frame-Parser (R/Z/X/W) gegen `tests/fixtures/` (`components/linea_mini/gicar_protocol.*`, Profil in `protocol_profile.h`)
- [x] Nicht blockierender, rein lesender Zustandsautomat: Boot-X-Probe, Polling 760 ms (`controller.*`)
- [ ] ESPHome-Anbindung (Schema, UART-Transport, Logging)
- [ ] Sensoren: Temperatur, Brühen, Pumpe, Heizung, Dampf, Standby, Verbunden
- [ ] Original-Modul entfernt, ESP als alleiniger Master

## Phase 3: Schreiben (nach Freigabe je Register)
- [ ] Solltemperatur 0x0007 (Grenzen 88–96 °C laut Quelle, Schritt prüfen)
- [ ] Dampf an/aus 0x00E1
- [ ] Standby/Wake 0x0000 inkl. Dampf-Wiederherstellung
- [ ] Pre-Infusion (erst Einheit klären)

## Phase 4: Reinigung und Brew-Stop
- [ ] Backflush: 6 Pulse, dann Poll auf ~10 s
- [ ] Brew-Stop per Standby-Toggle (ACK-gesteuert, garantierter Wake)

## Phase 5: BBW, HA, MQTT
- [ ] **5a, Brew-by-Weight über Home Assistant:** Waage (Acaia Lunar, vom Nutzer bestätigt) hängt an der vorhandenen HA-Integration. HA liefert das Gewicht, der ESP stoppt den Shot über den Brew-Stop (Phase 4). Latenz HA→ESP messen (Ziel: Stoppzeitpunkt mit Vorhalt/Offset einstellbar).
- [ ] **5b, direkte BLE-Kopplung ESP32 ↔ Waage (Ziel):** Gewicht per BLE-Notify im ESP, Stopp ohne Umweg über HA. Offene Punkte: Acaia-Protokoll der Lunar (Handshake, Gewichtsframes) ist hier **U** und erst zu recherchieren (nicht aus unlizenzierten Quellen kopieren); ESPHome hat keine fertige Acaia-Komponente (zu prüfen); WLAN+BLE-Koexistenz auf dem vorhandenen ESP32 (laut Quelle störte BLE-Scan ein Software-UART, Hardware-UART war unempfindlich); evtl. eigene `ble_client`-Komponente statt Lambda.
- [ ] Alternativ/Ergänzung: BLE-Waagen Felicita Arc, Bookoo Themis Ultra über `ble_client`-Sensor
- [ ] HA-Dashboard über die native ESPHome-API. **MQTT nur als Rückfall** (Mosquitto in HA vorhanden), falls die ESPHome-Umsetzung nicht wie erwartet funktioniert; wird nicht parallel gebaut
- [ ] Auto-Standby und Zeitplan im ESP (laufen nicht auf der Maschine)
