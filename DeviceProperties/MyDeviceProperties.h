#ifndef MY_DEVICE_PROPERTIES_H
#define MY_DEVICE_PROPERTIES_H

#include <Arduino.h>
#include <EEPROM.h>
#include <ArduinoJson.h>

#if defined(ESP8266)
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <HTTPClient.h>
#include <WiFi.h>
#endif

#include <CommonDebug.h>

#ifdef USE_TLS
#ifdef ESP8266
    #include <WiFiClientSecureBearSSL.h>
    #include <BearSSLHelpers.h>
  #elif defined(ESP32)
    #include <WiFiClientSecure.h>
  #endif
  #ifdef USE_TLS_CERTS
    #include "Certs.h"
  #endif
#endif

#ifdef USE_TLS
  #ifdef ESP8266
    using NetClient = BearSSL::WiFiClientSecure;
  #elif defined(ESP32)
    using NetClient = WiFiClientSecure;
  #endif
#else
  using NetClient = WiFiClient;
#endif

#define MYPROPS_LOG(msg) DBG_LOG("*MyProps:", msg)
#define MYPROPS_LOGF(fmt, ...) DBG_LOGF("*MyProps:", fmt, ##__VA_ARGS__)

class MyDeviceProperties {
public:
  MyDeviceProperties(size_t eepromSize = 512, size_t eepromOffset = 0,
                     size_t reservedTailBytes = 3, size_t jsonCapacity = 512,
                     bool verifyCert = true);

  bool begin(const char *serverAddress, const char *deviceTypeId);
  bool begin(const char *serverAddress, const char *deviceTypeId,
             size_t eepromOffset);
  bool fetchAndStoreIfChanged();
  bool loadFromEEPROM();
  JsonDocument &json();

private:
  String buildUrl() const;
  bool saveToEEPROM(const String &payload);
  String readPayloadFromEEPROM();
  size_t availableStorage() const;

  String serverAddress;
  String deviceTypeId;
  size_t eepromSize;
  size_t eepromOffset;
  size_t reservedTailBytes;
  bool verifyCert;
  JsonDocument doc;
  String cachedPayload;
#ifdef USE_TLS
  #if defined(USE_TLS_CERTS) && defined(ESP8266)
    BearSSL::X509List trustedRoots;
  #endif
#endif
  std::unique_ptr<NetClient> client { new NetClient };
};

#endif
