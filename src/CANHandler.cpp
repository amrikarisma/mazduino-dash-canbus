#include "CANHandler.h"
#include "DisplayConfig.h"
#include "Config.h"
#include "DataTypes.h"
#include <esp32_can.h>
#include "Arduino.h"

void setupCAN() {
  CAN0.setCANPins(GPIO_NUM_17, GPIO_NUM_16); // RX, TX
  CAN0.begin(getCanSpeed());                // Use configurable CAN speed
  CAN0.watchFor(0x360);                      // RPM, MAP, TPS
  CAN0.watchFor(0x361);                      // Fuel Pressure
  CAN0.watchFor(0x362);                      // Ignition Angle (Leading)
  CAN0.watchFor(0x368);                      // AFR 01
  CAN0.watchFor(0x369);                      // Trigger System Error Count
  CAN0.watchFor(0x370);                      // VSS
  CAN0.watchFor(0x372);                      // Voltage
  CAN0.watchFor(0x3E0);                      // CLT, IAT
  CAN0.watchFor(0x3E4);                      // Indicator

  isCANMode = true;  // Set communication mode indicator
  Serial.printf("CAN mode aktif. Speed: %u bps\n", getCanSpeed());
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
      // Process data based on ID
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
          uint16_t vss_raw = (can_message.data.byte[0] << 8) | can_message.data.byte[1];
          vss = vss_raw / 10.0;
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
    } else {
      Serial.println("Error reading CAN message.");
    }
  }

//   if (currentTime - lastPrintTime >= 1000) {
//     Serial.print("RPM: ");
//     Serial.print(rpm);
//     Serial.print(" MAP: ");
//     Serial.print(mapData);
//     Serial.print(" kPa TPS: ");
//     Serial.print(tps);
//     Serial.print(" % ADV:");
//     Serial.print(adv);
//     Serial.print(" ° Fuel Pressure: ");
//     Serial.print(fp);
//     Serial.print(" kPa AFR: ");
//     Serial.print(afrConv, 2);
//     Serial.print(" VSS: ");
//     Serial.print(vss);
//     Serial.print(" km/h Voltage: ");
//     Serial.print(bat, 2);
//     Serial.print(" V CLT: ");
//     Serial.print(clt);
//     Serial.print(" °C IAT: ");
//     Serial.print(iat);
//     Serial.print(" °C Trigger Error: ");
//     Serial.print(triggerError);
//     Serial.println();

//     lastPrintTime = currentTime;
//   }
}
