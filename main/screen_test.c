/*
 * Screen Test Application for SH1106 OLED Display
 * Target: ESP32-C6 (Minino)
 * 
 * This is a minimal standalone firmware for testing screen functionality
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "sh1106.h"

static const char *TAG = "SCREEN_TEST";

// Test logo bitmap (16x16)
static const unsigned char logo16_glcd_bmp[] = {
    0x00, 0xC0, 0x00, 0x03,
    0x00, 0xC0, 0x00, 0x03,
    0x00, 0xC0, 0x00, 0x03,
    0xC0, 0xC0, 0x03, 0x03,
    0xC0, 0xC0, 0x03, 0x03,
    0xC0, 0xFF, 0x03, 0xFF,
    0xC0, 0xFF, 0x03, 0xFF,
    0x00, 0xFF, 0x00, 0xFF,
    0x00, 0x3C, 0x00, 0x0F,
    0x80, 0xB0, 0x01, 0x0D,
    0xC0, 0xD8, 0x03, 0x0B,
    0xC0, 0xC0, 0x03, 0x03,
    0xC0, 0xC0, 0x03, 0x03,
    0xC0, 0xC0, 0x03, 0x03,
    0xC0, 0xC0, 0x03, 0x03,
    0x00, 0xC0, 0x00, 0x03,
};

static void test_splash(sh1106_t *display) {
    ESP_LOGI(TAG, "Test: Splash screen");
    sh1106_clear_display(display);
    sh1106_draw_string(display, 10, 10, "SH1106 Test", SH1106_WHITE, 2);
    sh1106_draw_string(display, 10, 30, "ESP32-C6", SH1106_WHITE, 2);
    sh1106_draw_string(display, 10, 50, "Minino", SH1106_WHITE, 1);
    ESP_LOGI(TAG, "Calling sh1106_display()...");
    esp_err_t ret = sh1106_display(display);
    ESP_LOGI(TAG, "Display result: %s", esp_err_to_name(ret));
    vTaskDelay(pdMS_TO_TICKS(2000));
}

static void test_text(sh1106_t *display) {
    ESP_LOGI(TAG, "Test: Text rendering");
    sh1106_clear_display(display);
    
    // Small text
    sh1106_draw_string(display, 0, 0, "Small text test", SH1106_WHITE, 1);
    
    // Medium text
    sh1106_draw_string(display, 0, 16, "Medium text", SH1106_WHITE, 2);
    
    // Large text
    sh1106_draw_string(display, 0, 40, "BIG", SH1106_WHITE, 3);
    
    sh1106_display(display);
    vTaskDelay(pdMS_TO_TICKS(2000));
}

static void test_lines(sh1106_t *display) {
    ESP_LOGI(TAG, "Test: Line drawing");
    sh1106_clear_display(display);
    
    // Draw grid
    for (int i = 0; i < SH1106_WIDTH; i += 8) {
        sh1106_draw_line(display, i, 0, i, SH1106_HEIGHT - 1, SH1106_WHITE);
    }
    for (int i = 0; i < SH1106_HEIGHT; i += 8) {
        sh1106_draw_line(display, 0, i, SH1106_WIDTH - 1, i, SH1106_WHITE);
    }
    
    // Draw diagonal
    sh1106_draw_line(display, 0, 0, SH1106_WIDTH - 1, SH1106_HEIGHT - 1, SH1106_WHITE);
    sh1106_draw_line(display, SH1106_WIDTH - 1, 0, 0, SH1106_HEIGHT - 1, SH1106_WHITE);
    
    sh1106_display(display);
    vTaskDelay(pdMS_TO_TICKS(2000));
}

static void test_rectangles(sh1106_t *display) {
    ESP_LOGI(TAG, "Test: Rectangle drawing");
    sh1106_clear_display(display);
    
    // Outline rectangles
    for (int i = 0; i < 30; i += 5) {
        sh1106_draw_rect(display, i, i, SH1106_WIDTH - 2*i, SH1106_HEIGHT - 2*i, SH1106_WHITE, false);
    }
    sh1106_display(display);
    vTaskDelay(pdMS_TO_TICKS(1500));
    
    // Filled rectangles
    sh1106_clear_display(display);
    for (int i = 0; i < 30; i += 6) {
        sh1106_draw_rect(display, i, i, SH1106_WIDTH - 2*i, SH1106_HEIGHT - 2*i, SH1106_WHITE, true);
    }
    sh1106_display(display);
    vTaskDelay(pdMS_TO_TICKS(1500));
}

static void test_circles(sh1106_t *display) {
    ESP_LOGI(TAG, "Test: Circle drawing");
    sh1106_clear_display(display);
    
    // Outline circles
    for (int i = 5; i < 32; i += 5) {
        sh1106_draw_circle(display, SH1106_WIDTH/2, SH1106_HEIGHT/2, i, SH1106_WHITE, false);
    }
    sh1106_display(display);
    vTaskDelay(pdMS_TO_TICKS(1500));
    
    // Filled circles
    sh1106_clear_display(display);
    sh1106_draw_circle(display, SH1106_WIDTH/2, SH1106_HEIGHT/2, 20, SH1106_WHITE, true);
    sh1106_display(display);
    vTaskDelay(pdMS_TO_TICKS(1500));
}

static void test_invert(sh1106_t *display) {
    ESP_LOGI(TAG, "Test: Invert display");
    sh1106_clear_display(display);
    sh1106_draw_string(display, 10, 20, "Invert Test", SH1106_WHITE, 2);
    sh1106_display(display);
    
    vTaskDelay(pdMS_TO_TICKS(1000));
    sh1106_invert_display(display, true);
    vTaskDelay(pdMS_TO_TICKS(1000));
    sh1106_invert_display(display, false);
    vTaskDelay(pdMS_TO_TICKS(500));
}

static void test_bitmap(sh1106_t *display) {
    ESP_LOGI(TAG, "Test: Bitmap rendering");
    sh1106_clear_display(display);
    
    // Draw logo at different positions
    for (int i = 0; i < 5; i++) {
        sh1106_draw_bitmap(display, 10 + i*10, 10, logo16_glcd_bmp, 16, 16, SH1106_WHITE);
        sh1106_display(display);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    
    vTaskDelay(pdMS_TO_TICKS(1500));
}

static void test_animation(sh1106_t *display) {
    ESP_LOGI(TAG, "Test: Animation");
    
    // Bouncing ball animation
    int x = 10, y = 10;
    int dx = 2, dy = 1;
    
    for (int frame = 0; frame < 100; frame++) {
        sh1106_clear_display(display);
        
        // Draw ball
        sh1106_draw_circle(display, x, y, 5, SH1106_WHITE, true);
        
        // Draw border
        sh1106_draw_rect(display, 0, 0, SH1106_WIDTH, SH1106_HEIGHT, SH1106_WHITE, false);
        
        sh1106_display(display);
        
        // Update position
        x += dx;
        y += dy;
        
        // Bounce
        if (x <= 5 || x >= SH1106_WIDTH - 6) dx = -dx;
        if (y <= 5 || y >= SH1106_HEIGHT - 6) dy = -dy;
        
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

static void test_scrolling_text(sh1106_t *display) {
    ESP_LOGI(TAG, "Test: Scrolling text effect");
    
    const char *text = "SCROLLING TEXT EFFECT ";
    int text_len = strlen(text);
    
    for (int offset = 0; offset < (text_len * 6 * 2); offset++) {
        sh1106_clear_display(display);
        
        for (int i = 0; i < 2; i++) {
            int x = (text_len * 6 * 2) - offset + (i * text_len * 6);
            while (x < SH1106_WIDTH) {
                if (x > -50) {
                    sh1106_draw_string(display, x, 24, text, SH1106_WHITE, 2);
                }
                x += text_len * 6 * 2;
            }
        }
        
        sh1106_display(display);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

static void test_all_patterns(sh1106_t *display) {
    ESP_LOGI(TAG, "Test: All patterns sequence");
    
    sh1106_clear_display(display);
    
    // Fill patterns - draw all pixels in a checkerboard pattern
    for (int pattern = 0; pattern < 4; pattern++) {
        sh1106_clear_display(display);
        for (int y = 0; y < SH1106_HEIGHT; y += 2) {
            for (int x = pattern; x < SH1106_WIDTH; x += 4) {
                sh1106_draw_pixel(display, x, y, SH1106_WHITE);
            }
        }
        sh1106_display(display);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    
    sh1106_clear_display(display);
}

void app_main(void) {
    ESP_LOGI(TAG, "Starting SH1106 Screen Test");
    ESP_LOGI(TAG, "Target: ESP32-C6 (Minino)");
    ESP_LOGI(TAG, "Display: SH1106 128x64 OLED");
    
    sh1106_t display;
    
    // Initialize display
    // I2C configuration for ESP32-C6 Minino
    // SDA: GPIO6, SCL: GPIO7 (adjust based on your hardware)
    int sda_gpio = 6;
    int scl_gpio = 7;
    int reset_gpio = -1; // No reset pin connected
    
    ESP_LOGI(TAG, "Initializing SH1106 on I2C (SDA=%d, SCL=%d)", sda_gpio, scl_gpio);
    
    esp_err_t ret = sh1106_init(&display, sda_gpio, scl_gpio, reset_gpio, I2C_NUM_0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SH1106: %s", esp_err_to_name(ret));
        return;
    }
    
    ESP_LOGI(TAG, "SH1106 initialized successfully");
    
    // Clear display
    sh1106_clear_display(&display);
    sh1106_display(&display);
    
    // Run test sequence
    while (true) {
        ESP_LOGI(TAG, "=== Starting test sequence ===");
        
        test_splash(&display);
        test_text(&display);
        test_lines(&display);
        test_rectangles(&display);
        test_circles(&display);
        test_invert(&display);
        test_bitmap(&display);
        test_animation(&display);
        test_scrolling_text(&display);
        test_all_patterns(&display);
        
        ESP_LOGI(TAG, "=== Test sequence complete ===");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
