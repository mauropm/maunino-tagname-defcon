# SH1106 OLED Screen Test for Electronic Cats Minino (ESP32-C6)

This is a minimal standalone firmware for testing SH1106 OLED displays on the **Electronic Cats Minino** development board (ESP32-C6). The firmware validates display functionality including I2C communication, framebuffer rendering, text, graphics, and animations.

## Hardware Requirements

- **Board**: Electronic Cats Minino (ESP32-C6)
- **Display**: SH1106 OLED 128x64 with I2C interface
- **Connection**:
  - I2C SDA: GPIO6
  - I2C SCL: GPIO7
  - VCC: 3.3V
  - GND: GND

## What This Demo Shows

The firmware runs a continuous loop of display tests:

1. **Splash Screen** - Shows "SH1106 Test", "ESP32-C6", and "Minino"
2. **Text Rendering** - Tests three font sizes (small, medium, large)
3. **Line Drawing** - Grid pattern and diagonal lines
4. **Rectangle Drawing** - Outline and filled rectangles
5. **Circle Drawing** - Concentric circles and filled circles
6. **Invert Test** - Toggles display inversion
7. **Bitmap Test** - Renders a 16x16 logo with animation
8. **Bouncing Ball Animation** - Ball bouncing within borders
9. **Scrolling Text** - Scrolling text effect
10. **Pattern Test** - Checkerboard fill patterns

## Prerequisites

### 1. ESP-IDF 5.4 Installation

Ensure you have ESP-IDF 5.4 installed with ESP32-C6 support.

### 2. Environment Configuration

This project requires the ESP-IDF environment to be properly configured. You need to add the ESP-IDF environment switching functions to your shell configuration file.

**For macOS/Linux with Zsh (recommended):**

Add the following to your `~/.zshrc` or `~/.zprofile`:

```bash
# ESP-IDF Environment Configuration
export IDF_TOOLS_BASE="$HOME/.espressif"

# ESP-IDF 5.x configuration
export IDF5_PATH="$HOME/esp/idf5"
export IDF5_PYTHON_ENV="$IDF_TOOLS_BASE/python_env/idf5_py3.11_env"

# Function to activate ESP-IDF 5.x environment
use_idf5() {
    export IDF_TOOLS_PATH="$IDF_TOOLS_BASE/idf5_tools"
    export IDF_PYTHON_ENV_PATH="$IDF5_PYTHON_ENV"
    export IDF_PATH="$IDF5_PATH"
    
    # Activate the environment
    . "$IDF5_PATH/export.sh" >/dev/null 2>&1
    echo "✓ Switched to ESP-IDF 5.4"
    echo "  Python: $(python --version 2>&1)"
    echo "  IDF: $(idf.py --version 2>&1)"
}

# Short alias
alias idf5='use_idf5'
```

**For Bash users:**

Add the same code to your `~/.bash_profile` or `~/.bashrc`.

**For Windows (WSL):**

Add to your `~/.bashrc` in WSL.

### 3. Verify Installation

After configuring your shell, verify the setup:

```bash
# Load your shell configuration
source ~/.zshrc  # or source ~/.bash_profile

# Activate ESP-IDF 5 environment
use_idf5

# Verify installation
idf.py --version
python --version
idf.py --list-targets
```

Expected output:
```
ESP-IDF v5.4.1
Python 3.11.x
esp32c6
```

## Build Instructions

### Quick Build (using scripts)

```bash
cd screen_test

# Build the project
./build.sh
```

### Manual Build

```bash
cd screen_test

# Activate ESP-IDF 5 environment
use_idf5

# Set target to ESP32-C6 (if not already set)
idf.py set-target esp32c6

# Build the project
idf.py build
```

### Build Output

On successful build, you'll see:
```
Generated /path/to/screen_test/build/screen_test.bin
screen_test.bin binary size 0x28b40 bytes (162 KB)
```

## Flash Instructions

### Using Flash Script

```bash
./flash.sh
```

### Manual Flash

```bash
# Activate ESP-IDF 5 environment
use_idf5

# Flash to device (replace with your serial port)
idf.py -p /dev/cu.usbmodem21201 flash
```

**Common serial ports:**
- macOS: `/dev/cu.usbmodem*` or `/dev/cu.usbserial-*`
- Linux: `/dev/ttyUSB0` or `/dev/ttyACM0`
- Windows: `COM3`, `COM4`, etc.

### Find Your Serial Port

