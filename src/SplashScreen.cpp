#include "SplashScreen.h"
#include "Config.h"
#include "GlobalVariables.h"
#include "NotoSansBold15.h"
#include "NotoSansBold36.h"

// External display object
extern TFT_eSPI display;

void showAnimatedSplashScreen() {
  display.fillScreen(TFT_BLACK);
  
  int centerX = display.width() / 2;
  int centerY = (display.height() / 2) - 35;
  
  // Animation sequence - no slide animations to avoid ghosting
  drawFadeInBackground(centerX, centerY);
  drawFadeInTitle(centerX, centerY);
  drawFadeInSubtitle(centerX, centerY);
  drawFadeInWebsite(centerX, centerY);
  drawPulsingTitle(centerX, centerY);
  drawLoadingBar(centerX, centerY);
  drawFadeOutTransition();
  
  // Final black screen
  display.fillScreen(TFT_BLACK);
}

void drawFadeInBackground(int centerX, int centerY) {
  // Animation 1: Fade in logo background
  for (int alpha = 0; alpha <= 40; alpha += 2) {
    uint16_t bgColor = display.color565(0, alpha/2, alpha); // Dark blue background
    display.fillRoundRect(centerX - 200, centerY - 60, 400, 120, 10, bgColor);
    delay(10);
  }
}

void drawFadeInTitle(int centerX, int centerY) {
  // Animation 2: Fade in main title
  display.loadFont(AA_FONT_LARGE);
  display.setTextDatum(TC_DATUM);
  
  uint16_t bgColor = display.color565(0, 20, 40);
  
  // Draw the background rectangle once
  display.fillRoundRect(centerX - 200, centerY - 60, 400, 120, 10, bgColor);
  
  // Fade in the title text
  for (int alpha = 0; alpha <= 255; alpha += 15) {
    // Clear title area
    display.fillRect(centerX - 150, centerY - 20, 300, 40, bgColor);
    
    // Draw title with fade effect
    uint16_t textColor = display.color565(alpha, alpha, alpha);
    display.setTextColor(textColor, bgColor);
    display.drawString("MAZDUINO Display", centerX, centerY);
    
    delay(20);
  }
}

void drawFadeInSubtitle(int centerX, int centerY) {
  // Animation 3: Fade in subtitle
  display.loadFont(AA_FONT_SMALL);
  String versionText = "Firmware version: " + String(version);
  uint16_t bgColor = display.color565(0, 20, 40);
  
  for (int alpha = 0; alpha <= 200; alpha += 20) {
    // Clear subtitle area
    display.fillRect(centerX - 120, centerY + 40, 240, 30, bgColor);
    
    // Draw subtitle with fade effect
    uint16_t textColor = display.color565(alpha, alpha, alpha);
    display.setTextColor(textColor, bgColor);
    display.drawString(versionText, centerX, centerY + 50);
    
    delay(20);
  }
}

void drawFadeInWebsite(int centerX, int centerY) {
  // Animation 4: Fade in website text
  String websiteText = "www.mazduino.com";
  uint16_t bgColor = display.color565(0, 20, 40);
  
  // Draw the background rectangle once
  display.fillRoundRect(centerX - 100, 290, 200, 30, 5, bgColor);
  
  // Fade in the website text
  for (int alpha = 0; alpha <= 255; alpha += 20) {
    // Clear website area
    display.fillRect(centerX - 100, 290, 200, 30, bgColor);
    
    // Draw website with fade effect
    uint16_t textColor = display.color565(0, alpha, alpha); // Cyan fade effect
    display.setTextColor(textColor, bgColor);
    display.drawString(websiteText, centerX, 300);
    
    delay(30);
  }
}

void drawPulsingTitle(int centerX, int centerY) {
  // Animation 5: Pulsing effect on main title
  display.loadFont(AA_FONT_LARGE);
  uint16_t bgColor = display.color565(0, 20, 40);
  
  for (int pulse = 0; pulse < 2; pulse++) {
    // Pulse bright
    for (int brightness = 255; brightness >= 180; brightness -= 10) {
      // Clear title area
      display.fillRect(centerX - 150, centerY - 20, 300, 40, bgColor);
      
      // Draw pulsing title
      uint16_t pulseColor = display.color565(brightness, brightness, brightness);
      display.setTextColor(pulseColor, bgColor);
      display.drawString("MAZDUINO Display", centerX, centerY);
      
      delay(15);
    }
    
    // Pulse dim
    for (int brightness = 180; brightness <= 255; brightness += 10) {
      // Clear title area
      display.fillRect(centerX - 150, centerY - 20, 300, 40, bgColor);
      
      // Draw pulsing title
      uint16_t pulseColor = display.color565(brightness, brightness, brightness);
      display.setTextColor(pulseColor, bgColor);
      display.drawString("MAZDUINO Display", centerX, centerY);
      
      delay(15);
    }
  }
}

void drawLoadingBar(int centerX, int centerY) {
  // Animation 6: Loading bar animation
  int barWidth = 280;
  int barHeight = 8;
  int barX = centerX - barWidth / 2;
  int barY = centerY + 75;
  uint16_t bgColor = display.color565(0, 20, 40);
  
  // Draw loading bar background
  display.fillRoundRect(barX - 2, barY - 2, barWidth + 4, barHeight + 4, 5, TFT_DARKGREY);
  display.fillRoundRect(barX, barY, barWidth, barHeight, 3, TFT_BLACK);
  
  // Animate loading bar
  for (int progress = 0; progress <= barWidth; progress += 4) {
    // Clear percentage area
    display.fillRect(centerX - 25, barY + 20, 50, 20, bgColor);
    
    // Draw progress bar
    for (int i = 0; i < progress; i++) {
      uint16_t barColor;
      if (i < barWidth * 0.4) {
        barColor = TFT_RED;
      } else if (i < barWidth * 0.8) {
        barColor = TFT_YELLOW;
      } else {
        barColor = TFT_GREEN;
      }
      display.drawFastVLine(barX + i, barY, barHeight, barColor);
    }
    
    // Show percentage
    int percentage = (progress * 100) / barWidth;
    display.setTextColor(TFT_WHITE, bgColor);
    display.loadFont(AA_FONT_SMALL);
    display.drawString(String(percentage) + "%", centerX, barY + 25);
    
    delay(10);
  }
  
  // Hold at 100% for a moment
  delay(300);
}

void drawFadeOutTransition() {
  // Animation 7: Fade out splash screen
  for (int alpha = 40; alpha >= 0; alpha -= 2) {
    uint16_t fadeColor = display.color565(0, alpha/2, alpha);
    display.fillScreen(fadeColor);
    delay(10);
  }
}