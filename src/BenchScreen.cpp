#include "BenchScreen.h"
#include "GlobalVariables.h"
#include "Config.h"
#include "Simulator.h"
#include "Roboto16.h"
#include "RobotoBold32.h"
#include "CANHandler.h"
#include <TFT_eSPI.h>
#include <esp32_can.h>

// Bench test constants (from RusEFI reference)
#define BENCH_HEADER 0x66
#define BENCH_IGN_SUBSYS 0x12
#define BENCH_INJ_SUBSYS 0x13
#define BENCH_ENGINE_SUBSYS 0x14
#define BENCH_ENGINE_STARTSTOP 0x09
#define RUSEFI_BENCH_CAN_ID 0x77000C  // ECU_CAN_BUS_USER_CONTROL

// External references
extern TFT_eSPI display;

// Static variable definitions
bool BenchScreen::initialized = false;
uint32_t BenchScreen::lastUpdate = 0;
bool BenchScreen::isTestRunning = false;
int BenchScreen::selectedButton = -1;
int BenchScreen::testMode = TEST_MODE_IDLE;
uint32_t BenchScreen::testStartTime = 0;
uint32_t BenchScreen::testDuration = 0;
int BenchScreen::selectedCylinder = 1;
bool BenchScreen::testResult = false;

// Global instance
BenchScreen benchScreen;

BenchScreen::BenchScreen() {
}

void BenchScreen::begin() {
    initialized = true;
    lastUpdate = 0;
    isTestRunning = false;
    selectedButton = -1;
    testMode = TEST_MODE_IDLE;
    testStartTime = 0;
    testDuration = 0;
    selectedCylinder = 1;
    testResult = false;
}

void BenchScreen::draw(bool forceRedraw) {
    if (forceRedraw) {
        lastUpdate = 0;
        // Don't clear screen to preserve background image
        // display.fillScreen(TFT_BLACK);
        
        // Clear only title area with black background
        display.fillRect(0, 0, display.width(), 30, TFT_BLACK);
        
        // Draw title
        display.loadFont(AA_FONT_LARGE);
        display.setTextColor(TFT_RED, TFT_BLACK);
        display.setTextDatum(TC_DATUM);
        display.drawString("BENCH TEST", display.width() / 2, 15);
    }
    
    // Draw ignition test buttons
    drawIgnitionButtons(forceRedraw);
    
    // Draw injector test buttons
    drawInjectorButtons(forceRedraw);
    
    // Draw engine control buttons
    drawEngineControlButtons(forceRedraw);
    
    // Draw screen indicator
    drawScreenIndicator(forceRedraw);
}

void BenchScreen::update() {
    // Reset selected button after short delay for visual feedback
    if (selectedButton != -1 && millis() - testStartTime > 300) {
        selectedButton = -1;
        draw(false);
    }
    
    // Update test duration if test is running
    if (isTestRunning && testStartTime > 0) {
        testDuration = millis() - testStartTime;
    }
    
    // Update with reduced frequency
    if (millis() - lastUpdate > 200) {
        draw(false);
        lastUpdate = millis();
    }
}

