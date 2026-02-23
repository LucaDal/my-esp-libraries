#include "FirmwareData.h"

FirmwareData::FirmwareData(const char *storagePath)
    : storagePath(storagePath ? storagePath : "/device/fw_version.txt") {
  loadVersion();
}

bool FirmwareData::ensureStorageReady() {
  if (fsReady) {
    return true;
  }
#if defined(ESP32)
  fsReady = LittleFS.begin(true);
#else
  fsReady = LittleFS.begin();
#endif
  if (!fsReady) {
    OTA_LOG("LittleFS begin fail");
    return false;
  }

  int split = storagePath.lastIndexOf('/');
  if (split > 0) {
    String dir = storagePath.substring(0, split);
    if (!LittleFS.exists(dir) && !LittleFS.mkdir(dir)) {
      OTA_LOG("mkdir for fw version fail");
      return false;
    }
  }
  return true;
}

void FirmwareData::loadVersion() {
  newFirmware.version = "0.0.0";

  if (!ensureStorageReady()) {
    return;
  }

  File f = LittleFS.open(storagePath, "r");
  if (!f) {
    OTA_LOG("fw version file missing, using 0.0.0");
    return;
  }

  String version = f.readString();
  f.close();
  version.trim();
  if (!version.isEmpty()) {
    newFirmware.version = version;
  }
}

void FirmwareData::saveVersion(String version) {
  version.trim();
  if (version.isEmpty()) {
    OTA_LOG("skip empty version");
    return;
  }
  if (!ensureStorageReady()) {
    return;
  }

  OTA_LOGF("save %s\n", version.c_str());
  File f = LittleFS.open(storagePath, "w");
  if (!f) {
    OTA_LOG("open fw version file in write fail");
    return;
  }

  f.print(version);
  f.close();
  newFirmware.version = version;
}

void FirmwareData::setNewFirmware(Firmware firmware) {
  if (newFirmware.version == firmware.version) {
    return;
  }
  OTA_LOGF("new %s old %s\n", firmware.version.c_str(),
           newFirmware.version.c_str());
  oldFirmwareVersion = newFirmware.version;
  newFirmware = firmware;
  if (newFirmware.version != "-1") {
    OTA_LOGF("version %s\n", newFirmware.version.c_str());
    OTA_LOGF("md5 %s\n", newFirmware.md5_checksum.c_str());
    hasNewFirmware = true;
  }
}

bool FirmwareData::hasNewUpdate() { return hasNewFirmware; }

String FirmwareData::getFirmwareMD5Image() { return newFirmware.md5_checksum; }

String FirmwareData::getNewFirmwareVersion() { return newFirmware.version; }

String FirmwareData::getOldFirmwareVersion() { return oldFirmwareVersion; }
