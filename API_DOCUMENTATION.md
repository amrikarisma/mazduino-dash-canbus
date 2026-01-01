# MAZDUINO Display API Documentation

## Overview
MAZDUINO Display menyediakan REST API yang dapat diakses dari jaringan lokal atau internet (tergantung konfigurasi WiFi). API mendukung CORS untuk integrasi dengan aplikasi web eksternal seperti mazduino.com.

**Key Features:**
- **Smart Power Management**: WiFi tidak akan mati otomatis jika ada aktivitas API dalam 30 detik terakhir
- **External Monitoring**: API dirancang khusus untuk integrasi dengan dashboard mazduino.com
- **Real-time Data**: Data ECU/CAN bus ter-update setiap saat
- **CORS Support**: Dapat diakses dari domain external tanpa masalah browser security

## Base URL
- **AP Mode**: `http://192.168.4.1`
- **Station Mode**: `http://<device-ip>` (cek IP di router DHCP client list)

## Authentication
Saat ini tidak ada autentikasi yang diperlukan. Semua endpoint bersifat public.

---

## Device Information API (Compatible with mazduino.com)

### Get Device Info
**Endpoint**: `GET /api/info` (Primary endpoint for mazduino.com)

**Description**: Mendapatkan informasi perangkat sesuai dengan format yang dibutuhkan mazduino.com.

**Headers**: 
- `Access-Control-Allow-Origin: *`
- `Access-Control-Allow-Methods: GET, OPTIONS`
- `Access-Control-Allow-Headers: Content-Type`

**Response**:
```json
{
  "device_type": "mazduino_display",
  "chip": "ESP32", 
  "device_name": "Mazduino Display #1",
  "hostname": "mazduino-display-001",
  "version": "1.0.0",
  "uptime": "02:34:12",
  "ip_address": "192.168.1.100",
  "mac_address": "AA:BB:CC:DD:EE:FF",
  "wifi_ssid": "MyNetwork",
  "wifi_signal": -45,
  "free_heap": 123456,
  "total_heap": 327680,
  "display": {
    "type": "TFT",
    "width": 480,
    "height": 320,
    "current_brightness": 50,
    "current_mode": "normal"
  }
}
```

**Field Descriptions**:
- `device_type`: Selalu `"mazduino_display"` untuk device discovery
- `chip`: Type chip ESP32
- `device_name`: Nama human-readable device
- `uptime`: Waktu hidup device dalam format HH:MM:SS
- `free_heap`: Memory bebas dalam bytes
- `display`: Info display termasuk brightness dan mode saat ini

---

## ECU Data API

### Get ECU/CAN Data  
**Endpoint**: `GET /api/can/data`

**Description**: Mendapatkan data real-time dari ECU melalui CAN bus atau Serial (Speeduino).

**Headers**: 
- `Access-Control-Allow-Origin: *`
- `Access-Control-Allow-Methods: GET, OPTIONS`
- `Access-Control-Allow-Headers: Content-Type`

**Response**:
```json
{
  "rpm": 2500,
  "coolant": 85.5,
  "iat": 25.2,
  "tps": 45.8,
  "map": 65.3,
  "afr": 14.7,
  "advance": 15.5,
  "trigger": 123,
  "voltage": 13.8,
  "fuel_pressure": 3.2,
  "vss": 65.0,
  "timestamp": 1234567890
}
```

**Field Descriptions**:
- `rpm`: Engine RPM
- `coolant`: Coolant temperature (°C)
- `iat`: Intake Air Temperature (°C)  
- `tps`: Throttle Position Sensor (%)
- `map`: Manifold Absolute Pressure (kPa)
- `afr`: Air-Fuel Ratio
- `advance`: Ignition advance (degrees)
- `trigger`: Trigger error counter
- `voltage`: Battery voltage (V)
- `fuel_pressure`: Fuel pressure (bar)
- `vss`: Vehicle Speed Sensor (km/h)
- `timestamp`: System timestamp in milliseconds

---

## Display Control API

### Display Text
**Endpoint**: `POST /api/display/text`

**Description**: Menampilkan teks pada display (akan diimplementasikan).

**Request Body**:
```json
{
  "text": "Hello World!",
  "x": 0,
  "y": 0,
  "clear_first": true
}
```

**Response**: `200 OK`
```json
{
  "success": true,
  "message": "Text displayed successfully"
}
```

### Set Display Brightness
**Endpoint**: `POST /api/display/brightness`

