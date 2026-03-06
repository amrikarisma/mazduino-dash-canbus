#include "MenuScreen.h"
#include "Config.h"
#include "DataTypes.h"
#include "Roboto16.h"
#include "RobotoBold32.h"

// Global instance
MenuScreen menuScreen;

MenuScreen::MenuScreen() : tft(nullptr), selectedItem(-1) {
}

void MenuScreen::begin(TFT_eSPI* display) {
  tft = display;
  initMenuButtons();
}

void MenuScreen::initMenuButtons() {
  // Create 2x2 grid layout 
  // Screen size: 480x320
  // Button size: 200x110 with margins
  
  uint16_t btnWidth = 200;
  uint16_t btnHeight = 110;
  uint16_t marginX = 20;
  uint16_t marginY = 15;
  uint16_t startX = (480 - (2 * btnWidth + marginX)) / 2; // Center horizontally
  uint16_t startY = (320 - (2 * btnHeight + marginY)) / 2; // Center vertically
  
  // Back button (top-left)
  menuButtons[MENU_BACK].x = startX;
  menuButtons[MENU_BACK].y = startY;
  menuButtons[MENU_BACK].w = btnWidth;
  menuButtons[MENU_BACK].h = btnHeight;
  menuButtons[MENU_BACK].label = "Main";
  menuButtons[MENU_BACK].color = TFT_ORANGE;
  menuButtons[MENU_BACK].item = MENU_BACK;
  
  // Config button (top-right)
  menuButtons[MENU_CONFIG].x = startX + btnWidth + marginX;
  menuButtons[MENU_CONFIG].y = startY;
  menuButtons[MENU_CONFIG].w = btnWidth;
  menuButtons[MENU_CONFIG].h = btnHeight;
  menuButtons[MENU_CONFIG].label = "Config";
  menuButtons[MENU_CONFIG].color = TFT_ORANGE;
  menuButtons[MENU_CONFIG].item = MENU_CONFIG;
  
  // Bench button (bottom-left)
  menuButtons[MENU_BENCH].x = startX;
  menuButtons[MENU_BENCH].y = startY + btnHeight + marginY;
  menuButtons[MENU_BENCH].w = btnWidth;
  menuButtons[MENU_BENCH].h = btnHeight;
  menuButtons[MENU_BENCH].label = "Bench Test";
  menuButtons[MENU_BENCH].color = TFT_ORANGE;
  menuButtons[MENU_BENCH].item = MENU_BENCH;
  
  // AC Control button (bottom-right)
  menuButtons[MENU_AC_CONTROL].x = startX + btnWidth + marginX;
  menuButtons[MENU_AC_CONTROL].y = startY + btnHeight + marginY;
  menuButtons[MENU_AC_CONTROL].w = btnWidth;
  menuButtons[MENU_AC_CONTROL].h = btnHeight;
  menuButtons[MENU_AC_CONTROL].label = "AC Control";
  menuButtons[MENU_AC_CONTROL].color = TFT_ORANGE;
  menuButtons[MENU_AC_CONTROL].item = MENU_AC_CONTROL;
}

void MenuScreen::draw(bool setup) {
  if (!tft) return;
  
  if (setup) {
    tft->fillScreen(TFT_BLACK);
    
    // Draw all buttons
    for (int i = 0; i < MENU_ITEM_COUNT; i++) {
      drawButton(menuButtons[i], false);
    }
  }
}

void MenuScreen::drawButton(const MenuButton& btn, bool highlighted) {
  if (!tft) return;
  
  uint16_t bgColor = highlighted ? TFT_WHITE : btn.color;
  uint16_t textColor = highlighted ? btn.color : TFT_WHITE;
  
  // Draw button background
  tft->fillRoundRect(btn.x, btn.y, btn.w, btn.h, 10, bgColor);
  
  // Draw button border
  tft->drawRoundRect(btn.x, btn.y, btn.w, btn.h, 10, TFT_WHITE);
  
  // Draw button text with smooth font
  tft->setTextColor(textColor, bgColor);
  tft->loadFont(AA_FONT_SMALL);
  tft->setTextDatum(MC_DATUM);
  tft->drawString(btn.label, btn.x + btn.w / 2, btn.y + btn.h / 2);
  tft->setTextDatum(TL_DATUM);
}

void MenuScreen::handleTouch(uint16_t x, uint16_t y) {
  MenuItem item = checkButtonPress(x, y);
  
  if (item != (MenuItem)-1) {
    // Button was pressed
    Serial.printf("[Menu] Button pressed: %d\n", item);
    
    switch (item) {
      case MENU_BACK:
        navigateTo(SCREEN_MAIN);
        Serial.println("[Menu] Opening Main screen");
        break;
        
      case MENU_CONFIG:
        navigateTo(SCREEN_CONFIG);
        Serial.println("[Menu] Opening Configuration screen");
        break;
        
      case MENU_BENCH:
        navigateTo(SCREEN_BENCH);
        Serial.println("[Menu] Opening Bench Test screen");
        break;
        
      case MENU_AC_CONTROL:
        navigateTo(SCREEN_AC);
        Serial.println("[Menu] Opening AC Control screen");
        break;
        
      default:
        break;
    }
  }
}

MenuItem MenuScreen::checkButtonPress(uint16_t x, uint16_t y) {
  for (int i = 0; i < MENU_ITEM_COUNT; i++) {
    const MenuButton& btn = menuButtons[i];
    
    if (x >= btn.x && x <= (btn.x + btn.w) &&
        y >= btn.y && y <= (btn.y + btn.h)) {
      return btn.item;
    }
  }
  
  return (MenuItem)-1; // No button pressed
}
