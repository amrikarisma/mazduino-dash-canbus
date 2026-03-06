#include "ACScreen.h"
#include "Config.h"
#include "DataTypes.h"
#include "ESPNowHandler.h"
#include "Roboto16.h"
#include "RobotoBold32.h"
#include <EEPROM.h>

// Global instance
ACScreen acScreen;

// Static variable for AC cutoff temperature
static float savedACCutoffTemp = 18.0; // Default 18°C

ACScreen::ACScreen() : tft(nullptr), acCutoffTemp(18.0), tempChanged(false),
                       cachedACTemp(-999.0), cachedACDataReceived(false), 
                       cachedCutoffTemp(-999.0), labelsDrawn(false), lastACInfoUpdate(0) {
}

void ACScreen::begin(TFT_eSPI* display) {
  tft = display;
  initButtons();
  loadACCutoffTemp();
}

void ACScreen::initButtons() {
  // Back button (top-left)
  buttons[AC_BTN_BACK].x = 20;
  buttons[AC_BTN_BACK].y = 20;
  buttons[AC_BTN_BACK].w = 100;
  buttons[AC_BTN_BACK].h = 50;
  buttons[AC_BTN_BACK].label = "Back";
  buttons[AC_BTN_BACK].color = TFT_DARKGREY;
  buttons[AC_BTN_BACK].button = AC_BTN_BACK;
  
  // Temp UP button (center-right)
  buttons[AC_BTN_TEMP_UP].x = 320;
  buttons[AC_BTN_TEMP_UP].y = 130;
  buttons[AC_BTN_TEMP_UP].w = 130;
  buttons[AC_BTN_TEMP_UP].h = 45;
  buttons[AC_BTN_TEMP_UP].label = "+";
  buttons[AC_BTN_TEMP_UP].color = TFT_RED;
  buttons[AC_BTN_TEMP_UP].button = AC_BTN_TEMP_UP;
  
  // Temp DOWN button (center-right, below UP)
  buttons[AC_BTN_TEMP_DOWN].x = 320;
  buttons[AC_BTN_TEMP_DOWN].y = 185;
  buttons[AC_BTN_TEMP_DOWN].w = 130;
  buttons[AC_BTN_TEMP_DOWN].h = 45;
  buttons[AC_BTN_TEMP_DOWN].label = "-";
  buttons[AC_BTN_TEMP_DOWN].color = TFT_BLUE;
  buttons[AC_BTN_TEMP_DOWN].button = AC_BTN_TEMP_DOWN;
  
  // SEND button (repurposed as empty slot for AC_BTN_SEND)
  buttons[AC_BTN_SEND].x = 0;
  buttons[AC_BTN_SEND].y = 0;
  buttons[AC_BTN_SEND].w = 0;
  buttons[AC_BTN_SEND].h = 0;
  buttons[AC_BTN_SEND].label = "";
  buttons[AC_BTN_SEND].color = TFT_BLACK;
  buttons[AC_BTN_SEND].button = AC_BTN_SEND;
}

void ACScreen::draw(bool setup) {
  if (!tft) return;
  
  if (setup) {
    tft->fillScreen(TFT_BLACK);
    labelsDrawn = false;
    cachedACTemp = -999.0;
    cachedACDataReceived = false;
    cachedCutoffTemp = -999.0;
    
    // Draw title using sprite with proper font
    TFT_eSprite spr(tft);
    spr.loadFont(RobotoBold32);
    spr.createSprite(200, 50);
    spr.fillSprite(TFT_BLACK);
    spr.setTextColor(TFT_WHITE, TFT_BLACK, true);
    spr.setTextDatum(MC_DATUM);
    spr.drawString("AC CONTROL", 100, 25);
    spr.pushSprite(140, 15);
    spr.deleteSprite();
    
    // Draw all buttons
    for (int i = 0; i < AC_BTN_COUNT; i++) {
      drawButton(buttons[i], false);
    }
  }
  
  // Always draw AC info (updates dynamically)
  drawACInfo();
}

void ACScreen::update() {
  // Redraw AC info to show real-time updates
  drawACInfo();
}