**macOS:**
```bash
ls /dev/cu.* | grep -E "usbmodem|serial"
```

**Linux:**
```bash
ls /dev/ttyUSB* /dev/ttyACM* 2>/dev/null
```

**Windows:**
```powershell
Get-PnpDevice -Class Ports | Where-Object { $_.Present -eq $True }
```

## Monitor Serial Output

### Using Monitor Script

```bash
./monitor.sh
```

### Manual Monitor

```bash
use_idf5
idf.py -p /dev/cu.usbmodem21201 monitor
```

### Expected Serial Output

```
I (0) SCREEN_TEST: Starting SH1106 Screen Test
I (0) SCREEN_TEST: Target: ESP32-C6 (Minino)
I (0) SCREEN_TEST: Display: SH1106 128x64 OLED
I (0) SCREEN_TEST: Initializing SH1106 on I2C (SDA=6, SCL=7)
I (0) SH1106: Display found at I2C address 0x3C
I (0) SH1106: SH1106 initialized on I2C port 0
I (0) SCREEN_TEST: SH1106 initialized successfully
I (0) SCREEN_TEST: === Starting test sequence ===
I (0) SCREEN_TEST: Test: Splash screen
I (0) SCREEN_TEST: Calling sh1106_display()...
I (0) SCREEN_TEST: Display result: ESP_OK
I (0) SH1106: Display update complete
```

## Clean Build Artifacts

```bash
./clean.sh
```

Or manually:
```bash
use_idf5
idf.py clean
```

## Troubleshooting

### Port Busy Error

**Error**: `Could not open /dev/cu.usbmodem21201, the port is busy`

**Solution**:
1. Close any other programs using the serial port
2. Disconnect and reconnect the USB cable
3. Try a different USB port

### I2C NACK Errors

**Error**: `I2C transaction unexpected nack detected`

**Solutions**:
1. Check I2C wiring (SDA/SCL)
2. Verify display is powered (3.3V)
3. Try different I2C address (0x3C vs 0x3D)
4. Add external pullup resistors (4.7kΩ) if needed

### Display Shows Inverted Colors

If you see white background with black text instead of black background with white text:

1. The display may need different initialization
2. Check the `SH1106_INVERT_DISPLAY` command in `sh1106.c`

### Build Errors

**Error**: `idf.py: command not found`

**Solution**:
1. Ensure ESP-IDF is installed
2. Source the export script: `source ~/esp/idf5/export.sh`
3. Or use: `use_idf5`

### Flash Size Warning

**Warning**: `Detected size(8192k) larger than the size in the binary image header(2048k)`

This is normal and can be ignored. The firmware works correctly with 8MB flash chips.

## Project Structure

```
screen_test/
├── CMakeLists.txt              # Project CMake configuration
├── sdkconfig.defaults          # SDK defaults for ESP32-C6
├── main/
│   ├── CMakeLists.txt          # Main component configuration
│   └── screen_test.c           # Test application code
├── components/
│   └── sh1106_driver/
│       ├── CMakeLists.txt      # Driver component configuration
│       ├── include/
│       │   └── sh1106.h        # Driver header
│       └── sh1106.c            # Driver implementation
├── build.sh                    # Build script
├── flash.sh                    # Flash script
├── monitor.sh                  # Monitor script
├── clean.sh                    # Clean script
└── README.md                   # This file
```

## Customization

### Change I2C Pins

Edit `main/screen_test.c`:
```c
int sda_gpio = 6;  // Change to your SDA pin
int scl_gpio = 7;  // Change to your SCL pin
```

### Change Display Size

Edit `components/sh1106_driver/include/sh1106.h`:
```c
// For 128x32 display
#define SH1106_128_32
// #define SH1106_128_64
```

### Adjust Contrast

In `sh1106.c`, modify the contrast value:
```c
sh1106_write_command(display, SH1106_SET_CONTRAST);
sh1106_write_command(display, 0xCF);  // Change 0xCF to adjust contrast
```

## License

Based on Adafruit SH1106 library.  
BSD License - see original repository for details.

## Version History

- **v1.0.0** - Initial release
  - ESP-IDF 5.4 compatible
  - ESP32-C6 support (Minino)
  - I2C address auto-detection (0x3C/0x3D)
  - Color inversion for proper rendering
  - Comprehensive display tests
  - Chunked I2C data transmission

## Support

For issues or questions:
1. Check the troubleshooting section above
2. Verify your ESP-IDF installation
3. Ensure proper wiring and power supply
4. Check serial output for error messages
