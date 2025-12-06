#include "SplashScreen.h"
#include "Config.h"
#include "GlobalVariables.h"
#include "NotoSansBold15.h"
#include "NotoSansBold36.h"
#include "splash_image/mercy.h"

// External display object
extern TFT_eSPI display;

void showAnimatedSplashScreen() {
  display.fillScreen(TFT_BLACK);
  
  // Display the mercy image with color byte swapping for correct colors
  display.setSwapBytes(true);  // Enable byte swapping for RGB565 color correction
  display.pushImage(0, 0, 480, 320, epd_bitmap_mercy);
  display.setSwapBytes(false); // Disable byte swapping after image display
  
  // Hold the image for 3 seconds
  delay(3000);
  
  // Simple fade to black
  display.fillScreen(TFT_BLACK);
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