#ifndef BENCH_SCREEN_H
#define BENCH_SCREEN_H

#include <Arduino.h>

// Forward declarations
class TFT_eSPI;

/**
 * BenchScreen - Handles the bench test display
 * Shows ignition, injector, and engine start/stop test controls
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
    void drawIgnitionButtons(bool forceRedraw);
    void drawInjectorButtons(bool forceRedraw);
    void drawFanAndFuelPumpButtons(bool forceRedraw);
    void drawScreenIndicator(bool forceRedraw);
    
    // Bench test functions
    bool benchIgnition(uint8_t cylinder);
    bool benchInjector(uint8_t cylinder);
    bool benchFan1();
    bool benchFan2();
    bool benchFuelpump();
    
    // Static variables for update tracking
    static bool initialized;
    static uint32_t lastUpdate;
    static bool isTestRunning;
    static int selectedButton;
    static int testMode;
    static uint32_t testStartTime;
    static uint32_t testDuration;
    static int selectedCylinder;
    static bool testResult;
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