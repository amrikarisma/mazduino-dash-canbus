#include "WebServerHandler.h"
#include "Config.h"
#include "DataTypes.h"
#include "DisplayConfig.h"
#include "SplashScreen.h"
#include "BacklightControl.h"
#include "Comms.h"
#include "OTAUpdater.h"
#include "version.h"
#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>
#include <EEPROM.h>
#include <esp_wifi.h>
#include <esp_bt.h>
#include <esp_bt_main.h>
#include <esp_task_wdt.h>
#if ENABLE_SIMULATOR
#include "Simulator.h"
#endif

// Global OTA updater instance
OTAUpdater otaUpdater;

// IP configuration - Simple approach, let ESP32 use default IP
// Default AP IP is usually 192.168.4.1

// Global variable for OTA update size  
static size_t otaUpdateSize = 0;

// Global variable for API activity tracking
static uint32_t lastApiActivity = 0;

// OTA upload page HTML
const char *uploadPage PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
  <head>
    <title>MAZDUINO Display Control</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
      body {
        font-family: Arial, sans-serif;
        margin: 0;
        padding: 20px;
        background-color: #1a1a1a;
        color: #ffffff;
      }
      .container {
        max-width: 600px;
        margin: 0 auto;
      }
      h1, h2 {
        color: #4CAF50;
        text-align: center;
      }
      h1 {
        border-bottom: 2px solid #4CAF50;
        padding-bottom: 10px;
        margin-bottom: 30px;
      }
      h2 {
        font-size: 18px;
        margin-bottom: 15px;
      }
      .section {
        background-color: #2a2a2a;
        padding: 20px;
        margin: 20px 0;
        border-radius: 8px;
        border: 1px solid #444;
        border-left: 4px solid #4CAF50;
      }
      .btn {
        padding: 12px 24px;
        font-size: 16px;
        background-color: #4CAF50;
        color: white;
        border: none;
        border-radius: 5px;
        cursor: pointer;
        margin: 5px;
        width: 100%;
        transition: background-color 0.3s;
      }
      .btn:hover {
        background-color: #45a049;
      }
      .btn.danger {
        background-color: #f44336;
      }
      .btn.danger:hover {
        background-color: #da190b;
      }
      .btn:disabled {
        background-color: #666;
        color: #999;
        cursor: not-allowed;
        opacity: 0.6;
      }
      .btn.active {
        background-color: #2E7D32;
        font-weight: bold;
        box-shadow: 0 0 10px rgba(76, 175, 80, 0.3);
      }
      .toggle-btn.off {
        background-color: #FF6B6B;
      }
      .toggle-btn.on {
        background-color: #4ECDC4;
      }
      .status {
        background-color: #333;
        padding: 10px;
        border-radius: 5px;
        margin: 10px 0;
      }
      .file-input {
        width: 100%;
        padding: 10px;
        background-color: #444;
        border: 1px solid #666;
        border-radius: 5px;
        color: white;
        margin: 10px 0;
      }
      .grid {
        display: grid;
        grid-template-columns: 1fr 1fr;
        gap: 10px;
      }
      .config-grid {
        display: grid;
        grid-template-columns: 1fr 1fr;
        gap: 15px;
        margin: 15px 0;
      }
      .config-item {
        display: flex;
        flex-direction: column;
      }
      .config-item label {
        font-weight: bold;
        margin-bottom: 5px;
        color: #4CAF50;
      }
      .config-item select {
        padding: 8px;
        background-color: #444;
        border: 1px solid #666;
        border-radius: 4px;
        color: white;
      }
      .indicator-grid {
        display: grid;
        grid-template-columns: repeat(4, 1fr);
        gap: 10px;
        margin: 15px 0;
      }
      .indicator-grid label {
        display: flex;
        align-items: center;
        gap: 8px;
        font-size: 14px;
      }
      .indicator-grid input[type="checkbox"] {
        width: 16px;
        height: 16px;
      }
      .config-controls {
        display: flex;
        gap: 10px;
        margin: 20px 0;
      }
      .config-controls .btn {
        flex: 1;
      }
      .splash-logo { display:none;}
      h3 {
        color: #4CAF50;
        font-size: 16px;
        margin: 20px 0 10px 0;
        border-bottom: 1px solid #444;
        padding-bottom: 5px;
      }
      .floating-config {
        position: fixed;
        bottom: 20px;
        right: 20px;
        background: rgba(42, 42, 42, 0.95);
        border: 2px solid #4CAF50;
        border-radius: 10px;
        padding: 15px;
        display: flex;
        align-items: center;
        gap: 20px;
        box-shadow: 0 4px 20px rgba(0, 0, 0, 0.3);
        backdrop-filter: blur(10px);
        z-index: 1000;
      }
      .floating-config h3 {
        margin: 0;
        color: #4CAF50;
        font-size: 14px;
        border: none;
        padding: 0;
      }
      .config-actions {
        display: flex;
        gap: 10px;
      }
      .config-actions .btn {
        padding: 8px 16px;
        font-size: 14px;
        margin: 0;
        width: auto;
        min-width: 120px;
      }
      .btn.primary {
        background-color: #4CAF50;
      }
      .btn.primary:disabled {
        background-color: #666;
        color: #999;
        cursor: not-allowed;
        opacity: 0.6;
      }
      .nav-bar {
        background-color: #2a2a2a;
        border: 1px solid #444;
        border-radius: 8px;
        margin: 20px 0;
        padding: 0;
        display: flex;
        flex-wrap: wrap;
        overflow: hidden;
      }
      .nav-item {
        flex: 1;
        min-width: 120px;
        padding: 12px 15px;
        background-color: #2a2a2a;
        color: #ffffff;
        border: none;
        border-right: 1px solid #444;
        cursor: pointer;
        font-size: 14px;
        font-weight: 500;
        text-align: center;
        transition: all 0.3s ease;
        white-space: nowrap;
        overflow: hidden;
        text-overflow: ellipsis;
      }
      .nav-item:last-child {
        border-right: none;
      }
      .nav-item:hover {
        background-color: #3a3a3a;
        color: #4CAF50;
      }
      .nav-item.active {
        background-color: #4CAF50;
        color: white;
        font-weight: bold;
      }
      .content-section {
        display: none;
      }
      .content-section.active {
        display: block;
      }
      @media (max-width: 600px) {
        .grid {
          grid-template-columns: 1fr;
        }
        .config-grid {
          grid-template-columns: 1fr;
        }
        .indicator-grid {
          grid-template-columns: repeat(2, 1fr);
        }
        .nav-bar {
          flex-direction: column;
        }
        .nav-item {
          border-right: none;
          border-bottom: 1px solid #444;
          flex: none;
        }
        .nav-item:last-child {
          border-bottom: none;
        }
        .floating-config {
          bottom: 10px;
          right: 10px;
          left: 10px;
          flex-direction: column;
          gap: 10px;
        }
        .config-actions {
          width: 100%;
        }
        .config-actions .btn {
          flex: 1;
        }
      }
    </style>
    <script>
      function toggleDisplay(button) {
        const isCurrentlyOff = button.classList.contains('off');
        if (isCurrentlyOff) {
          button.classList.remove('off');
          button.classList.add('on');
          button.textContent = "Display Mode: ECU Data";
          fetch('/toggle', { method: 'POST', body: 'on' });
        } else {
          button.classList.remove('on');
          button.classList.add('off');
          button.textContent = "Display Mode: Trigger Error";
          fetch('/toggle', { method: 'POST', body: 'off' });
        }
      }
      
      function setCommMode(mode) {
        // Update UI immediately
        updateCommModeButtons(mode === 'can' ? 0 : 1);
        // Mark as pending change
        markPendingChange('commMode', mode);
      }
      
      function updateCommModeButtons(currentMode) {
        const canBtn = document.getElementById('canModeBtn');
        const serialBtn = document.getElementById('serialModeBtn');
        const canConfig = document.getElementById('canConfiguration');
        const serialConfig = document.getElementById('serialConfiguration');
        
        if (currentMode === 0 || currentMode === 'CAN Bus') { // CAN mode
          if (canBtn) {
            canBtn.classList.add('active');
            canBtn.disabled = true;
          }
          if (serialBtn) {
            serialBtn.classList.remove('active');
            serialBtn.disabled = false;
          }
          
          if (canConfig) canConfig.style.display = 'block';
          if (serialConfig) serialConfig.style.display = 'none';
        } else { // Serial mode
          if (serialBtn) {
            serialBtn.classList.add('active');
            serialBtn.disabled = true;
          }
          if (canBtn) {
            canBtn.classList.remove('active');
            canBtn.disabled = false;
          }
          
          if (canConfig) canConfig.style.display = 'none';
          if (serialConfig) serialConfig.style.display = 'block';
        }
      }
      
      function toggleDebug() {
        fetch('/debug', { 
          method: 'POST', 
          headers: {'Content-Type': 'application/x-www-form-urlencoded'},
          body: 'toggle=1' 
        })
        .then(response => response.text())
        .then(data => {
          alert('Debug mode: ' + data);
        });
      }
      
      function setSimulator(mode) {
        fetch('/simulator', { 
          method: 'POST', 
          headers: {'Content-Type': 'application/x-www-form-urlencoded'},
          body: 'mode=' + mode 
        })
        .then(response => response.text())
        .then(data => {
          alert('Simulator: ' + data);
        });
      }
      
      function refreshStatus() {
        fetch('/status')
          .then(response => response.json())
          .then(data => {
            const uptime = Math.floor(data.uptime);
            const commColor = data.commMode === 'CAN Bus' ? '#4CAF50' : '#FF9800';
            
            // Update main status
            document.getElementById('status').innerHTML = 
              'Status: Connected<br>' +
              'WiFi: ' + data.wifiStatus + '<br>' +
              'Communication: <span style="color: ' + commColor + '; font-weight: bold;">' + data.commMode + ' Mode</span><br>' +
              'Debug Mode: ' + (data.debugMode ? 'ON' : 'OFF') + '<br>' +
              'Simulator: Mode ' + data.simulatorMode + '<br>' +
              'Uptime: ' + uptime + ' seconds<br>' +
              'Free Memory: ' + Math.round(data.freeHeap / 1024) + 'KB';
            
            // Update WiFi section
            document.getElementById('currentWifiMode').textContent = data.wifiMode;
            document.getElementById('currentIP').textContent = data.ipAddress;
            
            if (data.wifiMode === 'Station (Connected)') {
              document.getElementById('wifiStatus').innerHTML = 
                '<strong>Current Mode:</strong> <span style="color: #4CAF50">Station (Connected to Router)</span><br>' +
                '<strong>Network:</strong> ' + data.ssid + '<br>' +
                '<strong>IP Address:</strong> ' + data.ipAddress + '<br>' +
                '<strong>Signal Strength:</strong> ' + data.rssi + ' dBm<br>' +
                '<button class="btn danger" onclick="disconnectWiFi()">Switch to AP Mode</button>';
            } else {
              document.getElementById('wifiStatus').innerHTML = 
                '<strong>Current Mode:</strong> <span style="color: #FF9800">Access Point</span><br>' +
                '<strong>AP SSID:</strong> MAZDUINO_Display<br>' +
                '<strong>AP IP:</strong> ' + data.ipAddress + '<br>' +
                '<strong>Connected Clients:</strong> ' + data.clientCount;
            }
            
            // Update communication mode buttons and configurations
            updateCommModeButtons(data.commMode);
          })
          .catch(error => {
            console.error('Error fetching status:', error);
          });
      }
      
      function updatePanelConfig(position) {
        const select = document.getElementById('panel' + position);
        const dataSource = select.value;
        
        fetch('/configPanel', {
          method: 'POST',
          headers: {'Content-Type': 'application/x-www-form-urlencoded'},
          body: 'position=' + position + '&dataSource=' + dataSource
        })
        .then(response => response.text())
        .then(data => {
          console.log('Panel config updated:', data);
        });
      }
      
      function updateIndicatorConfig(indicator) {
        const checkbox = document.getElementById('ind' + indicator);
        const enabled = checkbox.checked;
        
        fetch('/configIndicator', {
          method: 'POST',
          headers: {'Content-Type': 'application/x-www-form-urlencoded'},
          body: 'indicator=' + indicator + '&enabled=' + (enabled ? '1' : '0')
        })
        .then(response => response.text())
        .then(data => {
          console.log('Indicator config updated:', data);
        });
      }
      
      function saveDisplayConfig() {
        fetch('/saveDisplayConfig', {
          method: 'POST'
        })
        .then(response => response.text())
        .then(data => {
          alert('Display configuration saved: ' + data);
        });
      }
      
      function resetDisplayConfig() {
        if (confirm('Reset display configuration to default? This will restart the device.')) {
          fetch('/resetDisplayConfig', {
            method: 'POST'
          })
          .then(response => response.text())
          .then(data => {
            alert('Display configuration reset: ' + data);
            location.reload();
          });
        }
      }
      
      function loadDisplayConfig() {
        fetch('/getDisplayConfig')
          .then(response => response.json())
          .then(data => {
            // Update panel selects
            for (let i = 0; i < 8; i++) {
              const select = document.getElementById('panel' + i);
              if (data.panels[i] && data.panels[i].enabled) {
                select.value = data.panels[i].dataSource;
              } else {
                select.value = 'disabled';
              }
            }
            
            // Update indicator checkboxes
            for (let i = 0; i < 8; i++) {
              const checkbox = document.getElementById('ind' + i);
              if (data.indicators[i]) {
                checkbox.checked = data.indicators[i].enabled;
              }
            }
          });
      }
      
      function updateCanSpeed() {
        const select = document.getElementById('canSpeedSelect');
        const speed = select.value;
        markPendingChange('canSpeed', speed);
      }
      function loadCanSpeed() {
        fetch('/canspeed')
          .then(response => response.text())
          .then(speed => {
            const select = document.getElementById('canSpeedSelect');
            if (select) select.value = speed;
          });
      }
      
      function updateSplashScreen() {
        const select = document.getElementById('splashSelect');
        const splash = select.value;
        markPendingChange('splash', splash);
      }
      
      function loadSplashScreen() {
        fetch('/splash')
          .then(response => response.text())
          .then(splash => {
            const select = document.getElementById('splashSelect');
            if (select) select.value = splash;
          });
      }
      
      function updateBrightness(value) {
        document.getElementById('brightnessValue').textContent = value;
        // Apply brightness immediately for instant LCD preview
        fetch('/brightness', {
          method: 'POST',
          headers: {'Content-Type': 'application/x-www-form-urlencoded'},
          body: 'brightness=' + value
        })
        .then(response => response.text())
        .then(data => {
          console.log('Brightness updated immediately:', value);
        })
        .catch(error => {
          console.error('Error updating brightness:', error);
        });
      }
      
      function loadBrightness() {
        fetch('/brightness')
          .then(response => response.text())
          .then(brightness => {
            const slider = document.getElementById('brightnessSlider');
            const value = document.getElementById('brightnessValue');
            if (slider) slider.value = brightness;
            if (value) value.textContent = brightness;
          });
      }
      
      function updateSpeeduinoMode() {
        const select = document.getElementById('speeduinoMode');
        const mode = select.value;
        markPendingChange('speeduinoMode', mode);
      }
      
      function loadSpeeduinoMode() {
        fetch('/speeduino-mode')
          .then(response => response.text())
          .then(mode => {
            const select = document.getElementById('speeduinoMode');
            if (select) select.value = mode;
          });
      }
      
      function loadVersionInfo() {
        fetch('/version')
          .then(response => response.json())
          .then(data => {
            const versionSpan = document.getElementById('firmwareVersion');
            const buildSpan = document.getElementById('buildInfo');
            if (versionSpan) versionSpan.textContent = 'v' + data.version;
            if (buildSpan) buildSpan.textContent = data.build + ' [' + data.hash + ']';
          })
          .catch(error => {
            console.log('Could not load version info:', error);
            const versionSpan = document.getElementById('firmwareVersion');
            const buildSpan = document.getElementById('buildInfo');
            if (versionSpan) versionSpan.textContent = 'v1.3.0';
            if (buildSpan) buildSpan.textContent = 'Development Build';
          });
      }
      
      // WiFi functions
      function scanNetworks() {
        const scanDiv = document.getElementById('networkScan');
        const listDiv = document.getElementById('networkList');
        
        scanDiv.style.display = 'block';
        listDiv.innerHTML = 'Scanning for networks...';
        
        fetch('/wifi/scan')
          .then(response => response.json())
          .then(networks => {
            if (networks.length === 0) {
              listDiv.innerHTML = 'No networks found';
              return;
            }
            
            let html = '';
            networks.forEach(network => {
              const signalStrength = network.rssi > -50 ? 'Strong' : 
                                   network.rssi > -70 ? 'Medium' : 'Weak';
              html += `<div style="padding: 8px; border: 1px solid #444; margin: 5px 0; border-radius: 4px; cursor: pointer; display: flex; justify-content: space-between; align-items: center;" onclick="selectNetwork('${network.ssid}')">`;
              html += `<span><strong>${network.ssid}</strong></span>`;
              html += `<span style="font-size: 12px; color: #999;">${signalStrength} (${network.rssi}dBm)</span>`;
              html += `</div>`;
            });
            
            listDiv.innerHTML = html;
          })
          .catch(error => {
            listDiv.innerHTML = 'Error scanning networks: ' + error;
            console.error('Network scan error:', error);
          });
      }
      
      function selectNetwork(ssid) {
        document.getElementById('wifiSsid').value = ssid;
      }
      
      function connectToRouter() {
        const ssid = document.getElementById('wifiSsid').value;
        const password = document.getElementById('wifiPassword').value;
        
        if (!ssid.trim()) {
          alert('Please enter router SSID');
          return;
        }
        
        if (!confirm(`Connect to router "${ssid}"?\n\nDevice will restart and try to connect.\nIf connection fails, it will fallback to AP mode.`)) {
          return;
        }
        
        fetch('/wifi/connect', {
          method: 'POST',
          headers: {'Content-Type': 'application/x-www-form-urlencoded'},
          body: `ssid=${encodeURIComponent(ssid)}&password=${encodeURIComponent(password)}`
        })
        .then(response => response.text())
        .then(data => {
          alert('Connection request sent. Device will restart and attempt connection.\n\nIf successful, find the new IP address in your router\'s DHCP client list.');
          setTimeout(() => location.reload(), 3000);
        })
        .catch(error => {
          alert('Error sending connection request: ' + error);
        });
      }
      
      function switchToAP() {
        if (confirm('Switch to Access Point mode?\n\nDevice will create its own WiFi network (MAZDUINO_Display) and restart.')) {
          fetch('/wifi/ap', { method: 'POST' })
            .then(response => response.text())
            .then(data => {
              alert('Switching to AP mode. Device will restart.\n\nConnect to MAZDUINO_Display network and visit 192.168.4.1');
              setTimeout(() => location.reload(), 3000);
            })
            .catch(error => {
              alert('Error switching to AP mode: ' + error);
            });
        }
      }
      
      function forgetWiFi() {
        if (confirm('Forget saved WiFi credentials?\n\nThis will clear stored router information and restart in AP mode.')) {
          fetch('/wifi/forget', { method: 'POST' })
            .then(response => response.text())
            .then(data => {
              alert('WiFi credentials cleared. Device will restart in AP mode.');
              setTimeout(() => location.reload(), 3000);
            })
            .catch(error => {
              alert('Error forgetting WiFi: ' + error);
            });
        }
      }
      
      function loadWiFiInfo() {
        fetch('/wifi/status')
          .then(response => response.json())
          .then(data => {
            document.getElementById('currentWifiMode').textContent = data.mode;
            document.getElementById('currentIP').textContent = data.ip;
            document.getElementById('clientCount').textContent = data.clients || '0';
            
            // Show station info if connected
            if (data.mode.includes('Station') && data.connected) {
              document.getElementById('wifiSsid').value = data.ssid || '';
              // Show signal strength if available
              if (data.rssi && data.rssi !== 0) {
                const signalInfo = document.getElementById('signalStrength');
                if (signalInfo) {
                  signalInfo.textContent = data.rssi + ' dBm';
                }
              }
            }
          })
          .catch(error => {
            console.error('Error loading WiFi info:', error);
            document.getElementById('currentWifiMode').textContent = 'Error loading';
            document.getElementById('currentIP').textContent = 'Error loading';
          });
      }
      
      // Navigation system
      function showSection(sectionId) {
        // Hide all sections
        const sections = document.querySelectorAll('.content-section');
        sections.forEach(section => {
          section.classList.remove('active');
        });
        
        // Remove active class from all nav items
        const navItems = document.querySelectorAll('.nav-item');
        navItems.forEach(item => {
          item.classList.remove('active');
        });
        
        // Show selected section
        const targetSection = document.getElementById(sectionId);
        if (targetSection) {
          targetSection.classList.add('active');
          
          // Auto-load WiFi info when WiFi section is opened
          if (sectionId === 'wifiSection') {
            loadWiFiInfo();
          }
        }
        
        // Add active class to clicked nav item
        const activeNavItem = document.querySelector(`[onclick="showSection('${sectionId}')"]`);
        if (activeNavItem) {
          activeNavItem.classList.add('active');
        }
        
        // Store current section in localStorage
        localStorage.setItem('activeSection', sectionId);
      }
      
      function initializeNavigation() {
        // Get stored section or default to 'statusSection'
        const activeSection = localStorage.getItem('activeSection') || 'statusSection';
        showSection(activeSection);
      }
      
      // Pending changes management
      let pendingChanges = {};
      
      function markPendingChange(key, value) {
        pendingChanges[key] = value;
        updatePendingStatus();
      }
      
      function updatePendingStatus() {
        const statusDiv = document.getElementById('pendingStatus');
        const saveBtn = document.getElementById('saveConfigBtn');
        const changeCount = Object.keys(pendingChanges).length;
        
        if (changeCount > 0) {
          statusDiv.textContent = `${changeCount} unsaved change${changeCount > 1 ? 's' : ''}`;
          statusDiv.style.color = '#FF9800';
          if (saveBtn) saveBtn.disabled = false;
        } else {
          statusDiv.textContent = 'All Saved';
          statusDiv.style.color = '#4CAF50';
          if (saveBtn) saveBtn.disabled = true;
        }
      }
      
      function saveAllConfiguration() {
        if (Object.keys(pendingChanges).length === 0) {
          alert('No changes to save');
          return;
        }
        
        // Save each pending change
        const promises = [];
        
        Object.entries(pendingChanges).forEach(([key, value]) => {
          let endpoint, body;
          
          switch(key) {
            case 'commMode':
              endpoint = '/setMode';
              body = 'mode=' + (value === 0 ? 'can' : 'serial');
              break;
            case 'canSpeed':
              endpoint = '/canspeed';
              body = 'speed=' + value;
              break;
            case 'splash':
              endpoint = '/splash';
              body = 'splash=' + value;
              break;
            case 'speeduinoMode':
              endpoint = '/speeduino-mode';
              body = 'mode=' + value;
              break;
            default:
              return;
          }
          
          promises.push(
            fetch(endpoint, {
              method: 'POST',
              headers: {'Content-Type': 'application/x-www-form-urlencoded'},
              body: body
            })
          );
        });
        
        Promise.all(promises)
          .then(responses => {
            pendingChanges = {};
            updatePendingStatus();
            alert('Configuration saved successfully!');
            
            // Restart if communication mode was changed
            if (pendingChanges.commMode !== undefined) {
              setTimeout(() => location.reload(), 3000);
            }
          })
          .catch(error => {
            console.error('Error saving configuration:', error);
            alert('Error saving configuration. Please try again.');
          });
      }
      
      function resetToDefault() {
        if (confirm('Reset all settings to default values? This will restart the device.')) {
          // Clear pending changes
          pendingChanges = {};
          updatePendingStatus();
          
          // Reset display configuration
          fetch('/resetDisplayConfig', { method: 'POST' })
            .then(() => {
              alert('Settings reset to default. Device will restart...');
              setTimeout(() => location.reload(), 3000);
            })
            .catch(error => {
              console.error('Error resetting configuration:', error);
              alert('Error resetting configuration. Please try again.');
            });
        }
      }
      
      const startTime = Date.now()/1000;
      setInterval(refreshStatus, 1000);
      
      // Load display config on page load
      window.onload = function() {
        loadDisplayConfig();
        loadCanSpeed();
        loadSplashScreen();
        loadBrightness();
        loadSpeeduinoMode();
        loadVersionInfo();
        function loadWiFiInfo() {
          fetch('/wifi/status')
            .then(response => response.json())
            .then(data => {
              // Update info section
              document.getElementById('infoWifiNetwork').textContent = data.network || 'Unknown';
              document.getElementById('infoWifiMode').textContent = data.mode || 'Unknown';
              document.getElementById('infoIPAddress').textContent = data.ip || 'Unknown';
              
              // Update WiFi Configuration section if it exists
              if (document.getElementById('currentWifiMode')) {
                document.getElementById('currentWifiMode').textContent = data.mode || 'Unknown';
              }
              if (document.getElementById('currentIP')) {
                document.getElementById('currentIP').textContent = data.ip || 'Unknown';
              }
              if (document.getElementById('clientCount')) {
                document.getElementById('clientCount').textContent = data.clients || '0';
              }
              
              // Update SSID field if in station mode
              if (data.mode.includes('Station') && data.connected && document.getElementById('wifiSsid')) {
                document.getElementById('wifiSsid').value = data.ssid || '';
              }
            })
            .catch(error => {
              console.log('WiFi info update failed:', error);
              document.getElementById('infoWifiNetwork').textContent = 'Error loading';
              document.getElementById('infoWifiMode').textContent = 'Error loading';
              document.getElementById('infoIPAddress').textContent = 'Error loading';
            });
        }
        
        // Auto-refresh WiFi status every 10 seconds
        setInterval(loadWiFiInfo, 10000);
        
        loadWiFiInfo(); // Load WiFi status
        refreshStatus(); // Initialize communication mode buttons
        updatePendingStatus(); // Initialize pending status
        initializeNavigation(); // Initialize navigation system
      };
    </script>
  </head>
  <body>
    <div class="container">
      <h1>MAZDUINO Display Control</h1>
      
      <!-- Navigation Bar -->
      <div class="nav-bar">
        <button class="nav-item" onclick="showSection('statusSection')">Status</button>
        <button class="nav-item" onclick="showSection('wifiSection')">WiFi</button>
        <button class="nav-item" onclick="showSection('brightnessSection')">Brightness</button>
        <button class="nav-item" onclick="showSection('displaySection')">Display</button>
        <button class="nav-item" onclick="showSection('debugSection')">Debug</button>
        <button class="nav-item" onclick="showSection('commSection')">Communication</button>
        <button class="nav-item" onclick="showSection('updateSection')">Update</button>
        <button class="nav-item" onclick="showSection('infoSection')">Info</button>
      </div>
      
      <div id="statusSection" class="section content-section active">
        <h2>System Status</h2>
        <div class="status" id="status">
          Status: Connected<br>
          WiFi: Active<br>
          Communication: <span id="commStatus" style="color: #4CAF50; font-weight: bold;">CAN Bus Mode</span><br>
          Debug Mode: OFF<br>
          Simulator: Mode 0<br>
          Ready for configuration
        </div>
      </div>
      
      <div id="wifiSection" class="section content-section">
        <h2>WiFi Configuration</h2>
        <div class="status" id="wifiStatus">
          <strong>Current Mode:</strong> <span id="currentWifiMode">Access Point (AP)</span><br>
          <strong>AP SSID:</strong> MAZDUINO_Display<br>
          <strong>IP Address:</strong> <span id="currentIP">192.168.4.1</span><br>
          <strong>Connected Clients:</strong> <span id="clientCount">0</span><br>
          <strong>Signal Strength:</strong> <span id="signalStrength">N/A</span>
        </div>
        
        <h3>Connect to Router</h3>
        <div class="config-item">
          <label for="wifiSsid">Router SSID:</label>
          <input type="text" id="wifiSsid" placeholder="Enter router name" style="width: 100%; padding: 10px; background: #444; border: 1px solid #666; color: white; border-radius: 4px; margin: 5px 0;">
        </div>
        <div class="config-item">
          <label for="wifiPassword">Password:</label>
          <input type="password" id="wifiPassword" placeholder="Enter router password" style="width: 100%; padding: 10px; background: #444; border: 1px solid #666; color: white; border-radius: 4px; margin: 5px 0;">
        </div>
        
        <div class="grid">
          <button class="btn" onclick="scanNetworks()">Scan Networks</button>
          <button class="btn" onclick="connectToRouter()">Connect to Router</button>
        </div>
        
        <div id="networkScan" style="display: none; margin-top: 15px;">
          <h3>Available Networks</h3>
          <div id="networkList" style="background: #2a2a2a; padding: 10px; border-radius: 5px; max-height: 200px; overflow-y: auto;">
            Scanning...
          </div>
        </div>
        
        <div class="grid" style="margin-top: 15px;">
          <button class="btn" onclick="switchToAP()">Switch to AP Mode</button>
          <button class="btn danger" onclick="forgetWiFi()">Forget WiFi</button>
        </div>
        
        <p style="font-size: 14px; opacity: 0.8; margin-top: 15px;">
          <strong>Access Point Mode:</strong> ESP32 creates its own network (MAZDUINO_Display)<br>
          <strong>Station Mode:</strong> ESP32 connects to your router (same network as laptop)<br><br>
          <strong>Station Benefits:</strong>
          <ul style="margin: 10px 0; padding-left: 20px;">
            <li>Access from anywhere on network</li>
            <li>No need to switch laptop WiFi</li>
            <li>Can access other internet services</li>
            <li>More stable connection</li>
          </ul>
          <strong>Note:</strong> If connection fails, device will fallback to AP mode automatically.
        </p>
      </div>
      
      <div id="brightnessSection" class="section content-section">
        <h2>Brightness Control</h2>
        <div style="display: flex; align-items: center; gap: 15px; margin: 15px 0;">
          <label for="brightnessSlider" style="font-weight: bold; color: #4CAF50; min-width: 120px;">Brightness:</label>
          <input type="range" id="brightnessSlider" min="0" max="255" value="100" 
                 style="flex: 1; height: 6px; background: #444; outline: none; border-radius: 3px;"
                 oninput="updateBrightness(this.value)" onchange="updateBrightness(this.value)">
          <span id="brightnessValue" style="min-width: 40px; font-weight: bold; color: #4CAF50;">100</span>
        </div>
        <div style="font-size: 12px; color: #999; margin-top: 5px;">
          Adjust display backlight brightness (0 = off, 255 = maximum)
        </div>
      </div>
      
      <div id="displaySection" class="section content-section">
        <h2>Display Configuration</h2>
        
        <!-- Panel Configuration -->
        <h3>Data Panels</h3>
        <div class="config-grid">
          <div class="config-item">
            <label>Position 1 (Left-Top):</label>
            <select id="panel0" onchange="updatePanelConfig(0)">
              <option value="disabled">Disabled</option>
              <option value="0">IAT</option>
              <option value="1">Coolant</option>
              <option value="2">AFR</option>
              <option value="3">ADV</option>
              <option value="4">Trigger</option>
              <option value="5">TPS</option>
              <option value="6">Voltage</option>
              <option value="7">MAP</option>
              <option value="8">RPM</option>
              <option value="9">FP</option>
              <option value="10">VSS</option>
            </select>
          </div>
          <div class="config-item">
            <label>Position 2 (Left-Middle):</label>
            <select id="panel1" onchange="updatePanelConfig(1)">
              <option value="disabled">Disabled</option>
              <option value="0">IAT</option>
              <option value="1">Coolant</option>
              <option value="2">AFR</option>
              <option value="3">ADV</option>
              <option value="4">Trigger</option>
              <option value="5">TPS</option>
              <option value="6">Voltage</option>
              <option value="7">MAP</option>
              <option value="8">RPM</option>
              <option value="9">FP</option>
              <option value="10">VSS</option>
            </select>
          </div>
          <div class="config-item">
            <label>Position 3 (Left-Bottom):</label>
            <select id="panel2" onchange="updatePanelConfig(2)">
              <option value="disabled">Disabled</option>
              <option value="0">IAT</option>
              <option value="1">Coolant</option>
              <option value="2">AFR</option>
              <option value="3">ADV</option>
              <option value="4">Trigger</option>
              <option value="5">TPS</option>
              <option value="6">Voltage</option>
              <option value="7">MAP</option>
              <option value="8">RPM</option>
              <option value="9">FP</option>
              <option value="10">VSS</option>
            </select>
          </div>
          <div class="config-item">
            <label>Position 4 (Right-Top):</label>
            <select id="panel3" onchange="updatePanelConfig(3)">
              <option value="disabled">Disabled</option>
              <option value="0">IAT</option>
              <option value="1">Coolant</option>
              <option value="2">AFR</option>
              <option value="3">ADV</option>
              <option value="4">Trigger</option>
              <option value="5">TPS</option>
              <option value="6">Voltage</option>
              <option value="7">MAP</option>
              <option value="8">RPM</option>
              <option value="9">FP</option>
              <option value="10">VSS</option>
            </select>
          </div>
          <div class="config-item">
            <label>Position 5 (Right-Middle):</label>
            <select id="panel4" onchange="updatePanelConfig(4)">
              <option value="disabled">Disabled</option>
              <option value="0">IAT</option>
              <option value="1">Coolant</option>
              <option value="2">AFR</option>
              <option value="3">ADV</option>
              <option value="4">Trigger</option>
              <option value="5">TPS</option>
              <option value="6">Voltage</option>
              <option value="7">MAP</option>
              <option value="8">RPM</option>
              <option value="9">FP</option>
              <option value="10">VSS</option>
            </select>
          </div>
          <div class="config-item">
            <label>Position 6 (Right-Bottom):</label>
            <select id="panel5" onchange="updatePanelConfig(5)">
              <option value="disabled">Disabled</option>
              <option value="0">IAT</option>
              <option value="1">Coolant</option>
              <option value="2">AFR</option>
              <option value="3">ADV</option>
              <option value="4">Trigger</option>
              <option value="5">TPS</option>
              <option value="6">Voltage</option>
              <option value="7">MAP</option>
              <option value="8">RPM</option>
              <option value="9">FP</option>
              <option value="10">VSS</option>
            </select>
          </div>
          <div class="config-item">
            <label>Position 7 (Bottom-Left):</label>
            <select id="panel6" onchange="updatePanelConfig(6)">
              <option value="disabled">Disabled</option>
              <option value="0">IAT</option>
              <option value="1">Coolant</option>
              <option value="2">AFR</option>
              <option value="3">ADV</option>
              <option value="4">Trigger</option>
              <option value="5">TPS</option>
              <option value="6">Voltage</option>
              <option value="7">MAP</option>
              <option value="8">RPM</option>
              <option value="9">FP</option>
              <option value="10">VSS</option>
            </select>
          </div>
          <div class="config-item">
            <label>Position 8 (Bottom-Right):</label>
            <select id="panel7" onchange="updatePanelConfig(7)">
              <option value="disabled">Disabled</option>
              <option value="0">IAT</option>
              <option value="1">Coolant</option>
              <option value="2">AFR</option>
              <option value="3">ADV</option>
              <option value="4">Trigger</option>
              <option value="5">TPS</option>
              <option value="6">Voltage</option>
              <option value="7">MAP</option>
              <option value="8">RPM</option>
              <option value="9">FP</option>
              <option value="10">VSS</option>
            </select>
          </div>
        </div>
        
        <!-- Indicator Configuration -->
        <h3>Status Indicators</h3>
        <div class="indicator-grid">
          <label><input type="checkbox" id="ind0" onchange="updateIndicatorConfig(0)"> SYNC</label>
          <label><input type="checkbox" id="ind1" onchange="updateIndicatorConfig(1)"> FAN</label>
          <label><input type="checkbox" id="ind2" onchange="updateIndicatorConfig(2)"> ASE</label>
          <label><input type="checkbox" id="ind3" onchange="updateIndicatorConfig(3)"> WUE</label>
          <label><input type="checkbox" id="ind4" onchange="updateIndicatorConfig(4)"> REV</label>
          <label><input type="checkbox" id="ind5" onchange="updateIndicatorConfig(5)"> LCH</label>
          <label><input type="checkbox" id="ind6" onchange="updateIndicatorConfig(6)"> AC</label>
          <label><input type="checkbox" id="ind7" onchange="updateIndicatorConfig(7)"> DFCO</label>
        </div>
        
        <p style="font-size: 14px; opacity: 0.8;">
          <strong>Display Configuration:</strong><br>
          <ul><li><strong>Data Panels:</strong> Choose which engine data to display in each position</li>
          <li><strong>Status Indicators:</strong> Select which status indicators to show at bottom</li>
          <li><strong>Layout:</strong> 8 data panels (4 top, 4 bottom) + indicator bar</li>
          <li><strong>Data Types:</strong> Float (AFR, Voltage), Integer (TPS, MAP, etc.), Boolean (indicators)</li></ul>
          <br>
          Configuration is saved to device memory and persists across restarts.
        </p>
      </div>
      
      <div id="debugSection" class="section content-section">
        <h2>Debug & Testing</h2>
        <div class="grid">
          <button class="btn" onclick="toggleDebug()">
            Toggle Debug Mode
          </button>
          <button class="btn" onclick="setSimulator('0')">
            Simulator OFF
          </button>
        </div>
        <div class="grid">
          <button class="btn" onclick="setSimulator('1')">
            RPM Sweep
          </button>
          <button class="btn" onclick="setSimulator('2')">
            Engine Idle
          </button>
        </div>
        <div class="grid">
          <button class="btn" onclick="setSimulator('3')">
            Driving Mode
          </button>
          <button class="btn" onclick="setSimulator('4')">
            Redline Mode
          </button>
        </div>
        <p style="font-size: 14px; opacity: 0.8;">
          <strong>Debug Mode:</strong> Shows CPU usage, FPS, and memory info at top center of display<br>
          <strong>Simulator Modes:</strong>
          <ul style="margin: 10px 0; padding-left: 20px;">
            <li><strong>OFF:</strong> Use real ECU data</li>
            <li><strong>RPM Sweep:</strong> RPM increases from 0 to 8000 continuously</li>
            <li><strong>Engine Idle:</strong> Simulates engine at idle (800 RPM)</li>
            <li><strong>Driving:</strong> Simulates normal driving conditions (2000-4000 RPM)</li>
            <li><strong>Redline:</strong> Simulates high RPM operation (6000+ RPM)</li>
          </ul>
          When simulator is active, "SIM" indicator appears on display. Use these modes to test the display without connecting to real ECU.
        </p>
      </div>
      
      <div id="commSection" class="section content-section">
        <h2>Communication Mode</h2>
        <div class="grid">
          <button id="canModeBtn" class="btn" onclick="setCommMode('can')">
            CAN Bus Mode
          </button>
          <button id="serialModeBtn" class="btn" onclick="setCommMode('serial')">
            Serial Mode
          </button>
        </div>
        
        <!-- CAN Configuration - only shown in CAN mode -->
        <div id="canConfiguration" style="display: none; margin-top: 20px; padding-top: 15px; border-top: 1px solid #444;">
          <h3>CAN Bus Configuration</h3>
          <div class="config-item">
            <label for="canSpeedSelect">CAN Speed:</label>
            <select id="canSpeedSelect" onchange="updateCanSpeed()">
              <option value="500000">500 Kbps</option>
              <option value="1000000">1 Mbps</option>
            </select>
          </div>
          <p style="font-size: 12px; opacity: 0.8; margin-top: 10px;">
            Select CAN speed according to your hardware/ECU requirements.<br>
            Changes will be saved and applied on next restart.
          </p>
        </div>
        
        <!-- Serial Configuration - only shown in Serial mode -->
        <div id="serialConfiguration" style="display: none; margin-top: 20px; padding-top: 15px; border-top: 1px solid #444;">
          <h3>Serial Communication Configuration</h3>
          <div class="config-item">
            <label for="speeduinoMode" style="font-weight: bold; color: #4CAF50;">Speeduino Data Mode:</label>
            <select id="speeduinoMode" onchange="updateSpeeduinoMode()" style="padding: 5px; background: #333; color: #fff; border: 1px solid #555; border-radius: 3px;">
              <option value="0">Mode A - Simple (75 bytes)</option>
              <option value="1">Mode N - Enhanced (119 bytes)</option>
            </select>
          </div>
          <div style="font-size: 12px; color: #999; margin-top: 10px;">
            <strong>Mode A:</strong> Compatible with Arduino Mega clones with limited capability<br>
            <strong>Mode N:</strong> Full enhanced data set for powerful controllers (recommended)
          </div>
        </div>
        
        <p style="font-size: 14px; opacity: 0.8; margin-top: 15px;">
          <strong>Communication Mode Explanation:</strong>
          <ul style="margin: 10px 0; padding-left: 20px;">
            <li><strong>CAN Bus Mode:</strong> Receives data via CAN bus (standard automotive protocol)</li>
            <li><strong>Serial Mode:</strong> Receives data via serial communication (UART)</li>
          </ul>
          The active mode is shown on the display:
          <ul style="margin: 10px 0; padding-left: 20px;">
            <li><strong>"CAN"</strong> appears in green at top-left for CAN Bus mode</li>
            <li><strong>"SER"</strong> appears in orange at top-left for Serial mode</li>
          </ul>
          <strong>WARNING:</strong> Changing communication mode will restart the device.
        </p>
      </div>
      
      <div id="updateSection" class="section content-section">
        <h2>Firmware Update</h2>
        <form method="POST" action="/update" enctype="multipart/form-data" id="uploadForm">
          <input type="file" name="firmware" class="file-input" accept=".bin" required id="firmwareFile">
          <button type="submit" class="btn danger" id="uploadBtn">Upload Firmware</button>
        </form>
        <div id="uploadProgress" style="display:none; margin-top:10px;">
          <p>Uploading firmware, please wait...</p>
          <div style="background:#444;height:20px;border-radius:10px;overflow:hidden;">
            <div id="progressBar" style="background:#4CAF50;height:100%;width:0%;transition:width 0.3s;"></div>
          </div>
        </div>
        <p style="font-size: 14px; opacity: 0.8;">
          <strong>WARNING:</strong> Only upload official MAZDUINO firmware files (.bin format).<br>
          <strong>DO NOT</strong> power off device during upload!
        </p>
        <script>
          document.getElementById('uploadForm').addEventListener('submit', function(e) {
            const fileInput = document.getElementById('firmwareFile');
            const uploadBtn = document.getElementById('uploadBtn');
            const progressDiv = document.getElementById('uploadProgress');
            const progressBar = document.getElementById('progressBar');
            
            if (fileInput.files.length === 0) {
              alert('Please select a firmware file first!');
              e.preventDefault();
              return;
            }
            
            const file = fileInput.files[0];
            if (!file.name.toLowerCase().endsWith('.bin')) {
              alert('File must be in .bin format!');
              e.preventDefault();
              return;
            }
            
            // More thorough file size validation
            if (file.size === 0) {
              alert('Error: Selected file is empty (0 bytes). Please select a valid firmware file.');
              e.preventDefault();
              return;
            }
            
            if (file.size < 100000) {  // Less than 100KB seems too small
              if (!confirm('Firmware file seems too small (' + Math.round(file.size/1024) + 'KB). A typical firmware is 1-3MB. Continue anyway?')) {
                e.preventDefault();
                return;
              }
            }
            
            if (file.size > 4000000) {  // Larger than 4MB is too big for ESP32
              alert('Firmware file is too large (' + Math.round(file.size/1024) + 'KB). Maximum supported size is 4MB.');
              e.preventDefault();
              return;
            }
            
            // Show confirmation dialog
            if (!confirm('Are you sure you want to update the firmware? The device will restart after upload.\\n\\nIMPORTANT: Ensure stable power supply during update!')) {
              e.preventDefault();
              return;
            }
            
            // Prepare system for upload
            fetch('/prepare-update', { method: 'POST' })
              .then(() => {
                console.log('System prepared for update');
              })
              .catch(err => {
                console.log('Warning: Could not prepare system for update:', err);
              });
            
            uploadBtn.disabled = true;
            uploadBtn.textContent = 'Preparing...';
            
            // Small delay to allow preparation
            setTimeout(() => {
              uploadBtn.textContent = 'Uploading...';
              startUpload();
            }, 1000);
            
            function startUpload() {
              progressDiv.style.display = 'block';
              
              const formData = new FormData();
              formData.append('firmware', file);
              
              const xhr = new XMLHttpRequest();
            
            xhr.upload.addEventListener('progress', function(e) {
              if (e.lengthComputable) {
                const percentComplete = (e.loaded / e.total) * 100;
                progressBar.style.width = percentComplete + '%';
                console.log('Upload progress: ' + Math.round(percentComplete) + '%');
              }
            });
            
            xhr.addEventListener('load', function() {
              if (xhr.status === 200) {
                progressBar.style.width = '100%';
                uploadBtn.textContent = 'Upload Complete!';
                alert('Firmware uploaded successfully! Device will restart in 3 seconds.');
                setTimeout(() => {
                  location.reload();
                }, 5000);
              } else {
                uploadBtn.disabled = false;
                uploadBtn.textContent = 'Upload Firmware';
                progressDiv.style.display = 'none';
                alert('Upload failed! Status: ' + xhr.status + '. Please try again.');
              }
            });
            
            xhr.addEventListener('error', function() {
              uploadBtn.disabled = false;
              uploadBtn.textContent = 'Upload Firmware';
              progressDiv.style.display = 'none';
              alert('Upload error occurred. Please check your connection and try again.');
            });
            
            xhr.addEventListener('timeout', function() {
              uploadBtn.disabled = false;
              uploadBtn.textContent = 'Upload Firmware';
              progressDiv.style.display = 'none';
              alert('Upload timed out. Please try again.');
            });
            
            xhr.timeout = 120000; // 2 minute timeout
            xhr.open('POST', '/update');
            xhr.send(formData);
            }
          });
        </script>
      </div>
      
      <div id="infoSection" class="section content-section">
        <h2>Information</h2>
        <p id="deviceInfo">
          <strong>WiFi Network:</strong> <span id="infoWifiNetwork">Loading...</span><br>
          <strong>WiFi Mode:</strong> <span id="infoWifiMode">Loading...</span><br>
          <strong>IP Address:</strong> <span id="infoIPAddress">Loading...</span><br>
          <strong>Version:</strong> <span id="firmwareVersion">Loading...</span><br>
          <strong>Build:</strong> <span id="buildInfo">Loading...</span>
        </p>
        <p style="font-size: 14px; opacity: 0.8;">
          <span id="powerSaveInfo">WiFi will automatically turn off after 1 minute of inactivity to save power.</span>
        </p>
        <div style="margin-top: 20px; padding-top: 15px; border-top: 1px solid #444;">
          <h3 style="color: #4CAF50; font-size: 14px; margin-bottom: 10px;">Follow Us</h3>
          <p style="font-size: 14px;">
            <strong>TikTok:</strong> <a href="https://www.tiktok.com/@mazduino" target="_blank" style="color: #4CAF50; text-decoration: none;">@mazduino</a><br>
            <strong>Website:</strong> <a href="https://www.mazduino.com" target="_blank" style="color: #4CAF50; text-decoration: none;">www.mazduino.com</a>
          </p>
        </div>
      </div>
      
      <!-- Hidden Splash Screen Section - not in navigation -->
      <div class="section splash-logo" style="display: none;">
        <h2>Splash Screen Configuration</h2>
        <div class="config-item">
          <label for="splashSelect">Splash Screen:</label>
          <select id="splashSelect" onchange="updateSplashScreen()">
            <option value="0">Mazduino</option>
            <option value="1">Mercedes</option>
            <option value="2">Hedon</option>
            <option value="3">Biies</option>
            <option value="4">Zycas</option>
            <option value="5">Spine</option>
          </select>
        </div>
        <p style="font-size: 14px; opacity: 0.8;">
          Pilih gambar yang akan ditampilkan saat startup.<br>
          Perubahan akan disimpan dan digunakan saat restart berikutnya.
        </p>
      </div>
    </div>
    
    <!-- Floating Configuration Section -->
    <div class="floating-config">
      <div>
        <h3>Global Configuration</h3>
        <div id="pendingStatus" style="color: #4CAF50; font-size: 12px;">All Saved</div>
      </div>
      <div class="config-actions">
        <button id="saveConfigBtn" class="btn primary" onclick="saveAllConfiguration()" disabled>
          Save Configuration
        </button>
        <button class="btn danger" onclick="resetToDefault()">
          Reset to Default
        </button>
      </div>
    </div>
    
  </body>