bool BenchScreen::handleTouch(uint16_t x, uint16_t y) {
#if ENABLE_DEBUG_MODE
    if (debugMode) {
        Serial.printf("[Bench] Touch at (%d,%d)\n", x, y);
    }
#endif
    
    // Check IGN buttons (IGN1-IGN8) - Row 1
    for (int i = 1; i <= 8; i++) {
        int btnX = 10 + (i-1) * 58;
        int btnY = 80;  // Moved down from 50
        if (x >= btnX && x <= (btnX + 55) && y >= btnY && y <= (btnY + 45)) {
            selectedButton = i; // IGN button ID (1-8)
            testStartTime = millis();
            
#if ENABLE_DEBUG_MODE
            if (debugMode) {
                Serial.printf("[Bench] IGN%d button pressed!\n", i);
            }
#endif
            
            // Show immediate visual feedback
            draw(true);
            
            // Send CAN command
            testResult = benchIgnition(i);
#if ENABLE_DEBUG_MODE
            if (debugMode) {
                Serial.printf("[Bench] IGN%d test: %s\n", i, testResult ? "OK" : "FAIL");
            }
#endif
            
            return true;
        }
    }
    
    // Check INJ buttons (INJ1-INJ8) - Row 2  
    for (int i = 1; i <= 8; i++) {
        int btnX = 10 + (i-1) * 58;
        int btnY = 150; // Moved down from 105
        if (x >= btnX && x <= (btnX + 55) && y >= btnY && y <= (btnY + 45)) {
            selectedButton = i + 10; // INJ button ID (11-18)
            testStartTime = millis();
            
#if ENABLE_DEBUG_MODE
            if (debugMode) {
                Serial.printf("[Bench] INJ%d button pressed!\n", i);
            }
#endif
            
            // Show immediate visual feedback
            draw(true);
            
            // Send CAN command
            testResult = benchInjector(i);
#if ENABLE_DEBUG_MODE
            if (debugMode) {
                Serial.printf("[Bench] INJ%d test: %s\n", i, testResult ? "OK" : "FAIL");
            }
#endif
            
            return true;
        }
    }
    
    // Check engine control buttons - Row 3
    struct EngineButton {
        uint16_t x, y, w, h;
        int id;
        const char* name;
    };
    
    EngineButton engineBtns[] = {
        {50, 220, 120, 50, 100, "START ENGINE"},  // Moved down from 180
        {200, 220, 120, 50, 101, "STOP ENGINE"},
        {350, 220, 100, 50, 102, "RESET"}
    };
    
    for (int i = 0; i < 3; i++) {
        if (x >= engineBtns[i].x && x <= (engineBtns[i].x + engineBtns[i].w) &&
            y >= engineBtns[i].y && y <= (engineBtns[i].y + engineBtns[i].h)) {
            
            selectedButton = engineBtns[i].id;
#if ENABLE_DEBUG_MODE
            if (debugMode) {
                Serial.printf("[Bench] Engine control button pressed: %s\n", engineBtns[i].name);
            }
#endif
            
            // Show immediate visual feedback
            draw(true);
            
            switch (engineBtns[i].id) {
                case 100: // Start Engine
                    testResult = startStopEngine();
#if ENABLE_DEBUG_MODE
                    if (debugMode) {
                        Serial.printf("[Bench] Engine start: %s\n", testResult ? "OK" : "FAIL");
                    }
#endif
                    break;
                case 101: // Stop Engine  
                    testResult = startStopEngine();
#if ENABLE_DEBUG_MODE
                    if (debugMode) {
                        Serial.printf("[Bench] Engine stop: %s\n", testResult ? "OK" : "FAIL");
                    }
#endif
                    break;
                case 102: // Reset
                    testResult = false;
#if ENABLE_DEBUG_MODE
                    if (debugMode) {
                        Serial.println("[Bench] System reset");
                    }
#endif
                    break;
            }
            
            return true;
        }
    }
    
    return false;
}

void BenchScreen::drawIgnitionButtons(bool forceRedraw) {
    if (forceRedraw) {
        // Clear IGN section area
        display.fillRect(0, 60, display.width(), 70, TFT_BLACK);
        
        // Draw IGN section header
        display.loadFont(AA_FONT_SMALL);
        display.setTextColor(TFT_ORANGE, TFT_BLACK);
        display.setTextDatum(TL_DATUM);
        display.drawString("IGNITION TEST:", 10, 65);
        
        // Draw IGN1-IGN8 buttons
        for (int i = 1; i <= 8; i++) {
            int x = 10 + (i-1) * 58;
            int y = 80;  // Moved down from 50
            bool isPressed = (selectedButton == i);
            uint16_t bgColor = isPressed ? TFT_YELLOW : TFT_DARKGREY;
            uint16_t textColor = isPressed ? TFT_BLACK : TFT_ORANGE;
            uint16_t borderColor = isPressed ? TFT_ORANGE : TFT_WHITE;
            
            // Add shadow effect for pressed state
            if (isPressed) {
                display.fillRect(x+2, y+2, 55, 45, TFT_DARKGREY);
            }
            
            display.fillRect(x, y, 55, 45, bgColor);
            display.drawRect(x, y, 55, 45, borderColor);
            
            display.loadFont(AA_FONT_SMALL);
            display.setTextColor(textColor, bgColor);
            display.setTextDatum(MC_DATUM);
            display.drawString("IGN" + String(i), x + 27, y + 22);
        }
    }
}

