# LiteWiFiManager

Provisioning Wi-Fi minimale per ESP8266/ESP32. Prova prima le credenziali gia salvate nel core Wi-Fi; se fallisce, apre un access point con una pagina web di configurazione.

## API

- `begin(apSsid, apPassword, timeoutMs, forcePortal)`
- `loop()`
- `isPortalActive()`
- `isConnected()`

## Esempio

```cpp
#include "LiteWiFiManager.h"

LiteWiFiManager wifiProvision;

void setup() {
  wifiProvision.begin("Weather Display");
}

void loop() {
  wifiProvision.loop();
}
```
