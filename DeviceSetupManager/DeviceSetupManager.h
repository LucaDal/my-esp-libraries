#ifndef DEVICE_SETUP_MANAGER_H
#define DEVICE_SETUP_MANAGER_H

#include <Arduino.h>
#include <CommonDebug.h>
#include <LittleFS.h>

#define DSM_LOG(msg) DBG_LOG("*DSM:", msg)
#define DSM_LOGF(fmt, ...) DBG_LOGF("*DSM:", fmt, ##__VA_ARGS__)

class DeviceSetupManager {
public:
  explicit DeviceSetupManager(const char *provisioningDir = "/provisioning");

  bool begin();
  bool saveDeviceId(const char *deviceId);
  String readDeviceId() const;

private:
  String _provisioningDir;
  bool _begun{false};
  static constexpr const char *_deviceIdFileName = "device_id.txt";
};

#endif
