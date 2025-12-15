#ifndef SIMLPEOTA_H
#define SIMLPEOTA_H

//to activate serial.Print()
//#define DEBUG

#include <Arduino.h>
#include "FirmwareData.h"
#include "Network.h"
#include "CommonOta.h"

class SimpleOTA {
private:
  FirmwareData *version;
  Network *network;
  unsigned long t1;
  const char * API_KEY;
  bool isInit = false;

  void initVersion(int EEPROMSize);
  void initNetwork(const char * base_url, bool useTLS);
  bool serverFirmwareCheck();
  bool startDownload();
  void init(int EEPROMSize, const char * API_KEY);
  //void updateFirmware();

public:
  SimpleOTA();
  void begin(int EEPROMSize,const char * server_address, const char * API_KEY, bool verifyCert);
  bool checkUpdates(unsigned long seconds);
};

#endif
