#pragma once

#include <Arduino.h>

// --- Pin assignments (Freenove ESP32 WROOM + TMC2208) ---
#define STEP_PIN  25
#define DIR_PIN   26
#define MS1_PIN   27
#define MS2_PIN   14

// --- Motor defaults ---
#define DEFAULT_MAX_SPEED     2000    // steps/sec
#define DEFAULT_ACCEL         1000    // steps/sec^2
#define MOTOR_STEPS_PER_REV   200
#define MICROSTEPS            16

// --- WiFi credentials ---
#define WIFI_SSID     "YOUR_WIFI_SSID"
#define WIFI_PASS     "YOUR_WIFI_PASSWORD"

// --- SinricPro credentials (from sinric.pro dashboard) ---
#define SINRIC_APP_KEY    "YOUR_APP_KEY"
#define SINRIC_APP_SECRET "YOUR_APP_SECRET"
#define SINRIC_BLINDS_ID  "YOUR_BLINDS_DEVICE_ID"

// --- Device settings ---
#define DEVICE_NAME       "smartblinds"    // mDNS: smartblinds.local

// --- Sunrise/sunset location ---
#define DEFAULT_LATITUDE    43.65f   // Toronto area - change to your location
#define DEFAULT_LONGITUDE  -79.38f
#define DEFAULT_UTC_OFFSET -5
