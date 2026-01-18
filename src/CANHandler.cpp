#include "CANHandler.h"
#include "DisplayConfig.h"
#include "Config.h"
#include "DataTypes.h"
#include "GlobalVariables.h"
#include <esp32_can.h>
#include "Arduino.h"

void setupCAN() {
  CAN0.setCANPins(GPIO_NUM_17, GPIO_NUM_16); // RX, TX
  CAN0.begin(getCanSpeed());                // Use configurable CAN speed
  
  // Setup CAN filters based on selected protocol
  if (canProtocol == CAN_PROTOCOL_HALTECH) {
    // Haltech CAN IDs
    CAN0.watchFor(0x360);                      // RPM, MAP, TPS
    CAN0.watchFor(0x361);                      // Fuel Pressure, Oil Pressure, Wastegate Pressure
    CAN0.watchFor(0x362);                      // Ignition Angle (Leading)
    CAN0.watchFor(0x368);                      // AFR 01
    CAN0.watchFor(0x369);                      // Trigger System Error Count
    CAN0.watchFor(0x370);                      // VSS
    CAN0.watchFor(0x372);                      // Voltage
    CAN0.watchFor(0x3E0);                      // CLT, IAT
    CAN0.watchFor(0x3E4);                      // Indicator
    Serial.printf("CAN mode aktif (Haltech). Speed: %u bps\n", getCanSpeed());
  } else if (canProtocol == CAN_PROTOCOL_RUSEFI) {
    // RusEFI CAN IDs - Standard Format
    CAN0.watchFor(RUSEFI_ID_0x200);            // Warning Counter, Status bits
    CAN0.watchFor(RUSEFI_ID_0x201);            // RPM, Ignition Timing, Duties, VSS, Flex
    CAN0.watchFor(RUSEFI_ID_0x202);            // PPS, TPS1, TPS2, Wastegate
    CAN0.watchFor(RUSEFI_ID_0x203);            // MAP, Temps, Fuel Level
    CAN0.watchFor(RUSEFI_ID_0x204);            // Oil Press/Temp, Fuel Temp, Battery
    CAN0.watchFor(RUSEFI_ID_0x207);            // Lambda 1&2, Fuel Pressures
    
    // RusEFI Extended IDs - Bench Test and Status
    CAN0.watchForRange(RUSEFI_BENCH_TEST_BASE_ADDRESS, RUSEFI_BENCH_TEST_BASE_ADDRESS + 0x20);
    CAN0.watchFor(RUSEFI_GDI4_BASE_ADDRESS);
    
    Serial.printf("CAN mode aktif (RusEFI). Speed: %u bps\n", getCanSpeed());
    Serial.println("[RusEFI] Listening for Standard (0x200-0x207) and Extended IDs (0x770000+, 0xBB20)");
  }

  isCANMode = true;  // Set communication mode indicator
}

void canTask(void *pvParameters) {
  while (1) {
    handleCANCommunication();
    vTaskDelay(1);
  }
}

void handleCANCommunication() {
  static uint32_t lastRefresh = millis();
  uint32_t elapsed = millis() - lastRefresh;
  refreshRate = (elapsed > 0) ? (1000 / elapsed) : 0;
  lastRefresh = millis();
  unsigned long currentTime = millis();
  
  isCANMode = true;  // We're in CAN mode when this function is called
  
  if (CAN0.available()) {
    CAN_FRAME can_message;
    if (CAN0.read(can_message)) {
      // Debug: Show any CAN message received
      static uint32_t lastCANDebug = 0;
      if (millis() - lastCANDebug > 3000) { // Debug every 3 seconds
        Serial.printf("[CAN] Message received - ID: 0x%08X, Protocol: %s, Extended: %s\n", 
                      can_message.id, 
                      (canProtocol == CAN_PROTOCOL_RUSEFI) ? "RusEFI" : "Haltech",
                      can_message.extended ? "Yes" : "No");
        lastCANDebug = millis();
      }
      
      // Route to appropriate protocol handler
      if (canProtocol == CAN_PROTOCOL_HALTECH) {
        handleHaltechCAN(can_message);
      } else if (canProtocol == CAN_PROTOCOL_RUSEFI) {
        handleRusEFICAN(can_message);
      }
    } else {
      Serial.println("Error reading CAN message.");
    }
  }
}

