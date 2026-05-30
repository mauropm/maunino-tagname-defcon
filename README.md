# DEF CON Badge - Electronic Cats Minino (ESP32-C6)

DEF CON-style electronic badge/tagname firmware for the **Electronic Cats Minino** (ESP32-C6) with SH1106 128x64 OLED display.

The firmware continuously loops through a sequence of animated screens:

1. **DEF CON Logo** (5s) - Pixel art DEF CON logo with fade-in effect
2. **Liquid Splash** (1s) - Procedural liquid/paint splash covering the screen
3. **Windshield Wiper** (5s) - Virtual wiper cleans the splash away, revealing the tagname
4. **Tagname "MAURO"** (5s) - Large centered text with CRT scanline flicker
5. **Electronic Cats Logo** (5s) - Pixel art Electronic Cats logo with fade-in
6. **Repeat forever** (~21s total cycle)

Pure C, ESP-IDF 5.x, static memory allocation, no dynamic allocation during animation.

## Hardware Requirements

- **Board**: Electronic Cats Minino (ESP32-C6)
- **Display**: SH1106 OLED 128x64 with I2C interface
- **Connection**:
  - I2C SDA: GPIO6
  - I2C SCL: GPIO7
  - VCC: 3.3V
  - GND: GND

## Animation Sequence

```
BOOT → DEFCON_LOGO → LIQUID_SPLASH → WIPER_REVEAL → TAGNAME → ELECTRONIC_CATS → LOOP
```

| Screen | Duration | Description |
|--------|----------|-------------|
| DEF CON Logo | 5s | 124x64px pixel art logo, centered, with fade-in |
| Liquid Splash | 1s | Procedural expanding blobs covering the display |
| Wiper Reveal | 5s | 3-pass windshield wiper cleaning animation |
| Tagname | 5s | "MAURO" at 3x size, centered, with scanline flicker |
| Electronic Cats | 5s | 124x64px pixel art logo, centered, with fade-in |

## Importing Custom Bitmaps

All logo bitmaps are stored in `main/badge_bitmaps.h` as C arrays. To add your own bitmap:

### Step-by-Step Guide

1. **Create pixel art** at https://pixelartvillage.com/
   - Design your logo or graphic
   - Export/download the image

2. **Resize the image**
   - Open the downloaded image in any image editor
   - Resize it to fit within 128x64 pixels (keep aspect ratio, best fit possible)

3. **Convert to Arduino C code** at https://javl.github.io/image2cpp/
   - Upload your resized image
   - Set **Canvas size** to `128x64`
   - Select **Drawing mode**: `Atkinson` dithering
   - Check **Scale to fit** (keep original size)
   - Check **Center** horizontally and vertically
   - Click **Generate code**
   - Copy the generated Arduino `PROGMEM` array

4. **Ask opencode to integrate it**
   - Paste the Arduino code and ask opencode to convert it to the project's bitmap format
   - The bitmap will be added to `main/badge_bitmaps.h` and wired into the appropriate screen

### Bitmap Format

Bitmaps in `badge_bitmaps.h` use MSB-first, row-by-row format:

```c
#define MY_LOGO_W 124
#define MY_LOGO_H 64

static const uint8_t my_logo_bmp[] = {
    0xff, 0xff, ..., // Row 0 (16 bytes for 124px width)
    0xff, 0xff, ..., // Row 1
    // ... 64 rows total
};
```

## Build Instructions

### Quick Build

```bash
./build.sh build
```

### Manual Build

```bash
use_idf5
idf.py build
```

## Flash Instructions

```bash
./build.sh flash
```

Or manually:

```bash
use_idf5
idf.py -p /dev/cu.usbserial-1 flash
```

## Monitor

```bash
./build.sh monitor
```

Or manually:

```bash
use_idf5
idf.py -p /dev/cu.usbserial-1 monitor
```

## Clean

```bash
./build.sh clean
```

## Project Structure

```
maunino-tagname-defcon/
├── CMakeLists.txt              # Project configuration
├── sdkconfig.defaults          # SDK defaults (ESP32-C6)
├── main/
│   ├── CMakeLists.txt          # Main component config
│   ├── conference_badge.c      # DEF CON badge firmware (animation state machine)
│   └── badge_bitmaps.h         # Bitmap assets (DEF CON, Electronic Cats)
├── components/
│   └── sh1106_driver/
│       ├── CMakeLists.txt
│       ├── include/sh1106.h    # Driver header
│       └── sh1106.c            # SH1106 driver implementation
├── build.sh                    # Build/flash/monitor helper
├── flash.sh                    # Flash script
├── monitor.sh                  # Serial monitor script
├── clean.sh                    # Clean build artifacts
└── README.md                   # This file
```

## Memory Usage

| Resource | Size |
|----------|------|
| Framebuffer (static) | 1024 bytes |
| Splash blobs (64 max) | 320 bytes |
| Wipe mask | 128 bytes |
| Bitmap assets | ~2KB |
| Task stack | 4096 bytes |

No dynamic allocation during animation. All buffers are static.

## Troubleshooting

### Port Busy Error

Close any other programs using the serial port, disconnect/reconnect USB.

### I2C NACK Errors

1. Check I2C wiring (SDA=GPIO6, SCL=GPIO7)
2. Verify display is powered (3.3V)
3. Try different I2C address (0x3C vs 0x3D)

### Display Shows Inverted Colors

Check the `SH1106_INVERT_DISPLAY` command in `sh1106.c` init sequence.

### Build Errors

```bash
use_idf5
source ~/esp/idf5/export.sh
idf.py build
```

## License

Based on Adafruit SH1106 library.
BSD License - see original repository for details.

## Support

For issues or questions:
1. Check the troubleshooting section above
2. Verify your ESP-IDF installation
3. Ensure proper wiring and power supply
4. Check serial output for error messages
