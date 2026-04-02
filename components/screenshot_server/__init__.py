import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import display, web_server_base
from esphome.const import CONF_ID

# Ensure standard components are loaded first
DEPENDENCIES = ['display', 'web_server_base']

# Create a namespace for the C++ code
screenshot_server_ns = cg.esphome_ns.namespace('screenshot_server')
ScreenshotServer = screenshot_server_ns.class_('ScreenshotServer', cg.Component)

# Define our YAML config keys
CONF_DISPLAY_ID = 'display_id'
CONF_WEB_SERVER_ID = 'web_server_id'

# Define the YAML schema
CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(ScreenshotServer),
    cv.Required(CONF_DISPLAY_ID): cv.use_id(display.DisplayBuffer),
    cv.Required(CONF_WEB_SERVER_ID): cv.use_id(web_server_base.WebServerBase),
}).extend(cv.COMPONENT_SCHEMA)

# Translate YAML to C++ code generation
async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    disp = await cg.get_variable(config[CONF_DISPLAY_ID])
    cg.add(var.set_display(disp))
    
    ws = await cg.get_variable(config[CONF_WEB_SERVER_ID])
    cg.add(var.set_web_server(ws))
