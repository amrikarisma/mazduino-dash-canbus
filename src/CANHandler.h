#ifndef CAN_HANDLER_H
#define CAN_HANDLER_H

// Function declarations
void setupCAN();
void handleCANCommunication();
void canTask(void *pvParameters);

#endif // CAN_HANDLER_H
