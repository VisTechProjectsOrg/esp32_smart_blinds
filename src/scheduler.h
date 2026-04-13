#pragma once

#include <Arduino.h>
#include "storage.h"

void schedulerInit();
void schedulerLoop();
void schedulerGetSunTimes(String& sunrise, String& sunset);
void schedulerUpdateSettings(const ScheduleSettings& s);