void handleHaltechCAN(CAN_FRAME &can_message) {
  // Process Haltech data based on ID
  switch (can_message.id) {
        case 0x360: {
          rpm = (can_message.data.byte[0] << 8) | can_message.data.byte[1];
          uint16_t map = (can_message.data.byte[2] << 8) | can_message.data.byte[3];
          uint16_t tps_raw = (can_message.data.byte[4] << 8) | can_message.data.byte[5];
          mapData = map / 10.0;
          tps = tps_raw / 10.0;
          break;
        }
        case 0x361: {
          uint16_t fuel_pressure = (can_message.data.byte[0] << 8) | can_message.data.byte[1];
          fp = fuel_pressure / 10 - 101.3;
          uint16_t oil_pressure = (can_message.data.byte[2] << 8) | can_message.data.byte[3];
          // op = oil_pressure / 10 - 101.3;
          // uint16_t wastegate_pressure = (can_message.data.byte[6] << 8) | can_message.data.byte[7];
          // wp = wastegate_pressure / 10 - 101.3;
          break;
        }
        case 0x368: {
          uint16_t afr_raw = (can_message.data.byte[0] << 8) | can_message.data.byte[1];
          float lambda = afr_raw / 1000.0;
          afrConv = lambda * 14.7;
          break;
        }
        case 0x369: {
          uint16_t trigger_raw = (can_message.data.byte[0] << 8) | can_message.data.byte[1];
          triggerError = trigger_raw;
          break;
        }
        case 0x370: {
          // Only update VSS from CAN if GPS is not providing valid speed data
          if (!gpsEnabled || !gpsDataValid) {
            uint16_t vss_raw = (can_message.data.byte[0] << 8) | can_message.data.byte[1];
            vss = vss_raw / 10.0;
          }
          break;
        }
        case 0x372: {
          uint16_t voltage = (can_message.data.byte[0] << 8) | can_message.data.byte[1];
          bat = voltage / 10.0;
          break;
        }
        case 0x3E0: {
          uint16_t clt_raw = (can_message.data.byte[0] << 8) | can_message.data.byte[1];
          uint16_t iat_raw = (can_message.data.byte[2] << 8) | can_message.data.byte[3];
          float clt_k = clt_raw / 10.0;
          float iat_k = iat_raw / 10.0;
          clt = clt_k - 273.15;
          iat = iat_k - 273.15;
          break;
        }
        case 0x3E4: {
          dfco = (can_message.data.byte[1] << 8) | can_message.data.byte[4];
          launch = (can_message.data.byte[2] << 8) | can_message.data.byte[6];
          airCon = (can_message.data.byte[3] << 8) | can_message.data.byte[4];
          fan = (can_message.data.byte[3] << 8) | can_message.data.byte[0];
          rev = (can_message.data.byte[2] << 8) | can_message.data.byte[5];
          break;
        }
        case 0x362: {
          uint16_t adv_raw = (can_message.data.byte[4] << 8) | can_message.data.byte[5];
          adv = adv_raw / 10.0;
          break;
        }
        default:
          break;
      }
}

