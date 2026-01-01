#ifndef BENCH_SCREEN_H
#define BENCH_SCREEN_H

#include <Arduino.h>

// Forward declarations
class TFT_eSPI;

/**
 * BenchScreen - Handles the bench test display
 * Shows test controls, data logging, and diagnostic information
 */
class BenchScreen {
public:
    BenchScreen();
    
    /**
     * Initialize the bench screen
     */
    void begin();
    
    /**
     * Draw the bench screen content
     * @param forceRedraw Force complete redraw of all elements
     */
    void draw(bool forceRedraw = false);
    
    /**
     * Update bench screen data
     */
    void update();
    
    /**
     * Handle touch events for bench test controls
     * @param x Touch X coordinate
     * @param y Touch Y coordinate
     * @return true if touch was handled
     */
    bool handleTouch(uint16_t x, uint16_t y);
    
private:
    // Helper functions
    void drawTestControls(bool forceRedraw);
    void drawDataLogger(bool forceRedraw);
    void drawDiagnostics(bool forceRedraw);
    void drawScreenIndicator(bool forceRedraw);
    
    // Test control buttons
    void drawStartTestButton(bool selected = false);
    void drawStopTestButton(bool selected = false);
    void drawResetButton(bool selected = false);
    void drawDataLogButton(bool selected = false);
    
    // Test modes
    void drawTestModeSelector(bool forceRedraw);
    void drawCurrentTestStatus(bool forceRedraw);
    
    // Data display
    void drawRealTimeData(bool forceRedraw);
    void drawTestResults(bool forceRedraw);
    
    // Static variables for update tracking
    static bool initialized;
    static uint32_t lastUpdate;
    static bool isTestRunning;
    static int selectedButton;
    static int testMode;
    static uint32_t testStartTime;
    static uint32_t testDuration;
};

// Test modes
enum BenchTestMode {
    TEST_MODE_IDLE = 0,
    TEST_MODE_RPM_SWEEP = 1,
    TEST_MODE_LOAD_TEST = 2,
    TEST_MODE_DATA_LOG = 3,
    TEST_MODE_DIAGNOSTIC = 4
};

// Global instance
extern BenchScreen benchScreen;

#endif // BENCH_SCREEN_H