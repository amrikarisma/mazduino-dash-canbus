#include "DisplayManager.h"
#include "Config.h"
#include "DataTypes.h"
#include "DisplayConfig.h"
#include "drawing_utils.h"
#include "NotoSansBold15.h"
#include "NotoSansBold36.h"
#include <EEPROM.h>
#if ENABLE_SIMULATOR
#include "Simulator.h"
#endif

TFT_eSPI display = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&display);

void setupDisplay() {
  display.init();
  display.setRotation(3);
  // Initialize display configuration
  initializeDisplayConfig();
}

void drawSplashScreenWithImage() {
  display.fillScreen(TFT_BLACK);
  display.loadFont(AA_FONT_LARGE);
  display.setTextColor(TFT_WHITE, TFT_BLACK);

  display.setTextDatum(TC_DATUM);
  int centerX = display.width() / 2;
  int centerY = (display.height() / 2) - 35;

  display.drawString("MAZDUINO Display", centerX, centerY);
  display.loadFont(AA_FONT_SMALL);
  display.drawString("Firmware version: " + String(version), centerX, centerY + 50);
  display.drawString("www.mazduino.com", centerX, 300);

  delay(5000);
}

void drawConfigurablePanels(bool setup) {
  // New panel positions to avoid RPM bar collision
  // Layout: 3 left column, 3 right column
  // RPM bar occupies roughly Y=40 to Y=150, so panels moved to avoid collision
  // Each panel is 80px tall, fitting 2 main panels per column + 1 bottom panel each
  int panelPositions[8][2] = {
    {5, 10},   // Position 0: Left-Top (AFR) - Y=160 to Y=240
    {5, 100},   // Position 1: Left-Middle (TPS) - Y=245 to Y=325
    {5, 190},   // Position 2: Left-Bottom (IAT) - Y=280 to Y=360, will be clipped but user requested
    {365, 10}, // Position 3: Right-Top (MAP) - Y=160 to Y=240
    {365, 100}, // Position 4: Right-Middle (ADV) - Y=245 to Y=325
    {365, 190}, // Position 5: Right-Bottom (FP) - Y=280 to Y=360, will be clipped but user requested
    {120, 190}, // Position 6: Center-Left (Coolant) - moved to center to avoid overlap
    {240, 190}  // Position 7: Center-Right (Voltage) - moved to center to avoid overlap
  };
  
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
    {5, 10},   // Position 0: Left-Top (AFR) - Y=160 to Y=240
    {5, 100},   // Position 1: Left-Middle (TPS) - Y=245 to Y=325
    {5, 190},   // Position 2: Left-Bottom (IAT) - Y=280 to Y=360, will be clipped but user requested
    {365, 10}, // Position 3: Right-Top (MAP) - Y=160 to Y=240
    {365, 100}, // Position 4: Right-Middle (ADV) - Y=245 to Y=325
    {365, 190}, // Position 5: Right-Bottom (FP) - Y=280 to Y=360, will be clipped but user requested
    {120, 190}, // Position 6: Center-Left (Coolant) - moved to center to avoid overlap
    {240, 190}  // Position 7: Center-Right (Voltage) - moved to center to avoid overlap
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
  int indicatorX = 10;  // Positioned between left and right columns
  int indicatorY = 285;  // Positioned in the middle area
  int indicatorWidth = 60; // Made slightly smaller to fit better
  
  for (int i = 0; i < currentDisplayConfig.activeIndicatorCount; i++) {
    IndicatorConfig &indicator = currentDisplayConfig.indicators[i];
    if (indicator.enabled && indicator.position < 8) {
      bool state = getIndicatorValue(indicator.indicator);
      drawSmallButton(indicatorX + (indicatorWidth * indicator.position), indicatorY, indicator.label, state);
    }
  }
}

// New function to replace itemDraw with configurable panels
void drawConfigurableData(bool setup) {
  // Draw RPM (always shown)
  if (lastRpm != rpm || setup) {
    drawRPMBarBlocks(rpm);
    spr.loadFont(AA_FONT_LARGE);
    spr.createSprite(100, 50);
    spr_width = spr.textWidth("7777");
    spr.setTextColor(TFT_WHITE, TFT_BLACK, true);
    spr.setTextDatum(TR_DATUM);
    spr.drawNumber(rpm, 100, 5);
    spr.pushSprite(190, 140);
    spr.deleteSprite();
    lastRpm = rpm;
  }
  
  // Draw configurable panels
  drawConfigurablePanels(setup);
  
  // Draw configurable indicators
  drawConfigurableIndicators();
}

void startUpDisplay() {
  display.fillScreen(TFT_BLACK);
  display.loadFont(AA_FONT_SMALL);
  spr.setColorDepth(16);
  display.setTextColor(TFT_WHITE, TFT_BLACK);
  display.drawString("RPM", 190, 120);
  
  // Use configurable display system
  drawConfigurableData(true);
  
  spr.loadFont(AA_FONT_LARGE);
  for (int i = 6000; i >= 0; i -= 250) {
    drawRPMBarBlocks(i);
    spr.createSprite(100, 50);
    spr_width = spr.textWidth("7777");
    spr.setTextColor(TFT_WHITE, TFT_BLACK, true);
    spr.setTextDatum(TR_DATUM);
    spr.drawNumber(i, 100, 5);
    spr.pushSprite(190, 140);
    spr.deleteSprite();
  }
}