void BenchScreen::drawInjectorButtons(bool forceRedraw) {
    if (forceRedraw) {
        // Clear INJ section area
        display.fillRect(0, 130, display.width(), 70, TFT_BLACK);
        
        // Draw INJ section header
        display.loadFont(AA_FONT_SMALL);
        display.setTextColor(TFT_CYAN, TFT_BLACK);
        display.setTextDatum(TL_DATUM);
        display.drawString("INJECTOR TEST:", 10, 135);
        
        // Draw INJ1-INJ8 buttons
        for (int i = 1; i <= 8; i++) {
            int x = 10 + (i-1) * 58;
            int y = 150; // Moved down from 105
            bool isPressed = (selectedButton == (i + 10));
            uint16_t bgColor = isPressed ? TFT_BLUE : TFT_DARKGREY;
            uint16_t textColor = isPressed ? TFT_WHITE : TFT_CYAN;
            uint16_t borderColor = isPressed ? TFT_CYAN : TFT_WHITE;
            
            // Add shadow effect for pressed state
            if (isPressed) {
                display.fillRect(x+2, y+2, 55, 45, TFT_DARKGREY);
            }
            
            display.fillRect(x, y, 55, 45, bgColor);
            display.drawRect(x, y, 55, 45, borderColor);
            
            display.loadFont(AA_FONT_SMALL);
            display.setTextColor(textColor, bgColor);
            display.setTextDatum(MC_DATUM);
            display.drawString("INJ" + String(i), x + 27, y + 22);
        }
    }
}

void BenchScreen::drawEngineControlButtons(bool forceRedraw) {
    if (forceRedraw) {
        // Clear engine control section area
        display.fillRect(0, 200, display.width(), 100, TFT_BLACK);
        
        // Draw engine control section header
        display.loadFont(AA_FONT_SMALL);
        display.setTextColor(TFT_GREEN, TFT_BLACK);
        display.setTextDatum(TL_DATUM);
        display.drawString("ENGINE CONTROL:", 10, 205);
        
        // Start Engine button
        bool startPressed = (selectedButton == 100);
        uint16_t startBg = startPressed ? TFT_GREEN : TFT_DARKGREEN;
        uint16_t startText = startPressed ? TFT_BLACK : TFT_WHITE;
        if (startPressed) display.fillRect(52, 222, 120, 50, TFT_DARKGREY);
        display.fillRect(50, 220, 120, 50, startBg);  // Moved down from 180
        display.drawRect(50, 220, 120, 50, TFT_GREEN);
        display.loadFont(AA_FONT_SMALL);
        display.setTextColor(startText, startBg);
        display.setTextDatum(MC_DATUM);
        display.drawString("START", 110, 245);
        
        // Stop Engine button
        bool stopPressed = (selectedButton == 101);
        uint16_t stopBg = stopPressed ? TFT_RED : TFT_MAROON;
        uint16_t stopText = stopPressed ? TFT_WHITE : TFT_WHITE;
        if (stopPressed) display.fillRect(202, 222, 120, 50, TFT_DARKGREY);
        display.fillRect(200, 220, 120, 50, stopBg);
        display.drawRect(200, 220, 120, 50, TFT_RED);
        display.setTextColor(stopText, stopBg);
        display.drawString("STOP", 260, 245);
        
        // Reset button
        bool resetPressed = (selectedButton == 102);
        uint16_t resetBg = resetPressed ? TFT_YELLOW : TFT_OLIVE;
        uint16_t resetText = resetPressed ? TFT_BLACK : TFT_WHITE;
        if (resetPressed) display.fillRect(352, 222, 100, 50, TFT_DARKGREY);
        display.fillRect(350, 220, 100, 50, resetBg);
        display.drawRect(350, 220, 100, 50, TFT_YELLOW);
        display.setTextColor(resetText, resetBg);
        display.drawString("RESET", 400, 245);
        
        // Draw status line
        String status = testResult ? "LAST TEST: PASS" : "LAST TEST: FAIL";
        uint16_t statusColor = testResult ? TFT_GREEN : TFT_RED;
        display.setTextColor(statusColor, TFT_BLACK);
        display.setTextDatum(TC_DATUM);
        display.drawString(status, display.width() / 2, 290);
    }
}

