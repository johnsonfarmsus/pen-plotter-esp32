#include <Arduino.h>

/*
 * ESP32 Motor Movement Test - X-Axis
 * Tests actual motor rotation using half-step sequence
 * Hardware: 28BYJ-48 stepper motor + ULN2003 driver
 */

// X-axis motor pins
#define X_IN1 13
#define X_IN2 14
#define X_IN3 27
#define X_IN4 26

// Y-axis motor pins
#define Y_IN1 16
#define Y_IN2 17
#define Y_IN3 5
#define Y_IN4 18

// Z-axis motor pins
#define Z_IN1 19
#define Z_IN2 21
#define Z_IN3 22
#define Z_IN4 23

// Visual indicator LEDs
#define ONBOARD_LED 2      // Will blink during movement
#define LED_CW 2           // Onboard LED for clockwise
#define LED_CCW 15         // External LED for counter-clockwise (GPIO 15)

// Half-step sequence for 28BYJ-48 motor (8 steps per cycle)
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

// Motor parameters
const int stepsPerRevolution = 4096;  // 28BYJ-48 in half-step mode
const int stepDelayMs = 2;            // Delay between steps (2ms = moderate speed)

int currentStep = 0;

// Function declarations
void rotateMotor(int steps, bool clockwise, char axis);
void setMotorStep(int step, char axis);
void stopMotor(char axis);
void stopAllMotors();

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n===========================================");
  Serial.println("ESP32 Motor Test - ALL THREE AXES");
  Serial.println("===========================================");
  Serial.println("Hardware: 28BYJ-48 + ULN2003");
  Serial.println("Pin assignments:");
  Serial.printf("  X-Axis: GPIO %d, %d, %d, %d\n", X_IN1, X_IN2, X_IN3, X_IN4);
  Serial.printf("  Y-Axis: GPIO %d, %d, %d, %d\n", Y_IN1, Y_IN2, Y_IN3, Y_IN4);
  Serial.printf("  Z-Axis: GPIO %d, %d, %d, %d\n", Z_IN1, Z_IN2, Z_IN3, Z_IN4);
  Serial.println("===========================================\n");

  // Configure X-axis GPIO pins as outputs
  pinMode(X_IN1, OUTPUT);
  pinMode(X_IN2, OUTPUT);
  pinMode(X_IN3, OUTPUT);
  pinMode(X_IN4, OUTPUT);

  // Configure Y-axis GPIO pins as outputs
  pinMode(Y_IN1, OUTPUT);
  pinMode(Y_IN2, OUTPUT);
  pinMode(Y_IN3, OUTPUT);
  pinMode(Y_IN4, OUTPUT);

  // Configure Z-axis GPIO pins as outputs
  pinMode(Z_IN1, OUTPUT);
  pinMode(Z_IN2, OUTPUT);
  pinMode(Z_IN3, OUTPUT);
  pinMode(Z_IN4, OUTPUT);

  // Configure LED pins
  pinMode(LED_CW, OUTPUT);
  pinMode(LED_CCW, OUTPUT);

  // Initialize all pins LOW
  stopAllMotors();
  digitalWrite(LED_CW, LOW);
  digitalWrite(LED_CCW, LOW);

  Serial.println("Pins configured. Motor test starts in 2 seconds...\n");
  Serial.println("LED Indicators:");
  Serial.printf("  Onboard LED (GPIO %d) = CLOCKWISE movement\n", LED_CW);
  Serial.printf("  External LED (GPIO %d) = COUNTER-CLOCKWISE movement\n", LED_CCW);
  Serial.println();
  delay(2000);
}

