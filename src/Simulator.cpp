#include "Simulator.h"
#include "DataTypes.h"
#include "Config.h"
#include "Arduino.h"

static uint8_t simulatorMode = SIMULATOR_MODE_OFF;
static uint32_t lastSimUpdate = 0;
static uint32_t simulatorStep = 0;
static bool rpmIncreasing = true;

void initializeSimulator() {
  simulatorMode = SIMULATOR_MODE_OFF;
  lastSimUpdate = millis();
  simulatorStep = 0;
  rpmIncreasing = true;
  
  Serial.println("=== MAZDUINO SIMULATOR INITIALIZED ===");
  Serial.println("Simulator starts in OFF mode by default");
  Serial.println("Available modes:");
  Serial.println("0 = OFF (use real data)");
  Serial.println("1 = RPM SWEEP (0-6000 RPM)");
  Serial.println("2 = ENGINE IDLE (800-900 RPM)");
  Serial.println("3 = DRIVING (1500-4000 RPM)");
  Serial.println("4 = REDLINE (5500-6000 RPM)");
  Serial.println("Send '1', '2', '3', '4' via Serial to change mode");
  Serial.println("Send '0' to turn off simulator");
  Serial.println("Send 'h' for help");
  Serial.println("======================================");
  
  // Keep simulator OFF by default - user must manually enable
  Serial.println("[SIM] Simulator OFF - Using real data from CAN/Serial");
}

void setSimulatorMode(uint8_t mode) {
  simulatorMode = mode;
  simulatorStep = 0;
  rpmIncreasing = true;
  
  // Reset RPM to appropriate starting value for each mode
  switch(mode) {
    case SIMULATOR_MODE_OFF:
      Serial.println("[SIM] Simulator OFF - Using real data");
      rpm = 0; // Reset to 0 when turning off
      break;
    case SIMULATOR_MODE_RPM_SWEEP:
      Serial.println("[SIM] RPM SWEEP Mode - 0 to 6000 RPM");
      rpm = 0; // Start from 0
      break;
    case SIMULATOR_MODE_ENGINE_IDLE:
      Serial.println("[SIM] ENGINE IDLE Mode - 800-900 RPM");
      rpm = 850; // Start at idle
      break;
    case SIMULATOR_MODE_DRIVING:
      Serial.println("[SIM] DRIVING Mode - 1500-4000 RPM");
      rpm = 2500; // Start at mid-range
      break;
    case SIMULATOR_MODE_REDLINE:
      Serial.println("[SIM] REDLINE Mode - 5500-6000 RPM");
      rpm = 5750; // Start at redline
      break;
    default:
      Serial.println("[SIM] Unknown mode, turning off");
      simulatorMode = SIMULATOR_MODE_OFF;
      rpm = 0;
      break;
  }
}

uint8_t getSimulatorMode() {
  return simulatorMode;
}

