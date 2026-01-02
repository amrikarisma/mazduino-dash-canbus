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
#include "bg/bg.h"
#include <EEPROM.h>
#include <WiFi.h>
#if ENABLE_SIMULATOR
#include "Simulator.h"
#endif

// External references
extern TouchHandler touchHandler;
extern TFT_eSPI display;
extern TFT_eSprite spr;

// Background loading state
static bool backgroundLoaded = false;
static bool displayInitialized = false;

// Background image loading function
void loadBackgroundImage() {
  if (!backgroundLoaded) {
    // Draw background image (480x320) from PROGMEM with RGB565 swap
    display.setSwapBytes(true);  // Enable byte swapping for RGB565
    display.pushImage(0, 0, 480, 320, epd_bitmap_bg);
    display.setSwapBytes(false); // Disable byte swapping after loading
    backgroundLoaded = true;
    Serial.println("[Display] Background image loaded with RGB565 swap");
  }
}

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
  
  // Skip if display was just initialized in startUpDisplay to prevent double loading
  if (!displayInitialized && screenChanged) {
    return;
  }
  
  if (screenChanged) {
    // Reset background flag and load background image only when screen changes
    backgroundLoaded = false;
    loadBackgroundImage();
    resetDrawingUtils(); // Reset static variables in drawing_utils
    lastScreen = currentScreen;
    Serial.printf("[Display] Screen changed to: %d\n", currentScreen);
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
  
  // Mark as initialized after first call
  displayInitialized = true;
}

void startUpDisplay() {
  // Initialize display settings and load background once
  display.loadFont(AA_FONT_SMALL);
  spr.setColorDepth(16);
  display.setTextColor(TFT_WHITE, TFT_BLACK);
  
  // Load background image once during startup
  loadBackgroundImage();
  
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
  
  // Mark display as initialized
  displayInitialized = true;
}
