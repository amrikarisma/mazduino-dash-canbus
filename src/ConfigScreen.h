#ifndef CONFIG_SCREEN_H
#define CONFIG_SCREEN_H

#include <Arduino.h>

// Forward declarations
class TFT_eSPI;

/**
 * ConfigScreen - Handles the configuration display
 * Shows system settings, WiFi status, version info, and configuration options
 */
class ConfigScreen {
public:
    ConfigScreen();
    
    /**
     * Initialize the configuration screen
     */
    void begin();
    
    /**
     * Draw the configuration screen content
     * @param forceRedraw Force complete redraw of all elements
     */
    void draw(bool forceRedraw = false);
    
    /**
     * Update configuration screen data
     */
    void update();
    
    /**
     * Handle touch events for configuration options
     * @param x Touch X coordinate
     * @param y Touch Y coordinate
     * @return true if touch was handled
     */
    bool handleTouch(uint16_t x, uint16_t y);
    
    /**
     * Handle touch release events
     * @return true if release was handled
     */
    bool handleTouchRelease();
    
private:
    // Helper functions
    void drawHeader(bool forceRedraw);
    void drawScreenIndicator(bool forceRedraw);
    
    // Configuration sections
    void drawCommModeSection(int y);
    void drawWiFiControlSection(int y);
    void drawBrightnessSection(int y);
    void drawBrightnessValueAndSlider(int y);
    void drawWiFiInfo(bool forceRedraw);
    
    // Configuration change handlers
    bool handleCommModeChange();
    bool handleWiFiControlTouch(uint16_t x, uint16_t y);
    bool handleBrightnessSliderTouch(uint16_t x);
    
    // Static variables for update tracking
    static bool initialized;
    static uint32_t lastUpdate;
    
    // Press and hold tracking
    static bool isPressing;
    static uint32_t pressStartTime;
};

// Global instance
extern ConfigScreen configScreen;

#endif // CONFIG_SCREEN_H