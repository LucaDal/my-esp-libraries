#include "DeviceSetupManager.h"

DeviceSetupManager::DeviceSetupManager(const char *provisioningDir)
    : _provisioningDir(provisioningDir ? provisioningDir : "/provisioning") {}

bool DeviceSetupManager::begin() {
  if (_begun) {
    return true;
  }

#if defined(ESP32)
  bool ok = LittleFS.begin(true);
#else
  bool ok = LittleFS.begin();
#endif
  if (!ok) {
    DSM_LOG("LittleFS begin failed");
    return false;
  }

  if (!LittleFS.exists(_provisioningDir)) {
    if (!LittleFS.mkdir(_provisioningDir)) {
      DSM_LOG("mkdir provisioning dir failed");
      return false;
    }
  }

  _begun = true;
  DSM_LOGF("LittleFS ready, provisioning dir=%s\n", _provisioningDir.c_str());
  return true;
}

bool DeviceSetupManager::saveDeviceId(const char *deviceId) {
  if (!begin()) {
    return false;
  }
  if (!deviceId || strlen(deviceId) == 0) {
    DSM_LOG("device id empty");
    return false;
  }

  String path = _provisioningDir + "/" + _deviceIdFileName;
  File f = LittleFS.open(path, "w");
  if (!f) {
    DSM_LOG("open device id file failed");
    return false;
  }
  if (!f.print(deviceId)) {
    f.close();
    DSM_LOG("write device id failed");
    return false;
  }
  f.close();
  DSM_LOGF("device id saved: %s\n", deviceId);
  return true;
}

String DeviceSetupManager::readDeviceId() const {
  if (!_begun) {
    return "";
  }
  String path = _provisioningDir + "/" + _deviceIdFileName;
  File f = LittleFS.open(path, "r");
  if (!f) {
    DSM_LOG("device id file not found");
    return "";
  }
  String deviceId = f.readStringUntil('\n');
  f.close();
  deviceId.trim();
  DSM_LOGF("device id loaded: %s\n", deviceId.c_str());
  return deviceId;
}