void loop() {
  // Test X-Axis
  Serial.println(">>> X-AXIS CLOCKWISE - 1 FULL ROTATION (Onboard LED ON) <<<");
  digitalWrite(LED_CW, HIGH);
  digitalWrite(LED_CCW, LOW);
  rotateMotor(stepsPerRevolution, true, 'X');
  digitalWrite(LED_CW, LOW);
  Serial.println("  Pause...\n");
  stopMotor('X');
  delay(2000);

  Serial.println(">>> X-AXIS COUNTER-CLOCKWISE - 1 FULL ROTATION <<<");
  digitalWrite(LED_CCW, HIGH);
  digitalWrite(LED_CW, LOW);
  rotateMotor(stepsPerRevolution, false, 'X');
  digitalWrite(LED_CCW, LOW);
  Serial.println("  Pause...\n");
  stopMotor('X');
  delay(2000);

  // Test Y-Axis
  Serial.println(">>> Y-AXIS CLOCKWISE - 1 FULL ROTATION (Onboard LED ON) <<<");
  digitalWrite(LED_CW, HIGH);
  digitalWrite(LED_CCW, LOW);
  rotateMotor(stepsPerRevolution, true, 'Y');
  digitalWrite(LED_CW, LOW);
  Serial.println("  Pause...\n");
  stopMotor('Y');
  delay(2000);

  Serial.println(">>> Y-AXIS COUNTER-CLOCKWISE - 1 FULL ROTATION <<<");
  digitalWrite(LED_CCW, HIGH);
  digitalWrite(LED_CW, LOW);
  rotateMotor(stepsPerRevolution, false, 'Y');
  digitalWrite(LED_CCW, LOW);
  Serial.println("  Pause...\n");
  stopMotor('Y');
  delay(2000);

  // Test Z-Axis
  Serial.println(">>> Z-AXIS CLOCKWISE - 1 FULL ROTATION (Onboard LED ON) <<<");
  digitalWrite(LED_CW, HIGH);
  digitalWrite(LED_CCW, LOW);
  rotateMotor(stepsPerRevolution, true, 'Z');
  digitalWrite(LED_CW, LOW);
  Serial.println("  Pause...\n");
  stopMotor('Z');
  delay(2000);

  Serial.println(">>> Z-AXIS COUNTER-CLOCKWISE - 1 FULL ROTATION <<<");
  digitalWrite(LED_CCW, HIGH);
  digitalWrite(LED_CW, LOW);
  rotateMotor(stepsPerRevolution, false, 'Z');
  digitalWrite(LED_CCW, LOW);
  Serial.println("  Pause...\n");
  stopMotor('Z');
  delay(2000);

  Serial.println("========== ALL AXES TESTED - CYCLE COMPLETE ==========\n\n");
  delay(3000);
}

/**
 * Rotate motor by specified number of steps
 * @param steps Number of steps to rotate
 * @param clockwise Direction (true = CW, false = CCW)
 * @param axis Axis to move ('X', 'Y', or 'Z')
 */
void rotateMotor(int steps, bool clockwise, char axis) {
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
    setMotorStep(currentStep, axis);

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
 * @param axis Axis to control ('X', 'Y', or 'Z')
 */
void setMotorStep(int step, char axis) {
  if (axis == 'X') {
    digitalWrite(X_IN1, halfStepSequence[step][0]);
    digitalWrite(X_IN2, halfStepSequence[step][1]);
    digitalWrite(X_IN3, halfStepSequence[step][2]);
    digitalWrite(X_IN4, halfStepSequence[step][3]);
  } else if (axis == 'Y') {
    digitalWrite(Y_IN1, halfStepSequence[step][0]);
    digitalWrite(Y_IN2, halfStepSequence[step][1]);
    digitalWrite(Y_IN3, halfStepSequence[step][2]);
    digitalWrite(Y_IN4, halfStepSequence[step][3]);
  } else if (axis == 'Z') {
    digitalWrite(Z_IN1, halfStepSequence[step][0]);
    digitalWrite(Z_IN2, halfStepSequence[step][1]);
    digitalWrite(Z_IN3, halfStepSequence[step][2]);
    digitalWrite(Z_IN4, halfStepSequence[step][3]);
  }
}

/**
 * Turn off motor coils for specified axis
 * @param axis Axis to stop ('X', 'Y', or 'Z')
 */
void stopMotor(char axis) {
  if (axis == 'X') {
    digitalWrite(X_IN1, LOW);
    digitalWrite(X_IN2, LOW);
    digitalWrite(X_IN3, LOW);
    digitalWrite(X_IN4, LOW);
  } else if (axis == 'Y') {
    digitalWrite(Y_IN1, LOW);
    digitalWrite(Y_IN2, LOW);
    digitalWrite(Y_IN3, LOW);
    digitalWrite(Y_IN4, LOW);
  } else if (axis == 'Z') {
    digitalWrite(Z_IN1, LOW);
    digitalWrite(Z_IN2, LOW);
    digitalWrite(Z_IN3, LOW);
    digitalWrite(Z_IN4, LOW);
  }
}

/**
 * Turn off all motor coils (all axes)
 */
void stopAllMotors() {
  stopMotor('X');
  stopMotor('Y');
  stopMotor('Z');
}
