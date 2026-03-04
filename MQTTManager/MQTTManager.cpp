#include "MQTTManager.h"

#define MQTTM_LOG(msg) DBG_LOG("*MQTTM:", msg)
#define MQTTM_LOGF(fmt, ...) DBG_LOGF("*MQTTM:", fmt, ##__VA_ARGS__)

MQTTManager::MQTTManager(const char *provisioningDir)
    : provisioningDir(provisioningDir ? provisioningDir : "/provisioning"),
      mqttClient(transportClient) {}

void MQTTManager::setTimezone(const char *tzValue) {
  timeSync.setTimezone(tzValue);
}

bool MQTTManager::begin(const char *host, uint16_t port,
                        MQTTMessageCallback callback, int keepAliveSeconds) {

    MQTTM_LOGF("Begin host=%s port=%u\n", host, static_cast<unsigned>(port));
  if (!host || strlen(host) == 0 || port == 0) {
    MQTTM_LOG("Invalid MQTT host/port");
    return false;
  }
  if (!configureSecureClient()) {
    return false;
  }

  mqttClient.setServer(host, port);
  if (callback != nullptr) {
    mqttClient.setCallback(callback);
  }
  if (keepAliveSeconds >= 0) {
    mqttClient.setKeepAlive(static_cast<uint16_t>(keepAliveSeconds));
  }
  return true;
}

void MQTTManager::setCallback(MQTTMessageCallback callback) {
  if (callback != nullptr) {
    mqttClient.setCallback(callback);
  }
}

void MQTTManager::setKeepAlive(uint16_t keepAliveSeconds) {
  mqttClient.setKeepAlive(keepAliveSeconds);
}

bool MQTTManager::connect(const char *clientId, const char *username,
                          const char *password) {
  if (!clientId || strlen(clientId) == 0) {
    MQTTM_LOG("Invalid MQTT client ID");
    return false;
  }

  if (username && strlen(username) > 0) {
    return mqttClient.connect(clientId, username, password ? password : "");
  }
  return mqttClient.connect(clientId);
}

void MQTTManager::disconnect() {
    mqttClient.disconnect();
}

bool MQTTManager::subscribe(const char *topic) {
  if (!topic || strlen(topic) == 0) {
    return false;
  }
  return mqttClient.subscribe(topic);
}

bool MQTTManager::publish(const char *topic, const char *payload,
                          bool retained) {
  if (!topic || strlen(topic) == 0) {
    return false;
  }
  return mqttClient.publish(topic, payload ? payload : "", retained);
}

bool MQTTManager::connected() { return mqttClient.connected(); }

bool MQTTManager::loop() { return mqttClient.loop(); }

int MQTTManager::state() { return mqttClient.state(); }

PubSubClient &MQTTManager::client() { return mqttClient; }

bool MQTTManager::ensureFsReady() {
  if (fsReady) {
    return true;
  }

  fsReady = LittleFS.begin();
  if (!fsReady) {
    MQTTM_LOG("LittleFS begin failed");
    return false;
  }
  return true;
}

bool MQTTManager::loadCertificates() {
  if (!ensureFsReady()) {
    return false;
  }
  if (!caPem.isEmpty() && !clientCertPem.isEmpty() && !clientKeyPem.isEmpty()) {
    return true;
  }

  if (!readPemFile("ca.crt", caPem) || !readPemFile("client.crt", clientCertPem) ||
      !readPemFile("client.key", clientKeyPem)) {
    return false;
  }

#if defined(USE_TLS) && defined(ESP8266)
  caList.reset(new BearSSL::X509List(caPem.c_str()));
  clientCertList.reset(new BearSSL::X509List(clientCertPem.c_str()));
  clientKey.reset(new BearSSL::PrivateKey(clientKeyPem.c_str()));
  if (!caList || !clientCertList || !clientKey) {
    MQTTM_LOG("Unable to allocate BearSSL certificate objects");
    return false;
  }
#endif

  return true;
}

bool MQTTManager::readPemFile(const char *fileName, String &out) {
  String path = provisioningDir + "/" + fileName;
  File f = LittleFS.open(path, "r");
  if (!f) {
    MQTTM_LOGF("Missing provisioning file: %s\n", path.c_str());
    return false;
  }

  out = f.readString();
  f.close();
  out.trim();
  if (out.isEmpty()) {
    MQTTM_LOGF("Empty provisioning file: %s\n", path.c_str());
    return false;
  }
  out += "\n";
  return true;
}

bool MQTTManager::configureSecureClient() {
#ifdef USE_TLS
  if (!timeSync.ensureTimeSynced(5000, 100)) {
    MQTTM_LOG("Time sync failed: TLS certificate validation unavailable");
    return false;
  }

  if (!loadCertificates()) {
    return false;
  }

#ifdef ESP8266
  transportClient.setTrustAnchors(caList.get());
  transportClient.setClientECCert(clientCertList.get(), clientKey.get(),
                               BR_KEYTYPE_KEYX | BR_KEYTYPE_SIGN,
                               BR_KEYTYPE_EC);
  transportClient.setBufferSizes(512, 512);
#else
  transportClient.setCACert(caPem.c_str());
  transportClient.setCertificate(clientCertPem.c_str());
  transportClient.setPrivateKey(clientKeyPem.c_str());
#endif

  MQTTM_LOG("TLS certificates loaded for MQTT");
#else
  MQTTM_LOG("USE_TLS disabled: MQTT client configured in plaintext mode");
#endif
  return true;
}
