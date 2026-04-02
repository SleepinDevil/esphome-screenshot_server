#include "screenshot_server.h"
#include "esphome/core/log.h"
#include <algorithm>

namespace esphome {
namespace screenshot_server {

static const char *const TAG = "screenshot_server";

void ScreenshotServer::setup() {
  if (!this->display_ || !this->web_server_) {
    ESP_LOGE(TAG, "Display or WebServer not configured!");
    this->mark_failed();
    return;
  }

  ESP_LOGCONFIG(TAG, "Setting up Screenshot Server endpoint at /screenshot.bmp");

  this->web_server_->get_server()->on("/screenshot.bmp", HTTP_GET, [this](AsyncWebServerRequest *request) {
    
    int width = this->display_->get_width();
    int height = this->display_->get_height();
    
    // 16-bit RGB565 requires 2 bytes per pixel, pad to multiple of 4
    int row_size = ((width * 2) + 3) & ~3; 
    size_t image_size = row_size * height;
    size_t header_size = 66; 
    size_t total_size = header_size + image_size;

    AsyncWebServerResponse *response = request->beginResponse("image/bmp", total_size, 
      [this, width, height, row_size, header_size, image_size](uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
        
        size_t bytes_written = 0;

        // 1. Write the BMP Header
        if (index < header_size) {
          uint8_t header[66] = {0};
          
          header[0] = 'B'; header[1] = 'M';
          uint32_t file_size = header_size + image_size;
          memcpy(&header[2], &file_size, 4);
          uint32_t offset = 66;
          memcpy(&header[10], &offset, 4);

          uint32_t dib_size = 40;
          memcpy(&header[14], &dib_size, 4);
          memcpy(&header[18], &width, 4);
          
          // Negative height flags image as Top-Down!
          int32_t neg_height = -height; 
          memcpy(&header[22], &neg_height, 4);
          
          uint16_t planes = 1;
          memcpy(&header[26], &planes, 2);
          uint16_t bpp = 16;
          memcpy(&header[28], &bpp, 2);
          uint32_t compression = 3; // BI_BITFIELDS
          memcpy(&header[30], &compression, 4);
          memcpy(&header[34], &image_size, 4);

          // RGB565 Bitmasks
          uint32_t r_mask = 0xF800;
          uint32_t g_mask = 0x07E0;
          uint32_t b_mask = 0x001F;
          memcpy(&header[54], &r_mask, 4);
          memcpy(&header[58], &g_mask, 4);
          memcpy(&header[62], &b_mask, 4);

          size_t header_bytes_to_write = std::min(maxLen, header_size - index);
          memcpy(buffer, header + index, header_bytes_to_write);
          bytes_written += header_bytes_to_write;
        }

        // 2. Stream the Raw Buffer Chunks
        if (index + bytes_written >= header_size) {
          size_t pixel_index = (index + bytes_written) - header_size;
          if (pixel_index < image_size) {
            size_t pixel_bytes_to_write = std::min(maxLen - bytes_written, image_size - pixel_index);
            
            uint8_t *fb = this->display_->get_buffer(); 
            if (fb != nullptr) {
              memcpy(buffer + bytes_written, fb + pixel_index, pixel_bytes_to_write);
              bytes_written += pixel_bytes_to_write;
            }
          }
        }
        return bytes_written;
    });
    
    response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    request->send(response);
  });
}

}  // namespace screenshot_server
}  // namespace esphome
