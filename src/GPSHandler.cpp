#include "GPSHandler.h"
#include "Config.h"
#include "DataTypes.h"
#include <string.h>

// Global GPS handler instance
GPSHandler gpsHandler;

GPSHandler::GPSHandler() : enabled(false), bufferIndex(0) {
    gpsSerial = nullptr;
    memset(nmeaBuffer, 0, sizeof(nmeaBuffer));
}

void GPSHandler::begin() {
    Serial.println("[GPS] Starting GPS initialization...");
    
    // Initialize GPS serial communication on Serial2
    gpsSerial = &Serial2;
    gpsSerial->begin(GPS_SERIAL_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
    
    Serial.printf("[GPS] Serial2 configured: RX=%d, TX=%d, Baud=%d\n", GPS_RX_PIN, GPS_TX_PIN, GPS_SERIAL_BAUD);
    
    // Initialize GPS variables
    gpsEnabled = GPS_ENABLE_BY_DEFAULT;
    gpsDataValid = false;
    gpsSpeed = 0.0;
    gpsLatitude = 0.0;
    gpsLongitude = 0.0;
    gpsHeading = 0.0;
    gpsAltitude = 0.0;
    gpsNumSats = 0;
    gpsLastUpdate = 0;
    
    enabled = gpsEnabled;
    
    Serial.printf("[GPS] GT-U7 initialized on Serial2 (RX:%d, TX:%d) at %d baud\n", 
                  GPS_RX_PIN, GPS_TX_PIN, GPS_SERIAL_BAUD);
    Serial.printf("[GPS] GPS enabled by default: %s\n", gpsEnabled ? "YES" : "NO");
    Serial.println("[GPS] Waiting for NMEA data from GT-U7 module...");
}

void GPSHandler::update() {
    if (!enabled || !gpsSerial) {
        static uint32_t lastDisabledMsg = 0;
        if (millis() - lastDisabledMsg > 10000) {
            Serial.printf("[GPS] Update skipped - enabled:%s, serial:%s\n", 
                         enabled ? "YES" : "NO", gpsSerial ? "OK" : "NULL");
            lastDisabledMsg = millis();
        }
        return;
    }
    
    static uint32_t lastUpdate = 0;
    uint32_t currentTime = millis();
    
    // Check for GPS timeout - only mark invalid if we had valid data before
    if (gpsDataValid && currentTime - gpsLastUpdate > GPS_TIMEOUT_MS) {
        static uint32_t lastTimeoutMsg = 0;
        if (currentTime - lastTimeoutMsg > 10000) {
            Serial.println("[GPS] Data timeout - GPS connection lost");
            lastTimeoutMsg = currentTime;
        }
        gpsDataValid = false;
        gpsSpeed = 0.0; // Reset speed on timeout
    }
    
    // Read available GPS data
    int bytesRead = 0;
    while (gpsSerial->available()) {
        bytesRead++;
        char c = gpsSerial->read();
        
        if (c == '$') {
            // Start of new NMEA sentence
            bufferIndex = 0;
            nmeaBuffer[bufferIndex++] = c;
        } else if (c == '\n' || c == '\r') {
            // End of NMEA sentence
            if (bufferIndex > 0) {
                nmeaBuffer[bufferIndex] = '\0';
                
                // Parse the complete NMEA sentence
                if (currentTime - lastUpdate >= GPS_UPDATE_INTERVAL_MS) {
                    parseNMEA(nmeaBuffer);
                    lastUpdate = currentTime;
                }
                
                bufferIndex = 0;
            }
        } else if (bufferIndex < sizeof(nmeaBuffer) - 1) {
            // Add character to buffer
            nmeaBuffer[bufferIndex++] = c;
        } else {
            // Buffer overflow protection
            Serial.println("[GPS] Warning: NMEA buffer overflow!");
            bufferIndex = 0;
        }
    }
    
    // Simplified byte processing notification
    static uint32_t lastDataReceived = 0;
    if (bytesRead > 0) {
        lastDataReceived = currentTime;
    }
}

void GPSHandler::enableGPS(bool enable) {
    enabled = enable;
    gpsEnabled = enable;
    
    Serial.printf("[GPS] GPS state changed to: %s\n", enable ? "ENABLED" : "DISABLED");
    
    if (!enable) {
        gpsDataValid = false;
        Serial.println("[GPS] GPS disabled - clearing all GPS data");
    } else {
        Serial.println("[GPS] GPS enabled - starting data collection");
        Serial.printf("[GPS] Listening on Serial2: RX=%d, TX=%d\n", GPS_RX_PIN, GPS_TX_PIN);
    }
}

bool GPSHandler::isGPSEnabled() {
    return enabled;
}

void GPSHandler::parseNMEA(const char* sentence) {
    if (strncmp(sentence, "$GPRMC", 6) == 0) {
        parseGPRMC(sentence);
    } else if (strncmp(sentence, "$GPGGA", 6) == 0) {
        parseGPGGA(sentence);
    } else if (strncmp(sentence, "$GPVTG", 6) == 0) {
        parseGPVTG(sentence);
    } else if (strncmp(sentence, "$GPGSA", 6) == 0) {
        parseGPGSA(sentence);
    } else if (strncmp(sentence, "$GPGSV", 6) == 0) {
        parseGPGSV(sentence);
    }
}

void GPSHandler::parseGPRMC(const char* sentence) {
    // $GPRMC,time,status,lat,latdir,lon,londir,speed,course,date,magvar,checksum
    // Example: $GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A
    
    char field[32];
    char status;
    
    // Field 2: Status (A = valid, V = invalid)
    extractField(sentence, 2, field, sizeof(field));
    status = field[0];
    
    // Always update timestamp even for invalid data to prevent timeout
    gpsLastUpdate = millis();
    
    if (status != 'A') {
        // GPS fix lost, but don't immediately invalidate - allow some grace time
        static uint32_t lastInvalidTime = 0;
        static uint32_t lastInvalidMsg = 0;
        
        if (lastInvalidTime == 0) {
            lastInvalidTime = millis(); // Mark when we first lost fix
        }
        
        // Only invalidate after 10 seconds of no fix
        if (millis() - lastInvalidTime > 10000) {
            if (gpsDataValid && millis() - lastInvalidMsg > 5000) {
                Serial.println("[GPS] No GPS fix for 10+ seconds - marking invalid");
                lastInvalidMsg = millis();
            }
            gpsDataValid = false;
            gpsSpeed = 0.0;
        }
        return;
    } else {
        // GPS fix restored
        static uint32_t lastValidMsg = 0;
        if (!gpsDataValid && millis() - lastValidMsg > 5000) {
            Serial.println("[GPS] GPS fix acquired!");
            lastValidMsg = millis();
        }
        // Reset invalid timer since we have valid fix
        static uint32_t lastInvalidTime = 0;
        lastInvalidTime = 0;
    }
    
    // Field 3: Latitude
    extractField(sentence, 3, field, sizeof(field));
    double lat = parseCoordinate(field, 'N');
    extractField(sentence, 4, field, sizeof(field)); // Latitude direction
    if (field[0] == 'S') lat = -lat;
    
    // Field 5: Longitude  
    extractField(sentence, 5, field, sizeof(field));
    double lon = parseCoordinate(field, 'E');
    extractField(sentence, 6, field, sizeof(field)); // Longitude direction
    if (field[0] == 'W') lon = -lon;
    
    // Field 7: Speed in knots
    extractField(sentence, 7, field, sizeof(field));
    float speedKnots = parseFloat(field);
    
    // Field 8: Course/heading
    extractField(sentence, 8, field, sizeof(field));
    float course = parseFloat(field);
    
    // Update GPS variables
    gpsLatitude = lat;
    gpsLongitude = lon;
    gpsSpeed = speedKnots * 1.852; // Convert knots to km/h
    gpsHeading = course;
    gpsDataValid = true;
    // gpsLastUpdate already set above
    
    // Essential GPS status output
    static uint32_t lastRMCDebug = 0;
    if (millis() - lastRMCDebug > 5000) {
        Serial.printf("[GPS] Speed: %.1f km/h | Status: %s\n", 
                      gpsSpeed, gpsDataValid ? "CONNECTED" : "NO FIX");
        lastRMCDebug = millis();
    }
    
#if ENABLE_DEBUG_MODE
    if (debugMode) {
        Serial.printf("[GPS] GPRMC - Lat: %.6f, Lon: %.6f, Speed: %.1f km/h, Heading: %.1f°\n", 
                      gpsLatitude, gpsLongitude, gpsSpeed, gpsHeading);
    }
#endif
}

void GPSHandler::parseGPGGA(const char* sentence) {
    // $GPGGA,time,lat,latdir,lon,londir,quality,numsat,hdop,alt,altunit,geoid,geoidunit,dgpstime,dgpsid,checksum
    // Example: $GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
    
    char field[32];
    
    // Field 6: Fix quality (0=invalid, 1=GPS fix, 2=DGPS fix)
    extractField(sentence, 6, field, sizeof(field));
    int quality = parseInt(field);
    
    if (quality == 0) {
        gpsDataValid = false;
        return;
    }
    
    // Field 7: Number of satellites
    extractField(sentence, 7, field, sizeof(field));
    gpsNumSats = (uint8_t)parseInt(field);
    
    // Field 9: Altitude
    extractField(sentence, 9, field, sizeof(field));
    gpsAltitude = parseFloat(field);
    
    gpsLastUpdate = millis();
    
#if ENABLE_DEBUG_MODE
    if (debugMode) {
        Serial.printf("[GPS] GPGGA - Quality: %d, Sats: %d, Alt: %.1fm\n", 
                      quality, gpsNumSats, gpsAltitude);
    }
#endif
}

void GPSHandler::parseGPVTG(const char* sentence) {
    // $GPVTG,course,T,course,M,speed_knots,N,speed_kmh,K,checksum
    // Example: $GPVTG,054.7,T,034.4,M,005.5,N,010.2,K*48
    
    char field[32];
    
    // Field 1: True course
    extractField(sentence, 1, field, sizeof(field));
    float trueCourse = parseFloat(field);
    
    // Field 5: Speed in knots  
    extractField(sentence, 5, field, sizeof(field));
    float speedKnots = parseFloat(field);
    
    // Field 7: Speed in km/h
    extractField(sentence, 7, field, sizeof(field));
    float speedKmh = parseFloat(field);
    
    // Update GPS variables if data is valid
    if (trueCourse >= 0 && trueCourse <= 360) {
        gpsHeading = trueCourse;
    }
    if (speedKmh >= 0) {
        gpsSpeed = speedKmh;
    } else if (speedKnots >= 0) {
        gpsSpeed = speedKnots * 1.852; // Convert knots to km/h
    }
    

    
    gpsLastUpdate = millis();
}

double GPSHandler::parseCoordinate(const char* coord, char direction) {
    if (!coord || strlen(coord) == 0) return 0.0;
    
    // Format: DDMM.MMMM or DDDMM.MMMM
    double value = atof(coord);
    int degrees = (int)(value / 100);
    double minutes = value - (degrees * 100);
    
    return degrees + (minutes / 60.0);
}

float GPSHandler::parseFloat(const char* str) {
    if (!str || strlen(str) == 0) return 0.0;
    return atof(str);
}

int GPSHandler::parseInt(const char* str) {
    if (!str || strlen(str) == 0) return 0;
    return atoi(str);
}

void GPSHandler::extractField(const char* sentence, int fieldNum, char* buffer, size_t bufferSize) {
    if (!sentence || !buffer || bufferSize == 0) {
        buffer[0] = '\0';
        return;
    }
    
    int currentField = 0;
    int pos = 0;
    int bufferPos = 0;
    
    // Skip to the desired field
    while (sentence[pos] && currentField < fieldNum) {
        if (sentence[pos] == ',') {
            currentField++;
        }
        pos++;
    }
    
    // Extract the field
    if (currentField == fieldNum) {
        while (sentence[pos] && sentence[pos] != ',' && sentence[pos] != '*' && 
               bufferPos < bufferSize - 1) {
            buffer[bufferPos++] = sentence[pos++];
        }
    }
    
    buffer[bufferPos] = '\0';
}

// Getter functions
float GPSHandler::getSpeed() { return gpsSpeed; }
double GPSHandler::getLatitude() { return gpsLatitude; }
double GPSHandler::getLongitude() { return gpsLongitude; }
float GPSHandler::getHeading() { return gpsHeading; }

void GPSHandler::parseGPGSA(const char* sentence) {
    // $GPGSA,A,3,04,05,,09,12,,,24,,,,,2.5,1.3,2.1*39
    // Field 1: Mode (A = Auto, M = Manual)
    // Field 2: Fix type (1 = No fix, 2 = 2D, 3 = 3D)
    // Fields 3-14: Satellite PRNs used in fix
    // Field 15: PDOP (Position Dilution of Precision)
    // Field 16: HDOP (Horizontal DOP)
    // Field 17: VDOP (Vertical DOP)
    
    char field[32];
    
    // Field 2: Fix type
    extractField(sentence, 2, field, sizeof(field));
    int fixType = parseInt(field);
    
    // Field 15: PDOP 
    extractField(sentence, 15, field, sizeof(field));
    float pdop = parseFloat(field);
    
    // Field 16: HDOP
    extractField(sentence, 16, field, sizeof(field));  
    float hdop = parseFloat(field);
    
    // Field 17: VDOP
    extractField(sentence, 17, field, sizeof(field));
    float vdop = parseFloat(field);
    
    // Essential fix status output
    static uint32_t lastGSADebug = 0;
    if (millis() - lastGSADebug > 8000) {
        if (fixType == 1) {
            Serial.println("[GPS] Signal: SEARCHING - move to open sky");
        } else if (fixType >= 2) {
            Serial.printf("[GPS] Signal: LOCKED (%s) | Accuracy: %.1f\n", 
                          fixType == 3 ? "3D" : "2D", hdop);
        }
        lastGSADebug = millis();
    }
    
    // Update GPS validity based on fix type and precision
    gpsDataValid = (fixType >= 2 && hdop > 0 && hdop < 10.0);
}

void GPSHandler::parseGPGSV(const char* sentence) {
    // $GPGSV,3,1,11,03,03,111,00,04,15,270,00,06,01,010,00,13,06,292,00*74
    // Field 1: Total number of sentences for full data
    // Field 2: Sentence number (1, 2, 3)  
    // Field 3: Total number of satellites in view
    // Fields 4+: Satellite data (PRN, Elevation, Azimuth, SNR) - 4 sats per sentence
    
    char field[32];
    
    // Field 2: Current sentence number
    extractField(sentence, 2, field, sizeof(field));
    int sentenceNum = parseInt(field);
    
    // Field 3: Total satellites in view (only in first sentence)
    if (sentenceNum == 1) {
        extractField(sentence, 3, field, sizeof(field));
        int satsInView = parseInt(field);
        
        // Count satellites with signal (SNR > 0)
        int satsWithSignal = 0;
        
        // Check up to 4 satellites in this sentence (fields 4, 8, 12, 16)
        for (int i = 0; i < 4; i++) {
            int snrField = 7 + (i * 4); // SNR fields are 7, 11, 15, 19
            extractField(sentence, snrField, field, sizeof(field));
            int snr = parseInt(field);
            if (snr > 0) {
                satsWithSignal++;
            }
        }
        
        // Essential satellite status
        static uint32_t lastGSVDebug = 0;
        if (millis() - lastGSVDebug > 10000) {
            Serial.printf("[GPS] Satellites: %d visible, %d strong\n", satsInView, satsWithSignal);
            lastGSVDebug = millis();
        }
    }
}
float GPSHandler::getAltitude() { return gpsAltitude; }
uint8_t GPSHandler::getSatellites() { return gpsNumSats; }
bool GPSHandler::isDataValid() { return gpsDataValid; }