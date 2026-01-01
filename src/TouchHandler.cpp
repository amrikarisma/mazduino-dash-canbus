#include "TouchHandler.h"
#include "Config.h"
#include <EEPROM.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <math.h>

// Create dedicated SPI instance for touch
SPIClass touchSPI(HSPI);

// Global touch handler instance
TouchHandler touchHandler;

// External display reference
extern TFT_eSPI display;
extern uint8_t currentScreen;

TouchHandler::TouchHandler() :
  touch(nullptr),
  lastTouchTime(0),
  debounceDelay(20), // Reduced from 50ms to 20ms
  swipeStarted(false),
  swipeStartX(0),
  swipeStartY(0),
  swipeStartTime(0),
  isCalibrated(false),
  calMinX(TOUCH_MIN_X),
  calMaxX(TOUCH_MAX_X),
  calMinY(TOUCH_MIN_Y),
  calMaxY(TOUCH_MAX_Y),
  touchAreas(nullptr),
  touchAreaCount(0)
{
  lastTouch.x = 0;
  lastTouch.y = 0;
  lastTouch.pressed = false;
  lastTouch.timestamp = 0;
  lastTouch.isValid = false;
}

TouchHandler::~TouchHandler() {
  if (touch) {
    delete touch;
  }
  clearTouchAreas();
}

bool TouchHandler::begin() {
  Serial.println("[Touch] Initializing XPT2046 touchscreen...");
  
  // Initialize dedicated SPI bus for touch
  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  Serial.printf("[Touch] Touch SPI initialized - CLK:%d, MISO:%d, MOSI:%d, CS:%d, IRQ:%d\n", 
                XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS, XPT2046_IRQ);
  
  // Initialize touch controller with dedicated SPI
  touch = new XPT2046_Touchscreen(XPT2046_CS, XPT2046_IRQ);
  touch->begin(touchSPI);
  
  if (!touch->tirqTouched()) {
    Serial.println("[Touch] WARNING: Touch IRQ not responding, but continuing...");
  }
  
  // Set rotation to match display
  touch->setRotation(1); // Landscape mode
  
  // Load calibration data from EEPROM
  loadCalibration();
  
  Serial.println("[Touch] XPT2046 touchscreen initialized successfully");
  Serial.printf("[Touch] Calibration: X(%d-%d) Y(%d-%d)\\n", 
                calMinX, calMaxX, calMinY, calMaxY);
  
  return true;
}

TouchEvent TouchHandler::readTouch() {
  TouchEvent event = {0, 0, false, millis(), false};
  
  if (!touch) {
    return event;
  }
  
  // Get raw touch coordinates
  TS_Point p = touch->getPoint();
  
  static bool wasTouched = false;
  
  if (p.z < 200) { // Minimum pressure threshold
    // No touch detected
    if (wasTouched) {
      // Touch was just released - return valid release event
      event.x = lastTouch.x;
      event.y = lastTouch.y;
      event.pressed = false;
      event.timestamp = millis();
      event.isValid = true;
      wasTouched = false;
      lastTouchTime = millis();
      return event;
    }
    return event; // No touch, return invalid event
  }
  
  // Touch detected - apply debounce only for new touches
  if (!wasTouched) {
    // New touch started
    if (millis() - lastTouchTime < debounceDelay) {
      return event; // Still in debounce period
    }
    wasTouched = true;
  }
  
  // Map raw coordinates to screen coordinates
  event.x = mapTouchX(p.x);
  event.y = mapTouchY(p.y);
  event.pressed = true;
  event.timestamp = millis();
  event.isValid = true;
  
  // Reduce debug output frequency
  static uint8_t debugCounter = 0;
  static uint16_t lastDebugX = 0, lastDebugY = 0;
  
  // Only print on significant position change or every 50th touch
  if (debugCounter++ % 50 == 0 || abs((int)event.x - (int)lastDebugX) > 30 || abs((int)event.y - (int)lastDebugY) > 30) {
    Serial.printf("[Touch] (%d,%d)\n", event.x, event.y);
    lastDebugX = event.x;
    lastDebugY = event.y;
  }
  
  lastTouch = event;
  lastTouchTime = millis();
  
  return event;
}

