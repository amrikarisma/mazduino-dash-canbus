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
        display.setTextColor(TFT_ORANGE, TFT_BLACK);
        display.setTextDatum(TC_DATUM);
        display.drawString("BENCH TEST", display.width() / 2, 15);
    }
    
    // Draw ignition test buttons
    drawIgnitionButtons(forceRedraw);
    
    // Draw injector test buttons
    drawInjectorButtons(forceRedraw);
    
    // Draw engine control buttons
    drawFanAndFuelPumpButtons(forceRedraw);
    
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
    
    // Check IGN buttons (IGN1-IGN8) - Row 1 (adjusted for 10px margin)
    for (int i = 1; i <= 8; i++) {
        int btnX = 15 + (i-1) * 55;  // Start at 15px with smaller spacing to fit in margin
        int btnY = 85;  // Moved down from 80
        if (x >= btnX && x <= (btnX + 50) && y >= btnY && y <= (btnY + 45)) {
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
    
    // Check INJ buttons (INJ1-INJ8) - Row 2 (adjusted for 10px margin)
    for (int i = 1; i <= 8; i++) {
        int btnX = 15 + (i-1) * 55;  // Start at 15px with smaller spacing to fit in margin
        int btnY = 155; // Moved down from 150
        if (x >= btnX && x <= (btnX + 50) && y >= btnY && y <= (btnY + 45)) {
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
    
    // Check FAN and Fuelpump buttons - Row 3
    struct AuxButton {
        uint16_t x, y, w, h;
        int id;
        const char* name;
    };
    
    AuxButton auxBtns[] = {
        {20, 225, 120, 50, 200, "FAN 1"},
        {170, 225, 120, 50, 201, "FAN 2"},
        {320, 225, 120, 50, 202, "FUELPUMP"}
    };
    
    for (int i = 0; i < 3; i++) {
        if (x >= auxBtns[i].x && x <= (auxBtns[i].x + auxBtns[i].w) &&
            y >= auxBtns[i].y && y <= (auxBtns[i].y + auxBtns[i].h)) {
            
            selectedButton = auxBtns[i].id;
#if ENABLE_DEBUG_MODE
            if (debugMode) {
                Serial.printf("[Bench] Auxiliary control button pressed: %s\n", auxBtns[i].name);
            }
#endif
            
            // Show immediate visual feedback
            draw(true);
            
            switch (auxBtns[i].id) {
                case 200: // FAN 1
                    testResult = benchFan1();
#if ENABLE_DEBUG_MODE
                    if (debugMode) {
                        Serial.printf("[Bench] FAN 1 test: %s\n", testResult ? "OK" : "FAIL");
                    }
#endif
                    break;
                case 201: // FAN 2
                    testResult = benchFan2();
#if ENABLE_DEBUG_MODE
                    if (debugMode) {
                        Serial.printf("[Bench] FAN 2 test: %s\n", testResult ? "OK" : "FAIL");
                    }
#endif
                    break;
                case 202: // Fuelpump
                    testResult = benchFuelpump();
#if ENABLE_DEBUG_MODE
                    if (debugMode) {
                        Serial.printf("[Bench] Fuelpump test: %s\n", testResult ? "OK" : "FAIL");
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
        display.drawString("IGNITION TEST:", 15, 70);  // Moved right and down for margin
        
        // Draw IGN1-IGN8 buttons (adjusted for 10px margin)
        for (int i = 1; i <= 8; i++) {
            int x = 15 + (i-1) * 55;  // Start at 15px with smaller spacing to fit in margin
            int y = 85;  // Moved down from 80
            bool isPressed = (selectedButton == i);
            uint16_t bgColor = isPressed ? TFT_WHITE : TFT_ORANGE;
            uint16_t textColor = isPressed ? TFT_BLACK : TFT_WHITE;
            uint16_t borderColor = TFT_WHITE;
            
            // Add shadow effect for pressed state
            if (isPressed) {
                display.fillRect(x+2, y+2, 50, 45, TFT_DARKGREY);
            }
            
            display.fillRect(x, y, 50, 45, bgColor);
            display.drawRect(x, y, 50, 45, borderColor);
            
            display.loadFont(AA_FONT_SMALL);
            display.setTextColor(textColor, bgColor);
            display.setTextDatum(MC_DATUM);
            display.drawString("IGN" + String(i), x + 25, y + 22);
        }
    }
}

void BenchScreen::drawInjectorButtons(bool forceRedraw) {
    if (forceRedraw) {
        // Clear INJ section area
        display.fillRect(0, 130, display.width(), 70, TFT_BLACK);
        
        // Draw INJ section header
        display.loadFont(AA_FONT_SMALL);
        display.setTextColor(TFT_ORANGE, TFT_BLACK);
        display.setTextDatum(TL_DATUM);
        display.drawString("INJECTOR TEST:", 15, 140);  // Moved right for margin
        
        // Draw INJ1-INJ8 buttons (adjusted for 10px margin)
        for (int i = 1; i <= 8; i++) {
            int x = 15 + (i-1) * 55;  // Start at 15px with smaller spacing to fit in margin
            int y = 155; // Moved down from 150
            bool isPressed = (selectedButton == (i + 10));
            uint16_t bgColor = isPressed ? TFT_WHITE : TFT_ORANGE;
            uint16_t textColorFinal = isPressed ? TFT_BLACK : TFT_WHITE;
            uint16_t borderColor = TFT_WHITE;
            
            // Add shadow effect for pressed state
            if (isPressed) {
                display.fillRect(x+2, y+2, 50, 45, TFT_DARKGREY);
            }
            
            display.fillRect(x, y, 50, 45, bgColor);
            display.drawRect(x, y, 50, 45, borderColor);
            
            display.loadFont(AA_FONT_SMALL);
            display.setTextColor(textColorFinal, bgColor);
            display.setTextDatum(MC_DATUM);
            display.drawString("INJ" + String(i), x + 25, y + 22);
        }
    }
}

void BenchScreen::drawFanAndFuelPumpButtons(bool forceRedraw) {
    if (forceRedraw) {
        // Clear section area
        display.fillRect(0, 200, display.width(), 100, TFT_BLACK);
        
        // Draw section header
        display.loadFont(AA_FONT_SMALL);
        display.setTextColor(TFT_ORANGE, TFT_BLACK);
        display.setTextDatum(TL_DATUM);
        display.drawString("AUXILIARY CONTROL:", 10, 205);
        
        // Define three buttons with proper spacing
        struct AuxButton {
            uint16_t x, y, w, h;
            int id;
            const char* label;
            uint16_t color;
        };
        
        AuxButton buttons[] = {
            {20, 225, 120, 50, 200, "FAN 1", TFT_ORANGE},
            {170, 225, 120, 50, 201, "FAN 2", TFT_ORANGE},
            {320, 225, 120, 50, 202, "FUELPUMP", TFT_ORANGE}
        };
        
        for (int i = 0; i < 3; i++) {
            bool isPressed = (selectedButton == buttons[i].id);
            uint16_t bgColor = isPressed ? TFT_WHITE : TFT_ORANGE;
            uint16_t textColor = isPressed ? TFT_BLACK : TFT_WHITE;
            uint16_t borderColor = TFT_WHITE;
            
            // Draw button background
            if (isPressed) display.fillRect(buttons[i].x + 2, buttons[i].y + 2, buttons[i].w, buttons[i].h, TFT_DARKGREY);
            display.fillRect(buttons[i].x, buttons[i].y, buttons[i].w, buttons[i].h, bgColor);
            display.drawRect(buttons[i].x, buttons[i].y, buttons[i].w, buttons[i].h, borderColor);
            
            // Draw button text
            display.loadFont(AA_FONT_SMALL);
            display.setTextColor(textColor, bgColor);
            display.setTextDatum(MC_DATUM);
            display.drawString(buttons[i].label, buttons[i].x + buttons[i].w / 2, buttons[i].y + buttons[i].h / 2);
        }
        
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

bool BenchScreen::benchFan1() {
    // TODO: Implement FAN 1 control command via CAN
    // Placeholder: return true for now (non-functional)
    Serial.println("[Bench] FAN 1 button pressed - command pending");
    return true;
}

bool BenchScreen::benchFan2() {
    // TODO: Implement FAN 2 control command via CAN
    // Placeholder: return true for now (non-functional)
    Serial.println("[Bench] FAN 2 button pressed - command pending");
    return true;
}

bool BenchScreen::benchFuelpump() {
    // TODO: Implement Fuelpump control command via CAN
    // Placeholder: return true for now (non-functional)
    Serial.println("[Bench] Fuelpump button pressed - command pending");
    return true;
}