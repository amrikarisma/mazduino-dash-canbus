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
    void drawSystemInfo(bool forceRedraw);
    void drawWiFiStatus(bool forceRedraw);
    void drawConfigOptions(bool forceRedraw);
    void drawScreenIndicator(bool forceRedraw);
    void drawPressProgress();
    void clearProgressArea();
    
    // Configuration sections
    void drawStatusSection(int y, bool selected = false);
    void drawCommModeSection(int y, bool selected = false);
    void drawWiFiSection(int y, bool selected = false);
    void drawDisplaySection(int y, bool selected = false);
    void drawBrightnessSection(int y, bool selected = false);
    void drawDebugSection(int y, bool selected = false);
    void drawInfoSection(int y, bool selected = false);
    
    // Configuration change handlers
    void handleSectionTouch(int sectionId);
    void toggleCommMode();
    void adjustBrightness();
    void toggleDebugMode();
    
    // Static variables for update tracking
    static bool initialized;
    static uint32_t lastUpdate;
    static int selectedSection;
    
    // Press and hold tracking
    static bool isPressing;
    static uint32_t pressStartTime;
    static int pressedSection;
    static uint16_t pressX, pressY;
};

// Global instance
extern ConfigScreen configScreen;

#endif // CONFIG_SCREEN_H