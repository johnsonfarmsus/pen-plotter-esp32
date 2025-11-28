# WiFi Pen Plotter - Project Plan & Structure

**Created:** November 2025
**Status:** Active Development - Custom Implementation

---

## Project Goal

Create a WiFi-enabled pen plotter with an MS Paint-style web interface that users can access via their phone/tablet to draw simple designs, add text, or upload images for plotting.

---

## Hardware

- **Microcontroller:** ESP32 DevKit (LUIRSAY 38-pin)
- **Motors:** 3x 28BYJ-48 stepper motors (5V, unipolar)
- **Drivers:** 3x ULN2003 motor driver boards
- **GPIO Pins:**
  - X-Axis: GPIO 13, 14, 27, 26
  - Y-Axis: GPIO 16, 17, 5, 18
  - Z-Axis: GPIO 19, 21, 22, 23
- **Motor Specs:**
  - 4096 steps/revolution (half-step mode)
  - 64:1 gear ratio
  - Unipolar control (4 phases per motor)

---

## Architecture Decision

### Why NOT FluidNC?

- **Too Complex:** FluidNC added unnecessary layers for our simple use case
- **Config Issues:** Difficult to upload config.yaml, captive portal problems
- **Overhead:** We don't need full CNC features, just basic XYZ movement
- **Development Friction:** Required WiFi for development, complicated serial config

### Custom Solution Approach

**Simple & Direct:**
1. Custom ESP32 sketch with built-in web server
2. Direct motor control (we already tested and works!)
3. Simple G-code parser for basic movement commands
4. WiFi AP mode serving the HTML interface
5. All development via USB serial (no WiFi needed until deployment)

---

## Project Structure

```
pen_plotter_esp32/
├── README.md                      # Main project documentation
├── PROJECT_PLAN.md               # This file - architecture & decisions
├── plotter_sketch/               # Main Arduino/PlatformIO project
│   ├── platformio.ini            # PlatformIO configuration
│   ├── src/
│   │   ├── main.cpp              # Main sketch (web server + setup)
│   │   ├── motor_control.h       # Motor control functions
│   │   ├── gcode_parser.h        # G-code interpreter
│   │   └── web_interface.h       # Embedded HTML interface
│   └── lib/                      # Libraries (if needed)
├── docs/                         # Documentation
│   ├── hardware-setup.md         # Wiring diagrams & setup
│   ├── user-guide.md             # How to use the plotter
│   └── development-notes.md      # Technical notes
└── archive/                      # Old files (FluidNC attempts, etc.)
```

---

## Software Components

### 1. Web Server (main.cpp)
- **Library:** ESP32 WebServer
- **Mode:** WiFi Access Point
- **SSID:** PlotterBot
- **Password:** plot2025
- **IP:** 192.168.4.1
- **Features:**
  - Serve HTML interface at root `/`
  - Accept G-code via POST to `/gcode`
  - Status endpoint `/status`

### 2. Motor Control (motor_control.h)
- **Control Type:** Unipolar half-step
- **Step Sequence:** 8-step pattern (already tested and working)
- **Functions:**
  - `void initMotors()` - Setup GPIO pins
  - `void stepMotor(char axis, int steps, bool dir)` - Move specific motor
  - `void setPosition(float x, float y, float z)` - Move to position
  - `void homeMotors()` - Return to origin

### 3. G-code Parser (gcode_parser.h)
- **Supported Commands:**
  - `G0 X__ Y__ Z__` - Rapid positioning (pen up)
  - `G1 X__ Y__ Z__ F__` - Linear move (pen down)
  - `G28` - Home all axes
  - `M3` - Pen down
  - `M5` - Pen up
  - `M114` - Get current position
- **Coordinate System:** Relative and absolute modes

### 4. Web Interface (web_interface.h)
- **Source:** plotter-interface.html (converted to C string)
- **Features:**
  - Drawing tools: Line, Rectangle, Circle, Freehand, Text
  - SVG import
  - G-code generation from drawings
  - Direct communication with ESP32 via fetch() API
  - Touch-friendly for mobile devices

---

## Development Workflow

### Phase 1: Core Motor Control ✓
- [x] Test motors with basic sketch
- [x] Verify GPIO pin mappings
- [x] Confirm half-step sequence works
- [x] Replace damaged ULN2003 boards

### Phase 2: Custom Sketch (Current)
- [ ] Create PlatformIO project structure
- [ ] Implement motor control module
- [ ] Add simple G-code parser
- [ ] Embed web interface
- [ ] Test via serial commands

### Phase 3: Web Server Integration
- [ ] Set up WiFi AP mode
- [ ] Serve HTML interface
- [ ] Handle G-code POST requests
- [ ] Test drawing → motor movement pipeline

### Phase 4: Interface Refinement
- [ ] Optimize for mobile/tablet
- [ ] Add calibration features
- [ ] Implement preview mode
- [ ] Error handling & status feedback

### Phase 5: Advanced Features (Future)
- [ ] Image to G-code conversion
- [ ] SVG optimization
- [ ] Pause/resume functionality
- [ ] Save/load drawings

