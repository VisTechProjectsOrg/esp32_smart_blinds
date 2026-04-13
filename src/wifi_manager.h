#pragma once

#include <Arduino.h>

// Global device name (loaded from flash, max 31 chars)
extern char deviceName[32];

void wifiInit();
void wifiLoop();
String wifiGetIP();
bool wifiIsConnected();
bool wifiIsAPMode();
