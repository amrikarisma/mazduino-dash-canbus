# EEPROM Memory Allocation Map

This document describes the EEPROM memory layout used by the Mazduino Dashboard system to prevent conflicts and make memory management easier.

## Memory Layout

| Address | Size | Purpose | Flag | Description |
|---------|------|---------|------|-------------|
| 0 | 1 byte | System | - | System initialization flag |
| 1 | 1 byte | Communication | - | Communication mode (0=CAN, 1=Serial) |
| 2 | 1 byte | Communication | - | CAN protocol (0=Haltech, 1=RusEFI) |
| 10 | 1 byte | Display | - | Splash screen selection |
| 11-12 | 2 bytes | Display | 0xAA | Brightness value + init flag |
| 14-15 | 2 bytes | Communication | 0xBB | Speeduino mode + init flag |
| 16-17 | 2 bytes | AC | 0xCC | AC controller enable + init flag |
| 18-19 | 2 bytes | AC | - | AC cutoff temperature value (temp × 10, uint16) |
| 20 | 1 byte | AC | 0xDD | AC cutoff initialization flag |
| 21-31 | 11 bytes | Reserved | - | Reserved for future features |
| 32-99 | 68 bytes | Reserved | - | Reserved for expansion |
| 100-299 | 200 bytes | Display | - | Display configuration |
| 300-399 | 100 bytes | Touch | - | Touch calibration |
| 400-499 | 100 bytes | WiFi | - | WiFi SSID/password |
| 500-511 | 12 bytes | WiFi | - | WiFi status flags |

## Constants in Config.h

```cpp
#define EEPROM_COMM_MODE_ADDR           1
#define EEPROM_CAN_PROTOCOL_ADDR        2
#define EEPROM_SPLASH_SCREEN_ADDR       10
#define EEPROM_BRIGHTNESS_ADDR          11
#define EEPROM_BRIGHTNESS_FLAG_ADDR     12
#define EEPROM_SPEEDUINO_MODE_ADDR      14
#define EEPROM_SPEEDUINO_FLAG_ADDR      15
#define EEPROM_AC_CONTROLLER_ADDR       16
#define EEPROM_AC_CONTROLLER_FLAG_ADDR  17
#define EEPROM_AC_CUTOFF_TEMP_ADDR      18
#define EEPROM_AC_CUTOFF_FLAG_ADDR      20
#define EEPROM_RESERVED_START_ADDR      21

#define EEPROM_BRIGHTNESS_FLAG          0xAA
#define EEPROM_SPEEDUINO_FLAG           0xBB
#define EEPROM_AC_CONTROLLER_FLAG       0xCC
#define EEPROM_AC_CUTOFF_FLAG           0xDD
```