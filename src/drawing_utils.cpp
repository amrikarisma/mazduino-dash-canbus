#include "drawing_utils.h"
#include <TFT_eSPI.h>

// External display objects
extern TFT_eSPI display;

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

void drawRPMBarBlocks(int rpm, int maxRPM) {
  int startX = 120;     // Starting X position
  int startY[30] = {80, 75, 70, 65, 60, 57, 54, 51, 48, 46, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45};
  int blockWidth = 6; // Width of each block
  int blockHeight = 70; // Height of each block
  int spacing = 2;     // Spacing between blocks
  int numBlocks = 30;  // Total number of blocks

  // Calculate number of filled blocks based on RPM value
  int filledBlocks = map(rpm, 0, maxRPM, 0, numBlocks);

  // Clear the entire bar area first
  display.fillRect(startX - 5, 40, (blockWidth + spacing) * numBlocks + 10, blockHeight + 50, TFT_BLACK);

  for (int i = 0; i < numBlocks; i++) {
    int blockX = startX + i * (blockWidth + spacing);
    int blockY = startY[i];
    uint16_t color;
    
    if (i < filledBlocks) {
      // Block is filled
      if (i < 15) {
        color = TFT_GREEN;   // Green for normal RPM
      } else if (i < 25) {
        color = TFT_ORANGE;  // Orange for high RPM
      } else {
        color = TFT_RED;     // Red for danger zone
      }
    } else {
      color = TFT_DARKGREY; // Unfilled block color
    }
    
    display.fillRect(blockX, blockY, blockWidth, blockHeight, color);
  }
}
