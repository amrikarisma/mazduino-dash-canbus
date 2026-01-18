#ifndef GPS_HANDLER_H
#define GPS_HANDLER_H

#include <Arduino.h>
#include <HardwareSerial.h>

// GPS Handler for GT-U7 module
class GPSHandler {
public:
    GPSHandler();
    
    void begin();
    void update();
    void enableGPS(bool enable);
    bool isGPSEnabled();
    
    // GPS data getters
    float getSpeed();      // km/h
    double getLatitude();  // degrees
    double getLongitude(); // degrees
    float getHeading();    // degrees
    float getAltitude();   // meters
    uint8_t getSatellites();
    bool isDataValid();
    
private:
    HardwareSerial* gpsSerial;
    bool enabled;
    char nmeaBuffer[128];
    uint8_t bufferIndex;
    
    // NMEA parsing functions
    void parseNMEA(const char* sentence);
    void parseGPRMC(const char* sentence);  // Position, velocity, time
    void parseGPGGA(const char* sentence);  // Position, altitude, satellites
    void parseGPVTG(const char* sentence);  // Velocity and heading
    void parseGPGSA(const char* sentence);  // Satellite status, fix type, precision
    void parseGPGSV(const char* sentence);  // Satellites in view, signal strength
    
    // Helper functions
    double parseCoordinate(const char* coord, char direction);
    float parseFloat(const char* str);
    int parseInt(const char* str);
    void extractField(const char* sentence, int fieldNum, char* buffer, size_t bufferSize);
};

// Global GPS handler instance
extern GPSHandler gpsHandler;

#endif // GPS_HANDLER_H