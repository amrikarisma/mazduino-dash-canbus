#include "MainScreen.h"
#include "DisplayManager.h"
#include "GlobalVariables.h"
#include "Config.h"
#include "Simulator.h"
#include "NotoSansBold15.h"
#include "NotoSansBold36.h"
#include "drawing_utils.h"
#include <TFT_eSPI.h>

// External references
extern TFT_eSPI display;
extern TFT_eSprite spr;
extern uint16_t spr_width;

// Static variable definitions
bool MainScreen::initialized = false;
uint32_t MainScreen::lastFullUpdate = 0;
bool MainScreen::screenIndicatorDrawn = false;

// Global instance
MainScreen mainScreen;

MainScreen::MainScreen() {
}

void MainScreen::begin() {
    initialized = true;
    lastFullUpdate = 0;
    screenIndicatorDrawn = false;
}

void MainScreen::draw(bool forceRedraw) {
    if (forceRedraw) {
        // Reset all static tracking variables on force redraw
        lastFullUpdate = 0;
        screenIndicatorDrawn = false;
    }
    
    // Use configurable display system with performance optimizations
    drawConfigurableData(forceRedraw);
    
    // Draw status indicators (simulator, comm mode, debug info)
    drawStatusIndicators(forceRedraw);
    
    // Draw screen indicator
    drawScreenIndicator(forceRedraw);
}

void MainScreen::update() {
    // Regular update without force redraw
    draw(false);
}

void MainScreen::drawConfigurableData(bool setup) {
    // Draw RPM and VSS with reduced frequency update (only when changed or setup)
    static uint32_t lastRpmUpdate = 0;
    static unsigned int lastVss = 999; // Different initial value to force first update
    static unsigned int lastRpm = 999; // Track last RPM for comparison
    
    // Reset static variables on force redraw (screen change)
    if (setup) {
        lastRpm = 999; // Force redraw by making values different
        lastVss = 999;
        lastRpmUpdate = 0;
    }
    
    if (lastRpm != rpm || lastVss != vss || setup || (millis() - lastRpmUpdate > 100)) {
        drawRPMBarBlocks(rpm); // Use default maxRPM from config
        
        // Draw RPM value with background and border
        spr.loadFont(AA_FONT_SMALL);
        spr.createSprite(90, 30);
        spr_width = spr.textWidth("8888");
        spr.setTextColor(TFT_WHITE, TFT_BLACK, true);
        spr.setTextDatum(BR_DATUM);
        spr.drawString("RPM", 80, 2); // Label at top
        spr.setTextDatum(BR_DATUM);
        spr.loadFont(AA_FONT_SMALL); // Use smaller font for RPM value to match existing pattern
        spr.drawNumber(rpm, 80, 30); // Value at bottom with smaller font
        spr.pushSprite(275, 15);
        spr.deleteSprite();
        
        // Draw VSS value with background and border
        spr.createSprite(120, 50);
        spr_width = spr.textWidth("888");
        spr.setTextColor(TFT_WHITE, TFT_BLACK, true);
        
        // Draw the numeric value with large font
        spr.setTextDatum(BR_DATUM);
        spr.loadFont(AA_FONT_LARGE);
        spr.drawNumber(vss, 60, 46); // Value positioned to left
        
        // Draw "kph" unit with smaller font
        spr.setTextDatum(BL_DATUM);
        spr.loadFont(AA_FONT_SMALL);
        spr.drawString("kph", 62, 42); // Unit positioned to right of value
        
        spr.pushSprite(250, 135);
        spr.deleteSprite();
        
        lastRpm = rpm;
        lastVss = vss;
        lastRpmUpdate = millis();
    }
    
    // Draw configurable panels with reduced frequency
    static uint32_t lastPanelUpdate = 0;
    if (setup) {
        lastPanelUpdate = 0; // Force panel redraw
    }
    if (setup || (millis() - lastPanelUpdate > 50)) { // Update panels every 50ms max
        drawConfigurablePanels(setup);
        lastPanelUpdate = millis();
    }
}