void BenchScreen::drawScreenIndicator(bool forceRedraw) {
    if (forceRedraw) {
        display.loadFont(AA_FONT_SMALL);
        display.setTextColor(TFT_WHITE, TFT_BLACK);
        display.setTextDatum(BR_DATUM);
        display.drawString("BENCH", display.width() - 5, display.height() - 5);
        
        // Show CAN status
        String canStatus = isCANMode ? "CAN: ON" : "CAN: OFF";
        display.setTextColor(isCANMode ? TFT_GREEN : TFT_RED, TFT_BLACK);
        display.setTextDatum(BL_DATUM);
        display.drawString(canStatus, 5, display.height() - 5);
    }
}

// Bench test implementation functions
bool BenchScreen::benchIgnition(uint8_t cylinder) {
    if (!isCANMode || canProtocol != CAN_PROTOCOL_RUSEFI) {
#if ENABLE_DEBUG_MODE
        if (debugMode) {
            Serial.println("[Bench] Error: RusEFI CAN mode required for bench test");
        }
#endif
        return false;
    }
    
    // Prepare bench test data for ignition (RusEFI format)
    uint8_t data[] = { BENCH_HEADER, 0x00, BENCH_IGN_SUBSYS, 0x00, cylinder, 0x00, 0x00, 0x00 };
    
    CAN_FRAME frame;
    frame.id = RUSEFI_BENCH_CAN_ID;  // Use RusEFI bench test CAN ID
    frame.extended = false;  // Standard CAN ID, not extended
    frame.length = 8;
    memcpy(frame.data.byte, data, 8);
    
    bool result = CAN0.sendFrame(frame);
#if ENABLE_DEBUG_MODE
    if (debugMode) {
        Serial.printf("[Bench] IGN%d test sent: %s\n", cylinder, result ? "OK" : "FAIL");
    }
#endif
    return result;
}

bool BenchScreen::benchInjector(uint8_t cylinder) {
    if (!isCANMode || canProtocol != CAN_PROTOCOL_RUSEFI) {
#if ENABLE_DEBUG_MODE
        if (debugMode) {
            Serial.println("[Bench] Error: RusEFI CAN mode required for bench test");
        }
#endif
        return false;
    }
    
    // Prepare bench test data for injector (RusEFI format)
    uint8_t data[] = { BENCH_HEADER, 0x00, BENCH_INJ_SUBSYS, 0x00, cylinder, 0x00, 0x00, 0x00 };
    
    CAN_FRAME frame;
    frame.id = RUSEFI_BENCH_CAN_ID;  // Use RusEFI bench test CAN ID
    frame.extended = false;  // Standard CAN ID, not extended
    frame.length = 8;
    memcpy(frame.data.byte, data, 8);
    
    bool result = CAN0.sendFrame(frame);
#if ENABLE_DEBUG_MODE
    if (debugMode) {
        Serial.printf("[Bench] INJ%d test sent: %s\n", cylinder, result ? "OK" : "FAIL");
    }
#endif
    return result;
}

bool BenchScreen::startStopEngine() {
    if (!isCANMode || canProtocol != CAN_PROTOCOL_RUSEFI) {
#if ENABLE_DEBUG_MODE
        if (debugMode) {
            Serial.println("[Bench] Error: RusEFI CAN mode required for engine control");
        }
#endif
        return false;
    }
    
    // Prepare engine start/stop data (RusEFI format)
    uint8_t data[] = { BENCH_HEADER, 0x00, BENCH_ENGINE_SUBSYS, 0x00, BENCH_ENGINE_STARTSTOP, 0x00, 0x00, 0x00 };
    
    CAN_FRAME frame;
    frame.id = RUSEFI_BENCH_CAN_ID;  // Use RusEFI bench test CAN ID
    frame.extended = false;  // Standard CAN ID, not extended
    frame.length = 8;
    memcpy(frame.data.byte, data, 8);
    
    bool result = CAN0.sendFrame(frame);
#if ENABLE_DEBUG_MODE
    if (debugMode) {
        Serial.printf("[Bench] Engine start/stop sent: %s\n", result ? "OK" : "FAIL");
    }
#endif
    return result;
}