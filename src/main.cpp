#include <Arduino.h>
#include "config.h"
#include "storage.h"
#include "motor.h"
#include "wifi_manager.h"
#include "sinric_blinds.h"
#include "web_server.h"
#include "scheduler.h"
#include <OTAUpdater.h>

#define FIRMWARE_VERSION "1.0.0"

// Button state
static bool lastBtnState = HIGH;
static unsigned long btnPressTime = 0;
static unsigned long lastBtnChange = 0;
static bool btnHandled = false;
static bool stoppedByButton = false;

void setup() {
    Serial.begin(115200);
    delay(200);
    Serial.println("\n=== Smart Blinds Controller ===");

    storageInit();

    // Factory reset: hold button during boot for 5 seconds
    pinMode(BTN_PIN, INPUT_PULLUP);
    if (digitalRead(BTN_PIN) == LOW) {
        Serial.println("Button held on boot - hold 5s to factory reset...");
        unsigned long start = millis();
        while (digitalRead(BTN_PIN) == LOW && millis() - start < 5000) {
            delay(100);
        }
        if (millis() - start >= 5000) {
            Serial.println("FACTORY RESET");
            clearAllSettings();
            delay(500);
            ESP.restart();
        }
        Serial.println("Released early - normal boot");
    }

    loadDeviceName(deviceName, sizeof(deviceName));
    Serial.printf("Device name: %s\n", deviceName);
    motorInit();

    wifiInit();
    sinricInit();
    webServerInit();

    OTAConfig ota = {};
    ota.githubOrg = "VisTechProjectsOrg";
    ota.githubRepo = "esp32_smart_blinds";
    ota.githubBranch = "firmware";
    ota.firmwareVersion = FIRMWARE_VERSION;
    ota.updatePath = "/update";
    otaInit(webServerGet(), ota);

    schedulerInit();

    Serial.println("Ready!");
}

void loop() {
    motorLoop();
    sinricLoop();
    wifiLoop();
    schedulerLoop();
    otaLoop();

    // Button logic:
    //   Moving + press = stop (pause)
    //   Paused + short press (<1s) = reverse direction
    //   Paused + long press (>1s) = continue same direction
    //   Stopped (at endpoint) + press = go opposite direction

    bool btnState = digitalRead(BTN_PIN);
    unsigned long now = millis();

    // Debounce - ignore changes within 250ms of last change
    if (btnState != lastBtnState && now - lastBtnChange < 250) {
        btnState = lastBtnState;
    }

    // Button pressed
    if (btnState == LOW && lastBtnState == HIGH) {
        lastBtnChange = now;
        btnPressTime = now;
        btnHandled = false;

        if (motorIsMoving()) {
            motorStop();
            stoppedByButton = true;
            btnHandled = true;
            Serial.println("[Button] Paused");
        }
    }

    // Button released
    if (btnState == HIGH && lastBtnState == LOW) {
        lastBtnChange = now;
    }
    if (btnState == HIGH && lastBtnState == LOW && !btnHandled) {
        unsigned long held = millis() - btnPressTime;

        if (stoppedByButton) {
            if (held > 1000) {
                // Long press after pause - continue same direction
                if (motorWasOpening()) {
                    motorOpen();
                    sinricSendPosition(100);
                    Serial.println("[Button] Continuing open");
                } else {
                    motorClose();
                    sinricSendPosition(0);
                    Serial.println("[Button] Continuing close");
                }
            } else {
                // Short press after pause - reverse
                if (motorWasOpening()) {
                    motorClose();
                    sinricSendPosition(0);
                    Serial.println("[Button] Reversing to close");
                } else {
                    motorOpen();
                    sinricSendPosition(100);
                    Serial.println("[Button] Reversing to open");
                }
            }
            stoppedByButton = false;
        } else {
            // Not paused - toggle based on current position
            if (motorGetPercent() > 50) {
                motorClose();
                sinricSendPosition(0);
                Serial.println("[Button] Closing");
            } else {
                motorOpen();
                sinricSendPosition(100);
                Serial.println("[Button] Opening");
            }
        }
    }

    lastBtnState = btnState;
}
