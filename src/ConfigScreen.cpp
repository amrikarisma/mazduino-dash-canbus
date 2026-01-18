#include "ConfigScreen.h"
#include "GlobalVariables.h"
#include "Config.h"
#include "DisplayConfig.h"
#include "BacklightControl.h"
#include "GPSHandler.h"
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
int ConfigScreen::selectedSection = -1;
bool ConfigScreen::isPressing = false;
uint32_t ConfigScreen::pressStartTime = 0;
int ConfigScreen::pressedSection = -1;
uint16_t ConfigScreen::pressX = 0;
uint16_t ConfigScreen::pressY = 0;

// Global instance
ConfigScreen configScreen;

ConfigScreen::ConfigScreen() {
}

void ConfigScreen::begin() {
    initialized = true;
    lastUpdate = 0;
    selectedSection = -1;
    isPressing = false;
    pressStartTime = 0;
    pressedSection = -1;
    pressX = 0;
    pressY = 0;
}

void ConfigScreen::draw(bool forceRedraw) {
    if (forceRedraw) {
        lastUpdate = 0;
    }
    
    // Draw system information at top
    drawSystemInfo(forceRedraw);
    
    // Draw WiFi status
    drawWiFiStatus(forceRedraw);
    
    // Draw GPS status
    drawGPSStatus(forceRedraw);
    
    // Draw configuration options
    drawConfigOptions(forceRedraw);
    
    // Draw press and hold progress if active
    if (isPressing && pressedSection != -1) {
        drawPressProgress();
    }
    
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
    // Define touch areas for each section (right side only for values)
    struct TouchArea {
        uint16_t x, y, w, h;
        int sectionId;
        const char* name;
    };
    
    TouchArea areas[] = {
        {170, 50, 300, 30, 0, "Status"},
        {170, 85, 300, 30, 1, "CommMode"},
        {170, 120, 300, 30, 2, "WiFi"},
        {170, 155, 300, 30, 3, "GPS"},
        {170, 190, 300, 30, 4, "Display"},
        {170, 225, 300, 30, 5, "Brightness"},
        {170, 260, 300, 30, 6, "Debug"},
        {170, 295, 300, 30, 7, "Info"}
    };
    
    // Check if touch is in any valid area
    int touchedSection = -1;
    for (int i = 0; i < 8; i++) {
        if (x >= areas[i].x && x <= (areas[i].x + areas[i].w) &&
            y >= areas[i].y && y <= (areas[i].y + areas[i].h)) {
            touchedSection = areas[i].sectionId;
            break;
        }
    }
    
    if (touchedSection != -1) {
        // Only allow press and hold for configurable sections
        if (touchedSection == 1 || touchedSection == 3 || touchedSection == 5 || touchedSection == 6) { // CommMode, GPS, Brightness, Debug
            if (!isPressing) {
                // Start press and hold
                isPressing = true;
                pressStartTime = millis();
                pressedSection = touchedSection;
                pressX = x;
                pressY = y;
                selectedSection = touchedSection;
                
                Serial.printf("[Config] Press started for section: %s\n", areas[touchedSection].name);
                draw(true); // Show selection
                return true;
            } else if (isPressing && pressedSection == touchedSection) {
                uint32_t elapsed = millis() - pressStartTime;
                
                // Only start showing hold progress after 500ms to confirm it's not a quick tap
                if (elapsed >= 500) {
                    // Check if 3 seconds elapsed (from start, not from 500ms mark)
                    if (elapsed >= 3000) {
                        // 3 seconds elapsed, execute the change
                        Serial.printf("[Config] 3-second hold completed for section: %s\n", areas[touchedSection].name);
                        handleSectionTouch(touchedSection);
                        
                        // Reset press state
                        isPressing = false;
                        pressStartTime = 0;
                        pressedSection = -1;
                        
                        draw(true);
                        return true;
                    }
                    // Still holding after 500ms, show progress
                    return true;
                }
                // Still in initial 500ms, just continue without progress display
                return true;
            }
        } else {
            // For non-configurable sections, just show selection
            selectedSection = touchedSection;
            draw(true);
            return true;
        }
    } else {
        // Touch outside valid areas - reset press state
        if (isPressing) {
            Serial.println("[Config] Press cancelled - touch moved outside area");
            isPressing = false;
            pressStartTime = 0;
            pressedSection = -1;
            selectedSection = -1;
            draw(true);
        }
    }
    
    return false;
}

