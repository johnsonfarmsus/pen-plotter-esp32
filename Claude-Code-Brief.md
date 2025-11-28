# Claude Code Project Brief: WiFi CNC Pen Plotter with FluidNC

## Project Overview

Build a **web-based drawing interface** that integrates with **FluidNC firmware** running on an ESP32 to create a WiFi-enabled CNC pen plotter suitable for high school classrooms.

**Core Workflow:**
1. Student connects phone/tablet to plotter's WiFi network
2. Opens web browser to `http://plotter.local`
3. Uses MS Paint-style drawing interface to create designs OR uploads SVG vectors
4. Clicks "Plot" button
5. Plotter draws the design
6. All controlled via open-source software (FluidNC + your custom interface)

**Target Audience:** High school students (14-18) and teachers  
**Hardware:** ESP32 DevKit (38-pin), 28BYJ-48 stepper motors, ULN2003 drivers  
**Budget:** ~$10 per plotter  
**Total Development Time:** 8-12 hours

---

## Technical Architecture

### System Overview

```
Student's Device (Phone/Tablet/Laptop)
    ↓
    [Browser connected to plotter WiFi]
    ↓
ESP32 Running FluidNC + Your Web Interface
    ├─ WiFi Access Point ("PlotterBot")
    ├─ HTTP Web Server (serves your HTML/CSS/JS)
    ├─ FluidNC G-code parser and motion controller
    ├─ GPIO motor drivers (to ULN2003 chips)
    └─ GPIO button inputs (emergency stop, jog, etc.)
    ↓
3× 28BYJ-48 Stepper Motors (X, Y, Z axes)
    ↓
3D-Printed Mechanical Assembly
```

### Technology Stack

**Firmware:**
- FluidNC (open-source CNC firmware, GPL-3.0)
- Runs on ESP32
- Pre-built, no compilation needed (just configuration)

**Web Interface (What You Build):**
- HTML5 Canvas (drawing)
- Vanilla JavaScript (no frameworks)
- Single HTML file (easy deployment)

**Communication:**
- HTTP POST for G-code upload
- WebSocket for real-time status updates
- RESTful API to FluidNC's `/api/gcode` endpoint

**Hardware:**
- ESP32 DevKit (38-pin version with 38 GPIO)
- Available GPIO pins: 18-20 (motors use 12, leaves 6-8 for buttons)

---

## Project Scope: What You're Building

### Phase 1: Web Drawing Interface (Core MVP)

**User-facing features:**
1. **Drawing Canvas**
   - 200mm × 200mm area (configurable)
   - Grid overlay (10mm spacing)
   - Visual feedback as shapes are drawn

2. **Shape Tools**
   - Line tool (click-to-click)
   - Rectangle (click-drag)
   - Circle (click-drag)
   - Text (click-place, type text, converts to outline)
   - Clear canvas
   - Undo last shape

3. **Drawing Preview**
   - Show pen-up moves as dashed lines
   - Show pen-down moves as solid lines
   - Estimated plot time display

4. **SVG Import**
   - File upload input
   - Parse SVG file
   - Convert SVG paths to drawable shapes
   - Preview uploaded design on canvas

5. **Plot Button**
   - Generate G-code from canvas shapes
   - Send G-code to FluidNC via HTTP POST
   - Show progress (optional: real-time status via WebSocket)
   - Emergency stop button (no wait for completion)

### Phase 2: Integration with FluidNC (Infrastructure)

**What FluidNC provides (you just configure):**
- ✅ WiFi access point
- ✅ HTTP web server
- ✅ G-code parser
- ✅ Motor control
- ✅ Position tracking
- ✅ Emergency stop (firmware handles `!` command)
- ✅ Web UI dashboard (alternative to your custom UI)

