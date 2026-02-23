#ifndef FIRMWAREDATA_H
#define FIRMWAREDATA_H

#include <Arduino.h>
#include <LittleFS.h>

#include "CommonOta.h"
#include "MyFirmware.h"

class FirmwareData {
private:
  Firmware newFirmware;
  String oldFirmwareVersion = "";
  bool hasNewFirmware{false};
  String storagePath;
  bool fsReady{false};

  bool ensureStorageReady();
  void loadVersion();

public:
  explicit FirmwareData(const char *storagePath = "/device/fw_version.txt");
  void saveVersion(String version);
  void setNewFirmware(Firmware new_firmware_version);
  bool hasNewUpdate();
  String getFirmwareMD5Image();
  String getNewFirmwareVersion();
  String getOldFirmwareVersion();
};

#endif
