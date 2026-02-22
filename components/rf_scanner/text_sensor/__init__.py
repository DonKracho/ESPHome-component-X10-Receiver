import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import (
    CONF_ID,
)

from .. import rf_scanner_ns, RfScanner

DEPENDENCIES = ["rf_scanner"]

RfSensor = rf_scanner_ns.class_("RfSensor", cg.Component, text_sensor.TextSensor)

CONF_RFSCANNER_ID = "rf_scanner"
CONF_CHANNEL = "channel"
CONF_COMMAND = "command"
CONF_COMMAND_NAME = "command_name"
CONF_DEVICE_PAD = "device_pad"
CONF_MOTION_PAD = "motion_pad"
CONF_NUMBER_PAD = "number_pad"
UPDATE_INTERVAL = "never"

CONFIG_SCHEMA = cv.All(
    text_sensor.text_sensor_schema(RfSensor)
    .extend(
        {
            cv.GenerateID(CONF_RFSCANNER_ID): cv.use_id(RfScanner),
            cv.Optional(CONF_CHANNEL): text_sensor.text_sensor_schema(
            ),
            cv.Optional(CONF_COMMAND): text_sensor.text_sensor_schema(
            ),
            cv.Optional(CONF_COMMAND_NAME): text_sensor.text_sensor_schema(
            ),
            cv.Optional(CONF_DEVICE_PAD): text_sensor.text_sensor_schema(
            ),
            cv.Optional(CONF_MOTION_PAD): text_sensor.text_sensor_schema(
            ),
            cv.Optional(CONF_NUMBER_PAD): text_sensor.text_sensor_schema(
            ),
        }
    ).extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(
        config[CONF_ID],
    )
    await cg.register_parented(var, config[CONF_RFSCANNER_ID])
    await cg.register_component(var, config)
    await text_sensor.register_text_sensor(var, config)
    
    if CONF_CHANNEL in config:
        sens = await text_sensor.new_text_sensor(config[CONF_CHANNEL])
        cg.add(var.set_channel_sensor(sens))
    if CONF_COMMAND in config:
        sens = await text_sensor.new_text_sensor(config[CONF_COMMAND])
        cg.add(var.set_command_sensor(sens))
    if CONF_COMMAND_NAME in config:
        sens = await text_sensor.new_text_sensor(config[CONF_COMMAND_NAME])
        cg.add(var.set_command_name_sensor(sens))
    if CONF_DEVICE_PAD in config:
        sens = await text_sensor.new_text_sensor(config[CONF_DEVICE_PAD])
        cg.add(var.set_device_pad_sensor(sens))
    if CONF_MOTION_PAD in config:
        sens = await text_sensor.new_text_sensor(config[CONF_MOTION_PAD])
        cg.add(var.set_motion_pad_sensor(sens))
    if CONF_NUMBER_PAD in config:
        sens = await text_sensor.new_text_sensor(config[CONF_NUMBER_PAD])
        cg.add(var.set_number_pad_sensor(sens))