bool TouchHandler::isTouched() {
  return touch && touch->touched();
}

void TouchHandler::update() {
  TouchEvent event = readTouch();
  
  if (event.isValid && event.pressed) {
    // Handle touch areas
    handleTouchAreas(event.x, event.y);
    
    // Draw touch feedback (optional)
    #if ENABLE_DEBUG_MODE
    drawTouchPoint(event.x, event.y);
    #endif
  }
}

uint16_t TouchHandler::mapTouchX(uint16_t rawX) {
  // Map raw X coordinate to screen X (0-480)
  if (rawX <= calMinX) return 0;
  if (rawX >= calMaxX) return 480;
  
  return map(rawX, calMinX, calMaxX, 0, 480);
}

uint16_t TouchHandler::mapTouchY(uint16_t rawY) {
  // Map raw Y coordinate to screen Y (0-320)
  if (rawY <= calMinY) return 0;
  if (rawY >= calMaxY) return 320;
  
  return map(rawY, calMinY, calMaxY, 0, 320);
}

void TouchHandler::addTouchArea(uint16_t x, uint16_t y, uint16_t w, uint16_t h, 
                                const char* label, void (*callback)()) {
  // Reallocate touch areas array
  TouchArea* newAreas = new TouchArea[touchAreaCount + 1];
  
  // Copy existing areas
  for (uint8_t i = 0; i < touchAreaCount; i++) {
    newAreas[i] = touchAreas[i];
  }
  
  // Add new area
  newAreas[touchAreaCount] = {x, y, w, h, label, callback};
  
  // Replace old array
  delete[] touchAreas;
  touchAreas = newAreas;
  touchAreaCount++;
  
  Serial.printf("[Touch] Added touch area: %s at (%d,%d,%d,%d)\\n", 
                label, x, y, w, h);
}

void TouchHandler::clearTouchAreas() {
  if (touchAreas) {
    delete[] touchAreas;
    touchAreas = nullptr;
  }
  touchAreaCount = 0;
}

void TouchHandler::handleTouchAreas(uint16_t x, uint16_t y) {
  for (uint8_t i = 0; i < touchAreaCount; i++) {
    TouchArea& area = touchAreas[i];
    
    // Check if touch is within area bounds
    if (x >= area.x && x <= (area.x + area.w) &&
        y >= area.y && y <= (area.y + area.h)) {
      
      Serial.printf("[Touch] Area touched: %s\\n", area.label);
      
      // Execute callback if available
      if (area.callback) {
        area.callback();
      }
      
      // Only trigger first matching area
      break;
    }
  }
}

void TouchHandler::calibrate() {
  Serial.println("[Touch] Starting touch calibration...");
  
  // Simple 2-point calibration
  display.fillScreen(TFT_BLACK);
  display.setTextColor(TFT_WHITE);
  display.setTextSize(2);
  
  // Top-left calibration point
  display.drawString("Touch top-left corner", 10, 10);
  display.fillCircle(30, 30, 5, TFT_RED);
  
  // Wait for touch
  while (!touch->touched()) {
    delay(50);
  }
  
  TS_Point p1 = touch->getPoint();
  calMinX = p1.x;
  calMinY = p1.y;
  
  delay(1000); // Debounce
  
  // Bottom-right calibration point  
  display.fillScreen(TFT_BLACK);
  display.drawString("Touch bottom-right corner", 10, 10);
  display.fillCircle(450, 290, 5, TFT_RED);
  
  while (!touch->touched()) {
    delay(50);
  }
  
  TS_Point p2 = touch->getPoint();
  calMaxX = p2.x;
  calMaxY = p2.y;
  
  // Save calibration
  isCalibrated = true;
  saveCalibration();
  
  display.fillScreen(TFT_BLACK);
  display.drawString("Calibration complete!", 10, 150);
  
  Serial.printf("[Touch] Calibration complete: X(%d-%d) Y(%d-%d)\\n", 
                calMinX, calMaxX, calMinY, calMaxY);
  
  delay(2000);
}

