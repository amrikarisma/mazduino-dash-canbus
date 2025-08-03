#include <Arduino.h>
#include <EEPROM.h>
#include <WiFi.h>
#include <WebServer.h>
#include <TFT_eSPI.h>

// Include all our modular headers
#include "Config.h"
#include "DataTypes.h"
#include "BacklightControl.h"
#include "CANHandler.h"
#include "DisplayManager.h"
#include "WebServerHandler.h"
#include "GlobalVariables.h"

// Include legacy headers for compatibility
#include "Comms.h"
#include "text_utils.h"
#include "drawing_utils.h"
#if ENABLE_SIMULATOR
#include "Simulator.h"
#endif

// Web server
WebServer server(80);

// Serial communication handling
void handleSerialCommunication()
{
  static uint32_t lastUpdate = millis();
  // Reduce serial communication frequency from 10ms to 20ms for better performance
  if (millis() - lastUpdate > 20)
  {
    requestData(50);
    lastUpdate = millis();
  }

  isCANMode = false;  // We're in Serial mode when this function is called

  static uint32_t lastRefresh = millis();
  uint32_t elapsed = millis() - lastRefresh;
  refreshRate = (elapsed > 0) ? (1000 / elapsed) : 0;
  lastRefresh = millis();
  
  // Reduce lazy update frequency from 100ms to 200ms for better performance
  if (lastRefresh - lazyUpdateTime > 200 || rpm < 100)
  {
    clt = getByte(7) - 40;
    iat = getByte(6) - 40;
    bat = getByte(9)* 0.1;
  }
  
  rpm = getWord(14);
  mapData = getWord(4) / 10;
  afrConv = getByte(10) / 10;
  tps = getByte(24) / 2;
  adv = (int8_t)getByte(23);
  fp = getByte(103);

  syncStatus = getBit(31, 7);
  ase = getBit(2, 2);
  wue = getBit(2, 3);
  rev = getBit(31, 2);
  launch = getBit(31, 0);
  airCon = getByte(122);
  fan = getBit(106, 3);
  dfco = getBit(1, 4);

  // Debug: Print data values occasionally
  static uint32_t lastDataDebug = 0;
  if (millis() - lastDataDebug > 5000) { // Print every 5 seconds
    Serial.printf("[SERIAL] RPM: %d, MAP: %.1f, TPS: %.1f, CLT: %d, IAT: %d\n", 
                  rpm, mapData, tps, clt, iat);
    lastDataDebug = millis();
  }
}

// Handle all serial commands in a single function to avoid conflicts
void handleSerialCommands()
{
  if (Serial.available() > 0)
  {
    char command = Serial.read();
    
    switch(command)
    {
#if ENABLE_SIMULATOR
      case '0':
        setSimulatorMode(SIMULATOR_MODE_OFF);
        break;
      case '1':
        setSimulatorMode(SIMULATOR_MODE_RPM_SWEEP);
        break;
      case '2':
        setSimulatorMode(SIMULATOR_MODE_ENGINE_IDLE);
        break;
      case '3':
        setSimulatorMode(SIMULATOR_MODE_DRIVING);
        break;
      case '4':
        setSimulatorMode(SIMULATOR_MODE_REDLINE);
        break;
#endif
#if ENABLE_DEBUG_MODE
      case 'd':
      case 'D':
        debugMode = !debugMode;
        Serial.printf("[DEBUG] Debug mode %s\n", debugMode ? "ON" : "OFF");
        Serial.printf("[DEBUG] debugMode variable value: %d\n", debugMode);
        Serial.printf("[DEBUG] CPU Usage: %.1f, FPS: %.1f\n", cpuUsage, fps);
        if (debugMode) {
          Serial.println("=== DEBUG MODE ENABLED ===");
          Serial.println("CPU Usage and FPS will be displayed");
          Serial.println("Serial debug prints enabled");
          Serial.println("Send 'd' again to disable");
          Serial.println("==========================");
        } else {
          Serial.println("=== DEBUG MODE DISABLED ===");
          Serial.println("Debug display cleared");
          Serial.println("Serial debug prints disabled");
          Serial.println("===========================");
        }
        break;
      case 'i':
      case 'I':
        // Show system info
        Serial.println("=== SYSTEM INFO ===");
        Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
        Serial.printf("Min Free Heap: %d bytes\n", ESP.getMinFreeHeap());
        Serial.printf("CPU Freq: %d MHz\n", ESP.getCpuFreqMHz());
        Serial.printf("Flash Size: %d bytes\n", ESP.getFlashChipSize());
        Serial.printf("Chip Model: %s\n", ESP.getChipModel());
        Serial.printf("Chip Revision: %d\n", ESP.getChipRevision());
        Serial.printf("Uptime: %d seconds\n", (millis() - startupTime) / 1000);
        Serial.println("==================");
        break;
#endif
      case 'h':
      case 'H':
        // Show help for all commands
        Serial.println("=== ALL COMMANDS ===");
#if ENABLE_SIMULATOR
        Serial.println("SIMULATOR COMMANDS:");
        Serial.println("0 = OFF (use real data)");
        Serial.println("1 = RPM SWEEP (0-6000 RPM)");
        Serial.println("2 = ENGINE IDLE (800-900 RPM)");
        Serial.println("3 = DRIVING (1500-4000 RPM)");
        Serial.println("4 = REDLINE (5500-6000 RPM)");
#endif
#if ENABLE_DEBUG_MODE
        Serial.println("DEBUG COMMANDS:");
        Serial.println("d = Toggle debug mode");
        Serial.println("i = Show system info");
#endif
        Serial.println("NETWORK COMMANDS:");
        Serial.println("w = Restart WiFi/Web Server");
        Serial.println("h = Show this help");
        Serial.println("===================");
        break;
      case 'w':
      case 'W':
        // Restart WiFi/Web Server
        Serial.println("Restarting WiFi and Web Server...");
        restartWebServer();
        break;
    }
  }
}