</html>
)rawliteral";

void setupWebServer()
{
  // Don't start immediately - will be called after 15 seconds in main.cpp
  Serial.println("Web server setup ready - will start after 15 seconds");
}

void startWebServer()
{
  Serial.println("Starting WiFi and Web Server...");
  
  // Check if Station mode is enabled
  bool stationMode = EEPROM.read(500) == 1;
  bool wifiConnected = false;
  
  if (stationMode) {
    // Try to connect to saved router
    String savedSSID = EEPROM.readString(400);
    String savedPassword = EEPROM.readString(450);
    
    if (savedSSID.length() > 0) {
      Serial.printf("Attempting to connect to router: %s\n", savedSSID.c_str());
      Serial.println("WiFi connection timeout: 20 seconds");
      
      WiFi.mode(WIFI_MODE_STA);
      WiFi.begin(savedSSID.c_str(), savedPassword.c_str());
      
      // Wait up to 20 seconds for connection with better feedback
      int attempts = 0;
      const int maxAttempts = 40; // 20 seconds (500ms intervals)
      
      while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
        delay(500);
        if (attempts % 4 == 0) {
          Serial.printf("Connecting... (%d/%ds)\n", attempts/2, maxAttempts/2);
        } else {
          Serial.print(".");
        }
        attempts++;
      }
      
      if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("\n✓ Connected to %s\n", savedSSID.c_str());
        Serial.printf("✓ Station IP: %s\n", WiFi.localIP().toString().c_str());
        Serial.printf("✓ Access web server at: http://%s/\n", WiFi.localIP().toString().c_str());
        Serial.printf("✓ Signal strength: %d dBm\n", WiFi.RSSI());
        wifiConnected = true;
        
        // Store successful connection info
        EEPROM.write(501, 1); // WiFi connected flag
        EEPROM.commit();
      } else {
        Serial.println("\n✗ Failed to connect to router after 20 seconds");
        Serial.println("✗ Falling back to AP mode for emergency access");
        stationMode = false;
        
        // Clear connection flag
        EEPROM.write(501, 0);
        EEPROM.commit();
      }
    } else {
      Serial.println("✗ No saved SSID found, using AP mode");
      stationMode = false;
    }
  }
  
  if (!stationMode) {
    // Start in Access Point mode
    Serial.println("Starting Access Point mode...");
    WiFi.mode(WIFI_MODE_AP);
    
    // Configure AP with better settings
    WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
    bool apStarted = WiFi.softAP(ssid, password, 1, 0, 8); // Channel 1, hidden=false, max_connections=8
    
    if (apStarted) {
      delay(2000); // Give AP time to fully start
      Serial.printf("✓ AP started: %s\n", ssid);
      Serial.printf("✓ AP IP Address: %s\n", WiFi.softAPIP().toString().c_str());
      Serial.printf("✓ Access web server at: http://%s/\n", WiFi.softAPIP().toString().c_str());
      Serial.println("✓ Connect to WiFi network 'MAZDUINO_Display' to access dashboard");
      
      // Store AP mode info
      EEPROM.write(501, 0); // WiFi not connected to router
      EEPROM.commit();
    } else {
      Serial.println("✗ Failed to start AP mode!");
    }
  }
  
  // Initialize OTA Updater
  if (!otaUpdater.initialize()) {
    Serial.println("WARNING: Failed to initialize OTA Updater");
  }

  server.on("/", HTTP_GET, handleRoot);
  server.on(
      "/update", HTTP_POST, [&]()
      {
        // Handle OTA completion response
        String message;
        int responseCode = 200;
        OTAState otaState = otaUpdater.getState();
        
        if (otaState == OTA_SUCCESS) {
          message = "Update successful! MAZDUINO Display will restart in 3 seconds...";
          Serial.println("OTA Update successful! Device will restart.");
        } else if (otaState == OTA_FAILED) {
          message = "FAILED! Error: " + otaUpdater.getError();
          Serial.println("OTA Update failed: " + otaUpdater.getError());
          responseCode = 500;
        } else {
          message = "Update was aborted";
          Serial.println("OTA Update aborted");
          responseCode = 409;
        }
        
        String response = "<!DOCTYPE html><html><head><title>Update Status</title>"
          "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
          "<style>body{font-family:Arial;background:#1a1a1a;color:#fff;text-align:center;padding:50px;}"
          ".success{color:#4CAF50;}.error{color:#f44336;}</style></head><body>"
          "<h1>Firmware Update</h1>"
          "<p class=\"" + String(otaState == OTA_SUCCESS ? "success" : "error") + "\">" + message + "</p>";
        
        if (otaState == OTA_FAILED || otaState == OTA_ABORTED) {
          response += "<p><a href=\"/\" style=\"color:#4CAF50;\">Back to Dashboard</a></p>";
        } else if (otaState == OTA_SUCCESS) {
          response += "<p>Please wait, device is restarting...</p><script>setTimeout(()=>{window.location.href='/';},5000);</script>";
        }
        
        response += "</body></html>";
        server.send(responseCode, "text/html", response);
      },
      handleUpdate);
  
  // Prepare system for update endpoint
  server.on("/prepare-update", HTTP_POST, [&]() {
    Serial.println("Preparing system for OTA update...");
    
    // Force garbage collection and free up memory
    ESP.getFreeHeap();
    
    // Reduce system load
    #if ENABLE_SIMULATOR
    setSimulatorMode(0); // Turn off simulator
    #endif
    
    // Disable debug mode to save memory
    debugMode = false;
    
    // Stop any unnecessary background tasks
    delay(100); // Give system time to cleanup
    
    Serial.printf("System preparation completed. Free heap: %u bytes\n", ESP.getFreeHeap());
    server.send(200, "text/plain", "System prepared for update");
  });
  
  // Removed duplicate WiFi status endpoint - using the one in main web server section

  server.on("/toggle", HTTP_POST, handleToggle);
  server.on("/setMode", HTTP_POST, [&]()
            {
              String mode = server.arg("mode");
              if (mode == "serial")
              {
                commMode = COMM_SERIAL;
              }
              else if (mode == "can")
              {
                commMode = COMM_CAN;
              }
              EEPROM.write(1, commMode);
              EEPROM.commit();
              server.send(200, "text/plain", "Mode updated");
              ESP.restart();
            });
  
  // Debug mode handler
  server.on("/debug", HTTP_POST, [&]()
            {
#if ENABLE_DEBUG_MODE
              debugMode = !debugMode;
              String status = debugMode ? "ON" : "OFF";
              server.send(200, "text/plain", status);
              Serial.printf("Web: Debug mode %s\n", status.c_str());
#else
              server.send(200, "text/plain", "Debug mode not enabled");
#endif
            });
  
  // Simulator handler
  server.on("/simulator", HTTP_POST, [&]()
            {
#if ENABLE_SIMULATOR
              String mode = server.arg("mode");
              int simMode = mode.toInt();
              setSimulatorMode(simMode);
              
              String modeNames[] = {"OFF", "RPM Sweep", "Engine Idle", "Driving", "Redline"};
              String modeName = (simMode >= 0 && simMode <= 4) ? modeNames[simMode] : "Unknown";
              
              server.send(200, "text/plain", modeName);
              Serial.printf("Web: Simulator set to %s (%d)\n", modeName.c_str(), simMode);
#else
              server.send(200, "text/plain", "Simulator not enabled");
#endif
            });
  
  // Status endpoint for real-time updates
  server.on("/status", HTTP_GET, [&]()
            {
              String json = "{";
              json += "\"commMode\":\"" + String(commMode == COMM_CAN ? "CAN Bus" : "Serial") + "\",";
              json += "\"debugMode\":" + String(debugMode ? "true" : "false") + ",";
#if ENABLE_SIMULATOR
              json += "\"simulatorMode\":" + String(getSimulatorMode()) + ",";
#else
              json += "\"simulatorMode\":0,";
#endif
              json += "\"uptime\":" + String(millis() / 1000) + ",";
              json += "\"freeHeap\":" + String(ESP.getFreeHeap()) + "";
              json += "}";
              server.send(200, "application/json", json);
            });
  
  // Display configuration endpoints
  server.on("/configPanel", HTTP_POST, [&]()
            {
              int position = server.arg("position").toInt();
              String dataSourceStr = server.arg("dataSource");
              
              if (position >= 0 && position < 8) {
                if (dataSourceStr == "disabled") {
                  currentDisplayConfig.panels[position].enabled = false;
                } else {
                  int dataSource = dataSourceStr.toInt();
                  if (dataSource >= 0 && dataSource < DATA_SOURCE_COUNT) {
                    currentDisplayConfig.panels[position].enabled = true;
                    currentDisplayConfig.panels[position].dataSource = dataSource;
                    currentDisplayConfig.panels[position].position = position;
                    
                    // Set appropriate data type and decimals
                    switch (dataSource) {
                      case DATA_SOURCE_AFR:
                      case DATA_SOURCE_VOLTAGE:
                        currentDisplayConfig.panels[position].dataType = DATA_TYPE_FLOAT;
                        currentDisplayConfig.panels[position].decimals = 1;
                        break;
                      case DATA_SOURCE_IAT:
                      case DATA_SOURCE_COOLANT:
                        currentDisplayConfig.panels[position].dataType = DATA_TYPE_UINT;
                        currentDisplayConfig.panels[position].decimals = 0;
                        break;
                      default:
                        currentDisplayConfig.panels[position].dataType = DATA_TYPE_INT;
                        currentDisplayConfig.panels[position].decimals = 0;
                        break;
                    }
                    
                    // Copy label from data source
                    strcpy(currentDisplayConfig.panels[position].label, getDataSourceName(dataSource));
                  }
                }
              }
              
              server.send(200, "text/plain", "Panel configured");
            });
  
  server.on("/configIndicator", HTTP_POST, [&]()
            {
              int indicator = server.arg("indicator").toInt();
              bool enabled = server.arg("enabled") == "1";
              
              if (indicator >= 0 && indicator < 8) {
                currentDisplayConfig.indicators[indicator].enabled = enabled;
                currentDisplayConfig.indicators[indicator].indicator = indicator;
                currentDisplayConfig.indicators[indicator].position = indicator;
                strcpy(currentDisplayConfig.indicators[indicator].label, getIndicatorName(indicator));
              }
              
              server.send(200, "text/plain", "Indicator configured");
            });
  
  server.on("/saveDisplayConfig", HTTP_POST, [&]()
            {
              saveDisplayConfig();
              server.send(200, "text/plain", "Configuration saved");
            });
  
  server.on("/resetDisplayConfig", HTTP_POST, [&]()
            {
              resetDisplayConfigToDefault();
              server.send(200, "text/plain", "Configuration reset");
              delay(1000);
              ESP.restart();
            });
  
  server.on("/getDisplayConfig", HTTP_GET, [&]()
            {
              String json = "{";
              json += "\"panels\":[";
              for (int i = 0; i < 8; i++) {
                if (i > 0) json += ",";
                json += "{";
                json += "\"enabled\":" + String(currentDisplayConfig.panels[i].enabled ? "true" : "false") + ",";
                json += "\"dataSource\":" + String(currentDisplayConfig.panels[i].dataSource) + ",";
                json += "\"position\":" + String(currentDisplayConfig.panels[i].position);
                json += "}";
              }
              json += "],";
              json += "\"indicators\":[";
              for (int i = 0; i < 8; i++) {
                if (i > 0) json += ",";
                json += "{";
                json += "\"enabled\":" + String(currentDisplayConfig.indicators[i].enabled ? "true" : "false") + ",";
                json += "\"indicator\":" + String(currentDisplayConfig.indicators[i].indicator) + ",";
                json += "\"position\":" + String(currentDisplayConfig.indicators[i].position);
                json += "}";
              }
              json += "]";
              json += "}";
              server.send(200, "application/json", json);
            });
  
  server.on("/canspeed", HTTP_GET, handleCanSpeed);
  server.on("/canspeed", HTTP_POST, handleCanSpeed);
  server.on("/canprotocol", HTTP_GET, handleCanProtocol);
  server.on("/canprotocol", HTTP_POST, handleCanProtocol);
  
  // Splash screen configuration handler
  server.on("/splash", HTTP_GET, [&]() {
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", getSplashScreenSelection());
    server.send(200, "text/plain", buf);
  });
  
  server.on("/splash", HTTP_POST, [&]() {
    if (server.hasArg("splash")) {
      int splash = server.arg("splash").toInt();
      if (splash >= 0 && splash <= 5) { // SPLASH_MAZDUINO, SPLASH_MERCY, SPLASH_HEDON, SPLASH_BIIES, SPLASH_ZYCAS, SPLASH_SPINE
        setSplashScreenSelection(splash);
        server.send(200, "text/plain", "OK");
        const char* splashNames[] = {"Mazduino", "Mercedes", "Hedon", "Biies", "Zycas", "Spine"};
        Serial.printf("Splash screen set to %s via webserver\n", splashNames[splash]);
      } else {
        server.send(400, "text/plain", "Invalid splash selection");
      }
    } else {
      server.send(400, "text/plain", "Missing splash param");
    }
  });

  // Brightness control handlers
  server.on("/brightness", HTTP_GET, [&]() {
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", backlightBrightness);
    server.send(200, "text/plain", buf);
  });
  
  server.on("/brightness", HTTP_POST, [&]() {
    if (server.hasArg("brightness")) {
      int brightness = server.arg("brightness").toInt();
      if (brightness >= 0 && brightness <= 255) {
        backlightBrightness = brightness;
        setBacklightBrightness(backlightBrightness);
        server.send(200, "text/plain", "OK");
        Serial.printf("Brightness set to %d via webserver\n", brightness);
      } else {
        server.send(400, "text/plain", "Invalid brightness value (0-255)");
      }
    } else {
      server.send(400, "text/plain", "Missing brightness param");
    }
  });
  
  // Speeduino data mode handlers
  server.on("/speeduino-mode", HTTP_GET, [&]() {
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", speeduinoDataMode);
    server.send(200, "text/plain", buf);
  });
  
  server.on("/speeduino-mode", HTTP_POST, [&]() {
    Serial.printf("[WEB] Speeduino mode POST request received\n");
    if (server.hasArg("mode")) {
      int mode = server.arg("mode").toInt();
      Serial.printf("[WEB] Requested mode: %d\n", mode);
      if (mode == SPEEDUINO_MODE_A || mode == SPEEDUINO_MODE_N) {
        Serial.printf("[WEB] Setting Speeduino data mode to: %s\n", 
                      (mode == SPEEDUINO_MODE_A) ? "A (Simple)" : "N (Enhanced)");
        setSpeeduinoDataMode(mode);
        server.send(200, "text/plain", "OK");
      } else {
        Serial.printf("[WEB] Invalid mode value: %d\n", mode);
        server.send(400, "text/plain", "Invalid mode value (0=A, 1=N)");
      }
    } else {
      Serial.println("[WEB] Missing mode parameter");
      server.send(400, "text/plain", "Missing mode param");
    }
  });
  
  // Version information endpoint
  server.on("/version", HTTP_GET, [&]() {
    String json = "{";
    #ifdef VERSION_STRING
      json += "\"version\":\"" + String(VERSION_STRING) + "\",";
      json += "\"build\":\"" + String(BUILD_DATE) + " " + String(BUILD_TIME) + "\",";
      json += "\"hash\":\"" + String(BUILD_HASH) + "\",";
      json += "\"branch\":\"" + String(BUILD_BRANCH) + "\"";
    #else
      json += "\"version\":\"1.3.0\",";
      json += "\"build\":\"Development Build\",";
      json += "\"hash\":\"dev\",";
      json += "\"branch\":\"unknown\"";
    #endif
    json += "}";
    server.send(200, "application/json", json);
  });
  
  // Restart endpoint
  server.on("/restart", HTTP_POST, [&]() {
    server.send(200, "text/plain", "Restarting...");
    Serial.println("[WEB] Restart requested via web interface");
    delay(1000);
    ESP.restart();
  });
  
  // Factory reset endpoint
  server.on("/factory-reset", HTTP_POST, [&]() {
    server.send(200, "text/plain", "Factory reset initiated");
    Serial.println("[WEB] Factory reset requested via web interface");
    
    // Clear all EEPROM
    for (int i = 0; i < EEPROM_SIZE; i++) {
      EEPROM.write(i, 0xFF);
    }
    EEPROM.commit();
    
    Serial.println("[WEB] EEPROM cleared, restarting...");
    delay(2000);
    ESP.restart();
  });
  
  // WiFi endpoints with CORS support for external access
  server.on("/wifi/status", HTTP_GET, [&]() {
    // Track API activity for external monitoring
    lastApiActivity = millis();
    
    // Add CORS headers for external web access
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    
    String json = "{";
    json += "\"mode\":\"";
    
    // Determine current WiFi mode and connection status
    if (WiFi.getMode() == WIFI_MODE_AP) {
      json += "Access Point (AP)";
    } else if (WiFi.getMode() == WIFI_MODE_STA) {
      if (WiFi.isConnected()) {
        json += "Station (Connected to Router)";
      } else {
        json += "Station (Disconnected)";
      }
    } else if (WiFi.getMode() == WIFI_MODE_APSTA) {
      json += "Access Point + Station";
    } else {
      json += "Off";
    }
    json += "\",";
    
    // IP addresses
    if (WiFi.getMode() == WIFI_MODE_STA && WiFi.isConnected()) {
      json += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
    } else {
      json += "\"ip\":\"" + WiFi.softAPIP().toString() + "\",";
    }
    json += "\"apip\":\"" + WiFi.softAPIP().toString() + "\",";
    
    // Client count for AP mode
    json += "\"clients\":" + String(WiFi.softAPgetStationNum()) + ",";
    
    // Station mode info
    if (WiFi.getMode() == WIFI_MODE_STA || WiFi.getMode() == WIFI_MODE_APSTA) {
      if (WiFi.isConnected()) {
        json += "\"ssid\":\"" + WiFi.SSID() + "\",";
        json += "\"rssi\":" + String(WiFi.RSSI()) + ",";
        json += "\"connected\":true,";
        json += "\"network\":\"" + WiFi.SSID() + "\"";
      } else {
        json += "\"ssid\":\"\",";
        json += "\"rssi\":0,";
        json += "\"connected\":false,";
        json += "\"network\":\"Not Connected\"";
      }
    } else {
      json += "\"ssid\":\"\",";
      json += "\"rssi\":0,";
      json += "\"connected\":false,";
      json += "\"network\":\"MAZDUINO_Display\"";
    }
    
    // Add system info for external API
    json += ",\"device\":\"MAZDUINO Display\"";
    json += ",\"version\":\"1.0\"";
    json += ",\"uptime\":" + String(millis() / 1000);
    
    json += "}";
    server.send(200, "application/json", json);
  });
  
  server.on("/wifi/scan", HTTP_GET, [&]() {
    WiFi.scanDelete(); // Clean previous scan
    int n = WiFi.scanNetworks(false, true); // async=false, show_hidden=true
    
    String json = "[";
    for (int i = 0; i < n; i++) {
      if (i > 0) json += ",";
      json += "{";
      json += "\"ssid\":\"" + WiFi.SSID(i) + "\",";
      json += "\"rssi\":" + String(WiFi.RSSI(i)) + ",";
      json += "\"encryption\":" + String(WiFi.encryptionType(i));
      json += "}";
    }
    json += "]";
    
    server.send(200, "application/json", json);
  });
  
  server.on("/wifi/connect", HTTP_POST, [&]() {
    String ssid = server.arg("ssid");
    String password = server.arg("password");
    
    if (ssid.length() == 0) {
      server.send(400, "text/plain", "SSID required");
      return;
    }
    
    server.send(200, "text/plain", "Connecting to " + ssid);
    
    // Save credentials to EEPROM (use addresses after existing config)
    EEPROM.writeString(400, ssid);
    EEPROM.writeString(450, password); 
    EEPROM.write(500, 1); // Station mode flag
    EEPROM.commit();
    
    Serial.printf("[WiFi] Connecting to router: %s\n", ssid.c_str());
    delay(1000);
    ESP.restart();
  });
  
  server.on("/wifi/ap", HTTP_POST, [&]() {
    server.send(200, "text/plain", "Switching to AP mode");
    
    // Clear station mode flag
    EEPROM.write(500, 0);
    EEPROM.commit();
    
    Serial.println("[WiFi] Switching to AP mode");
    delay(1000);
    ESP.restart();
  });
  
  server.on("/wifi/forget", HTTP_POST, [&]() {
    server.send(200, "text/plain", "WiFi credentials forgotten");
    
    // Clear saved credentials and station mode flag
    EEPROM.writeString(400, "");
    EEPROM.writeString(450, "");
    EEPROM.write(500, 0);
    EEPROM.commit();
    
    Serial.println("[WiFi] Credentials forgotten, switching to AP mode");
    delay(1000);
    ESP.restart();
  });

  // External API endpoints for mazduino.com integration
  server.on("/api/device/info", HTTP_GET, [&]() {
    // Track API activity
    lastApiActivity = millis();
    
    // Add CORS headers
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    
    String json = "{";
    json += "\"device\":\"MAZDUINO Display\",";
    json += "\"version\":\"1.0\",";
    json += "\"uptime\":" + String(millis() / 1000) + ",";
    json += "\"free_heap\":" + String(ESP.getFreeHeap()) + ",";
    json += "\"chip_id\":\"" + String((uint32_t)ESP.getEfuseMac(), HEX) + "\",";
    json += "\"flash_size\":" + String(ESP.getFlashChipSize()) + ",";
    json += "\"cpu_freq\":" + String(ESP.getCpuFreqMHz()) + ",";
    json += "\"sdk_version\":\"" + String(ESP.getSdkVersion()) + "\"";
    json += "}";
    
    Serial.println("[API] Device info requested");
    server.send(200, "application/json", json);
  });

  server.on("/api/can/data", HTTP_GET, [&]() {
    // Track API activity
    lastApiActivity = millis();
    
    // Add CORS headers
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    
    // Include basic CAN data for external monitoring
    // Use the actual global variables defined in DataTypes.h/GlobalVariables.cpp
    
    String json = "{";
    json += "\"rpm\":" + String(rpm) + ",";
    json += "\"coolant\":" + String(clt) + ",";
    json += "\"iat\":" + String(iat) + ",";
    json += "\"tps\":" + String(tps) + ",";
    json += "\"map\":" + String(mapData) + ",";
    json += "\"afr\":" + String(afrConv, 2) + ",";
    json += "\"advance\":" + String(adv) + ",";
    json += "\"trigger\":" + String(triggerError) + ",";
    json += "\"voltage\":" + String(bat, 2) + ",";
    json += "\"fuel_pressure\":" + String(fp) + ",";
    json += "\"vss\":" + String(vss) + ",";
    json += "\"timestamp\":" + String(millis()) + "";
    json += "}";
    
    server.send(200, "application/json", json);
  });

  // CORS preflight handler
  server.on("/api/device/info", HTTP_OPTIONS, [&]() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    server.send(200, "text/plain", "");
  });

  server.on("/api/can/data", HTTP_OPTIONS, [&]() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    server.send(200, "text/plain", "");
  });

  server.on("/wifi/status", HTTP_OPTIONS, [&]() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    server.send(200, "text/plain", "");
  });

  server.begin();
  wifiActive = true;
  Serial.println("Web server active.");
  esp_wifi_set_max_tx_power(78);
}

