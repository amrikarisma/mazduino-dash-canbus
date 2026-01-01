#include "DisplayManager.h"
#include "Config.h"
#include "DataTypes.h"
#include "DisplayConfig.h"
#include "drawing_utils.h"
#include "SplashScreen.h"
#include "NotoSansBold15.h"
#include "NotoSansBold36.h"
#include "TouchHandler.h"
#include "MainScreen.h"
#include "ConfigScreen.h"
#include "BenchScreen.h"
#include <EEPROM.h>
#include <WiFi.h>
#if ENABLE_SIMULATOR
#include "Simulator.h"
#endif

// External references
extern TouchHandler touchHandler;

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
    {0, 200},   // Position 2: Left-Bottom (IAT) - Y=245 to Y=325
    {120, 20},   // Position 3: Center-Top (MAP) - Y=160 to Y=240
    {120, 110},   // Position 4: Center-Middle (ECT) - Y=245 to Y=325
    {120, 200},  // Position 5: Center-Bottom (Baro) - Y=245 to Y=325
    {390, 20},  // Position 6: Right-Top (AFR2) - Y=160 to Y=240
    {390, 110}   // Position 7: Right-Middle (Unused) - Y=245 to Y=325
  };

  if (panel.position >= 8) return;

  int x = panelPositions[panel.position][0];
  int y = panelPositions[panel.position][1];

  float value = getDataValue(panel.dataSource);
  uint16_t valueColor = getDataSourceColor(panel.dataSource, value);
  drawDataBox(x, y, panel.label, value, panel.color, 0, panel.decimals, setup);
}

void drawDataBox(int x, int y, const char *label, const float value, uint16_t labelColor, const float valueToCompare, const int decimal, bool setup) {
  // Create sprite for data box
  spr.createSprite(100, 80);
  
  // Calculate colors
  uint16_t valueColor = TFT_WHITE;
  uint16_t bgColor = TFT_BLACK;
  
  // Warning color logic
  if (valueToCompare > 0 && value > valueToCompare) {
    valueColor = TFT_RED;
  } else if (value < 0) {
    valueColor = TFT_YELLOW;
  }
  
  // Clear sprite
  spr.fillSprite(bgColor);
  
  // Draw border
  spr.drawRect(0, 0, 100, 80, TFT_DARKGREY);
  
  // Draw label
  spr.loadFont(AA_FONT_SMALL);
  spr.setTextColor(labelColor, bgColor);
  spr.setTextDatum(TC_DATUM);
  spr.drawString(label, 50, 5);
  
  // Draw value
  spr.loadFont(AA_FONT_LARGE);
  spr.setTextColor(valueColor, bgColor);
  spr.setTextDatum(MC_DATUM);
  
  if (decimal == 0) {
    spr.drawNumber((int)value, 50, 45);
  } else {
    spr.drawFloat(value, decimal, 50, 45);
  }
  
  // Push sprite to display
  spr.pushSprite(x, y);
  spr.deleteSprite();
}

void drawData() {
  static uint8_t lastScreen = 255; // Initialize to invalid screen
  bool screenChanged = (currentScreen != lastScreen);
  
  if (screenChanged) {
    // Force full screen redraw when screen changes
    display.fillScreen(TFT_BLACK);
    lastScreen = currentScreen;
    
    // Initialize screens if not already done
    static bool screensInitialized = false;
    if (!screensInitialized) {
      mainScreen.begin();
      configScreen.begin();
      benchScreen.begin();
      screensInitialized = true;
    }
  }
  
  // Draw different screens based on currentScreen using modular classes
  switch (currentScreen) {
    case SCREEN_MAIN:
      mainScreen.draw(screenChanged);
      break;
    case SCREEN_CONFIG:
      configScreen.draw(screenChanged);
      break;
    case SCREEN_BENCH:
      benchScreen.draw(screenChanged);
      break;
    default:
      // Fallback to main screen
      currentScreen = SCREEN_MAIN;
      mainScreen.draw(true);
      break;
  }
}

void startUpDisplay() {
  display.fillScreen(TFT_BLACK);
  display.loadFont(AA_FONT_SMALL);
  spr.setColorDepth(16);
  display.setTextColor(TFT_WHITE, TFT_BLACK);
  
  // Initialize main screen and draw initial data
  mainScreen.begin();
  mainScreen.draw(true);
}

void drawConfigurableIndicators() {
  // Draw indicators based on current configuration
  for (int i = 0; i < currentDisplayConfig.activeIndicatorCount; i++) {
    IndicatorConfig &indicator = currentDisplayConfig.indicators[i];
    if (indicator.enabled) {
      bool isActive = getIndicatorValue(indicator.indicator);
      
      // Draw indicator at bottom of screen
      int x = 10 + (i * 60);
      int y = 290;
      
      uint16_t color = isActive ? TFT_GREEN : TFT_DARKGREY;
      display.fillRect(x, y, 50, 20, color);
      display.drawRect(x, y, 50, 20, TFT_WHITE);
      
      display.loadFont(AA_FONT_SMALL);
      display.setTextColor(TFT_BLACK, color);
      display.setTextDatum(MC_DATUM);
      display.drawString(indicator.label, x + 25, y + 10);
    }
  }
}
