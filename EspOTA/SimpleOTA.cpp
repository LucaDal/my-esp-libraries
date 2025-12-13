#include "SimpleOTA.h"

static SimpleOTA* instance = NULL;

SimpleOTA::SimpleOTA() {
  OTA_LOG("start client");
  t1 = 0;
  instance = this;
}


void SimpleOTA::init(int EEPROMSize, const char* API_KEY) {
  this->initVersion(EEPROMSize);
  this->API_KEY = API_KEY;
  this->isInit = true;
  checkUpdates(0);
}

/**
 * @brief initialize OTA class and check updates.
 * Firmware data will be saved to the last EEPROM address;
 * address_IP is the DNS or IP address, do NOT pass https or http type;
 * verifyCert is only used if flag USE_TLS in platformio.ini is set;
 */
void SimpleOTA::begin(int EEPROMSize, const char* server_address, const char* API_KEY, bool verifyCert = true) {
  this->initNetwork(server_address, verifyCert);
  init(EEPROMSize, API_KEY);
}

/**
 * @brief return false if something goes wrong
 * called from the main thread
 */
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

void SimpleOTA::initVersion(int EEPROMSize) {
  version = new FirmwareData(EEPROMSize);
  OTA_LOGF("current version %s\n", version->getNewFirmwareVersion().c_str());
}

void SimpleOTA::initNetwork(const char* base_url, bool useTLS) {
  OTA_LOG("init network");
  network = new Network(base_url, useTLS);
  network->WiFiBegin();
}

bool SimpleOTA::startDownload() {
  if (network->fileDownload(API_KEY, version->getFirmwareMD5Image(), version->getOldFirmwareVersion())) {
    OTA_LOG("saving new version to EEPROM");
    version->saveVersion(version->getNewFirmwareVersion());//save only if update goes fine
    OTA_LOG("restarting board");
    delay(1000); // Wait a second and restart
    ESP.restart();
  }
  return false;
}
/**
 * return false if failed, true if no update found.
 */
bool SimpleOTA::serverFirmwareCheck() {
  version->setNewFirmware(network->checkVersion(API_KEY));
  if (version->getNewFirmwareVersion() == "-1") {
    OTA_LOG("server not responding");
    return false;
  }
  else {
    if (version->hasNewUpdate()) {
      OTA_LOG("new build available, start download");
      return startDownload();
    }
    else {
      OTA_LOG("version up to date");
    }
    return true;
  }
}

