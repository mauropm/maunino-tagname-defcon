/*
 * SH1106 OLED Display Driver Implementation for ESP-IDF 5
 * I2C Interface for ESP32-C6
 */

#include "sh1106.h"
#include <string.h>
#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "SH1106";

// Framebuffer for display
static uint8_t sh1106_buffer[SH1106_WIDTH * SH1106_HEIGHT / 8];

// Font data (simplified 5x7 font)
static const uint8_t font5x7[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // Space
    {0x00, 0x00, 0x5F, 0x00, 0x00}, // !
    {0x00, 0x07, 0x00, 0x07, 0x00}, // "
    {0x14, 0x7F, 0x14, 0x7F, 0x14}, // #
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // $
    {0x23, 0x13, 0x08, 0x64, 0x62}, // %
    {0x36, 0x49, 0x55, 0x22, 0x50}, // &
    {0x00, 0x05, 0x03, 0x00, 0x00}, // '
    {0x00, 0x1C, 0x22, 0x41, 0x00}, // (
    {0x00, 0x41, 0x22, 0x1C, 0x00}, // )
    {0x14, 0x08, 0x3E, 0x08, 0x14}, // *
    {0x08, 0x08, 0x3E, 0x08, 0x08}, // +
    {0x00, 0x50, 0x30, 0x00, 0x00}, // ,
    {0x08, 0x08, 0x08, 0x08, 0x08}, // -
    {0x00, 0x60, 0x60, 0x00, 0x00}, // .
    {0x20, 0x10, 0x08, 0x04, 0x02}, // /
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x46, 0x49, 0x45, 0x43, 0x00}, // 9
    {0x00, 0x36, 0x36, 0x00, 0x00}, // :
    {0x00, 0x56, 0x36, 0x00, 0x00}, // ;
    {0x08, 0x14, 0x22, 0x41, 0x00}, // <
    {0x14, 0x14, 0x14, 0x14, 0x14}, // =
    {0x00, 0x41, 0x22, 0x14, 0x08}, // >
    {0x02, 0x01, 0x51, 0x09, 0x06}, // ?
    {0x32, 0x49, 0x79, 0x41, 0x3E}, // @
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // A
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
    {0x7F, 0x41, 0x41, 0x41, 0x3E}, // D
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // F
    {0x3E, 0x41, 0x41, 0x51, 0x32}, // G
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // I
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // J
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // K
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // L
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // M
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // N
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // P
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // Q
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // R
    {0x46, 0x49, 0x49, 0x49, 0x31}, // S
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // T
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // V
    {0x3F, 0x40, 0x38, 0x40, 0x3F}, // W
    {0x63, 0x14, 0x08, 0x14, 0x63}, // X
    {0x07, 0x08, 0x70, 0x08, 0x07}, // Y
    {0x61, 0x51, 0x49, 0x45, 0x43}, // Z
};

int16_t sh1106_get_width(void) {
    return SH1106_WIDTH;
}

int16_t sh1106_get_height(void) {
    return SH1106_HEIGHT;
}

static esp_err_t sh1106_write_command(sh1106_t *display, uint8_t cmd) {
    uint8_t buffer[2] = {0x00, cmd}; // Co=0, D/C=0
    return i2c_master_transmit(display->i2c_dev, buffer, 2, portMAX_DELAY);
}

