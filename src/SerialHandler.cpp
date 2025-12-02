#include "SerialHandler.h"
#include "Config.h"
#include "DataTypes.h"
#include "Comms.h"
#include "GlobalVariables.h"
#include "Arduino.h"

void setupSerial() {
  Serial1.begin(UART_BAUD, SERIAL_8N1, RXD, TXD);
  Serial.printf("Serial mode setup complete. Pins: RX=%d, TX=%d, Baud=%d\n", RXD, TXD, UART_BAUD);
}

void serialTask(void *pvParameters) {
  Serial.println("Serial communication task started on core 0");
  
  while (1) {
    handleSerialCommunication();
    vTaskDelay(1); // Allow other tasks to run
  }
}

void handleSerialCommunication() {
  static uint32_t lastUpdate = millis();
  static uint32_t lastRefresh = millis();
  
  // Request data every 15ms for faster response (was 20ms)
  if (millis() - lastUpdate > 15) {
    requestData(30); // Reduced timeout from 50ms to 30ms
    lastUpdate = millis();
  }

  isCANMode = false;  // We're in Serial mode when this function is called

  uint32_t currentTime = millis();
  uint32_t elapsed = currentTime - lastRefresh;
  refreshRate = (elapsed > 0) ? (1000 / elapsed) : 0;
  lastRefresh = currentTime;
  
  // Update temperature and voltage data every 150ms (was 200ms)
  if (currentTime - lazyUpdateTime > 150 || rpm < 100) {
    clt = getByte(7) - 40;
    iat = getByte(6) - 40;
    bat = getByte(9) * 0.1;
    lazyUpdateTime = currentTime;
  }
  
  // Read primary engine data
  rpm = getWord(14);
  mapData = getWord(4);
  afrConv = getByte(10) * 0.1;
  tps = getByte(24) / 2.0;
  adv = (int8_t)getByte(23);
  fp = getByte(103);
  vss = getWord(100);
  
  // Read status bits
  syncStatus = getBit(31, 7);
  ase = getBit(2, 2);
  wue = getBit(2, 3);
  rev = getBit(31, 2);
  launch = getBit(31, 0);
  airCon = getBit(122, 1);
  fan = getBit(106, 3);
  dfco = getBit(1, 4);

  // Debug: Print data values occasionally
  static uint32_t lastDataDebug = 0;
  if (currentTime - lastDataDebug > 5000) { // Print every 5 seconds
    Serial.printf("[SERIAL] RPM: %d, MAP: %.1f, TPS: %.1f, CLT: %d, IAT: %d\n", 
                  rpm, mapData, tps, clt, iat);
    Serial.printf("[SERIAL] AFR: %.2f, FP: %d, ADV: %d, RefreshRate: %dHz\n", 
                  afrConv, fp, adv, refreshRate);
    lastDataDebug = currentTime;
  }
}
