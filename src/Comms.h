#ifndef COMMS_H
#define COMMS_H

#include "Arduino.h"
#define DATA_LEN 300

static uint8_t buffer[DATA_LEN];
void requestData(uint16_t timeout = 20);

bool getBit(uint16_t address, uint8_t bit);
uint8_t getByte(uint16_t address);
uint16_t getWord(uint16_t address);

// Speeduino data mode functions
void loadSpeeduinoDataModeFromEEPROM();
void saveSpeeduinoDataModeToEEPROM();
void setSpeeduinoDataMode(uint8_t mode);

#endif //COMMS_H