#ifndef DRAWING_UTILS_H
#define DRAWING_UTILS_H

#include "text_utils.h"

void drawCenteredTextSmall(int x, int y, int w, int h, const char* text, int textSize, uint16_t color) {
  display.setTextDatum(MC_DATUM);
  display.setTextColor(color, TFT_BLACK);
  display.drawString(text, x, y); 
}

void drawSmallButton(int x, int y, const char* label, bool value) {
  const int BTN_WIDTH = 50;
  const int BTN_HEIGHT = 30;
  uint16_t activeColor = (label == "REV" || label == "LCH" ) ? TFT_RED : TFT_GREEN;
  uint16_t fillColor = value ? activeColor : TFT_WHITE; 
  display.drawRoundRect(x, y, BTN_WIDTH, BTN_HEIGHT, 5, fillColor);
  drawCenteredTextSmall(x+BTN_WIDTH/2, y+BTN_HEIGHT/2, BTN_WIDTH, BTN_HEIGHT, label, 1, fillColor);
}

void drawRPMBarBlocks(int rpm, int maxRPM = 6000) {
  int startX = 120;     // Starting X position
  int startY[30] = {80, 75, 70, 65, 60, 57, 54, 51, 48, 46, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45};
  int blockWidth = 6; // Width of each block
  int blockHeight = 70; // Height of each block
  int spacing = 2;     // Spacing between blocks
  int numBlocks = 30;  // Total number of blocks

  // Calculate number of filled blocks based on RPM value
  int filledBlocks = map(rpm, 0, maxRPM, 0, numBlocks);
  
  // Draw the blocks one by one
  for (int i = 0; i < numBlocks; i++) {
    int x = startX + i * (blockWidth + spacing);
    int y = startY[i];
    uint16_t color = TFT_BLACK; // Default color for empty block
    
    // Assign color based on block's position and RPM value
    if (i < filledBlocks) {
      if (i < numBlocks * 0.6) {
        color = TFT_GREEN;
      } else if (i < numBlocks * 0.8) {
        color = TFT_YELLOW;
      } else {
        color = TFT_RED;
      }
    }

    display.fillRect(x, y, blockWidth, blockHeight, color);
  }
}

#endif