esp_err_t sh1106_init(sh1106_t *display, int sda_io, int scl_io, int reset_io, i2c_port_t i2c_port) {
    esp_err_t ret;
    
    memset(display, 0, sizeof(sh1106_t));
    display->sda_io_num = sda_io;
    display->scl_io_num = scl_io;
    display->reset_io_num = reset_io;
    display->vcc_state = SH1106_SWITCH_CAP_VCC;
    
    // Configure I2C bus
    i2c_master_bus_config_t i2c_bus_cfg = {
        .i2c_port = i2c_port,
        .scl_io_num = scl_io,
        .sda_io_num = sda_io,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    
    ret = i2c_new_master_bus(&i2c_bus_cfg, &display->i2c_bus);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create I2C bus: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Try to detect display at both common I2C addresses
    uint8_t test_addresses[] = {0x3C, 0x3D};
    bool found = false;
    
    for (int i = 0; i < 2; i++) {
        display->i2c_address = test_addresses[i];
        
        i2c_device_config_t i2c_dev_cfg = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address = display->i2c_address,
            .scl_speed_hz = 100000, // Slower speed for better reliability
        };
        
        ret = i2c_master_bus_add_device(display->i2c_bus, &i2c_dev_cfg, &display->i2c_dev);
        if (ret == ESP_OK) {
            // Try to send a command to verify communication
            ret = sh1106_write_command(display, SH1106_DISPLAY_OFF);
            if (ret == ESP_OK) {
                ESP_LOGI(TAG, "Display found at I2C address 0x%02X", display->i2c_address);
                found = true;
                break;
            }
            i2c_master_bus_rm_device(display->i2c_dev);
        }
    }
    
    if (!found) {
        ESP_LOGE(TAG, "No display found at addresses 0x3C or 0x3D");
        i2c_del_master_bus(display->i2c_bus);
        return ESP_ERR_NOT_FOUND;
    }
    
    // Re-add device with faster speed after successful detection
    i2c_device_config_t i2c_dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = display->i2c_address,
        .scl_speed_hz = 400000,
    };
    i2c_master_bus_rm_device(display->i2c_dev);
    ret = i2c_master_bus_add_device(display->i2c_bus, &i2c_dev_cfg, &display->i2c_dev);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to re-add I2C device: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Configure reset pin if specified
    if (reset_io >= 0) {
        gpio_config_t io_conf = {
            .pin_bit_mask = (1ULL << reset_io),
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        gpio_config(&io_conf);
        
        // Reset sequence
        gpio_set_level(reset_io, 1);
        vTaskDelay(pdMS_TO_TICKS(1));
        gpio_set_level(reset_io, 0);
        vTaskDelay(pdMS_TO_TICKS(10));
        gpio_set_level(reset_io, 1);
    }
    
    vTaskDelay(pdMS_TO_TICKS(10));
    
    // Initialization sequence for 128x64 SH1106 display
    sh1106_write_command(display, SH1106_DISPLAY_OFF);
    sh1106_write_command(display, SH1106_SET_DISPLAY_CLOCK);
    sh1106_write_command(display, 0x80);
    sh1106_write_command(display, SH1106_SET_MULTIPLEX);
    sh1106_write_command(display, 0x3F);
    sh1106_write_command(display, SH1106_SET_DISPLAY_OFFSET);
    sh1106_write_command(display, 0x00);
    sh1106_write_command(display, SH1106_SET_START_LINE | 0x0);
    sh1106_write_command(display, SH1106_CHARGE_PUMP);
    sh1106_write_command(display, 0x14);
    sh1106_write_command(display, SH1106_MEMORY_MODE);
    sh1106_write_command(display, 0x00);
    sh1106_write_command(display, SH1106_SEG_REMAP | 0x1);
    sh1106_write_command(display, SH1106_COM_SCAN_DEC);
    sh1106_write_command(display, SH1106_SET_COMPINS);
    sh1106_write_command(display, 0x12);
    sh1106_write_command(display, SH1106_SET_CONTRAST);
    sh1106_write_command(display, 0xCF);
    sh1106_write_command(display, SH1106_SET_PRECHARGE);
    sh1106_write_command(display, 0xF1);
    sh1106_write_command(display, SH1106_SET_VCOMH_DETECT);
    sh1106_write_command(display, 0x40);
    sh1106_write_command(display, SH1106_DISPLAY_ALL_ON);
    sh1106_write_command(display, SH1106_INVERT_DISPLAY); // Invert for proper colors (black bg, white text)
    sh1106_write_command(display, SH1106_DISPLAY_ON);
    
    display->initialized = true;
    ESP_LOGI(TAG, "SH1106 initialized on I2C port %d", i2c_port);
    
    return ESP_OK;
}

esp_err_t sh1106_deinit(sh1106_t *display) {
    if (!display->initialized) return ESP_ERR_INVALID_STATE;
    
    sh1106_display_off(display);
    i2c_master_bus_rm_device(display->i2c_dev);
    i2c_del_master_bus(display->i2c_bus);
    display->initialized = false;
    
    ESP_LOGI(TAG, "SH1106 deinitialized");
    return ESP_OK;
}

