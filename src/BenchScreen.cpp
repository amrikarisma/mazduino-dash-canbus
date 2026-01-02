#include "BenchScreen.h"
#include "GlobalVariables.h"
#include "Config.h"
#include "Simulator.h"
#include "Roboto16.h"
#include "RobotoBold32.h"
#include <TFT_eSPI.h>

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
}

void BenchScreen::draw(bool forceRedraw) {
    if (forceRedraw) {
        lastUpdate = 0;
        
        // Draw title
        display.loadFont(AA_FONT_LARGE);
        display.setTextColor(TFT_RED, TFT_BLACK);
        display.setTextDatum(TC_DATUM);
        display.drawString("BENCH TEST", display.width() / 2, 8);
    }
    
    // Draw test controls
    drawTestControls(forceRedraw);
    
    // Draw data logger section
    drawDataLogger(forceRedraw);
    
    // Draw diagnostics
    drawDiagnostics(forceRedraw);
    
    // Draw screen indicator
    drawScreenIndicator(forceRedraw);
}

void BenchScreen::update() {
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
    // Define touch areas for test control buttons
    struct TouchArea {
        uint16_t x, y, w, h;
        int buttonId;
        const char* name;
    };
    
    TouchArea buttons[] = {
        {20, 50, 100, 40, 0, "Start Test"},
        {130, 50, 100, 40, 1, "Stop Test"},
        {240, 50, 100, 40, 2, "Reset"},
        {350, 50, 100, 40, 3, "Data Log"}
    };
    
    for (int i = 0; i < 4; i++) {
        if (x >= buttons[i].x && x <= (buttons[i].x + buttons[i].w) &&
            y >= buttons[i].y && y <= (buttons[i].y + buttons[i].h)) {
            
            selectedButton = buttons[i].buttonId;
            Serial.printf("[Bench] Button pressed: %s\\n", buttons[i].name);
            
            // Handle button actions
            switch (selectedButton) {
                case 0: // Start Test
                    if (!isTestRunning) {
                        isTestRunning = true;
                        testStartTime = millis();
                        testDuration = 0;
                        Serial.println("[Bench] Test started");
                    }
                    break;
                    
                case 1: // Stop Test
                    if (isTestRunning) {
                        isTestRunning = false;
                        Serial.printf("[Bench] Test stopped after %d seconds\\n", testDuration / 1000);
                    }
                    break;
                    
                case 2: // Reset
                    isTestRunning = false;
                    testStartTime = 0;
                    testDuration = 0;
                    testMode = TEST_MODE_IDLE;
                    Serial.println("[Bench] Test data reset");
                    break;
                    
                case 3: // Data Log
                    Serial.println("[Bench] Data logging toggled");
                    break;
            }
            
            // Force redraw to show button state
            draw(true);
            return true;
        }
    }
    
    return false;
}

void BenchScreen::drawTestControls(bool forceRedraw) {
    if (forceRedraw) {
        // Draw control buttons
        drawStartTestButton(selectedButton == 0);
        drawStopTestButton(selectedButton == 1);
        drawResetButton(selectedButton == 2);
        drawDataLogButton(selectedButton == 3);
        
        // Draw test mode selector
        drawTestModeSelector(forceRedraw);
        
        // Draw current test status
        drawCurrentTestStatus(forceRedraw);
    }
}

void BenchScreen::drawDataLogger(bool forceRedraw) {
    if (forceRedraw) {
        // Draw data logger section header
        display.loadFont(AA_FONT_SMALL);
        display.setTextColor(TFT_YELLOW, TFT_BLACK);
        display.setTextDatum(TL_DATUM);
        display.drawString("DATA LOGGER", 20, 120);
        
        // Draw real-time data
        drawRealTimeData(forceRedraw);
        
        // Draw test results if available
        if (testDuration > 0) {
            drawTestResults(forceRedraw);
        }
    }
}

void BenchScreen::drawDiagnostics(bool forceRedraw) {
    if (forceRedraw) {
        // Draw diagnostics section
        display.loadFont(AA_FONT_SMALL);
        display.setTextColor(TFT_CYAN, TFT_BLACK);
        display.setTextDatum(TL_DATUM);
        display.drawString("DIAGNOSTICS", 20, 200);
        
        // Show system stats
        String memInfo = "Free Heap: " + String(ESP.getFreeHeap() / 1024) + "KB";
        String canInfo = "CAN Status: " + String(isCANMode ? "Active" : "Inactive");
        String commInfo = "Comm Rate: " + String(fps, 1) + " FPS";
        
        display.setTextColor(TFT_WHITE, TFT_BLACK);
        display.drawString(memInfo, 20, 220);
        display.drawString(canInfo, 20, 240);
        display.drawString(commInfo, 20, 260);
    }
}

void BenchScreen::drawScreenIndicator(bool forceRedraw) {
    if (forceRedraw) {
        display.loadFont(AA_FONT_SMALL);
        display.setTextColor(TFT_WHITE, TFT_BLACK);
        display.setTextDatum(BR_DATUM);
        display.drawString("BENCH", display.width() - 5, display.height() - 5);
    }
}

