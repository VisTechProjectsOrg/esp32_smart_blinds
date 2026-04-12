#pragma once

#include <Arduino.h>

void wifiInit();
void wifiLoop();
String wifiGetIP();
bool wifiIsConnected();
