#include "ESPNowHandler.h"
#include "Config.h"
#include "DataTypes.h"
#include <esp_wifi.h>

// Local variables for AC data (not global to avoid conflicts)
ACControllerData receivedACData;
unsigned long lastACDataTime = 0;

void initESPNow() {
  Serial.println("=== ESP-NOW Initialization ===");
  
  // Set WiFi mode if not already set (will be compatible with webserver)
  if (WiFi.getMode() == WIFI_MODE_NULL) {
    WiFi.mode(WIFI_STA);
    Serial.println("WiFi mode set to STA for ESP-NOW");
  } else {
    Serial.printf("WiFi already initialized in mode: %d\n", WiFi.getMode());
  }
  
  Serial.print("ESP32 MAC Address: ");
  Serial.println(WiFi.macAddress());
  Serial.println("Configure ESP32C3 sender with this MAC address");
  Serial.println("Or use broadcast: FF:FF:FF:FF:FF:FF");
  Serial.println("==============================");
  
  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Register callback function for receiving data
  esp_now_register_recv_cb([](const esp_now_recv_info_t *recv_info, const uint8_t *data, int len) {
    const uint8_t *mac_addr = recv_info->src_addr;
    onESPNowDataReceived(mac_addr, data, len);
  });
  
  // Initialize AC data with default values
  receivedACData.temperature = 0.0;
  receivedACData.voltage = 0.0;
  receivedACData.compressorStatus = false;
  receivedACData.wasCompressorCutoff = false;
  strcpy(receivedACData.speedMode, "UNKNOWN");
  receivedACData.uptime = 0;
  receivedACData.runtime = 0.0;
  
  Serial.println("ESP-NOW initialized successfully");
  Serial.println("Ready to receive AC controller data...");
  Serial.println("Waiting for data from ESP32C3...");
}

void reinitESPNowAfterWiFi() {
  Serial.println("=== Re-initializing ESP-NOW after WiFi change ===");
  
  // Deinitialize first
  esp_now_deinit();
  delay(100);
  
  // Re-initialize
  if (esp_now_init() != ESP_OK) {
    Serial.println("ERROR: Failed to re-initialize ESP-NOW");
    return;
  }
  
  // Re-register callback
  esp_now_register_recv_cb([](const esp_now_recv_info_t *recv_info, const uint8_t *data, int len) {
    const uint8_t *mac_addr = recv_info->src_addr;
    onESPNowDataReceived(mac_addr, data, len);
  });
  
  Serial.println("ESP-NOW re-initialized successfully");
  Serial.printf("Ready to receive on MAC: %s\n", WiFi.macAddress().c_str());
  Serial.println("==================================================");
}

void onESPNowDataReceived(const uint8_t *mac, const uint8_t *incomingData, int len) {
  // Verify data length
  if (len != sizeof(ACControllerData)) {
    Serial.printf("AC: Invalid data size received: %d bytes\n", len);
    return;
  }
  
  // Copy received data
  memcpy(&receivedACData, incomingData, sizeof(receivedACData));
  
  // Update global variables
  acDataReceived = true;
  lastACDataTime = millis();
  acLastUpdate = millis();
  
  // Update global AC data variables
  acCurrentTemp = receivedACData.temperature;
  
  // Simple confirmation message
  Serial.printf("AC: Data received - %.1f°C\n", receivedACData.temperature);
}

void printACData() {
  Serial.println("=== AC Controller Data ===");
  Serial.printf("Temperature: %.2f°C\n", receivedACData.temperature);
  Serial.printf("Amplifier Voltage: %.3fV\n", receivedACData.voltage);
  Serial.printf("Compressor: %s\n", receivedACData.compressorStatus ? "ON" : "OFF");
  Serial.printf("Cutoff Flag: %s\n", receivedACData.wasCompressorCutoff ? "YES" : "NO");
  Serial.printf("Speed Mode: %s\n", receivedACData.speedMode);
  Serial.printf("Uptime: %lu seconds (%.1f hours)\n", receivedACData.uptime, receivedACData.uptime / 3600.0);
  Serial.printf("Compressor Runtime: %.1f seconds\n", receivedACData.runtime);
  Serial.println("==========================");
}

bool isACDataValid() {
  if (!acDataReceived) return false;
  
  // Check if data is not too old
  unsigned long currentTime = millis();
  unsigned long dataAge = currentTime - lastACDataTime;
  
  if (dataAge > AC_DATA_TIMEOUT_MS) {
    return false;
  }
  
  // Basic sanity checks
  if (receivedACData.temperature < -50 || receivedACData.temperature > 100) {
    return false;
  }
  
  if (receivedACData.voltage < 0 || receivedACData.voltage > 30) {
    return false;
  }
  
  return true;
}

String getACStatusString() {
  if (!isACDataValid()) {
    return "AC: NO DATA";
  }
  
  if (!receivedACData.compressorStatus) {
    return "AC: OFF (" + String(receivedACData.temperature, 1) + "°C)";
  }
  
  return "AC: " + String(receivedACData.speedMode) + " " + String(receivedACData.temperature, 1) + "°C";
}

void updateACControllerData() {
  // Update global variables with received AC data
  if (acDataReceived && isACDataValid()) {
    acCurrentTemp = receivedACData.temperature;
    acLastUpdate = lastACDataTime;
  }
}