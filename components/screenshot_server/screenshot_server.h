#pragma once

#include "esphome/core/component.h"
#include "esphome/components/display/display_buffer.h"
#include "esphome/components/web_server_base/web_server_base.h"

namespace esphome {
namespace screenshot_server {

class ScreenshotServer : public Component {
 public:
  void set_display(display::DisplayBuffer *display) { display_ = display; }
  void set_web_server(web_server_base::WebServerBase *web_server) { web_server_ = web_server; }

  void setup() override;
  
  // Set setup priority to load after the web server and display initialize
  float get_setup_priority() const override { return setup_priority::LATE; }

 protected:
  display::DisplayBuffer *display_{nullptr};
  web_server_base::WebServerBase *web_server_{nullptr};
};

}  // namespace screenshot_server
}  // namespace esphome
