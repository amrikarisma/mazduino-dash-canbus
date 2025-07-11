#include "DisplayManager.h"
#include "Config.h"
#include "DataTypes.h"
#include "drawing_utils.h"
#include "NotoSansBold15.h"
#include "NotoSansBold36.h"
#include <EEPROM.h>
#if ENABLE_SIMULATOR
#include "Simulator.h"
#endif

TFT_eSPI display = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&display);

void setupDisplay() {
  display.init();
  display.setRotation(3);
}

void drawSplashScreenWithImage() {
  display.fillScreen(TFT_BLACK);
  display.loadFont(AA_FONT_LARGE);
  display.setTextColor(TFT_WHITE, TFT_BLACK);

  display.setTextDatum(TC_DATUM);
  int centerX = display.width() / 2;
  int centerY = (display.height() / 2) - 35;

  display.drawString("MAZDUINO Display", centerX, centerY);
  display.loadFont(AA_FONT_SMALL);
  display.drawString("Firmware version: " + String(version), centerX, centerY + 50);
  display.drawString("www.mazduino.com", centerX, 300);

  delay(5000);
}

void itemDraw(bool setup) {
  const char *labels[] = {"AFR", "TPS", "ADV", "MAP"};
  float values[] = {afrConv, tps, adv, mapData};
  float lastValues[] = {lastAfrConv, lastTps, lastAdv, lastMapData};
  int positions[][2] = {{5, 190}, {360, 190}, {120, 190}, {360, 10}};
  uint16_t colors[] = {(afrConv < 13.0) ? TFT_ORANGE : ((afrConv > 14.7) ? TFT_RED : TFT_GREEN), TFT_WHITE, TFT_RED, TFT_WHITE};

  for (int v = 0; v < 4; v++) {
    drawDataBox(positions[v][0], positions[v][1], labels[v], values[v], colors[v], lastValues[v], (v == 0) ? 1 : 0, setup);
    lastValues[v] = values[v];
  }

  if ((millis() - lazyUpdateTime > 1000) || setup) {
    const char *labelsLazy[4] = {"IAT", "Coolant", "Voltage", (EEPROM.read(0) == 1) ? "Trigger" : "FP"};
    float valuesLazy[4] = {iat, clt, bat, (EEPROM.read(0) == 1) ? triggerError : fp};
    float lastValuesLazy[4] = {lastIat, lastClt, lastBat, (EEPROM.read(0) == 1) ? lastTriggerError : lastFp};

    int positionsLazy[][2] = {{5, 10}, {5, 100}, {360, 100}, {240, 190}};
    uint16_t colorsLazy[] = {TFT_WHITE, (clt > 95) ? TFT_RED : TFT_WHITE,
                             ((bat < 11.5 || bat > 14.5) ? TFT_ORANGE : TFT_GREEN), TFT_WHITE};

    for (int l = 0; l < 4; l++) {
      drawDataBox(positionsLazy[l][0], positionsLazy[l][1], labelsLazy[l], valuesLazy[l], colorsLazy[l], lastValuesLazy[l], (l == 2) ? 1 : 0, setup);
      lastValuesLazy[l] = valuesLazy[l];
    }

    lazyUpdateTime = millis();
  }
  
  // Center buttons
  display.loadFont(AA_FONT_SMALL);
  const char *buttonLabels[] = {"SYNC", "FAN", "ASE", "WUE", "REV", "LCH", "AC", "DFCO"};
  bool buttonStates[] = {syncStatus, fan, ase, wue, rev, launch, airCon, dfco};
  for (int i = 0; i < 8; i++) {
    drawSmallButton((10 + 60 * i), 285, buttonLabels[i], buttonStates[i]);
  }
}

void startUpDisplay() {
  display.fillScreen(TFT_BLACK);
  display.loadFont(AA_FONT_SMALL);
  spr.setColorDepth(16);
  display.setTextColor(TFT_WHITE, TFT_BLACK);
  display.drawString("RPM", 190, 120);
  itemDraw(true);
  spr.loadFont(AA_FONT_LARGE);
  for (int i = 6000; i >= 0; i -= 250) {
    drawRPMBarBlocks(i);
    spr.createSprite(100, 50);
    spr_width = spr.textWidth("7777");
    spr.setTextColor(TFT_WHITE, TFT_BLACK, true);
    spr.setTextDatum(TR_DATUM);
    spr.drawNumber(i, 100, 5);
    spr.pushSprite(190, 140);
    spr.deleteSprite();
  }
}

