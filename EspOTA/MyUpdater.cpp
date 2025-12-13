#include "MyUpdater.h"

void update_started() {
  OTA_LOG("callback start");
}

void update_finished() {
  OTA_LOG("callback end");
}

void update_progress(int cur, int total) {
  OTA_LOGF("progress %d/%d\n", cur, total);
}

void update_error(int err) {
  OTA_LOGF("error %d\n", err);
}

MyUpdater::MyUpdater(String md5Checksum) {
#if defined(DEBUG) && defined(ESP8266)
  ESPhttpUpdate.onStart(update_started);
  ESPhttpUpdate.onEnd(update_finished);
  ESPhttpUpdate.onProgress(update_progress);
  ESPhttpUpdate.onError(update_error);
#endif
#if defined(DEBUG) && defined(ESP32)
  httpUpdate.onStart(update_started);
  httpUpdate.onEnd(update_finished);
  httpUpdate.onProgress(update_progress);
  httpUpdate.onError(update_error);
#endif
#ifdef ESP8266
  ESPhttpUpdate.rebootOnUpdate(false);
  ESPhttpUpdate.setMD5sum(md5Checksum);
#endif
#ifdef ESP32
  httpUpdate.rebootOnUpdate(false);
#endif
}

bool MyUpdater::startUpdate(HTTPClient &client, String currentFirmwareVersion) {
  t_httpUpdate_return ret;
#ifdef ESP8266
  ret = ESPhttpUpdate.update(client, currentFirmwareVersion);
#endif
#ifdef ESP32
  ret = httpUpdate.update(client, currentFirmwareVersion);
#endif
  switch (ret) {
  case HTTP_UPDATE_FAILED:
    OTA_LOGF("update failed (%d): %s\n",
             #ifdef ESP8266
             ESPhttpUpdate.getLastError(),
             ESPhttpUpdate.getLastErrorString().c_str()
             #elif defined(ESP32)
             httpUpdate.getLastError(),
             httpUpdate.getLastErrorString().c_str()
             #endif
    );
    return false;

  case HTTP_UPDATE_NO_UPDATES:
    OTA_LOG("no updates");
    break;

  case HTTP_UPDATE_OK:
    OTA_LOG("update ok");
    return true;
  }
  return false;
}
