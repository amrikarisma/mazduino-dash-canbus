#ifndef TOUCH_HANDLER_H
#define TOUCH_HANDLER_H

#include <XPT2046_Touchscreen.h>
#include <SPI.h>
#include "Config.h"

// Touch button areas (for web interface navigation)
struct TouchArea {
  uint16_t x, y, w, h;
  const char* label;
  void (*callback)();
};

// Touch event structure
struct TouchEvent {
  uint16_t x, y;
  bool pressed;
  uint32_t timestamp;
  bool isValid;
};

// Swipe direction enum
enum SwipeDirection {
  SWIPE_NONE,
  SWIPE_LEFT,
  SWIPE_RIGHT,
  SWIPE_UP,
  SWIPE_DOWN
};

// Swipe event structure
struct SwipeEvent {
  SwipeDirection direction;
  uint16_t startX, startY;
  uint16_t endX, endY;
  uint32_t duration;
  bool isValid;
};

class TouchHandler {
private:
  XPT2046_Touchscreen* touch;
  uint32_t lastTouchTime;
  uint16_t debounceDelay;
    // Swipe tracking
  bool swipeStarted;
  uint16_t swipeStartX, swipeStartY;
  uint32_t swipeStartTime;
    // Calibration
  bool isCalibrated;
  uint16_t calMinX, calMaxX, calMinY, calMaxY;
  
  // Touch areas for navigation
  TouchArea* touchAreas;
  
public:
  uint8_t touchAreaCount;
  TouchEvent lastTouch; // Make lastTouch accessible
  
public:
  TouchHandler();
  ~TouchHandler();
  
  // Initialization
  bool begin();
  void calibrate();
  bool isCalibrationValid();
  
  // Touch reading
  TouchEvent readTouch();
  TouchEvent readTouchRaw(); // For swipe detection without stability filtering
  bool isTouched();
  void update();
  
  // Swipe detection
  SwipeEvent detectSwipe();
  void resetSwipe();
  
  // Coordinate mapping
  uint16_t mapTouchX(uint16_t rawX);
  uint16_t mapTouchY(uint16_t rawY);
  
  // Touch area management
  void addTouchArea(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const char* label, void (*callback)());
  void clearTouchAreas();
  void handleTouchAreas(uint16_t x, uint16_t y);
  
  // Calibration data
  void saveCalibration();
  void loadCalibration();
  void setCalibrationData(uint16_t minX, uint16_t maxX, uint16_t minY, uint16_t maxY);
  
  // Debug functions
  void printTouchInfo();
  void drawTouchPoint(uint16_t x, uint16_t y);
};

// Global touch handler instance
extern TouchHandler touchHandler;

// Touch callback functions for navigation
void onStatusSectionTouch();
void onWifiSectionTouch();
void onDisplaySectionTouch();
void onBrightnessSectionTouch();
void onDebugSectionTouch();
void onInfoSectionTouch();
void onBackTouch();

// Touch utility functions
void setupTouchNavigation();
void handleTouchNavigation();
void drawTouchButtons();

#endif // TOUCH_HANDLER_H