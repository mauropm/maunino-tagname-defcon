# SH1106 OLED Screen Test for ESP32-C6 (Minino)

## Overview

This is a minimal standalone firmware for testing SH1106 OLED display functionality on the Minino (ESP32-C6) development board. The firmware validates:

- I2C communication
- SH1106 initialization
- Framebuffer rendering
- Text rendering
- Graphics primitives
- Display refresh
- Animation stability

## Hardware Requirements

### Supported Hardware

- **MCU**: ESP32-C6 (Minino development board)
- **Display**: SH1106 OLED display
- **Interface**: I2C
- **Display Size**: 128x64 pixels (configurable)

### GPIO Mapping

Default I2C configuration for Minino:

| Function | GPIO Pin | Notes |
|----------|----------|-------|
| I2C SDA  | GPIO6    | Data line with internal pullup |
| I2C SCL  | GPIO7    | Clock line with internal pullup |
| Reset    | -        | Optional (not connected by default) |

**Note**: Adjust GPIO pins in `main/screen_test.c` if your hardware uses different pins.

## I2C Configuration

- **I2C Port**: I2C_NUM_0
- **I2C Address**: 0x3C (default for SH1106)
- **Clock Speed**: 400 kHz
- **Pullup**: Internal pullups enabled

## ESP-IDF 5 Setup

### Prerequisites

1. ESP-IDF 5.4 installed
2. Python 3.11 environment
3. ESP32-C6 target support

### Environment Activation

```bash
# Source the ESP-IDF environment
source ~/.zshrc.esp_idf

# Activate ESP-IDF 5
use_idf5
```

### Verify Installation

```bash
# Check ESP-IDF version
idf.py --version

# Check Python version
python --version

# Verify ESP32-C6 target support
idf.py --list-targets
```

Expected output:
```
ESP-IDF v5.4.1
Python 3.11.x
esp32c6
```

## Project Structure

```
screen_test/
├── CMakeLists.txt              # Project CMake configuration
├── sdkconfig.defaults          # Default SDK configuration
├── main/
│   ├── CMakeLists.txt          # Component CMake configuration
│   └── screen_test.c           # Main application code
├── components/
│   └── sh1106_driver/
│       ├── CMakeLists.txt      # Driver CMake configuration
│       ├── include/
│       │   └── sh1106.h        # Driver header file
│       └── sh1106.c            # Driver implementation
├── build.sh                    # Build script
├── flash.sh                    # Flash script
├── monitor.sh                  # Monitor script
├── clean.sh                    # Clean script
└── README_SCREEN_TEST.md       # This file
```

## Build Instructions

### Using Build Scripts (Recommended)

```bash
cd screen_test

# Build the project
./build.sh

# Flash to device
./flash.sh

# Monitor serial output
./monitor.sh

# Clean build artifacts
./clean.sh
```

### Manual Build

```bash
cd screen_test

# Set target to ESP32-C6
idf.py set-target esp32c6

# Build the project
idf.py build

# Flash to device
idf.py -p /dev/cu.usbserial-1 flash

# Monitor serial output
idf.py -p /dev/cu.usbserial-1 monitor
```

### Build for Production

```bash
# Build with size optimization
idf.py -D CMAKE_BUILD_TYPE=Release build
```

## Flash Instructions for macOS

### 1. Identify Serial Port

```bash
# List available serial ports
ls /dev/cu.usbserial*
```

### 2. Flash Using Script

```bash
./flash.sh
```

### 3. Flash Manually

```bash
idf.py -p /dev/cu.usbserial-1 flash
```

### 4. Reset Device

After flashing, press the RESET button on your Minino board or disconnect/reconnect power.

## Expected Serial Output

```
I (0) cpu_start: Starting scheduler on AP CPU.
I (0) SCREEN_TEST: Starting SH1106 Screen Test
I (0) SCREEN_TEST: Target: ESP32-C6 (Minino)
I (0) SCREEN_TEST: Display: SH1106 128x64 OLED
I (0) SCREEN_TEST: Initializing SH1106 on I2C (SDA=6, SCL=7)
I (0) SH1106: SH1106 initialized on I2C port 0
I (0) SCREEN_TEST: SH1106 initialized successfully
I (0) SCREEN_TEST: === Starting test sequence ===
I (0) SCREEN_TEST: Test: Splash screen
I (0) SCREEN_TEST: Test: Text rendering
I (0) SCREEN_TEST: Test: Line drawing
I (0) SCREEN_TEST: Test: Rectangle drawing
I (0) SCREEN_TEST: Test: Circle drawing
I (0) SCREEN_TEST: Test: Invert display
I (0) SCREEN_TEST: Test: Bitmap rendering
I (0) SCREEN_TEST: Test: Animation
I (0) SCREEN_TEST: Test: Scrolling text effect
I (0) SCREEN_TEST: Test: All patterns
I (0) SCREEN_TEST: === Test sequence complete ===
```

## Expected Screen Behavior

The firmware runs a continuous loop of display tests:

1. **Splash Screen** (2s)
   - Shows "SH1106 Test"
   - Shows "ESP32-C6"
   - Shows "Minino"

