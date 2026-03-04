#pragma once

#include <Arduino.h>
#include <CommonDebug.h>
#include <LittleFS.h>
#include <PubSubClient.h>
#include <TimeSyncManager.h>
#include <memory>

#ifdef USE_TLS
#ifdef ESP8266
#include <BearSSLHelpers.h>
#include <WiFiClientSecureBearSSL.h>
using MQTTTransportClient = BearSSL::WiFiClientSecure;
#else
#include <WiFiClientSecure.h>
using MQTTTransportClient = WiFiClientSecure;
#endif
#else
#include <WiFiClient.h>
using MQTTTransportClient = WiFiClient;
#endif

using MQTTMessageCallback = void (*)(char *topic, uint8_t *payload,
                                     unsigned int length);

class MQTTManager {
public:
  explicit MQTTManager(const char *provisioningDir = "/provisioning");

  bool begin(const char *host, uint16_t port,
             MQTTMessageCallback callback = nullptr, int keepAliveSeconds = 60);
  void setTimezone(const char *tzValue);
  void setCallback(MQTTMessageCallback callback);
  void setKeepAlive(uint16_t keepAliveSeconds);

  bool connect(const char *clientId, const char *username = nullptr,
               const char *password = nullptr);
  void disconnect();
  bool subscribe(const char *topic);
  bool publish(const char *topic, const char *payload, bool retained = false);
  bool connected();
  bool loop();
  int state();

  PubSubClient &client();

private:
  bool ensureFsReady();
  bool loadCertificates();
  bool readPemFile(const char *fileName, String &out);
  bool configureSecureClient();

  String provisioningDir;
  bool fsReady{false};
  String caPem;
  String clientCertPem;
  String clientKeyPem;
  TimeSyncManager timeSync;
  MQTTTransportClient transportClient;
  PubSubClient mqttClient;

#if defined(USE_TLS) && defined(ESP8266)
  std::unique_ptr<BearSSL::X509List> caList;
  std::unique_ptr<BearSSL::X509List> clientCertList;
  std::unique_ptr<BearSSL::PrivateKey> clientKey;
#endif
};
