#ifndef NETWORK_H
#define NETWORK_H

#include <ArduinoJson.h>
#include "MyUpdater.h"
#include "MyFirmware.h"
#include "OtaDebug.h"

#ifdef ESP8266
  #include <ESP8266WiFi.h>
  #include <ESP8266HTTPClient.h>
  #include <WiFiClientSecureBearSSL.h>
  #include <BearSSLHelpers.h>
#elif defined(ESP32)
  #include <WiFi.h>
  #include <WiFiClientSecure.h>
  #include <HTTPClient.h>
#endif

#ifdef USE_TLS
  #include "Certs.h"
  #ifdef ESP8266
    using NetClient = BearSSL::WiFiClientSecure;
  #elif defined(ESP32)
    using NetClient = WiFiClientSecure;
  #endif
#else
  #ifdef ESP8266
    #include <WiFiClient.h>
  #endif
  using NetClient = WiFiClient;
#endif

class Network {
private:
  HTTPClient httpClient;

  std::unique_ptr<NetClient> client { new NetClient };

  #if defined(ESP8266) && defined(USE_TLS)
    BearSSL::X509List trustedRoots;
  #endif
  JsonDocument doc;
  String BASE_URL;
  void setClock();
  bool startConnectionTo(const char *server_api_address, String api_key, String path);

public:
  // verifyCert: true = validate CA (when TLS is compiled in), false = setInsecure()
  Network(const char *base_url, bool verifyCert);
  void WiFiBegin();
  bool isConnected();
  Firmware checkVersion(String api_key);
  bool fileDownload(String api_key, String md5Checksum, String currentVersion);
};

#endif // NETWORK_H