void BenchScreen::drawStartTestButton(bool selected) {
    uint16_t bgColor = selected ? TFT_GREEN : (isTestRunning ? TFT_DARKGREEN : TFT_DARKGREY);
    uint16_t textColor = selected ? TFT_BLACK : TFT_WHITE;
    
    display.fillRect(20, 50, 100, 40, bgColor);
    display.drawRect(20, 50, 100, 40, TFT_WHITE);
    
    display.loadFont(AA_FONT_SMALL);
    display.setTextColor(textColor, bgColor);
    display.setTextDatum(MC_DATUM);
    display.drawString("START", 70, 70);
}

void BenchScreen::drawStopTestButton(bool selected) {
    uint16_t bgColor = selected ? TFT_RED : (!isTestRunning ? TFT_MAROON : TFT_DARKGREY);
    uint16_t textColor = selected ? TFT_WHITE : TFT_WHITE;
    
    display.fillRect(130, 50, 100, 40, bgColor);
    display.drawRect(130, 50, 100, 40, TFT_WHITE);
    
    display.loadFont(AA_FONT_SMALL);
    display.setTextColor(textColor, bgColor);
    display.setTextDatum(MC_DATUM);
    display.drawString("STOP", 180, 70);
}

void BenchScreen::drawResetButton(bool selected) {
    uint16_t bgColor = selected ? TFT_YELLOW : TFT_DARKGREY;
    uint16_t textColor = selected ? TFT_BLACK : TFT_WHITE;
    
    display.fillRect(240, 50, 100, 40, bgColor);
    display.drawRect(240, 50, 100, 40, TFT_WHITE);
    
    display.loadFont(AA_FONT_SMALL);
    display.setTextColor(textColor, bgColor);
    display.setTextDatum(MC_DATUM);
    display.drawString("RESET", 290, 70);
}

void BenchScreen::drawDataLogButton(bool selected) {
    uint16_t bgColor = selected ? TFT_BLUE : TFT_DARKGREY;
    uint16_t textColor = selected ? TFT_WHITE : TFT_WHITE;
    
    display.fillRect(350, 50, 100, 40, bgColor);
    display.drawRect(350, 50, 100, 40, TFT_WHITE);
    
    display.loadFont(AA_FONT_SMALL);
    display.setTextColor(textColor, bgColor);
    display.setTextDatum(MC_DATUM);
    display.drawString("LOG", 400, 70);
}

void BenchScreen::drawTestModeSelector(bool forceRedraw) {
    if (forceRedraw) {
        display.loadFont(AA_FONT_SMALL);
        display.setTextColor(TFT_WHITE, TFT_BLACK);
        display.setTextDatum(TL_DATUM);
        display.drawString("Test Mode:", 20, 100);
        
        const char* modes[] = {"IDLE", "RPM SWEEP", "LOAD TEST", "DATA LOG", "DIAGNOSTIC"};
        display.setTextColor(TFT_ORANGE, TFT_BLACK);
        display.drawString(modes[testMode], 120, 100);
    }
}

void BenchScreen::drawCurrentTestStatus(bool forceRedraw) {
    if (forceRedraw) {
        String status = isTestRunning ? "RUNNING" : "STOPPED";
        uint16_t statusColor = isTestRunning ? TFT_GREEN : TFT_RED;
        
        display.loadFont(AA_FONT_SMALL);
        display.setTextColor(TFT_WHITE, TFT_BLACK);
        display.setTextDatum(TL_DATUM);
        display.drawString("Status:", 250, 100);
        
        display.setTextColor(statusColor, TFT_BLACK);
        display.drawString(status, 320, 100);
        
        if (isTestRunning && testDuration > 0) {
            String duration = "Duration: " + String(testDuration / 1000) + "s";
            display.setTextColor(TFT_YELLOW, TFT_BLACK);
            display.drawString(duration, 380, 100);
        }
    }
}

void BenchScreen::drawRealTimeData(bool forceRedraw) {
    // Show current ECU data in real-time
    String rpmStr = "RPM: " + String(rpm);
    String vssStr = "VSS: " + String(vss) + " km/h";
    String tpsStr = "TPS: " + String(tps, 1) + "%";
    String iatStr = "IAT: " + String(iat, 1) + "°C";
    
    display.loadFont(AA_FONT_SMALL);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setTextDatum(TL_DATUM);
    
    display.drawString(rpmStr, 20, 140);
    display.drawString(vssStr, 120, 140);
    display.drawString(tpsStr, 220, 140);
    display.drawString(iatStr, 320, 140);
}

void BenchScreen::drawTestResults(bool forceRedraw) {
    if (testDuration > 1000) { // Only show if test ran for more than 1 second
        display.loadFont(AA_FONT_SMALL);
        display.setTextColor(TFT_CYAN, TFT_BLACK);
        display.setTextDatum(TL_DATUM);
        
        String testTime = "Test Duration: " + String(testDuration / 1000) + "s";
        String maxRpm = "Max RPM: " + String(rpm); // Would track actual max in real implementation
        String avgVss = "Avg VSS: " + String(vss); // Would calculate actual average
        
        display.drawString("TEST RESULTS:", 20, 160);
        display.setTextColor(TFT_WHITE, TFT_BLACK);
        display.drawString(testTime, 20, 175);
        display.drawString(maxRpm, 180, 175);
        display.drawString(avgVss, 320, 175);
    }
}