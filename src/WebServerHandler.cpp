#include "WebServerHandler.h"
#include "Config.h"
#include "DataTypes.h"
#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>
#include <EEPROM.h>
#include <esp_wifi.h>

// IP configuration
IPAddress ip(192, 168, 1, 80);
IPAddress netmask(255, 255, 255, 0);

// OTA upload page HTML
const char *uploadPage PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
  <head>
    <title>MAZDUINO Display OTA Update</title>
    <style>
      body {
        display: flex;
        justify-content: center;
        align-items: center;
        flex-direction: column;
        height: 100vh;
        background-color: black;
        color: white;
      }
      .btn {
        padding: 10px 20px;
        font-size: 16px;
        background-color: #007BFF;
        color: white;
        border: none;
        border-radius: 5px;
        cursor: pointer;
      }
      .toggle-btn.off {
        background-color: #FF0000;
      }
    </style>
    <script>
      function toggleState(button) {
        if (button.classList.contains('off')) {
          button.classList.remove('off');
          button.textContent = "Display 2";
          // Kirim status ON ke server
          fetch('/toggle', { method: 'POST', body: 'on' });
        } else {
          button.classList.add('off');
          button.textContent = "Display 1";
          // Kirim status OFF ke server
          fetch('/toggle', { method: 'POST', body: 'off' });
        }
      }
    </script>
  </head>
  <body>
    <h1>MAZDUINO Display OTA Update</h1>
    <form method="POST" action="/update" enctype="multipart/form-data">
      <input type="file" name="firmware">
      <input type="submit" class="btn" value="Upload">
    </form>
    <hr>
    <h2>Display Control</h2>
    <p>Sabar ya nanti diupdate :) </p>
    <!-- <button class="btn toggle-btn" onclick="toggleState(this)">Display 1</button> -->

  </body>
</html>
)rawliteral";

void setupWebServer()
{
  WiFi.mode(WIFI_MODE_AP);
  WiFi.softAPConfig(ip, ip, netmask);
  WiFi.softAP(ssid, password);

  server.on("/", HTTP_GET, handleRoot);
  server.on(
      "/update", HTTP_POST, [&]()
      {
      server.send(200, "text/plain", (Update.hasError()) ? "Gagal update!" : "Update berhasil! MAZDUINO Display akan restart.");
      delay(1000);
      ESP.restart(); },
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
  server.begin();
  Serial.println("Web server aktif.");
  esp_wifi_set_max_tx_power(78);
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
    Serial.printf("Memulai update: %s\n", upload.filename.c_str());
    if (!Update.begin(UPDATE_SIZE_UNKNOWN))
    {
      Update.printError(Serial);
    }
  }
  else if (upload.status == UPLOAD_FILE_WRITE)
  {
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
    {
      Update.printError(Serial);
    }
  }
  else if (upload.status == UPLOAD_FILE_END)
  {
    if (Update.end(true))
    {
      // Update successful
    }
    else
    {
      Update.printError(Serial);
    }
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

void handleWebServerClients()
{
  static uint32_t lastClientCheck = 0;
  
  if (millis() - lastClientCheck >= 1000)
  {
    lastClientCheck = millis();
    int clientCount = WiFi.softAPgetStationNum();

    if (clientCount > 0)
    {
      clientConnected = true;
      lastClientCheck = millis();
    }
    else if (millis() - lastClientCheckTimeout > wifiTimeout)
    {
      clientConnected = false;
      lastClientCheckTimeout = millis();
    }

    // Turn off WiFi if RPM > 100 or no devices connected for 30 seconds
    if ((rpm > 100 || !clientConnected) && wifiActive)
    {
      WiFi.mode(WIFI_OFF);
      server.stop();
      wifiActive = false;
    }
    // Turn on WiFi if RPM ≤ 100 and devices are connected
    else if (rpm <= 100 && clientConnected && !wifiActive)
    {
      WiFi.mode(WIFI_AP);
      WiFi.softAPConfig(ip, ip, netmask);
      WiFi.softAP(ssid, password);
      server.begin();
      wifiActive = true;
      server.handleClient();
    }
  }

  if (rpm <= 100 && wifiActive)
  {
    server.handleClient();
  }
}
