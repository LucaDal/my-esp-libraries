#include "Network.h"

Network::Network(const char *base_url, bool verifyCert) {
#ifdef USE_TLS
  this->BASE_URL = String("https://");
  #ifdef ESP8266
    this->trustedRoots.append(cert_ISRG_X1);
    this->trustedRoots.append(cert_ISRG_X2);
    if (verifyCert) {
      this->client->setTrustAnchors(&trustedRoots);
      this->client->setSSLVersion(BR_TLS12, BR_TLS12);
      setClock();
    } else {
      this->client->setInsecure();
    }
    // reduce memory so another instance of clientSecure can be defined
    this->client->setBufferSizes(512, 264);
  #endif
  #ifdef ESP32
    if (verifyCert) {
      this->client->setCACert(cert_ISRG_X1);
    } else {
      this->client->setInsecure();
    }
  #endif
#else
  (void)verifyCert;
  this->BASE_URL = String("http://");
#endif
  this->BASE_URL += String(base_url);
}

void Network::setClock() {
#ifdef ESP8266
  configTime("Europe/Rome", "europe.pool.ntp.org");
#endif
#ifdef DEBUG
  OTA_LOGF("%lu ntp sync\n", millis());
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(250);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.print(F("\r\n"));
  struct tm timeinfo; // NOLINT(cppcoreguidelines-pro-type-member-init)
  gmtime_r(&now, &timeinfo);
  OTA_LOGF("utc %s", asctime(&timeinfo));
  localtime_r(&now, &timeinfo);
  OTA_LOGF("local %s", asctime(&timeinfo));
#endif
}

void Network::WiFiBegin() {
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
}

bool Network::isConnected() { return WiFi.status() == WL_CONNECTED; }

bool Network::startConnectionTo(const char *server_api_address, String api_key,
                                String path) {
  bool http_connected = false;
  String targetURL = this->BASE_URL + server_api_address + api_key + path;
  OTA_LOG(targetURL.c_str());
  http_connected = httpClient.begin(*client, targetURL);

  return http_connected;
}

Firmware Network::checkVersion(String api_key) {

  Firmware firmware;
  firmware.version = "-1";
  OTA_LOG("checking version");
  if (isConnected()) {

    bool http_connected = startConnectionTo("/ota/", api_key, "/version");

    if (http_connected) {
      OTA_LOG("connected");
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

bool Network::fileDownload(String api_key, String md5Checksum,
                           String currentVersion) {

  if (isConnected()) {
    MyUpdater update = MyUpdater(md5Checksum);
    bool http_connected = startConnectionTo("/ota/", api_key, "/build");

    if (http_connected) {
      OTA_LOG("connected, start download");
      bool return_value = update.startUpdate(this->httpClient, currentVersion);
      httpClient.end();
      return return_value;
    }
  }
  return false;
}
