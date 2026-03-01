#include "MyDeviceProperties.h"

MyDeviceProperties::MyDeviceProperties(bool verifyCert, const char *storagePath)
    : storagePath(storagePath ? storagePath : "/device/properties.json"),
      verifyCert(verifyCert) {
#ifdef USE_TLS
#ifdef ESP8266
#ifdef USE_TLS_CERTS
  trustedRoots.append(cert_ISRG_X1);
  trustedRoots.append(cert_ISRG_X2);
  if (verifyCert) {
    client->setTrustAnchors(&trustedRoots);
    client->setSSLVersion(BR_TLS12, BR_TLS12);
  } else {
    client->setInsecure();
  }
#else
  client->setInsecure();
#endif
#elif defined(ESP32)
#ifdef USE_TLS_CERTS
  if (verifyCert) {
    client->setCACert(cert_ISRG_X1);
  } else {
    client->setInsecure();
  }
#else
  client->setInsecure();
#endif
#endif
#endif
}

bool MyDeviceProperties::ensureStorageReady() {
  if (fsReady) {
    return true;
  }
#if defined(ESP32)
  fsReady = LittleFS.begin(true);
#else
  fsReady = LittleFS.begin();
#endif
  if (!fsReady) {
    MYPROPS_LOG("LittleFS begin fail");
    return false;
  }

  int split = storagePath.lastIndexOf('/');
  if (split > 0) {
    String dir = storagePath.substring(0, split);
    if (!LittleFS.exists(dir) && !LittleFS.mkdir(dir)) {
      MYPROPS_LOG("mkdir for storage fail");
      return false;
    }
  }
  return true;
}

bool MyDeviceProperties::begin(const char *serverAddress,
                               const char *deviceId,
                               const char *deviceSecret) {
  this->serverAddress = serverAddress ? serverAddress : "";
  this->deviceId = deviceId ? deviceId : "";
  this->deviceSecret = deviceSecret ? deviceSecret : "";
  MYPROPS_LOGF("begin server=%s device=%s path=%s\n", this->serverAddress.c_str(),
               this->deviceId.c_str(), storagePath.c_str());
  return ensureStorageReady() && loadFromStorage();
}

const char *MyDeviceProperties::Get(const char *key,
                                    const char *defaultValue) const {
  const char *fallback = defaultValue ? defaultValue : "";
  if (!key || key[0] == '\0') {
    return fallback;
  }

  JsonVariantConst value = doc[key];
  if (value.isNull()) {
    return fallback;
  }

  const char *result = value.as<const char *>();
  return result ? result : fallback;
}

int MyDeviceProperties::GetInt(const char *key, int defaultValue) const {
  if (!key || key[0] == '\0') {
    return defaultValue;
  }

  JsonVariantConst value = doc[key];
  if (value.isNull()) {
    return defaultValue;
  }

  int result = value.as<int>();
  return value.is<int>() ? result : defaultValue;
}

bool MyDeviceProperties::GetBool(const char *key, bool defaultValue) const {
  if (!key || key[0] == '\0') {
    return defaultValue;
  }

  JsonVariantConst value = doc[key];
  if (value.isNull()) {
    return defaultValue;
  }

  bool result = value.as<bool>();
  return value.is<bool>() ? result : defaultValue;
}

float MyDeviceProperties::GetFloat(const char *key, float defaultValue) const {
  if (!key || key[0] == '\0') {
    return defaultValue;
  }

  JsonVariantConst value = doc[key];
  if (value.isNull()) {
    return defaultValue;
  }

  float result = value.as<float>();
  return value.is<float>() || value.is<int>() ? result : defaultValue;
}

JsonDocument &MyDeviceProperties::json() { return doc; }

String MyDeviceProperties::buildUrl() const {
  String protocol = "http://";
#ifdef USE_TLS
  protocol = "https://";
#endif
  return protocol + serverAddress + "/ota/properties";
}

void MyDeviceProperties::addDeviceHeaders(HTTPClient &http) const {
  http.addHeader("x-device-code", deviceId);
  http.addHeader("x-device-secret", deviceSecret);
}

bool MyDeviceProperties::saveToStorage(const String &payload) {
  if (!ensureStorageReady()) {
    return false;
  }

  File f = LittleFS.open(storagePath, "w");
  if (!f) {
    MYPROPS_LOG("open storage file in write fail");
    return false;
  }

  bool ok = f.print(payload);
  f.close();
  if (!ok) {
    MYPROPS_LOG("write payload fail");
    return false;
  }

  cachedPayload = payload;
  return true;
}

bool MyDeviceProperties::loadFromStorage() {
  if (!ensureStorageReady()) {
    return false;
  }

  File f = LittleFS.open(storagePath, "r");
  if (!f) {
    doc.clear();
    MYPROPS_LOG("storage file missing");
    return false;
  }

  String payload = f.readString();
  f.close();
  payload.trim();

  if (payload.isEmpty()) {
    doc.clear();
    MYPROPS_LOG("storage payload empty");
    return false;
  }

  DeserializationError err = deserializeJson(doc, payload);
  if (err) {
    doc.clear();
    MYPROPS_LOG("storage parse fail");
    return false;
  }

  cachedPayload = payload;
  MYPROPS_LOG("storage payload loaded");
  return true;
}

bool MyDeviceProperties::fetchAndStoreIfChanged() {
  if (!WiFi.isConnected()) {
    MYPROPS_LOG("wifi down");
    return false;
  }

  String url = buildUrl();
  MYPROPS_LOGF("get %s\n", url.c_str());
  HTTPClient http;
#ifdef USE_TLS
  if (verifyCert) {
#ifdef ESP8266
    client->setBufferSizes(512, 264);
#endif
  }
#endif

  bool httpConnected = http.begin(*client, url);
  if (!httpConnected) {
    http.end();
    MYPROPS_LOG("http begin fail");
    return false;
  }

  addDeviceHeaders(http);
  int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK) {
    http.end();
    MYPROPS_LOGF("http code %d\n", httpCode);
    return false;
  }

  String payload = http.getString();
  http.end();
  payload.trim();

  MYPROPS_LOGF("payload %s\n", payload.c_str());

  if (payload == cachedPayload) {
    MYPROPS_LOG("unchanged");
    return true;
  }

  DeserializationError err = deserializeJson(doc, payload);
  if (err) {
    MYPROPS_LOG("json parse fail");
    return false;
  }

  return saveToStorage(payload);
}
