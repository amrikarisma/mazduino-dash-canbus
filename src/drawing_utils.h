#ifndef DRAWING_UTILS_H
#define DRAWING_UTILS_H

#include <stdint.h>

void drawCenteredTextSmall(int x, int y, int w, int h, const char* text, int textSize, uint16_t color);
void drawSmallButton(int x, int y, const char* label, bool value);
void drawRPMBarBlocks(int rpm, int maxRPM = 6000);

#endif // DRAWING_UTILS_H
