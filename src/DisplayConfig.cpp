#include "DisplayConfig.h"
#include "DataTypes.h"
#include "Config.h"
#include "SplashScreen.h"
#include <EEPROM.h>
#include <TFT_eSPI.h>

// GPS Speed smoothing variables
static float displayedGpsSpeed = 0.0;
static uint32_t lastGpsSpeedUpdate = 0;
static const float GPS_SPEED_SMOOTH_RATE = 3.0; // units per second max change rate

// Default display configuration
DisplayConfiguration defaultDisplayConfig = {
  // Default panels configuration - New layout to avoid RPM bar collision
  {
    {DATA_SOURCE_AFR, DATA_TYPE_FLOAT, 0, 1, true, "AFR", "", TFT_GREEN},      // Position 0: Left-Top
    {DATA_SOURCE_TPS, DATA_TYPE_INT, 1, 0, true, "TPS", "%", TFT_WHITE},       // Position 1: Left-Middle  
    {DATA_SOURCE_IAT, DATA_TYPE_UINT, 2, 0, true, "IAT", "°C", TFT_WHITE},     // Position 2: Left-Bottom
    {DATA_SOURCE_MAP, DATA_TYPE_INT, 3, 0, true, "MAP", "kPa", TFT_WHITE},     // Position 3: Right-Top
    {DATA_SOURCE_ADV, DATA_TYPE_INT, 4, 0, true, "ADV", "°", TFT_RED},         // Position 4: Right-Middle
    {DATA_SOURCE_FP, DATA_TYPE_INT, 5, 0, true, "FP", "psi", TFT_WHITE},       // Position 5: Right-Bottom
    {DATA_SOURCE_COOLANT, DATA_TYPE_UINT, 6, 0, true, "Coolant", "°C", TFT_WHITE}, // Position 6: Bottom-Left
    {DATA_SOURCE_VOLTAGE, DATA_TYPE_FLOAT, 7, 1, true, "Voltage", "V", TFT_GREEN}  // Position 7: Bottom-Right
  },
  // Default indicators configuration
  {
    {INDICATOR_SYNC, 0, true, "SYNC"},
    {INDICATOR_FAN, 1, true, "FAN"},
    {INDICATOR_ASE, 2, true, "ASE"},
    {INDICATOR_WUE, 3, true, "WUE"},
    {INDICATOR_REV, 4, true, "REV"},
    {INDICATOR_LCH, 5, true, "LCH"},
    {INDICATOR_AC, 6, true, "AC"},
    {INDICATOR_DFCO, 7, true, "DFCO"}
  },
  8, // activePanelCount
  8, // activeIndicatorCount
  0, // rpmDisplayMode (bar)
  true, // showSystemIndicators
  500000 // canSpeed default 500Kbps
};

DisplayConfiguration currentDisplayConfig;

void initializeDisplayConfig() {
  // Load configuration from EEPROM or use default
  loadDisplayConfig();
}

void saveDisplayConfig() {
  // Save to EEPROM using address defined in Config.h
  Serial.println("[DisplayConfig] Saving configuration to EEPROM...");
  for (int i = 0; i < 8; i++) {
    if (currentDisplayConfig.panels[i].enabled) {
      Serial.printf("[DisplayConfig] Panel %d: DataSource=%d (%s), Type=%d, Decimals=%d\n", 
                    i, currentDisplayConfig.panels[i].dataSource,
                    getDataSourceName(currentDisplayConfig.panels[i].dataSource),
                    currentDisplayConfig.panels[i].dataType,
                    currentDisplayConfig.panels[i].decimals);
    } else {
      Serial.printf("[DisplayConfig] Panel %d: DISABLED\n", i);
    }
  }
  
  EEPROM.put(EEPROM_DISPLAY_CONFIG_ADDR, currentDisplayConfig);
  EEPROM.commit();
  Serial.printf("[DisplayConfig] Configuration saved to EEPROM address %d\n", EEPROM_DISPLAY_CONFIG_ADDR);
}

