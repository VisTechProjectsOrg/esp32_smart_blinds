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

// Sunrise/sunset settings
void saveSunSettings(bool autoOpen, bool autoClose, int sunriseOffset, int sunsetOffset);
void loadSunSettings(bool& autoOpen, bool& autoClose, int& sunriseOffset, int& sunsetOffset);
