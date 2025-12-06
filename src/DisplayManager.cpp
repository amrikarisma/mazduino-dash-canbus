#include "DisplayManager.h"
#include "Config.h"
#include "DataTypes.h"
#include "DisplayConfig.h"
#include "drawing_utils.h"
#include "SplashScreen.h"
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
  // Use the new modular animated splash screen
  showAnimatedSplashScreen();
}

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
    {0, 20},   // Position 0: Left-Top (AFR) - Y=160 to Y=240
    {0, 110},   // Position 1: Left-Middle (TPS) - Y=245 to Y=325
    {0, 200},   // Position 2: Left-Bottom (IAT) - Y=280 to Y=360, will be clipped but user requested
    {390, 20}, // Position 3: Right-Top (MAP) - Y=160 to Y=240
    {390, 110}, // Position 4: Right-Middle (ADV) - Y=245 to Y=325
    {390, 200}, // Position 5: Right-Bottom (FP) - Y=280 to Y=360, will be clipped but user requested
    {130, 200}, // Position 6: Center-Left (Coolant) - moved to center to avoid overlap
    {250, 200}  // Position 7: Center-Right (Voltage) - moved to center to avoid overlap
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
    
    // Draw RPM value with background and border
    spr.loadFont(AA_FONT_SMALL);
    spr.createSprite(90, 30);
    spr_width = spr.textWidth("8888");
    spr.setTextColor(TFT_WHITE, TFT_BLACK, true);
    spr.setTextDatum(BR_DATUM);
    spr.drawString("RPM", 80, 2); // Label at top
    spr.setTextDatum(BR_DATUM);
    spr.loadFont(AA_FONT_SMALL); // Use smaller font for RPM value to match existing pattern
    spr.drawNumber(rpm, 80, 30); // Value at bottom with smaller font
    spr.pushSprite(275, 15);
    spr.deleteSprite();
    
    // Draw VSS value with background and border
    spr.createSprite(120, 50);
    spr_width = spr.textWidth("888");
    spr.setTextColor(TFT_WHITE, TFT_BLACK, true);
    
    // Draw the numeric value with large font
    spr.setTextDatum(BR_DATUM);
    spr.loadFont(AA_FONT_LARGE);
    spr.drawNumber(vss, 60, 46); // Value positioned to left
    
    // Draw "kph" unit with smaller font
    spr.setTextDatum(BL_DATUM);
    spr.loadFont(AA_FONT_SMALL);
    spr.drawString("kph", 62, 42); // Unit positioned to right of value
    
    spr.pushSprite(250, 135);
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

void startUpDisplay() {
  display.fillScreen(TFT_BLACK);
  display.loadFont(AA_FONT_SMALL);
  spr.setColorDepth(16);
  display.setTextColor(TFT_WHITE, TFT_BLACK);
  
  // Use configurable display system
  drawConfigurableData(true);
  
  // Animation with both RPM and VSS values
  spr.loadFont(AA_FONT_SMALL);
  for (int i = DEFAULT_MAX_RPM; i >= 0; i -= 250) {
    drawRPMBarBlocks(i); // Use default maxRPM from config
  }
}

void drawDataBox(int x, int y, const char *label, const float value, uint16_t labelColor, const float valueToCompare, const int decimal, bool setup) {
  const int BOX_WIDTH = 80;
  const int BOX_HEIGHT = 80;
  const int LABEL_HEIGHT = BOX_HEIGHT / 2;

  if (setup) {
    // Clear the entire data box area first only during setup
    display.fillRect(x, y, BOX_WIDTH, BOX_HEIGHT, TFT_BLACK);
    
    // Draw border around the entire panel
    display.drawRoundRect(x, y, BOX_WIDTH, BOX_HEIGHT, 5, TFT_DARKGREY);      // Outer border
    display.drawRoundRect(x + 1, y + 1, BOX_WIDTH - 2, BOX_HEIGHT - 2, 4, labelColor); // Inner border with label color
    
    spr.loadFont(AA_FONT_SMALL);
    spr.createSprite(BOX_WIDTH, LABEL_HEIGHT);
    spr.fillSprite(TFT_BLACK);  // Clear sprite background
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
    spr.fillSprite(TFT_BLACK);  // Clear sprite background
    spr.setTextDatum(TC_DATUM);
    spr_width = spr.textWidth("333");
    spr.setTextColor(labelColor, TFT_BLACK, true);
    if (decimal > 0) {
      spr.drawFloat(value, decimal, 40, 5);
    } else {
      spr.drawNumber(value, 40, 5);
    }
    spr.pushSprite(x, y + LABEL_HEIGHT - 15);
    spr.deleteSprite();
  }
}

void drawData() {
  // Use configurable display system with performance optimizations
  drawConfigurableData(false);
  
#if ENABLE_SIMULATOR
  // Draw simulator indicator if simulator is active (with reduced update frequency)
  static uint8_t lastSimMode = SIMULATOR_MODE_OFF;
  static uint32_t lastSimUpdate = 0;
  uint8_t currentSimMode = getSimulatorMode();
  
  // Only redraw if simulator mode has changed or every 500ms
  if (currentSimMode != lastSimMode || (millis() - lastSimUpdate > 500)) {
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
    lastSimUpdate = millis();
  }
#endif

  // Draw communication mode indicator (top left) with reduced update frequency
  static bool lastCommMode = true;  // Track changes
  static String lastCommText = "";
  static uint32_t lastCommUpdate = 0;
  
  String currentCommText = isCANMode ? "CAN" : "SER";
  
  // Only redraw if communication mode has changed or every 1000ms
  if (isCANMode != lastCommMode || currentCommText != lastCommText || (millis() - lastCommUpdate > 1000)) {
    // Clear the comm mode area first
    display.fillRect(5, 5, 40, 15, TFT_BLACK);
    
    // Draw new communication mode
    display.loadFont(AA_FONT_SMALL);
    display.setTextColor(isCANMode ? TFT_GREEN : TFT_ORANGE, TFT_BLACK);
    display.setTextDatum(TL_DATUM);
    display.drawString(currentCommText, 5, 5);
    
    lastCommMode = isCANMode;
    lastCommText = currentCommText;
    lastCommUpdate = millis();
  }

#if ENABLE_DEBUG_MODE
  static bool lastDebugMode = false;
  static String lastDebugInfo = "";
  
  if (debugMode) {
    // Create debug info string - show only essential info in one line
    String debugInfo = "CPU:" + String(cpuUsage, 1) + "% FPS:" + String(fps, 1) + " Heap:" + String(ESP.getFreeHeap()/1024) + "K";
    
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
