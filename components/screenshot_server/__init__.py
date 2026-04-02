import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

# Ensure standard components are loaded first
DEPENDENCIES = ['display', 'web_server']

screenshot_server_ns = cg.esphome_ns.namespace('screenshot_server')
ScreenshotServer = screenshot_server_ns.class_('ScreenshotServer', cg.Component)

CONF_DISPLAY_ID = 'display_id'

# Relaxed Schema: Accept ANY component ID and let C++ figure it out
CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(ScreenshotServer),
    cv.Required(CONF_DISPLAY_ID): cv.use_id(), 
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    # Blindly pass the ID to C++
    disp = await cg.get_variable(config[CONF_DISPLAY_ID])
    cg.add(var.set_display(disp))