void drawDataBox(int x, int y, const char *label, const float value, uint16_t labelColor, const float valueToCompare, const int decimal, bool setup) {
  const int BOX_WIDTH = 100;
  const int BOX_HEIGHT = 80;
  const int LABEL_HEIGHT = BOX_HEIGHT / 2;

  if (setup) {
    spr.loadFont(AA_FONT_SMALL);
    spr.createSprite(BOX_WIDTH, LABEL_HEIGHT);
    spr.setTextColor(labelColor, TFT_BLACK, true);
    spr.drawString(label, 50, 5);
    spr.setTextDatum(TC_DATUM);
    if (label == "AFR") {
      spr.pushSprite(x - 10, y);
    } else {
      spr.pushSprite(x, y);
    }
  }
  
  if (valueToCompare != value) {
    spr.loadFont(AA_FONT_LARGE);
    spr.createSprite(BOX_WIDTH, LABEL_HEIGHT);
    spr.setTextDatum(TC_DATUM);
    spr_width = spr.textWidth("333");
    spr.setTextColor(labelColor, TFT_BLACK, true);
    if (decimal > 0) {
      spr.drawFloat(value, decimal, 50, 5);
    } else {
      spr.drawNumber(value, 50, 5);
    }
    spr.pushSprite(x, y + LABEL_HEIGHT - 15);
    spr.deleteSprite();
  }
}

void drawData() {
  if (lastRpm != rpm) {
    drawRPMBarBlocks(rpm);
    spr.loadFont(AA_FONT_LARGE);
    spr.createSprite(100, 50);
    spr_width = spr.textWidth("7777");
    spr.setTextColor(TFT_WHITE, TFT_BLACK, true);
    spr.setTextDatum(TR_DATUM);
    spr.drawNumber(rpm, 100, 5);
    spr.pushSprite(190, 140);
    spr.deleteSprite();
    lastRpm = rpm;
  }
  itemDraw(false);
  
#if ENABLE_SIMULATOR
  // Draw simulator indicator if simulator is active
  static uint8_t lastSimMode = SIMULATOR_MODE_OFF;
  uint8_t currentSimMode = getSimulatorMode();
  
  if (currentSimMode != SIMULATOR_MODE_OFF) {
    display.loadFont(AA_FONT_SMALL);
    display.setTextColor(TFT_YELLOW, TFT_BLACK);
    display.setTextDatum(TR_DATUM);
    display.drawString("SIM", display.width() - 5, 5);
  } else if (lastSimMode != SIMULATOR_MODE_OFF) {
    // Clear the SIM indicator when simulator is turned off
    display.fillRect(display.width() - 30, 5, 25, 15, TFT_BLACK);
  }
  
  lastSimMode = currentSimMode;
#endif

#if ENABLE_DEBUG_MODE
  // Draw debug information at center top if debug mode is active
  static bool lastDebugMode = false;
  if (debugMode) {
    display.loadFont(AA_FONT_SMALL);
    display.setTextColor(TFT_CYAN, TFT_BLACK);
    display.setTextDatum(TC_DATUM);
    
    int centerX = display.width() / 2;
    
    // Create debug info string - show only essential info in one line
    String debugInfo = "CPU:" + String(cpuUsage, 1) + "% FPS:" + String(fps, 1) + " Heap:" + String(ESP.getFreeHeap()/1024) + "K";
    
    // Draw debug info at center top
    display.drawString(debugInfo, centerX, 5);
    
    lastDebugMode = true;
  } else {
    // Clear debug area when debug mode is turned off
    if (lastDebugMode) {
      // Clear the top center area where debug info was displayed
      display.fillRect(0, 5, display.width(), 20, TFT_BLACK);
      lastDebugMode = false;
    }
  }
#endif
}