void TouchHandler::saveCalibration() {
  // Save calibration data to EEPROM (addresses 600-611)
  EEPROM.writeUShort(600, calMinX);
  EEPROM.writeUShort(602, calMaxX);
  EEPROM.writeUShort(604, calMinY);
  EEPROM.writeUShort(606, calMaxY);
  EEPROM.write(608, 0xAA); // Calibration valid marker
  EEPROM.commit();
  
  Serial.println("[Touch] Calibration data saved to EEPROM");
}

void TouchHandler::loadCalibration() {
  // Load calibration data from EEPROM
  if (EEPROM.read(608) == 0xAA) {
    calMinX = EEPROM.readUShort(600);
    calMaxX = EEPROM.readUShort(602);
    calMinY = EEPROM.readUShort(604);
    calMaxY = EEPROM.readUShort(606);
    isCalibrated = true;
    
    Serial.println("[Touch] Calibration data loaded from EEPROM");
  } else {
    Serial.println("[Touch] No valid calibration found, using defaults");
  }
}

bool TouchHandler::isCalibrationValid() {
  return isCalibrated && EEPROM.read(608) == 0xAA;
}

void TouchHandler::setCalibrationData(uint16_t minX, uint16_t maxX, 
                                     uint16_t minY, uint16_t maxY) {
  calMinX = minX;
  calMaxX = maxX;
  calMinY = minY;
  calMaxY = maxY;
  isCalibrated = true;
  saveCalibration();
}

void TouchHandler::printTouchInfo() {
  if (!touch) return;
  
  Serial.println("=== Touch Info ===");
  Serial.printf("Calibrated: %s\\n", isCalibrated ? "Yes" : "No");
  Serial.printf("X Range: %d - %d\\n", calMinX, calMaxX);
  Serial.printf("Y Range: %d - %d\\n", calMinY, calMaxY);
  Serial.printf("Touch Areas: %d\\n", touchAreaCount);
  Serial.printf("Last Touch: (%d, %d) at %lu\\n", 
                lastTouch.x, lastTouch.y, lastTouch.timestamp);
}

void TouchHandler::drawTouchPoint(uint16_t x, uint16_t y) {
  // Draw temporary touch indicator
  display.fillCircle(x, y, 3, TFT_YELLOW);
  
  // Remove after short delay (in a real implementation, 
  // you'd want this to be non-blocking)
  delay(100);
  display.fillCircle(x, y, 3, TFT_BLACK);
}

