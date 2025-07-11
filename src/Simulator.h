#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <stdint.h>

// Simulator configuration
#define SIMULATOR_MODE_OFF 0
#define SIMULATOR_MODE_RPM_SWEEP 1
#define SIMULATOR_MODE_ENGINE_IDLE 2
#define SIMULATOR_MODE_DRIVING 3
#define SIMULATOR_MODE_REDLINE 4

// Function declarations
void initializeSimulator();
void updateSimulatorData();
void setSimulatorMode(uint8_t mode);
uint8_t getSimulatorMode();

#endif // SIMULATOR_H
