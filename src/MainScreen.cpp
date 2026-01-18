#include "MainScreen.h"
#include "Config.h"
#include "DataTypes.h"
#include "DisplayConfig.h"
#include "drawing_utils.h"
#include "SplashScreen.h"
#include "ESPNowHandler.h"
#include "Roboto16.h"
#include "Roboto21.h"
#include "RobotoBold32.h"
#include "Roboto48.h"
#include <EEPROM.h>
#if ENABLE_SIMULATOR
#include "Simulator.h"
#endif

TFT_eSPI display = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&display);

void drawConfigurablePanels(bool setup) {
  // Draw each enabled panel
  for (int i = 0; i < currentDisplayConfig.activePanelCount; i++) {
    DisplayPanel &panel = currentDisplayConfig.panels[i];
    if (panel.enabled && panel.position < 8) {
      drawModularDataPanel(panel, setup);
    }
  }
}

void drawModularDataPanel(const DisplayPanel &panel, bool setup) {
  // New panel positions to avoid RPM bar collision
  // RPM bar occupies roughly Y=40 to Y=150, so panels moved to avoid collision
  // Each panel is 80px tall, fitting 2 main panels per column + 1 bottom panel each
  int panelPositions[8][2] = {
    {240, 160},   // Position 0: Left-Top (AFR) - Y=160 to Y=240
    {80, 220},   // Position 1: Left-Middle (TPS) - Y=245 to Y=325
    {160, 220},   // Position 2: Left-Bottom (IAT) - Y=280 to Y=360, will be clipped but user requested
    {320, 160}, // Position 3: Right-Top (MAP) - Y=160 to Y=240
    {400, 160}, // Position 4: Right-Middle (ADV) - Y=245 to Y=325
    {400, 220}, // Position 5: Right-Bottom (FP) - Y=280 to Y=360, will be clipped but user requested
    {240, 220}, // Position 6: Center-Left (Coolant) - moved to center to avoid overlap
    {320, 220}  // Position 7: Center-Right (Voltage) - moved to center to avoid overlap
  };
  
  if (panel.position >= 8) return;
  
  int x = panelPositions[panel.position][0];
  int y = panelPositions[panel.position][1];
  
  // Get current value
  float currentValue = getDataValue(panel.dataSource);
  
  // Get color based on data source and value
  uint16_t color = getDataSourceColor(panel.dataSource, currentValue);
  
  // Use existing drawDataBox function with enhanced parameters
  static float lastValues[8] = {-999, -999, -999, -999, -999, -999, -999, -999};
  
  if (setup || lastValues[panel.position] != currentValue) {
    drawDataBox(x, y, panel.label, currentValue, color, lastValues[panel.position], panel.decimals, setup);
    lastValues[panel.position] = currentValue;
  }
}

void drawConfigurableIndicators() {
  // Draw indicators based on configuration
  // Position indicators in the middle area between left/right columns
  // IAT and FP panels are at Y=280-360, center panels at Y=285-365
  // Place indicators between the main columns at Y=330
  int indicatorX = 5;  // Positioned between left and right columns
  int indicatorY = 290;  // Positioned in the middle area
  int indicatorWidth = 60; // Made slightly smaller to fit better
  
  // Pack enabled indicators without gaps
  int currentPosition = 0;
  for (int i = 0; i < currentDisplayConfig.activeIndicatorCount; i++) {
    IndicatorConfig &indicator = currentDisplayConfig.indicators[i];
    if (indicator.enabled && indicator.position < 8) {
      bool state = getIndicatorValue(indicator.indicator);
      // Use currentPosition instead of indicator.position to pack without gaps
      drawSmallButton(indicatorX + (indicatorWidth * currentPosition), indicatorY, indicator.label, state);
      currentPosition++; // Increment position for next enabled indicator
    }
  }
}

