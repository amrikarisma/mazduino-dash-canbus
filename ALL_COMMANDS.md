# Mazduino Dashboard - All Commands Reference

## 📋 **Serial Commands Summary**

### 🎮 **Simulator Commands** (Mode: Simulator)
```
0 = OFF (use real data)
1 = RPM SWEEP (0-6000 RPM)
2 = ENGINE IDLE (800-900 RPM)  
3 = DRIVING (1500-4000 RPM)
4 = REDLINE (5500-6000 RPM)
h = Show simulator help
```

### 🔧 **Debug Commands** (Mode: Debug)
```
d = Toggle debug mode ON/OFF
i = Show system information
```

### 🌐 **Web Server Commands** (Mode: WiFi Access Point)
```
Network: MAZDUINO_Display (password: 12345678)
IP: 192.168.4.1
Auto-starts: 15 seconds after boot
Auto-shuts: 1 minute no connections

Web Interface Features:
- Real-time status updates
- Display mode toggle (ECU Data/Trigger Error)
- Communication mode selection (CAN/Serial)
- Debug mode toggle
- Simulator mode controls (0-4)
- OTA firmware update
- System information display
```

### 📊 **All Available Commands**
```
=== SIMULATOR COMMANDS ===
0 = Turn OFF simulator
1 = RPM sweep mode
2 = Engine idle mode  
3 = Driving mode
4 = Redline mode
h = Simulator help menu

=== DEBUG COMMANDS ===
d = Toggle debug display
i = System info dump

=== WEB INTERFACE COMMANDS ===
/debug - Toggle debug mode
/simulator - Control simulator modes
/status - Get real-time status (JSON)
/toggle - Toggle display configuration
/setMode - Change communication mode

=== HELP COMMANDS ===
h = Simulator help
i = System information
```

## 🎯 **Visual Indicators**

### On Display
```
[CAN/SER]     [DEBUG INFO]     [SIM]
Top-left      Top-center       Top-right

CAN  = Green text (CAN Bus mode)
SER  = Orange text (Serial mode)
DEBUG = Cyan text (CPU/FPS/Heap info)
SIM  = Yellow text (Simulator active)
```

### On Web Interface
```
Communication Mode:
- CAN Bus: Green indicator
- Serial: Orange indicator

Real-time Status:
- Updates every second
- Shows current mode, debug status, simulator mode
- Memory usage and uptime
```

## 🚀 **Quick Start Guide**

### 1. **Upload & Monitor:**
```bash
# Upload firmware
/Users/amri/Library/Python/3.12/bin/pio run -t upload

# Monitor serial
/Users/amri/Library/Python/3.12/bin/pio device monitor --baud 115200
```

### 2. **Expected Startup:**
```
=== MAZDUINO DASHBOARD STARTING ===
Communication mode: CAN
CAN mode aktif.
=== MAZDUINO SIMULATOR INITIALIZED ===
Available modes: 0,1,2,3,4,h
=== DEBUG MODE AVAILABLE ===
Send 'd' to toggle debug mode
Send 'i' for system info
```

### 3. **Test Commands:**
```
# Enable debug mode
d

# Expected output:
[DEBUG] Debug mode ON
=== DEBUG MODE ENABLED ===
CPU Usage and FPS will be displayed
Serial debug prints enabled
Send 'd' again to disable

# Check system info
i

# Expected output:
=== SYSTEM INFO ===
Free Heap: 291840 bytes
Min Free Heap: 285632 bytes
CPU Freq: 240 MHz
Flash Size: 4194304 bytes
Chip Model: ESP32
Chip Revision: 1
Uptime: 45 seconds
```

### 4. **Visual Confirmation:**
**Debug Mode ON:**
- Pojok kiri atas: `CPU:15.2% FPS:12.3 Heap:285KB DBG`
- Pojok kanan atas: `SIM` (jika simulator aktif)

**Debug Mode OFF:**
- Area debug bersih
- Hanya `SIM` indicator jika simulator aktif