void handleRusEFICAN(CAN_FRAME &can_message) {
  // Process RusEFI data based on ID
  switch (can_message.id) {
    case RUSEFI_ID_0x200: {
      // WarningCounter : 0|16@1+ (1,0) [0|0] "" Vector__XXX
      // LastError : 16|16@1+ (1,0) [0|0] "" Vector__XXX
      // RevLimAct : 32|1@1+ (1,0) [0|0] "" Vector__XXX
      // MainRelayAct : 33|1@1+ (1,0) [0|0] "" Vector__XXX
      // FuelPumpAct : 34|1@1+ (1,0) [0|0] "" Vector__XXX
      // CELAct : 35|1@1+ (1,0) [0|0] "" Vector__XXX
      // EGOHeatAct : 36|1@1+ (1,0) [0|0] "" Vector__XXX
      // LambdaProtectAct : 37|1@1+ (1,0) [0|0] "" Vector__XXX
      // CurrentGear : 40|8@1+ (1,0) [0|0] "" Vector__XXX
      // DistanceTraveled : 48|16@1+ (0.1,0) [0|6553.5] "km" Vector__XXX
      // Fan : 38|1@1+ (1,0) [0|0] "" Vector__XXX
      // Fan2 : 39|1@1+ (1,0) [0|0] "" Vector__XXX
      warningCounter = (can_message.data.byte[1] << 8) | can_message.data.byte[0];
      lastError = (can_message.data.byte[3] << 8) | can_message.data.byte[2];
      revLimAct = (can_message.data.byte[4] & 0x01) != 0;
      mainRelayAct = (can_message.data.byte[4] & 0x02) != 0;
      fuelPumpAct = (can_message.data.byte[4] & 0x04) != 0;
      celAct = (can_message.data.byte[4] & 0x08) != 0;
      egoHeatAct = (can_message.data.byte[4] & 0x10) != 0;
      lambdaProtectAct = (can_message.data.byte[4] & 0x20) != 0;
      fan = (can_message.data.byte[4] & 0x40) != 0;
      fan2 = (can_message.data.byte[4] & 0x80) != 0;
      currentGear = can_message.data.byte[5]; // Current detected gear from RusEFI
      distanceTraveled = (can_message.data.byte[7] << 8) | can_message.data.byte[6];
      
      // Map RusEFI indicators to existing variables for display compatibility
      rev = revLimAct; // Map rev limiter to existing rev variable for display
      break;
    }
    case RUSEFI_ID_0x201: {
      // RPM : 0|16@1+ (1,0) [0|0] "RPM" Vector__XXX
      // IgnitionTiming : 16|16@1- (0.02,0) [0|0] "deg" Vector__XXX
      // InjDuty : 32|8@1+ (0.5,0) [0|100] "%" Vector__XXX
      // IgnDuty : 40|8@1+ (0.5,0) [0|100] "%" Vector__XXX
      // VehicleSpeed : 48|8@1+ (1,0) [0|255] "kph" Vector__XXX
      // FlexPct : 56|8@1+ (1,0) [0|100] "%" Vector__XXX
      rpm = (can_message.data.byte[1] << 8) | can_message.data.byte[0];
      int16_t ign_raw = (can_message.data.byte[3] << 8) | can_message.data.byte[2];
      ignitionTiming = ign_raw * 0.02;
      adv = ignitionTiming; // Map to existing advance variable
      injDuty = can_message.data.byte[4] * 0.5;
      ignDuty = can_message.data.byte[5] * 0.5;
      
      // Only update VSS from RusEFI if GPS is not providing valid speed data
      if (!gpsEnabled || !gpsDataValid) {
        vss = can_message.data.byte[6];
      }
      
      flexPct = can_message.data.byte[7];
      
      // Debug output
      static uint32_t lastDebug = 0;
      if (millis() - lastDebug > 1000) { // Debug every 1 second
        Serial.printf("[RusEFI] RPM: %d, TPS: %.1f, MAP: %.1f, Adv: %.1f\n", 
                      rpm, tps, mapData, adv);
        lastDebug = millis();
      }
      break;
    }
    case RUSEFI_ID_0x202: {
      // PPS : 0|16@1- (0.01,0) [0|100] "%" Vector__XXX
      // TPS1 : 16|16@1- (0.01,0) [0|100] "%" Vector__XXX
      // TPS2 : 32|16@1- (0.01,0) [0|100] "%" Vector__XXX
      // Wastegate : 48|16@1- (0.01,0) [0|100] "%" Vector__XXX
      int16_t pps_raw = (can_message.data.byte[1] << 8) | can_message.data.byte[0];
      int16_t tps1_raw = (can_message.data.byte[3] << 8) | can_message.data.byte[2];
      int16_t tps2_raw = (can_message.data.byte[5] << 8) | can_message.data.byte[4];
      int16_t wg_raw = (can_message.data.byte[7] << 8) | can_message.data.byte[6];
      pps = pps_raw * 0.01;
      tps1 = tps1_raw * 0.01;
      tps2 = tps2_raw * 0.01;
      tps = tps1; // Map to existing TPS variable
      wastegate = wg_raw * 0.01;
      break;
    }
    case RUSEFI_ID_0x203: {
      // MAP : 0|16@1+ (0.03333333,0) [0|0] "kPa" Vector__XXX
      // CoolantTemp : 16|8@1+ (1,-40) [-40|200] "deg C" Vector__XXX
      // IntakeTemp : 24|8@1+ (1,-40) [-40|200] "deg C" Vector__XXX
      // AUX1Temp : 32|8@1+ (1,-40) [-40|200] "deg C" Vector__XXX
      // AUX2Temp : 40|8@1+ (1,-40) [-40|200] "deg C" Vector__XXX
      // MCUTemp : 48|8@1+ (1,-40) [-40|100] "deg C" Vector__XXX
      // FuelLevel : 56|8@1+ (0.5,0) [0|0] "%" Vector__XXX
      uint16_t map_raw = (can_message.data.byte[1] << 8) | can_message.data.byte[0];
      mapData = map_raw * 0.03333333;
      clt = can_message.data.byte[2] - 40;
      iat = can_message.data.byte[3] - 40;
      aux1Temp = can_message.data.byte[4] - 40;
      aux2Temp = can_message.data.byte[5] - 40;
      mcuTemp = can_message.data.byte[6] - 40;
      fuelLevel = can_message.data.byte[7] * 0.5;
      
      // Debug output for MAP and temps
      static uint32_t lastDebugMAP = 0;
      if (millis() - lastDebugMAP > 2000) { // Debug every 2 seconds
        Serial.printf("[RusEFI] MAP: %.1f kPa, CLT: %d°C, IAT: %d°C\n", 
                      mapData, clt, iat);
        lastDebugMAP = millis();
      }
      break;
    }
    case RUSEFI_ID_0x204: {
      // OilPress : 16|16@1+ (0.03333333,0) [0|0] "kPa" Vector__XXX
      // OilTemperature : 32|8@1+ (1,-40) [-40|215] "deg C" Vector__XXX
      // FuelTemperature : 40|8@1+ (1,-40) [-40|215] "deg C" Vector__XXX
      // BattVolt : 48|16@1+ (0.001,0) [0|25] "mV" Vector__XXX
      uint16_t oil_press_raw = (can_message.data.byte[3] << 8) | can_message.data.byte[2];
      oilPressure = oil_press_raw * 0.03333333;
      fp = oilPressure; // Map to existing fuel pressure (since it's used for display)
      oilTemp = can_message.data.byte[4] - 40;
      fuelTemp = can_message.data.byte[5] - 40;
      uint16_t batt_raw = (can_message.data.byte[7] << 8) | can_message.data.byte[6];
      bat = batt_raw * 0.001;
      break;
    }
    case RUSEFI_ID_0x207: {
      // Lam1 : 0|16@1+ (0.0001,0) [0|2] "lambda" Vector__XXX
      // Lam2 : 16|16@1+ (0.0001,0) [0|2] "lambda" Vector__XXX
      // FpLow : 32|16@1+ (0.03333333,0) [0|0] "kPa" Vector__XXX
      // FpHigh : 48|16@1+ (0.1,0) [0|0] "bar" Vector__XXX
      uint16_t lam1_raw = (can_message.data.byte[1] << 8) | can_message.data.byte[0];
      uint16_t lam2_raw = (can_message.data.byte[3] << 8) | can_message.data.byte[2];
      uint16_t fp_low_raw = (can_message.data.byte[5] << 8) | can_message.data.byte[4];
      uint16_t fp_high_raw = (can_message.data.byte[7] << 8) | can_message.data.byte[6];
      lam1 = lam1_raw * 0.0001;
      lam2 = lam2_raw * 0.0001;
      afrConv = lam1 * 14.7; // Convert lambda to AFR for display
      fpLow = fp_low_raw * 0.03333333;
      fpHigh = fp_high_raw * 0.1;
      break;
    }
    default:
      // Handle Extended CAN IDs for RusEFI bench test and status
      if (can_message.id >= RUSEFI_BENCH_TEST_BASE_ADDRESS && 
          can_message.id <= RUSEFI_BENCH_TEST_BASE_ADDRESS + 0x20) {
        handleRusEFIExtendedCAN(can_message);
      } else if (can_message.id == RUSEFI_GDI4_BASE_ADDRESS) {
        handleRusEFIGDI4CAN(can_message);
      }
      break;
  }
}

