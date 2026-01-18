#include "SplashScreen.h"
#include "Config.h"
#include "GlobalVariables.h"
#include "BacklightControl.h"
#include "Roboto16.h"
#include "RobotoBold32.h"
#include "Arduino.h"
#include "version.h"
#include "splash_image/mazduino.h"
#include <EEPROM.h>

// External display object
extern TFT_eSPI display;

// Global splash screen selection variable
int selectedSplashScreen = DEFAULT_SPLASH_SCREEN;

void showAnimatedSplashScreen() {
  display.fillScreen(TFT_BLACK);
  
  // Load splash screen preference from EEPROM
  selectedSplashScreen = EEPROM.read(EEPROM_SPLASH_SCREEN_ADDR); // Use centralized EEPROM address
  if (selectedSplashScreen != SPLASH_MAZDUINO && selectedSplashScreen != SPLASH_MERCY && selectedSplashScreen != SPLASH_HEDON && selectedSplashScreen != SPLASH_BIIES && selectedSplashScreen != SPLASH_ZYCAS && selectedSplashScreen != SPLASH_SPINE) {
    selectedSplashScreen = DEFAULT_SPLASH_SCREEN; // Default to mazduino if invalid
  }
  
  // Display the selected image with color byte swapping for correct colors
  display.setSwapBytes(true);  // Enable byte swapping for RGB565 color correction
  
  if (selectedSplashScreen == SPLASH_MAZDUINO) {
    // Mazduino uses monochrome bitmap format, use drawBitmap with white color
    display.drawBitmap(0, 0, epd_bitmap_mazduino_invert, 480, 320, TFT_WHITE, TFT_BLACK);
  } else {
    // For all other splash screens, show informative text
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setTextDatum(MC_DATUM);
    display.drawString("MAZDUINO DISPLAY", 240, 140, 4);
    display.drawString("Selected splash image not available", 240, 180, 2);
    display.drawString("(Only Mazduino available for flash optimization)", 240, 200, 2);
    display.drawString("Please select Mazduino splash", 240, 240, 2);
  }
  
  display.setSwapBytes(false); // Disable byte swapping after image display
  
  // Display version information overlay
  drawVersionInfo(240, 300);
  
  // Get target brightness, cap at 200 for smooth fade
  int targetBrightness = (backlightBrightness > 200) ? 200 : (int)backlightBrightness;
  
  // Gradual fade-in effect from completely off to target brightness
  for (int brightness = 0; brightness <= targetBrightness; brightness += 5) {
    ledcWrite(BACKLIGHT_CHANNEL, brightness);
    delay(50); // Smooth fade-in over ~2 seconds
  }
  
  // Hold the image for 2 seconds at target brightness
  delay(2000);
  
  // Quick fade out back to off
  for (int brightness = targetBrightness; brightness >= 0; brightness -= 10) {
    ledcWrite(BACKLIGHT_CHANNEL, brightness);
    delay(20);
  }
  
  // Simple fade to black
  display.fillScreen(TFT_BLACK);
}

int getSplashScreenSelection() {
  return selectedSplashScreen;
}

void setSplashScreenSelection(int selection) {
  if (selection == SPLASH_MAZDUINO || selection == SPLASH_MERCY || selection == SPLASH_HEDON || selection == SPLASH_BIIES || selection == SPLASH_ZYCAS || selection == SPLASH_SPINE) {
    selectedSplashScreen = selection;
    EEPROM.write(EEPROM_SPLASH_SCREEN_ADDR, selection);
    EEPROM.commit();
  }
}

// Legacy functions kept for compatibility
void drawFadeInBackground(int centerX, int centerY) {
  // Legacy function - now unused
}

void drawFadeInTitle(int centerX, int centerY) {
  // Legacy function - now unused  
}

void drawFadeInSubtitle(int centerX, int centerY) {
  // Legacy function - now unused
}

void drawFadeInWebsite(int centerX, int centerY) {
  // Legacy function - now unused
}

void drawPulsingTitle(int centerX, int centerY) {
  // Legacy function - now unused
}

void drawLoadingBar(int centerX, int centerY) {
  // Legacy function - now unused
}

void drawVersionInfo(int centerX, int centerY) {
  // Set text properties for version info
  display.setTextColor(TFT_WHITE, TFT_BLACK);
  display.setTextDatum(MC_DATUM);
  display.setTextSize(1);
  
  // Create version strings with fallback if version.h is not available
  #ifdef VERSION_STRING
    String versionText = "v" + String(VERSION_STRING);
    String buildInfo = String(BUILD_DATE) + " " + String(BUILD_TIME);
    String commitInfo = "[" + String(BUILD_HASH) + "]";
  #else
    String versionText = "v1.3.0";
    String buildInfo = "Development Build";
    String commitInfo = "[dev]";
  #endif
  
  // Draw version info at bottom of screen with semi-transparent background
  int yPos = centerY;
  
  // Draw background rectangle for better readability
  display.fillRoundRect(centerX - 120, yPos - 30, 240, 50, 8, TFT_BLACK);
  display.drawRoundRect(centerX - 120, yPos - 30, 240, 50, 8, TFT_DARKGREY);
  
  // Draw version text
  display.setTextSize(2);
  display.drawString(versionText, centerX, yPos - 15);
  display.setTextSize(1);
  display.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  display.drawString(buildInfo, centerX, yPos + 3);
  display.drawString(commitInfo, centerX, yPos + 13);
}

void drawFadeOutTransition() {
  // Legacy function - now unused
}