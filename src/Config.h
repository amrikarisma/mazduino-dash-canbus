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
#define RXD 26  // Changed from 16 to avoid conflict with CAN TX
#define TXD 25  // Changed from 17 to avoid conflict with CAN RX

// Backlight control
#define BACKLIGHT_PIN 32
#define BACKLIGHT_CHANNEL 0
#define BACKLIGHT_FREQ 5000
#define BACKLIGHT_RESOLUTION 8
#define BACKLIGHT_BRIGHTNESS 100 // 0-255 (0 = off, 255 = max brightness)

// Communication modes
#define COMM_CAN 0
#define COMM_SERIAL 1

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
#define EEPROM_SIZE 512

// Simulator configuration
#define ENABLE_SIMULATOR 1  // Set to 0 to disable simulator completely

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
#define DEFAULT_SPLASH_SCREEN SPLASH_MAZDUINO

// Speeduino data mode options
#define SPEEDUINO_MODE_A 0  // Simple data set (75 bytes) - for Arduino Mega clones with limited capability
#define SPEEDUINO_MODE_N 1  // Enhanced data set (119 bytes) - full data for powerful controllers
#define DEFAULT_SPEEDUINO_MODE SPEEDUINO_MODE_N

// EEPROM Memory Allocation Map
// ============================
// Address 0:      System initialization flag
// Address 1:      Communication mode (COMM_CAN=0, COMM_SERIAL=1)
// Address 10:     Splash screen selection (0-5)
// Address 11-12:  Display brightness + initialization flag (0xAA)
// Address 14-15:  Speeduino data mode + initialization flag (0xBB)
// Address 16-31:  Reserved for future features
// Address 32+:    Display panel configurations

#define EEPROM_COMM_MODE_ADDR           1
#define EEPROM_SPLASH_SCREEN_ADDR       10
#define EEPROM_BRIGHTNESS_ADDR          11
#define EEPROM_BRIGHTNESS_FLAG_ADDR     12
#define EEPROM_SPEEDUINO_MODE_ADDR      14  
#define EEPROM_SPEEDUINO_FLAG_ADDR      15
#define EEPROM_RESERVED_START_ADDR      16
#define EEPROM_PANEL_CONFIG_START_ADDR  32

// EEPROM initialization flags
#define EEPROM_BRIGHTNESS_FLAG          0xAA
#define EEPROM_SPEEDUINO_FLAG           0xBB

#endif // CONFIG_H
