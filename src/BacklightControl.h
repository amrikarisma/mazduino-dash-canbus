#ifndef BACKLIGHT_CONTROL_H
#define BACKLIGHT_CONTROL_H

#include <stdint.h>

// Function declarations
void setupBacklight(bool isInitialStartup = false);
void setBacklightBrightness(uint8_t brightness);
void saveBrightnessToEEPROM();
void loadBrightnessFromEEPROM();
void enableBacklightAfterSplash();
void enableBacklightWithFadeIn();

// Brightness adjustment helpers
void increaseBrightness(uint8_t amount);
void decreaseBrightness(uint8_t amount);
#endif // BACKLIGHT_CONTROL_H