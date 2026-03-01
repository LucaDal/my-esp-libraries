#include "SimpleOTA.h"

static SimpleOTA *instance = NULL;

SimpleOTA::SimpleOTA() {
  OTA_LOG("start client");
  instance = this;
}

void SimpleOTA::init(const char *deviceTypeId, const char *deviceId,
                     const char *deviceSecret) {
  this->initVersion();
  this->deviceTypeId = deviceTypeId ? deviceTypeId : "";
  this->deviceId = deviceId ? deviceId : "";
  this->deviceSecret = deviceSecret ? deviceSecret : "";
  this->isInit = true;
  checkUpdates(0);
}

void SimpleOTA::begin(const char *server_address, const char *deviceTypeId,
                      const char *deviceId, const char *deviceSecret,
                      bool verifyCert) {
  this->initNetwork(server_address, verifyCert);
  init(deviceTypeId, deviceId, deviceSecret);
}

bool SimpleOTA::checkUpdates(unsigned long seconds) {
  if (!this->isInit)
    return false;
  if (millis() - t1 >= seconds * 1000) {
    OTA_LOG("checking update");
    t1 = millis();
    if (network->isConnected())
      return this->serverFirmwareCheck();
  }
  return true;
}

void SimpleOTA::initVersion() {
  version = new FirmwareData();
  OTA_LOGF("current version %s\n", version->getNewFirmwareVersion().c_str());
}

void SimpleOTA::initNetwork(const char *base_url, bool useTLS) {
  OTA_LOG("init network");
  network = new Network(base_url, useTLS);
  network->WiFiBegin();
}

bool SimpleOTA::startDownload() {
  if (network->fileDownload(deviceTypeId, deviceId, deviceSecret,
                            version->getFirmwareMD5Image(),
                            version->getOldFirmwareVersion())) {
    OTA_LOG("saving new version to storage");
    version->saveVersion(version->getNewFirmwareVersion());
    OTA_LOG("restarting board");
    delay(1000);
    ESP.restart();
  }
  return false;
}

bool SimpleOTA::serverFirmwareCheck() {
  version->setNewFirmware(network->checkVersion(deviceTypeId, deviceSecret));
  if (version->getNewFirmwareVersion() == "-1") {
    OTA_LOG("server not responding");
    return false;
  } else {
    if (version->hasNewUpdate()) {
      OTA_LOG("new build available, start download");
      return startDownload();
    } else {
      OTA_LOG("version up to date");
    }
    return true;
  }
}
