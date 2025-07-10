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

// Web server
WebServer server(80);

// Serial communication handling
void handleSerialCommunication()
{
  static uint32_t lastUpdate = millis();
  if (millis() - lastUpdate > 10)
  {
    requestData(50);
    lastUpdate = millis();
  }

  static uint32_t lastRefresh = millis();
  uint32_t elapsed = millis() - lastRefresh;
  refreshRate = (elapsed > 0) ? (1000 / elapsed) : 0;
  lastRefresh = millis();
  
  if (lastRefresh - lazyUpdateTime > 100 || rpm < 100)
  {
    clt = getByte(7) - 40;
    iat = getByte(6) - 40;
    bat = getByte(9);
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
}

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
  EEPROM.write(1, 0);
  EEPROM.commit();
  commMode = EEPROM.read(1);
  
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

  // Initialize web server (currently commented out in original)
  // setupWebServer();

  EEPROM.write(0, 1);
  delay(500);
  startUpDisplay();
  startupTime = millis();
  lazyUpdateTime = startupTime;
  lastClientCheckTimeout = startupTime;
}

void loop()
{
  // Handle serial communication if not in CAN mode
  if (commMode != COMM_CAN)
  {
    handleSerialCommunication();
  }

  // Update backlight brightness
  adjustBacklightAutomatically();

  // Update display
  drawData();

  // Handle web server (currently commented out in original)
  // handleWebServerClients();
}
