#include "sinric_blinds.h"
#include "motor.h"
#include "config.h"
#include <SinricPro.h>
#include <SinricProBlinds.h>

static SinricProBlinds& blinds = SinricPro[SINRIC_BLINDS_ID];

// Google Home: "open/close the blinds"
bool onPowerState(const String& deviceId, bool& state) {
    Serial.printf("[SinricPro] Power: %s\n", state ? "on (open)" : "off (close)");
    if (state) {
        motorOpen();
    } else {
        motorClose();
    }
    return true;
}

// Google Home: "set blinds to 50%"
bool onRangeValue(const String& deviceId, int& value) {
    Serial.printf("[SinricPro] Set position: %d%%\n", value);
    motorMoveTo(value);
    return true;
}

// Google Home: "raise/lower the blinds"
bool onAdjustRangeValue(const String& deviceId, int& valueDelta) {
    int current = motorGetPercent();
    int target = constrain(current + valueDelta, 0, 100);
    Serial.printf("[SinricPro] Adjust position: %d%% -> %d%%\n", current, target);
    valueDelta = target;
    motorMoveTo(target);
    return true;
}

void sinricInit() {
    blinds.onPowerState(onPowerState);
    blinds.onRangeValue(onRangeValue);
    blinds.onAdjustRangeValue(onAdjustRangeValue);

    SinricPro.onConnected([]() {
        Serial.println("[SinricPro] Connected");
    });
    SinricPro.onDisconnected([]() {
        Serial.println("[SinricPro] Disconnected");
    });

    SinricPro.begin(SINRIC_APP_KEY, SINRIC_APP_SECRET);
    Serial.println("[SinricPro] Initialized");
}

void sinricLoop() {
    SinricPro.handle();
}

// Call this after motor finishes moving to sync state back to Google Home
void sinricSendPosition(int percent) {
    blinds.sendRangeValueEvent(percent);
}
