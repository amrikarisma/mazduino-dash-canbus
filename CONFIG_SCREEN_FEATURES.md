# Configuration Screen Features

## Overview
Configuration Screen sekarang mendukung kontrol touch untuk mengubah pengaturan sistem secara real-time.

## Fitur yang Ditambahkan

### 1. Communication Mode Control
**Posisi:** Section kedua (85px dari atas)
**Fungsi:** Touch untuk cycling melalui mode komunikasi

**Cycle Order:**
1. **CAN Haltech 500Kbps** → 2. **CAN Haltech 1Mbps** → 3. **CAN RusEFI 500Kbps** → 4. **CAN RusEFI 1Mbps** → 5. **Serial Mode A** → 6. **Serial Mode N** → (kembali ke 1)

**Display Info:**
- CAN Mode: "CAN [Haltech/RusEFI] [500K/1M]"  
- Serial Mode: "Serial [Mode A/Mode N]"

### 2. Brightness Control
**Posisi:** Section kelima (190px dari atas)
**Fungsi:** Touch untuk cycling melalui level brightness

**Brightness Levels:**
- 25% (64/255) → 50% (128/255) → 75% (192/255) → 100% (255/255) → (kembali ke 25%)

**Features:**
- Perubahan langsung diterapkan ke LCD
- Disimpan otomatis ke EEPROM
- Display: "💡 Brightness XX%"

### 3. Debug Mode Toggle  
**Posisi:** Section keenam (225px dari atas)
**Fungsi:** Touch untuk toggle debug mode ON/OFF

**Display:** "🔧 Debug Mode [ON/OFF]"

## Technical Implementation

### Touch Areas
```cpp
TouchArea areas[] = {
    {10, 50, 460, 30, 0, "Status"},
    {10, 85, 460, 30, 1, "CommMode"},    // NEW
    {10, 120, 460, 30, 2, "WiFi"},
    {10, 155, 460, 30, 3, "Display"},
    {10, 190, 460, 30, 4, "Brightness"}, // ENHANCED
    {10, 225, 460, 30, 5, "Debug"},      // ENHANCED  
    {10, 260, 460, 30, 6, "Info"}
};
```

### Configuration Functions
- `handleSectionTouch(int sectionId)` - Route touch ke handler yang sesuai
- `toggleCommMode()` - Cycle komunikasi mode dengan debounce 1 detik
- `adjustBrightness()` - Cycle brightness dengan debounce 200ms
- `toggleDebugMode()` - Toggle debug dengan debounce 500ms

### Data Storage
**Communication Mode:**
- `commMode` → EEPROM address 1
- `canProtocol` → EEPROM address 2  
- `canSpeed` → DisplayConfig
- `speeduinoDataMode` → EEPROM address 14

**Brightness:**
- `backlightBrightness` → EEPROM address 11
- Flag → EEPROM address 12 (0xAA)

**Debug Mode:**
- `debugMode` → Runtime variable (tidak disimpan)

## User Interface

### Display Format
```
📊 System Status        [CPU/Mem info]
🔗 Communication       [Mode Protocol Speed]  
📡 WiFi                [Status/IP]
🖥️ Display Options      [Theme & Layout]
💡 Brightness          [XX%]
🔧 Debug Mode          [ON/OFF]
ℹ️ System Info         [Free Memory]
```

### Visual Feedback
- **Selected section:** Background biru, teks putih
- **Normal section:** Background abu gelap, teks abu muda
- **Real-time updates:** Brightness langsung berubah di LCD
- **Serial logging:** Setiap perubahan dicetak ke Serial

## Touch Integration

### TouchHandler Integration
```cpp
// CONFIG screen touches handled by configScreen
if (currentScreen == SCREEN_CONFIG) {
    if (configScreen.handleTouch(touch.x, touch.y)) {
        Serial.printf("[Touch] Config handled touch at (%d, %d)\n", touch.x, touch.y);
    }
}
```

### Debounce Protection
- Communication Mode: 1000ms (mencegah perubahan terlalu cepat)
- Brightness: 200ms (smooth cycling)
- Debug Mode: 500ms (prevent accidents)

## Benefits

### 1. Ease of Use
- Tidak perlu serial terminal untuk mengubah pengaturan
- Touch interface intuitif
- Visual feedback immediate

### 2. Real-time Configuration
- Brightness berubah langsung saat ditouch
- Communication mode tersimpan otomatis
- Tidak perlu restart untuk sebagian besar perubahan

### 3. Complete Control
- Semua protokol CAN (Haltech + RusEFI)
- Semua kecepatan CAN (500K + 1M)
- Semua mode Serial (A + N)
- Fine-grained brightness control

### 4. User Feedback
- Status ditampilkan real-time di screen
- Serial logging untuk debugging
- Visual selection highlighting

## Usage Instructions

### Mengubah Communication Mode:
1. Swipe ke CONFIG screen
2. Touch pada "🔗 Communication" section
3. Mode akan cycle: Haltech 500K → Haltech 1M → RusEFI 500K → RusEFI 1M → Serial A → Serial N
4. Pengaturan tersimpan otomatis

### Mengubah Brightness:
1. Touch pada "💡 Brightness" section  
2. Level akan cycle: 25% → 50% → 75% → 100%
3. Perubahan langsung terlihat di LCD
4. Setting tersimpan ke EEPROM

### Toggle Debug Mode:
1. Touch pada "🔧 Debug Mode" section
2. Mode toggle antara ON ↔ OFF
3. Debug info muncul di layar saat ON

## Compatibility Notes

- ✅ Compatible dengan semua screen modes existing
- ✅ Swipe gestures tetap berfungsi normal  
- ✅ Serial commands tetap tersedia sebagai backup
- ✅ Web interface tetap berfungsi
- ✅ EEPROM settings preserved across restarts
- ✅ No conflicts dengan touch areas lain

## Future Enhancements

Potential additions:
- Volume control untuk buzzer/beeper
- Screen timeout settings
- Color theme selection
- Custom splash screen selection
- Advanced CAN filtering options