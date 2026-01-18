#include "DataTypes.h"
#include "Config.h"
#include "version.h"

// Configuration variables from Config.h
#ifdef VERSION_STRING
const char *version = VERSION_STRING;
#else
const char *version = "1.3.0-touch.dev";
#endif
const char *ssid = DEFAULT_SSID;
const char *password = DEFAULT_PASSWORD;

// Global variables for ECU data
uint8_t iat = 0, clt = 0;
uint8_t refreshRate = 0;
unsigned int rpm = 0, lastRpm, vss = 0;
int mapData, tps, adv, fp, triggerError = 0;
float bat = 0.0, afrConv = 0.0;
bool syncStatus, fan, ase, wue, rev, launch, airCon, dfco;

// Additional variables for RusEFI protocol
float oilPressure = 0.0, oilTemp = 0.0, fuelLevel = 0.0, fuelTemp = 0.0;
float ignitionTiming = 0.0, injDuty = 0.0, ignDuty = 0.0, flexPct = 0.0;
float pps = 0.0, tps1 = 0.0, tps2 = 0.0, wastegate = 0.0;
float aux1Temp = 0.0, aux2Temp = 0.0, mcuTemp = 0.0;
float lam1 = 0.0, lam2 = 0.0, fpLow = 0.0, fpHigh = 0.0;
uint8_t currentGear = 0;
uint16_t warningCounter = 0, lastError = 0, distanceTraveled = 0;
bool revLimAct = false, mainRelayAct = false, fuelPumpAct = false, celAct = false, egoHeatAct = false, lambdaProtectAct = false, fan2 = false;

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
uint8_t backlightBrightness = BACKLIGHT_BRIGHTNESS;
uint8_t speeduinoDataMode = DEFAULT_SPEEDUINO_MODE; // Default to enhanced mode

// Communication variables
int commMode = COMM_CAN;
uint8_t canProtocol = DEFAULT_CAN_PROTOCOL;  // 0 = Haltech, 1 = RusEFI
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

// Screen management
uint8_t currentScreen = SCREEN_MAIN;

// GPS data variables
bool gpsEnabled = false;
bool gpsDataValid = false;
float gpsSpeed = 0.0;      // Vehicle speed from GPS in km/h
double gpsLatitude = 0.0;  // GPS latitude in degrees
double gpsLongitude = 0.0; // GPS longitude in degrees
float gpsHeading = 0.0;    // GPS heading in degrees (0-359)
float gpsAltitude = 0.0;   // GPS altitude in meters
uint8_t gpsNumSats = 0;    // Number of GPS satellites
uint32_t gpsLastUpdate = 0; // Last GPS data update timestamp

// AC Controller data variables (ESP-NOW)
bool acControllerEnabled = true;  // Default enabled
bool acDataReceived = false;
float acCurrentTemp = 0.0;
uint32_t acLastUpdate = 0;