void drawDataBox(int x, int y, const char *label, const float value, uint16_t labelColor, const float valueToCompare, const int decimal, bool setup) {
  const int BOX_WIDTH = 100;
  const int BOX_HEIGHT = 80;
  const int LABEL_HEIGHT = BOX_HEIGHT / 2;

  if (setup) {
    // Clear the entire data box area first only during setup
    display.fillRect(x, y, BOX_WIDTH, BOX_HEIGHT, TFT_BLACK);
    
    spr.loadFont(AA_FONT_SMALL);
    spr.createSprite(BOX_WIDTH, LABEL_HEIGHT);
    spr.fillSprite(TFT_BLACK);  // Clear sprite background
    spr.setTextColor(labelColor, TFT_BLACK, true);
    spr.setTextDatum(TC_DATUM);
    spr.drawString(label, 50, 5);
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
    spr.fillSprite(TFT_BLACK);  // Clear sprite background
    spr.setTextDatum(TC_DATUM);
    spr_width = spr.textWidth("333");
    spr.setTextColor(labelColor, TFT_BLACK, true);
    if (decimal > 0) {
      spr.drawFloat(value, decimal, 50, 5);
    } else {
      spr.drawNumber(value, 50, 5);
    }
    spr.pushSprite(x, y + LABEL_HEIGHT - 15);
    spr.deleteSprite();
  }
}

void drawData() {
  // Use configurable display system
  drawConfigurableData(false);
  
#if ENABLE_SIMULATOR
  // Draw simulator indicator if simulator is active
  static uint8_t lastSimMode = SIMULATOR_MODE_OFF;
  uint8_t currentSimMode = getSimulatorMode();
  
  // Only redraw if simulator mode has changed
  if (currentSimMode != lastSimMode) {
    if (currentSimMode != SIMULATOR_MODE_OFF) {
      // Clear the SIM area first
      display.fillRect(display.width() - 30, 5, 25, 15, TFT_BLACK);
      
      // Draw SIM indicator
      display.loadFont(AA_FONT_SMALL);
      display.setTextColor(TFT_YELLOW, TFT_BLACK);
      display.setTextDatum(TR_DATUM);
      display.drawString("SIM", display.width() - 5, 5);
    } else {
      // Clear the SIM indicator when simulator is turned off
      display.fillRect(display.width() - 30, 5, 25, 15, TFT_BLACK);
    }
    
    lastSimMode = currentSimMode;
  }
#endif

  // Draw communication mode indicator (top left)
  static bool lastCommMode = true;  // Track changes
  static String lastCommText = "";
  
  String currentCommText = isCANMode ? "CAN" : "SER";
  
  // Only redraw if communication mode has changed
  if (isCANMode != lastCommMode || currentCommText != lastCommText) {
    // Clear the comm mode area first
    display.fillRect(5, 5, 40, 15, TFT_BLACK);
    
    // Draw new communication mode
    display.loadFont(AA_FONT_SMALL);
    display.setTextColor(isCANMode ? TFT_GREEN : TFT_ORANGE, TFT_BLACK);
    display.setTextDatum(TL_DATUM);
    display.drawString(currentCommText, 5, 5);
    
    lastCommMode = isCANMode;
    lastCommText = currentCommText;
  }

#if ENABLE_DEBUG_MODE
  // Draw debug information at center top if debug mode is active
  static bool lastDebugMode = false;
  static String lastDebugInfo = "";
  
  if (debugMode) {
    // Create debug info string - show only essential info in one line
    String debugInfo = "CPU:" + String(cpuUsage, 1) + "% FPS:" + String(fps, 1) + " Heap:" + String(ESP.getFreeHeap()/1024) + "K";
    
    // Only redraw if debug info has changed or we just entered debug mode
    if (debugInfo != lastDebugInfo || !lastDebugMode) {
      int centerX = display.width() / 2;
      
      // Clear the debug area first to prevent font overlap
      display.fillRect(centerX - 120, 5, 240, 20, TFT_BLACK);
      
      // Draw debug info
      display.loadFont(AA_FONT_SMALL);
      display.setTextColor(TFT_CYAN, TFT_BLACK);
      display.setTextDatum(TC_DATUM);
      display.drawString(debugInfo, centerX, 5);
      
      lastDebugInfo = debugInfo;
    }
    
    lastDebugMode = true;
  } else {
    // Clear debug area when debug mode is turned off
    if (lastDebugMode) {
      // Clear the top center area where debug info was displayed
      display.fillRect(0, 5, display.width(), 20, TFT_BLACK);
      lastDebugMode = false;
      lastDebugInfo = "";
    }
  }
#endif
}
