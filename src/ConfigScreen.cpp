#include "ConfigScreen.h"
#include "GlobalVariables.h"
#include "Config.h"
#include "DisplayConfig.h"
#include "BacklightControl.h"
#include "Roboto16.h"
#include "RobotoBold32.h"
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <EEPROM.h>

// External references
extern TFT_eSPI display;

// Static variable definitions
bool ConfigScreen::initialized = false;
uint32_t ConfigScreen::lastUpdate = 0;
bool ConfigScreen::isPressing = false;
uint32_t ConfigScreen::pressStartTime = 0;

// Cached values to avoid unnecessary redraws
static bool hasCachedState = false;
static int cachedCommMode = -1;
static int cachedCanProtocol = -1;
static int cachedSpeeduinoMode = -1;
static int cachedWiFiActive = -1;
static int cachedBrightness = -1;

// Global instance
ConfigScreen configScreen;

ConfigScreen::ConfigScreen() {
}

void ConfigScreen::begin() {
    initialized = true;
    lastUpdate = 0;
    isPressing = false;
    pressStartTime = 0;
    hasCachedState = false;
}

void ConfigScreen::draw(bool forceRedraw) {
    if (forceRedraw || !hasCachedState) {
        lastUpdate = 0;
        display.fillScreen(TFT_BLACK);

        drawHeader(true);
        drawCommModeSection(60);
        drawWiFiControlSection(130);
        drawBrightnessSection(200);
        drawWiFiInfo(true);
        drawScreenIndicator(true);

        cachedCommMode = (int)commMode;
        cachedCanProtocol = (int)canProtocol;
        cachedSpeeduinoMode = (int)speeduinoDataMode;
        cachedWiFiActive = wifiActive ? 1 : 0;
        cachedBrightness = (int)backlightBrightness;
        hasCachedState = true;
        return;
    }

    // Redraw only when related values changed
    if ((int)commMode != cachedCommMode ||
        (int)canProtocol != cachedCanProtocol ||
        (int)speeduinoDataMode != cachedSpeeduinoMode) {
        drawCommModeSection(60);
        cachedCommMode = (int)commMode;
        cachedCanProtocol = (int)canProtocol;
        cachedSpeeduinoMode = (int)speeduinoDataMode;
    }

    if ((wifiActive ? 1 : 0) != cachedWiFiActive) {
        drawWiFiControlSection(130);
        drawWiFiInfo(true);
        cachedWiFiActive = wifiActive ? 1 : 0;
    }

    if ((int)backlightBrightness != cachedBrightness) {
        drawBrightnessValueAndSlider(200);
        cachedBrightness = (int)backlightBrightness;
    }

    drawWiFiInfo(false);
}

void ConfigScreen::update() {
    // Avoid full periodic redraw to prevent flicker.
    // Only refresh dynamic WiFi info area.
    if (millis() - lastUpdate > 500) {
        drawWiFiInfo(false);
        lastUpdate = millis();
    }
}

bool ConfigScreen::handleTouch(uint16_t x, uint16_t y) {
    // Define touch areas for each section
    struct TouchArea {
        uint16_t x, y, w, h;
        int sectionId;
        const char* name;
    };
    
    TouchArea areas[] = {
        {0, 50, 480, 60, 0, "CommMode"},          // Communication Mode
        {0, 120, 480, 70, 1, "WiFiControl"},      // WiFi Control section
        {0, 190, 480, 60, 2, "Brightness"}        // Brightness slider
    };
    
    // Check if touch is in any valid area
    int touchedSection = -1;
    for (int i = 0; i < 3; i++) {
        if (x >= areas[i].x && x <= (areas[i].x + areas[i].w) &&
            y >= areas[i].y && y <= (areas[i].y + areas[i].h)) {
            touchedSection = areas[i].sectionId;
            break;
        }
    }
    
    if (touchedSection != -1) {
        uint32_t currentTime = millis();
        uint32_t timeSinceLastPress = currentTime - pressStartTime;
        
        // Simple tap-based interaction (300ms)
        if (timeSinceLastPress >= 300 || !isPressing) {
            isPressing = true;
            pressStartTime = currentTime;
            Serial.printf("[Config] Section touched: %s\n", areas[touchedSection].name);

            bool changed = false;
            // Handle different sections
            if (touchedSection == 0) {
                // Communication Mode - cycle through modes
                changed = handleCommModeChange();
            } else if (touchedSection == 1) {
                // WiFi Control - check which button was pressed
                changed = handleWiFiControlTouch(x, y);
            } else if (touchedSection == 2) {
                // Brightness - adjust based on x position (slider)
                changed = handleBrightnessSliderTouch(x);
            }

            if (changed) {
                draw(false);
                return true;
            }
        }
    } else {
        isPressing = false;
    }
    
    return false;
}

