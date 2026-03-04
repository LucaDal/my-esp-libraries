# MQTTManager

Wrapper per `PubSubClient` con TLS mutuale da certificati provisionati in LittleFS.

## File attesi

- `/provisioning/ca.crt`
- `/provisioning/client.crt`
- `/provisioning/client.key`

## API

- `begin(host, port, callback, keepAliveSeconds)`
- `setTimezone("TZ string POSIX")`
- `connect(clientId, username, password)`
- `subscribe(topic)`
- `publish(topic, payload, retained)`
- `connected()`
- `loop()`
- `state()`
- `client()`

## Esempio

```cpp
#include "MQTTManager.h"

MQTTManager mqtt;

void mqttCallback(char *topic, uint8_t *payload, unsigned int length) {
}

void setup() {
  mqtt.setTimezone("CET-1CEST,M3.5.0/2,M10.5.0/3"); // opzionale, default Roma
  // mqtt.setTimezone("UTC0"); // se vuoi UTC puro
  mqtt.begin("broker.example.com", 8883, mqttCallback);
  mqtt.connect("device-01");
  mqtt.subscribe("lab/topic");
}

void loop() {
  mqtt.loop();
}
```

## Note

- Su ESP8266 usa BearSSL con buffer TLS `512 x 512`.
- Se `username` e vuoto, `connect(...)` usa solo `clientId`.
- Con `USE_TLS` attivo, prima di configurare TLS viene eseguita `TimeSyncManager::ensureTimeSynced(...)`.
- Con `USE_TLS` disattivo, il client MQTT lavora in plaintext (nessun certificato richiesto).