esp_err_t sh1106_display_on(sh1106_t *display) {
    return sh1106_write_command(display, SH1106_DISPLAY_ON);
}

esp_err_t sh1106_display_off(sh1106_t *display) {
    return sh1106_write_command(display, SH1106_DISPLAY_OFF);
}

esp_err_t sh1106_clear_display(sh1106_t *display) {
    memset(sh1106_buffer, 0, sizeof(sh1106_buffer));
    return ESP_OK;
}

esp_err_t sh1106_invert_display(sh1106_t *display, bool invert) {
    return sh1106_write_command(display, invert ? SH1106_INVERT_DISPLAY : SH1106_NORMAL_DISPLAY);
}

esp_err_t sh1106_set_contrast(sh1106_t *display, uint8_t contrast) {
    sh1106_write_command(display, SH1106_SET_CONTRAST);
    return sh1106_write_command(display, contrast);
}

void sh1106_drawPixel_internal(int16_t x, int16_t y, uint16_t color) {
    if ((x < 0) || (x >= SH1106_WIDTH) || (y < 0) || (y >= SH1106_HEIGHT))
        return;
    
    switch (color) {
        case SH1106_WHITE:
            sh1106_buffer[x + (y / 8) * SH1106_WIDTH] |= (1 << (y & 7));
            break;
        case SH1106_BLACK:
            sh1106_buffer[x + (y / 8) * SH1106_WIDTH] &= ~(1 << (y & 7));
            break;
        case SH1106_INVERSE:
            sh1106_buffer[x + (y / 8) * SH1106_WIDTH] ^= (1 << (y & 7));
            break;
    }
}

esp_err_t sh1106_draw_pixel(sh1106_t *display, int16_t x, int16_t y, uint16_t color) {
    sh1106_drawPixel_internal(x, y, color);
    return ESP_OK;
}

esp_err_t sh1106_display(sh1106_t *display) {
    if (!display->initialized) return ESP_ERR_INVALID_STATE;
    
    esp_err_t ret;
    uint8_t buffer[129]; // Command + 128 bytes data
    
    for (int8_t page = 0; page < (SH1106_HEIGHT / 8); page++) {
        // Set page address (0xB0 + page)
        buffer[0] = 0x00; // Command byte
        buffer[1] = 0xB0 | page;
        ret = i2c_master_transmit(display->i2c_dev, buffer, 2, portMAX_DELAY);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to set page %d", page);
            return ret;
        }
        
        // Set column address low nibble (0x00-0x0F)
        buffer[0] = 0x00;
        buffer[1] = 0x00; // Low column = 0
        ret = i2c_master_transmit(display->i2c_dev, buffer, 2, portMAX_DELAY);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to set column low");
            return ret;
        }
        
        // Set column address high nibble (0x10-0x1F)
        buffer[0] = 0x00;
        buffer[1] = 0x10; // High column = 0x10
        ret = i2c_master_transmit(display->i2c_dev, buffer, 2, portMAX_DELAY);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to set column high");
            return ret;
        }
        
        // Write data bytes - send in chunks
        buffer[0] = 0x40; // Data mode
        int chunk_size = 16; // Send 16 bytes at a time
        for (int j = 0; j < SH1106_WIDTH; j += chunk_size) {
            int bytes_to_send = (SH1106_WIDTH - j < chunk_size) ? (SH1106_WIDTH - j) : chunk_size;
            for (int k = 0; k < bytes_to_send; k++) {
                buffer[k + 1] = sh1106_buffer[page * SH1106_WIDTH + j + k];
            }
            ret = i2c_master_transmit(display->i2c_dev, buffer, bytes_to_send + 1, portMAX_DELAY);
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "Failed to write data chunk at page %d, col %d", page, j);
                return ret;
            }
        }
    }
    
    ESP_LOGI(TAG, "Display update complete");
    return ESP_OK;
}

