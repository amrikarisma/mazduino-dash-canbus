#include "drawing_utils.h"
#include <TFT_eSPI.h>
#include "Roboto21.h"
#include "Roboto16.h"

// External display object
extern TFT_eSPI display;

// ================= CONFIG =================
#define NUM_BLOCKS        60
#define BLOCK_WIDTH       6
#define BLOCK_SPACING     2

#define BLOCK_HEIGHT_MIN  60
#define BLOCK_HEIGHT_MAX  80

#define START_X           0

#define Y_BOTTOM          160
#define Y_TOP             25

#define AGGRESSIVE_RPM_END 4000
// ==========================================

#define BORDER_COLOR     TFT_LIGHTGREY
#define TICK_COLOR       TFT_LIGHTGREY
#define TICK_HEIGHT      6
#define BORDER_THICKNESS 5
#define BORDER_SPACING   3  // Gap between border and blocks
#define LABEL_Y (Y_TOP - 22)

// ===== GLOBAL STATE VARIABLES =====
static bool labelsDrawn = false;
static uint32_t lastLabelUpdate = 0;
static int lastFilledBlocks = -1;
static uint32_t lastDrawUpdate = 0;
static bool firstRun = true;

// ---------- UI ----------
void drawCenteredTextSmall(int x, int y, int w, int h,
                           const char* text, int textSize, uint16_t color) {
  display.loadFont(AA_FONT_SMALL);
  display.setTextDatum(MC_DATUM);
  display.setTextColor(color, TFT_BLACK);
  display.drawString(text, x, y);
  display.unloadFont();
}

void drawSmallButton(int x, int y, const char* label, bool value) {
  const int BTN_WIDTH = 50;
  const int BTN_HEIGHT = 25;
  uint16_t activeColor = (label == "REV" || label == "LCH") ? TFT_RED : TFT_GREEN;
  uint16_t fillColor = value ? activeColor : TFT_WHITE;
  display.drawRoundRect(x, y, BTN_WIDTH, BTN_HEIGHT, 5, fillColor);
  drawCenteredTextSmall(x + BTN_WIDTH / 2, y + BTN_HEIGHT / 2,
                        BTN_WIDTH, BTN_HEIGHT, label, 1, fillColor);
}

void resetDrawingUtils() {
  labelsDrawn = false;
  lastLabelUpdate = 0;
  lastFilledBlocks = -1;
  lastDrawUpdate = 0;
  firstRun = true;
}

// ---------- SMOOTH ----------
float smoothStep(float t) {
  return t * t * (3.0f - 2.0f * t);
}

// ================= Y CURVE =================
float rpmCurveY(int index, int maxRPM) {
  float rpm = (float)index / (NUM_BLOCKS - 1) * maxRPM;

  if (rpm <= AGGRESSIVE_RPM_END) {
    float x = rpm / AGGRESSIVE_RPM_END;
    x = powf(x, 0.55f);
    x = smoothStep(x);
    return x;
  }
  return 1.0f;
}

// ================= HEIGHT CURVE =================
float rpmCurveHeight(int index, int maxRPM) {
  float rpm = (float)index / (NUM_BLOCKS - 1) * maxRPM;
  float t = rpm / maxRPM;
  t = powf(t, 0.75f);
  t = smoothStep(t);
  return constrain(t, 0.0f, 1.0f);
}

// ================= POSITION =================
int getRPMBlockY(int index, int maxRPM) {
  float t = rpmCurveY(index, maxRPM);
  return Y_BOTTOM - t * (Y_BOTTOM - Y_TOP);
}

int getRPMBlockHeight(int index, int maxRPM) {
  float t = rpmCurveHeight(index, maxRPM);
  return BLOCK_HEIGHT_MIN +
         t * (BLOCK_HEIGHT_MAX - BLOCK_HEIGHT_MIN);
}

// ================= RPM → BLOCK =================
int findBlockIndexByRPM(int rpmLabel, int maxRPM) {
  int bestIndex = 0;
  int bestDiff = 999999;

  for (int i = 0; i < NUM_BLOCKS; i++) {
    int rpmAtBlock = (float)i / (NUM_BLOCKS - 1) * maxRPM;
    int diff = abs(rpmAtBlock - rpmLabel);

    if (diff < bestDiff) {
      bestDiff = diff;
      bestIndex = i;
    }
  }
  return bestIndex;
}

