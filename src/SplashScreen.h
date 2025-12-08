#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <TFT_eSPI.h>

// Function declarations
void showAnimatedSplashScreen();
void drawFadeInBackground(int centerX, int centerY);
void drawFadeInTitle(int centerX, int centerY);
void drawFadeInSubtitle(int centerX, int centerY);
void drawFadeInWebsite(int centerX, int centerY);
void drawPulsingTitle(int centerX, int centerY);
void drawLoadingBar(int centerX, int centerY);
void drawFadeOutTransition();

// Splash screen selection functions
int getSplashScreenSelection();
void setSplashScreenSelection(int selection);

#endif // SPLASHSCREEN_H
