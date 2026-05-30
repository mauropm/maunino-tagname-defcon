/*
 * DEF CON Badge Firmware for Electronic Cats Minino
 * ESP-IDF 5.x - SH1106 128x64 OLED Display
 *
 * Animation sequence:
 *   DEF CON Logo (5s) -> Liquid Splash (1s) -> Wiper Reveal (5s) ->
 *   Tagname "MAURO" (5s) -> Electronic Cats Logo (5s) -> Loop
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "sh1106.h"
#include "badge_bitmaps.h"

static const char *TAG = "DEFCON";

/* ============================================================================
 * Configuration
 * ============================================================================ */
#define DISP_WIDTH              SH1106_WIDTH
#define DISP_HEIGHT             SH1106_HEIGHT

#define TIME_DEFCON_MS          5000
#define TIME_SPLASH_MS          1000
#define TIME_WIPER_MS           5000
#define TIME_TAGNAME_MS         5000
#define TIME_ECAT_MS            5000

#define FRAME_MS                50
#define FRAMES_PER_SEC          (1000 / FRAME_MS)

/* ============================================================================
 * State Machine
 * ============================================================================ */
typedef enum {
    STATE_DEFCON,
    STATE_SPLASH,
    STATE_WIPER,
    STATE_TAGNAME,
    STATE_ECAT,
    STATE_COUNT
} badge_state_t;

static const char *state_names[] = {
    "DEFCON_LOGO",
    "LIQUID_SPLASH",
    "WIPER_REVEAL",
    "TAGNAME",
    "ELECTRONIC_CATS"
};

/* ============================================================================
 * Splash state - procedural liquid coverage
 * ============================================================================ */
#define MAX_SPLASH_BLOBS        64

typedef struct {
    int8_t x, y;
    uint8_t radius;
    uint8_t max_radius;
} splash_blob_t;

static splash_blob_t splash_blobs[MAX_SPLASH_BLOBS];
static int splash_blob_count = 0;
static int splash_frame = 0;

/* Simple LCG random for deterministic animation (no heap) */
static uint32_t rng_state = 0xDEADBEEF;

static uint32_t rng_next(void) {
    rng_state = rng_state * 1103515245 + 12345;
    return rng_state;
}

static int rng_range(int min, int max) {
    return min + (rng_next() % (max - min + 1));
}

static void splash_init(void) {
    splash_blob_count = 0;
    splash_frame = 0;
    rng_state = 0xDEADBEEF;
}

static void splash_add_blob(void) {
    if (splash_blob_count >= MAX_SPLASH_BLOBS) return;
    splash_blobs[splash_blob_count].x = rng_range(-10, DISP_WIDTH + 10);
    splash_blobs[splash_blob_count].y = rng_range(-10, DISP_HEIGHT + 10);
    splash_blobs[splash_blob_count].radius = 1;
    splash_blobs[splash_blob_count].max_radius = rng_range(8, 25);
    splash_blob_count++;
}

static void splash_update(void) {
    splash_frame++;
    if (splash_frame % 2 == 0 && splash_blob_count < MAX_SPLASH_BLOBS) {
        splash_add_blob();
    }
    for (int i = 0; i < splash_blob_count; i++) {
        if (splash_blobs[i].radius < splash_blobs[i].max_radius) {
            splash_blobs[i].radius += 1;
        }
    }
}

static void splash_draw(sh1106_t *display) {
    for (int i = 0; i < splash_blob_count; i++) {
        sh1106_draw_circle(display,
            splash_blobs[i].x, splash_blobs[i].y,
            splash_blobs[i].radius, SH1106_WHITE, true);
    }
}

/* ============================================================================
 * Wiper state - windshield wiper reveal
 * ============================================================================ */
#define WIPE_COLS               DISP_WIDTH

static uint8_t wipe_clean[WIPE_COLS];
static int wiper_frame = 0;
static int wiper_total_frames;
static int wiper_pass = 0;

static void wiper_init(void) {
    memset(wipe_clean, 0, sizeof(wipe_clean));
    wiper_frame = 0;
    wiper_total_frames = TIME_WIPER_MS / FRAME_MS;
    wiper_pass = 0;
    rng_state = 0xCAFE1234;
    splash_blob_count = 0;
    for (int i = 0; i < MAX_SPLASH_BLOBS; i++) {
        splash_blobs[i].x = rng_range(0, DISP_WIDTH - 1);
        splash_blobs[i].y = rng_range(0, DISP_HEIGHT - 1);
        splash_blobs[i].radius = rng_range(10, 30);
        splash_blobs[i].max_radius = splash_blobs[i].radius;
        splash_blob_count++;
    }
}