bool ConfigScreen::handleTouchRelease() {
    isPressing = false;
    return false;
}

bool ConfigScreen::handleCommModeChange() {
    static uint32_t lastToggle = 0;
    if (millis() - lastToggle < 1000) return false;
    lastToggle = millis();
    
    if (commMode == COMM_CAN) {
        if (canProtocol == CAN_PROTOCOL_HALTECH) {
            // Haltech 1M -> RusEFI 500K
            setCanProtocol(CAN_PROTOCOL_RUSEFI);
            setCanSpeed(500000);
            Serial.println("[Config] CAN RusEFI 500Kbps");
        } else {
            // RusEFI 500K -> Serial Mode A
            commMode = COMM_SERIAL;
            speeduinoDataMode = SPEEDUINO_MODE_A;
            EEPROM.write(EEPROM_COMM_MODE_ADDR, commMode);
            EEPROM.write(EEPROM_SPEEDUINO_MODE_ADDR, speeduinoDataMode);
            EEPROM.write(EEPROM_SPEEDUINO_FLAG_ADDR, 0xBB);
            EEPROM.commit();
            Serial.println("[Config] Serial Mode A");
        }
    } else {
        if (speeduinoDataMode == SPEEDUINO_MODE_A) {
            // Serial A -> Serial N
            speeduinoDataMode = SPEEDUINO_MODE_N;
            EEPROM.write(EEPROM_SPEEDUINO_MODE_ADDR, speeduinoDataMode);
            EEPROM.write(EEPROM_SPEEDUINO_FLAG_ADDR, 0xBB);
            EEPROM.commit();
            Serial.println("[Config] Serial Mode N");
        } else {
            // Serial N -> Haltech 1M
            commMode = COMM_CAN;
            setCanProtocol(CAN_PROTOCOL_HALTECH);
            setCanSpeed(1000000);
            EEPROM.write(EEPROM_COMM_MODE_ADDR, commMode);
            EEPROM.commit();
            Serial.println("[Config] CAN Haltech 1Mbps");
        }
    }

    return true;
}

bool ConfigScreen::handleWiFiControlTouch(uint16_t x, uint16_t y) {
    (void)y;
    // Three button zones: ON (90-170), OFF (170-250), AUTO (250-330)
    static uint32_t lastToggle = 0;
    if (millis() - lastToggle < 500) return false;
    lastToggle = millis();
    bool changed = false;
    
    if (x >= 90 && x < 170) {
        // WiFi ON button
        if (!wifiActive) {
            wifiActive = true;
            changed = true;
            Serial.println("[Config] WiFi ON");
        }
    } else if (x >= 170 && x < 250) {
        // WiFi OFF button
        if (wifiActive) {
            wifiActive = false;
            WiFi.mode(WIFI_OFF);
            changed = true;
            Serial.println("[Config] WiFi OFF");
        }
    } else if (x >= 250 && x < 330) {
        // WiFi AUTO button  
        wifiActive = true;
        WiFi.mode(WIFI_AP_STA);
        changed = true;
        Serial.println("[Config] WiFi AUTO");
    }

    return changed;
}