void loadDisplayConfig() {
  // Try to load from EEPROM using address defined in Config.h
  DisplayConfiguration tempConfig;
  EEPROM.get(EEPROM_DISPLAY_CONFIG_ADDR, tempConfig);
  
  // Check if loaded config is valid
  bool isValid = true;
  
  // Basic validation
  if (tempConfig.activePanelCount > 8 || tempConfig.activeIndicatorCount > 8) {
    isValid = false;
  }
  
  // Validate data sources for each panel
  if (isValid) {
    for (int i = 0; i < 8; i++) {
      if (tempConfig.panels[i].enabled && 
          tempConfig.panels[i].dataSource >= DATA_SOURCE_COUNT) {
        Serial.printf("Invalid data source %d in panel %d\n", tempConfig.panels[i].dataSource, i);
        isValid = false;
        break;
      }
    }
  }
  
  if (isValid) {
    currentDisplayConfig = tempConfig;
    Serial.println("[DisplayConfig] Configuration loaded from EEPROM");
    for (int i = 0; i < 8; i++) {
      if (currentDisplayConfig.panels[i].enabled) {
        Serial.printf("[DisplayConfig] Loaded Panel %d: DataSource=%d (%s)\n", 
                      i, currentDisplayConfig.panels[i].dataSource,
                      getDataSourceName(currentDisplayConfig.panels[i].dataSource));
      }
    }
  } else {
    // Use default configuration
    currentDisplayConfig = defaultDisplayConfig;
    Serial.println("[DisplayConfig] Using default display configuration - EEPROM data invalid");
  }
}

void resetDisplayConfigToDefault() {
  currentDisplayConfig = defaultDisplayConfig;
  saveDisplayConfig();
  
  // Also reset splash screen to default (Mazduino)
  setSplashScreenSelection(DEFAULT_SPLASH_SCREEN);
  
  Serial.println("Display configuration reset to default");
  Serial.println("Splash screen reset to Mazduino");
}

float getSmoothGpsSpeed() {
  uint32_t currentTime = millis();
  float deltaTime = (currentTime - lastGpsSpeedUpdate) / 1000.0; // Convert to seconds
  
  if (lastGpsSpeedUpdate == 0) {
    // First update, initialize
    displayedGpsSpeed = gpsSpeed;
    lastGpsSpeedUpdate = currentTime;
    return displayedGpsSpeed;
  }
  
  float speedDifference = gpsSpeed - displayedGpsSpeed;
  float maxChange = GPS_SPEED_SMOOTH_RATE * deltaTime;
  
  // Limit the change rate
  if (speedDifference > maxChange) {
    displayedGpsSpeed += maxChange;
  } else if (speedDifference < -maxChange) {
    displayedGpsSpeed -= maxChange;
  } else {
    displayedGpsSpeed = gpsSpeed;
  }
  
  // Ensure displayed speed doesn't go negative
  if (displayedGpsSpeed < 0.0) {
    displayedGpsSpeed = 0.0;
  }
  
  lastGpsSpeedUpdate = currentTime;
  return displayedGpsSpeed;
}

float getDataValue(uint8_t dataSource) {
  switch (dataSource) {
    case DATA_SOURCE_IAT:
      return (float)iat;
    case DATA_SOURCE_COOLANT:
      return (float)clt;
    case DATA_SOURCE_AFR:
      return afrConv;
    case DATA_SOURCE_ADV:
      return (float)adv;
    case DATA_SOURCE_TRIGGER:
      return (float)triggerError;
    case DATA_SOURCE_TPS:
      return (float)tps;
    case DATA_SOURCE_VOLTAGE:
      return bat;
    case DATA_SOURCE_MAP:
      return (float)mapData;
    case DATA_SOURCE_RPM:
      return (float)rpm;
    case DATA_SOURCE_FP:
      return (float)fp;
    case DATA_SOURCE_VSS:
      // Use GPS speed if available and valid, otherwise use ECU VSS
      if (gpsEnabled && gpsDataValid) {
        return getSmoothGpsSpeed();
      }
      return (float)vss;
    case DATA_SOURCE_GPS_SPEED:
      return getSmoothGpsSpeed();
    case DATA_SOURCE_GPS_HEADING:
      return gpsHeading;
    case DATA_SOURCE_GPS_ALTITUDE:
      return gpsAltitude;
    case DATA_SOURCE_GPS_SATELLITES:
      return (float)gpsNumSats;
    case DATA_SOURCE_AC_TEMP:
      return acCurrentTemp;
    default:
      return 0.0;
  }
}

bool getIndicatorValue(uint8_t indicator) {
  switch (indicator) {
    case INDICATOR_SYNC:
      return syncStatus;
    case INDICATOR_FAN:
      return fan;
    case INDICATOR_ASE:
      return ase;
    case INDICATOR_WUE:
      return wue;
    case INDICATOR_REV:
      return rev;
    case INDICATOR_LCH:
      return launch;
    case INDICATOR_AC:
      return airCon;
    case INDICATOR_DFCO:
      return dfco;
    case INDICATOR_GPS:
      return gpsEnabled && gpsDataValid;
    default:
      return false;
  }
}

