#ifndef KEYPAD_SCREEN_H
#define KEYPAD_SCREEN_H

#include <TFT_eSPI.h>

// ECU Features yang bisa diaktifkan
enum ECUFeature {
  FEATURE_START_STOP_ENGINE = 0,
  FEATURE_LAUNCH_CONTROL = 1,
  FEATURE_ANTILAG = 2,
  FEATURE_ROLLING_ANTILAG = 3,
  FEATURE_FLAT_SHIFT = 4,
  FEATURE_FAN = 5,
  FEATURE_BACK = 6,
  FEATURE_COUNT = 7
};

struct FeatureButton {
  uint16_t x, y, w, h;
  const char* label;
  ECUFeature feature;
  bool enabled; // Status toggle
};

class KeypadScreen {
private:
  TFT_eSPI* tft;
  FeatureButton buttons[FEATURE_COUNT];
  
  void initButtons();
  void drawButton(const FeatureButton& btn, bool pressed);
  void toggleFeature(ECUFeature feature);
  void sendCANCommand(ECUFeature feature, bool enabled);
  
public:
  KeypadScreen();
  void begin(TFT_eSPI* display);
  void draw(bool setup);
  void handleTouch(uint16_t x, uint16_t y);
  ECUFeature checkButtonPress(uint16_t x, uint16_t y);
};

extern KeypadScreen keypadScreen;

#endif // KEYPAD_SCREEN_H
