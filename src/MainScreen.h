#ifndef MAIN_SCREEN_H
#define MAIN_SCREEN_H

#include <Arduino.h>

// Forward declarations
class TFT_eSPI;
class TFT_eSprite;

/**
 * MainScreen - Handles the main dashboard display
 * Shows ECU data like RPM, VSS, and configurable data panels
 */
class MainScreen {
public:
    MainScreen();
    
    /**
     * Initialize the main screen
     */
    void begin();
    
    /**
     * Draw the main screen content
     * @param forceRedraw Force complete redraw of all elements
     */
    void draw(bool forceRedraw = false);
    
    /**
     * Update main screen data (called frequently)
     */
    void update();
    
private:
    // Helper functions
    void drawConfigurableData(bool setup);
    void drawStatusIndicators(bool forceRedraw);
    void drawScreenIndicator(bool forceRedraw);
    
    // Static variables for update tracking
    static bool initialized;
    static uint32_t lastFullUpdate;
    static bool screenIndicatorDrawn;
};

// Global instance
extern MainScreen mainScreen;

#endif // MAIN_SCREEN_H