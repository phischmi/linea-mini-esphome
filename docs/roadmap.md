# Roadmap

Jede Phase endet mit einem Review durch den Nutzer. Writes erst ab Phase 2 und nur nach Freigabe.

## Phase 0: Wissensbasis und Gerüst
- [x] Protokoll- und Hardware-Doku mit Quellen/Status
- [x] Fixtures aus Mitschnitten, Prüfsummen geprüft
- [x] Repo-Gerüst
- [x] Lizenz: GPL-3.0
- [ ] Offene Entscheidungen klären (Board: ESP32 vorhanden, Typ prüft der Nutzer zuhause; GPIO, MQTT, Gegenstecker)

## Phase 1: Sniffer (passiv)
- [ ] `examples/sniffer.yaml` lauffähig machen (zwei UARTs parallel zu CN11, nur lauschen)
- [ ] Eigene Mitschnitte (zuerst mit Original-Modul, Gateway-FW v7.4.2): Boot, Idle, Brew, Steam, Standby/Wake, Backflush
- [ ] Offene Fragen O1–O11 mit echten Daten schließen
- [ ] Pegel an CN11 messen, Loopback-Test

## Phase 2: Komponente read-only
- [ ] Hostseitig testbarer Frame-Parser (R/Z/X/W) gegen `tests/fixtures/`
- [ ] Nicht blockierender Zustandsautomat: Boot-X-Probe, Polling 760 ms
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
- [ ] BLE-Waage (Felicita Arc, Bookoo Themis Ultra) über `ble_client`
- [ ] Brew-by-Weight
- [ ] HA-Dashboard, optional MQTT
- [ ] Auto-Standby und Zeitplan im ESP (laufen nicht auf der Maschine)
