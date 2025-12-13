# MyESP_OTA

Minimal OTA helper for ESP8266/ESP32. It checks a backend for a new firmware version and downloads it via HTTP or HTTPS (controlled by the `USE_TLS` build flag).

## Features
- HTTP or HTTPS (when `USE_TLS` is defined; Let’s Encrypt roots included).
- Optional certificate verification (`verifyCert` argument).
- EEPROM-backed version tracking.
- Compact debug logs when `DEBUG` is defined.

## Dependencies
- ArduinoJson (v7)
- WiFi/WiFiClientSecure (board core)

## Usage
```cpp
#include <Arduino.h>
#include "SimpleOTA.h"
#include "secret_data.h" // defines PORTAL_SERVER_IP, DEVICE_TYPE_ID

SimpleOTA ota;

void setup() {
  Serial.begin(115200);
  // connect WiFi here...

  // EEPROM size for version storage, server (no http/https prefix), device id, verifyCert
  ota.begin(512, PORTAL_SERVER_IP, DEVICE_TYPE_ID, false);
}

void loop() {
  ota.checkUpdates(300); // check every 300 seconds
}
```

## HTTPS
- Build with `-DUSE_TLS` to enable HTTPS. Without it, OTA uses HTTP only.
- `verifyCert=true` uses embedded Let’s Encrypt roots on ESP8266/ESP32; `false` calls `setInsecure()` (lighter but no validation).

## Debug
- Uncomment `//#define DEBUG` in `SimpleOTA.h` (or add `-DDEBUG` to build flags) to enable `*OTA:` logs.
- Leave disabled to avoid extra flash usage.
