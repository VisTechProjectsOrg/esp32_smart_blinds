#include "wifi_manager.h"
#include "config.h"
#include <WiFi.h>
#include <ESPmDNS.h>

static unsigned long lastReconnectAttempt = 0;

void wifiInit() {
    Serial.printf("[WiFi] Connecting to %s", WIFI_SSID);

    WiFi.setSleep(false);
    WiFi.setAutoReconnect(true);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 60) {
        Serial.print(".");
        delay(250);
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("\n[WiFi] Connected! IP: %s\n", WiFi.localIP().toString().c_str());

        if (MDNS.begin(DEVICE_NAME)) {
            Serial.printf("[mDNS] http://%s.local\n", DEVICE_NAME);
        }
    } else {
        Serial.println("\n[WiFi] Connection failed - will keep retrying");
    }
}

void wifiLoop() {
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
    return WiFi.localIP().toString();
}

bool wifiIsConnected() {
    return WiFi.status() == WL_CONNECTED;
}
