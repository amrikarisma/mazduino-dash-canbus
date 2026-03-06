#include "KeypadScreen.h"
#include "Config.h"
#include "DataTypes.h"
#include "Roboto16.h"
#include "RobotoBold32.h"
#include "CANHandler.h"

// Global instance
KeypadScreen keypadScreen;

KeypadScreen::KeypadScreen() : tft(nullptr) {
}

void KeypadScreen::begin(TFT_eSPI* display) {
  tft = display;
  initButtons();
}

void KeypadScreen::initButtons() {
  // Grid 3x2 + Back button
  uint16_t btnW = 140;
  uint16_t btnH = 80;
  uint16_t marginX = 15;
  uint16_t marginY = 10;
  uint16_t startX = 20;
  uint16_t startY = 50;
  
  // Row 1
  buttons[FEATURE_START_STOP_ENGINE] = {startX, startY, btnW, btnH, "Start Stop", FEATURE_START_STOP_ENGINE, false};
  buttons[FEATURE_LAUNCH_CONTROL] = {(uint16_t)(startX + btnW + marginX), startY, btnW, btnH, "Launch", FEATURE_LAUNCH_CONTROL, false};
  buttons[FEATURE_ANTILAG] = {(uint16_t)(startX + 2*(btnW + marginX)), startY, btnW, btnH, "Antilag", FEATURE_ANTILAG, false};
  
  // Row 2
  startY += btnH + marginY;
  buttons[FEATURE_ROLLING_ANTILAG] = {startX, startY, btnW, btnH, "Rolling", FEATURE_ROLLING_ANTILAG, false};
  buttons[FEATURE_FLAT_SHIFT] = {(uint16_t)(startX + btnW + marginX), startY, btnW, btnH, "Flatshift", FEATURE_FLAT_SHIFT, false};
  buttons[FEATURE_FAN] = {(uint16_t)(startX + 2*(btnW + marginX)), startY, btnW, btnH, "FAN", FEATURE_FAN, false};
  
  // Back button
  startY += btnH + marginY + 10;
  buttons[FEATURE_BACK] = {startX, startY, 200, 50, "Back", FEATURE_BACK, false};
}

void KeypadScreen::draw(bool setup) {
  if (!tft) return;
  
  if (setup) {
    tft->fillScreen(TFT_BLACK);
    
    // Title
    tft->loadFont(AA_FONT_LARGE);
    tft->setTextColor(TFT_WHITE, TFT_BLACK);
    tft->setTextDatum(TC_DATUM);
    tft->drawString("ECU", 240, 15);
    
    // Draw all buttons
    for (int i = 0; i < FEATURE_COUNT; i++) {
      drawButton(buttons[i], false);
    }
  }
}

void KeypadScreen::drawButton(const FeatureButton& btn, bool pressed) {
  if (!tft) return;

  const uint16_t activeBgColor = TFT_BLUE;
  const uint16_t inactiveBgColor = TFT_DARKGREY;
  const uint16_t activeBorderColor = TFT_WHITE;
  const uint16_t inactiveBorderColor = TFT_LIGHTGREY;
  
  uint16_t bgColor = btn.enabled ? activeBgColor : inactiveBgColor;
  uint16_t textColor = TFT_WHITE;
  uint16_t borderColor = pressed ? TFT_WHITE : (btn.enabled ? activeBorderColor : inactiveBorderColor);
  
  // Background
  tft->fillRoundRect(btn.x, btn.y, btn.w, btn.h, 8, bgColor);
  
  // Border (thicker if enabled)
  if (btn.enabled) {
    tft->drawRoundRect(btn.x, btn.y, btn.w, btn.h, 8, borderColor);
    tft->drawRoundRect(btn.x+1, btn.y+1, btn.w-2, btn.h-2, 7, borderColor);
  } else {
    tft->drawRoundRect(btn.x, btn.y, btn.w, btn.h, 8, borderColor);
  }
  
  // Text
  tft->loadFont(AA_FONT_SMALL);
  tft->setTextColor(textColor, bgColor);
  tft->setTextDatum(MC_DATUM);
  tft->drawString(btn.label, btn.x + btn.w/2, btn.y + btn.h/2);
  
  // Status indicator
  if (btn.feature != FEATURE_BACK) {
    const char* status = btn.enabled ? "ON" : "OFF";
    tft->setTextColor(TFT_WHITE, bgColor);
    tft->drawString(status, btn.x + btn.w/2, btn.y + btn.h - 12);
  }
}

void KeypadScreen::handleTouch(uint16_t x, uint16_t y) {
  ECUFeature feature = checkButtonPress(x, y);
  
  if (feature != (ECUFeature)-1) {
    if (feature == FEATURE_BACK) {
      currentScreen = SCREEN_MAIN;
    } else {
      toggleFeature(feature);
    }
  }
}

ECUFeature KeypadScreen::checkButtonPress(uint16_t x, uint16_t y) {
  for (int i = 0; i < FEATURE_COUNT; i++) {
    const FeatureButton& btn = buttons[i];
    if (x >= btn.x && x <= (btn.x + btn.w) &&
        y >= btn.y && y <= (btn.y + btn.h)) {
      return btn.feature;
    }
  }
  return (ECUFeature)-1;
}

void KeypadScreen::toggleFeature(ECUFeature feature) {
  buttons[feature].enabled = !buttons[feature].enabled;
  // Redraw button
  drawButton(buttons[feature], false);
  
  // Send CAN command if RusEFI mode
  if (isCANMode && canProtocol == CAN_PROTOCOL_RUSEFI) {
    sendCANCommand(feature, buttons[feature].enabled);
  }
}

void KeypadScreen::sendCANCommand(ECUFeature feature, bool enabled) {
  // TODO: Implement actual CAN send based on RusEFI protocol
}
