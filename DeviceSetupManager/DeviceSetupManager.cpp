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

  _deviceId = loadProvisioningValue(_deviceIdFileName);
  _deviceSecret = loadProvisioningValue(_deviceSecretFileName);
  _deviceTypeId = loadProvisioningValue(_deviceTypeIdFileName);
  _portalServerIp = loadProvisioningValue(_portalServerIpFileName);
  _begun = true;
  DSM_LOGF("LittleFS ready, provisioning dir=%s\n", _provisioningDir.c_str());
  DSM_LOGF("DEVICE ID [%s]\n", deviceId());
  DSM_LOGF("DEVICE SECRET [%s]\n", strlen(deviceSecret()) > 0 ? "set" : "missing");
  DSM_LOGF("DEVICE TYPE ID [%s]\n", deviceTypeId());
  DSM_LOGF("PORTAL SERVER [%s]\n", portalServerIp());
  return true;
}

bool DeviceSetupManager::saveDeviceId(const char *deviceId) {
  bool ok = saveProvisioningValue(_deviceIdFileName, deviceId);
  if (ok) {
    _deviceId = deviceId;
  }
  return ok;
}

bool DeviceSetupManager::saveDeviceSecret(const char *deviceSecret) {
  bool ok = saveProvisioningValue(_deviceSecretFileName, deviceSecret);
  if (ok) {
    _deviceSecret = deviceSecret;
  }
  return ok;
}

const char *DeviceSetupManager::deviceId() const {
  return _deviceId.c_str();
}

const char *DeviceSetupManager::deviceSecret() const {
  return _deviceSecret.c_str();
}

const char *DeviceSetupManager::deviceTypeId() const {
  return _deviceTypeId.c_str();
}

const char *DeviceSetupManager::portalServerIp() const {
  return _portalServerIp.c_str();
}

bool DeviceSetupManager::isProvisioningReady() const {
  return _begun &&
         !_deviceId.isEmpty() &&
         !_deviceSecret.isEmpty() &&
         !_deviceTypeId.isEmpty() &&
         !_portalServerIp.isEmpty();
}

bool DeviceSetupManager::saveProvisioningValue(const char *fileName,
                                               const char *value) {
  if (!begin()) {
    return false;
  }
  if (!fileName || strlen(fileName) == 0 || !value || strlen(value) == 0) {
    DSM_LOG("invalid provisioning value");
    return false;
  }

  String path = _provisioningDir + "/" + fileName;
  File f = LittleFS.open(path, "w");
  if (!f) {
    DSM_LOG("open provisioning file failed");
    return false;
  }
  if (!f.print(value)) {
    f.close();
    DSM_LOG("write provisioning value failed");
    return false;
  }
  f.close();
  DSM_LOGF("provisioning value saved: %s=%s\n", fileName, value);
  return true;
}

String DeviceSetupManager::loadProvisioningValue(const char *fileName) const {
  if (!fileName || strlen(fileName) == 0) {
    return "";
  }

  String path = _provisioningDir + "/" + fileName;
  File f = LittleFS.open(path, "r");
  if (!f) {
    DSM_LOGF("provisioning file not found: %s\n", fileName);
    return "";
  }
  String value = f.readStringUntil('\n');
  f.close();
  value.trim();
  DSM_LOGF("provisioning value loaded: %s=%s\n", fileName, value.c_str());
  return value;
}