// New function to replace itemDraw with configurable panels
void drawConfigurableData(bool setup) {
  // Draw RPM and VSS with reduced frequency update (only when changed or setup)
  static uint32_t lastRpmUpdate = 0;
  static unsigned int lastVss = 999; // Different initial value to force first update
  if (lastRpm != rpm || lastVss != vss || setup || (millis() - lastRpmUpdate > 100)) {
    drawRPMBarBlocks(rpm); // Use default maxRPM from config
    
    // Draw Gear Position in top left corner
    static int lastGear = -1;
    // Use global currentGear from RusEFI CAN data if available, otherwise calculate
    int displayGear = currentGear; // Global currentGear from RusEFI
    if (currentGear == 0) {
      // Fallback to simple gear calculation if no CAN data
      displayGear = (vss > 0 && rpm > 800) ? ((rpm / 1000) + (vss / 40)) % 6 + 1 : 0;
    }
    if (setup || lastGear != displayGear) {
      spr.createSprite(60, 60);
      spr.fillSprite(TFT_BLACK);
      spr.setTextDatum(MC_DATUM);
      spr.loadFont(Roboto48);
      spr.setTextColor(TFT_CYAN, TFT_BLACK, true);
      if (displayGear == 0) {
        spr.drawString("N", 30, 30);
      } else {
        spr.drawNumber(displayGear, 30, 30);
      }
      spr.pushSprite(10, 10);
      spr.deleteSprite();
      
      // Draw GEAR label below gear position
      spr.createSprite(60, 20);
      spr.fillSprite(TFT_BLACK);
      spr.setTextDatum(MC_DATUM);
      spr.loadFont(AA_FONT_SMALL);
      spr.setTextColor(TFT_WHITE, TFT_BLACK, true);
      spr.drawString("GEAR", 30, 10);
      spr.pushSprite(10, 65);
      spr.deleteSprite();
      
      lastGear = displayGear;
    }
    
    // Draw RPM value with proper positioning
    spr.createSprite(80, 40);
    spr.fillSprite(TFT_BLACK);
    spr.setTextDatum(MC_DATUM);
    spr.loadFont(RobotoBold32);
    spr.setTextColor(TFT_WHITE, TFT_BLACK, true);
    spr.drawNumber(rpm, 40, 20); // Centered in sprite
    spr.pushSprite(390, 65);
    spr.deleteSprite();
    
    // Draw VSS value with proper positioning
    spr.createSprite(100, 50);
    spr.fillSprite(TFT_BLACK);
    spr.setTextDatum(MC_DATUM);
    spr.loadFont(Roboto48);
    spr.setTextColor(TFT_WHITE, TFT_BLACK, true);
    spr.drawNumber(vss, 50, 25); // Centered in sprite
    spr.pushSprite(190, 115);
    spr.deleteSprite();
    
    // Draw "kph" unit label aligned with VSS
    spr.createSprite(50, 25);
    spr.fillSprite(TFT_BLACK);
    spr.setTextDatum(MC_DATUM);
    spr.loadFont(AA_FONT_SMALL);
    spr.setTextColor(TFT_WHITE, TFT_BLACK, true);
    spr.drawString("kph", 25, 12); // Centered in sprite
    spr.pushSprite(280, 120);
    spr.deleteSprite();
    
    lastRpm = rpm;
    lastVss = vss;
    lastRpmUpdate = millis();
  }
  
  // Draw configurable panels with reduced frequency
  static uint32_t lastPanelUpdate = 0;
  if (setup || (millis() - lastPanelUpdate > 50)) { // Update panels every 50ms max
    drawConfigurablePanels(setup);
    lastPanelUpdate = millis();
  }
  
  // Draw configurable indicators with reduced frequency
  static uint32_t lastIndicatorUpdate = 0;
  if (setup || (millis() - lastIndicatorUpdate > 100)) { // Update indicators every 100ms max
    drawConfigurableIndicators();
    lastIndicatorUpdate = millis();
  }
}

void drawDataBox(int x, int y, const char *label, const float value, uint16_t labelColor, const float valueToCompare, const int decimal, bool setup) {
  const int BOX_WIDTH = 70;
  const int BOX_HEIGHT = 70;
  const int LABEL_HEIGHT = BOX_HEIGHT / 2;

  if (setup) {
    // Clear the entire data box area first only during setup
    // display.fillRect(x, y, BOX_WIDTH, BOX_HEIGHT, TFT_BLACK);

    spr.loadFont(AA_FONT_SMALL);
    spr.createSprite(BOX_WIDTH, LABEL_HEIGHT);
    // spr.fillSprite(TFT_BLACK);  // Clear sprite background
    spr.setTextColor(labelColor, TFT_BLACK, true);
    spr.setTextDatum(TC_DATUM);
    spr.drawString(label, 40, 5);
    if (label == "AFR") {
      spr.pushSprite(x - 10, y);
    } else {
      spr.pushSprite(x, y);
    }
    spr.deleteSprite();
  }
  
  if (setup || valueToCompare != value) {
    spr.loadFont(AA_FONT_LARGE);
    spr.createSprite(BOX_WIDTH, LABEL_HEIGHT);
    spr.fillSprite(TFT_ORANGE);  // Clear sprite background
    spr.setTextDatum(TC_DATUM);
    spr.setTextColor(labelColor, TFT_ORANGE, true);
    if (decimal > 0) {
      spr.drawFloat(value, decimal, 40, 5);
    } else {
      spr.drawNumber(value, 40, 5);
    }
    spr.pushSprite(x, y + LABEL_HEIGHT - 15);
    spr.deleteSprite();
  }
}

