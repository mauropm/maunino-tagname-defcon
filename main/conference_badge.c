/*
 * Conference Badge Firmware for Electronic Cats Minino
 * ESP-IDF 5.x - SH1106 128x64 OLED Display
 * 
 * A lightweight looping demo showcasing:
 * - Text rendering (PWNTERREY 2026, @mauropm)
 * - Graphics primitives (grid, circles, rectangles)
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "sh1106.h"

static const char *TAG = "BADGE";

// ============================================================================
// Configuration
// ============================================================================
#define MODE_DURATION_MS        4000

// Display dimensions
#define DISP_WIDTH              SH1106_WIDTH
#define DISP_HEIGHT             SH1106_HEIGHT
#define DISP_CENTER_X           (DISP_WIDTH / 2)
#define DISP_CENTER_Y           (DISP_HEIGHT / 2)

// ============================================================================
// Demo Mode State Machine
// ============================================================================
typedef enum {
    MODE_PRIMITIVES,
    MODE_TEXT,
    MODE_GRAPHICS,
    MODE_TEXT_2,
    MODE_COUNT
} demo_mode_t;

static const char *mode_names[] = {
    "PRIMITIVES",
    "TEXT",
    "GRAPHICS",
    "TEXT_2"
};

// ============================================================================
// Mode: Text Rendering (PWNTERREY 2026)
// ============================================================================
static void mode_text(sh1106_t *display) {
    sh1106_clear_display(display);
    
    sh1106_draw_string(display, 8, 10, "PWNTERREY", SH1106_WHITE, 2);
    sh1106_draw_string(display, 32, 32, "2026", SH1106_WHITE, 2);
    
    sh1106_draw_line(display, 0, 5, DISP_WIDTH - 1, 5, SH1106_WHITE);
    sh1106_draw_line(display, 0, 58, DISP_WIDTH - 1, 58, SH1106_WHITE);
    
    sh1106_display(display);
    vTaskDelay(pdMS_TO_TICKS(MODE_DURATION_MS));
}

// ============================================================================
// Mode: Text Rendering 2 (MAUROPM)
// ============================================================================
static void mode_text_2(sh1106_t *display) {
    sh1106_clear_display(display);
    
    sh1106_draw_string(display, 16, 28, "MAUROPM", SH1106_WHITE, 2);
    
    sh1106_draw_line(display, 0, 5, DISP_WIDTH - 1, 5, SH1106_WHITE);
    sh1106_draw_line(display, 0, 58, DISP_WIDTH - 1, 58, SH1106_WHITE);
    
    sh1106_display(display);
    vTaskDelay(pdMS_TO_TICKS(MODE_DURATION_MS));
}

// ============================================================================
// Mode: Graphics Pattern
// ============================================================================
static void mode_graphics(sh1106_t *display) {
    sh1106_clear_display(display);
    
    for (int i = 0; i < DISP_WIDTH; i += 8) {
        sh1106_draw_line(display, i, 0, i, DISP_HEIGHT - 1, SH1106_WHITE);
    }
    
    for (int i = 0; i < DISP_HEIGHT; i += 8) {
        sh1106_draw_line(display, 0, i, DISP_WIDTH - 1, i, SH1106_WHITE);
    }
    
    sh1106_draw_line(display, 0, 0, DISP_WIDTH - 1, DISP_HEIGHT - 1, SH1106_WHITE);
    sh1106_draw_line(display, DISP_WIDTH - 1, 0, 0, DISP_HEIGHT - 1, SH1106_WHITE);
    
    sh1106_draw_rect(display, 10, 10, DISP_WIDTH - 20, DISP_HEIGHT - 20, SH1106_WHITE, false);
    sh1106_draw_rect(display, 20, 20, DISP_WIDTH - 40, DISP_HEIGHT - 40, SH1106_WHITE, false);
    
    sh1106_display(display);
    vTaskDelay(pdMS_TO_TICKS(MODE_DURATION_MS));
}

// ============================================================================
// Mode: Primitives Animation
// ============================================================================
static void mode_primitives(sh1106_t *display) {
    int frame_count = MODE_DURATION_MS / 100;
    
    for (int frame = 0; frame < frame_count; frame++) {
        sh1106_clear_display(display);
        
        int radius = 5 + (frame % 25);
        sh1106_draw_circle(display, DISP_CENTER_X, DISP_CENTER_Y, radius, SH1106_WHITE, false);
        sh1106_draw_circle(display, DISP_CENTER_X, DISP_CENTER_Y, radius - 4, SH1106_WHITE, true);
        
        int box_offset = (frame * 3) % 30;
        sh1106_draw_rect(display, box_offset, box_offset, 
                        DISP_WIDTH - 2 * box_offset, DISP_HEIGHT - 2 * box_offset, 
                        SH1106_WHITE, false);
        
        sh1106_display(display);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// ============================================================================
// Main Display Task
// ============================================================================
static void display_task(void *pvParameters) {
    sh1106_t *display = (sh1106_t *)pvParameters;
    demo_mode_t current_mode = MODE_PRIMITIVES;
    
    ESP_LOGI(TAG, "Display task started");
    
    while (1) {
        ESP_LOGI(TAG, "Mode: %s", mode_names[current_mode]);
        
        switch (current_mode) {
            case MODE_TEXT:
                mode_text(display);
                break;
            case MODE_GRAPHICS:
                mode_graphics(display);
                break;
            case MODE_TEXT_2:
                mode_text_2(display);
                break;
            case MODE_PRIMITIVES:
                mode_primitives(display);
                break;
            default:
                break;
        }
        
        current_mode = (demo_mode_t)((current_mode + 1) % MODE_COUNT);
    }
}

// ============================================================================
// App Main
// ============================================================================
void app_main(void) {
    ESP_LOGI(TAG, "Conference Badge Firmware");
    ESP_LOGI(TAG, "Electronic Cats Minino - ESP32-C6");
    ESP_LOGI(TAG, "SH1106 128x64 OLED Display");
    
    sh1106_t display;
    int sda_gpio = 6;
    int scl_gpio = 7;
    int reset_gpio = -1;
    
    ESP_LOGI(TAG, "Initializing display (SDA=%d, SCL=%d)", sda_gpio, scl_gpio);
    
    esp_err_t ret = sh1106_init(&display, sda_gpio, scl_gpio, reset_gpio, I2C_NUM_0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Display init failed: %s", esp_err_to_name(ret));
        return;
    }
    
    ESP_LOGI(TAG, "Display initialized");
    
    sh1106_clear_display(&display);
    sh1106_display(&display);
    
    xTaskCreate(display_task, "display_task", 4096, &display, 5, NULL);
    
    vTaskDelete(NULL);
}