void stopWebServer()
{
  if (wifiActive) {
    Serial.println("Stopping WiFi and Web Server to save power...");
    server.stop();
    WiFi.mode(WIFI_OFF);
    
    // Also disable Bluetooth to save power
    btStop();
    
    wifiActive = false;
    Serial.println("WiFi and Bluetooth disabled for power saving");
  }
}

void restartWebServer()
{
  if (!wifiActive) {
    Serial.println("Restarting WiFi and Web Server...");
    startWebServer();
  }
}

void handleRoot()
{
  server.send(200, "text/html", uploadPage);
}

void handleUpdate()
{
  // Use the new OTA updater with FreeRTOS task
  // Handle file upload for OTA update
  server.on("/update", HTTP_POST, [&]() {
    // This is the response after upload completes
    String message;
    int responseCode = 200;
    OTAState otaState = otaUpdater.getState();
    
    if (otaState == OTA_SUCCESS) {
      message = "Update successful! MAZDUINO Display will restart in 3 seconds...";
      Serial.println("OTA Update successful! Device will restart.");
    } else if (otaState == OTA_FAILED) {
      message = "FAILED! Error: " + otaUpdater.getError();
      Serial.println("OTA Update failed: " + otaUpdater.getError());
      responseCode = 500;
    } else {
      message = "Update completed with unknown status";
      responseCode = 500;
    }
    
    server.send(responseCode, "text/html", 
      "<html><body><h2>OTA Update</h2><p>" + message + "</p>"
      "<br><a href='/'>Return to Dashboard</a></body></html>");
  }, [&]() {
    // This handles the actual file upload data
    HTTPUpload& upload = server.upload();
    
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("OTA Update Start: %s\n", upload.filename.c_str());
      
      if (!otaUpdater.startOTA(upload.totalSize)) {
        Serial.println("Failed to start OTA update");
        return;
      }
    } 
    else if (upload.status == UPLOAD_FILE_WRITE) {
      if (!otaUpdater.writeData(upload.buf, upload.currentSize)) {
        Serial.println("Failed to write OTA data");
        return;
      }
    } 
    else if (upload.status == UPLOAD_FILE_END) {
      Serial.printf("OTA Update End: %u bytes\n", upload.totalSize);
      
      if (!otaUpdater.finishOTA()) {
        Serial.println("Failed to finish OTA update");
        return;
      }
    } 
    else if (upload.status == UPLOAD_FILE_ABORTED) {
      Serial.println("OTA Update Aborted");
      otaUpdater.abortOTA();
    }
  });
}