## 🎯 **Use Cases**

### **Development & Testing:**
```
1. Enable debug: 'd'
2. Start simulator: '1' (RPM sweep)
3. Monitor performance
4. Check system info: 'i'
5. Disable debug: 'd'
```

### **Performance Monitoring:**
```
1. Enable debug: 'd'
2. Monitor CPU usage, FPS, memory
3. Test different modes: '1','2','3','4'
4. Check system stability
```

### **Troubleshooting:**
```
1. Check system info: 'i'
2. Enable debug: 'd'
3. Monitor metrics in real-time
4. Check for memory leaks
5. Verify performance
```

## 📊 **Expected Performance**

### **Normal Operation:**
- **CPU Usage**: 10-30%
- **FPS**: 8-15 FPS
- **Free Heap**: >250KB
- **Loop Time**: <100ms

### **With Debug Mode:**
- **CPU Usage**: +5-10% overhead
- **FPS**: Minimal impact
- **Free Heap**: -5KB for debug variables
- **Loop Time**: +10-20ms

### **With Simulator:**
- **CPU Usage**: +10-20% (depends on mode)
- **FPS**: Minimal impact
- **Free Heap**: -10KB for simulator data
- **Loop Time**: +20-30ms

## 🔧 **Configuration**

### **Enable/Disable Features:**
Edit `src/Config.h`:
```cpp
#define ENABLE_SIMULATOR 1   // 1=ON, 0=OFF
#define ENABLE_DEBUG_MODE 1  // 1=ON, 0=OFF
```

### **Debug Update Rates:**
Edit `src/main.cpp`:
```cpp
// CPU & FPS update: 1000ms (1 second)
// Serial debug print: 2000ms (2 seconds)
```

## ✅ **Success Criteria**

### **Startup Test:**
- [x] Simulator initializes with modes 0,1,2,3,4,h
- [x] Debug mode available with 'd','i' commands
- [x] Visual indicators work (SIM, DBG)
- [x] Serial commands respond properly

### **Simulator Test:**
- [x] Mode 1: RPM sweep 0→6000→0
- [x] Mode 2: Idle ~850 RPM
- [x] Mode 3: Driving 1500-4000 RPM
- [x] Mode 4: Redline 5750-5940 RPM
- [x] Mode 0: OFF, back to real data

### **Debug Test:**
- [x] 'd' toggles debug display ON/OFF
- [x] CPU usage shows reasonable values
- [x] FPS updates continuously
- [x] Memory info is accurate
- [x] System info command 'i' works

### **Performance Test:**
- [x] CPU usage <50% in normal operation
- [x] FPS >5 for smooth display
- [x] Memory >200KB free
- [x] No crashes or hangs
- [x] Responsive to commands

## 🎉 **Complete Feature Set**

### ✅ **Implemented Features:**
1. **Modular Code Structure** - Clean separation of concerns
2. **CAN Bus Support** - Real ECU data communication
3. **Simulator Mode** - 5 realistic test modes
4. **Debug Mode** - Performance monitoring
5. **Visual Indicators** - SIM, DBG status
6. **Serial Commands** - Interactive control
7. **RPM Bar Display** - Smooth animation
8. **Sensor Monitoring** - AFR, TPS, MAP, etc.
9. **Backlight Control** - Auto-adjust based on RPM
10. **Memory Management** - Heap monitoring
11. **FPS Monitoring** - Display performance
12. **CPU Usage** - System load monitoring
13. **System Info** - Hardware details
14. **Error Handling** - Graceful degradation
15. **Configuration** - Compile-time options

**Mazduino Dashboard sekarang lengkap dengan semua fitur debug dan monitoring! 🚀**

### **Final Commands:**
- **Simulator**: `0`,`1`,`2`,`3`,`4`,`h`
- **Debug**: `d`,`i`
- **Visual**: SIM indicator, DBG display
- **Performance**: CPU, FPS, Memory monitoring
