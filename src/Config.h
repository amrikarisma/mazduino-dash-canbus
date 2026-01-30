#ifndef CONFIG_H
#define CONFIG_H

// Firmware version
extern const char *version;

// WiFi Configuration
#define DEFAULT_SSID "MAZDUINO_Display"
#define DEFAULT_PASSWORD "12345678"
extern const char *ssid;
extern const char *password;

// Pin definitions
#define UART_BAUD 115200
#define RXD 25  // Changed from 16 to avoid conflict with CAN TX
#define TXD 26  // Changed from 17 to avoid conflict with CAN RX

// Backlight control
#define BACKLIGHT_PIN 32
#define BACKLIGHT_CHANNEL 0
#define BACKLIGHT_FREQ 5000
#define BACKLIGHT_RESOLUTION 8
#define BACKLIGHT_BRIGHTNESS 100 // 0-255 (0 = off, 255 = max brightness)

// Communication modes
#define COMM_CAN 0
#define COMM_SERIAL 1

// CAN Protocol types
#define CAN_PROTOCOL_HALTECH 0
#define CAN_PROTOCOL_RUSEFI 1
#define DEFAULT_CAN_PROTOCOL CAN_PROTOCOL_HALTECH

// RusEFI CAN IDs - Standard Format (11-bit)
#define RUSEFI_ID_0x200 0x200  // Warning Counter, Last Error, Status Bits, Current Gear, Distance
#define RUSEFI_ID_0x201 0x201  // RPM, Ignition Timing, Injection Duty, Ignition Duty, Vehicle Speed, Flex%
#define RUSEFI_ID_0x202 0x202  // PPS, TPS1, TPS2, Wastegate
#define RUSEFI_ID_0x203 0x203  // MAP, Coolant Temp, Intake Temp, AUX1 Temp, AUX2 Temp, MCU Temp, Fuel Level
#define RUSEFI_ID_0x204 0x204  // Oil Pressure, Oil Temperature, Fuel Temperature, Battery Voltage
#define RUSEFI_ID_0x207 0x207  // Lambda 1, Lambda 2, Fuel Pressure Low, Fuel Pressure High

// RusEFI Extended CAN IDs (29-bit) - Based on actual RusEFI implementation
#define RUSEFI_GDI4_BASE_ADDRESS 0xBB20
#define RUSEFI_BENCH_TEST_BASE_ADDRESS 0x770000

// RusEFI Bench Test Packet IDs
#define RUSEFI_EVENT_COUNTERS (RUSEFI_BENCH_TEST_BASE_ADDRESS + 0)
#define RUSEFI_RAW_ANALOG_1 (RUSEFI_BENCH_TEST_BASE_ADDRESS + 1)
#define RUSEFI_HW_QC_IO_CONTROL (RUSEFI_BENCH_TEST_BASE_ADDRESS + 2)
#define RUSEFI_BOARD_STATUS (RUSEFI_BENCH_TEST_BASE_ADDRESS + 3)
#define RUSEFI_BUTTON_COUNTERS (RUSEFI_BENCH_TEST_BASE_ADDRESS + 4)
#define RUSEFI_IO_META_INFO (RUSEFI_BENCH_TEST_BASE_ADDRESS + 5)
#define RUSEFI_RAW_ANALOG_2 (RUSEFI_BENCH_TEST_BASE_ADDRESS + 6)
#define RUSEFI_PIN_STATE (RUSEFI_BENCH_TEST_BASE_ADDRESS + 7)
#define RUSEFI_AUX_DIGITAL_COUNTERS (RUSEFI_BENCH_TEST_BASE_ADDRESS + 8)
#define RUSEFI_MANUAL_PIN_TEST (RUSEFI_BENCH_TEST_BASE_ADDRESS + 9)
#define RUSEFI_ECU_CONFIG_BROADCAST (RUSEFI_BENCH_TEST_BASE_ADDRESS + 0x0D)
#define RUSEFI_GPS_INPUT (RUSEFI_BENCH_TEST_BASE_ADDRESS + 0x15)  // 0x770015 - GPS input to ECU

// GPS Configuration (GT-U7)
#define GPS_ENABLE_BY_DEFAULT true
#define GPS_SEND_INTERVAL_MS 100  // Send GPS data every 100ms
#define GPS_SERIAL_BAUD 9600     // GT-U7 default baudrate
#define GPS_RX_PIN 25            // GPIO25 for GPS RX
#define GPS_TX_PIN 26            // GPIO26 for GPS TX  
#define GPS_UPDATE_INTERVAL_MS 1000  // Parse GPS every 1 second
#define GPS_TIMEOUT_MS 15000     // GPS data timeout (15 seconds)

// RPM Configuration
#define DEFAULT_MAX_RPM 9000

