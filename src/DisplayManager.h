#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <TFT_eSPI.h>
#include "DisplayConfig.h"

// External display objects
extern TFT_eSPI display;
extern TFT_eSprite spr;

// Function declarations
void setupDisplay();
void drawSplashScreenWithImage();
void startUpDisplay();
void drawDataBox(int x, int y, const char *label, const float value, uint16_t labelColor, const float valueToCompare, const int decimal, bool setup);
void drawData();
void drawConfigurableData(bool setup);
void drawConfigurablePanels(bool setup);
void drawConfigurableIndicators();
void drawModularDataPanel(const DisplayPanel &panel, bool setup);

#endif // DISPLAY_MANAGER_H
