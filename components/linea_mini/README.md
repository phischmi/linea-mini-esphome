# components/linea_mini

Platzhalter für die ESPHome External Component (siehe `docs/decisions/0001-esphome-external-component.md`).

## Zweck
UART-Master für die GICAR-Platine der Linea Mini über CN11: Polling (760 ms), Frame-Parser, Zustandsautomat, Sensoren.

## TODO (nach Phase 1)
- Parser ohne ESPHome-Header, hostseitig gegen `tests/fixtures/` testbar
- Zustandsautomat statt blockierendem Boot-Handshake
- Read-only-Schalter als Default, Writes nur für freigegebene Register
- Sensoren/Entities, Konfig-Schema in `__init__.py`
