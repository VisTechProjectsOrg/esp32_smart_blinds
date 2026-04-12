#include "scheduler.h"
#include "motor.h"
#include "sinric_blinds.h"
#include "storage.h"
#include "wifi_manager.h"
#include "config.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>

static bool autoOpen = true;
static bool autoClose = true;
static int sunriseOffsetMin = 0;
static int sunsetOffsetMin = 0;

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

    HTTPClient http;
    char url[128];
    snprintf(url, sizeof(url),
             "http://api.sunrise-sunset.org/json?lat=%.4f&lng=%.4f&formatted=0",
             DEFAULT_LATITUDE, DEFAULT_LONGITUDE);

    http.begin(url);
    http.setTimeout(5000);
    int code = http.GET();

    if (code == 200) {
        String payload = http.getString();
        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, payload);

        if (!err && doc["status"] == "OK") {
            // API returns ISO 8601 UTC times like "2026-04-12T10:32:00+00:00"
            const char* riseStr = doc["results"]["sunrise"];
            const char* setStr = doc["results"]["sunset"];

            if (riseStr && setStr) {
                // Parse hours and minutes from ISO time
                // Format: "YYYY-MM-DDTHH:MM:SS+00:00"
                int rH, rM, sH, sM;
                sscanf(strchr(riseStr, 'T') + 1, "%d:%d", &rH, &rM);
                sscanf(strchr(setStr, 'T') + 1, "%d:%d", &sH, &sM);

                // Convert from UTC to local
                rH += DEFAULT_UTC_OFFSET;
                sH += DEFAULT_UTC_OFFSET;
                if (rH < 0) rH += 24;
                if (sH < 0) sH += 24;
                if (rH >= 24) rH -= 24;
                if (sH >= 24) sH -= 24;

                sunriseMin = rH * 60 + rM;
                sunsetMin = sH * 60 + sM;
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

void schedulerInit() {
    // NTP time sync
    configTime(DEFAULT_UTC_OFFSET * 3600, 0, "pool.ntp.org", "time.nist.gov");
    Serial.println("[Scheduler] NTP sync started");

    // Load saved settings
    loadSunSettings(autoOpen, autoClose, sunriseOffsetMin, sunsetOffsetMin);
}

void schedulerLoop() {
    unsigned long now = millis();
    if (now - lastCheck < 30000) return;  // check every 30 seconds
    lastCheck = now;

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo, 100)) return;  // no time yet

    int today = timeinfo.tm_yday;
    int nowMin = timeinfo.tm_hour * 60 + timeinfo.tm_min;

    // Fetch sun times once per day (or if never fetched)
    if (lastFetchDay != today) {
        fetchSunTimes();
        triggeredOpenToday = false;
        triggeredCloseToday = false;
    }

    if (sunriseMin < 0 || sunsetMin < 0) return;
    if (!motorIsCalibrated()) return;

    // Sunrise trigger
    int openAt = sunriseMin + sunriseOffsetMin;
    if (autoOpen && !triggeredOpenToday && nowMin >= openAt && nowMin < openAt + 5) {
        Serial.println("[Scheduler] Sunrise trigger - opening");
        motorOpen();
        sinricSendPosition(100);
        triggeredOpenToday = true;
    }

    // Sunset trigger
    int closeAt = sunsetMin + sunsetOffsetMin;
    if (autoClose && !triggeredCloseToday && nowMin >= closeAt && nowMin < closeAt + 5) {
        Serial.println("[Scheduler] Sunset trigger - closing");
        motorClose();
        sinricSendPosition(0);
        triggeredCloseToday = true;
    }
}

void schedulerGetSunTimes(String& sunrise, String& sunset) {
    sunrise = sunriseStr;
    sunset = sunsetStr;
}

void schedulerUpdateSettings(bool ao, bool ac, int sunOff, int setOff) {
    autoOpen = ao;
    autoClose = ac;
    sunriseOffsetMin = sunOff;
    sunsetOffsetMin = setOff;
}
