#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <Arduino.h>

// Motor pin definitions
// X-Axis
#define X_IN1 13
#define X_IN2 14
#define X_IN3 27
#define X_IN4 26

// Y-Axis
#define Y_IN1 16
#define Y_IN2 17
#define Y_IN3 5
#define Y_IN4 18

// Z-Axis (Pen up/down)
#define Z_IN1 19
#define Z_IN2 21
#define Z_IN3 22
#define Z_IN4 23

// Motor specifications
#define STEPS_PER_REV 4096      // 28BYJ-48 in half-step mode
#define STEPS_PER_MM_X 40       // Steps per mm for X axis (20% more reach than Y)
#define STEPS_PER_MM_Y 40       // Steps per mm for Y axis
#define STEPS_PER_MM_Z 50       // Steps per mm for Z axis (pen) - reduced to 1/4

// Axis inversion (set to -1 to invert, 1 for normal)
#define X_INVERT -1             // Inverted to fix mirror image
#define Y_INVERT 1              // Normal direction

// Half-step sequence for 28BYJ-48
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

// Current step position for each motor
int xCurrentStep = 0;
int yCurrentStep = 0;
int zCurrentStep = 0;

// Current position in mm
float currentX = 0.0;
float currentY = 0.0;
float currentZ = 0.0;

// Step delay (microseconds) - controls speed
int stepDelayUs = 2000;  // 2ms = moderate speed

/**
 * Initialize all motor pins as outputs
 */
void initMotors() {
  // X-Axis
  pinMode(X_IN1, OUTPUT);
  pinMode(X_IN2, OUTPUT);
  pinMode(X_IN3, OUTPUT);
  pinMode(X_IN4, OUTPUT);

  // Y-Axis
  pinMode(Y_IN1, OUTPUT);
  pinMode(Y_IN2, OUTPUT);
  pinMode(Y_IN3, OUTPUT);
  pinMode(Y_IN4, OUTPUT);

  // Z-Axis
  pinMode(Z_IN1, OUTPUT);
  pinMode(Z_IN2, OUTPUT);
  pinMode(Z_IN3, OUTPUT);
  pinMode(Z_IN4, OUTPUT);

  // Set all outputs LOW initially
  digitalWrite(X_IN1, LOW);
  digitalWrite(X_IN2, LOW);
  digitalWrite(X_IN3, LOW);
  digitalWrite(X_IN4, LOW);

  digitalWrite(Y_IN1, LOW);
  digitalWrite(Y_IN2, LOW);
  digitalWrite(Y_IN3, LOW);
  digitalWrite(Y_IN4, LOW);

  digitalWrite(Z_IN1, LOW);
  digitalWrite(Z_IN2, LOW);
  digitalWrite(Z_IN3, LOW);
  digitalWrite(Z_IN4, LOW);

  Serial.println("Motors initialized");
}

/**
 * Set motor coils to specific step in sequence
 */
void setMotorStep(char axis, int step) {
  step = step % 8;  // Ensure step is 0-7

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
 * Step a motor a specific number of steps
 * @param axis 'X', 'Y', or 'Z'
 * @param steps Number of steps (positive or negative)
 */
void stepMotor(char axis, int steps) {
  int* currentStep;

  // Get pointer to current step for this axis
  if (axis == 'X') currentStep = &xCurrentStep;
  else if (axis == 'Y') currentStep = &yCurrentStep;
  else if (axis == 'Z') currentStep = &zCurrentStep;
  else return;

  bool clockwise = steps > 0;
  steps = abs(steps);

  for (int i = 0; i < steps; i++) {
    // Move to next step
    if (clockwise) {
      (*currentStep)++;
      if (*currentStep >= 8) *currentStep = 0;
    } else {
      (*currentStep)--;
      if (*currentStep < 0) *currentStep = 7;
    }

    // Set motor to new step
    setMotorStep(axis, *currentStep);

    // Wait before next step
    delayMicroseconds(stepDelayUs);
  }
}

/**
 * Move to absolute position in mm
 * @param x Target X position
 * @param y Target Y position
 * @param z Target Z position (pen up/down)
 */
void moveTo(float x, float y, float z) {
  // Calculate steps needed for each axis (with inversion)
  int xSteps = (int)((x - currentX) * STEPS_PER_MM_X * X_INVERT);
  int ySteps = (int)((y - currentY) * STEPS_PER_MM_Y * Y_INVERT);
  int zSteps = (int)((z - currentZ) * STEPS_PER_MM_Z);

  // Move Z first (pen up before moving, or down after)
  if (zSteps > 0) {  // Moving pen up
    stepMotor('Z', zSteps);
    delay(100);  // Wait for pen to stabilize
  }

  // Move X and Y simultaneously (simple linear interpolation)
  int maxSteps = max(abs(xSteps), abs(ySteps));

  for (int i = 0; i < maxSteps; i++) {
    // Step X if needed
    if (abs(xSteps) > 0 && i < abs(xSteps)) {
      stepMotor('X', xSteps > 0 ? 1 : -1);
    }

    // Step Y if needed
    if (abs(ySteps) > 0 && i < abs(ySteps)) {
      stepMotor('Y', ySteps > 0 ? 1 : -1);
    }
  }

  // Move Z last if going down
  if (zSteps < 0) {  // Moving pen down
    stepMotor('Z', zSteps);
    delay(100);  // Wait for pen to stabilize
  }

  // Update current position
  currentX = x;
  currentY = y;
  currentZ = z;  // Always update Z to keep tracking in sync
}

/**
 * Home all axes (return to 0,0 with pen up)
 */
void homeMotors() {
  Serial.println("Homing motors...");
  moveTo(0, 0, 5.0);  // Keep pen up at Z=5mm
  Serial.println("Homing complete");
}

/**
 * Pen up (Z = 5mm)
 */
void penUp() {
  moveTo(currentX, currentY, 5.0);
}

/**
 * Pen down (Z = 0mm)
 */
void penDown() {
  moveTo(currentX, currentY, 0.0);
}

/**
 * Stop all motors (turn off coils)
 */
void stopAllMotors() {
  digitalWrite(X_IN1, LOW);
  digitalWrite(X_IN2, LOW);
  digitalWrite(X_IN3, LOW);
  digitalWrite(X_IN4, LOW);

  digitalWrite(Y_IN1, LOW);
  digitalWrite(Y_IN2, LOW);
  digitalWrite(Y_IN3, LOW);
  digitalWrite(Y_IN4, LOW);

  digitalWrite(Z_IN1, LOW);
  digitalWrite(Z_IN2, LOW);
  digitalWrite(Z_IN3, LOW);
  digitalWrite(Z_IN4, LOW);
}

#endif // MOTOR_CONTROL_H
