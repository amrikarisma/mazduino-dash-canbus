#ifndef SERIAL_HANDLER_H
#define SERIAL_HANDLER_H

#include "Arduino.h"

// Function declarations
void setupSerial();
void handleSerialCommunication();
void serialTask(void *pvParameters);

#endif // SERIAL_HANDLER_H