// Handler for RusEFI Extended CAN messages (Bench Test format)
void handleRusEFIExtendedCAN(CAN_FRAME &can_message) {
  uint32_t packet_id = can_message.id;
  
  switch (packet_id) {
    case RUSEFI_ECU_CONFIG_BROADCAST: {
      // ECU configuration broadcast - may contain engine data
      static uint32_t lastExtDebug = 0;
      if (millis() - lastExtDebug > 5000) {
        Serial.printf("[RusEFI-Ext] ECU Config Broadcast received\n");
        lastExtDebug = millis();
      }
      break;
    }
    case RUSEFI_BOARD_STATUS: {
      // Board status information
      static uint32_t lastBoardDebug = 0;
      if (millis() - lastBoardDebug > 5000) {
        Serial.printf("[RusEFI-Ext] Board Status received\n");
        lastBoardDebug = millis();
      }
      break;
    }
    default: {
      // Generic extended ID handler
      static uint32_t lastGenericDebug = 0;
      if (millis() - lastGenericDebug > 10000) {
        Serial.printf("[RusEFI-Ext] Extended ID: 0x%08X received\n", packet_id);
        lastGenericDebug = millis();
      }
      break;
    }
  }
}

// Handler for RusEFI GDI4 CAN messages
void handleRusEFIGDI4CAN(CAN_FRAME &can_message) {
  static uint32_t lastGDI4Debug = 0;
  if (millis() - lastGDI4Debug > 5000) {
    Serial.printf("[RusEFI-GDI4] GDI4 message received\n");
    lastGDI4Debug = millis();
  }
}

