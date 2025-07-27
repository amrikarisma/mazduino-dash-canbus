#ifndef DISPLAY_CONFIG_H
#define DISPLAY_CONFIG_H

#include <stdint.h>

// Data types for display
enum DisplayDataType {
  DATA_TYPE_FLOAT,
  DATA_TYPE_INT,
  DATA_TYPE_UINT,
  DATA_TYPE_BOOL
};

// Data sources available
enum DataSource {
  DATA_SOURCE_IAT,
  DATA_SOURCE_COOLANT,
  DATA_SOURCE_AFR,
  DATA_SOURCE_ADV,
  DATA_SOURCE_TRIGGER,
  DATA_SOURCE_TPS,
  DATA_SOURCE_VOLTAGE,
  DATA_SOURCE_MAP,
  DATA_SOURCE_RPM,
  DATA_SOURCE_FP,
  DATA_SOURCE_VSS,
  DATA_SOURCE_COUNT
};

// Indicator sources
enum IndicatorSource {
  INDICATOR_SYNC,
  INDICATOR_FAN,
  INDICATOR_ASE,
  INDICATOR_WUE,
  INDICATOR_REV,
  INDICATOR_LCH,
  INDICATOR_AC,
  INDICATOR_DFCO,
  INDICATOR_COUNT
};

// Display panel configuration
struct DisplayPanel {
  uint8_t dataSource;     // DataSource enum
  uint8_t dataType;       // DisplayDataType enum
  uint8_t position;       // Position on screen (0-7)
  uint8_t decimals;       // Number of decimal places for float
  bool enabled;           // Whether to display this panel
  char label[10];         // Label to display
  char unit[5];           // Unit string (°C, V, etc.)
  uint16_t color;         // Text color
};

// Indicator configuration
struct IndicatorConfig {
  uint8_t indicator;      // IndicatorSource enum
  uint8_t position;       // Position in indicator bar (0-7)
  bool enabled;           // Whether to display this indicator
  char label[6];          // Label to display
};

// Main display configuration
struct DisplayConfiguration {
  DisplayPanel panels[8];           // 8 data panels
  IndicatorConfig indicators[8];    // 8 indicators
  uint8_t activePanelCount;         // Number of active panels
  uint8_t activeIndicatorCount;     // Number of active indicators
  uint8_t rpmDisplayMode;           // RPM display mode (0=bar, 1=digital)
  bool showSystemIndicators;        // Show CAN/SER, DEBUG, SIM
  uint32_t canSpeed;                // CAN speed in bps (e.g. 500000, 1000000)
};

// Default configuration
extern DisplayConfiguration defaultDisplayConfig;
extern DisplayConfiguration currentDisplayConfig;

// Function prototypes
void initializeDisplayConfig();
void saveDisplayConfig();
void loadDisplayConfig();
void resetDisplayConfigToDefault();
float getDataValue(uint8_t dataSource);
bool getIndicatorValue(uint8_t indicator);
const char* getDataSourceName(uint8_t dataSource);
const char* getIndicatorName(uint8_t indicator);
uint16_t getDataSourceColor(uint8_t dataSource, float value);
// New CAN speed accessors
uint32_t getCanSpeed();
void setCanSpeed(uint32_t speed);

#endif // DISPLAY_CONFIG_H
