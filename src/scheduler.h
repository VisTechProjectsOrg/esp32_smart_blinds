#pragma once

#include <Arduino.h>

void schedulerInit();
void schedulerLoop();
void schedulerGetSunTimes(String& sunrise, String& sunset);
void schedulerUpdateSettings(bool autoOpen, bool autoClose, int sunriseOff, int sunsetOff);