**Description**: Mengatur brightness display.

**Request Body**:
```json
{
  "brightness": 75
}
```

**Parameters**:
- `brightness`: Integer 0-100

**Response**: `200 OK`
```json
{
  "success": true,
  "brightness": 75,
  "message": "Brightness set to 75%"
}
```

### Display Status
**Endpoint**: `GET /api/display/status`

**Description**: Mendapatkan status display saat ini.

**Response**:
```json
{
  "brightness": 50,
  "mode": "normal",
  "current_text": "Dashboard Active",
  "is_displaying": true,
  "last_update": "2026-01-02T10:30:00Z"
}
```

---

## WiFi Management API

### Get WiFi Status
**Endpoint**: `GET /wifi/status`

**Description**: Mendapatkan status WiFi lengkap termasuk mode, IP address, dan koneksi.

**Response**:
```json
{
  "mode": "Access Point (AP)" | "Station (Connected to Router)" | "Station (Disconnected)" | "Access Point + Station" | "Off",
  "ip": "192.168.4.1",
  "apip": "192.168.4.1",
  "clients": 2,
  "ssid": "MyRouter",
  "rssi": -45,
  "connected": true,
  "network": "MyRouter",
  "device": "MAZDUINO Display",
  "version": "1.0",
  "uptime": 3600
}
```

### Scan WiFi Networks
**Endpoint**: `GET /wifi/scan`

**Description**: Scan jaringan WiFi yang tersedia di sekitar.

**Response**:
```json
[
  {
    "ssid": "MyRouter",
    "rssi": -45,
    "encryption": "WPA2"
  }
]
```

### Connect to Router
**Endpoint**: `POST /wifi/connect`

**Description**: Menghubungkan device ke router WiFi.

**Request Body**:
```
Content-Type: application/x-www-form-urlencoded
ssid=MyRouter&password=mypassword123
```

**Response**: `200 OK` - Device akan restart dan mencoba koneksi

---

## Smart Power Management

### API Activity Tracking
API memiliki sistem tracking aktivitas pintar:

- **WiFi Stay-Alive**: Jika ada request ke endpoint API dalam 30 detik terakhir, WiFi tidak akan dimatikan
- **External Monitoring**: Ideal untuk dashboard mazduino.com yang melakukan polling reguler
- **Auto-Shutdown**: WiFi tetap akan mati jika tidak ada aktivitas selama 1 menit untuk menghemat power
- **Logging**: Aktivitas API akan dicatat di serial output untuk debugging

### Monitored Endpoints:
- `GET /api/device/info` - Device information
- `GET /api/can/data` - ECU data (most frequently accessed)
- `GET /wifi/status` - WiFi status

**Example Log Output**:
```
[API] Device info requested - WiFi kept active
[WiFi] API activity detected - keeping WiFi active
No clients connected and no API activity for 1 minute - shutting down WiFi/Bluetooth
```

---

## Integration dengan Mazduino.com

### Automatic Device Discovery
```javascript
// Scan for devices on network
const discoveredDevices = [];

// Check common ESP32 ports
const commonPorts = [80, 8080, 3000, 8000];
const networkRange = '192.168.1.'; // Adjust based on network

for(let i = 1; i <= 254; i++) {
  for(const port of commonPorts) {
    try {
      const response = await fetch(`http://${networkRange}${i}:${port}/api/info`, {
        method: 'GET',
        timeout: 2000
      });
      
      if(response.ok) {
        const deviceInfo = await response.json();
        if(deviceInfo.device_type === 'mazduino_display') {
          discoveredDevices.push({
            ip: `${networkRange}${i}`,
            port: port,
            ...deviceInfo
          });
        }
      }
    } catch(e) {
      // Device not found or timeout
    }
  }
}
```

### Real-time Dashboard
```javascript
// Auto-refresh dashboard every 1 second with smart error handling
class MazduinoDashboard {
  constructor(deviceIP) {
    this.deviceIP = deviceIP;
    this.isConnected = false;
    this.retryCount = 0;
    this.maxRetries = 3;
  }
  
  async startMonitoring() {
    setInterval(async () => {
      try {
        const [deviceInfo, canData, wifiStatus] = await Promise.all([
          fetch(`http://${this.deviceIP}/api/device/info`).then(r => r.json()),
          fetch(`http://${this.deviceIP}/api/can/data`).then(r => r.json()),
          fetch(`http://${this.deviceIP}/wifi/status`).then(r => r.json())
        ]);
        
        // Reset retry count on success
        this.retryCount = 0;
        this.isConnected = true;
        
        this.updateDashboard(deviceInfo, canData, wifiStatus);
        
      } catch (error) {
        this.handleConnectionError(error);
      }
    }, 1000);
  }
  
