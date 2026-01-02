#ifndef CAN_HANDLER_H
#define CAN_HANDLER_H

#include <esp32_can.h>

// Function declarations
void setupCAN();
void handleCANCommunication();
void handleHaltechCAN(CAN_FRAME &can_message);
void handleRusEFICAN(CAN_FRAME &can_message);
void canTask(void *pvParameters);

#endif // CAN_HANDLER_H