// Bresenham's line algorithm
esp_err_t sh1106_draw_line(sh1106_t *display, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    int16_t dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int16_t dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int16_t err = dx + dy, e2;
    
    while (true) {
        sh1106_drawPixel_internal(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
    return ESP_OK;
}

esp_err_t sh1106_draw_rect(sh1106_t *display, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color, bool filled) {
    if (filled) {
        for (int16_t i = y; i < y + h; i++) {
            for (int16_t j = x; j < x + w; j++) {
                sh1106_drawPixel_internal(j, i, color);
            }
        }
    } else {
        sh1106_draw_line(display, x, y, x + w - 1, y, color);
        sh1106_draw_line(display, x + w - 1, y, x + w - 1, y + h - 1, color);
        sh1106_draw_line(display, x + w - 1, y + h - 1, x, y + h - 1, color);
        sh1106_draw_line(display, x, y + h - 1, x, y, color);
    }
    return ESP_OK;
}

// Midpoint circle algorithm
esp_err_t sh1106_draw_circle(sh1106_t *display, int16_t x0, int16_t y0, int16_t r, uint16_t color, bool filled) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;
    
    if (filled) {
        sh1106_draw_line(display, x0, y0 - r, x0, y0 + r, color);
        sh1106_draw_line(display, x0 - r, y0, x0 + r, y0, color);
    } else {
        sh1106_drawPixel_internal(x0, y0 + r, color);
        sh1106_drawPixel_internal(x0, y0 - r, color);
        sh1106_drawPixel_internal(x0 + r, y0, color);
        sh1106_drawPixel_internal(x0 - r, y0, color);
    }
    
    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;
        
        if (filled) {
            sh1106_draw_line(display, x0 - x, y0 + y, x0 + x, y0 + y, color);
            sh1106_draw_line(display, x0 - x, y0 - y, x0 + x, y0 - y, color);
            sh1106_draw_line(display, x0 - y, y0 + x, x0 + y, y0 + x, color);
            sh1106_draw_line(display, x0 - y, y0 - x, x0 + y, y0 - x, color);
        } else {
            sh1106_drawPixel_internal(x0 + x, y0 + y, color);
            sh1106_drawPixel_internal(x0 - x, y0 + y, color);
            sh1106_drawPixel_internal(x0 + x, y0 - y, color);
            sh1106_drawPixel_internal(x0 - x, y0 - y, color);
            sh1106_drawPixel_internal(x0 + y, y0 + x, color);
            sh1106_drawPixel_internal(x0 - y, y0 + x, color);
            sh1106_drawPixel_internal(x0 + y, y0 - x, color);
            sh1106_drawPixel_internal(x0 - y, y0 - x, color);
        }
    }
    return ESP_OK;
}

esp_err_t sh1106_draw_char(sh1106_t *display, int16_t x, int16_t y, char c, uint16_t color, int size) {
    if (c < 32 || c > 127) return ESP_OK;
    
    const uint8_t *char_data = font5x7[c - 32];
    
    for (int8_t i = 0; i < 5; i++) {
        for (int8_t j = 0; j < 7; j++) {
            if ((char_data[i] >> j) & 0x01) {
                if (size == 1) {
                    sh1106_drawPixel_internal(x + i, y + j, color);
                } else {
                    sh1106_draw_rect(display, x + i * size, y + j * size, size, size, color, true);
                }
            }
        }
    }
    return ESP_OK;
}

esp_err_t sh1106_draw_string(sh1106_t *display, int16_t x, int16_t y, const char *str, uint16_t color, int size) {
    int16_t cursor = x;
    while (*str) {
        sh1106_draw_char(display, cursor, y, *str, color, size);
        cursor += 6 * size;
        str++;
    }
    return ESP_OK;
}

esp_err_t sh1106_draw_bitmap(sh1106_t *display, int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color) {
    int16_t byte_width = (w + 7) / 8;
    
    for (int16_t j = 0; j < h; j++) {
        for (int16_t i = 0; i < w; i++) {
            uint8_t byte = bitmap[(j * byte_width) + (i / 8)];
            if (byte & (128 >> (i & 7))) {
                sh1106_drawPixel_internal(x + i, y + j, color);
            }
        }
    }
    return ESP_OK;
}
