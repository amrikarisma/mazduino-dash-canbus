#ifndef MENU_SCREEN_H
#define MENU_SCREEN_H

#include <TFT_eSPI.h>

// Menu items
enum MenuItem {
  MENU_BACK = 0,
  MENU_CONFIG = 1,
  MENU_BENCH = 2,
  MENU_AC_CONTROL = 3,
  MENU_ITEM_COUNT = 4
};

// Menu button structure
struct MenuButton {
  uint16_t x, y, w, h;
  const char* label;
  uint16_t color;
  MenuItem item;
};

class MenuScreen {
private:
  TFT_eSPI* tft;
  MenuButton menuButtons[MENU_ITEM_COUNT];
  int selectedItem;
  
  void initMenuButtons();
  void drawButton(const MenuButton& btn, bool highlighted);
  
public:
  MenuScreen();
  void begin(TFT_eSPI* display);
  void draw(bool setup);
  void handleTouch(uint16_t x, uint16_t y);
  MenuItem checkButtonPress(uint16_t x, uint16_t y);
};

// Global instance
extern MenuScreen menuScreen;

#endif // MENU_SCREEN_H