void MainScreen::drawStatusIndicators(bool forceRedraw) {
#if ENABLE_SIMULATOR
    // Draw simulator indicator if simulator is active (with reduced update frequency)
    static uint8_t lastSimMode = SIMULATOR_MODE_OFF;
    static uint32_t lastSimUpdate = 0;
    uint8_t currentSimMode = getSimulatorMode();
    
    // Force redraw or update if simulator mode has changed or every 500ms
    if (forceRedraw || currentSimMode != lastSimMode || (millis() - lastSimUpdate > 500)) {
        if (currentSimMode != SIMULATOR_MODE_OFF) {
            // Clear the SIM area first
            display.fillRect(display.width() - 30, 5, 25, 15, TFT_BLACK);
            
            // Draw SIM indicator
            display.loadFont(AA_FONT_SMALL);
            display.setTextColor(TFT_YELLOW, TFT_BLACK);
            display.setTextDatum(TR_DATUM);
            display.drawString("SIM", display.width() - 5, 5);
        } else {
            // Clear the SIM indicator when simulator is turned off
            display.fillRect(display.width() - 30, 5, 25, 15, TFT_BLACK);
        }
        
        lastSimMode = currentSimMode;
        lastSimUpdate = millis();
    }
#endif

    // Draw communication mode indicator (top left) with reduced update frequency
    static bool lastCommMode = true;  // Track changes
    static String lastCommText = "";
    static uint32_t lastCommUpdate = 0;
    
    String currentCommText = isCANMode ? "CAN" : "SER";
    
    // Force redraw or update if communication mode has changed or every 1000ms
    if (forceRedraw || isCANMode != lastCommMode || currentCommText != lastCommText || (millis() - lastCommUpdate > 1000)) {
        // Clear the comm mode area first
        display.fillRect(5, 5, 40, 15, TFT_BLACK);
        
        // Draw new communication mode
        display.loadFont(AA_FONT_SMALL);
        display.setTextColor(isCANMode ? TFT_GREEN : TFT_ORANGE, TFT_BLACK);
        display.setTextDatum(TL_DATUM);
        display.drawString(currentCommText, 5, 5);
        
        lastCommMode = isCANMode;
        lastCommText = currentCommText;
        lastCommUpdate = millis();
    }

#if ENABLE_DEBUG_MODE
    static bool lastDebugMode = false;
    static String lastDebugInfo = "";
    
    if (debugMode) {
        // Create debug info string - show only essential info in one line
        String debugInfo = "CPU:" + String(cpuUsage, 1) + "% FPS:" + String(fps, 1) + " Heap:" + String(ESP.getFreeHeap()/1024) + "K";
        
        if (forceRedraw || debugInfo != lastDebugInfo || !lastDebugMode) {
            int centerX = display.width() / 2;
            
            // Clear the debug area first to prevent font overlap
            display.fillRect(centerX - 120, 5, 240, 20, TFT_BLACK);
            
            // Draw debug info
            display.loadFont(AA_FONT_SMALL);
            display.setTextColor(TFT_CYAN, TFT_BLACK);
            display.setTextDatum(TC_DATUM);
            display.drawString(debugInfo, centerX, 5);
            
            lastDebugInfo = debugInfo;
        }
        
        lastDebugMode = true;
    } else {
        // Clear debug area when debug mode is turned off
        if (lastDebugMode || forceRedraw) {
            // Clear the top center area where debug info was displayed
            display.fillRect(0, 5, display.width(), 20, TFT_BLACK);
            lastDebugMode = false;
            lastDebugInfo = "";
        }
    }
#endif
}

void MainScreen::drawScreenIndicator(bool forceRedraw) {
    // Draw screen indicator (bottom right) - always draw on force redraw
    if (forceRedraw || !screenIndicatorDrawn) {
        display.loadFont(AA_FONT_SMALL);
        display.setTextColor(TFT_WHITE, TFT_BLACK);
        display.setTextDatum(BR_DATUM);
        display.drawString("MAIN", display.width() - 5, display.height() - 5);
        screenIndicatorDrawn = true;
    }
}