#include "BacklightControl.h"
#include "Config.h"
#include "DataTypes.h"
#include "Arduino.h"

void setupBacklight() {
  ledcSetup(BACKLIGHT_CHANNEL, BACKLIGHT_FREQ, BACKLIGHT_RESOLUTION);
  ledcAttachPin(BACKLIGHT_PIN, BACKLIGHT_CHANNEL);
  ledcWrite(BACKLIGHT_CHANNEL, BACKLIGHT_BRIGHTNESS);
}

void setBacklightBrightness(uint8_t brightness) {
  ledcWrite(BACKLIGHT_CHANNEL, brightness);
}

void adjustBacklightAutomatically() {
  static uint32_t lastBrightnessCheck = 0;
  
  if (millis() - lastBrightnessCheck > 1000) { // Check every second
    uint8_t newBrightness = BACKLIGHT_BRIGHTNESS;
    
    // Reduce brightness when engine is running (RPM > 500)
    if (rpm > 6000) {
      newBrightness = 150; // Dimmer when driving
    } else {
      newBrightness = 80; // Brighter when idle/parked
    }
    
    setBacklightBrightness(newBrightness);
    lastBrightnessCheck = millis();
  }
}
