#ifndef CAN_HANDLER_H
#define CAN_HANDLER_H

#include <esp32_can.h>

// Function declarations
void setupCAN();
void handleCANCommunication();
void handleHaltechCAN(CAN_FRAME &can_message);
void handleRusEFICAN(CAN_FRAME &can_message);
void handleRusEFIExtendedCAN(CAN_FRAME &can_message);
void handleRusEFIGDI4CAN(CAN_FRAME &can_message);
void canTask(void *pvParameters);
void sendGPSData();  // Send GPS data to RusEFI ECU

#endif // CAN_HANDLER_H