bool ConfigScreen::handleBrightnessSliderTouch(uint16_t x) {
    // Slider area: x from 60 to 420 (360px wide)
    // Map to brightness 1%-100% (3-255)
    static uint32_t lastAdjust = 0;
    if (millis() - lastAdjust < 30) return false;
    lastAdjust = millis();
    
    uint16_t sliderStart = 60;
    uint16_t sliderEnd = 420;
    
    if (x < sliderStart) x = sliderStart;
    if (x > sliderEnd) x = sliderEnd;
    
    // Calculate brightness based on position with minimum safe level (~1%)
    uint8_t newBrightness = (uint8_t)(3 + (((x - sliderStart) * (255 - 3)) / (sliderEnd - sliderStart)));
    if (newBrightness == backlightBrightness) {
        return false;
    }
    setBacklightBrightness(newBrightness);
    
    Serial.printf("[Config] Brightness: %d%% (%d/255)\n", 
                  (backlightBrightness * 100) / 255, backlightBrightness);

    return true;
}

void ConfigScreen::drawHeader(bool forceRedraw) {
    if (forceRedraw) {
        display.fillRect(0, 0, 480, 40, TFT_BLACK);
        display.loadFont(AA_FONT_LARGE);
        display.setTextColor(TFT_ORANGE, TFT_BLACK);
        display.setTextDatum(TC_DATUM);
        display.drawString("CONFIGURATION", 240, 10);
    }
}

void ConfigScreen::drawCommModeSection(int y) {
    // Draw section background
    uint16_t bgColor = TFT_ORANGE;
    display.fillRect(10, y, 460, 50, bgColor);
    display.drawRect(10, y, 460, 50, TFT_WHITE);
    
    // Draw label
    display.loadFont(AA_FONT_SMALL);
    display.setTextColor(TFT_WHITE, bgColor);
    display.setTextDatum(TL_DATUM);
    display.drawString("Communication", 20, y + 8);
    
    // Draw current mode
    String commInfo;
    if (commMode == COMM_CAN) {
        if (canProtocol == CAN_PROTOCOL_HALTECH) {
            commInfo = "CAN Haltech 1Mbps";
        } else {
            commInfo = "CAN RusEFI 500Kbps";
        }
    } else {
        String mode = (speeduinoDataMode == SPEEDUINO_MODE_A) ? "Mode A" : "Mode N";
        commInfo = "Serial " + mode;
    }
    
    display.setTextDatum(TL_DATUM);
    display.drawString(commInfo, 20, y + 28);
    
    // Draw "Tap to change" hint
    display.setTextDatum(BR_DATUM);
    display.drawString("Tap to change", 450, y + 40);
}

void ConfigScreen::drawWiFiControlSection(int y) {
    // Draw section background
    uint16_t bgColor = TFT_ORANGE;
    display.fillRect(10, y, 460, 60, bgColor);
    display.drawRect(10, y, 460, 60, TFT_WHITE);
    
    // Draw label
    display.loadFont(AA_FONT_SMALL);
    display.setTextColor(TFT_WHITE, bgColor);
    display.setTextDatum(TL_DATUM);
    display.drawString("WiFi Control", 20, y + 8);
    
    // Draw three buttons: ON, OFF, AUTO
    uint16_t btnY = y + 28;
    uint16_t btnH = 22;
    
    // Button 1: ON (X: 90-170)
    uint16_t onBg = (wifiActive) ? TFT_WHITE : TFT_ORANGE;
    display.fillRect(90, btnY, 80, btnH, onBg);
    display.drawRect(90, btnY, 80, btnH, TFT_WHITE);
    display.setTextColor(wifiActive ? TFT_BLACK : TFT_WHITE, onBg);
    display.setTextDatum(MC_DATUM);
    display.drawString("ON", 130, btnY + 11);
    
    // Button 2: OFF (X: 170-250)
    uint16_t offBg = (!wifiActive) ? TFT_WHITE : TFT_ORANGE;
    display.fillRect(170, btnY, 80, btnH, offBg);
    display.drawRect(170, btnY, 80, btnH, TFT_WHITE);
    display.setTextColor(!wifiActive ? TFT_BLACK : TFT_WHITE, offBg);
    display.drawString("OFF", 210, btnY + 11);
    
    // Button 3: AUTO (X: 250-330)
    uint16_t autoBg = TFT_ORANGE;
    display.fillRect(250, btnY, 80, btnH, autoBg);
    display.drawRect(250, btnY, 80, btnH, TFT_WHITE);
    display.setTextColor(TFT_WHITE, autoBg);
    display.drawString("AUTO", 290, btnY + 11);
}

