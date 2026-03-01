#ifndef MY_DEVICE_PROPERTIES_H
#define MY_DEVICE_PROPERTIES_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

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
#include <BearSSLHelpers.h>
#include <WiFiClientSecureBearSSL.h>
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
  explicit MyDeviceProperties(bool verifyCert = true,
                              const char *storagePath = "/device/properties.json");

  bool begin(const char *serverAddress, const char *deviceId,
             const char *deviceSecret);
  bool fetchAndStoreIfChanged();
  bool loadFromStorage();
  const char *Get(const char *key, const char *defaultValue = "") const;
  int GetInt(const char *key, int defaultValue = 0) const;
  bool GetBool(const char *key, bool defaultValue = false) const;
  float GetFloat(const char *key, float defaultValue = 0.0f) const;
  JsonDocument &json();

private:
  bool ensureStorageReady();
  String buildUrl() const;
  void addDeviceHeaders(HTTPClient &http) const;
  bool saveToStorage(const String &payload);

  String serverAddress;
  String deviceId;
  String deviceSecret;
  String storagePath;
  bool verifyCert;
  bool fsReady{false};
  JsonDocument doc;
  String cachedPayload;
#ifdef USE_TLS
#if defined(USE_TLS_CERTS) && defined(ESP8266)
  BearSSL::X509List trustedRoots;
#endif
#endif
  std::unique_ptr<NetClient> client{new NetClient};
};

#endif