// Calculate CPU usage percentage (simplified approach)
void updateCpuUsage()
{
  static uint32_t lastMeasureTime = 0;
  static uint32_t loopCount = 0;
  
  uint32_t currentTime = millis();
  loopCount++;
  
  if (lastMeasureTime == 0) {
    lastMeasureTime = currentTime;
    cpuUsage = 0.0;
    return;
  }
  
  if (currentTime - lastMeasureTime >= 1000) { // Update every second
    // Simple approximation: more loops per second = higher CPU usage
    // This is a rough estimate for demonstration
    float loopsPerSecond = (float)loopCount / ((float)(currentTime - lastMeasureTime) / 1000.0);
    cpuUsage = min(loopsPerSecond / 10.0, 100.0); // Scale to reasonable range
    
    lastMeasureTime = currentTime;
    loopCount = 0;
    
    // Debug print CPU calculation
    if (debugMode) {
      Serial.printf("[DEBUG] CPU calc - Loops/sec: %.1f, Usage: %.1f%%\n", 
                    loopsPerSecond, cpuUsage);
    }
  }
}

// Calculate FPS (frames per second)
void updateFPS()
{
  frameCount++;
  uint32_t currentTime = millis();
  
  if (lastFpsUpdate == 0) {
    lastFpsUpdate = currentTime;
    fps = 0.0;
    return;
  }
  
  if (currentTime - lastFpsUpdate >= 1000) { // Update every second
    float timeDelta = (float)(currentTime - lastFpsUpdate) / 1000.0;
    fps = (float)frameCount / timeDelta;
    frameCount = 0;
    lastFpsUpdate = currentTime;
    
    // Debug print FPS calculation
    if (debugMode) {
      Serial.printf("[DEBUG] FPS calc - Frames: %d, Delta: %.2fs, FPS: %.1f\n", 
                    frameCount, timeDelta, fps);
    }
  }
}

#if ENABLE_DEBUG_MODE
// Print debug information
void printDebugInfo()
{
  if (!debugMode) return;
  
  static uint32_t lastDebugPrint = 0;
  uint32_t currentTime = millis();
  
  // Increase debug print interval from 2s to 5s to reduce serial overhead
  if (currentTime - lastDebugPrint >= 5000) { // Print every 5 seconds
    Serial.println("=== DEBUG INFO ===");
    Serial.printf("CPU Usage: %.1f%%\n", cpuUsage);
    Serial.printf("FPS: %.1f\n", fps);
    Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("Min Free Heap: %d bytes\n", ESP.getMinFreeHeap());
    Serial.printf("RPM: %d\n", rpm);
    Serial.printf("Loop Time: %dms\n", millis() - loopStartTime);
    Serial.printf("Uptime: %ds\n", (currentTime - startupTime) / 1000);
    Serial.printf("WiFi Active: %s\n", wifiActive ? "true" : "false");
    Serial.printf("Client Connected: %s\n", clientConnected ? "true" : "false");
    Serial.println("==================");
    lastDebugPrint = currentTime;
  }
}
#endif

