#include <Arduino.h>
#include "storage.h"
#include "motor.h"
#include "wifi_manager.h"
#include "sinric_blinds.h"
#include "web_server.h"
#include "scheduler.h"

void setup() {
    Serial.begin(115200);
    delay(200);
    Serial.println("\n=== Smart Blinds Controller ===");

    storageInit();
    motorInit();
    wifiInit();
    sinricInit();
    webServerInit();
    schedulerInit();

    Serial.println("Ready!");
}

void loop() {
    motorLoop();
    sinricLoop();
    wifiLoop();
    schedulerLoop();
}
