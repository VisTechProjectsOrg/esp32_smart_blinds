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

void saveMotorSpeed(int speed, int accel) {
    prefs.putInt("mSpeed", speed);
    prefs.putInt("mAccel", accel);
}

void loadMotorSpeed(int& speed, int& accel) {
    speed = prefs.getInt("mSpeed", DEFAULT_MAX_SPEED);
    accel = prefs.getInt("mAccel", DEFAULT_ACCEL);
}

void saveScheduleSettings(const ScheduleSettings& s) {
    prefs.putBool("autoOpen", s.autoOpen);
    prefs.putBool("autoClose", s.autoClose);
    prefs.putString("openMode", s.openMode);
    prefs.putString("closeMode", s.closeMode);
    prefs.putInt("sunOff", s.sunriseOffset);
    prefs.putInt("setOff", s.sunsetOffset);
    prefs.putInt("openH", s.openHour);
    prefs.putInt("openM", s.openMin);
    prefs.putInt("closeH", s.closeHour);
    prefs.putInt("closeM", s.closeMin);
}

void loadScheduleSettings(ScheduleSettings& s) {
    s.autoOpen = prefs.getBool("autoOpen", true);
    s.autoClose = prefs.getBool("autoClose", true);
    String om = prefs.getString("openMode", "sunrise");
    strncpy(s.openMode, om.c_str(), sizeof(s.openMode) - 1);
    String cm = prefs.getString("closeMode", "sunset");
    strncpy(s.closeMode, cm.c_str(), sizeof(s.closeMode) - 1);
    s.sunriseOffset = prefs.getInt("sunOff", 0);
    s.sunsetOffset = prefs.getInt("setOff", 0);
    s.openHour = prefs.getInt("openH", 8);
    s.openMin = prefs.getInt("openM", 0);
    s.closeHour = prefs.getInt("closeH", 21);
    s.closeMin = prefs.getInt("closeM", 0);
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