bool ConfigScreen::handleTouchRelease() {
    if (isPressing) {
        uint32_t elapsed = millis() - pressStartTime;
        
        // If release happens before 500ms, it's considered a quick tap, not a hold attempt
        if (elapsed < 500) {
            Serial.printf("[Config] Quick tap detected (%dms) - ignoring\n", elapsed);
        } else {
            Serial.printf("[Config] Hold released early (%dms) - cancelled\n", elapsed);
        }
        
        // Reset press state
        isPressing = false;
        pressStartTime = 0;
        pressedSection = -1;
        selectedSection = -1;
        draw(true);
        return true;
    }
    return false;
}

void ConfigScreen::handleSectionTouch(int sectionId) {
    switch (sectionId) {
        case 1: // Communication Mode
            toggleCommMode();
            break;
        case 3: // GPS Mode
            toggleGPSMode();
            break;
        case 5: // Brightness (shifted due to GPS addition)
            adjustBrightness();
            break;
        case 6: // Debug Mode (shifted due to GPS addition)
            toggleDebugMode();
            break;
        default:
            // Other sections don't have toggle behavior yet
            break;
    }
}

void ConfigScreen::toggleCommMode() {
    static uint32_t lastToggle = 0;
    if (millis() - lastToggle < 1000) return; // Debounce
    lastToggle = millis();
    
    if (commMode == COMM_CAN) {
        // CAN mode - cycle through: Haltech 1M -> RusEFI 500K -> Serial A -> Serial N
        if (canProtocol == CAN_PROTOCOL_HALTECH) {
            // Switch from Haltech 1M to RusEFI 500K
            setCanProtocol(CAN_PROTOCOL_RUSEFI);
            setCanSpeed(500000);
            Serial.println("[Config] CAN RusEFI 500Kbps");
        } else {
            // Switch from RusEFI 500K to Serial Mode A
            commMode = COMM_SERIAL;
            speeduinoDataMode = SPEEDUINO_MODE_A;
            EEPROM.write(EEPROM_COMM_MODE_ADDR, commMode);
            EEPROM.write(EEPROM_SPEEDUINO_MODE_ADDR, speeduinoDataMode);
            EEPROM.write(EEPROM_SPEEDUINO_FLAG_ADDR, 0xBB);
            EEPROM.commit();
            Serial.println("[Config] Serial Mode A");
        }
    } else {
        // Serial mode - cycle between Mode A and Mode N, then back to CAN
        if (speeduinoDataMode == SPEEDUINO_MODE_A) {
            // Switch from Serial A to Serial N
            speeduinoDataMode = SPEEDUINO_MODE_N;
            EEPROM.write(EEPROM_SPEEDUINO_MODE_ADDR, speeduinoDataMode);
            EEPROM.write(EEPROM_SPEEDUINO_FLAG_ADDR, 0xBB);
            EEPROM.commit();
            Serial.println("[Config] Serial Mode N");
        } else {
            // Switch from Serial N to Haltech 1M
            commMode = COMM_CAN;
            setCanProtocol(CAN_PROTOCOL_HALTECH);
            setCanSpeed(1000000);
            EEPROM.write(EEPROM_COMM_MODE_ADDR, commMode);
            EEPROM.commit();
            Serial.println("[Config] CAN Haltech 1Mbps");
        }
    }
}

void ConfigScreen::adjustBrightness() {
    static uint32_t lastAdjust = 0;
    if (millis() - lastAdjust < 200) return; // Debounce
    lastAdjust = millis();
    
    // Cycle through brightness levels: 25%, 50%, 75%, 100%
    if (backlightBrightness < 64) {
        backlightBrightness = 128; // 50%
    } else if (backlightBrightness < 128) {
        backlightBrightness = 192; // 75%
    } else if (backlightBrightness < 192) {
        backlightBrightness = 255; // 100%
    } else {
        backlightBrightness = 64; // 25%
    }
    
    // Apply brightness and save to EEPROM using the proper function
    setBacklightBrightness(backlightBrightness);
    
    Serial.printf("[Config] Brightness set to %d%% (%d/255)\n", 
                  (backlightBrightness * 100) / 255, backlightBrightness);
}

