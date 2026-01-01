#include "BacklightControl.h"
#include "Config.h"
#include "DataTypes.h"
#include "Arduino.h"
#include <EEPROM.h>

// EEPROM address for brightness (using address 11 to avoid conflicts)
#define BRIGHTNESS_EEPROM_ADDR 11

void setupBacklight(bool isInitialStartup) {
  // Load brightness from EEPROM
  loadBrightnessFromEEPROM();
  
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
  ledcWrite(BACKLIGHT_CHANNEL, brightness);
  // Save to EEPROM whenever brightness is changed
  saveBrightnessToEEPROM();
}

void saveBrightnessToEEPROM() {
  EEPROM.write(BRIGHTNESS_EEPROM_ADDR, backlightBrightness);
  EEPROM.write(BRIGHTNESS_EEPROM_ADDR + 1, 0xAA); // Initialization flag
  EEPROM.commit();
  Serial.printf("Brightness saved to EEPROM: %d\n", backlightBrightness);
}

void loadBrightnessFromEEPROM() {
  // Read both brightness and initialization flag
  uint8_t savedBrightness = EEPROM.read(BRIGHTNESS_EEPROM_ADDR);
  uint8_t initFlag = EEPROM.read(BRIGHTNESS_EEPROM_ADDR + 1);
  
  // Check if EEPROM has been initialized (flag != 0xAA)
  if (initFlag != 0xAA) {
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
  Serial.printf("Backlight enabled after splash screen with brightness: %d\n", backlightBrightness);
}


