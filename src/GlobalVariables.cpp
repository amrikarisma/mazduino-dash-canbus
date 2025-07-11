#include "DataTypes.h"
#include "Config.h"

// Configuration variables from Config.h
const char *version = "1.2";
const char *ssid = "MAZDUINO_Display";
const char *password = "12345678";

// Global variables for ECU data
uint8_t iat = 0, clt = 0;
uint8_t refreshRate = 0;
unsigned int rpm = 0, lastRpm, vss = 0;
int mapData, tps, adv, fp, triggerError = 0;
float bat = 0.0, afrConv = 0.0;
bool syncStatus, fan, ase, wue, rev, launch, airCon, dfco;

// Last values for comparison
int lastIat = -1, lastClt = -1, lastTps = -1, lastAdv = -1, lastMapData = -1, lastFp = -1, lastTriggerError = -1;
float lastBat = -1, lastAfrConv = -1;
unsigned int lastRefreshRate = -1;

// System variables
bool first_run = true;
uint32_t lastPrintTime = 0;
uint32_t startupTime;
uint32_t lazyUpdateTime;
uint16_t spr_width = 0;

// Communication variables
int commMode = COMM_CAN;
bool sent = false, received = true;
bool isCANMode = true;  // Default to CAN mode

// WiFi variables
bool wifiActive = true;
uint32_t lastClientCheck = 0;
uint32_t lastClientCheckTimeout = 0;
uint32_t wifiTimeout = 30000;
bool clientConnected = true;

// Debug variables
bool debugMode = false;
float cpuUsage = 0.0;
float fps = 0.0;
uint32_t frameCount = 0;
uint32_t lastFpsUpdate = 0;
uint32_t lastCpuMeasure = 0;
uint32_t loopStartTime = 0;
