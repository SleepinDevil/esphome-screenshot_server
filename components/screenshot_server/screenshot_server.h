#pragma once

#include "esphome/core/component.h"
#include "esphome/components/display/display_buffer.h"
#include "esphome/components/web_server_base/web_server_base.h"

namespace esphome {
namespace screenshot_server {

class ScreenshotServer : public Component {
 public:
  void set_display(display::DisplayBuffer *display) { display_ = display; }

  void setup() override;
  float get_setup_priority() const override { return setup_priority::LATE; }

  display::DisplayBuffer *display_{nullptr};
};

}  // namespace screenshot_server
}  // namespace esphome
