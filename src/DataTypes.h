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

// Communication variables
extern int commMode;
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

#endif // DATATYPES_H
