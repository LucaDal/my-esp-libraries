#pragma once

#include <Arduino.h>
#include <CommonDebug.h>
#include <LittleFS.h>
#include <PubSubClient.h>
#include <memory>

#ifdef ESP8266
#include <BearSSLHelpers.h>
#include <WiFiClientSecureBearSSL.h>
using MQTTSecureClient = BearSSL::WiFiClientSecure;
#else
#include <WiFiClientSecure.h>
using MQTTSecureClient = WiFiClientSecure;
#endif

using MQTTMessageCallback = void (*)(char *topic, uint8_t *payload,
                                     unsigned int length);

class MQTTManager {
public:
  explicit MQTTManager(const char *provisioningDir = "/provisioning");

  bool begin(const char *host, uint16_t port,
             MQTTMessageCallback callback = nullptr,
             int keepAliveSeconds = 60);
  void setCallback(MQTTMessageCallback callback);
  void setKeepAlive(uint16_t keepAliveSeconds);

  bool connect(const char *clientId, const char *username = nullptr,
               const char *password = nullptr);
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
  MQTTSecureClient secureClient;
  PubSubClient mqttClient;

#ifdef ESP8266
  std::unique_ptr<BearSSL::X509List> caList;
  std::unique_ptr<BearSSL::X509List> clientCertList;
  std::unique_ptr<BearSSL::PrivateKey> clientKey;
#endif
};
