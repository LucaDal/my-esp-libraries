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
  const char *deviceId() const;
  const char *deviceTypeId() const;
  const char *portalServerIp() const;

private:
  String _provisioningDir;
  bool _begun{false};
  String _deviceId;
  String _deviceTypeId;
  String _portalServerIp;
  static constexpr const char *_deviceIdFileName = "device_id.txt";
  static constexpr const char *_deviceTypeIdFileName = "device_type_id.txt";
  static constexpr const char *_portalServerIpFileName = "portal_server_ip.txt";

  bool saveProvisioningValue(const char *fileName, const char *value);
  String loadProvisioningValue(const char *fileName) const;
};

#endif