// Swipe detection implementation
SwipeEvent TouchHandler::detectSwipe() {
  SwipeEvent swipe;
  swipe.isValid = false;
  swipe.direction = SWIPE_NONE;
  
  TouchEvent currentTouch = readTouch();
  
  if (currentTouch.isValid && currentTouch.pressed) {
    if (!swipeStarted) {
      // Start tracking swipe
      swipeStarted = true;
      swipeStartX = currentTouch.x;
      swipeStartY = currentTouch.y;
      swipeStartTime = currentTouch.timestamp;
      Serial.printf("[Swipe] Started at (%d,%d) time:%d\n", swipeStartX, swipeStartY, swipeStartTime);
    } else {
      // Update current position during swipe
      lastTouch = currentTouch;
    }
  } else if (swipeStarted && !currentTouch.pressed) {
    // Touch released, check if it was a swipe
    uint32_t swipeDuration = millis() - swipeStartTime;
    
    Serial.printf("[Swipe] Ended. Duration: %dms, lastTouch valid: %d\n", swipeDuration, lastTouch.isValid);
    
    if (swipeDuration < SWIPE_MAX_TIME && lastTouch.isValid) {
      int16_t deltaX = lastTouch.x - swipeStartX;
      int16_t deltaY = lastTouch.y - swipeStartY;
      
      // Check if movement is significant enough
      uint16_t distance = sqrt(deltaX * deltaX + deltaY * deltaY);
      
      Serial.printf("[Swipe] Delta: (%d,%d), Distance: %d, Min: %d\n", deltaX, deltaY, distance, SWIPE_MIN_DISTANCE);
      
      if (distance >= SWIPE_MIN_DISTANCE) {
        // Determine swipe direction (prioritize horizontal)
        if (abs(deltaX) > abs(deltaY)) {
          // Horizontal swipe
          if (deltaX > 0) {
            swipe.direction = SWIPE_RIGHT;
          } else {
            swipe.direction = SWIPE_LEFT;
          }
        } else {
          // Vertical swipe
          if (deltaY > 0) {
            swipe.direction = SWIPE_DOWN;
          } else {
            swipe.direction = SWIPE_UP;
          }
        }
        
        swipe.startX = swipeStartX;
        swipe.startY = swipeStartY;
        swipe.endX = lastTouch.x;
        swipe.endY = lastTouch.y;
        swipe.duration = swipeDuration;
        swipe.isValid = true;
        
        Serial.printf("[Touch] Swipe detected: %s, distance: %d, duration: %dms\n",
                      swipe.direction == SWIPE_LEFT ? "LEFT" :
                      swipe.direction == SWIPE_RIGHT ? "RIGHT" :
                      swipe.direction == SWIPE_UP ? "UP" : "DOWN",
                      distance, swipeDuration);
      } else {
        Serial.printf("[Swipe] Distance too small: %d < %d\n", distance, SWIPE_MIN_DISTANCE);
      }
    } else {
      if (swipeDuration >= SWIPE_MAX_TIME) {
        Serial.printf("[Swipe] Too slow: %dms >= %dms\n", swipeDuration, SWIPE_MAX_TIME);
      }
      if (!lastTouch.isValid) {
        Serial.println("[Swipe] No valid lastTouch");
      }
    }
    
    // Reset swipe tracking
    swipeStarted = false;
  }
  
  return swipe;
}

void TouchHandler::resetSwipe() {
  swipeStarted = false;
}

// Navigation callback functions
void onStatusSectionTouch() {
  Serial.println("[Touch] Status section selected");
  // Add logic to switch to status section
}

void onWifiSectionTouch() {
  Serial.println("[Touch] WiFi section selected");
  // Add logic to switch to WiFi section
}

void onDisplaySectionTouch() {
  Serial.println("[Touch] Display section selected");
  // Add logic to switch to display section
}

void onBrightnessSectionTouch() {
  Serial.println("[Touch] Brightness section selected");
  // Add logic to adjust brightness
}

void onDebugSectionTouch() {
  Serial.println("[Touch] Debug section selected");
  // Add logic to switch to debug section
}

void onInfoSectionTouch() {
  Serial.println("[Touch] Info section selected");
  // Add logic to switch to info section
}

void onBackTouch() {
  Serial.println("[Touch] Back button pressed");
  // Add logic to go back
}

// Setup touch navigation areas
void setupTouchNavigation() {
  touchHandler.clearTouchAreas();
  
  // Add navigation button areas (adjust coordinates based on your UI)
  touchHandler.addTouchArea(10, 10, 60, 30, "Status", onStatusSectionTouch);
  touchHandler.addTouchArea(80, 10, 60, 30, "WiFi", onWifiSectionTouch);
  touchHandler.addTouchArea(150, 10, 60, 30, "Display", onDisplaySectionTouch);
  touchHandler.addTouchArea(220, 10, 80, 30, "Brightness", onBrightnessSectionTouch);
  touchHandler.addTouchArea(310, 10, 60, 30, "Debug", onDebugSectionTouch);
  touchHandler.addTouchArea(380, 10, 50, 30, "Info", onInfoSectionTouch);
  
  // Back button (bottom right)
  touchHandler.addTouchArea(420, 280, 50, 30, "Back", onBackTouch);
  
  Serial.printf("[Touch] Navigation setup complete with %d touch areas\\n", 
                touchHandler.touchAreaCount);
}

