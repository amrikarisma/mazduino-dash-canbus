#ifndef ESPNOWHANDLER_H
#define ESPNOWHANDLER_H

#include <esp_now.h>
#include <WiFi.h>
#include <Arduino.h>

// Structure to receive AC data from ESP32C3 - must match sender exactly
struct ACControllerData {
  float temperature;
  float voltage;
  bool compressorStatus;
  bool wasCompressorCutoff;
  char speedMode[32];
  unsigned long uptime;
  float runtime;
};

// Structure to send AC settings to ESP32C3
struct ACSettingsData {
  float cutoffTemperature;
  uint8_t mode; // 0=cool, 1=heat, 2=auto
  bool enabled;
  uint32_t timestamp;
};

// External variables
extern ACControllerData receivedACData;
extern bool acDataReceived;
extern unsigned long lastACDataTime;

// Function declarations
void initESPNow();
void reinitESPNowAfterWiFi();
void onESPNowDataReceived(const uint8_t *mac, const uint8_t *incomingData, int len);
void printACData();
bool isACDataValid();
String getACStatusString();
void updateACControllerData();

// Send functions
void sendACCutoffToESP32C3(float cutoffTemp);
void sendACSettingsToESP32C3(const ACSettingsData& settings);

// Constants

#endif