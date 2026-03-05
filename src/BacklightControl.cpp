#include "BacklightControl.h"
#include "Config.h"
#include "DataTypes.h"
#include "Arduino.h"
#include <EEPROM.h>

void setupBacklight(bool isInitialStartup) {
  // Load brightness from EEPROM
  loadBrightnessFromEEPROM();
  
  // Setup LEDC using modern ESP32 Arduino core API
  ledcAttach(BACKLIGHT_PIN, BACKLIGHT_FREQ, BACKLIGHT_RESOLUTION);
  
  // If initial startup, set LED to OFF initially
  // so splash screen can fade from completely dark
  if (isInitialStartup) {
    ledcWrite(BACKLIGHT_PIN, 0); // Start completely off for smooth fade-in
    Serial.printf("Backlight initialized OFF for smooth splash fade-in (saved brightness: %d)\n", backlightBrightness);
  } else {
    ledcWrite(BACKLIGHT_PIN, backlightBrightness);
    Serial.printf("Backlight initialized with brightness: %d\n", backlightBrightness);
  }
}

void setBacklightBrightness(uint8_t brightness) {
  backlightBrightness = brightness;
  ledcWrite(BACKLIGHT_PIN, brightness);
  // Save to EEPROM whenever brightness is changed
  saveBrightnessToEEPROM();
}

void saveBrightnessToEEPROM() {
  EEPROM.write(EEPROM_BRIGHTNESS_ADDR, backlightBrightness);
  EEPROM.write(EEPROM_BRIGHTNESS_FLAG_ADDR, EEPROM_BRIGHTNESS_FLAG); // Initialization flag
  EEPROM.commit();
  Serial.printf("Brightness saved to EEPROM: %d\n", backlightBrightness);
}

void loadBrightnessFromEEPROM() {
  // Read both brightness and initialization flag
  uint8_t savedBrightness = EEPROM.read(EEPROM_BRIGHTNESS_ADDR);
  uint8_t initFlag = EEPROM.read(EEPROM_BRIGHTNESS_FLAG_ADDR);
  
  // Check if EEPROM has been initialized (flag != EEPROM_BRIGHTNESS_FLAG)
  if (initFlag != EEPROM_BRIGHTNESS_FLAG) {
    // EEPROM not initialized, use default brightness value of 150
    backlightBrightness = 150;
    saveBrightnessToEEPROM(); // Save default to EEPROM
    Serial.println("Using default brightness 150 (EEPROM uninitialized)");
  } else {
    // Use saved brightness (all values 0-255 are valid)
    backlightBrightness = savedBrightness;
    Serial.printf("Brightness loaded from EEPROM: %d\n", backlightBrightness);
  }
}

void enableBacklightAfterSplash() {
  // Turn on backlight with saved brightness after splash screen
  ledcWrite(BACKLIGHT_PIN, backlightBrightness);
}

void enableBacklightWithFadeIn() {
  // Gradually fade in backlight from 0 to saved brightness
  const uint8_t FADE_STEPS = 50;  // Number of steps for fade-in
  const uint16_t FADE_DELAY = 20; // Delay in ms between each step
  
  for (uint8_t step = 0; step <= FADE_STEPS; step++) {
    // Calculate brightness for current step
    uint8_t currentBrightness = (backlightBrightness * step) / FADE_STEPS;
    ledcWrite(BACKLIGHT_PIN, currentBrightness);
    delay(FADE_DELAY);
  }
  
  // Ensure we reach the exact target brightness
  ledcWrite(BACKLIGHT_PIN, backlightBrightness);
}