# Hardware Setup Guide

Complete wiring and assembly instructions for the WiFi Pen Plotter.

## Components List

### Electronics
- **1x ESP32 DevKit** (38-pin recommended, 30-pin works too)
- **3x 28BYJ-48 Stepper Motors** (5V unipolar)
- **3x ULN2003 Motor Driver Boards** (usually included with motors)
- **1x 5V Power Supply** (2A minimum for reliable operation)
- **Jumper Wires** (Male-to-Female Dupont wires)
- **Breadboard** (optional, for organizing connections)

### Mechanical (user-provided)
- Frame/chassis for X-Y movement
- Linear rails or rods
- Belts/pulleys or lead screws
- Pen holder mechanism
- Z-axis lift mechanism

## Wiring Diagram

### ESP32 GPIO to ULN2003 Connections

```
ESP32 GPIO     ULN2003 X-Axis    ULN2003 Y-Axis    ULN2003 Z-Axis
----------     --------------    --------------    --------------
GPIO 13   -->  IN1
GPIO 14   -->  IN2
GPIO 27   -->  IN3
GPIO 26   -->  IN4

GPIO 16   -->                    IN1
GPIO 17   -->                    IN2
GPIO 5    -->                    IN3
GPIO 18   -->                    IN4

GPIO 19   -->                                      IN1
GPIO 21   -->                                      IN2
GPIO 22   -->                                      IN3
GPIO 23   -->                                      IN4

GND       -->  GND (connect all GND pins together - CRITICAL!)
```

### Power Distribution

⚠️ **CRITICAL: Do NOT power motors from ESP32 USB!**

```
5V Power Supply (2A minimum)
│
├── Positive (+)
│   ├── ULN2003 X-Axis (+5-12V terminal)
│   ├── ULN2003 Y-Axis (+5-12V terminal)
│   ├── ULN2003 Z-Axis (+5-12V terminal)
│   └── ESP32 VIN (optional if not using USB)
│
└── Negative (-)
    ├── ULN2003 X-Axis (GND terminal)
    ├── ULN2003 Y-Axis (GND terminal)
    ├── ULN2003 Z-Axis (GND terminal)
    └── ESP32 GND (MUST SHARE COMMON GROUND!)
```

**Common Ground Rule:** ESP32 and all motor drivers MUST share the same ground connection!

## Step-by-Step Wiring

### 1. Plug Motors into Drivers

Each 28BYJ-48 motor has a keyed 5-pin connector:
- Plug X-axis motor into X-axis ULN2003 board
- Plug Y-axis motor into Y-axis ULN2003 board
- Plug Z-axis motor into Z-axis ULN2003 board

Label the drivers to avoid confusion!

### 2. Connect ESP32 to ULN2003 Boards

Using Dupont jumper wires:

**X-Axis (horizontal movement):**
- GPIO 13 → X-axis ULN2003 IN1
- GPIO 14 → X-axis ULN2003 IN2
- GPIO 27 → X-axis ULN2003 IN3
- GPIO 26 → X-axis ULN2003 IN4

**Y-Axis (forward/back movement):**
- GPIO 16 → Y-axis ULN2003 IN1
- GPIO 17 → Y-axis ULN2003 IN2
- GPIO 5 → Y-axis ULN2003 IN3
- GPIO 18 → Y-axis ULN2003 IN4

**Z-Axis (pen up/down):**
- GPIO 19 → Z-axis ULN2003 IN1
- GPIO 21 → Z-axis ULN2003 IN2
- GPIO 22 → Z-axis ULN2003 IN3
- GPIO 23 → Z-axis ULN2003 IN4

### 3. Connect Power

**Ground first (safety!):**
1. Connect all three ULN2003 GND terminals together
2. Connect ESP32 GND to the common ground

**Then 5V power:**
1. Connect 5V+ from power supply to all three ULN2003 boards
2. Power ESP32 via USB (for programming) OR share the 5V supply