static void wiper_update(void) {
    wiper_frame++;

    float progress = (float)wiper_frame / wiper_total_frames;

    if (progress < 0.33f) {
        wiper_pass = 1;
        float p = progress / 0.33f;
        int sweep_x = (int)(p * DISP_WIDTH);
        int width = 12;
        for (int x = sweep_x - width; x < sweep_x + width; x++) {
            if (x >= 0 && x < WIPE_COLS) {
                wipe_clean[x] = DISP_HEIGHT;
            }
        }
    } else if (progress < 0.66f) {
        wiper_pass = 2;
        float p = (progress - 0.33f) / 0.33f;
        int sweep_x = DISP_WIDTH - 1 - (int)(p * DISP_WIDTH);
        int width = 14;
        for (int x = sweep_x - width; x < sweep_x + width; x++) {
            if (x >= 0 && x < WIPE_COLS) {
                wipe_clean[x] = DISP_HEIGHT;
            }
        }
    } else {
        wiper_pass = 3;
        float p = (progress - 0.66f) / 0.34f;
        int sweep_x = (int)(p * DISP_WIDTH);
        int width = 16;
        for (int x = sweep_x - width; x < sweep_x + width; x++) {
            if (x >= 0 && x < WIPE_COLS) {
                wipe_clean[x] = DISP_HEIGHT;
            }
        }
    }
}

static void wiper_draw(sh1106_t *display) {
    sh1106_draw_string(display, 24, 24, "MAURO", SH1106_WHITE, 3);

    for (int i = 0; i < splash_blob_count; i++) {
        int bx = splash_blobs[i].x;
        int by = splash_blobs[i].y;
        int br = splash_blobs[i].radius;

        for (int dy = -br; dy <= br; dy++) {
            for (int dx = -br; dx <= br; dx++) {
                if (dx * dx + dy * dy > br * br) continue;
                int px = bx + dx;
                int py = by + dy;
                if (px < 0 || px >= DISP_WIDTH || py < 0 || py >= DISP_HEIGHT) continue;
                if (wipe_clean[px] > 0 && py < wipe_clean[px]) continue;
                sh1106_draw_pixel(display, px, py, SH1106_WHITE);
            }
        }
    }

    int wiper_x = 0;
    float progress = (float)wiper_frame / wiper_total_frames;
    if (progress < 0.33f) {
        wiper_x = (int)((progress / 0.33f) * DISP_WIDTH);
    } else if (progress < 0.66f) {
        wiper_x = DISP_WIDTH - 1 - (int)(((progress - 0.33f) / 0.33f) * DISP_WIDTH);
    } else {
        wiper_x = (int)(((progress - 0.66f) / 0.34f) * DISP_WIDTH);
    }
    sh1106_draw_line(display, wiper_x, 0, wiper_x, DISP_HEIGHT - 1, SH1106_WHITE);
    sh1106_draw_line(display, wiper_x + 1, 0, wiper_x + 1, DISP_HEIGHT - 1, SH1106_WHITE);
    sh1106_draw_line(display, wiper_x - 1, 0, wiper_x - 1, DISP_HEIGHT - 1, SH1106_WHITE);
}

/* ============================================================================
 * DEF CON Logo Screen
 * ============================================================================ */
static void draw_defcon_logo(sh1106_t *display, int alpha) {
    int logo_x = (DISP_WIDTH - DEFCON_LOGO_W) / 2;
    int logo_y = 0;
    int bw = (DEFCON_LOGO_W + 7) / 8;

    for (int16_t j = 0; j < DEFCON_LOGO_H; j++) {
        for (int16_t i = 0; i < DEFCON_LOGO_W; i++) {
            uint8_t byte = defcon_logo_bmp[(j * bw) + (i / 8)];
            if (!(byte & (128 >> (i & 7)))) {
                if (alpha > 128 || (alpha > 0 && (j + i) % 4 < 2)) {
                    sh1106_draw_pixel(display, logo_x + i, logo_y + j, SH1106_WHITE);
                }
            }
        }
    }
}

static void state_defcon(sh1106_t *display) {
    int total_frames = TIME_DEFCON_MS / FRAME_MS;
    for (int frame = 0; frame < total_frames; frame++) {
        sh1106_clear_display(display);

        int alpha = (frame < 10) ? (frame * 25) : 255;
        draw_defcon_logo(display, alpha);

        sh1106_display(display);
        vTaskDelay(pdMS_TO_TICKS(FRAME_MS));
    }
}

/* ============================================================================
 * Liquid Splash Screen
 * ============================================================================ */
