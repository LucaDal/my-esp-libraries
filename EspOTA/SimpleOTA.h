#ifndef SIMLPEOTA_H
#define SIMLPEOTA_H

#include <Arduino.h>

#include "CommonOta.h"
#include "FirmwareData.h"
#include "Network.h"

class SimpleOTA {
private:
  FirmwareData *version{nullptr};
  Network *network{nullptr};
  unsigned long t1{0};
  const char *API_KEY{nullptr};
  bool isInit{false};

  void initVersion();
  void initNetwork(const char *base_url, bool useTLS);
  bool serverFirmwareCheck();
  bool startDownload();
  void init(const char *API_KEY);

public:
  SimpleOTA();
  void begin(const char *server_address, const char *API_KEY, bool verifyCert);
  bool checkUpdates(unsigned long seconds);
};

#endif
