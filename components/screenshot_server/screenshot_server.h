#pragma once

#include "esphome/core/component.h"
#include "esphome/components/web_server_base/web_server_base.h"
#include "esphome/components/st7701s/st7701s.h"

namespace esphome {
namespace screenshot_server {

// MAGIC TRICK: This bypasses C++ access modifiers to unlock the protected hardware handle
class ST7701SAccessor : public st7701s::ST7701S {
public:
  esp_lcd_panel_handle_t get_handle() {
    return this->panel_handle_;
  }
};

class ScreenshotServer : public Component {
 public:
  // Accept the component and cast it to the ST7701S type
  template <typename T> void set_display(T *display) { 
    display_ = (st7701s::ST7701S*)display; 
  }

  void setup() override;
  float get_setup_priority() const override { return setup_priority::LATE; }

  st7701s::ST7701S *display_{nullptr};
};

}  // namespace screenshot_server
}  // namespace esphome