void ACScreen::drawACInfo() {
  if (!tft) return;
  
  // Create sprite object for value-only updates
  TFT_eSprite spr(tft);
  
  // Draw labels only on first setup (static content)
  if (!labelsDrawn) {
    // Clear info area (only label/value region, not buttons on right)
    tft->fillRect(40, 80, 270, 180, TFT_BLACK);
    
    // Draw "Current AC:" label
    spr.loadFont(AA_FONT_SMALL);
    spr.createSprite(150, 25);
    spr.fillSprite(TFT_BLACK);
    spr.setTextColor(TFT_CYAN, TFT_BLACK, true);
    spr.setTextDatum(TL_DATUM);
    spr.drawString("Current AC:", 0, 0);
    spr.pushSprite(40, 90);
    spr.deleteSprite();
    
    // Draw "Cutoff Temp:" label
    spr.loadFont(AA_FONT_SMALL);
    spr.createSprite(150, 25);
    spr.fillSprite(TFT_BLACK);
    spr.setTextColor(TFT_CYAN, TFT_BLACK, true);
    spr.setTextDatum(TL_DATUM);
    spr.drawString("Cutoff Temp:", 0, 0);
    spr.pushSprite(40, 150);
    spr.deleteSprite();
    
    labelsDrawn = true;
  }
  
  // Update Current AC Temperature value (only if changed)
  {
    bool dataChanged = false;
    
    if (acDataReceived != cachedACDataReceived) {
      dataChanged = true;
      cachedACDataReceived = acDataReceived;
    }
    
    if (acDataReceived && isACDataValid()) {
      if (acCurrentTemp != cachedACTemp) {
        dataChanged = true;
        cachedACTemp = acCurrentTemp;
      }
    }
    
    if (dataChanged) {
      // Use sprite to only update value, not label
      spr.loadFont(AA_FONT_LARGE);
      spr.createSprite(120, 40);
      spr.fillSprite(TFT_BLACK);
      spr.setTextDatum(TL_DATUM);
      
      if (acDataReceived && isACDataValid()) {
        spr.setTextColor(TFT_GREEN, TFT_BLACK, true);
        char tempStr[32];
        sprintf(tempStr, "%.1fC", acCurrentTemp);
        spr.drawString(tempStr, 0, 0);
      } else {
        spr.setTextColor(TFT_RED, TFT_BLACK, true);
        spr.drawString("NO DATA", 0, 0);
      }
      
      spr.pushSprite(150, 90);
      spr.deleteSprite();
    }
  }
  
  // Update Cutoff Temperature value (only if changed)
  {
    if (acCutoffTemp != cachedCutoffTemp) {
      cachedCutoffTemp = acCutoffTemp;
      
      // Use sprite to only update value, not label
      spr.loadFont(AA_FONT_LARGE);
      spr.createSprite(120, 40);
      spr.fillSprite(TFT_BLACK);
      spr.setTextDatum(TL_DATUM);
      spr.setTextColor(tempChanged ? TFT_YELLOW : TFT_WHITE, TFT_BLACK, true);
      
      char cutoffStr[32];
      sprintf(cutoffStr, "%.1fC", acCutoffTemp);
      spr.drawString(cutoffStr, 0, 0);
      
      spr.pushSprite(150, 150);
      spr.deleteSprite();
      
      // Show send status indicator
      if (tempChanged) {
        spr.loadFont(AA_FONT_SMALL);
        spr.createSprite(80, 20);
        spr.fillSprite(TFT_BLACK);
        spr.setTextColor(TFT_YELLOW, TFT_BLACK, true);
        spr.setTextDatum(TL_DATUM);
        spr.drawString("*unsaved", 0, 0);
        spr.pushSprite(150, 170);
        spr.deleteSprite();
      }
    }
  }
}

void ACScreen::drawButton(const ACButtonDef& btn, bool highlighted) {
  if (!tft) return;
  
  uint16_t bgColor = highlighted ? TFT_WHITE : btn.color;
  uint16_t textColor = highlighted ? btn.color : TFT_WHITE;
  
  // Draw button text using sprite for consistent font rendering
  TFT_eSprite spr(tft);
  spr.loadFont(AA_FONT_LARGE);
  spr.createSprite(btn.w - 4, btn.h - 4);
  spr.fillSprite(bgColor);
  spr.setTextColor(textColor, bgColor, true);
  spr.setTextDatum(MC_DATUM);
  spr.drawString(btn.label, (btn.w - 4) / 2, (btn.h - 4) / 2);
  spr.pushSprite(btn.x + 2, btn.y + 2);
  spr.deleteSprite();
}

