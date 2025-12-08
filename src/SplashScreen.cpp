#include "SplashScreen.h"
#include "Config.h"
#include "GlobalVariables.h"
#include "NotoSansBold15.h"
#include "NotoSansBold36.h"
#include "splash_image/mercy.h"
#include "splash_image/mazduino.h"
#include <EEPROM.h>

// External display object
extern TFT_eSPI display;

// Global splash screen selection variable
int selectedSplashScreen = DEFAULT_SPLASH_SCREEN;

void showAnimatedSplashScreen() {
  display.fillScreen(TFT_BLACK);
  
  // Load splash screen preference from EEPROM
  selectedSplashScreen = EEPROM.read(10); // Use EEPROM address 10 for splash selection
  if (selectedSplashScreen != SPLASH_MERCY && selectedSplashScreen != SPLASH_MAZDUINO) {
    selectedSplashScreen = DEFAULT_SPLASH_SCREEN; // Default to mercy if invalid
  }
  
  // Display the selected image with color byte swapping for correct colors
  display.setSwapBytes(true);  // Enable byte swapping for RGB565 color correction
  
  if (selectedSplashScreen == SPLASH_MAZDUINO) {
    // Mazduino uses monochrome bitmap format, use drawBitmap with white color
    display.drawBitmap(0, 0, epd_bitmap_mazduino_invert, 480, 320, TFT_WHITE, TFT_BLACK);
  } else {
    // Mercy uses uint16_t RGB565 format, can use pushImage directly  
    display.pushImage(0, 0, 480, 320, epd_bitmap_mercy);
  }
  
  display.setSwapBytes(false); // Disable byte swapping after image display
  
  // Hold the image for 3 seconds
  delay(3000);
  
  // Simple fade to black
  display.fillScreen(TFT_BLACK);
}

int getSplashScreenSelection() {
  return selectedSplashScreen;
}

void setSplashScreenSelection(int selection) {
  if (selection == SPLASH_MERCY || selection == SPLASH_MAZDUINO) {
    selectedSplashScreen = selection;
    EEPROM.write(10, selection);
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

void drawFadeOutTransition() {
  // Legacy function - now unused
}