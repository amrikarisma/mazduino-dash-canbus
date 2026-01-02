#include "DisplayManager.h"
#include "Config.h"
#include "DataTypes.h"
#include "DisplayConfig.h"
#include "drawing_utils.h"
#include "SplashScreen.h"
#include "Roboto16.h"
#include "RobotoBold32.h"
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
extern TFT_eSPI display;
extern TFT_eSprite spr;

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

void drawData() {
  // Track previous screen to only clear when switching
  static uint8_t lastScreen = 255; // Initialize to invalid screen to force first draw
  bool screenChanged = (lastScreen != currentScreen);
  
  if (screenChanged) {
    display.fillScreen(TFT_BLACK);
    resetDrawingUtils(); // Reset static variables in drawing_utils
    lastScreen = currentScreen;
  }
  
  // Switch between screens based on current screen
  switch (currentScreen) {
    case SCREEN_MAIN:
      // Use configurable display system with performance optimizations
      drawConfigurableData(screenChanged);
      break;
    case SCREEN_CONFIG:
      configScreen.draw(screenChanged);
      break;
    case SCREEN_BENCH:
      benchScreen.draw(screenChanged);
      break;
    default:
      // Use configurable display system with performance optimizations
      drawConfigurableData(screenChanged);
      break;
  }
}

void startUpDisplay() {
  display.fillScreen(TFT_BLACK);
  display.loadFont(AA_FONT_SMALL);
  spr.setColorDepth(16);
  display.setTextColor(TFT_WHITE, TFT_BLACK);
  
  // Switch between screens based on current screen with initial load
  switch (currentScreen) {
    case SCREEN_MAIN:
      // Use configurable display system
      drawConfigurableData(true);
      break;
    case SCREEN_CONFIG:
      configScreen.draw(true);
      break;
    case SCREEN_BENCH:
      benchScreen.draw(true);
      break;
    default:
      // Use configurable display system
      drawConfigurableData(true);
      break;
  }
}
