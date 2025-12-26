#include "WebServerHandler.h"
#include "Config.h"
#include "DataTypes.h"
#include "DisplayConfig.h"
#include "SplashScreen.h"
#include "BacklightControl.h"
#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>
#include <EEPROM.h>
#include <esp_wifi.h>
#include <esp_bt.h>
#include <esp_bt_main.h>
#if ENABLE_SIMULATOR
#include "Simulator.h"
#endif

// IP configuration - Simple approach, let ESP32 use default IP
// Default AP IP is usually 192.168.4.1

// Global variable for OTA update size  
static size_t otaUpdateSize = 0;

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
        if (confirm('This will restart the device. Continue?')) {
          fetch('/setMode', { 
            method: 'POST', 
            headers: {'Content-Type': 'application/x-www-form-urlencoded'},
            body: 'mode=' + mode 
          });
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
            document.getElementById('status').innerHTML = 
              'Status: Connected<br>' +
              'WiFi: Active<br>' +
              'Communication: <span style="color: ' + commColor + '; font-weight: bold;">' + data.commMode + ' Mode</span><br>' +
              'Debug Mode: ' + (data.debugMode ? 'ON' : 'OFF') + '<br>' +
              'Simulator: Mode ' + data.simulatorMode + '<br>' +
              'Uptime: ' + uptime + ' seconds<br>' +
              'Free Memory: ' + Math.round(data.freeHeap / 1024) + 'KB';
            
            // Show/hide CAN configuration based on communication mode
            const canConfig = document.getElementById('canConfiguration');
            if (canConfig) {
              canConfig.style.display = data.commMode === 'CAN Bus' ? 'block' : 'none';
            }
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
        fetch('/canspeed', {
          method: 'POST',
          headers: {'Content-Type': 'application/x-www-form-urlencoded'},
          body: 'speed=' + speed
        })
        .then(response => response.text())
        .then(data => {
          alert('CAN speed updated: ' + speed + ' bps');
        });
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
        fetch('/splash', {
          method: 'POST',
          headers: {'Content-Type': 'application/x-www-form-urlencoded'},
          body: 'splash=' + splash
        })
        .then(response => response.text())
        .then(data => {
          let splashName;
          switch(splash) {
            case '0': splashName = 'Mazduino'; break;
            case '1': splashName = 'Mercedes'; break;
            case '2': splashName = 'Hedon'; break;
            case '3': splashName = 'Biies'; break;
            case '4': splashName = 'Zycas'; break;
            case '5': splashName = 'Spine'; break;
            default: splashName = 'Unknown'; break;
          }
          alert('Splash screen updated: ' + splashName);
        });
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
        fetch('/brightness', {
          method: 'POST',
          headers: {'Content-Type': 'application/x-www-form-urlencoded'},
          body: 'brightness=' + value
        })
        .then(response => response.text())
        .then(data => {
          document.getElementById('brightnessValue').textContent = value;
          if (data !== 'OK') {
            alert('Error setting brightness: ' + data);
          }
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
      
      const startTime = Date.now()/1000;
      setInterval(refreshStatus, 1000);
      
      // Load display config on page load
      window.onload = function() {
        loadDisplayConfig();
        loadCanSpeed();
        loadSplashScreen();
        loadBrightness();
      };
    </script>
  </head>
  <body>
    <div class="container">
      <h1>MAZDUINO Display Control</h1>
      
      <div class="section">
        <h2>System Status</h2>
        <div class="status" id="status">
          Status: Connected<br>
          WiFi: Active<br>
          Communication: <span id="commStatus" style="color: #4CAF50; font-weight: bold;">CAN Bus Mode</span><br>
          Ready for configuration
        </div>
      </div>
      
      <div class="section">
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
      
      <div class="section">
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
        
        <div class="config-controls">
          <button class="btn" onclick="saveDisplayConfig()">Save Configuration</button>
          <button class="btn danger" onclick="resetDisplayConfig()">Reset to Default</button>
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
      
      <div class="section">
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
      
      <div class="section">
        <h2>Communication Mode</h2>
        <div class="grid">
          <button class="btn" onclick="setCommMode('can')">
            CAN Bus Mode
          </button>
          <button class="btn" onclick="setCommMode('serial')">
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
      
      <div class="section">
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
            
            if (file.size < 100000) {  // Less than 100KB seems too small
              if (!confirm('Firmware file seems too small (' + Math.round(file.size/1024) + 'KB). Continue anyway?')) {
                e.preventDefault();
                return;
              }
            }
            
            uploadBtn.disabled = true;
            uploadBtn.textContent = 'Uploading...';
            progressDiv.style.display = 'block';
            
            // Simulate progress (since we can't get real progress easily)
            let progress = 0;
            const progressBar = document.getElementById('progressBar');
            const interval = setInterval(() => {
              progress += Math.random() * 15;
              if (progress > 90) progress = 90;
              progressBar.style.width = progress + '%';
            }, 500);
            
            // Stop simulation after form submission
            setTimeout(() => clearInterval(interval), 1000);
          });
        </script>
      </div>
      
      <div class="section">
        <h2>Information</h2>
        <p>
          <strong>WiFi Network:</strong> MAZDUINO_Display<br>
          <strong>IP Address:</strong> 192.168.4.1<br>
          <strong>Version:</strong> 1.2
        </p>
        <p style="font-size: 14px; opacity: 0.8;">
          WiFi will automatically turn off after 1 minute of inactivity to save power.
        </p>
        <div style="margin-top: 20px; padding-top: 15px; border-top: 1px solid #444;">
          <h3 style="color: #4CAF50; font-size: 14px; margin-bottom: 10px;">Follow Us</h3>
          <p style="font-size: 14px;">
            <strong>TikTok:</strong> <a href="https://www.tiktok.com/@mazduino" target="_blank" style="color: #4CAF50; text-decoration: none;">@mazduino</a><br>
            <strong>Website:</strong> <a href="https://www.mazduino.com" target="_blank" style="color: #4CAF50; text-decoration: none;">www.mazduino.com</a>
          </p>
        </div>
      </div>
      
      <div class="section splash-logo">
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
  
  WiFi.mode(WIFI_MODE_AP);
  // Use default IP configuration (192.168.4.1)
  WiFi.softAP(ssid, password);
  
  // Wait for AP to be ready
  delay(1000);

  server.on("/", HTTP_GET, handleRoot);
  server.on(
      "/update", HTTP_POST, [&]()
      {
        String message;
        if (Update.hasError()) {
          message = "FAILED! Error: " + String(Update.getError());
          Serial.println("OTA Update failed with error: " + String(Update.getError()));
        } else {
          message = "Update successful! MAZDUINO Display will restart in 3 seconds...";
          Serial.println("OTA Update successful! Restarting...");
        }
        
        String response = "<!DOCTYPE html><html><head><title>Update Status</title>"
          "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
          "<style>body{font-family:Arial;background:#1a1a1a;color:#fff;text-align:center;padding:50px;}"
          ".success{color:#4CAF50;}.error{color:#f44336;}</style></head><body>"
          "<h1>Firmware Update</h1>"
          "<p class=\"" + String(Update.hasError() ? "error" : "success") + "\">" + message + "</p>";
        
        if (Update.hasError()) {
          response += "<p><a href=\"/\" style=\"color:#4CAF50;\">Back to Dashboard</a></p>";
        } else {
          response += "<p>Please wait, device is restarting...</p><script>setTimeout(()=>{window.location.href='/';},5000);</script>";
        }
        
        response += "</body></html>";
        server.send(200, "text/html", response);
          
        if (!Update.hasError()) {
          delay(1000);
          ESP.restart();
        }
      },
      handleUpdate);
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
  
  server.begin();
  wifiActive = true;
  Serial.println("Web server active.");
  Serial.printf("WiFi AP: %s\n", ssid);
  
  Serial.printf("AP IP Address: %s\n", WiFi.softAPIP().toString().c_str());
  Serial.printf("Access web server at: http://%s/\n", WiFi.softAPIP().toString().c_str());
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
  HTTPUpload &upload = server.upload();

  if (upload.status == UPLOAD_FILE_START)
  {
    Serial.printf("Starting update: %s\n", upload.filename.c_str());
    Serial.printf("File size: %u bytes\n", upload.totalSize);
    
    // Calculate available space for OTA update
    otaUpdateSize = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
    
    if (!Update.begin(otaUpdateSize))
    {
      Serial.println("Update.begin() failed!");
      Update.printError(Serial);
      return;
    }
    Serial.printf("Update started with size: %u bytes\n", otaUpdateSize);
  }
  else if (upload.status == UPLOAD_FILE_WRITE)
  {
    size_t written = Update.write(upload.buf, upload.currentSize);
    if (written != upload.currentSize)
    {
      Serial.printf("Update.write() failed! Written: %u, Expected: %u\n", written, upload.currentSize);
      Update.printError(Serial);
      return;
    }
    
    // Show progress every 64KB
    static size_t lastProgress = 0;
    if (upload.totalSize > 0 && (upload.totalSize - lastProgress) >= 65536) {
      Serial.printf("Update progress: %u/%u bytes (%.1f%%)\n", 
                   upload.totalSize, otaUpdateSize, 
                   (float)upload.totalSize / otaUpdateSize * 100.0);
      lastProgress = upload.totalSize;
    }
  }
  else if (upload.status == UPLOAD_FILE_END)
  {
    Serial.printf("Update completed. Total: %u bytes\n", upload.totalSize);
    
    if (Update.end(true))
    {
      Serial.println("Update successful! Device will restart...");
    }
    else
    {
      Serial.println("Update failed!");
      Update.printError(Serial);
    }
  }
  else if (upload.status == UPLOAD_FILE_ABORTED)
  {
    Serial.println("Update aborted!");
    Update.end();
  }
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

void handleWebServerClients()
{
  static uint32_t lastClientCheck = 0;
  static uint32_t lastClientConnectedTime = 0;
  static uint32_t webServerStartTime = millis(); // Track when web server started
  static bool hasBeenConnected = false;
  
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
      
      // Turn off WiFi and Bluetooth after 60 seconds (1 minute) of no connections
      if (hasBeenConnected && wifiActive && 
          (millis() - lastClientConnectedTime > 60000))
      {
        Serial.println("No clients connected for 1 minute - shutting down WiFi/Bluetooth");
        stopWebServer();
        return;
      }
      
      // Also turn off if no one has ever connected after 5 minutes from web server start
      if (!hasBeenConnected && wifiActive && 
          (millis() - webServerStartTime > 300000))
      {
        Serial.println("No clients ever connected after 5 minutes - shutting down WiFi/Bluetooth");
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
