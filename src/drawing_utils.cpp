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
  static int lastFilledBlocks = -1; // Remember last state to avoid unnecessary redraws
  static uint32_t lastUpdate = 0;
  static bool firstRun = true;
  
  // Limit update frequency to reduce flicker
  if (millis() - lastUpdate < 50) { // Update max every 50ms
    return;
  }
  lastUpdate = millis();
  
  int startX = 120;     // Starting X position
  int startY[30] = {80, 75, 70, 65, 60, 57, 54, 51, 48, 46, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45};
  int blockWidth = 6;   // Width of each block
  int blockHeight = 70; // Height of each block
  int spacing = 2;      // Spacing between blocks
  int numBlocks = 30;   // Total number of blocks

  // Calculate number of filled blocks based on RPM value
  int filledBlocks = 0;
  if (rpm > 0) {
    filledBlocks = map(rpm, 0, maxRPM, 1, numBlocks); // Start from 1, not 0
    filledBlocks = constrain(filledBlocks, 1, numBlocks); // Ensure 1-30 range when RPM > 0
  }
  // When RPM = 0, filledBlocks = 0 (no blocks lit)
  
  // Only redraw if the number of filled blocks has changed OR first run
  if (filledBlocks == lastFilledBlocks && !firstRun) {
    return;
  }

  // If first run, clear entire area and initialize all blocks as empty
  if (firstRun) {
    display.fillRect(startX - 5, 40, (blockWidth + spacing) * numBlocks + 10, blockHeight + 50, TFT_BLACK);
    // Draw all blocks as empty initially
    for (int i = 0; i < numBlocks; i++) {
      int blockX = startX + i * (blockWidth + spacing);
      int blockY = startY[i];
      display.fillRect(blockX, blockY, blockWidth, blockHeight, TFT_DARKGREY);
    }
    firstRun = false;
  }

  // Force update ALL blocks to ensure correct state
  for (int i = 0; i < numBlocks; i++) {
    int blockX = startX + i * (blockWidth + spacing);
    int blockY = startY[i];
    uint16_t color;
    
    // Determine color based on current state - VERY EXPLICIT
    if (rpm == 0) {
      // Force all blocks to be empty when RPM is 0
      color = TFT_DARKGREY;
    } else if (i < filledBlocks) {
      // Block should be filled when RPM > 0
      if (i < 15) {
        color = TFT_GREEN;   // Green for normal RPM
      } else if (i < 25) {
        color = TFT_ORANGE;  // Orange for high RPM
      } else {
        color = TFT_RED;     // Red for danger zone
      }
    } else {
      // Block should be empty
      color = TFT_DARKGREY;
    }
    
    // Only redraw if the color should change
    if (lastFilledBlocks == -1 || 
        (i < filledBlocks) != (i < lastFilledBlocks) ||
        (rpm == 0 && lastFilledBlocks > 0)) {
      display.fillRect(blockX, blockY, blockWidth, blockHeight, color);
    }
  }
  
  lastFilledBlocks = filledBlocks;
}
