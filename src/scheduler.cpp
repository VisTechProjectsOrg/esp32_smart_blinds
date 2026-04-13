#include "scheduler.h"
#include "motor.h"
#include "sinric_blinds.h"
#include "storage.h"
#include "wifi_manager.h"
#include "config.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <time.h>

static ScheduleSettings sched;

// Sun times as minutes since midnight (local time)
static int sunriseMin = -1;
static int sunsetMin = -1;

// Formatted strings for display
static String sunriseStr = "--:--";
static String sunsetStr = "--:--";

static bool triggeredOpenToday = false;
static bool triggeredCloseToday = false;
static int lastFetchDay = -1;
static unsigned long lastCheck = 0;

static String minutesToTime(int minutes) {
    if (minutes < 0) return "--:--";
    int h = minutes / 60;
    int m = minutes % 60;
    char buf[8];
    snprintf(buf, sizeof(buf), "%d:%02d%s",
             h > 12 ? h - 12 : (h == 0 ? 12 : h),
             m, h >= 12 ? "PM" : "AM");
    return String(buf);
}

static void fetchSunTimes() {
    if (!wifiIsConnected()) return;

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    float lat, lng;
    loadLocation(lat, lng);

    char url[128];
    snprintf(url, sizeof(url),
             "https://api.sunrise-sunset.org/json?lat=%.4f&lng=%.4f&formatted=0",
             lat, lng);

    http.begin(client, url);
    http.setTimeout(5000);
    int code = http.GET();

    if (code == 200) {
        String payload = http.getString();
        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, payload);

        if (!err && doc["status"] == "OK") {
            const char* riseStr = doc["results"]["sunrise"];
            const char* setStr = doc["results"]["sunset"];

            if (riseStr && setStr) {
                // Parse UTC times and convert using system timezone (auto DST)
                int rH, rM, rS, sH, sM, sS;
                int rY, rMo, rD, sY, sMo, sD;
                // "2026-04-12T10:32:15+00:00"
                sscanf(riseStr, "%d-%d-%dT%d:%d:%d", &rY, &rMo, &rD, &rH, &rM, &rS);
                sscanf(setStr, "%d-%d-%dT%d:%d:%d", &sY, &sMo, &sD, &sH, &sM, &sS);

                // Build UTC time_t and convert to local via mktime/localtime
                struct tm utcRise = {};
                utcRise.tm_year = rY - 1900; utcRise.tm_mon = rMo - 1; utcRise.tm_mday = rD;
                utcRise.tm_hour = rH; utcRise.tm_min = rM; utcRise.tm_sec = rS;
                time_t riseEpoch = mktime(&utcRise) - _timezone;
                struct tm localRise;
                localtime_r(&riseEpoch, &localRise);

                struct tm utcSet = {};
                utcSet.tm_year = sY - 1900; utcSet.tm_mon = sMo - 1; utcSet.tm_mday = sD;
                utcSet.tm_hour = sH; utcSet.tm_min = sM; utcSet.tm_sec = sS;
                time_t setEpoch = mktime(&utcSet) - _timezone;
                struct tm localSet;
                localtime_r(&setEpoch, &localSet);

                sunriseMin = localRise.tm_hour * 60 + localRise.tm_min;
                sunsetMin = localSet.tm_hour * 60 + localSet.tm_min;
                sunriseStr = minutesToTime(sunriseMin);
                sunsetStr = minutesToTime(sunsetMin);

                struct tm timeinfo;
                getLocalTime(&timeinfo);
                lastFetchDay = timeinfo.tm_yday;

                Serial.printf("[Scheduler] Sunrise: %s, Sunset: %s\n",
                              sunriseStr.c_str(), sunsetStr.c_str());
            }
        }
    } else {
        Serial.printf("[Scheduler] API fetch failed: %d\n", code);
    }
    http.end();
}

static void autoDetectLocation() {
    if (!wifiIsConnected()) return;

    // Skip if user already set a custom location
    float lat, lng;
    loadLocation(lat, lng);
    if (lat != DEFAULT_LATITUDE || lng != DEFAULT_LONGITUDE) {
        Serial.printf("[Scheduler] Using saved location: %.4f, %.4f\n", lat, lng);
        return;
    }

    HTTPClient http;
    http.begin("http://ip-api.com/json/?fields=lat,lon");
    http.setTimeout(5000);
    int code = http.GET();

    if (code == 200) {
        String payload = http.getString();
        JsonDocument doc;
        if (deserializeJson(doc, payload) == DeserializationError::Ok) {
            float newLat = doc["lat"] | 0.0f;
            float newLon = doc["lon"] | 0.0f;
            if (newLat != 0.0f && newLon != 0.0f) {
                saveLocation(newLat, newLon);
                Serial.printf("[Scheduler] Auto-detected location: %.4f, %.4f\n", newLat, newLon);
            }
        }
    } else {
        Serial.printf("[Scheduler] Location detect failed: %d\n", code);
    }
    http.end();
}

void schedulerInit() {
    configTzTime("EST5EDT,M3.2.0,M11.1.0", "pool.ntp.org", "time.nist.gov");
    Serial.println("[Scheduler] NTP sync started (auto DST)");

    loadScheduleSettings(sched);
    autoDetectLocation();
}

void schedulerLoop() {
    unsigned long now = millis();
    if (now - lastCheck < 30000) return;
    lastCheck = now;

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo, 100)) return;

    int today = timeinfo.tm_yday;
    int nowMin = timeinfo.tm_hour * 60 + timeinfo.tm_min;

    // Fetch sun times once per day
    if (lastFetchDay != today) {
        fetchSunTimes();
        triggeredOpenToday = false;
        triggeredCloseToday = false;
    }

    if (!motorIsCalibrated()) return;

    // Open trigger
    int openAt = -1;
    if (strcmp(sched.openMode, "fixed") == 0) {
        openAt = sched.openHour * 60 + sched.openMin;
    } else if (sunriseMin >= 0) {
        openAt = sunriseMin + sched.sunriseOffset;
    }

    if (sched.autoOpen && openAt >= 0 && !triggeredOpenToday && nowMin >= openAt && nowMin < openAt + 5) {
        Serial.printf("[Scheduler] Open trigger at %s\n",
                       strcmp(sched.openMode, "fixed") == 0 ? "fixed time" : "sunrise");
        motorOpen();
        sinricSendPosition(100);
        triggeredOpenToday = true;
    }

    // Close trigger
    int closeAt = -1;
    if (strcmp(sched.closeMode, "fixed") == 0) {
        closeAt = sched.closeHour * 60 + sched.closeMin;
    } else if (sunsetMin >= 0) {
        closeAt = sunsetMin + sched.sunsetOffset;
    }

    if (sched.autoClose && closeAt >= 0 && !triggeredCloseToday && nowMin >= closeAt && nowMin < closeAt + 5) {
        Serial.printf("[Scheduler] Close trigger at %s\n",
                       strcmp(sched.closeMode, "fixed") == 0 ? "fixed time" : "sunset");
        motorClose();
        sinricSendPosition(0);
        triggeredCloseToday = true;
    }
}

void schedulerGetSunTimes(String& sunrise, String& sunset) {
    sunrise = sunriseStr;
    sunset = sunsetStr;
}

void schedulerUpdateSettings(const ScheduleSettings& s) {
    sched = s;
}
