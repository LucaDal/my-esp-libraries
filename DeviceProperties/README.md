# MyDeviceProperties

Scarica `/ota/properties` dal backend, salva il JSON in LittleFS e lo espone in RAM tramite `ArduinoJson`.

## API

- `begin(serverAddress, deviceId, deviceSecret)`
- `fetchAndStoreIfChanged()`
- `loadFromStorage()`
- `Get(key, defaultValue)`
- `GetInt(key, defaultValue)`
- `GetBool(key, defaultValue)`
- `GetFloat(key, defaultValue)`
- `json()`

## Esempio

```cpp
#include "MyDeviceProperties.h"

MyDeviceProperties props;

void setup() {
  props.begin("example.com/api", "DEVICE-01", "secret");
  props.fetchAndStoreIfChanged();

  const char *topic = props.Get("topic");
  int port = props.GetInt("MQTT_PORT", 8883);
  bool enabled = props.GetBool("enabled", false);
  float lat = props.GetFloat("latitude", 0.0f);
}
```

## Note

- I dati sono salvati in `/device/properties.json`.
- La richiesta usa gli header `x-device-code` e `x-device-secret`.
- Con `USE_TLS` usa HTTPS; senza, usa HTTP.
