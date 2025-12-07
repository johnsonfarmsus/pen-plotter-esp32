# WiFi Pen Plotter

A WiFi-enabled pen plotter with an MS Paint-style web interface. Draw on your phone or tablet and watch it plot!

Full writeup and 3d print files here https://hackaday.io/project/204593-pen-plotter-esp32 and https://www.printables.com/model/1503490-pen-plotter-esp32.

## Features

- **WiFi Control** - No USB connection needed, control via web browser
- **Touch-Friendly Interface** - Draw with your finger on phone/tablet
- **Drawing Tools** - Freehand draw and text plotting with Block font
- **Text Plotting** - Multi-line text support with adjustable sizes
- **File Upload** - Upload SVG or G-code files for plotting
- **Real-time Preview** - See your drawing before plotting
- **Custom Motor Control** - Direct stepper motor control without external firmware

## Quick Start

### Get the Firmware

**Option 1: Download Release (Recommended)**

1. Go to the [Releases page](https://github.com/johnsonfarmsus/pen-plotter-esp32/releases)
2. Download the latest release (`.zip` file)
3. Extract the zip file to a folder on your computer (e.g., `Downloads/pen-plotter-esp32`)

**Option 2: Clone from GitHub**

```bash
git clone https://github.com/johnsonfarmsus/pen-plotter-esp32.git
cd pen-plotter-esp32
```

### Prerequisites

Install PlatformIO Core:

**Windows:**
```bash
# Install using pip (requires Python 3.6+)
pip install platformio

# Verify installation
pio --version
```

**macOS/Linux:**
```bash
# Install using pip
pip3 install platformio

# Verify installation
pio --version
```

For detailed installation instructions, see the [PlatformIO installation guide](https://docs.platformio.org/en/latest/core/installation/index.html).

### 1. Upload the Firmware

**Connect your ESP32** to your computer via USB, then navigate to the firmware folder and upload:

**Windows:**
```bash
cd pen-plotter-esp32/plotter_sketch
pio run --target upload
```

**macOS/Linux:**
```bash
cd pen-plotter-esp32/plotter_sketch
pio run --target upload
```

**What happens during upload:**
- PlatformIO detects your ESP32 and compiles the firmware
- Upload takes 30-60 seconds
- You'll see a "SUCCESS" message when complete

**If upload fails:**
- Press and hold the "BOOT" button on your ESP32, then try uploading again
- Some boards require holding BOOT + pressing RESET to enter programming mode
- Verify your USB cable supports data transfer (not just power)

### 2. Connect to PlotterBot WiFi

- SSID: `PlotterBot`
- Password: `plot2025`

### 3. Open Web Interface

Open your browser to: `http://192.168.4.1` or `http://plotter.local`

### 4. Draw and Plot!

- Select a drawing tool (Draw or Text)
- Create your design on the canvas or upload an SVG/G-code file
- Click "Send to Plotter" to start plotting
- Watch your creation come to life!

## Hardware Requirements

### Components

- **ESP32 DevKit** (38-pin recommended)
- **3x 28BYJ-48 stepper motors** (5V unipolar)
- **3x ULN2003 driver boards** (Darlington transistor arrays)
- **Power Supply** - 5V DC (1-2A recommended)
- **Pen holder mechanism** - For Z-axis
- **Frame/Rails** - For X and Y movement

### Wiring

See [HARDWARE_SETUP.md](HARDWARE_SETUP.md) for detailed wiring diagrams.

**GPIO Pin Assignments:**
- X-Axis Motor: GPIO 13, 14, 27, 26
- Y-Axis Motor: GPIO 16, 17, 5, 18
- Z-Axis Motor: GPIO 19, 21, 22, 23

## Calibration

The plotter includes calibration settings in `plotter_sketch/src/motor_control.h`:

```cpp
#define STEPS_PER_MM_X 40    // Adjust for your X-axis mechanical setup
#define STEPS_PER_MM_Y 40    // Adjust for your Y-axis mechanical setup
#define STEPS_PER_MM_Z 50    // Adjust for pen lift height
```

**To calibrate:**
1. Draw a known size (e.g., 100x100px square)
2. Measure the actual plotted size
3. Adjust `STEPS_PER_MM_X/Y` proportionally
4. Higher values = smaller movements, Lower values = larger movements

**Axis Inversion:**
If your plotter is mirrored, change the inversion settings:
```cpp
#define X_INVERT -1  // Change to 1 if X-axis is backwards
#define Y_INVERT 1   // Change to -1 if Y-axis is backwards
```

## Architecture

Built with custom firmware (no GRBL/FluidNC dependency).

### Key Components

- **motor_control.h** - Direct stepper motor control using half-step sequences
- **gcode_parser.h** - Simple G-code interpreter (G0, G1, G28, M3, M5)
- **web_interface.h** - Embedded HTML/CSS/JS interface with text plotting and SVG support
- **main.cpp** - WiFi AP, web server, and captive portal setup

## Development

Built with PlatformIO for ESP32.

### Build Commands

```bash
cd plotter_sketch

# Build only
pio run

# Build and upload
pio run --target upload

# Monitor serial output
pio device monitor
```

### Project Structure

```
pen_plotter_esp32/
├── plotter_sketch/          # Main PlatformIO project
│   ├── src/
│   │   ├── main.cpp         # Entry point, WiFi & web server
│   │   ├── motor_control.h  # Stepper motor control
│   │   ├── gcode_parser.h   # G-code interpreter
│   │   └── web_interface.h  # Embedded web UI
│   └── platformio.ini       # PlatformIO configuration
├── HARDWARE_SETUP.md       # Hardware assembly guide
├── README.md               # This file
└── LICENSE                 # GNU AGPL v3.0
```

## Web Interface

The web interface is fully embedded in the ESP32 firmware (no SPIFFS required).

**Drawing Tools:**
- ✏️ Freehand Draw - Touch/click and drag to draw
- 📝 Text - Add multi-line text with Block font (adjustable sizes: Small, Medium, Large, Extra Large)
- 📁 Upload - Upload SVG or G-code files

**Controls:**
- 🚀 Send to Plotter - Execute your drawing (with confirmation dialog)
- 🗑️ Clear - Erase canvas

## Troubleshooting

**Plotter doesn't move:**
- Check power supply to motors
- Verify GPIO pin connections
- Check serial monitor for errors

**Drawing is mirrored:**
- Adjust `X_INVERT` or `Y_INVERT` in motor_control.h

**Drawing is wrong size/shape:**
- Calibrate `STEPS_PER_MM_X` and `STEPS_PER_MM_Y`
- Check mechanical setup (belts, pulleys, etc.)

**Pen doesn't touch paper:**
- Adjust `STEPS_PER_MM_Z` value
- Check Z-axis motor connection
- Verify pen mechanism height

**WiFi doesn't appear:**
- Wait 10-15 seconds after power-on
- Check serial monitor for "AP started" message
- Try rebooting ESP32

## G-Code Support

Supported G-code commands:
- `G0` - Rapid positioning (pen up)
- `G1` - Linear move (pen down)
- `G28` - Home all axes
- `G90` - Absolute positioning mode
- `G91` - Relative positioning mode
- `M3` - Pen down
- `M5` - Pen up
- `M114` - Report current position
- `M18/M84` - Disable motors


## Attribution

This project has 3d parts and inspiration from:

https://www.thingiverse.com/thing:4637226

https://www.thingiverse.com/thing:4607077

https://www.thingiverse.com/thing:6797388

https://www.pcbway.com/project/shareproject/Build_a_simple_3D_Arduino_Mini_CNC_Plotter_e2c3f905.html