void handleToggle()
{
  if (server.method() == HTTP_POST)
  {
    bool toggleState = EEPROM.read(0) || false;
    String body = server.arg("plain");
    if (body == "on")
    {
      toggleState = 1;
      Serial.println("Toggle: ON");
    }
    else if (body == "off")
    {
      toggleState = 0;
      Serial.println("Toggle: OFF");
    }
    EEPROM.write(0, toggleState);
    EEPROM.commit();

    server.send(200, "text/plain", "OK");
    delay(1000);
    ESP.restart();
  }
}

void handleCanSpeed() {
  if (server.method() == HTTP_GET) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%u", getCanSpeed());
    server.send(200, "text/plain", buf);
  } else if (server.method() == HTTP_POST) {
    if (server.hasArg("speed")) {
      uint32_t speed = server.arg("speed").toInt();
      if (speed == 500000 || speed == 1000000) {
        setCanSpeed(speed);
        server.send(200, "text/plain", "OK");
        Serial.printf("CAN speed set to %u bps via webserver\n", speed);
      } else {
        server.send(400, "text/plain", "Invalid speed");
      }
    } else {
      server.send(400, "text/plain", "Missing speed param");
    }
  } else {
    server.send(405, "text/plain", "Method Not Allowed");
  }
}

void handleCanProtocol() {
  if (server.method() == HTTP_GET) {
    uint8_t protocol = getCanProtocol();
    const char* protocolName = (protocol == CAN_PROTOCOL_HALTECH) ? "haltech" : "rusefi";
    server.send(200, "text/plain", protocolName);
  } else if (server.method() == HTTP_POST) {
    if (server.hasArg("protocol")) {
      String protocolStr = server.arg("protocol");
      protocolStr.toLowerCase();
      if (protocolStr == "haltech") {
        setCanProtocol(CAN_PROTOCOL_HALTECH);
        server.send(200, "text/plain", "OK");
        Serial.println("CAN protocol set to Haltech via webserver");
      } else if (protocolStr == "rusefi") {
        setCanProtocol(CAN_PROTOCOL_RUSEFI);
        server.send(200, "text/plain", "OK");
        Serial.println("CAN protocol set to RusEFI via webserver");
      } else {
        server.send(400, "text/plain", "Invalid protocol (use: haltech or rusefi)");
      }
    } else {
      server.send(400, "text/plain", "Missing protocol param");
    }
  } else {
    server.send(405, "text/plain", "Method Not Allowed");
  }
}