void setup()
{
  EEPROM.begin(EEPROM_SIZE);
  
  // Initialize backlight control
  setupBacklight();
  
  // Initialize display
  setupDisplay();
  drawSplashScreenWithImage();
  display.fillScreen(TFT_BLACK);
  
  Serial.begin(UART_BAUD);
  commMode = EEPROM.read(1);
  
  // If EEPROM is uninitialized (0xFF), set default to CAN mode
  if (commMode == 255) {
    commMode = COMM_CAN;
    EEPROM.write(1, commMode);
    EEPROM.commit();
  }
  
  Serial.println("=== MAZDUINO DASHBOARD STARTING ===");
  Serial.printf("Communication mode: %s\n", (commMode == COMM_CAN) ? "CAN" : "Serial");
  
  // Synchronize isCANMode with commMode
  isCANMode = (commMode == COMM_CAN);
  
  if (commMode == COMM_CAN)
  {
    // Initialize CAN communication
    setupCAN();
    
    // Create CAN task
    xTaskCreatePinnedToCore(canTask, "CAN Task", 4096, NULL, 1, NULL, 0);
    
    Serial.println("CAN mode aktif.");
  }
  else
  {
    Serial1.begin(UART_BAUD, SERIAL_8N1, RXD, TXD);
    Serial.println("Serial mode aktif.");
  }

  // Initialize web server setup (will start after 15 seconds)
  setupWebServer();

#if ENABLE_SIMULATOR
  // Initialize simulator
  initializeSimulator();
  delay(1000); // Give time for simulator messages to appear
#endif

#if ENABLE_DEBUG_MODE
  // Initialize debug mode
  debugMode = false;
  cpuUsage = 0.0;
  fps = 0.0;
  frameCount = 0;
  lastFpsUpdate = 0;  // Reset to 0 for proper initialization
  lastCpuMeasure = 0; // Reset to 0 for proper initialization
  
  Serial.println("=== DEBUG MODE AVAILABLE ===");
  Serial.println("Send 'd' to toggle debug mode");
  Serial.println("Send 'i' for system info");
  Serial.println("Debug shows CPU usage & FPS");
  Serial.printf("Initial debug values - CPU: %.1f%%, FPS: %.1f\n", cpuUsage, fps);
  Serial.println("============================");
#endif

  EEPROM.write(0, 1);
  delay(500);
  startUpDisplay();
  startupTime = millis();
  lazyUpdateTime = startupTime;
  lastClientCheckTimeout = startupTime;
}

void loop()
{
#if ENABLE_DEBUG_MODE
  loopStartTime = millis(); // Start timing for CPU usage
#endif

  // Start web server after 15 seconds to avoid high startup power consumption
  static bool webServerStarted = false;
  if (!webServerStarted && (millis() - startupTime >= 15000)) {
    startWebServer();
    webServerStarted = true;
  }

  // Handle all serial commands (simulator and debug) in one place
  handleSerialCommands();

#if ENABLE_SIMULATOR
  // Update simulator data if enabled - this overrides real data
  updateSimulatorData();
  
  // Reduce debug print frequency for simulator from 5s to 10s
  static uint32_t lastDebugPrint = 0;
  if (millis() - lastDebugPrint > 10000) {
    Serial.printf("[DEBUG] Simulator running, current RPM: %d, mode: %d, commMode: %d\n", rpm, getSimulatorMode(), commMode);
    lastDebugPrint = millis();
  }
#endif

#if ENABLE_DEBUG_MODE
  // Update debug metrics
  updateCpuUsage();
  updateFPS();
  
  // Print debug info if enabled
  printDebugInfo();
#endif
  
  // Handle real data communication only if simulator is OFF
#if ENABLE_SIMULATOR
  if (getSimulatorMode() == SIMULATOR_MODE_OFF) {
#endif
    // Handle serial communication if not in CAN mode
    if (commMode != COMM_CAN)
    {
      handleSerialCommunication();
    }
#if ENABLE_SIMULATOR
  }
#endif

  // Update backlight brightness
  adjustBacklightAutomatically();

  // Update display
  drawData();

  // Handle web server clients with power-saving logic
  // Reduce web server check frequency from every loop to every 10ms
  static uint32_t lastWebServerCheck = 0;
  if (millis() - lastWebServerCheck >= 10) {
    handleWebServerClients();
    lastWebServerCheck = millis();
  }
  
  // Add small yield to prevent watchdog issues and improve multitasking
  yield();
}
