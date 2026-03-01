# DeviceSetupManager

Legge i dati di provisioning da LittleFS, nella cartella `/provisioning`.

## API

- `begin()`
- `saveDeviceId(deviceId)`
- `saveDeviceSecret(deviceSecret)`
- `deviceId()`
- `deviceSecret()`
- `deviceTypeId()`
- `portalServerIp()`

## File attesi

- `/provisioning/device_id.txt`
- `/provisioning/device_secret.txt`
- `/provisioning/device_type_id.txt`
- `/provisioning/portal_server_ip.txt`

## Esempio

```cpp
#include "DeviceSetupManager.h"

DeviceSetupManager setupMgr;

void setup() {
  setupMgr.begin();
  Serial.println(setupMgr.deviceId());
  Serial.println(setupMgr.deviceTypeId());
}
```