void handleTouchNavigation() {
  static bool touchStarted = false;
  static uint16_t startX = 0, startY = 0;
  static uint16_t maxX = 0, minX = 0; // Track extreme positions
  static uint32_t startTime = 0;
  static bool longPressDetected = false;
  
  TouchEvent touch = touchHandler.readTouch();
  
  if (touch.isValid && touch.pressed) {
    if (!touchStarted) {
      // Start of touch
      touchStarted = true;
      startX = touch.x;
      startY = touch.y;
      maxX = touch.x;
      minX = touch.x;
      startTime = millis();
      longPressDetected = false;
    } else {
      // Track extreme positions during touch
      if (touch.x > maxX) maxX = touch.x;
      if (touch.x < minX) minX = touch.x;
      
      // Check for long press (hold for 800ms without much movement)
      uint32_t holdTime = millis() - startTime;
      uint16_t movement = abs((int)touch.x - (int)startX) + abs((int)touch.y - (int)startY);
      
      if (!longPressDetected && holdTime > 800 && movement < 20) {
        longPressDetected = true;
        Serial.printf("[LongPress] Detected at (%d,%d) after %dms\n", touch.x, touch.y, holdTime);
        // Handle long press action here if needed
      }
    }
  } else if (touchStarted && !touch.pressed) {
    // End of touch - check for swipe (only if not long press)
    uint32_t duration = millis() - startTime;
    
    if (!longPressDetected) {
      // Calculate movement range
      uint16_t horizontalRange = maxX - minX;
      int16_t deltaX = maxX > startX + 15 ? (maxX - startX) : (minX < startX - 15 ? (minX - startX) : 0);
      
      // Only show debug for meaningful gestures (reduce spam)
      if (horizontalRange > 10 || duration > 50) {
        Serial.printf("[Swipe] Range:%d Delta:%d Time:%dms ", horizontalRange, deltaX, duration);
        
        // Much more sensitive thresholds
        if (horizontalRange > 25 && duration < 1000 && abs(deltaX) > 20) {
          bool swipeDetected = false;
          
          if (deltaX > 0) {
            // Swipe RIGHT
            Serial.print("RIGHT -> ");
            if (currentScreen == SCREEN_MAIN) {
              currentScreen = SCREEN_BENCH;
              Serial.println("BENCH");
              swipeDetected = true;
            } else if (currentScreen == SCREEN_CONFIG) {
              currentScreen = SCREEN_MAIN;
              Serial.println("MAIN");
              swipeDetected = true;
            } else {
              Serial.println("(no change)");
            }
          } else {
            // Swipe LEFT  
            Serial.print("LEFT -> ");
            if (currentScreen == SCREEN_MAIN) {
              currentScreen = SCREEN_CONFIG;
              Serial.println("CONFIG");
              swipeDetected = true;
            } else if (currentScreen == SCREEN_BENCH) {
              currentScreen = SCREEN_MAIN;
              Serial.println("MAIN");
              swipeDetected = true;
            } else {
              Serial.println("(no change)");
            }
          }
          
          // Screen clearing now handled by DisplayManager
          // if (swipeDetected) {
          //   display.fillScreen(TFT_BLACK);
          // }
        } else {
          if (horizontalRange > 10) Serial.println("(too small/slow)");
        }
      }
    }
    
    touchStarted = false;
  }
  
  // Handle regular touch areas for current screen
  if (touch.isValid && touch.pressed) {
    touchHandler.handleTouchAreas(touch.x, touch.y);
  }
}

void drawTouchButtons() {
  // Draw visible button outlines for navigation
  display.drawRect(10, 10, 60, 30, TFT_WHITE);
  display.drawString("Status", 15, 20);
  
  display.drawRect(80, 10, 60, 30, TFT_WHITE);
  display.drawString("WiFi", 85, 20);
  
  display.drawRect(150, 10, 60, 30, TFT_WHITE);
  display.drawString("Display", 155, 20);
  
  display.drawRect(220, 10, 80, 30, TFT_WHITE);
  display.drawString("Brightness", 225, 20);
  
  display.drawRect(310, 10, 60, 30, TFT_WHITE);
  display.drawString("Debug", 315, 20);
  
  display.drawRect(380, 10, 50, 30, TFT_WHITE);
  display.drawString("Info", 385, 20);
  
  display.drawRect(420, 280, 50, 30, TFT_WHITE);
  display.drawString("Back", 425, 290);
}