void ACScreen::handleTouch(uint16_t x, uint16_t y) {
  ACButton btn = checkButtonPress(x, y);
  
  if (btn != (ACButton)-1) {
    Serial.printf("[AC] Button pressed: %d\n", btn);
    
    static uint32_t lastSendTime = 0;
    const uint32_t SEND_DEBOUNCE_MS = 500; // Debounce send to avoid spam
    
    switch (btn) {
      case AC_BTN_BACK:
        navigateBack();
        Serial.println("[AC] Returning to previous screen");
        break;
        
      case AC_BTN_TEMP_UP:
        acCutoffTemp += 0.5;
        if (acCutoffTemp > 30.0) acCutoffTemp = 30.0;
        tempChanged = true;
        Serial.printf("[AC] Cutoff temp increased to %.1f\n", acCutoffTemp);
        
        // Auto-send data after debounce period
        if (millis() - lastSendTime > SEND_DEBOUNCE_MS) {
          saveACCutoffTemp();
          sendACCutoffToESP32C3(acCutoffTemp);
          tempChanged = false;
          lastSendTime = millis();
          Serial.printf("[AC] Auto-sent cutoff temp to ESP32C3: %.1f\n", acCutoffTemp);
        }
        break;
        
      case AC_BTN_TEMP_DOWN:
        acCutoffTemp -= 0.5;
        if (acCutoffTemp < 10.0) acCutoffTemp = 10.0;
        tempChanged = true;
        Serial.printf("[AC] Cutoff temp decreased to %.1f\n", acCutoffTemp);
        
        // Auto-send data after debounce period
        if (millis() - lastSendTime > SEND_DEBOUNCE_MS) {
          saveACCutoffTemp();
          sendACCutoffToESP32C3(acCutoffTemp);
          tempChanged = false;
          lastSendTime = millis();
          Serial.printf("[AC] Auto-sent cutoff temp to ESP32C3: %.1f\n", acCutoffTemp);
        }
        break;
        
      case AC_BTN_SEND:
        // Button not used anymore, but kept for enum compatibility
        break;
        
      default:
        break;
    }
  }
}

ACButton ACScreen::checkButtonPress(uint16_t x, uint16_t y) {
  for (int i = 0; i < AC_BTN_COUNT; i++) {
    const ACButtonDef& btn = buttons[i];
    
    if (x >= btn.x && x <= (btn.x + btn.w) &&
        y >= btn.y && y <= (btn.y + btn.h)) {
      return btn.button;
    }
  }
  
  return (ACButton)-1;
}

void ACScreen::loadACCutoffTemp() {
  // Read from EEPROM
  uint8_t flag = EEPROM.read(EEPROM_AC_CUTOFF_FLAG_ADDR);
  
  if (flag == EEPROM_AC_CUTOFF_FLAG) {
    // Read temperature as uint16 (multiplied by 10)
    uint16_t tempInt = EEPROM.readUShort(EEPROM_AC_CUTOFF_TEMP_ADDR);
    acCutoffTemp = tempInt / 10.0;
    savedACCutoffTemp = acCutoffTemp;
    Serial.printf("[AC] Loaded cutoff temp from EEPROM: %.1f\n", acCutoffTemp);
  } else {
    // Use default
    acCutoffTemp = 18.0;
    savedACCutoffTemp = acCutoffTemp;
    Serial.println("[AC] Using default cutoff temp: 18.0°C");
  }
}

void ACScreen::saveACCutoffTemp() {
  // Save temperature as uint16 (multiplied by 10)
  uint16_t tempInt = (uint16_t)(acCutoffTemp * 10.0f);
  
  EEPROM.writeUShort(EEPROM_AC_CUTOFF_TEMP_ADDR, tempInt);
  EEPROM.write(EEPROM_AC_CUTOFF_FLAG_ADDR, EEPROM_AC_CUTOFF_FLAG);
  EEPROM.commit();
  
  savedACCutoffTemp = acCutoffTemp;
  Serial.printf("[AC] Saved cutoff temp to EEPROM: %.1f\n", acCutoffTemp);
}

void ACScreen::setACCutoffTemp(float temp) {
  acCutoffTemp = temp;
  tempChanged = true;
}

// Global functions
void loadACCutoffFromEEPROM() {
  uint8_t flag = EEPROM.read(EEPROM_AC_CUTOFF_FLAG_ADDR);
  
  if (flag == EEPROM_AC_CUTOFF_FLAG) {
    uint16_t tempInt = EEPROM.readUShort(EEPROM_AC_CUTOFF_TEMP_ADDR);
    savedACCutoffTemp = tempInt / 10.0;
  } else {
    savedACCutoffTemp = 18.0;
  }
}

void saveACCutoffToEEPROM(float temp) {
  uint16_t tempInt = (uint16_t)(temp * 10.0f);
  EEPROM.writeUShort(EEPROM_AC_CUTOFF_TEMP_ADDR, tempInt);
  EEPROM.write(EEPROM_AC_CUTOFF_FLAG_ADDR, EEPROM_AC_CUTOFF_FLAG);
  EEPROM.commit();
  savedACCutoffTemp = temp;
}

float getACCutoffTemp() {
  return savedACCutoffTemp;
}
