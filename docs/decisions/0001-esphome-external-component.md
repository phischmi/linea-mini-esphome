# 0001: ESPHome External Component statt eigener Firmware

Status: vorgeschlagen (2026-10-14)

## Kontext
Das Original-IoT-Modul (Cloud-Zwang) soll durch einen ESP32 mit ESPHome ersetzt werden. Die Steuerung läuft über die native ESPHome-API zu Home Assistant, optional MQTT. Es gibt keine fertige ESPHome-Komponente für die Linea Mini (Stand Recherche; nur die verwandte `gicar` Komponente für Ascaso Baby T im Repo `antondlr/gicar-serial`, anderes Frameformat).

## Entscheidung
Eine **External Component** unter `components/linea_mini/` (Python-Schema + C++ `UARTDevice`/`PollingComponent`) mit einem nicht blockierenden Zustandsautomaten. Der Frame-Parser ist hostseitig testbar (reines C++ ohne ESPHome-Header).

## Begründung
- ESPHome liefert WLAN, API, OTA, MQTT und HA-Entitäten fertig; die Quelle (`linea-mini-controller`) baut dies mit PlatformIO und eigener MQTT-Schicht nach.
- `uart:` kennt `inverted: true` auf `rx_pin`/`tx_pin` (V, ESPH-Doku). Dass im ESP-IDF-Backend die Inversion in Hardware läuft (`uart_set_line_inverse`), ist U (Doku nennt es nicht; Quelle nutzt Hardware-UART mit Inversion live). Kein Software-UART nötig.
- `rx_buffer_size` ist auf 256 vorbelegt (V, ESPH); beim Brühen kommen >600 B/s, daher 1024–2048 setzen.
- Der Boot-Handshake darf `loop()` nicht blockieren (ESPHome-Watchdog/WLAN): Zustandsautomat statt `delay(2000)`/Blocking-Reads.
- Lizenzen: ESPHome-Python MIT, C/C++-Runtime GPLv3. Eine External Component, die ESPHome-Header einbindet, ist abgeleitet; Entscheidung des Nutzers: **GPL-3.0** (`LICENSE`).

## Konsequenzen
- Struktur: `components/linea_mini/{__init__.py, linea_mini.h, linea_mini.cpp}`; später ggf. `sensor.py`, `binary_sensor.py`, `climate.py`.
- Read-only ist Default; Writes werden als eigene, explizit freigegebene Services/Entities gebaut.
- Später: Brew-by-Weight. Die Waage des Nutzers ist eine Acaia Lunar; zuerst über die vorhandene HA-Integration, Ziel ist die direkte BLE-Kopplung im ESP32 (Roadmap Phase 5b). Alternativ-Waagen: Felicita Arc, Bookoo Themis Ultra über `ble_client`-Sensor mit Lambda. Dokumentation zu Notify + Lambda ist dünn (U); das Acaia-Protokoll ist noch nicht recherchiert (U).
- Transport zu HA: native ESPHome-API. MQTT (Mosquitto in HA vorhanden) ist nur ein **Rückfall**, falls die ESPHome-Umsetzung nicht wie erwartet funktioniert, und wird nicht parallel entwickelt.
- Alternative (verworfen): eigene PlatformIO-Firmware wie LMC. Mehr Aufwand für WLAN/OTA/HA, weniger Wiederverwendung.
