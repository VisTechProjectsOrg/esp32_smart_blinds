#include "storage.h"
#include "config.h"
#include <Preferences.h>

static Preferences prefs;

void storageInit() {
    prefs.begin("blinds", false);
}

void saveCalibration(long closedPos, long openPos) {
    prefs.putLong("calClosed", closedPos);
    prefs.putLong("calOpen", openPos);
    prefs.putBool("calDone", true);
}

void loadCalibration(long& closedPos, long& openPos) {
    closedPos = prefs.getLong("calClosed", 0);
    openPos = prefs.getLong("calOpen", 0);
}

bool hasCalibration() {
    return prefs.getBool("calDone", false);
}

void saveCurrentPosition(long pos) {
    prefs.putLong("curPos", pos);
}

long loadCurrentPosition() {
    return prefs.getLong("curPos", 0);
}

void saveDeviceName(const char* name) {
    prefs.putString("devName", name);
}

void loadDeviceName(char* name, size_t maxLen) {
    String s = prefs.getString("devName", DEVICE_NAME);
    strncpy(name, s.c_str(), maxLen - 1);
    name[maxLen - 1] = '\0';
}

void saveSunSettings(bool autoOpen, bool autoClose, int sunriseOffset, int sunsetOffset) {
    prefs.putBool("autoOpen", autoOpen);
    prefs.putBool("autoClose", autoClose);
    prefs.putInt("sunOff", sunriseOffset);
    prefs.putInt("setOff", sunsetOffset);
}

void loadSunSettings(bool& autoOpen, bool& autoClose, int& sunriseOffset, int& sunsetOffset) {
    autoOpen = prefs.getBool("autoOpen", true);
    autoClose = prefs.getBool("autoClose", true);
    sunriseOffset = prefs.getInt("sunOff", 0);
    sunsetOffset = prefs.getInt("setOff", 0);
}
