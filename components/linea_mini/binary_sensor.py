import esphome.codegen as cg
from esphome.components import binary_sensor
import esphome.config_validation as cv
from esphome.const import DEVICE_CLASS_CONNECTIVITY, DEVICE_CLASS_RUNNING

from . import CONF_LINEA_MINI_ID, LineaMiniComponent

DEPENDENCIES = ["linea_mini"]

CONF_CONNECTED = "connected"
CONF_BREWING = "brewing"
CONF_PUMP = "pump"
CONF_HEATING = "heating"
CONF_STEAM = "steam"
CONF_STANDBY = "standby"

CONFIG_SCHEMA = {
    cv.GenerateID(CONF_LINEA_MINI_ID): cv.use_id(LineaMiniComponent),
    cv.Optional(CONF_CONNECTED): binary_sensor.binary_sensor_schema(
        device_class=DEVICE_CLASS_CONNECTIVITY
    ),
    cv.Optional(CONF_BREWING): binary_sensor.binary_sensor_schema(
        device_class=DEVICE_CLASS_RUNNING, icon="mdi:coffee"
    ),
    cv.Optional(CONF_PUMP): binary_sensor.binary_sensor_schema(
        device_class=DEVICE_CLASS_RUNNING, icon="mdi:pump"
    ),
    cv.Optional(CONF_HEATING): binary_sensor.binary_sensor_schema(
        device_class=DEVICE_CLASS_RUNNING, icon="mdi:radiator"
    ),
    cv.Optional(CONF_STEAM): binary_sensor.binary_sensor_schema(
        icon="mdi:kettle-steam"
    ),
    cv.Optional(CONF_STANDBY): binary_sensor.binary_sensor_schema(
        icon="mdi:power-sleep"
    ),
}


async def to_code(config):
    hub = await cg.get_variable(config[CONF_LINEA_MINI_ID])
    for key in (
        CONF_CONNECTED,
        CONF_BREWING,
        CONF_PUMP,
        CONF_HEATING,
        CONF_STEAM,
        CONF_STANDBY,
    ):
        if conf := config.get(key):
            sens = await binary_sensor.new_binary_sensor(conf)
            cg.add(getattr(hub, f"set_{key}_binary_sensor")(sens))
