import esphome.codegen as cg
from esphome.components import text_sensor
import esphome.config_validation as cv
from esphome.const import ENTITY_CATEGORY_DIAGNOSTIC

from . import CONF_LINEA_MINI_ID, LineaMiniComponent

DEPENDENCIES = ["linea_mini"]

CONF_SERIAL_NUMBER = "serial_number"

CONFIG_SCHEMA = {
    cv.GenerateID(CONF_LINEA_MINI_ID): cv.use_id(LineaMiniComponent),
    cv.Optional(CONF_SERIAL_NUMBER): text_sensor.text_sensor_schema(
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC, icon="mdi:identifier"
    ),
}


async def to_code(config):
    hub = await cg.get_variable(config[CONF_LINEA_MINI_ID])
    if conf := config.get(CONF_SERIAL_NUMBER):
        sens = await text_sensor.new_text_sensor(conf)
        cg.add(hub.set_serial_number_text_sensor(sens))
