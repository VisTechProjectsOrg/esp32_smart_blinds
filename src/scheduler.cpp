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
    char url[128];
    snprintf(url, sizeof(url),
             "https://api.sunrise-sunset.org/json?lat=%.4f&lng=%.4f&formatted=0",
             DEFAULT_LATITUDE, DEFAULT_LONGITUDE);

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
                int rH, rM, sH, sM;
                sscanf(strchr(riseStr, 'T') + 1, "%d:%d", &rH, &rM);
                sscanf(strchr(setStr, 'T') + 1, "%d:%d", &sH, &sM);

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
    configTime(DEFAULT_UTC_OFFSET * 3600, 0, "pool.ntp.org", "time.nist.gov");
    Serial.println("[Scheduler] NTP sync started");

    loadScheduleSettings(sched);
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
