#include "screenshot_server.h"
#include "esphome/core/log.h"
#include <string>
#include "esp_lcd_panel_rgb.h"

namespace esphome {
namespace screenshot_server {

static const char *const TAG = "screenshot_server";

class ScreenshotHandler : public AsyncWebHandler {
 public:
  ScreenshotHandler(ScreenshotServer *parent) : parent_(parent) {}

  bool canHandle(AsyncWebServerRequest *request) {
    if (request->method() != HTTP_GET) return false;
#ifdef USE_ESP_IDF
    return request->url_to() == "/screenshot.bmp";
#else
    return request->url() == "/screenshot.bmp";
#endif
  }

  bool can_handle(AsyncWebServerRequest *request) {
    return canHandle(request);
  }

  void handleRequest(AsyncWebServerRequest *request) {
    int width = parent_->display_->get_width();
    int height = parent_->display_->get_height();
    
    int row_size = ((width * 2) + 3) & ~3; 
    size_t image_size = row_size * height;
    size_t header_size = 66; 
    size_t total_size = header_size + image_size;

    uint8_t header[66] = {0};
    header[0] = 'B'; header[1] = 'M';
    uint32_t file_size = total_size;
    memcpy(&header[2], &file_size, 4);
    uint32_t offset = 66;
    memcpy(&header[10], &offset, 4);

    uint32_t dib_size = 40;
    memcpy(&header[14], &dib_size, 4);
    memcpy(&header[18], &width, 4);
    
    int32_t neg_height = -height; 
    memcpy(&header[22], &neg_height, 4);
    
    uint16_t planes = 1;
    memcpy(&header[26], &planes, 2);
    uint16_t bpp = 16;
    memcpy(&header[28], &bpp, 2);
    uint32_t compression = 3; 
    memcpy(&header[30], &compression, 4);
    memcpy(&header[34], &image_size, 4);

    uint32_t r_mask = 0xF800;
    uint32_t g_mask = 0x07E0;
    uint32_t b_mask = 0x001F;
    memcpy(&header[54], &r_mask, 4);
    memcpy(&header[58], &g_mask, 4);
    memcpy(&header[62], &b_mask, 4);

    std::string bmp_data;
    bmp_data.reserve(total_size);
    bmp_data.append((char*)header, header_size);

    // -- EXTRACT THE RAW ESP-IDF HARDWARE BUFFER --
    esp_lcd_panel_handle_t handle = ((ST7701SAccessor*)parent_->display_)->get_handle();
    void *fb = nullptr;
    
    if (handle != nullptr) {
        // Grab the pointer directly from the LCD DMA engine
        esp_lcd_rgb_panel_get_frame_buffer(handle, 1, &fb);
    }
    
    if (fb != nullptr) {
      bmp_data.append((char*)fb, image_size);
    } else {
      ESP_LOGE(TAG, "Failed to extract hardware framebuffer from ST7701S!");
    }

    AsyncWebServerResponse *response = request->beginResponse(200, "image/bmp", bmp_data);
    response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    request->send(response);
  }

  void handle_request(AsyncWebServerRequest *request) {
    handleRequest(request);
  }

 private:
  ScreenshotServer *parent_;
};

void ScreenshotServer::setup() {
  if (web_server_base::global_web_server_base == nullptr) {
    ESP_LOGE(TAG, "WebServerBase not found!");
    this->mark_failed();
    return;
  }

  if (this->display_ == nullptr) {
    ESP_LOGE(TAG, "Display not configured!");
    this->mark_failed();
    return;
  }

  ESP_LOGCONFIG(TAG, "Setting up hardware-level Screenshot Server endpoint at /screenshot.bmp");
  web_server_base::global_web_server_base->get_server()->addHandler(new ScreenshotHandler(this));
}

}  // namespace screenshot_server
}  // namespace esphome
