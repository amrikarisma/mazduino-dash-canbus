#include "ConfigScreen.h"
#include "GlobalVariables.h"
#include "Config.h"
#include "NotoSansBold15.h"
#include "NotoSansBold36.h"
#include <TFT_eSPI.h>
#include <WiFi.h>

// External references
extern TFT_eSPI display;

// Static variable definitions
bool ConfigScreen::initialized = false;
uint32_t ConfigScreen::lastUpdate = 0;
int ConfigScreen::selectedSection = -1;

// Global instance
ConfigScreen configScreen;

ConfigScreen::ConfigScreen() {
}

void ConfigScreen::begin() {
    initialized = true;
    lastUpdate = 0;
    selectedSection = -1;
}

void ConfigScreen::draw(bool forceRedraw) {
    if (forceRedraw) {
        lastUpdate = 0;
    }
    
    // Draw system information at top
    drawSystemInfo(forceRedraw);
    
    // Draw WiFi status
    drawWiFiStatus(forceRedraw);
    
    // Draw configuration options
    drawConfigOptions(forceRedraw);
    
    // Draw screen indicator
    drawScreenIndicator(forceRedraw);
}

void ConfigScreen::update() {
    // Update with reduced frequency
    if (millis() - lastUpdate > 500) {
        draw(false);
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
        {10, 50, 460, 30, 0, "Status"},
        {10, 85, 460, 30, 1, "WiFi"},
        {10, 120, 460, 30, 2, "Display"},
        {10, 155, 460, 30, 3, "Brightness"},
        {10, 190, 460, 30, 4, "Debug"},
        {10, 225, 460, 30, 5, "Info"}
    };
    
    for (int i = 0; i < 6; i++) {
        if (x >= areas[i].x && x <= (areas[i].x + areas[i].w) &&
            y >= areas[i].y && y <= (areas[i].y + areas[i].h)) {
            
            selectedSection = areas[i].sectionId;
            Serial.printf("[Config] Selected section: %s\\n", areas[i].name);
            
            // Force redraw to show selection
            draw(true);
            return true;
        }
    }
    
    return false;
}

void ConfigScreen::drawSystemInfo(bool forceRedraw) {
    static String lastVersionInfo = "";
    String currentVersionInfo = "Version: " + String(version);
    
    if (forceRedraw || currentVersionInfo != lastVersionInfo) {
        // Clear header area
        display.fillRect(0, 5, display.width(), 35, TFT_BLACK);
        
        // Draw title
        display.loadFont(AA_FONT_LARGE);
        display.setTextColor(TFT_CYAN, TFT_BLACK);
        display.setTextDatum(TC_DATUM);
        display.drawString("CONFIGURATION", display.width() / 2, 8);
        
        // Draw version info
        display.loadFont(AA_FONT_SMALL);
        display.setTextColor(TFT_WHITE, TFT_BLACK);
        display.setTextDatum(TC_DATUM);
        display.drawString(currentVersionInfo, display.width() / 2, 28);
        
        lastVersionInfo = currentVersionInfo;
    }
}

void ConfigScreen::drawWiFiStatus(bool forceRedraw) {
    static String lastWiFiStatus = "";
    static uint32_t lastWiFiUpdate = 0;
    
    // Update WiFi status every 2 seconds or on force redraw
    if (forceRedraw || (millis() - lastWiFiUpdate > 2000)) {
        String wifiStatus;
        uint16_t statusColor;
        
        if (WiFi.status() == WL_CONNECTED) {
            wifiStatus = "WiFi: Connected (" + WiFi.localIP().toString() + ")";
            statusColor = TFT_GREEN;
        } else {
            wifiStatus = "WiFi: AP Mode (" + String(ssid) + ")";
            statusColor = TFT_ORANGE;
        }
        
        if (forceRedraw || wifiStatus != lastWiFiStatus) {
            // Clear WiFi status area
            display.fillRect(10, 260, display.width() - 20, 25, TFT_BLACK);
            
            // Draw WiFi status
            display.loadFont(AA_FONT_SMALL);
            display.setTextColor(statusColor, TFT_BLACK);
            display.setTextDatum(TL_DATUM);
            display.drawString(wifiStatus, 10, 265);
            
            lastWiFiStatus = wifiStatus;
        }
        
        lastWiFiUpdate = millis();
    }
}

void ConfigScreen::drawConfigOptions(bool forceRedraw) {
    if (forceRedraw) {
        // Draw configuration sections
        drawStatusSection(50, selectedSection == 0);
        drawWiFiSection(85, selectedSection == 1);
        drawDisplaySection(120, selectedSection == 2);
        drawBrightnessSection(155, selectedSection == 3);
        drawDebugSection(190, selectedSection == 4);
        drawInfoSection(225, selectedSection == 5);
    }
}