void ConfigScreen::drawBrightnessSection(int y) {
    // Draw section background
    uint16_t bgColor = TFT_ORANGE;
    display.fillRect(10, y, 460, 50, bgColor);
    display.drawRect(10, y, 460, 50, TFT_WHITE);
    
    // Draw label
    display.loadFont(AA_FONT_SMALL);
    display.setTextColor(TFT_WHITE, bgColor);
    display.setTextDatum(TL_DATUM);
    display.drawString("Brightness", 20, y + 8);

    drawBrightnessValueAndSlider(y);
}

void ConfigScreen::drawBrightnessValueAndSlider(int y) {
    // Update only the dynamic brightness sub-area to reduce flicker
    display.fillRect(300, y + 4, 150, 16, TFT_ORANGE);

    String brightnessText = String((backlightBrightness * 100) / 255) + "%";
    display.loadFont(AA_FONT_SMALL);
    display.setTextColor(TFT_WHITE, TFT_ORANGE);
    display.setTextDatum(TR_DATUM);
    display.drawString(brightnessText, 440, y + 8);

    uint16_t sliderY = y + 28;
    uint16_t sliderH = 6;

    // Clear previous thumb/line artifacts then redraw slider only
    display.fillRect(56, sliderY - 2, 368, sliderH + 6, TFT_ORANGE);
    display.drawRect(60, sliderY, 360, sliderH, TFT_LIGHTGREY);

    uint16_t fillWidth = (360 * backlightBrightness) / 255;
    display.fillRect(60, sliderY, fillWidth, sliderH, TFT_WHITE);

    uint16_t thumbX = 60 + fillWidth;
    display.fillCircle(thumbX, sliderY + 3, 4, TFT_WHITE);
}

void ConfigScreen::drawWiFiInfo(bool forceRedraw) {
    static String lastWiFiInfo = "";
    static uint32_t lastUpdate = 0;
    
    if (forceRedraw || (millis() - lastUpdate > 2000)) {
        String wifiInfo;
        uint16_t infoColor;
        
        if (wifiActive && WiFi.status() == WL_CONNECTED) {
            // Station mode - show STA IP
            wifiInfo = "WiFi STA: " + WiFi.localIP().toString();
            infoColor = TFT_GREEN;
        } else if (wifiActive) {
            // AP mode
            wifiInfo = "WiFi AP: " + WiFi.softAPIP().toString();
            infoColor = TFT_ORANGE;
        } else {
            // WiFi off
            wifiInfo = "WiFi: OFF";
            infoColor = TFT_RED;
        }
        
        if (forceRedraw || wifiInfo != lastWiFiInfo) {
            // Draw new info
            display.loadFont(AA_FONT_SMALL);
            display.setTextColor(infoColor, TFT_BLACK);
            display.setTextDatum(TL_DATUM);
            display.setTextPadding(460);
            display.drawString(wifiInfo, 10, 285);
            display.setTextPadding(0);
            
            lastWiFiInfo = wifiInfo;
        }
        
        lastUpdate = millis();
    }
}

void ConfigScreen::drawScreenIndicator(bool forceRedraw) {
    if (forceRedraw) {
        display.loadFont(AA_FONT_SMALL);
        display.setTextColor(TFT_WHITE, TFT_BLACK);
        display.setTextDatum(BR_DATUM);
        display.drawString("CONFIG", 475, 315);
    }
}



