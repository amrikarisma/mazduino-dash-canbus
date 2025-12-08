#ifndef CONFIG_H
#define CONFIG_H

// Firmware version
extern const char *version;

// WiFi Configuration
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
#define DEFAULT_MAX_RPM 8000

// Other constants
#define EEPROM_SIZE 512

// Simulator configuration
#define ENABLE_SIMULATOR 1  // Set to 0 to disable simulator completely

// Debug configuration
#define ENABLE_DEBUG_MODE 1  // Set to 0 to disable debug mode completely

// Font definitions
#define AA_FONT_SMALL NotoSansBold15
#define AA_FONT_LARGE NotoSansBold36

// Splash screen options
#define SPLASH_MERCY 0
#define SPLASH_MAZDUINO 1
#define DEFAULT_SPLASH_SCREEN SPLASH_MAZDUINO

#endif // CONFIG_H
