#include "Network.h"

Network::Network(const char *base_url, bool verifyCert)
    : verifyCertEnabled(verifyCert) {
#ifdef USE_TLS
  this->BASE_URL = String("https://");
  #ifdef ESP8266
    this->trustedRoots.append(cert_ISRG_X1);
    this->trustedRoots.append(cert_ISRG_X2);
    if (verifyCertEnabled) {
      this->client->setTrustAnchors(&trustedRoots);
      this->client->setSSLVersion(BR_TLS12, BR_TLS12);
    } else {
      this->client->setInsecure();
    }
    // reduce memory so another instance of clientSecure can be defined
    this->client->setBufferSizes(512, 264);
  #endif
  #ifdef ESP32
    if (verifyCertEnabled) {
      this->client->setCACert(cert_ISRG_X1);
    } else {
      this->client->setInsecure();
    }
  #endif
#else
  this->BASE_URL = String("http://");
#endif
  this->BASE_URL += String(base_url);
}

void Network::WiFiBegin() {
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
}

bool Network::isConnected() { return WiFi.status() == WL_CONNECTED; }

bool Network::startConnectionTo(const String &path) {
#if defined(USE_TLS)
  if (verifyCertEnabled && !timeSync.ensureTimeSynced(5000, 100)) {
    OTA_LOG("time sync failed, cannot validate TLS certificate");
    return false;
  }
#endif

  bool http_connected = false;
  String targetURL = this->BASE_URL + path;
  OTA_LOG(targetURL.c_str());
  http_connected = httpClient.begin(*client, targetURL);

  return http_connected;
}

void Network::addVersionHeaders(const String &deviceTypeId,
                                const String &deviceSecret) {
  httpClient.addHeader("x-device-type-id", deviceTypeId);
  httpClient.addHeader("x-device-secret", deviceSecret);
}

void Network::addBuildHeaders(const String &deviceTypeId,
                              const String &deviceId,
                              const String &deviceSecret) {
  httpClient.addHeader("x-device-type-id", deviceTypeId);
  httpClient.addHeader("x-device-code", deviceId);
  httpClient.addHeader("x-device-secret", deviceSecret);
}

Firmware Network::checkVersion(const String &deviceTypeId,
                               const String &deviceSecret) {

  Firmware firmware;
  firmware.version = "-1";
  OTA_LOG("checking version");
  if (isConnected()) {

    bool http_connected = startConnectionTo("/ota/version");

    if (http_connected) {
      OTA_LOG("connected");
      addVersionHeaders(deviceTypeId, deviceSecret);
      int httpCode = httpClient.GET();
      if (httpCode == HTTP_CODE_OK) {
        String payload = httpClient.getString();
        OTA_LOGF("payload %s\n", payload.c_str());
        DeserializationError error = deserializeJson(doc, payload);
        if (error) {
          OTA_LOG("json parse fail");
          return firmware;
        }
        firmware.version = doc["version"].as<String>();
        firmware.md5_checksum = doc["md5Checksum"].as<String>();
      } else {
        OTA_LOGF("get version fail %s\n",
                 httpClient.errorToString(httpCode).c_str());
      }
      httpClient.end();
    } else {
      OTA_LOG("unable to connect");
    }
  }
  return firmware;
}

bool Network::fileDownload(const String &deviceTypeId, const String &deviceId,
                           const String &deviceSecret, String md5Checksum,
                           String currentVersion) {

  if (isConnected()) {
    MyUpdater update = MyUpdater(md5Checksum);
    bool http_connected = startConnectionTo("/ota/build");

    if (http_connected) {
      OTA_LOG("connected, start download");
      addBuildHeaders(deviceTypeId, deviceId, deviceSecret);
      bool return_value = update.startUpdate(this->httpClient, currentVersion);
      httpClient.end();
      return return_value;
    }
  }
  return false;
}
