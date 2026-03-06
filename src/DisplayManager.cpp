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
#include "MenuScreen.h"
#include "ACScreen.h"
#include "KeypadScreen.h"
#if ENABLE_BACKGROUND_IMAGE
#include "bg/bg.h"
#endif
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
#if ENABLE_BACKGROUND_IMAGE
    // Draw background image (480x320) from PROGMEM with RGB565 swap
    display.setSwapBytes(true);  // Enable byte swapping for RGB565
    display.pushImage(0, 0, 480, 320, epd_bitmap_bg);
    display.setSwapBytes(false); // Disable byte swapping after loading
    Serial.println("[Display] Background image loaded with RGB565 swap");
#else
    display.fillScreen(TFT_BLACK);
    Serial.println("[Display] Background image disabled, using black fill");
#endif
    backgroundLoaded = true;
  }
}

void setupDisplay() {
  display.init();
  display.setRotation(3);
  // Initialize display configuration
  initializeDisplayConfig();
}

void drawSplashScreenWithImage() {
  // Skip splash screen if DISABLE_SPLASH flag is set
#ifndef DISABLE_SPLASH
  // Use the new modular animated splash screen
  showAnimatedSplashScreen();
#endif
}

void drawData() {
  // Track previous screen to only clear when switching
  static uint8_t lastScreen = 255; // Initialize to invalid screen to force first draw
  static uint32_t startupTime = millis();
  bool screenChanged = (lastScreen != currentScreen);
  
  // If display was just initialized, update lastScreen to current screen to prevent false screen change detection
  if (displayInitialized && lastScreen == 255) {
    lastScreen = currentScreen;
    screenChanged = false; // No actual screen change, just initialization
    Serial.printf("[Display] Initial screen sync: setting lastScreen to %d\n", currentScreen);
    return; // Skip this cycle since startUpDisplay already handled the initial draw
  }
  
  // Prevent immediate redraw after startup (give 2 seconds grace period)
  if (displayInitialized && (millis() - startupTime < 2000) && !screenChanged) {
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
    case SCREEN_MENU:
      menuScreen.draw(screenChanged);
      break;
    case SCREEN_AC:
      acScreen.draw(screenChanged);
      // drawACInfo() now has internal throttling to prevent flickering
      break;
    case SCREEN_KEYPAD:
      keypadScreen.draw(screenChanged);
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
  Serial.println("[Display] Starting initial display setup...");
  
  // Initialize display settings and load background once
  display.loadFont(AA_FONT_SMALL);
  spr.setColorDepth(16);
  display.setTextColor(TFT_WHITE, TFT_BLACK);
  
  // Load background image once during startup
  loadBackgroundImage();
  
  // Switch between screens based on current screen with initial load
  switch (currentScreen) {
    case SCREEN_MAIN:
      Serial.println("[Display] Drawing initial MAIN screen...");
      // Use configurable display system
      drawConfigurableData(true);
      break;
    case SCREEN_CONFIG:
      Serial.println("[Display] Drawing initial CONFIG screen...");
      configScreen.draw(true);
      break;
    case SCREEN_BENCH:
      Serial.println("[Display] Drawing initial BENCH screen...");
      benchScreen.draw(true);
      break;
    case SCREEN_MENU:
      Serial.println("[Display] Drawing initial MENU screen...");
      menuScreen.draw(true);
      break;
    case SCREEN_AC:
      Serial.println("[Display] Drawing initial AC screen...");
      acScreen.draw(true);
      break;
    case SCREEN_KEYPAD:
      Serial.println("[Display] Drawing initial KEYPAD screen...");
      keypadScreen.draw(true);
      break;
    default:
      Serial.println("[Display] Drawing default MAIN screen...");
      // Use configurable display system
      drawConfigurableData(true);
      break;
  }
  
  displayInitialized = true; // Set flag to prevent immediate redraw
  Serial.println("[Display] Initial display setup complete");
}