**What you provide:**
- Custom HTML/CSS/JS interface (simpler than full rewrite)
- G-code generation from shapes
- File hosting on ESP32 (copy HTML to FluidNC's storage)

### Phase 3: GPIO Button Support (Physical Controls)

**Physical buttons wired to GPIO pins:**
- Emergency stop (red, prominent)
- Jog X+, X-, Y+, Y-, Z+ (optional, if space allows)
- Mode selector (optional, for future features)

**How they work:**
- Button presses detected as GPIO interrupts
- FluidNC interprets them (stop, jog commands)
- No additional code needed (FluidNC built-in support)

---

## Detailed Specifications

### Web Interface File Structure

**Single HTML file** with embedded CSS and JavaScript:

```
plotter-interface.html (~400-500 lines total)
├─ HTML structure
│  ├─ Canvas element (400×400px display, 200×200mm scale)
│  ├─ Tool buttons (Line, Rectangle, Circle, Text, Clear, Undo)
│  ├─ File upload input (for SVG)
│  ├─ Control buttons (Preview, Plot, Stop, Reset)
│  └─ Status display (progress, position, messages)
│
├─ CSS (embedded in <style>)
│  ├─ Canvas styling
│  ├─ Button layout
│  ├─ Responsive design (works on mobile)
│  └─ Grid overlay on canvas
│
└─ JavaScript (embedded in <script>)
   ├─ Canvas drawing engine (~100 lines)
   ├─ Shape tools implementation (~80 lines)
   ├─ SVG parser (~60 lines)
   ├─ G-code generator (~100 lines)
   ├─ FluidNC API communication (~40 lines)
   └─ Event handlers and UI logic (~80 lines)
```

### Drawing Canvas Implementation

**Grid-based coordinate system:**
```javascript
const CANVAS_WIDTH_MM = 200;
const CANVAS_HEIGHT_MM = 200;
const CANVAS_DISPLAY_SIZE = 400; // pixels

// Scale: 2 pixels = 1mm on plotter
const MM_TO_PIXELS = CANVAS_DISPLAY_SIZE / CANVAS_WIDTH_MM; // 2

// When user clicks at pixel (100, 150), convert to mm:
const x_mm = (100 / MM_TO_PIXELS); // 50mm
const y_mm = (150 / MM_TO_PIXELS); // 75mm
```

**Grid overlay:**
```javascript
// Draw 10mm grid
const GRID_SIZE_MM = 10;
const GRID_SIZE_PX = GRID_SIZE_MM * MM_TO_PIXELS; // 20px

for (let x = 0; x <= CANVAS_WIDTH_MM; x += GRID_SIZE_MM) {
    drawLine(x * MM_TO_PIXELS, 0, x * MM_TO_PIXELS, CANVAS_DISPLAY_SIZE);
}
```

### Shape Representation

**Internal shape storage:**
```javascript
const shapes = [
    {
        type: 'line',
        x1: 10, y1: 10,
        x2: 50, y2: 50
    },
    {
        type: 'rectangle',
        x: 20, y: 20,
        width: 30, height: 40
    },
    {
        type: 'circle',
        x: 100, y: 100,
        radius: 25
    },
    {
        type: 'text',
        x: 50, y: 150,
        text: 'Hello',
        fontSize: 12
    }
];
```

### G-code Generation

**Output format (standard Grbl/FluidNC):**
```gcode
G21              ; Millimeter mode
G90              ; Absolute positioning
G0 Z1            ; Pen up (move Z to 1mm height)
G0 X10 Y10       ; Move to start position
G0 Z0            ; Pen down (Z to 0mm)
G1 X50 Y50 F1000 ; Draw line at 1000 mm/min feed rate
G0 Z1            ; Pen up
G0 X0 Y0         ; Return home
M2               ; Program end
```

**Generation pseudo-code:**
```javascript
function shapesToGcode(shapes) {
    let gcode = "G21 G90 G0 Z1\n";
    
    shapes.forEach(shape => {
        gcode += "G0 Z1\n"; // Pen up
        
        if (shape.type === 'line') {
            gcode += `G0 X${shape.x1} Y${shape.y1}\n`;
            gcode += "G0 Z0\n"; // Pen down
            gcode += `G1 X${shape.x2} Y${shape.y2} F1000\n`;
        }
        else if (shape.type === 'circle') {
            // Approximate with line segments
            const segments = 36;
            for (let i = 0; i <= segments; i++) {
                const angle = (i / segments) * 2 * Math.PI;
                const x = shape.x + shape.radius * Math.cos(angle);
                const y = shape.y + shape.radius * Math.sin(angle);
                
                if (i === 0) {
                    gcode += `G0 X${x.toFixed(3)} Y${y.toFixed(3)}\n`;
                    gcode += "G0 Z0\n";
                } else {
                    gcode += `G1 X${x.toFixed(3)} Y${y.toFixed(3)} F1000\n`;
                }
            }
        }
        // ... handle other shapes
        
        gcode += "G0 Z1\n"; // Pen up
    });
    
    gcode += "G0 X0 Y0\n"; // Return home
    gcode += "M2\n"; // Program end
    
    return gcode;
}
```

### SVG Import

**Parse SVG path elements:**
```javascript
async function uploadSVG(file) {
    const text = await file.text();
    const parser = new DOMParser();
    const doc = parser.parseFromString(text, 'text/xml');
    
    // Find all path elements
    const paths = doc.querySelectorAll('path');
    
    paths.forEach(pathElement => {
        const pathData = pathElement.getAttribute('d');
        
        // Simple parser: extract move (M) and line (L) commands
        const commands = pathData.match(/[A-Z][^A-Z]*/g);
        
        commands.forEach(command => {
            const type = command[0];
            const coords = command.slice(1).trim().split(/[\s,]+/);
            
            if (type === 'M') {
                // Move command (start of path)
                currentX = parseFloat(coords[0]);
                currentY = parseFloat(coords[1]);
            } else if (type === 'L') {
                // Line command
                const x = parseFloat(coords[0]);
                const y = parseFloat(coords[1]);
                
                shapes.push({
                    type: 'line',
                    x1: currentX, y1: currentY,
                    x2: x, y2: y
                });
                
                currentX = x;
                currentY = y;
            }
        });
    });
    
    redrawCanvas();
}
```

### FluidNC API Communication

**Send G-code to plotter:**
```javascript
async function sendToPlotter() {
    const gcode = shapesToGcode(shapes);
    
    try {
        const response = await fetch('/api/gcode', {
            method: 'POST',
            headers: {
                'Content-Type': 'text/plain',
            },
            body: gcode
        });
        
        if (response.ok) {
            updateStatus('Plotting...');
            // Optional: poll for completion
        } else {
            updateStatus('Error: ' + response.statusText);
        }
    } catch (error) {
        updateStatus('Connection error: ' + error.message);
    }
}
```

**Real-time status updates (optional WebSocket):**
```javascript
function startStatusMonitoring() {
    const ws = new WebSocket('ws://' + window.location.host + '/ws');
    
    ws.onmessage = (event) => {
        const status = JSON.parse(event.data);
        
        // Update position display
        document.getElementById('position').innerText = 
            `X: ${status.x.toFixed(2)} Y: ${status.y.toFixed(2)} Z: ${status.z.toFixed(2)}`;
        
        // Update progress
        if (status.progress !== undefined) {
            document.getElementById('progress').value = status.progress;
        }
    };
}
```

### Emergency Stop

**User clicks stop button → sends stop command to FluidNC:**
```javascript
async function emergencyStop() {
    // FluidNC accepts feed hold command
    await fetch('/api/stop', { method: 'POST' });
    updateStatus('STOPPED');
}
```

---

## FluidNC Configuration (YAML)

**File: `config.yaml` to be uploaded to ESP32**

```yaml
# Classroom CNC Plotter - FluidNC Configuration
# Hardware: ESP32 DevKit + 28BYJ-48 motors + ULN2003 drivers

board: "ESP32 Dev Board"
name: "PlotterBot"

# Motor definitions
motors:
  X:
    class: Stepper
    steps_per_mm: 100
    max_rate_mm_per_min: 3000
    acceleration_mm_per_sec2: 500
    direction_pin: GPIO.14
    step_pin: GPIO.12
    
  Y:
    class: Stepper
    steps_per_mm: 100
    max_rate_mm_per_min: 3000
    acceleration_mm_per_sec2: 500
    direction_pin: GPIO.16
    step_pin: GPIO.15
    
  Z:
    class: Stepper
    steps_per_mm: 100
    max_rate_mm_per_min: 1000
    acceleration_mm_per_sec2: 200
    direction_pin: GPIO.26
    step_pin: GPIO.25

# WiFi Access Point Configuration
wifi:
  mode: AP
  ssid: "PlotterBot"
  password: "plot2025"

# Axis limits (optional)
axes:
  x:
    max_travel_mm: 200
    soft_limits: true
  y:
    max_travel_mm: 200
    soft_limits: true
  z:
    max_travel_mm: 50

# Control pins (for physical buttons)
control:
  cycle_start_pin: GPIO.32
  feed_hold_pin: GPIO.33
  reset_pin: GPIO.34
```

---

## Deployment Process

### Step 1: Flash FluidNC (One-time setup)

1. Download Web Installer: https://github.com/breiler/fluid-installer
2. Connect ESP32 via USB
3. Run installer script
4. Wait for completion

### Step 2: Configure FluidNC

1. Edit `config.yaml` with motor pins for your hardware
2. Open browser to `http://192.168.4.1`
3. Upload `config.yaml` via FluidNC Web UI

### Step 3: Deploy Web Interface

1. Prepare `plotter-interface.html` (your custom code)
2. Upload to ESP32 via FluidNC's file manager
3. Access at `http://plotter.local/plotter-interface.html`
   OR serve at root if FluidNC configured to do so

### Step 4: Test

1. Power on plotter
2. Connect phone to "PlotterBot" WiFi
3. Open `http://plotter.local`
4. Draw test shape
5. Click Plot
6. Verify motors move correctly

---

## Browser Compatibility

**Required features:**
- HTML5 Canvas
- Fetch API
- FileReader API (for SVG upload)
- ES6 JavaScript

**Tested browsers:**
- ✅ Chrome/Edge (Primary)
- ✅ Firefox (Secondary)
- ✅ Safari (Secondary)
- ✅ Mobile browsers (Safari on iOS, Chrome on Android)

---

## User Interface Layout

```
┌──────────────────────────────────────────────────────┐
│  PlotterBot - Pen Plotter                            │
├──────────────────────────────────────────────────────┤
│                                                      │
│  DRAWING TOOLS     CANVAS               STATUS       │
│  ────────────      ──────               ──────       │
│  ○ Line            ┌────────────────┐   Connected    │
│  ○ Rectangle       │                │   Idle         │
│  ○ Circle          │  [Grid 10mm]   │   Position:    │
│  ○ Text            │                │   X: 0 Y: 0    │
│                    │                │                │
│  [Clear] [Undo]    │                │   Progress:    │
│                    └────────────────┘   ──────       │
│  FILE UPLOAD                                        │
│  [Choose SVG File]                                   │
│  [Upload]                                            │
│                                                      │
│  ACTIONS                                             │
│  ┌────────────────────────────────────┐             │
│  │ [PREVIEW] [PLOT] [STOP]  [RESET]   │             │
│  │   (Blue)  (Green) (Red)   (Gray)    │             │
│  └────────────────────────────────────┘             │
│                                                      │
└──────────────────────────────────────────────────────┘
```

---

## Code Complexity Estimate

| Component | Lines | Complexity |
|-----------|-------|-----------|
| Canvas drawing | 100 | Low |
| Shape tools | 80 | Low |
| SVG parser | 60 | Medium |
| G-code generator | 100 | Medium |
| FluidNC API calls | 40 | Low |
| UI logic + event handlers | 80 | Low |
| HTML + CSS | 40 | Low |
| **Total** | ~500 | Low-Medium |

---

## Development Priorities (If Time Limited)

**Phase 1 (Must Have):**
1. ✅ Canvas drawing (lines only)
2. ✅ G-code generation from lines
3. ✅ Send to FluidNC via HTTP POST
4. ✅ Stop button

**Phase 2 (Should Have):**
1. ✅ Additional shapes (rectangle, circle)
2. ✅ SVG import
3. ✅ Text tool
4. ✅ Preview toolpath

**Phase 3 (Nice to Have):**
1. ⚠️ Real-time status via WebSocket
2. ⚠️ Save/load designs locally
3. ⚠️ Estimated plot time
4. ⚠️ Design gallery

---

## Testing Checklist

- [ ] Canvas draws shapes correctly
- [ ] G-code generated matches intended drawing
- [ ] G-code sends to FluidNC without errors
- [ ] Motors move in correct directions
- [ ] Pen (Z-axis) lifts and lowers
- [ ] Emergency stop halts all motors
- [ ] Multiple shapes plot in correct sequence
- [ ] SVG import parses file correctly
- [ ] Text converts to outline paths
- [ ] Works on Chrome and Firefox
- [ ] Works on mobile browsers
- [ ] Status display updates in real-time

---

## Success Criteria

**Working plotter when:**
1. ✅ Student can connect to plotter WiFi
2. ✅ Student can open web interface at `http://plotter.local`
3. ✅ Student can draw a simple shape
4. ✅ Student can click "Plot"
5. ✅ Plotter draws the shape accurately
6. ✅ Process repeats with new designs
7. ✅ All 5 plotters work identically

---

## Deliverables

**Primary:** Single HTML file (`plotter-interface.html`) with:
- Embedded CSS
- Embedded JavaScript
- ~500 lines total

**Secondary (Optional):**
- JavaScript split into separate files (drawing.js, gcode.js, fluidnc-api.js)
- Separate CSS file
- README with deployment instructions

**Bonus (For GitHub):**
- Complete project repo with docs/
- Hardware specifications
- Bill of materials
- Assembly instructions
- Example SVG designs
- GitHub Pages documentation site

---

## Questions to Clarify During Development

1. Should drawing be raster (pixels) or vector (mathematical shapes)?
   - **Answer:** Vector (mathematical) - easier to convert to G-code
   
2. Should users see real-time preview of G-code movement?
   - **Answer:** Optional enhancement after MVP
   
3. How many undo steps? 
   - **Answer:** At least 1 (last action), ideally 5-10
   
4. Should text always be filled outlines or support hollow text?
   - **Answer:** Filled outlines (easier to plot)
   
5. Should plotter auto-return to home after plotting?
   - **Answer:** Yes, always return to (0,0) for safety

---

## Final Notes

- **Keep it simple first:** Get lines working, then add shapes
- **Test on real hardware early:** Don't wait until end
- **Users don't care about code:** They care about drawing → plotting working
- **GPIO buttons can wait:** Web interface is more important
- **Document as you go:** Future you will thank present you
- **Open source means:** Share your work! Submit to GitHub

This is an achievable, impactful project that will give students real-world experience with:
- Web development (HTML/CSS/JavaScript)
- IoT/WiFi connectivity
- CNC/G-code concepts
- Open-source software
- Hardware integration

Good luck! You've got this.