void handleWebServerClients()
{
  static uint32_t lastClientCheck = 0;
  static uint32_t lastClientConnectedTime = 0;
  static uint32_t lastApiActivity = 0; // Track last API call
  static uint32_t webServerStartTime = millis(); // Track when web server started
  static bool hasBeenConnected = false;
  static bool apiActiveRecently = false;
  
  // Check client status every 1 second instead of every loop for better performance
  if (millis() - lastClientCheck >= 1000)
  {
    lastClientCheck = millis();
    int clientCount = WiFi.softAPgetStationNum();

    if (clientCount > 0)
    {
      clientConnected = true;
      lastClientConnectedTime = millis();
      hasBeenConnected = true;
      
      // Debug print every 30 seconds when clients are connected (reduced frequency)
      static uint32_t lastDebugPrint = 0;
      if (millis() - lastDebugPrint >= 30000) {
        Serial.printf("WiFi clients connected: %d\n", clientCount);
        lastDebugPrint = millis();
      }
    }
    else
    {
      clientConnected = false;
      
      // Check for recent API activity (within last 30 seconds)
      bool apiActiveRecently = (millis() - lastApiActivity < 30000);
      
      // Don't turn off WiFi if API was accessed recently
      if (apiActiveRecently) {
        lastClientConnectedTime = millis(); // Reset timeout if API is active
        Serial.println("[WiFi] API activity detected - keeping WiFi active");
      }
      
      // Turn off WiFi and Bluetooth after 60 seconds (1 minute) of no connections AND no API activity
      if (hasBeenConnected && wifiActive && 
          (millis() - lastClientConnectedTime > 60000) && !apiActiveRecently)
      {
        Serial.println("No clients connected and no API activity for 1 minute - shutting down WiFi/Bluetooth");
        stopWebServer();
        return;
      }
      
      // Also turn off if no one has ever connected after 5 minutes from web server start AND no API activity
      if (!hasBeenConnected && wifiActive && 
          (millis() - webServerStartTime > 300000) && !apiActiveRecently)
      {
        Serial.println("No clients ever connected and no API activity after 5 minutes - shutting down WiFi/Bluetooth");
        stopWebServer();
        return;
      }
    }
  }

  // Handle client requests only if WiFi is active
  if (wifiActive)
  {
    server.handleClient();
  }
}