2. **Text Rendering** (2s)
   - Small text (size 1)
   - Medium text (size 2)
   - Large text (size 3)

3. **Line Drawing** (2s)
   - Grid pattern
   - Diagonal lines

4. **Rectangle Drawing** (3s)
   - Outline rectangles
   - Filled rectangles

5. **Circle Drawing** (3s)
   - Concentric circles (outline)
   - Filled circle

6. **Invert Test** (2.5s)
   - Normal display
   - Inverted display
   - Back to normal

7. **Bitmap Test** (1.5s)
   - Logo animation

8. **Bouncing Ball Animation** (5s)
   - Ball bouncing within borders

9. **Scrolling Text** (variable)
   - Scrolling text effect

10. **Pattern Test** (1.6s)
    - Fill patterns

After completing all tests, the sequence repeats.

## Test Patterns Description

### Graphics Primitives Tested

| Test | Description |
|------|-------------|
| Pixel | Single pixel on/off |
| Line | Bresenham line algorithm |
| Rectangle | Outline and filled rectangles |
| Circle | Midpoint circle algorithm |
| Text | 5x7 font rendering |
| Bitmap | 16x16 bitmap rendering |
| Animation | Frame-by-frame animation |

### Performance Metrics

- **Frame Rate**: ~20-30 FPS (depending on content)
- **I2C Speed**: 400 kHz
- **Memory Usage**: ~1KB for framebuffer

## Troubleshooting

### Display Not Working

**Problem**: Display remains blank

**Solutions**:
1. Check I2C connections (SDA/SCL)
2. Verify power supply (3.3V)
3. Check display address (0x3C vs 0x3D)
4. Try different I2C pins

### I2C Communication Errors

**Problem**: "Failed to initialize SH1106"

**Solutions**:
1. Check I2C pullup resistors
2. Verify I2C address with `i2c_scanner`
3. Try lower I2C speed (100kHz)
4. Check for short circuits

### Display Shows Garbage

**Problem**: Random pixels or corrupted display

**Solutions**:
1. Check power supply stability
2. Verify I2C clock speed
3. Check for EMI interference
4. Reset the display

### Build Errors

**Problem**: "idf.py: command not found"

**Solutions**:
1. Install ESP-IDF 5.4
2. Source the export script: `source ~/esp/idf5/export.sh`
3. Use the build script: `./build.sh`

### Flash Errors

**Problem**: "Failed to connect to ESP32-C6"

**Solutions**:
1. Check USB cable
2. Press BOOT button while flashing
3. Verify serial port name
4. Install USB-serial drivers

## Memory Layout

```
Flash:
- 0x1000: Bootloader
- 0x8000: Partition table
- 0x10000: Application
- 0x300000: End of app

RAM:
- Framebuffer: 1024 bytes (128x64/8)
- Stack: 8KB default
- Heap: ~200KB available
```

## Customization

### Changing Display Size

Edit `components/sh1106_driver/include/sh1106.h`:

```c
// For 128x32 display
#define SH1106_128_32
// #define SH1106_128_64
```

### Changing I2C Address

Edit `components/sh1106_driver/include/sh1106.h`:

```c
#define SH1106_I2C_ADDRESS 0x3D  // or 0x3C
```

### Adding Custom Tests

Edit `main/screen_test.c` and add your test function:

```c
static void test_custom(sh1106_t *display) {
    sh1106_clear_display(display);
    // Your test code here
    sh1106_display(display);
}
```

## API Reference

### Driver Functions

```c
// Initialization
esp_err_t sh1106_init(sh1106_t *display, int sda_io, int scl_io, int reset_io, i2c_port_t i2c_port);
esp_err_t sh1106_deinit(sh1106_t *display);

// Display control
esp_err_t sh1106_display_on(sh1106_t *display);
esp_err_t sh1106_display_off(sh1106_t *display);
esp_err_t sh1106_clear_display(sh1106_t *display);
esp_err_t sh1106_invert_display(sh1106_t *display, bool invert);
esp_err_t sh1106_set_contrast(sh1106_t *display, uint8_t contrast);

// Drawing functions
esp_err_t sh1106_draw_pixel(sh1106_t *display, int16_t x, int16_t y, uint16_t color);
esp_err_t sh1106_draw_line(sh1106_t *display, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
esp_err_t sh1106_draw_rect(sh1106_t *display, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color, bool filled);
esp_err_t sh1106_draw_circle(sh1106_t *display, int16_t x0, int16_t y0, int16_t r, uint16_t color, bool filled);
esp_err_t sh1106_draw_char(sh1106_t *display, int16_t x, int16_t y, char c, uint16_t color, int size);
esp_err_t sh1106_draw_string(sh1106_t *display, int16_t x, int16_t y, const char *str, uint16_t color, int size);
esp_err_t sh1106_draw_bitmap(sh1106_t *display, int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color);
esp_err_t sh1106_display(sh1106_t *display);
```

## License

Based on Adafruit SH1106 library.
BSD license - see original repository for details.

## Version History

- **v1.0.0** - Initial release
  - ESP-IDF 5.4 compatible
  - ESP32-C6 support
  - I2C interface
  - Basic graphics primitives
  - Text rendering
  - Animation tests
