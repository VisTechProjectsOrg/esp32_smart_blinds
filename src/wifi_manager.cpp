#include "wifi_manager.h"
#include "storage.h"
#include "config.h"
#include <WiFi.h>
#include <ESPmDNS.h>

char deviceName[32] = "";

static bool apMode = false;
static unsigned long lastReconnectAttempt = 0;

bool wifiIsAPMode() { return apMode; }

static void startAP() {
    apMode = true;
    WiFi.mode(WIFI_AP);
    String apName = String(deviceName) + "-setup";
    WiFi.softAP(apName.c_str());
    Serial.printf("[WiFi] AP mode: connect to '%s' at 192.168.4.1\n", apName.c_str());
}

void wifiInit() {
    // Try stored credentials first, fall back to hardcoded
    char ssid[64] = "", pass[64] = "";
    if (hasWiFiCreds()) {
        loadWiFiCreds(ssid, sizeof(ssid), pass, sizeof(pass));
        Serial.printf("[WiFi] Using stored credentials for '%s'\n", ssid);
    } else if (strlen(WIFI_SSID) > 0) {
        strncpy(ssid, WIFI_SSID, sizeof(ssid) - 1);
        strncpy(pass, WIFI_PASS, sizeof(pass) - 1);
        Serial.printf("[WiFi] Using hardcoded credentials for '%s'\n", ssid);
    }

    if (strlen(ssid) == 0) {
        Serial.println("[WiFi] No credentials configured");
        startAP();
        return;
    }

    Serial.printf("[WiFi] Connecting to %s", ssid);
    WiFi.setSleep(false);
    WiFi.setAutoReconnect(true);
    WiFi.begin(ssid, pass);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 60) {
        Serial.print(".");
        delay(250);
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("\n[WiFi] Connected! IP: %s\n", WiFi.localIP().toString().c_str());

        if (MDNS.begin(deviceName)) {
            Serial.printf("[mDNS] http://%s.local\n", deviceName);
        }
    } else {
        Serial.println("\n[WiFi] Connection failed - starting AP mode");
        startAP();
    }
}

void wifiLoop() {
    if (apMode) return;

    if (WiFi.status() != WL_CONNECTED) {
        unsigned long now = millis();
        if (now - lastReconnectAttempt > 30000) {
            lastReconnectAttempt = now;
            Serial.println("[WiFi] Reconnecting...");
            WiFi.reconnect();
        }
    }
}

String wifiGetIP() {
    if (apMode) return WiFi.softAPIP().toString();
    return WiFi.localIP().toString();
}

bool wifiIsConnected() {
    return WiFi.status() == WL_CONNECTED;
}
