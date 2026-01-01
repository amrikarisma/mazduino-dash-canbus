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
    
private:
    // Helper functions
    void drawSystemInfo(bool forceRedraw);
    void drawWiFiStatus(bool forceRedraw);
    void drawConfigOptions(bool forceRedraw);
    void drawScreenIndicator(bool forceRedraw);
    
    // Configuration sections
    void drawStatusSection(int y, bool selected = false);
    void drawWiFiSection(int y, bool selected = false);
    void drawDisplaySection(int y, bool selected = false);
    void drawBrightnessSection(int y, bool selected = false);
    void drawDebugSection(int y, bool selected = false);
    void drawInfoSection(int y, bool selected = false);
    
    // Static variables for update tracking
    static bool initialized;
    static uint32_t lastUpdate;
    static int selectedSection;
};

// Global instance
extern ConfigScreen configScreen;

#endif // CONFIG_SCREEN_H