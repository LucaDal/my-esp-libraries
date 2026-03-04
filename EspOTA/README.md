# EspOTA

Helper OTA per ESP8266/ESP32. Controlla la disponibilita di nuovo firmware sul backend e, se presente, lo scarica.

## API

- `begin(serverAddress, deviceTypeId, deviceId, deviceSecret, verifyCert)`
- `checkUpdates(seconds)`

## Esempio

```cpp
#include "SimpleOTA.h"

SimpleOTA ota;

void setup() {
  ota.begin("example.com/api", "THERMO", "DEVICE-01", "secret", true);
}

void loop() {
  ota.checkUpdates(3600);
}
```

## Note

- La versione firmware e salvata in `/device/fw_version.txt`.
- Con `USE_TLS` usa HTTPS; senza, usa HTTP.
- Con verifica certificati attiva, sincronizza l'ora via `TimeSyncManager` prima delle richieste HTTPS.
