#pragma once

void storageInit();

// Calibration
void saveCalibration(long closedPos, long openPos);
void loadCalibration(long& closedPos, long& openPos);
bool hasCalibration();

// Motor position (survives power loss)
void saveCurrentPosition(long pos);
long loadCurrentPosition();

// Sunrise/sunset settings
void saveSunSettings(bool autoOpen, bool autoClose, int sunriseOffset, int sunsetOffset);
void loadSunSettings(bool& autoOpen, bool& autoClose, int& sunriseOffset, int& sunsetOffset);
