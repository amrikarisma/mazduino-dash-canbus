#ifndef MAIN_SCREEN_H
#define MAIN_SCREEN_H

#include <TFT_eSPI.h>
#include "DisplayConfig.h"

// External display objects
extern TFT_eSPI display;
extern TFT_eSprite spr;

// Main screen function declarations
void drawConfigurablePanels(bool setup);
void drawModularDataPanel(const DisplayPanel &panel, bool setup);
void drawConfigurableIndicators();
void drawConfigurableData(bool setup);
void drawDataBox(int x, int y, const char *label, const float value, uint16_t labelColor, const float valueToCompare, const int decimal, bool setup);

#endif // MAIN_SCREEN_H