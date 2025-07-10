#ifndef CONFIG_H
#define CONFIG_H

// Firmware version
extern const char *version;

// WiFi Configuration
extern const char *ssid;
extern const char *password;

// Pin definitions
#define UART_BAUD 115200
#define RXD 16
#define TXD 17

// Backlight control
#define BACKLIGHT_PIN 32
#define BACKLIGHT_CHANNEL 0
#define BACKLIGHT_FREQ 5000
#define BACKLIGHT_RESOLUTION 8
#define BACKLIGHT_BRIGHTNESS 100 // 0-255 (0 = off, 255 = max brightness)

// Communication modes
#define COMM_CAN 0
#define COMM_SERIAL 1

// Other constants
#define EEPROM_SIZE 512

// Font definitions
#define AA_FONT_SMALL NotoSansBold15
#define AA_FONT_LARGE NotoSansBold36

#endif // CONFIG_H