const char* getDataSourceName(uint8_t dataSource) {
  switch (dataSource) {
    case DATA_SOURCE_IAT: return "IAT";
    case DATA_SOURCE_COOLANT: return "Coolant";
    case DATA_SOURCE_AFR: return "AFR";
    case DATA_SOURCE_ADV: return "ADV";
    case DATA_SOURCE_TRIGGER: return "Trigger";
    case DATA_SOURCE_TPS: return "TPS";
    case DATA_SOURCE_VOLTAGE: return "Voltage";
    case DATA_SOURCE_MAP: return "MAP";
    case DATA_SOURCE_RPM: return "RPM";
    case DATA_SOURCE_FP: return "FP";
    case DATA_SOURCE_VSS: return "VSS";
    case DATA_SOURCE_GPS_SPEED: return "GPS Speed";
    case DATA_SOURCE_GPS_HEADING: return "GPS Head";
    case DATA_SOURCE_GPS_ALTITUDE: return "GPS Alt";
    case DATA_SOURCE_GPS_SATELLITES: return "GPS Sats";
    case DATA_SOURCE_AC_TEMP: return "AC Temp";
    default: return "Unknown";
  }
}

const char* getIndicatorName(uint8_t indicator) {
  switch (indicator) {
    case INDICATOR_SYNC: return "SYNC";
    case INDICATOR_FAN: return "FAN";
    case INDICATOR_ASE: return "ASE";
    case INDICATOR_WUE: return "WUE";
    case INDICATOR_REV: return "REV";
    case INDICATOR_LCH: return "LCH";
    case INDICATOR_AC: return "AC";
    case INDICATOR_DFCO: return "DFCO";
    case INDICATOR_GPS: return "GPS";
    default: return "Unknown";
  }
}

uint16_t getDataSourceColor(uint8_t dataSource, float value) {
  switch (dataSource) {
    // case DATA_SOURCE_AFR:
    //   return (value < 13.0) ? TFT_ORANGE : ((value > 14.7) ? TFT_RED : TFT_GREEN);
    // case DATA_SOURCE_COOLANT:
    //   return (value > 95) ? TFT_RED : TFT_WHITE;
    // case DATA_SOURCE_VOLTAGE:
    //   return (value < 11.5 || value > 14.5) ? TFT_ORANGE : TFT_GREEN;
    // case DATA_SOURCE_ADV:
    //   return TFT_RED;
    // case DATA_SOURCE_AC_TEMP:
    //   // AC temperature color coding: Cold=Blue, Normal=Cyan, Hot=Red
    //   if (value < 20) return TFT_BLUE;
    //   else if (value > 35) return TFT_RED;
    //   else return TFT_CYAN;
    default:
      return TFT_WHITE;
  }
}

uint32_t getCanSpeed() {
  if (currentDisplayConfig.canSpeed == 500000 || currentDisplayConfig.canSpeed == 1000000) {
    return currentDisplayConfig.canSpeed;
  } else {
    // Jika belum pernah di-set atau nilai tidak valid, pakai default 500000
    return 500000;
  }
}

void setCanSpeed(uint32_t speed) {
  currentDisplayConfig.canSpeed = speed;
  saveDisplayConfig();
}

uint8_t getCanProtocol() {
  return canProtocol;
}

void setCanProtocol(uint8_t protocol) {
  canProtocol = protocol;
  
  // Save to EEPROM
  EEPROM.write(EEPROM_CAN_PROTOCOL_ADDR, protocol);
  EEPROM.commit();
  
  Serial.printf("CAN protocol set to: %s\n", (protocol == CAN_PROTOCOL_HALTECH) ? "Haltech" : "RusEFI");
}

void loadCanProtocol() {
  uint8_t storedProtocol = EEPROM.read(EEPROM_CAN_PROTOCOL_ADDR);
  
  // Check if it's a valid protocol value
  if (storedProtocol == CAN_PROTOCOL_HALTECH || storedProtocol == CAN_PROTOCOL_RUSEFI) {
    canProtocol = storedProtocol;
    Serial.printf("CAN protocol loaded: %s\n", (canProtocol == CAN_PROTOCOL_HALTECH) ? "Haltech" : "RusEFI");
  } else {
    canProtocol = DEFAULT_CAN_PROTOCOL;
    Serial.printf("Using default CAN protocol: %s\n", (canProtocol == CAN_PROTOCOL_HALTECH) ? "Haltech" : "RusEFI");
  }
}
