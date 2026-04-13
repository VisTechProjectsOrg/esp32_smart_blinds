# ESP32 Smart Blinds Controller

ESP32-based smart blinds controller that drives a bead chain using a NEMA 17 stepper motor + TMC2208 driver. Control your blinds via Google Home, a web UI, sunrise/sunset schedule, or a physical button.

## Features

- **Google Home** voice control via SinricPro ("Hey Google, open the bedroom blinds to 50%")
- **Web UI** - mobile-friendly dashboard with position slider, open/close buttons
- **Sunrise/sunset scheduling** - auto-open at sunrise, auto-close at sunset (or fixed times)
- **Calibration** - jog motor via web UI to set open/close endpoints, then control by percentage
- **Physical button** - toggle open/close, pause mid-move, reverse or continue
- **OTA firmware updates** - upload new firmware from the browser
- **Position memory** - remembers position across power cycles
- **Multi-unit support** - rename each device, same firmware on all units
- **AP setup mode** - configure WiFi from a hotspot on first boot
- **Factory reset** - via web UI or hold button 5s on boot
- **Auto DST** - Eastern timezone with automatic daylight saving

## Hardware

| Component | Details |
|---|---|
| Microcontroller | ESP32 (Freenove WROOM) |
| Stepper driver | TMC2208 (StealthChop, silent) |
| Motor | NEMA 17 |
| Microstepping | 1/16 |

### Pin Assignments

| Pin | Function |
|---|---|
| GPIO 25 | STEP |
| GPIO 26 | DIR |
| GPIO 27 | MS1 |
| GPIO 14 | MS2 |
| GPIO 33 | EN (motor enable/disable) |
| GPIO 32 | Physical button (to GND) |

## Setup

1. Clone this repo
2. Copy `include/config.example.h` to `include/config.h`
3. Fill in your WiFi and SinricPro credentials in `config.h`
4. Build and flash with PlatformIO: `pio run -t upload`
5. Open `http://smartblinds.local` (or the IP shown in serial monitor)
6. Calibrate: expand Calibration section, jog motor to closed position, click "Set as CLOSED", jog to open, click "Set as OPEN"
7. Set up schedule, speed, device name in the web UI

### SinricPro Setup

1. Create a free account at [sinric.pro](https://sinric.pro)
2. Add a new device with type **Blinds**
3. Copy App Key, App Secret, and Device ID into `config.h`
4. Link SinricPro to Google Home in the Google Home app

### AP Setup Mode

If no WiFi is configured (or connection fails), the device creates a hotspot:
- SSID: `smartblinds-setup`
- Connect and go to `192.168.4.1` to configure WiFi and all settings

## Project Structure

```
src/
  main.cpp           - setup/loop, button logic, factory reset
  motor.cpp/h        - stepper control, position tracking, calibration
  wifi_manager.cpp/h - WiFi connection, AP mode, mDNS
  sinric_blinds.cpp/h - SinricPro Google Home integration
  web_server.cpp/h   - REST API endpoints
  web_ui.h           - embedded HTML/CSS/JS (PROGMEM)
  scheduler.cpp/h    - sunrise/sunset fetch, time-based triggers
  storage.cpp/h      - Preferences (flash) for all settings
include/
  config.h           - credentials and pin assignments (gitignored)
  config.example.h   - template for config.h
```

## OTA Updates

Navigate to `http://smartblinds.local/update` to upload new firmware from the browser. Drag and drop `firmware.bin` or use the GitHub auto-fetch button.

## Dependencies

- [AccelStepper](https://github.com/waspinator/AccelStepper)
- [SinricPro](https://github.com/sinricpro/esp8266-esp32-sdk)
- [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson)
- [ESP32OTAUpdater](https://github.com/VisTechProjectsOrg/esp32_ota_updater)