void updateSimulatorData() {
  if (simulatorMode == SIMULATOR_MODE_OFF) {
    return; // Don't override real data
  }
  
  uint32_t currentTime = millis();
  if (currentTime - lastSimUpdate < 100) { // Update every 100ms
    return;
  }
  lastSimUpdate = currentTime;
  
  // Generate realistic sensor data based on RPM
  switch(simulatorMode) {
    case SIMULATOR_MODE_RPM_SWEEP:
      // Sweep from 0 to 6000 RPM and back
      if (rpmIncreasing) {
        rpm = simulatorStep * 50; // Increase by 50 RPM each step
        if (rpm >= 6000) {
          rpmIncreasing = false;
          simulatorStep = 120; // 6000/50 = 120
        } else {
          simulatorStep++;
        }
      } else {
        rpm = simulatorStep * 50; // Decrease by 50 RPM each step
        if (rpm <= 0) {
          rpmIncreasing = true;
          simulatorStep = 0;
        } else {
          simulatorStep--;
        }
      }
      break;
      
    case SIMULATOR_MODE_ENGINE_IDLE:
      // Realistic idle with small variations
      rpm = 850 + (simulatorStep % 10) * 5; // 850-895 RPM
      simulatorStep++;
      break;
      
    case SIMULATOR_MODE_DRIVING:
      // Simulate driving RPM patterns
      rpm = 2500 + sin(simulatorStep * 0.1) * 1000; // 1500-3500 RPM sine wave
      simulatorStep++;
      break;
      
    case SIMULATOR_MODE_REDLINE:
      // High RPM simulation
      rpm = 5750 + (simulatorStep % 20) * 10; // 5750-5940 RPM
      simulatorStep++;
      break;
  }
  
  // Generate correlated sensor data based on RPM
  if (rpm == 0) {
    // Engine off
    mapData = 30;  // Atmospheric pressure
    tps = 0;       // Throttle closed
    adv = 0;       // No advance
    afrConv = 14.7; // Stoichiometric
    fp = 0;        // No fuel pressure
    triggerError = 0;
    vss = 0;       // Not moving
    
    // Temperature sensors (engine off values)
    clt = 25;      // Ambient temp
    iat = 25;      // Ambient temp
    bat = 12.8;    // Resting battery voltage
    
    // All indicators off
    syncStatus = false;
    fan = false;
    ase = false;
    wue = false;
    rev = false;
    launch = false;
    airCon = false;
    dfco = false;
  } else {
    // Engine running - generate realistic values
    mapData = map(rpm, 800, 6000, 35, 95);  // 35-95 kPa MAP
    tps = map(rpm, 800, 6000, 5, 85);       // 5-85% TPS
    adv = map(rpm, 800, 6000, 10, 35);      // 10-35° advance
    
    // AFR varies with load
    if (rpm < 1000) {
      afrConv = 14.7; // Idle stoich
    } else if (rpm > 4000) {
      afrConv = 12.5; // Rich at high RPM
    } else {
      afrConv = 14.2; // Slightly lean cruise
    }
    
    fp = map(rpm, 800, 6000, 250, 350);     // 250-350 kPa fuel pressure
    triggerError = 0;                        // No errors in sim
    vss = map(rpm, 800, 6000, 0, 120);     // 0-120 km/h speed
    
    // Temperature sensors (engine running)
    clt = map(rpm, 800, 6000, 85, 95);     // 85-95°C coolant
    iat = map(rpm, 800, 6000, 30, 45);     // 30-45°C intake air
    bat = 14.2;                            // Charging voltage
    
    // Engine indicators based on RPM
    syncStatus = true;                      // Engine synced
    fan = (clt > 90);                      // Fan on when hot
    ase = (clt < 60);                      // After start enrichment when cold
    wue = (clt < 80);                      // Warm up enrichment
    rev = (rpm > 5500);                    // Rev limiter warning
    launch = false;                        // Launch control off
    airCon = false;                        // AC off
    dfco = (rpm > 2000 && tps < 10);      // DFCO at high RPM, low throttle
  }
  
  // Add some realistic noise/variation
  if (rpm > 0) {
    rpm += random(-10, 10);
    mapData += random(-2, 2);
    afrConv += (random(-20, 20) / 100.0);   // ±0.2 AFR variation
    bat += (random(-5, 5) / 100.0);         // ±0.05V variation
    tps += random(-2, 2);                   // ±2% TPS variation
    adv += random(-1, 1);                   // ±1° advance variation
    
    // Ensure values stay within realistic bounds
    // Don't constrain RPM too tightly in sweep mode to allow proper cycling
    if (simulatorMode == SIMULATOR_MODE_RPM_SWEEP) {
      rpm = constrain(rpm, 0, 6500); // Allow little headroom for sweep
    } else {
      rpm = constrain(rpm, 0, 7000);
    }
    mapData = constrain(mapData, 20, 105);
    afrConv = constrain(afrConv, 10.0, 18.0);
    bat = constrain(bat, 11.0, 15.0);
    tps = constrain(tps, 0, 100);
    adv = constrain(adv, -5, 40);
  }
  
  // Print current values every 2 seconds
  static uint32_t lastPrint = 0;
  if (currentTime - lastPrint > 2000) {
    Serial.printf("[SIM] RPM:%d MAP:%d TPS:%d AFR:%.1f BAT:%.1f VSS:%d TriggerErr:%d\n", 
                  rpm, mapData, tps, afrConv, bat, vss, triggerError);
    
    // Extra debug for RPM sweep mode
    if (simulatorMode == SIMULATOR_MODE_RPM_SWEEP) {
      Serial.printf("[SIM] Sweep Debug - Step:%d Direction:%s Target RPM:%d\n", 
                    simulatorStep, rpmIncreasing ? "UP" : "DOWN", simulatorStep * 50);
    }
    
    lastPrint = currentTime;
  }
}