// Send GPS data to RusEFI ECU via CAN (CAN ID 0x770015)
void sendGPSData() {
  // Only send if GPS is enabled, data is valid, and using RusEFI protocol
  if (!gpsEnabled || !gpsDataValid || canProtocol != CAN_PROTOCOL_RUSEFI || !isCANMode) {
    return;
  }
  
  static uint32_t lastGPSSend = 0;
  uint32_t currentTime = millis();
  
  // Send GPS data at configured interval
  if (currentTime - lastGPSSend < GPS_SEND_INTERVAL_MS) {
    return;
  }
  
  // Prepare GPS data frame (8 bytes)
  CAN_FRAME gpsFrame;
  gpsFrame.id = RUSEFI_GPS_INPUT;  // 0x770015
  gpsFrame.extended = true;        // Extended CAN ID 
  gpsFrame.length = 8;
  
  // GPS Data Format (based on common GPS CAN implementations):
  // Bytes 0-1: Speed (km/h * 100) - 16-bit
  // Bytes 2-3: Heading (degrees * 100) - 16-bit  
  // Bytes 4: Number of satellites
  // Bytes 5-7: Reserved/Status
  
  uint16_t speedScaled = (uint16_t)(gpsSpeed * 100);
  uint16_t headingScaled = (uint16_t)(gpsHeading * 100);
  
  gpsFrame.data.byte[0] = (speedScaled >> 8) & 0xFF;    // Speed high byte
  gpsFrame.data.byte[1] = speedScaled & 0xFF;           // Speed low byte
  gpsFrame.data.byte[2] = (headingScaled >> 8) & 0xFF;  // Heading high byte
  gpsFrame.data.byte[3] = headingScaled & 0xFF;         // Heading low byte
  gpsFrame.data.byte[4] = gpsNumSats;                   // Satellite count
  gpsFrame.data.byte[5] = gpsDataValid ? 0x01 : 0x00;   // GPS validity flag
  gpsFrame.data.byte[6] = 0x00;                         // Reserved
  gpsFrame.data.byte[7] = 0x00;                         // Reserved
  
  bool result = CAN0.sendFrame(gpsFrame);
  
  if (result) {
    lastGPSSend = currentTime;
#if ENABLE_DEBUG_MODE
    if (debugMode) {
      Serial.printf("[GPS] Data sent - Speed: %.1f km/h, Heading: %.1f°, Sats: %d\n", 
                    gpsSpeed, gpsHeading, gpsNumSats);
    }
#endif
  } else {
#if ENABLE_DEBUG_MODE
    if (debugMode) {
      Serial.println("[GPS] Failed to send GPS data");
    }
#endif
  }
}