  handleConnectionError(error) {
    this.retryCount++;
    
    if(this.retryCount >= this.maxRetries) {
      this.isConnected = false;
      console.error('Device disconnected after max retries');
      this.showOfflineStatus();
    } else {
      console.warn(`Connection retry ${this.retryCount}/${this.maxRetries}`);
    }
  }
  
  updateDashboard(deviceInfo, canData, wifiStatus) {
    // Update UI elements
    document.getElementById('rpm-value').textContent = canData.rpm;
    document.getElementById('coolant-temp').textContent = canData.coolant + '°C';
    document.getElementById('device-uptime').textContent = deviceInfo.uptime;
    document.getElementById('connection-status').textContent = 'Connected';
    
    // Update charts, gauges, etc.
    this.updateRPMGauge(canData.rpm);
    this.updateTempChart(canData.coolant, canData.iat);
  }
}

// Usage
const dashboard = new MazduinoDashboard('192.168.1.100');
dashboard.startMonitoring();
```

---

## Error Handling

### Common HTTP Status Codes:
- `200`: Success
- `400`: Bad Request (missing parameters)
- `404`: Endpoint not found
- `500`: Internal server error

### Error Response Format:
```json
{
  "success": false,
  "error": "invalid_parameter",
  "message": "Human readable error message"
}
```

### Network Connectivity Issues:
1. **Device Discovery**: Try multiple IP ranges and common ports
2. **Connection Timeout**: Use 2-second timeout for discovery, 5-second for data
3. **Retry Logic**: Implement exponential backoff for failed requests
4. **Fallback Mode**: Show cached data when device is temporarily unavailable

---

## Performance Considerations

### Request Frequency:
- **ECU Data**: Up to 10 requests/second (for real-time monitoring)
- **Device Info**: 1 request every 10 seconds
- **WiFi Status**: 1 request every 30 seconds

### Memory Usage:
- **Free Heap**: Monitor via `/api/device/info` - device will restart if critically low
- **Flash Usage**: Currently at 47.7% - safe for OTA updates

### Network Optimization:
- **Keep-Alive**: API calls prevent WiFi shutdown - ideal for continuous monitoring
- **CORS**: No preflight delays for simple GET requests
- **JSON Minified**: Responses are compact for faster transmission

---

## Security Notes

1. **Local Network**: Recommended for local network usage only
2. **No Authentication**: Open access - consider network segmentation  
3. **CORS Open**: Accessible from any domain for web integration
4. **Read-Only**: Most endpoints are read-only for safety
5. **Rate Limiting**: None implemented - client-side throttling recommended

---

## Development & Testing

### cURL Examples:
```bash
# Device discovery
curl -m 2 http://192.168.1.100/api/info

# Real-time ECU data
curl http://192.168.1.100/api/can/data

# WiFi status
curl http://192.168.1.100/wifi/status

# Set brightness
curl -X POST -H "Content-Type: application/json" \
  -d '{"brightness":80}' \
  http://192.168.1.100/api/display/brightness
```

### JavaScript Testing:
```javascript
// Test device connectivity
async function testDevice(ip) {
  try {
    const response = await fetch(`http://${ip}/api/info`, {
      method: 'GET',
      timeout: 2000
    });
    
    if(response.ok) {
      const info = await response.json();
      console.log('Device found:', info.device_name);
      return true;
    }
  } catch(e) {
    console.log('Device not responding at', ip);
  }
  return false;
}

// Test data quality
async function validateECUData(ip) {
  const data = await fetch(`http://${ip}/api/can/data`).then(r => r.json());
  
  const validations = {
    rpm: data.rpm >= 0 && data.rpm <= 10000,
    coolant: data.coolant >= -40 && data.coolant <= 150,
    voltage: data.voltage >= 10 && data.voltage <= 16
  };
  
  console.log('Data validation:', validations);
  return Object.values(validations).every(v => v);
}
```

---

## Changelog

### Version 1.0 (January 2026)
- Initial API release compatible with mazduino.com
- Smart power management with API activity tracking
- WiFi management endpoints
- Real-time ECU/CAN data API
- Display control endpoints  
- CORS support for external web integration
- Device discovery capabilities
- Performance optimized for continuous monitoring