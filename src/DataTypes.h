#ifndef DATATYPES_H
#define DATATYPES_H

#include <stdint.h>

// Global variables for ECU data
extern uint8_t iat, clt;
extern uint8_t refreshRate;
extern unsigned int rpm, lastRpm, vss;
extern int mapData, tps, adv, fp, triggerError;
extern float bat, afrConv;
extern bool syncStatus, fan, ase, wue, rev, launch, airCon, dfco;

// Additional variables for RusEFI protocol
extern float oilPressure, oilTemp, fuelLevel, fuelTemp;
extern float ignitionTiming, injDuty, ignDuty, flexPct;
extern float pps, tps1, tps2, wastegate;
extern float aux1Temp, aux2Temp, mcuTemp;
extern float lam1, lam2, fpLow, fpHigh;
extern uint8_t currentGear;
extern uint16_t warningCounter, lastError, distanceTraveled;
extern bool revLimAct, mainRelayAct, fuelPumpAct, celAct, egoHeatAct, lambdaProtectAct, fan2;

// Last values for comparison
extern int lastIat, lastClt, lastTps, lastAdv, lastMapData, lastFp, lastTriggerError;
extern float lastBat, lastAfrConv;
extern unsigned int lastRefreshRate;

// System variables
extern bool first_run;
extern uint32_t lastPrintTime;
extern uint32_t startupTime;
extern uint32_t lazyUpdateTime;
extern uint16_t spr_width;
extern uint8_t backlightBrightness;
extern uint8_t speeduinoDataMode; // 0 = Mode A (simple), 1 = Mode N (enhanced)

// Communication variables
extern int commMode;
extern uint8_t canProtocol;  // 0 = Haltech, 1 = RusEFI
extern bool sent, received;
extern bool isCANMode;  // true for CAN, false for Serial

// WiFi variables
extern bool wifiActive;
extern uint32_t lastClientCheck;
extern uint32_t lastClientCheckTimeout;
extern uint32_t wifiTimeout;
extern bool clientConnected;

// Debug variables
extern bool debugMode;
extern float cpuUsage;
extern float fps;
extern uint32_t frameCount;
extern uint32_t lastFpsUpdate;
extern uint32_t lastCpuMeasure;
extern uint32_t loopStartTime;

// Screen management
extern uint8_t currentScreen;

#endif // DATATYPES_H
