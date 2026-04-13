#pragma once

#include <Arduino.h>

void storageInit();

// Calibration
void saveCalibration(long closedPos, long openPos);
void loadCalibration(long& closedPos, long& openPos);
bool hasCalibration();

// Motor position (survives power loss)
void saveCurrentPosition(long pos);
long loadCurrentPosition();

// Device name (for mDNS)
void saveDeviceName(const char* name);
void loadDeviceName(char* name, size_t maxLen);

// Motor speed settings
void saveMotorSpeed(int speed, int accel);
void loadMotorSpeed(int& speed, int& accel);

// Location
void saveLocation(float lat, float lng);
void loadLocation(float& lat, float& lng);

// WiFi credentials
void saveWiFiCreds(const char* ssid, const char* pass);
void loadWiFiCreds(char* ssid, size_t ssidLen, char* pass, size_t passLen);
bool hasWiFiCreds();

// Factory reset
void clearAllSettings();

// Schedule settings
struct ScheduleSettings {
    bool autoOpen;
    bool autoClose;
    char openMode[8];    // "sunrise" or "fixed"
    char closeMode[8];   // "sunset" or "fixed"
    int sunriseOffset;
    int sunsetOffset;
    int openHour;        // fixed open time
    int openMin;
    int closeHour;       // fixed close time
    int closeMin;
};

void saveScheduleSettings(const ScheduleSettings& s);
void loadScheduleSettings(ScheduleSettings& s);

// Legacy compatibility
void saveSunSettings(bool autoOpen, bool autoClose, int sunriseOffset, int sunsetOffset);
void loadSunSettings(bool& autoOpen, bool& autoClose, int& sunriseOffset, int& sunsetOffset);