// ================= BLOCK → X =================
inline int blockIndexToX(int block) {
  return START_X +
         block * (BLOCK_WIDTH + BLOCK_SPACING) +
         BLOCK_WIDTH / 2;
}

// ================= LABEL =================
void drawRPMLabels(int maxRPM) {
  if (!labelsDrawn || firstRun || millis() - lastLabelUpdate > 2500) {

    // Clear label area dynamically
    display.fillRect(
      START_X,
      Y_TOP - 30, // Clear area above the highest blocks
      (BLOCK_WIDTH + BLOCK_SPACING) * NUM_BLOCKS,
      25,
      TFT_BLACK
    );

    display.loadFont(Roboto21);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setTextDatum(MC_DATUM);

    for (int rpmLabel = 1000; rpmLabel < maxRPM; rpmLabel += 1000) {
      int block = findBlockIndexByRPM(rpmLabel, maxRPM);
      int blockY = getRPMBlockY(block, maxRPM);
      int blockH = getRPMBlockHeight(block, maxRPM);
      int actualY = blockY + (BLOCK_HEIGHT_MAX - blockH); // Same positioning as drawRPMBarBlocks
      int labelY = actualY - 15; // 15px above the actual block position
      
      display.drawString(String(rpmLabel / 1000),
                         blockIndexToX(block),
                         labelY);
    }

    display.unloadFont();
    labelsDrawn = true;
    lastLabelUpdate = millis();
  }
}

// ================= TICKS =================
void drawRPMTicks(int maxRPM) {
  for (int rpm = 1000; rpm < maxRPM; rpm += 1000) {
    int block = findBlockIndexByRPM(rpm, maxRPM);
    int x = blockIndexToX(block);
    int blockY = getRPMBlockY(block, maxRPM);
    int blockH = getRPMBlockHeight(block, maxRPM);
    int actualY = blockY + (BLOCK_HEIGHT_MAX - blockH); // Same positioning as drawRPMBarBlocks

    // Top tick - above the actual block position
    display.drawFastVLine(x,
                          actualY - TICK_HEIGHT - 2,
                          TICK_HEIGHT,
                          TICK_COLOR);

    // Bottom tick - below the actual block position
    display.drawFastVLine(x,
                          actualY + blockH + 2,
                          TICK_HEIGHT,
                          TICK_COLOR);
  }
}

// ================= MAIN DRAW =================
void drawRPMBarBlocks(int rpm, int maxRPM) {
  if (millis() - lastDrawUpdate < 60) return;
  lastDrawUpdate = millis();

  int filledBlocks = 0;
  if (rpm > 0) {
    filledBlocks = findBlockIndexByRPM(rpm, maxRPM) + 1;
    filledBlocks = constrain(filledBlocks, 0, NUM_BLOCKS);
  }

  if (firstRun) {
    display.fillRect(
      START_X - 10,
      Y_TOP - 50,
      (BLOCK_WIDTH + BLOCK_SPACING) * NUM_BLOCKS + 20,
      (Y_BOTTOM - Y_TOP) + BLOCK_HEIGHT_MAX + 80,
      TFT_BLACK
    );

    drawRPMLabels(maxRPM);
    drawRPMTicks(maxRPM);
    firstRun = false;
  }

  for (int i = 0; i < NUM_BLOCKS; i++) {
    int x = START_X + i * (BLOCK_WIDTH + BLOCK_SPACING);
    int y = getRPMBlockY(i, maxRPM);
    int h = getRPMBlockHeight(i, maxRPM);

    uint16_t color = TFT_LIGHTGREY;
    if (rpm > 0 && i < filledBlocks) {
      float ratio = (float)i / NUM_BLOCKS;
      if (ratio < 0.75f)       color = TFT_ORANGE;
      else                    color = TFT_RED;
    }

    if (lastFilledBlocks == -1 ||
        (i < filledBlocks) != (i < lastFilledBlocks)) {

      display.fillRect(
        x,
        y + (BLOCK_HEIGHT_MAX - h),
        BLOCK_WIDTH,
        h,
        color
      );
    }
  }

  lastFilledBlocks = filledBlocks;
}