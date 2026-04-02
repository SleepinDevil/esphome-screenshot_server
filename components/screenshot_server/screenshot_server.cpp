#include "screenshot_server.h"
#include "esphome/core/log.h"
#include <string>

namespace esphome {
namespace screenshot_server {

static const char *const TAG = "screenshot_server";

// ESP-IDF requires a dedicated Handler class instead of a simple lambda function
class ScreenshotHandler : public AsyncWebHandler {
 public:
  ScreenshotHandler(ScreenshotServer *parent) : parent_(parent) {}

  bool canHandle(AsyncWebServerRequest *request) override {
    if (request->method() != HTTP_GET) return false;
    return request->url() == "/screenshot.bmp";
  }

  void handleRequest(AsyncWebServerRequest *request) override {
    int width = parent_->width_func_();
    int height = parent_->height_func_();
    
    int row_size = ((width * 2) + 3) & ~3; 
    size_t image_size = row_size * height;
    size_t header_size = 66; 
    size_t total_size = header_size + image_size;

    // 1. Generate the BMP Header
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

    // 2. Assemble the full file in PSRAM
    // std::string safely handles binary data and dynamically routes 
    // allocations this large (~460KB) straight to the ESP32-S3's PSRAM.
    std::string bmp_data;
    bmp_data.reserve(total_size);
    bmp_data.append((char*)header, header_size);

    uint8_t *fb = parent_->buffer_func_(); 
    if (fb != nullptr) {
      bmp_data.append((char*)fb, image_size);
    }

    // 3. Send to ESP-IDF Web Server
    AsyncWebServerResponse *response = request->beginResponse(200, "image/bmp", bmp_data);
    response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    request->send(response);
  }

 private:
  ScreenshotServer *parent_;
};

void ScreenshotServer::setup() {
  if (web_server_base::global_web_server_base == nullptr) {
    ESP_LOGE(TAG, "WebServerBase not found! Ensure web_server is enabled in YAML.");
    this->mark_failed();
    return;
  }

  if (!this->width_func_ || !this->height_func_ || !this->buffer_func_) {
    ESP_LOGE(TAG, "Display not correctly configured!");
    this->mark_failed();
    return;
  }

  ESP_LOGCONFIG(TAG, "Setting up Screenshot Server endpoint at /screenshot.bmp");

  // Register the custom handler, supporting both ESP-IDF and Arduino methods securely
#ifdef USE_ESP_IDF
  web_server_base::global_web_server_base->get_server()->add_handler(new ScreenshotHandler(this));
#else
  web_server_base::global_web_server_base->get_server()->addHandler(new ScreenshotHandler(this));
#endif
}

}  // namespace screenshot_server
}  // namespace esphome
