#include "SplashScreen.h"
#include "Config.h"
#include "GlobalVariables.h"
#include "BacklightControl.h"
#include "Roboto16.h"
#include "RobotoBold32.h"
#include "Arduino.h"
#include "version.h"
// Conditional includes based on build-time splash selection to save flash memory
#if BUILD_DEFAULT_SPLASH == 0  // SPLASH_MAZDUINO
#include "splash_image/mazduino.h"
#elif BUILD_DEFAULT_SPLASH == 6  // SPLASH_JW
#include "splash_image/jw.h"
#elif BUILD_DEFAULT_SPLASH == 7  // SPLASH_BMW
#include "splash_image/bmw.h"
#elif BUILD_DEFAULT_SPLASH == 1  // SPLASH_MERCY
#include "splash_image/mercy.h"
#elif BUILD_DEFAULT_SPLASH == 2  // SPLASH_HEDON
#include "splash_image/hedon.h"
#elif BUILD_DEFAULT_SPLASH == 3  // SPLASH_BIIES
#include "splash_image/biies.h"
#elif BUILD_DEFAULT_SPLASH == 4  // SPLASH_ZYCAS
#include "splash_image/zycas.h"
#elif BUILD_DEFAULT_SPLASH == 5  // SPLASH_SPINE
#include "splash_image/spine.h"
#else
// Default fallback to Mazduino
#include "splash_image/mazduino.h"
#endif
#include <EEPROM.h>

// External display object
extern TFT_eSPI display;

// Global splash screen selection variable
int selectedSplashScreen = DEFAULT_SPLASH_SCREEN;

void showAnimatedSplashScreen() {
  display.fillScreen(TFT_BLACK);
  
  // Load splash screen preference from EEPROM
  selectedSplashScreen = EEPROM.read(EEPROM_SPLASH_SCREEN_ADDR); // Use centralized EEPROM address
  
  // Force to build-time selected splash to save flash memory
  // Runtime selection disabled to reduce flash usage
  selectedSplashScreen = BUILD_DEFAULT_SPLASH;
  
  Serial.printf("[Splash] Build-time splash selection: %d\n", selectedSplashScreen);
  
  // Display the selected image with color byte swapping for correct colors
  display.setSwapBytes(true);  // Enable byte swapping for RGB565 color correction
  
  // Conditional rendering based on build-time selection to save flash memory
#if BUILD_DEFAULT_SPLASH == 0  // SPLASH_MAZDUINO
  // Mazduino uses monochrome bitmap format, use drawBitmap with white color
  display.drawBitmap(0, 0, epd_bitmap_mazduino_invert, 480, 320, TFT_WHITE, TFT_BLACK);
  Serial.println("[Splash] Displaying Mazduino splash");
#elif BUILD_DEFAULT_SPLASH == 6  // SPLASH_JW
  // JW uses RGB565 color format, display full color image (480x320)
  display.pushImage(0, 0, 480, 320, epd_bitmap_jw);
  Serial.println("[Splash] Displaying JW splash");
#elif BUILD_DEFAULT_SPLASH == 7  // SPLASH_BMW
  // BMW uses RGB565 color format, display full color image (480x320)
  display.pushImage(0, 0, 480, 320, epd_bitmap_bmw);
  Serial.println("[Splash] Displaying BMW splash");
#elif BUILD_DEFAULT_SPLASH == 1  // SPLASH_MERCY
  display.pushImage(0, 0, 480, 320, epd_bitmap_mercy);
  Serial.println("[Splash] Displaying Mercy splash");
#elif BUILD_DEFAULT_SPLASH == 2  // SPLASH_HEDON
  display.pushImage(0, 0, 480, 320, epd_bitmap_hedon);
  Serial.println("[Splash] Displaying Hedon splash");
#elif BUILD_DEFAULT_SPLASH == 3  // SPLASH_BIIES
  display.pushImage(0, 0, 480, 320, epd_bitmap_biies);
  Serial.println("[Splash] Displaying Biies splash");
#elif BUILD_DEFAULT_SPLASH == 4  // SPLASH_ZYCAS
  display.pushImage(0, 0, 480, 320, epd_bitmap_zycas);
  Serial.println("[Splash] Displaying Zycas splash");
#elif BUILD_DEFAULT_SPLASH == 5  // SPLASH_SPINE
  display.pushImage(0, 0, 480, 320, epd_bitmap_spine);
  Serial.println("[Splash] Displaying Spine splash");
#else
  // Default fallback to Mazduino (monochrome)
  display.drawBitmap(0, 0, epd_bitmap_mazduino_invert, 480, 320, TFT_WHITE, TFT_BLACK);
  Serial.println("[Splash] Displaying Mazduino splash (fallback)");
#endif
  
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
  if (selection == SPLASH_MAZDUINO || selection == SPLASH_MERCY || selection == SPLASH_HEDON || selection == SPLASH_BIIES || selection == SPLASH_ZYCAS || selection == SPLASH_SPINE || selection == SPLASH_JW) {
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