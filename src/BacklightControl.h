#ifndef BACKLIGHT_CONTROL_H
#define BACKLIGHT_CONTROL_H

#include <stdint.h>

// Function declarations
void setupBacklight();
void setBacklightBrightness(uint8_t brightness);
void adjustBacklightAutomatically();

#endif // BACKLIGHT_CONTROL_H