---

## Design Decisions & Rationale

### 1. Why Embedded HTML vs SPIFFS?
**Decision:** Embed HTML as C string in code

**Rationale:**
- Simpler deployment (no filesystem upload needed)
- Faster serving (no file I/O overhead)
- Single firmware file to flash
- Easier version control (HTML + code together)

**Trade-off:** Larger firmware size, but ESP32 has plenty of flash

### 2. Why PlatformIO over Arduino IDE?
**Decision:** Use PlatformIO

**Rationale:**
- Better dependency management
- Easier multi-file organization
- Command-line builds (tested and working)
- Better for version control

### 3. Why WiFi AP mode only?
**Decision:** Access Point mode (not Station mode)

**Rationale:**
- No external WiFi dependency
- Direct peer-to-peer connection
- Simpler user experience (just connect to PlotterBot)
- No internet needed for plotting

**Future:** Could add Station mode as option for cloud features

### 4. Why Custom G-code Parser vs Library?
**Decision:** Write minimal custom parser

**Rationale:**
- Only need ~6 commands
- Avoid external dependencies
- Full control over behavior
- Educational value
- Smaller code footprint

---

## Motor Control Specifications

### Steps Per MM Calculations
- **X/Y Axes:** 80 steps/mm
  - Based on: 4096 steps/rev ÷ pulley/belt configuration
- **Z Axis (Pen):** 200 steps/mm
  - Finer control for pen up/down

### Movement Speeds
- **Rapid (G0):** 2000 mm/min
- **Drawing (G1):** 1000 mm/min (adjustable)
- **Homing:** 500 mm/min

### Limits
- **X Travel:** 200mm
- **Y Travel:** 200mm
- **Z Travel:** 20mm (pen up/down)

---

## API Specification

### Web Interface → ESP32 Communication

#### POST /gcode
Send G-code commands to execute

**Request:**
```json
{
  "gcode": "G0 X10 Y10\nG1 X20 Y20\n"
}
```

**Response:**
```json
{
  "status": "ok",
  "lines_processed": 2,
  "message": "G-code executed successfully"
}
```

#### GET /status
Get current plotter status

**Response:**
```json
{
  "state": "idle",
  "position": {
    "x": 10.5,
    "y": 20.3,
    "z": 0.0
  },
  "pen": "up"
}
```

---

## Testing Strategy

### Unit Testing
1. **Motor Control:** Test each axis independently
2. **G-code Parser:** Test command parsing accuracy
3. **Web Server:** Test HTTP endpoints

### Integration Testing
1. **Drawing → G-code:** Verify interface generates correct G-code
2. **G-code → Movement:** Verify motors move correctly
3. **End-to-End:** Draw simple shape, verify plotted output

### User Acceptance Testing
1. Connect via phone to PlotterBot WiFi
2. Draw simple design
3. Upload and plot
4. Verify output matches expectation

---

## Known Issues & Solutions

### Issue: FluidNC Config Upload
**Problem:** Couldn't upload config.yaml via HTTP or serial
**Solution:** Abandoned FluidNC, building custom solution

### Issue: ULN2003 Reverse Polarity
**Problem:** Connected power backwards, fried Y and Z boards
**Solution:** Replaced boards, documented correct polarity

### Issue: Serial Spam (92.168.0.2)
**Problem:** WiFi status messages flooding serial output
**Solution:** Will filter in custom code or disable WiFi during serial debug

---

## Future Enhancements

### V1.1 Features
- [ ] Drawing templates library
- [ ] Grid overlay for precision
- [ ] Undo/redo functionality
- [ ] Export drawings as SVG

### V2.0 Features
- [ ] Image to line-art conversion
- [ ] Multiple pen colors (manual swap)
- [ ] Automatic calibration routine
- [ ] Cloud save/share drawings

### V3.0 Features
- [ ] Camera alignment assistance
- [ ] Multi-layer drawings
- [ ] Animation/time-lapse plotting
- [ ] Mobile app (instead of web interface)

---

## Resources & References

### Documentation
- [ESP32 Arduino Core](https://docs.espressif.com/projects/arduino-esp32/)
- [28BYJ-48 Datasheet](http://www.4tronix.co.uk/arduino/Stepper-Motors.php)
- [G-code Reference](https://marlinfw.org/meta/gcode/)

### Libraries Used
- ESP32 WebServer (built-in)
- WiFi (built-in)
- No external dependencies!

---

## Project Timeline

- **Nov 23, 2025:** Project started, requirements defined
- **Nov 26, 2025:** Hardware tested, motors working
- **Nov 27, 2025:** FluidNC attempted and abandoned
- **Nov 27, 2025:** Custom solution architecture designed
- **Nov 27, 2025:** Project cleanup and restructure

---

## Contributors

- Trevor Johnson - Hardware & Software Development
- Claude (Anthropic) - Code Assistant

---

## License

Personal project - All rights reserved

---

*Last Updated: November 27, 2025*
