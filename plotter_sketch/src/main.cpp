/**
 * PlotterBot - WiFi Pen Plotter
 *
 * Custom ESP32 sketch with web interface for pen plotting
 * Hardware: ESP32 + 28BYJ-48 motors + ULN2003 drivers
 *
 * Features:
 * - WiFi AP mode (SSID: PlotterBot, Password: plot2025)
 * - Web-based drawing interface
 * - G-code parser for motor control
 * - Direct motor control (no external firmware needed)
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

// Include our custom modules
#include "motor_control.h"
#include "gcode_parser.h"
#include "web_interface.h"

// WiFi Access Point credentials
const char* AP_SSID = "PlotterBot";
const char* AP_PASSWORD = "plot2025";

// Web server on port 80
WebServer server(80);

// Plotter state
String plotterState = "idle";  // idle, plotting, homing
int totalLines = 0;

/**
 * Serve the main HTML interface
 */
void handleRoot() {
  server.send(200, "text/html", HTML_PAGE);
}

/**
 * Handle G-code POST requests
 */
void handleGCode() {
  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
    return;
  }

  String gcode = server.arg("plain");

  if (gcode.length() == 0) {
    server.send(400, "text/plain", "No G-code provided");
    return;
  }

  Serial.println("\n=== Received G-code ===");
  Serial.println(gcode);
  Serial.println("=======================\n");

  plotterState = "plotting";

  // Execute the G-code
  int linesExecuted = executeGCodeBlock(gcode);
  totalLines += linesExecuted;

  plotterState = "idle";

  // Send response
  String response = "Executed " + String(linesExecuted) + " lines successfully";
  server.send(200, "text/plain", response);

  Serial.println("\n=== G-code Complete ===");
  Serial.print("Lines executed: ");
  Serial.println(linesExecuted);
  Serial.println("=======================\n");
}

/**
 * Handle status requests
 */
void handleStatus() {
  String json = "{";
  json += "\"state\":\"" + plotterState + "\",";
  json += "\"x\":" + String(currentX, 2) + ",";
  json += "\"y\":" + String(currentY, 2) + ",";
  json += "\"z\":" + String(currentZ, 2) + ",";
  json += "\"totalLines\":" + String(totalLines);
  json += "}";

  server.send(200, "application/json", json);
}

/**
 * Handle 404 errors
 */
void handleNotFound() {
  server.send(404, "text/plain", "404: Not Found");
}

/**
 * Setup function - runs once at startup
 */
void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n\n");
  Serial.println("========================================");
  Serial.println("      PlotterBot - WiFi Pen Plotter    ");
  Serial.println("========================================");
  Serial.println();

  // Initialize motors
  Serial.println("[1/3] Initializing motors...");
  initMotors();
  Serial.println("      ✓ Motors ready");
  Serial.println();

  // Set up WiFi Access Point
  Serial.println("[2/3] Starting WiFi Access Point...");
  Serial.print("      SSID: ");
  Serial.println(AP_SSID);
  Serial.print("      Password: ");
  Serial.println(AP_PASSWORD);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("      ✓ AP started at: ");
  Serial.println(IP);
  Serial.println();

  // Set up web server routes
  Serial.println("[3/3] Starting web server...");
  server.on("/", handleRoot);
  server.on("/gcode", handleGCode);
  server.on("/status", handleStatus);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("      ✓ Web server started on port 80");
  Serial.println();

  Serial.println("========================================");
  Serial.println("         PLOTTER BOT READY! ");
  Serial.println("========================================");
  Serial.println();
  Serial.println("To use:");
  Serial.println("1. Connect to WiFi network: " + String(AP_SSID));
  Serial.println("2. Open browser to: http://" + IP.toString());
  Serial.println("3. Start drawing and plotting!");
  Serial.println();
  Serial.println("========================================");
  Serial.println();
}

/**
 * Main loop - runs continuously
 */
void loop() {
  // Handle web server requests
  server.handleClient();

  // Small delay to prevent watchdog timer issues
  delay(10);
}
