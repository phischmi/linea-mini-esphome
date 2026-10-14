import esphome.codegen as cg
from esphome.components import sensor
import esphome.config_validation as cv
from esphome.const import (
    DEVICE_CLASS_TEMPERATURE,
    ENTITY_CATEGORY_DIAGNOSTIC,
    STATE_CLASS_MEASUREMENT,
    STATE_CLASS_TOTAL_INCREASING,
    UNIT_CELSIUS,
)

from . import CONF_LINEA_MINI_ID, LineaMiniComponent

DEPENDENCIES = ["linea_mini"]

CONF_COFFEE_TEMPERATURE = "coffee_temperature"
CONF_SETPOINT_TEMPERATURE = "setpoint_temperature"
CONF_FRAMES_OK = "frames_ok"
CONF_FRAMES_BAD = "frames_bad"

CONFIG_SCHEMA = {
    cv.GenerateID(CONF_LINEA_MINI_ID): cv.use_id(LineaMiniComponent),
    cv.Optional(CONF_COFFEE_TEMPERATURE): sensor.sensor_schema(
        unit_of_measurement=UNIT_CELSIUS,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_TEMPERATURE,
        state_class=STATE_CLASS_MEASUREMENT,
    ),
    cv.Optional(CONF_SETPOINT_TEMPERATURE): sensor.sensor_schema(
        unit_of_measurement=UNIT_CELSIUS,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_TEMPERATURE,
        state_class=STATE_CLASS_MEASUREMENT,
    ),
    cv.Optional(CONF_FRAMES_OK): sensor.sensor_schema(
        accuracy_decimals=0,
        state_class=STATE_CLASS_TOTAL_INCREASING,
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
    ),
    cv.Optional(CONF_FRAMES_BAD): sensor.sensor_schema(
        accuracy_decimals=0,
        state_class=STATE_CLASS_TOTAL_INCREASING,
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
    ),
}


async def to_code(config):
    hub = await cg.get_variable(config[CONF_LINEA_MINI_ID])
    for key in (
        CONF_COFFEE_TEMPERATURE,
        CONF_SETPOINT_TEMPERATURE,
        CONF_FRAMES_OK,
        CONF_FRAMES_BAD,
    ):
        if conf := config.get(key):
            sens = await sensor.new_sensor(conf)
            cg.add(getattr(hub, f"set_{key}_sensor")(sens))
