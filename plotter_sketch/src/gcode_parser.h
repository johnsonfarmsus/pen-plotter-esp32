#ifndef GCODE_PARSER_H
#define GCODE_PARSER_H

#include <Arduino.h>
#include "motor_control.h"

// Movement mode
bool absoluteMode = true;  // true = G90 (absolute), false = G91 (relative)

// Last commanded position (for relative moves)
float lastX = 0.0;
float lastY = 0.0;
float lastZ = 0.0;

/**
 * Parse a single G-code line and execute it
 * @param line G-code command string
 * @return true if command executed successfully
 */
bool executeGCode(String line) {
  line.trim();
  line.toUpperCase();

  // Skip empty lines and comments
  if (line.length() == 0 || line.startsWith(";") || line.startsWith("(")) {
    return true;
  }

  Serial.print("Executing: ");
  Serial.println(line);

  // Extract command (G0, G1, M3, etc.)
  String command = "";
  int spaceIndex = line.indexOf(' ');
  if (spaceIndex > 0) {
    command = line.substring(0, spaceIndex);
  } else {
    command = line;
  }

  // Parse parameters
  float x = currentX;
  float y = currentY;
  float z = currentZ;
  bool hasX = false, hasY = false, hasZ = false;

  // Extract X parameter
  int xIndex = line.indexOf('X');
  if (xIndex >= 0) {
    float xValue = line.substring(xIndex + 1).toFloat();
    x = absoluteMode ? xValue : currentX + xValue;
    hasX = true;
  }

  // Extract Y parameter
  int yIndex = line.indexOf('Y');
  if (yIndex >= 0) {
    float yValue = line.substring(yIndex + 1).toFloat();
    y = absoluteMode ? yValue : currentY + yValue;
    hasY = true;
  }

  // Extract Z parameter
  int zIndex = line.indexOf('Z');
  if (zIndex >= 0) {
    float zValue = line.substring(zIndex + 1).toFloat();
    z = absoluteMode ? zValue : currentZ + zValue;
    hasZ = true;
  }

  // Execute command
  if (command == "G0" || command == "G00") {
    // Rapid positioning (pen up)
    Serial.println("  -> Rapid move (pen up)");
    penUp();
    if (hasX || hasY) {
      moveTo(x, y, 5.0);  // Keep pen up
    }
    return true;
  }
  else if (command == "G1" || command == "G01") {
    // Linear move (pen down)
    Serial.println("  -> Linear move (pen down)");
    penDown();
    if (hasX || hasY || hasZ) {
      moveTo(x, y, z);
    }
    return true;
  }
  else if (command == "G28") {
    // Home all axes
    Serial.println("  -> Homing");
    homeMotors();
    return true;
  }
  else if (command == "G90") {
    // Absolute positioning mode
    Serial.println("  -> Absolute mode");
    absoluteMode = true;
    return true;
  }
  else if (command == "G91") {
    // Relative positioning mode
    Serial.println("  -> Relative mode");
    absoluteMode = false;
    return true;
  }
  else if (command == "M3") {
    // Pen down
    Serial.println("  -> Pen down");
    penDown();
    return true;
  }
  else if (command == "M5") {
    // Pen up
    Serial.println("  -> Pen up");
    penUp();
    return true;
  }
  else if (command == "M114") {
    // Get current position
    Serial.print("  -> Position: X=");
    Serial.print(currentX);
    Serial.print(" Y=");
    Serial.print(currentY);
    Serial.print(" Z=");
    Serial.println(currentZ);
    return true;
  }
  else if (command == "M18" || command == "M84") {
    // Disable motors
    Serial.println("  -> Motors disabled");
    stopAllMotors();
    return true;
  }
  else {
    Serial.print("  -> Unknown command: ");
    Serial.println(command);
    return false;
  }
}

/**
 * Parse and execute multiple lines of G-code
 * @param gcode Multi-line G-code string
 * @return Number of lines executed successfully
 */
int executeGCodeBlock(String gcode) {
  int linesExecuted = 0;
  int startIndex = 0;

  while (startIndex < gcode.length()) {
    int endIndex = gcode.indexOf('\n', startIndex);
    if (endIndex == -1) {
      endIndex = gcode.length();
    }

    String line = gcode.substring(startIndex, endIndex);

    if (executeGCode(line)) {
      linesExecuted++;
    }

    startIndex = endIndex + 1;
  }

  return linesExecuted;
}

#endif // GCODE_PARSER_H
