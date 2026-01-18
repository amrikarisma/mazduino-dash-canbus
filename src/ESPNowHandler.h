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

// Constants

#endif