void ConfigScreen::drawScreenIndicator(bool forceRedraw) {
    if (forceRedraw) {
        display.loadFont(AA_FONT_SMALL);
        display.setTextColor(TFT_WHITE, TFT_BLACK);
        display.setTextDatum(BR_DATUM);
        display.drawString("CONFIG", display.width() - 5, display.height() - 5);
    }
}

void ConfigScreen::drawStatusSection(int y, bool selected) {
    uint16_t bgColor = selected ? TFT_BLUE : TFT_DARKGREY;
    uint16_t textColor = selected ? TFT_WHITE : TFT_LIGHTGREY;
    
    // Draw background
    display.fillRect(10, y, 460, 30, bgColor);
    display.drawRect(10, y, 460, 30, TFT_WHITE);
    
    // Draw icon and text
    display.loadFont(AA_FONT_SMALL);
    display.setTextColor(textColor, bgColor);
    display.setTextDatum(TL_DATUM);
    display.drawString("📊 System Status", 20, y + 8);
    
    // Show current communication mode
    String commMode = isCANMode ? "CAN Active" : "Serial Active";
    display.setTextDatum(TR_DATUM);
    display.drawString(commMode, 460, y + 8);
}

void ConfigScreen::drawWiFiSection(int y, bool selected) {
    uint16_t bgColor = selected ? TFT_BLUE : TFT_DARKGREY;
    uint16_t textColor = selected ? TFT_WHITE : TFT_LIGHTGREY;
    
    display.fillRect(10, y, 460, 30, bgColor);
    display.drawRect(10, y, 460, 30, TFT_WHITE);
    
    display.loadFont(AA_FONT_SMALL);
    display.setTextColor(textColor, bgColor);
    display.setTextDatum(TL_DATUM);
    display.drawString("📶 WiFi Settings", 20, y + 8);
    
    String wifiMode = (WiFi.status() == WL_CONNECTED) ? "STA Mode" : "AP Mode";
    display.setTextDatum(TR_DATUM);
    display.drawString(wifiMode, 460, y + 8);
}

void ConfigScreen::drawDisplaySection(int y, bool selected) {
    uint16_t bgColor = selected ? TFT_BLUE : TFT_DARKGREY;
    uint16_t textColor = selected ? TFT_WHITE : TFT_LIGHTGREY;
    
    display.fillRect(10, y, 460, 30, bgColor);
    display.drawRect(10, y, 460, 30, TFT_WHITE);
    
    display.loadFont(AA_FONT_SMALL);
    display.setTextColor(textColor, bgColor);
    display.setTextDatum(TL_DATUM);
    display.drawString("🖥️ Display Options", 20, y + 8);
    
    display.setTextDatum(TR_DATUM);
    display.drawString("Theme & Layout", 460, y + 8);
}

void ConfigScreen::drawBrightnessSection(int y, bool selected) {
    uint16_t bgColor = selected ? TFT_BLUE : TFT_DARKGREY;
    uint16_t textColor = selected ? TFT_WHITE : TFT_LIGHTGREY;
    
    display.fillRect(10, y, 460, 30, bgColor);
    display.drawRect(10, y, 460, 30, TFT_WHITE);
    
    display.loadFont(AA_FONT_SMALL);
    display.setTextColor(textColor, bgColor);
    display.setTextDatum(TL_DATUM);
    display.drawString("💡 Brightness", 20, y + 8);
    
    String brightness = String(backlightBrightness) + "%";
    display.setTextDatum(TR_DATUM);
    display.drawString(brightness, 460, y + 8);
}

void ConfigScreen::drawDebugSection(int y, bool selected) {
    uint16_t bgColor = selected ? TFT_BLUE : TFT_DARKGREY;
    uint16_t textColor = selected ? TFT_WHITE : TFT_LIGHTGREY;
    
    display.fillRect(10, y, 460, 30, bgColor);
    display.drawRect(10, y, 460, 30, TFT_WHITE);
    
    display.loadFont(AA_FONT_SMALL);
    display.setTextColor(textColor, bgColor);
    display.setTextDatum(TL_DATUM);
    display.drawString("🔧 Debug Mode", 20, y + 8);
    
    String debugStatus = debugMode ? "ON" : "OFF";
    display.setTextDatum(TR_DATUM);
    display.drawString(debugStatus, 460, y + 8);
}

void ConfigScreen::drawInfoSection(int y, bool selected) {
    uint16_t bgColor = selected ? TFT_BLUE : TFT_DARKGREY;
    uint16_t textColor = selected ? TFT_WHITE : TFT_LIGHTGREY;
    
    display.fillRect(10, y, 460, 30, bgColor);
    display.drawRect(10, y, 460, 30, TFT_WHITE);
    
    display.loadFont(AA_FONT_SMALL);
    display.setTextColor(textColor, bgColor);
    display.setTextDatum(TL_DATUM);
    display.drawString("ℹ️ System Info", 20, y + 8);
    
    String freeHeap = String(ESP.getFreeHeap() / 1024) + "KB";
    display.setTextDatum(TR_DATUM);
    display.drawString(freeHeap, 460, y + 8);
}