// Touch screen pins (dedicated SPI bus)
#define XPT2046_CS 33
#define XPT2046_IRQ 27
#define XPT2046_MOSI 13  // DIN
#define XPT2046_MISO 12  // DO  
#define XPT2046_CLK 14

// Touch calibration defaults
#define TOUCH_MIN_X 200
#define TOUCH_MAX_X 3700
#define TOUCH_MIN_Y 240
#define TOUCH_MAX_Y 3800

// Screen definitions
#define SCREEN_MAIN 0
#define SCREEN_CONFIG 1
#define SCREEN_BENCH 2
#define TOTAL_SCREENS 3

// Swipe gesture constants
#define SWIPE_MIN_DISTANCE 25
#define SWIPE_MAX_TIME 1000

// Other constants
#define EEPROM_SIZE 1024  // Increased from 512 to 1024 for display config

// Simulator configuration
#define ENABLE_SIMULATOR 1  // Set to 0 to disable simulator completely

// AC Controller configuration (ESP-NOW)
#define AC_CONTROLLER_ENABLE_BY_DEFAULT true
#define AC_DATA_TIMEOUT_MS 120000  // 2 minutes timeout for AC data
#define AC_MODE_COOL 0
#define AC_MODE_HEAT 1
#define AC_MODE_AUTO 2
#define AC_MODE_FAN 3
#define AC_MODE_DRY 4

// Debug configuration
#define ENABLE_DEBUG_MODE 1  // Set to 0 to disable debug mode completely

// Font definitions
#define AA_FONT_SMALL Roboto16
#define AA_FONT_LARGE RobotoBold32

// Splash screen options
#define SPLASH_MAZDUINO 0
#define SPLASH_MERCY 1
#define SPLASH_HEDON 2
#define SPLASH_BIIES 3
#define SPLASH_ZYCAS 4
#define SPLASH_SPINE 5
#define SPLASH_JW 6
#define SPLASH_BMW 7

// Build-time splash screen selection - change this to set default
#ifndef BUILD_DEFAULT_SPLASH
#define BUILD_DEFAULT_SPLASH SPLASH_BMW  // Default BMW splash screen
#endif

#define DEFAULT_SPLASH_SCREEN BUILD_DEFAULT_SPLASH

// Speeduino data mode options
#define SPEEDUINO_MODE_A 0  // Simple data set (75 bytes) - for Arduino Mega clones with limited capability
#define SPEEDUINO_MODE_N 1  // Enhanced data set (119 bytes) - full data for powerful controllers
#define DEFAULT_SPEEDUINO_MODE SPEEDUINO_MODE_N

// EEPROM Memory Allocation Map
// ============================
// Address 0:      System initialization flag
// Address 1:      Communication mode (COMM_CAN=0, COMM_SERIAL=1)
// Address 2:      CAN Protocol (HALTECH=0, RUSEFI=1)
// Address 10:     Splash screen selection (0-5)
// Address 11-12:  Display brightness + initialization flag (0xAA)
// Address 14-15:  Speeduino data mode + initialization flag (0xBB)
// Address 16-17:  AC Controller enable + initialization flag (0xCC)
// Address 18-31:  Reserved for future features
// Address 32-99:  Reserved for expansion
// Address 100-299: Display configuration (up to 200 bytes)
// Address 300-399: Touch calibration data
// Address 400-499: WiFi configuration (SSID/Password)
// Address 500-511: WiFi status flags

#define EEPROM_COMM_MODE_ADDR           1
#define EEPROM_CAN_PROTOCOL_ADDR        2
#define EEPROM_SPLASH_SCREEN_ADDR       10
#define EEPROM_BRIGHTNESS_ADDR          11
#define EEPROM_BRIGHTNESS_FLAG_ADDR     12
#define EEPROM_SPEEDUINO_MODE_ADDR      14  
#define EEPROM_SPEEDUINO_FLAG_ADDR      15
#define EEPROM_AC_CONTROLLER_ADDR       16
#define EEPROM_AC_CONTROLLER_FLAG_ADDR  17
#define EEPROM_RESERVED_START_ADDR      18
#define EEPROM_DISPLAY_CONFIG_ADDR      100
#define EEPROM_TOUCH_CALIB_ADDR         300
#define EEPROM_WIFI_CONFIG_ADDR         400
#define EEPROM_WIFI_STATUS_ADDR         500
#define EEPROM_PANEL_CONFIG_START_ADDR  32  // Legacy - kept for compatibility

// EEPROM initialization flags
#define EEPROM_BRIGHTNESS_FLAG          0xAA
#define EEPROM_SPEEDUINO_FLAG           0xBB
#define EEPROM_AC_CONTROLLER_FLAG       0xCC

#endif // CONFIG_H
