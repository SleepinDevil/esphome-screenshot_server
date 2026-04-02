#pragma once

#include "esphome/core/component.h"
#include "esphome/components/web_server_base/web_server_base.h"
#include <functional>

namespace esphome {
namespace screenshot_server {

class ScreenshotServer : public Component {
 public:
  template <typename T> void set_display(T *display) {
    this->width_func_ = [display]() -> int { return display->get_width(); };
    this->height_func_ = [display]() -> int { return display->get_height(); };
    this->buffer_func_ = [display]() -> uint8_t* { return (uint8_t*)display->get_buffer(); };
  }

  void setup() override;
  float get_setup_priority() const override { return setup_priority::LATE; }

  // Moved to public so the WebHandler can read them
  std::function<int()> width_func_;
  std::function<int()> height_func_;
  std::function<uint8_t*()> buffer_func_;
};

}  // namespace screenshot_server
}  // namespace esphome
