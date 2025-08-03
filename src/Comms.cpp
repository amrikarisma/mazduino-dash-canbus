#include "Arduino.h"
#include "Comms.h"

void requestData(uint16_t timeout)
{
  Serial1.setTimeout(timeout);

  while (Serial1.available()) Serial1.read(); // Flush buffer

  Serial1.write('n');

  uint32_t start = millis();
  uint32_t end = start;
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
        Serial.printf("[SERIAL] Data received: %d bytes\n", dataLen);
        lastDebugPrint = millis();
      }
    } else {
      Serial.println("[SERIAL] Data overflow: Invalid data length");
      Serial.println(dataLen);
    }
  } else {
    // Debug: Print timeout or no data
    static uint32_t lastTimeoutPrint = 0;
    if (millis() - lastTimeoutPrint > 10000) { // Print every 10 seconds
      Serial.printf("[SERIAL] No data received - timeout: %dms, available: %d\n", 
                    end - start, Serial1.available());
      lastTimeoutPrint = millis();
    }
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