static void state_splash(sh1106_t *display) {
    splash_init();
    int total_frames = TIME_SPLASH_MS / FRAME_MS;

    sh1106_clear_display(display);
    sh1106_display(display);

    for (int frame = 0; frame < total_frames; frame++) {
        splash_update();
        splash_draw(display);
        sh1106_display(display);
        vTaskDelay(pdMS_TO_TICKS(FRAME_MS));
    }

    sh1106_clear_display(display);
    for (int i = 0; i < splash_blob_count; i++) {
        sh1106_draw_circle(display,
            splash_blobs[i].x, splash_blobs[i].y,
            splash_blobs[i].radius, SH1106_WHITE, true);
    }
}

/* ============================================================================
 * Wiper Reveal Screen
 * ============================================================================ */
static void state_wiper(sh1106_t *display) {
    wiper_init();
    int total_frames = TIME_WIPER_MS / FRAME_MS;

    for (int frame = 0; frame < total_frames; frame++) {
        sh1106_clear_display(display);
        wiper_update();
        wiper_draw(display);
        sh1106_display(display);
        vTaskDelay(pdMS_TO_TICKS(FRAME_MS));
    }
}

/* ============================================================================
 * Tagname Screen - "MAURO"
 * ============================================================================ */
static void state_tagname(sh1106_t *display) {
    int total_frames = TIME_TAGNAME_MS / FRAME_MS;

    for (int frame = 0; frame < total_frames; frame++) {
        sh1106_clear_display(display);

        sh1106_draw_string(display, 24, 24, "MAURO", SH1106_WHITE, 3);

        sh1106_draw_line(display, 0, 4, DISP_WIDTH - 1, 4, SH1106_WHITE);
        sh1106_draw_line(display, 0, DISP_HEIGHT - 4, DISP_WIDTH - 1, DISP_HEIGHT - 4, SH1106_WHITE);

        if (frame % 8 < 2) {
            for (int y = 0; y < DISP_HEIGHT; y += 3) {
                for (int x = 0; x < DISP_WIDTH; x++) {
                    sh1106_draw_pixel(display, x, y, SH1106_WHITE);
                }
            }
        }

        sh1106_display(display);
        vTaskDelay(pdMS_TO_TICKS(FRAME_MS));
    }
}

/* ============================================================================
 * Electronic Cats Logo Screen
 * ============================================================================ */
static void draw_ecat_logo(sh1106_t *display, int alpha) {
    int logo_x = (DISP_WIDTH - ECAT_LOGO_W) / 2;
    int logo_y = 0;
    int bw = (ECAT_LOGO_W + 7) / 8;

    for (int16_t j = 0; j < ECAT_LOGO_H; j++) {
        for (int16_t i = 0; i < ECAT_LOGO_W; i++) {
            uint8_t byte = ecat_logo_bmp[(j * bw) + (i / 8)];
            if (byte & (128 >> (i & 7))) {
                if (alpha > 128 || (alpha > 0 && (j + i) % 4 < 2)) {
                    sh1106_draw_pixel(display, logo_x + i, logo_y + j, SH1106_WHITE);
                }
            }
        }
    }
}

static void state_ecat(sh1106_t *display) {
    int total_frames = TIME_ECAT_MS / FRAME_MS;

    for (int frame = 0; frame < total_frames; frame++) {
        sh1106_clear_display(display);

        int alpha = (frame < 10) ? (frame * 25) : 255;
        draw_ecat_logo(display, alpha);

        sh1106_display(display);
        vTaskDelay(pdMS_TO_TICKS(FRAME_MS));
    }
}

/* ============================================================================
 * Main Display Task
 * ============================================================================ */
static void display_task(void *pvParameters) {
    sh1106_t *display = (sh1106_t *)pvParameters;
    badge_state_t current_state = STATE_DEFCON;

    ESP_LOGI(TAG, "DEF CON Badge display task started");

    while (1) {
        ESP_LOGI(TAG, "State: %s", state_names[current_state]);

        switch (current_state) {
            case STATE_DEFCON:
                state_defcon(display);
                break;
            case STATE_SPLASH:
                state_splash(display);
                break;
            case STATE_WIPER:
                state_wiper(display);
                break;
            case STATE_TAGNAME:
                state_tagname(display);
                break;
            case STATE_ECAT:
                state_ecat(display);
                break;
            default:
                break;
        }

        current_state = (badge_state_t)((current_state + 1) % STATE_COUNT);
    }
}

/* ============================================================================
 * App Main
 * ============================================================================ */
void app_main(void) {
    ESP_LOGI(TAG, "DEF CON Badge Firmware");
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

    xTaskCreate(display_task, "defcon_task", 4096, &display, 5, NULL);

    vTaskDelete(NULL);
}
