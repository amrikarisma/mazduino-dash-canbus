# RUSEFI CAN Protocol Support

## Overview
Dashboard sekarang mendukung dua protokol CAN:
1. **Haltech** - protokol original
2. **RusEFI** - protokol baru yang ditambahkan

## Perubahan yang Dilakukan

### 1. Penambahan Variabel Baru (DataTypes.h & GlobalVariables.cpp)
Variabel tambahan untuk data RusEFI:
```cpp
// Oil & Fuel
float oilPressure, oilTemp, fuelLevel, fuelTemp;
float ignitionTiming, injDuty, ignDuty, flexPct;
float pps, tps1, tps2, wastegate;
float aux1Temp, aux2Temp, mcuTemp;
float lam1, lam2, fpLow, fpHigh;

// Status & Info
uint8_t currentGear;
uint16_t warningCounter, lastError, distanceTraveled;
bool revLimAct, mainRelayAct, fuelPumpAct, celAct, egoHeatAct, lambdaProtectAct, fan2;
```

### 2. Konfigurasi Protokol (Config.h)
```cpp
// CAN Protocol types
#define CAN_PROTOCOL_HALTECH 0
#define CAN_PROTOCOL_RUSEFI 1
#define DEFAULT_CAN_PROTOCOL CAN_PROTOCOL_HALTECH

// RusEFI CAN IDs
#define RUSEFI_ID_0x200 0x200  // Warning Counter, Status Bits, Gear
#define RUSEFI_ID_0x201 0x201  // RPM, Ignition Timing, Duties, VSS
#define RUSEFI_ID_0x202 0x202  // PPS, TPS1, TPS2, Wastegate  
#define RUSEFI_ID_0x203 0x203  // MAP, Temperatures, Fuel Level
#define RUSEFI_ID_0x204 0x204  // Oil Press/Temp, Fuel Temp, Battery
#define RUSEFI_ID_0x207 0x207  // Lambda 1&2, Fuel Pressures
```

### 3. Handler Terpisah (CANHandler.cpp)
- `handleHaltechCAN()` - untuk protokol Haltech
- `handleRusEFICAN()` - untuk protokol RusEFI

### 4. Sistem Konfigurasi (DisplayConfig.cpp)
Fungsi untuk menyimpan/memuat protokol:
```cpp
uint8_t getCanProtocol();
void setCanProtocol(uint8_t protocol);  
void loadCanProtocol();
```

## Mapping Data RusEFI

### CAN ID 0x200 - Status & Warning
| Data | Bytes | Scaling | Deskripsi |
|------|-------|---------|-----------|
| WarningCounter | 0-1 | 1 | Jumlah warning |
| LastError | 2-3 | 1 | Kode error terakhir |
| Status Bits | 4 | bit flags | RevLim, MainRelay, FuelPump, CEL, dll |
| CurrentGear | 5 | 1 | Gear saat ini |
| DistanceTraveled | 6-7 | 0.1 km | Jarak tempuh |

### CAN ID 0x201 - Engine Core
| Data | Bytes | Scaling | Deskripsi |
|------|-------|---------|-----------|
| RPM | 0-1 | 1 | Engine RPM |
| IgnitionTiming | 2-3 | 0.02° | Timing pengapian |
| InjDuty | 4 | 0.5% | Duty cycle injector |
| IgnDuty | 5 | 0.5% | Duty cycle ignition |
| VehicleSpeed | 6 | 1 kph | Kecepatan |
| FlexPct | 7 | 1% | Persentase ethanol |

### CAN ID 0x202 - Throttle & Wastegate
| Data | Bytes | Scaling | Deskripsi |
|------|-------|---------|-----------|
| PPS | 0-1 | 0.01% | Pedal position sensor |
| TPS1 | 2-3 | 0.01% | Throttle position 1 |
| TPS2 | 4-5 | 0.01% | Throttle position 2 |
| Wastegate | 6-7 | 0.01% | Wastegate position |

### CAN ID 0x203 - Temperatures & MAP
| Data | Bytes | Scaling | Deskripsi |
|------|-------|---------|-----------|
| MAP | 0-1 | 0.03333 kPa | Manifold pressure |
| CoolantTemp | 2 | 1°C - 40 | Suhu coolant |
| IntakeTemp | 3 | 1°C - 40 | Suhu intake |
| AUX1Temp | 4 | 1°C - 40 | Suhu auxiliary 1 |
| AUX2Temp | 5 | 1°C - 40 | Suhu auxiliary 2 |
| MCUTemp | 6 | 1°C - 40 | Suhu MCU |
| FuelLevel | 7 | 0.5% | Level bahan bakar |

### CAN ID 0x204 - Oil & Battery
| Data | Bytes | Scaling | Deskripsi |
|------|-------|---------|-----------|
| OilPress | 2-3 | 0.03333 kPa | Tekanan oli |
| OilTemperature | 4 | 1°C - 40 | Suhu oli |
| FuelTemperature | 5 | 1°C - 40 | Suhu bahan bakar |
| BattVolt | 6-7 | 0.001 V | Tegangan baterai |

### CAN ID 0x207 - Lambda & Fuel Pressure
| Data | Bytes | Scaling | Deskripsi |
|------|-------|---------|-----------|
| Lam1 | 0-1 | 0.0001 λ | Lambda sensor 1 |
| Lam2 | 2-3 | 0.0001 λ | Lambda sensor 2 |
| FpLow | 4-5 | 0.03333 kPa | Fuel pressure low |
| FpHigh | 6-7 | 0.1 bar | Fuel pressure high |

## Perintah Serial Baru

### Switching Protocol
```
p atau P = Switch CAN protocol (Haltech ↔ RusEFI)
s atau S = Show CAN status dan data
```

### Informasi Status
Perintah `s` menampilkan:
- Mode komunikasi (CAN/Serial)
- Protokol CAN aktif (Haltech/RusEFI) 
- Kecepatan CAN (bps)
- Data sensor real-time
- Data khusus RusEFI (jika aktif): Gear, Oil Pressure/Temp, Fuel Level

## Cara Penggunaan

### 1. Switch ke RusEFI Protocol
1. Hubungkan via serial terminal
2. Ketik `p` dan tekan Enter
3. Restart device
4. Dashboard akan menggunakan protokol RusEFI

### 2. Kembali ke Haltech Protocol  
1. Ketik `p` lagi via serial
2. Restart device
3. Dashboard kembali ke protokol Haltech

### 3. Monitor Status
- Ketik `s` untuk melihat status dan data real-time
- Ketik `h` untuk help menu lengkap

## Kompatibilitas Data

Data yang compatible antara kedua protokol:
- RPM → RPM
- VSS → VehicleSpeed  
- MAP → MAP
- CLT → CoolantTemp
- IAT → IntakeTemp
- TPS → TPS1
- ADV → IgnitionTiming
- AFR → Lam1 * 14.7
- Battery → BattVolt
- FP → OilPressure (untuk display)

Data tambahan RusEFI:
- Current Gear
- Oil Temperature
- Fuel Level
- Injection/Ignition Duty
- Multiple lambda sensors
- Status flags (Rev limiter, CEL, dll)

## Catatan Penting

1. **Restart Required**: Perubahan protokol memerlukan restart device
2. **EEPROM Storage**: Konfigurasi disimpan di EEPROM address 2
3. **Default**: Protokol default adalah Haltech untuk kompatibilitas
4. **Backward Compatible**: Semua fungsi existing tetap berfungsi
5. **Memory Usage**: Penambahan ~50 variabel global untuk data RusEFI