### 4. Verify Before Power-On

✅ Checklist:
- [ ] All grounds connected together
- [ ] Motor power from external 5V supply (NOT USB)
- [ ] GPIO pin numbers match the firmware
- [ ] No shorts between power and ground
- [ ] Motors plugged into correct axis drivers

## Testing

### 1. Upload Firmware

```bash
cd plotter_sketch
pio run --target upload
```

Watch serial monitor for "PlotterBot" WiFi network confirmation.

### 2. Connect to WiFi

- Network: `PlotterBot`
- Password: `plot2025`
- Open browser: `http://192.168.4.1`

### 3. Test Motors

Use the web interface control buttons:
- **Home** - All motors should move to (0, 0) position
- **Pen Up** - Z motor lifts
- **Pen Down** - Z motor lowers

### 4. Test Plotting

1. Draw a small test shape
2. Click "Send to Plotter"
3. Motors should execute the drawing!

## Calibration

Edit `plotter_sketch/src/motor_control.h`:

```cpp
// Motor calibration (adjust for your mechanical setup)
#define STEPS_PER_MM_X 52    // X-axis steps per millimeter
#define STEPS_PER_MM_Y 40    // Y-axis steps per millimeter
#define STEPS_PER_MM_Z 50    // Z-axis (pen lift) steps per mm

// Axis inversion (flip direction if needed)
#define X_INVERT -1          // -1 = inverted, 1 = normal
#define Y_INVERT 1           // -1 = inverted, 1 = normal
```

**To calibrate:**
1. Draw a 100×100 pixel square in the web interface
2. Measure the actual plotted dimensions
3. Calculate new values:
   ```
   NEW_STEPS = OLD_STEPS × (DESIRED_MM / ACTUAL_MM)
   ```
4. Update motor_control.h and re-upload

**Example:**
- Drew 100×100px square
- Plotted 80mm × 100mm instead of desired 100mm × 100mm
- New X value: `52 × (100/80) = 65`
- Keep Y at 40 (already correct)

## Pin Reference

| Axis | Motor | ESP32 GPIO Pins | ULN2003 Pins |
|------|-------|----------------|--------------|
| X    | Left/Right | 13, 14, 27, 26 | IN1-IN4 |
| Y    | Forward/Back | 16, 17, 5, 18 | IN1-IN4 |
| Z    | Pen Up/Down | 19, 21, 22, 23 | IN1-IN4 |

## Troubleshooting

**Motors don't move:**
- Check 5V power supply is ON and connected
- Verify GPIO wiring matches pin assignments
- Check motor connectors are fully seated
- Look for errors in serial monitor

**Motors move wrong direction:**
- Edit motor_control.h: Change `X_INVERT` or `Y_INVERT`
- Re-upload firmware

**Plotter draws wrong size:**
- Adjust `STEPS_PER_MM_X` and `STEPS_PER_MM_Y`
- Higher values = smaller movement
- Lower values = larger movement

**Pen doesn't touch paper:**
- Adjust `STEPS_PER_MM_Z` value
- Check Z-axis mechanical setup
- Verify pen holder height

**WiFi doesn't appear:**
- Wait 15-20 seconds after power-on
- Check serial monitor for "AP started" message
- Verify ESP32 is powered and programmed

**Drawing is mirrored:**
- Flip `X_INVERT` from -1 to 1 (or vice versa)
- Or flip `Y_INVERT` if mirrored vertically

## Safety Notes

⚠️ **Important:**
1. Never power motors from ESP32 USB port
2. Use 2A minimum power supply
3. Common ground is essential
4. Check polarity before connecting power
5. Don't run motors without mechanical load

## Next Steps

After successful hardware assembly:
1. Upload firmware ([see README](README.md))
2. Connect to PlotterBot WiFi
3. Test motors via web interface
4. Calibrate movement
5. Start plotting!

For software setup and usage, see main [README.md](README.md).
