#include "web_server.h"
#include "web_ui.h"
#include "motor.h"
#include "storage.h"
#include "scheduler.h"
#include "wifi_manager.h"
#include "config.h"
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

static AsyncWebServer server(80);

AsyncWebServer& webServerGet() { return server; }

// Buffer for collecting POST body data
static String bodyBuffer;

static void sendStatus(AsyncWebServerRequest* request) {
    JsonDocument doc;
    doc["position"] = motorGetPercent();
    doc["moving"] = motorIsMoving();
    doc["calibrated"] = motorIsCalibrated();
    doc["ip"] = wifiGetIP();
    doc["name"] = deviceName;

    String sunrise, sunset;
    schedulerGetSunTimes(sunrise, sunset);
    doc["sunrise"] = sunrise;
    doc["sunset"] = sunset;

    String json;
    serializeJson(doc, json);
    request->send(200, "application/json", json);
}

void webServerInit() {
    // Serve the web UI
    server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send_P(200, "text/html", INDEX_HTML);
    });

    // Status endpoint
    server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest* request) {
        sendStatus(request);
    });

    // Open
    server.on("/api/open", HTTP_POST, [](AsyncWebServerRequest* request) {
        motorOpen();
        sendStatus(request);
    });

    // Close
    server.on("/api/close", HTTP_POST, [](AsyncWebServerRequest* request) {
        motorClose();
        sendStatus(request);
    });

    // Stop
    server.on("/api/stop", HTTP_POST, [](AsyncWebServerRequest* request) {
        motorStop();
        sendStatus(request);
    });

    // Move to position - uses query param: /api/move?pos=50
    server.on("/api/move", HTTP_POST, [](AsyncWebServerRequest* request) {
        if (request->hasParam("pos", true)) {
            int pos = request->getParam("pos", true)->value().toInt();
            motorMoveTo(pos);
        }
        sendStatus(request);
    },
    NULL,
    // Body handler for JSON
    [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
        if (index == 0) bodyBuffer = "";
        bodyBuffer += String((char*)data).substring(0, len);
        if (index + len == total) {
            JsonDocument doc;
            if (deserializeJson(doc, bodyBuffer) == DeserializationError::Ok) {
                int pos = doc["position"] | 0;
                motorMoveTo(pos);
            }
        }
    });

    // Jog - uses query param: /api/jog?dir=1
    server.on("/api/jog", HTTP_POST, [](AsyncWebServerRequest* request) {
        if (request->hasParam("dir", true)) {
            int dir = request->getParam("dir", true)->value().toInt();
            motorJog(dir);
        }
        sendStatus(request);
    },
    NULL,
    [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
        if (index == 0) bodyBuffer = "";
        bodyBuffer += String((char*)data).substring(0, len);
        if (index + len == total) {
            JsonDocument doc;
            if (deserializeJson(doc, bodyBuffer) == DeserializationError::Ok) {
                int dir = doc["direction"] | 1;
                motorJog(dir);
            }
        }
    });

    // Jog stop
    server.on("/api/jogStop", HTTP_POST, [](AsyncWebServerRequest* request) {
        motorJogStop();
        sendStatus(request);
    });

    // Calibration: set closed
    server.on("/api/setClosed", HTTP_POST, [](AsyncWebServerRequest* request) {
        motorSetClosed();
        JsonDocument doc;
        doc["msg"] = "Closed position set";
        String json;
        serializeJson(doc, json);
        request->send(200, "application/json", json);
    });

    // Calibration: set open
    server.on("/api/setOpen", HTTP_POST, [](AsyncWebServerRequest* request) {
        motorSetOpen();
        JsonDocument doc;
        doc["msg"] = "Open position set - calibration complete";
        doc["calibrated"] = true;
        String json;
        serializeJson(doc, json);
        request->send(200, "application/json", json);
    });

    // Get schedule settings
    server.on("/api/schedule", HTTP_GET, [](AsyncWebServerRequest* request) {
        bool autoOpen, autoClose;
        int sunriseOff, sunsetOff;
        loadSunSettings(autoOpen, autoClose, sunriseOff, sunsetOff);

        JsonDocument doc;
        doc["autoOpen"] = autoOpen;
        doc["autoClose"] = autoClose;
        doc["sunriseOffset"] = sunriseOff;
        doc["sunsetOffset"] = sunsetOff;
        String json;
        serializeJson(doc, json);
        request->send(200, "application/json", json);
    });

    // Save schedule settings
    server.on("/api/schedule", HTTP_POST, [](AsyncWebServerRequest* request) {
        sendStatus(request);
    },
    NULL,
    [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
        if (index == 0) bodyBuffer = "";
        bodyBuffer += String((char*)data).substring(0, len);
        if (index + len == total) {
            JsonDocument doc;
            if (deserializeJson(doc, bodyBuffer) == DeserializationError::Ok) {
                bool ao = doc["autoOpen"] | true;
                bool ac = doc["autoClose"] | true;
                int sunOff = doc["sunriseOffset"] | 0;
                int setOff = doc["sunsetOffset"] | 0;
                saveSunSettings(ao, ac, sunOff, setOff);
                schedulerUpdateSettings(ao, ac, sunOff, setOff);
            }
        }
    });

    // Rename device
    server.on("/api/rename", HTTP_POST, [](AsyncWebServerRequest* request) {
        sendStatus(request);
    },
    NULL,
    [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
        if (index == 0) bodyBuffer = "";
        bodyBuffer += String((char*)data).substring(0, len);
        if (index + len == total) {
            JsonDocument doc;
            if (deserializeJson(doc, bodyBuffer) == DeserializationError::Ok) {
                const char* newName = doc["name"];
                if (newName && strlen(newName) > 0 && strlen(newName) < 32) {
                    saveDeviceName(newName);
                    strncpy(deviceName, newName, sizeof(deviceName) - 1);
                    Serial.printf("[Web] Device renamed to: %s (reboot to apply mDNS)\n", deviceName);
                }
            }
        }
    });

    server.begin();
    Serial.println("[Web] Server started on port 80");
}
