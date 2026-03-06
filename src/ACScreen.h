#ifndef AC_SCREEN_H
#define AC_SCREEN_H

#include <TFT_eSPI.h>

// AC button types
enum ACButton {
  AC_BTN_BACK = 0,
  AC_BTN_TEMP_UP = 1,
  AC_BTN_TEMP_DOWN = 2,
  AC_BTN_SEND = 3,
  AC_BTN_COUNT = 4
};

// AC button structure
struct ACButtonDef {
  uint16_t x, y, w, h;
  const char* label;
  uint16_t color;
  ACButton button;
};

class ACScreen {
private:
  TFT_eSPI* tft;
  ACButtonDef buttons[AC_BTN_COUNT];
  float acCutoffTemp;
  bool tempChanged;
  
  // Cache untuk smooth rendering (prevent flickering)
  float cachedACTemp;
  bool cachedACDataReceived;
  float cachedCutoffTemp;
  bool labelsDrawn;
  uint32_t lastACInfoUpdate;
  
  void initButtons();
  void drawButton(const ACButtonDef& btn, bool highlighted);
  void drawACInfo();
  void loadACCutoffTemp();
  void saveACCutoffTemp();
  
public:
  ACScreen();
  void begin(TFT_eSPI* display);
  void draw(bool setup);
  void update();
  void handleTouch(uint16_t x, uint16_t y);
  ACButton checkButtonPress(uint16_t x, uint16_t y);
  
  float getACCutoffTemp() const { return acCutoffTemp; }
  void setACCutoffTemp(float temp);
};

// Global instance
extern ACScreen acScreen;

// Functions to save/load AC cutoff temperature
void loadACCutoffFromEEPROM();
void saveACCutoffToEEPROM(float temp);
float getACCutoffTemp();

#endif // AC_SCREEN_H
