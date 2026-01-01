#include "Arduino.h"
#include "Comms.h"
#include "Config.h"
#include "DataTypes.h"
#include <EEPROM.h>

void requestData(uint16_t timeout)
{
  Serial1.setTimeout(timeout);

  while (Serial1.available()) Serial1.read(); // Flush buffer

  // Send command based on selected data mode
  char command = (speeduinoDataMode == SPEEDUINO_MODE_A) ? 'A' : 'n';
  Serial1.write(command);

  uint32_t start = millis();
  uint32_t end = start;
  
  if (speeduinoDataMode == SPEEDUINO_MODE_A) {
    // Mode A: Wait for 'A' confirmation + 75 bytes of data
    while (Serial1.available() < 76 && (end - start) < timeout)
    {
      end = millis();
    }
    
    if (end - start < timeout && Serial1.available() >= 76)
    {
      Serial1.read(); // 'A' confirmation
      Serial1.readBytes(buffer, 75); // Read 75 bytes of data
      
      // Debug: Print successful data read
      static uint32_t lastDebugPrint = 0;
      if (millis() - lastDebugPrint > 5000) { // Print every 5 seconds
        Serial.printf("[SERIAL] Mode A - Data received: 75 bytes\n");
        lastDebugPrint = millis();
      }
    }
  } else {
    // Mode N: Wait for 'n' + 0x32 + length byte + data
    while (Serial1.available() < 3 && (end - start) < timeout)
    {
      end = millis();
    }
    
    if (end - start < timeout && Serial1.available() >= 3)
    {
      Serial1.read(); // 'n'
      Serial1.read(); // 0x32
      uint8_t dataLen = Serial1.read();
      if (dataLen <= DATA_LEN) {
        Serial1.readBytes(buffer, dataLen);
        // Debug: Print successful data read
        static uint32_t lastDebugPrint = 0;
        if (millis() - lastDebugPrint > 5000) { // Print every 5 seconds
          Serial.printf("[SERIAL] Mode N - Data received: %d bytes\n", dataLen);
          lastDebugPrint = millis();
        }
      } else {
        Serial.println("[SERIAL] Data overflow: Invalid data length");
        Serial.println(dataLen);
      }
    }
  }
  
  if (end - start >= timeout) {
    // Debug: Print timeout or no data
    static uint32_t lastTimeoutPrint = 0;
    if (millis() - lastTimeoutPrint > 10000) { // Print every 10 seconds
      Serial.printf("[SERIAL] No data received - timeout: %dms, available: %d, mode: %s\n", 
                    end - start, Serial1.available(), 
                    (speeduinoDataMode == SPEEDUINO_MODE_A) ? "A" : "N");
      lastTimeoutPrint = millis();
    }
  }
}

void loadSpeeduinoDataModeFromEEPROM() {
  uint8_t savedMode = EEPROM.read(EEPROM_SPEEDUINO_MODE_ADDR);
  uint8_t initFlag = EEPROM.read(EEPROM_SPEEDUINO_FLAG_ADDR);
  
  Serial.printf("[SPEEDUINO] Loading from EEPROM - savedMode: %d, initFlag: 0x%02X\n", savedMode, initFlag);
  
  // Check if EEPROM has been initialized
  if (initFlag != EEPROM_SPEEDUINO_FLAG) {
    // EEPROM not initialized, use default mode
    speeduinoDataMode = DEFAULT_SPEEDUINO_MODE;
    saveSpeeduinoDataModeToEEPROM();
    Serial.printf("[SPEEDUINO] Using default data mode: %s (EEPROM uninitialized)\n", 
                  (speeduinoDataMode == SPEEDUINO_MODE_A) ? "A (Simple)" : "N (Enhanced)");
  } else {
    // Use saved mode if valid
    if (savedMode == SPEEDUINO_MODE_A || savedMode == SPEEDUINO_MODE_N) {
      speeduinoDataMode = savedMode;
      Serial.printf("[SPEEDUINO] Data mode loaded from EEPROM: %s\n", 
                    (speeduinoDataMode == SPEEDUINO_MODE_A) ? "A (Simple)" : "N (Enhanced)");
    } else {
      // Invalid mode, use default
      speeduinoDataMode = DEFAULT_SPEEDUINO_MODE;
      saveSpeeduinoDataModeToEEPROM();
      Serial.printf("[SPEEDUINO] Invalid mode in EEPROM (%d), using default: %s\n", 
                    savedMode, (speeduinoDataMode == SPEEDUINO_MODE_A) ? "A (Simple)" : "N (Enhanced)");
    }
  }
}

void saveSpeeduinoDataModeToEEPROM() {
  Serial.printf("[SPEEDUINO] Saving mode %d to EEPROM address %d\n", speeduinoDataMode, EEPROM_SPEEDUINO_MODE_ADDR);
  EEPROM.write(EEPROM_SPEEDUINO_MODE_ADDR, speeduinoDataMode);
  EEPROM.write(EEPROM_SPEEDUINO_FLAG_ADDR, EEPROM_SPEEDUINO_FLAG); // Initialization flag
  bool success = EEPROM.commit();
  Serial.printf("[SPEEDUINO] Data mode saved to EEPROM: %s (commit: %s)\n", 
                (speeduinoDataMode == SPEEDUINO_MODE_A) ? "A (Simple)" : "N (Enhanced)",
                success ? "OK" : "FAILED");
}

void setSpeeduinoDataMode(uint8_t mode) {
  if (mode == SPEEDUINO_MODE_A || mode == SPEEDUINO_MODE_N) {
    speeduinoDataMode = mode;
    saveSpeeduinoDataModeToEEPROM();
    Serial.printf("[SPEEDUINO] Data mode changed to: %s\n", 
                  (speeduinoDataMode == SPEEDUINO_MODE_A) ? "A (Simple)" : "N (Enhanced)");
  }
}

bool getBit(uint16_t address, uint8_t bit) {
  if (address < DATA_LEN) {
    return bitRead(buffer[address], bit);
  }
  return false;
}
uint8_t getByte(uint16_t address) {
  if (address < DATA_LEN) {
    return buffer[address];
  }
  return 0;
}

uint16_t getWord(uint16_t address) {
  if (address < DATA_LEN - 1) {
    return makeWord(buffer[address + 1], buffer[address]);
  }
  return 0;
}