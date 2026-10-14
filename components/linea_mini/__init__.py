"""ESPHome external component for the La Marzocco Linea Mini (read-only).

Talks to the GICAR main board over CN11 using a UART. This version only reads: it sends
the boot probe and read requests, never a write command. See docs/protocol/.
"""

import esphome.codegen as cg
from esphome.components import uart
import esphome.config_validation as cv
from esphome.const import CONF_ID

DEPENDENCIES = ["uart"]
AUTO_LOAD = ["sensor", "binary_sensor", "text_sensor"]
CODEOWNERS = []

CONF_LINEA_MINI_ID = "linea_mini_id"
CONF_SEND_PROBE = "send_probe"

linea_mini_ns = cg.esphome_ns.namespace("linea_mini")
LineaMiniComponent = linea_mini_ns.class_(
    "LineaMiniComponent", cg.Component, uart.UARTDevice
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(LineaMiniComponent),
            # The original gateway sends one X probe after the machine boots (docs/protocol/
            # sequences.md, 1). Set false to skip it and poll right away (open question O9).
            cv.Optional(CONF_SEND_PROBE, default=True): cv.boolean,
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA)
)

# CN11 runs 9600 8N1. Both lines are idle-low, so set `inverted: true` on rx_pin and tx_pin
# of the uart (cannot be checked here). A tx_pin is required for the read requests.
FINAL_VALIDATE_SCHEMA = uart.final_validate_device_schema(
    "linea_mini",
    baud_rate=9600,
    require_rx=True,
    require_tx=True,
    data_bits=8,
    parity="NONE",
    stop_bits=1,
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
    cg.add(var.set_send_probe(config[CONF_SEND_PROBE]))
