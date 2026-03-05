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
#endif // BACKLIGHT_CONTROL_H