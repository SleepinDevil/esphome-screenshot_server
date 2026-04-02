import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

DEPENDENCIES = ['display', 'web_server']

screenshot_server_ns = cg.esphome_ns.namespace('screenshot_server')
ScreenshotServer = screenshot_server_ns.class_('ScreenshotServer', cg.Component)

CONF_DISPLAY_ID = 'display_id'

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(ScreenshotServer),
    cv.Required(CONF_DISPLAY_ID): cv.use_id(cg.Component), 
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    disp = await cg.get_variable(config[CONF_DISPLAY_ID])
    cg.add(var.set_display(disp))
