# EEPROM Memory Allocation Map

This document describes the EEPROM memory layout used by the Mazduino Dashboard system to prevent conflicts and make memory management easier.

## Memory Layout

| Address | Size | Purpose | Flag | Description |
|---------|------|---------|------|-------------|
| 0 | 1 byte | System | - | System initialization flag |
| 1 | 1 byte | Communication | - | Communication mode (0=CAN, 1=Serial) |
| 10 | 1 byte | Display | - | Splash screen selection (0-5) |
| 11-12 | 2 bytes | Display | 0xAA | Display brightness + initialization flag |
| 14-15 | 2 bytes | Communication | 0xBB | Speeduino data mode + initialization flag |
| 16-31 | 16 bytes | Reserved | - | Reserved for future features |
| 32+ | Variable | Display | - | Display panel configurations |

## Constants in Config.h

```cpp
#define EEPROM_COMM_MODE_ADDR           1
#define EEPROM_SPLASH_SCREEN_ADDR       10
#define EEPROM_BRIGHTNESS_ADDR          11
#define EEPROM_BRIGHTNESS_FLAG_ADDR     12
#define EEPROM_SPEEDUINO_MODE_ADDR      14  
#define EEPROM_SPEEDUINO_FLAG_ADDR      15
#define EEPROM_RESERVED_START_ADDR      16
#define EEPROM_PANEL_CONFIG_START_ADDR  32

#define EEPROM_BRIGHTNESS_FLAG          0xAA
#define EEPROM_SPEEDUINO_FLAG           0xBB
```

## Usage Guidelines

### Adding New EEPROM Data
1. Update this document first
2. Add constants to Config.h
3. Use initialization flags for validation
4. Test thoroughly to avoid conflicts

### Initialization Flags
- Always use unique flag values (0xAA, 0xBB, etc.)
- Check flag before reading data
- Write flag after writing data
- Commit EEPROM after all writes

### Best Practices
- Use Config.h constants instead of hardcoded addresses
- Document any new allocations here
- Leave gaps for future expansion
- Use meaningful constant names

## Feature Implementation

### Brightness Control
- **Address:** 11-12
- **Flag:** 0xAA
- **Default:** 150
- **Files:** BacklightControl.cpp

### Speeduino Data Mode
- **Address:** 14-15
- **Flag:** 0xBB
- **Default:** Mode N (Enhanced)
- **Files:** Comms.cpp, WebServerHandler.cpp

### Communication Mode
- **Address:** 1
- **Values:** 0=CAN, 1=Serial
- **Files:** main.cpp

### Splash Screen
- **Address:** 10
- **Values:** 0-5 (Different splash screens)
- **Files:** SplashScreen.cpp, WebServerHandler.cpp

## Reserved Space
Addresses 16-31 are reserved for future features. When implementing new features that need EEPROM storage, start from address 16 and update this document accordingly.