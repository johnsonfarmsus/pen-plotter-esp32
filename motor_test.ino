/*
 * ESP32 Motor Test Sketch - X-Axis Only
 *
 * Hardware: 28BYJ-48 stepper motor + ULN2003 driver
 * Purpose: Verify wiring before deploying FluidNC
 *
 * X-Axis Pin Assignment:
 *   GPIO 13 -> ULN2003 IN1
 *   GPIO 14 -> ULN2003 IN2
 *   GPIO 27 -> ULN2003 IN3
 *   GPIO 26 -> ULN2003 IN4
 *
 * Expected Behavior:
 *   - Motor rotates clockwise 1 full revolution (slow)
 *   - Pauses 2 seconds
 *   - Motor rotates counter-clockwise 1 full revolution
 *   - Repeats continuously
 *
 * If motor doesn't move:
 *   1. Check all 4 GPIO connections
 *   2. Verify 5V power to ULN2003 board
 *   3. Verify GND connection to ULN2003 board
 *   4. Check motor is plugged into ULN2003
 */

// Pin definitions for X-axis motor
#define IN1 13
#define IN2 14
#define IN3 27
#define IN4 26

// Half-step sequence for 28BYJ-48 motor (8 steps per cycle)
// This gives smoother motion and higher resolution
const int halfStepSequence[8][4] = {
  {1, 0, 0, 0},  // Step 0
  {1, 1, 0, 0},  // Step 1
  {0, 1, 0, 0},  // Step 2
  {0, 1, 1, 0},  // Step 3
  {0, 0, 1, 0},  // Step 4
  {0, 0, 1, 1},  // Step 5
  {0, 0, 0, 1},  // Step 6
  {1, 0, 0, 1}   // Step 7
};

// Full-step sequence (alternative, less smooth but simpler)
// Uncomment this and comment out half-step if you want to try full-stepping
/*
const int fullStepSequence[4][4] = {
  {1, 0, 0, 0},
  {0, 1, 0, 0},
  {0, 0, 1, 0},
  {0, 0, 0, 1}
};
*/

// Motor parameters
const int stepsPerRevolution = 4096;  // 28BYJ-48 in half-step mode
const int stepDelayMs = 2;            // Delay between steps (2ms = moderate speed)

int currentStep = 0;

void setup() {
  // Initialize serial for debugging
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n=================================");
  Serial.println("ESP32 Motor Test - X-Axis Only");
  Serial.println("=================================");
  Serial.println("Hardware: 28BYJ-48 + ULN2003");
  Serial.println("Pin assignments:");
  Serial.printf("  IN1 = GPIO %d\n", IN1);
  Serial.printf("  IN2 = GPIO %d\n", IN2);
  Serial.printf("  IN3 = GPIO %d\n", IN3);
  Serial.printf("  IN4 = GPIO %d\n", IN4);
  Serial.println("=================================\n");

  // Configure GPIO pins as outputs
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Initialize all pins LOW
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  Serial.println("Pins configured. Starting motor test in 2 seconds...\n");
  delay(2000);
}

void loop() {
  // Rotate clockwise (forward) one full revolution
  Serial.println("Rotating CLOCKWISE (1 revolution)...");
  rotateMotor(stepsPerRevolution, true);

  // Pause
  Serial.println("Pausing...\n");
  stopMotor();
  delay(2000);

  // Rotate counter-clockwise (backward) one full revolution
  Serial.println("Rotating COUNTER-CLOCKWISE (1 revolution)...");
  rotateMotor(stepsPerRevolution, false);

  // Pause
  Serial.println("Pausing...\n");
  stopMotor();
  delay(2000);
}

/**
 * Rotate motor by specified number of steps
 * @param steps Number of steps to rotate
 * @param clockwise Direction (true = CW, false = CCW)
 */
void rotateMotor(int steps, bool clockwise) {
  for (int i = 0; i < steps; i++) {
    // Move to next step
    if (clockwise) {
      currentStep++;
      if (currentStep >= 8) currentStep = 0;
    } else {
      currentStep--;
      if (currentStep < 0) currentStep = 7;
    }

    // Set motor coils according to sequence
    setMotorStep(currentStep);

    // Wait before next step
    delay(stepDelayMs);

    // Print progress every 512 steps (1/8 revolution)
    if (i % 512 == 0 && i > 0) {
      Serial.printf("  Progress: %d/%d steps\n", i, steps);
    }
  }

  Serial.println("  Complete!");
}

/**
 * Set motor coils to a specific step in the sequence
 * @param step Step number (0-7 for half-step)
 */
void setMotorStep(int step) {
  digitalWrite(IN1, halfStepSequence[step][0]);
  digitalWrite(IN2, halfStepSequence[step][1]);
  digitalWrite(IN3, halfStepSequence[step][2]);
  digitalWrite(IN4, halfStepSequence[step][3]);
}

/**
 * Turn off all motor coils (reduces power consumption and heat)
 */
void stopMotor() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

/*
 * TROUBLESHOOTING:
 *
 * Motor doesn't move at all:
 *   - Check 5V power to ULN2003 board
 *   - Check GND connection to ULN2003 board
 *   - Verify all 4 GPIO wires are connected
 *   - Make sure motor white JST connector is plugged in
 *
 * Motor vibrates but doesn't rotate:
 *   - One or more GPIO pins may be disconnected
 *   - Try swapping IN3 and IN4 wires
 *   - Check that all pins are securely connected
 *
 * Motor rotates wrong direction:
 *   - This is normal - we can fix in software
 *   - Just note the direction for FluidNC config
 *
 * Motor is very hot:
 *   - Normal for 28BYJ-48 during continuous operation
 *   - If too hot to touch, check GND connection
 *   - Add stopMotor() calls during pauses to reduce heat
 *
 * Motor stutters or skips steps:
 *   - Increase stepDelayMs (try 3 or 4)
 *   - Check power supply is at least 5V 1A
 *   - Verify GND is properly connected
 */
