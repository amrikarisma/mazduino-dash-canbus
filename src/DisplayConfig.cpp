#include "DisplayConfig.h"
#include "DataTypes.h"
#include "Config.h"
#include <EEPROM.h>
#include <TFT_eSPI.h>

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
  // Save to EEPROM starting from address 10 (avoid conflict with existing settings)
  EEPROM.put(10, currentDisplayConfig);
  EEPROM.commit();
  Serial.println("Display configuration saved to EEPROM");
}

void loadDisplayConfig() {
  // Try to load from EEPROM
  DisplayConfiguration tempConfig;
  EEPROM.get(10, tempConfig);
  
  // Check if loaded config is valid (simple validation)
  if (tempConfig.activePanelCount <= 8 && tempConfig.activeIndicatorCount <= 8) {
    currentDisplayConfig = tempConfig;
    Serial.println("Display configuration loaded from EEPROM");
  } else {
    // Use default configuration
    currentDisplayConfig = defaultDisplayConfig;
    Serial.println("Using default display configuration");
  }
}

void resetDisplayConfigToDefault() {
  currentDisplayConfig = defaultDisplayConfig;
  saveDisplayConfig();
  Serial.println("Display configuration reset to default");
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
      return (float)vss;
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
    default: return "Unknown";
  }
}

uint16_t getDataSourceColor(uint8_t dataSource, float value) {
  switch (dataSource) {
    case DATA_SOURCE_AFR:
      return (value < 13.0) ? TFT_ORANGE : ((value > 14.7) ? TFT_RED : TFT_GREEN);
    case DATA_SOURCE_COOLANT:
      return (value > 95) ? TFT_RED : TFT_WHITE;
    case DATA_SOURCE_VOLTAGE:
      return (value < 11.5 || value > 14.5) ? TFT_ORANGE : TFT_GREEN;
    case DATA_SOURCE_ADV:
      return TFT_RED;
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