void ConfigScreen::toggleDebugMode() {
    static uint32_t lastToggle = 0;
    if (millis() - lastToggle < 500) return; // Debounce
    lastToggle = millis();
    
    debugMode = !debugMode;
    Serial.printf("[Config] Debug mode %s\n", debugMode ? "ON" : "OFF");
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
            // Clear WiFi status area (moved up to make room for GPS)
            display.fillRect(10, 330, display.width() - 20, 25, TFT_BLACK);
            
            // Draw WiFi status
            display.loadFont(AA_FONT_SMALL);
            display.setTextColor(statusColor, TFT_BLACK);
            display.setTextDatum(TL_DATUM);
            display.drawString(wifiStatus, 10, 335);
            
            lastWiFiStatus = wifiStatus;
        }
        
        lastWiFiUpdate = millis();
    }
}

void ConfigScreen::drawConfigOptions(bool forceRedraw) {
    if (forceRedraw) {
        // Draw configuration sections
        drawStatusSection(50, selectedSection == 0);
        drawCommModeSection(85, selectedSection == 1);
        drawWiFiSection(120, selectedSection == 2);
        drawGPSSection(155, selectedSection == 3);
        drawDisplaySection(190, selectedSection == 4);
        drawBrightnessSection(225, selectedSection == 5);
        drawDebugSection(260, selectedSection == 6);
        drawInfoSection(295, selectedSection == 7);
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
    
    // Draw text
    display.loadFont(AA_FONT_SMALL);
    display.setTextColor(textColor, bgColor);
    display.setTextDatum(TL_DATUM);
    display.drawString("System Status", 20, y + 8);
    
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
    display.drawString("WiFi Settings", 20, y + 8);
    
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
    display.drawString("Display Options", 20, y + 8);
    
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
    display.drawString("Brightness", 20, y + 8);
    
    String brightness = String((backlightBrightness * 100) / 255) + "%";
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
    display.drawString("Debug Mode", 20, y + 8);
    
    String debugStatus = debugMode ? "ON" : "OFF";
    display.setTextDatum(TR_DATUM);
    display.drawString(debugStatus, 460, y + 8);
}

void ConfigScreen::drawCommModeSection(int y, bool selected) {
    uint16_t bgColor = selected ? TFT_BLUE : TFT_DARKGREY;
    uint16_t textColor = selected ? TFT_WHITE : TFT_LIGHTGREY;
    
    display.fillRect(10, y, 460, 30, bgColor);
    display.drawRect(10, y, 460, 30, TFT_WHITE);
    
    display.loadFont(AA_FONT_SMALL);
    display.setTextColor(textColor, bgColor);
    display.setTextDatum(TL_DATUM);
    display.drawString("Communication", 20, y + 8);
    
    // Show current communication mode and protocol/speed
    String commInfo;
    if (commMode == COMM_CAN) {
        if (canProtocol == CAN_PROTOCOL_HALTECH) {
            commInfo = "CAN Haltech 1M";
        } else {
            commInfo = "CAN RusEFI 500K";
        }
    } else {
        String mode = (speeduinoDataMode == SPEEDUINO_MODE_A) ? "Mode A" : "Mode N";
        commInfo = "Serial " + mode;
    }
    
    display.setTextDatum(TR_DATUM);
    display.drawString(commInfo, 460, y + 8);
}

void ConfigScreen::drawInfoSection(int y, bool selected) {
    uint16_t bgColor = selected ? TFT_BLUE : TFT_DARKGREY;
    uint16_t textColor = selected ? TFT_WHITE : TFT_LIGHTGREY;
    
    display.fillRect(10, y, 460, 30, bgColor);
    display.drawRect(10, y, 460, 30, TFT_WHITE);
    
    display.loadFont(AA_FONT_SMALL);
    display.setTextColor(textColor, bgColor);
    display.setTextDatum(TL_DATUM);
    display.drawString("System Info", 20, y + 8);
    
    String freeHeap = String(ESP.getFreeHeap() / 1024) + "KB";
    display.setTextDatum(TR_DATUM);
    display.drawString(freeHeap, 460, y + 8);
}

void ConfigScreen::drawPressProgress() {
    if (!isPressing || pressedSection == -1) return;
    
    uint32_t elapsed = millis() - pressStartTime;
    
    // Only show progress after 500ms to confirm it's not a quick tap
    if (elapsed < 500) return;
    
    // Calculate progress from the 500ms mark (0-100% over remaining 2.5 seconds)
    uint8_t progress = (elapsed >= 3000) ? 100 : (uint8_t)((elapsed - 500) * 100 / 2500); // 2500ms = 100%
    
    // Draw progress bar at bottom of pressed section
    int sectionY = 50 + (pressedSection * 35);
    int progressY = sectionY + 25;
    
    // Clear progress bar area
    display.fillRect(10, progressY, 460, 3, TFT_BLACK);
    
    // Draw progress bar background
    display.drawRect(10, progressY, 460, 3, TFT_DARKGREY);
    
    // Draw progress fill
    int progressWidth = (460 * progress) / 100;
    uint16_t progressColor = (progress < 100) ? TFT_YELLOW : TFT_GREEN;
    display.fillRect(11, progressY + 1, progressWidth, 1, progressColor);
    
    // Draw progress text
    display.loadFont(AA_FONT_SMALL);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setTextDatum(TC_DATUM);
    
    uint32_t remainingMs = (3000 > elapsed) ? (3000 - elapsed) : 0;
    String progressText = "Hold " + String((remainingMs + 999) / 1000) + "s"; // Round up
    if (progress >= 100) {
        progressText = "ACTIVATED!";
    }
    
    display.drawString(progressText, 240, progressY + 8);
}

void ConfigScreen::drawGPSStatus(bool forceRedraw) {
    static String lastGPSStatus = "";
    static uint32_t lastGPSUpdate = 0;
    
    // Update GPS status every 2 seconds or on force redraw
    if (forceRedraw || (millis() - lastGPSUpdate > 2000)) {
        String gpsStatus;
        uint16_t statusColor;
        
        if (gpsEnabled) {
            if (gpsDataValid) {
                gpsStatus = "GPS: Connected (" + String(gpsNumSats) + " sats, " + String(gpsSpeed, 1) + " km/h)";
                statusColor = TFT_GREEN;
            } else {
                gpsStatus = "GPS: Enabled (No valid data)";
                statusColor = TFT_ORANGE;
            }
        } else {
            gpsStatus = "GPS: Disabled (GT-U7 on GPIO25/26)";
            statusColor = TFT_RED;
        }
        
        if (forceRedraw || gpsStatus != lastGPSStatus) {
            // Clear GPS status area (positioned between WiFi and bottom)
            display.fillRect(10, 275, display.width() - 20, 25, TFT_BLACK);
            
            // Draw GPS status
            display.loadFont(AA_FONT_SMALL);
            display.setTextColor(statusColor, TFT_BLACK);
            display.setTextDatum(TL_DATUM);
            display.drawString(gpsStatus, 10, 280);
            
            lastGPSStatus = gpsStatus;
        }
        
        lastGPSUpdate = millis();
    }
}

void ConfigScreen::drawGPSSection(int y, bool selected) {
    uint16_t bgColor = selected ? TFT_BLUE : TFT_DARKGREY;
    uint16_t textColor = selected ? TFT_WHITE : TFT_LIGHTGREY;
    
    display.fillRect(10, y, 460, 30, bgColor);
    display.drawRect(10, y, 460, 30, TFT_WHITE);
    
    display.loadFont(AA_FONT_SMALL);
    display.setTextColor(textColor, bgColor);
    display.setTextDatum(TL_DATUM);
    display.drawString("GPS Module (GT-U7)", 20, y + 8);
    
    // Show GPS status
    String gpsInfo;
    if (gpsEnabled) {
        if (gpsDataValid) {
            gpsInfo = "ON (" + String(gpsNumSats) + " sats)";
        } else {
            gpsInfo = "ON (No data)";
        }
    } else {
        gpsInfo = "OFF";
    }
    
    display.setTextDatum(TR_DATUM);
    display.drawString(gpsInfo, 460, y + 8);
}

void ConfigScreen::toggleGPSMode() {
    static uint32_t lastToggle = 0;
    if (millis() - lastToggle < 1000) return; // Debounce
    lastToggle = millis();
    
    gpsEnabled = !gpsEnabled;
    gpsHandler.enableGPS(gpsEnabled);
    
    Serial.printf("[Config] GPS %s\n", gpsEnabled ? "Enabled" : "Disabled");
    
    // Force redraw to show updated status
    draw(true);
}