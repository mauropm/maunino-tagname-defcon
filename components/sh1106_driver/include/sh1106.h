/*
 * SH1106 OLED Display Driver for ESP-IDF 5
 * I2C Interface for ESP32-C6
 * 
 * Based on Adafruit SH1106 library
 * Ported to ESP-IDF 5 for Minino (ESP32-C6)
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "driver/i2c_master.h"

// SH1106 I2C Address
#define SH1106_I2C_ADDRESS 0x3C

// Display dimensions
#define SH1106_128_64
#ifdef SH1106_128_64
    #define SH1106_WIDTH  128
    #define SH1106_HEIGHT 64
#endif

#ifdef SH1106_128_32
    #define SH1106_WIDTH  128
    #define SH1106_HEIGHT 32
#endif

// Commands
#define SH1106_SET_CONTRAST        0x81
#define SH1106_DISPLAY_ALL_ON      0xA4
#define SH1106_DISPLAY_ALL_ON_RESUME 0xA5
#define SH1106_NORMAL_DISPLAY      0xA6
#define SH1106_INVERT_DISPLAY      0xA7
#define SH1106_DISPLAY_OFF         0xAE
#define SH1106_DISPLAY_ON          0xAF

#define SH1106_SET_DISPLAY_OFFSET  0xD3
#define SH1106_SET_COMPINS         0xDA

#define SH1106_SET_VCOMH_DETECT    0xDB

#define SH1106_SET_DISPLAY_CLOCK   0xD5
#define SH1106_SET_PRECHARGE       0xD9

#define SH1106_SET_MULTIPLEX       0xA8

#define SH1106_SET_LOW_COLUMN      0x02
#define SH1106_SET_HIGH_COLUMN     0x10

#define SH1106_SET_START_LINE      0x40

#define SH1106_MEMORY_MODE         0x20
#define SH1106_COLUMN_ADDR         0x21
#define SH1106_PAGE_ADDR           0x22

#define SH1106_COM_SCAN_INC        0xC0
#define SH1106_COM_SCAN_DEC        0xC8

#define SH1106_SEG_REMAP           0xA0

#define SH1106_CHARGE_PUMP         0x8D

#define SH1106_EXTERNAL_VCC        0x01
#define SH1106_SWITCH_CAP_VCC      0x02

// Scrolling commands
#define SH1106_ACTIVATE_SCROLL     0x2F
#define SH1106_DEACTIVATE_SCROLL   0x2E
#define SH1106_SET_VScroll_AREA    0xA3
#define SH1106_RIGHT_H_SCROLL      0x26
#define SH1106_LEFT_H_SCROLL       0x27
#define SH1106_VRIGHT_H_SCROLL     0x29
#define SH1106_VLEFT_H_SCROLL      0x2A

// Color definitions
#define SH1106_BLACK 0
#define SH1106_WHITE 1
#define SH1106_INVERSE 2

// Driver configuration structure
typedef struct {
    i2c_master_bus_handle_t i2c_bus;
    i2c_master_dev_handle_t i2c_dev;
    int sda_io_num;
    int scl_io_num;
    int reset_io_num;
    uint8_t i2c_address;
    uint8_t vcc_state;
    bool initialized;
} sh1106_t;

// Forward declaration of opaque types (defined in i2c_master.h)
typedef struct i2c_master_bus_t i2c_master_bus_t;
typedef struct i2c_master_dev_t i2c_master_dev_t;

// Public functions
esp_err_t sh1106_init(sh1106_t *display, int sda_io, int scl_io, int reset_io, i2c_port_t i2c_port);
esp_err_t sh1106_deinit(sh1106_t *display);
esp_err_t sh1106_display_on(sh1106_t *display);
esp_err_t sh1106_display_off(sh1106_t *display);
esp_err_t sh1106_clear_display(sh1106_t *display);
esp_err_t sh1106_invert_display(sh1106_t *display, bool invert);
esp_err_t sh1106_set_contrast(sh1106_t *display, uint8_t contrast);
esp_err_t sh1106_draw_pixel(sh1106_t *display, int16_t x, int16_t y, uint16_t color);
esp_err_t sh1106_draw_line(sh1106_t *display, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
esp_err_t sh1106_draw_rect(sh1106_t *display, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color, bool filled);
esp_err_t sh1106_draw_circle(sh1106_t *display, int16_t x0, int16_t y0, int16_t r, uint16_t color, bool filled);
esp_err_t sh1106_display(sh1106_t *display);
esp_err_t sh1106_draw_char(sh1106_t *display, int16_t x, int16_t y, char c, uint16_t color, int size);
esp_err_t sh1106_draw_string(sh1106_t *display, int16_t x, int16_t y, const char *str, uint16_t color, int size);
esp_err_t sh1106_draw_bitmap(sh1106_t *display, int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color);

// Utility functions
int16_t sh1106_get_width(void);
int16_t sh1106_get_height(void);
