#include "TimeSyncManager.h"

bool TimeSyncManager::syncedThisBoot = false;
String TimeSyncManager::syncedTimezoneThisBoot = "";

TimeSyncManager::TimeSyncManager(const char *tz, const char *ntp1,
                                 const char *ntp2)
    : tz((tz && tz[0] != '\0') ? tz : "CET-1CEST,M3.5.0/2,M10.5.0/3"),
      ntp1((ntp1 && ntp1[0] != '\0') ? ntp1 : "europe.pool.ntp.org"),
      ntp2((ntp2 && ntp2[0] != '\0') ? ntp2 : "pool.ntp.org") {}

void TimeSyncManager::setTimezone(const char *tzValue) {
  String nextTz = (tzValue && tzValue[0] != '\0')
                      ? String(tzValue)
                      : String("CET-1CEST,M3.5.0/2,M10.5.0/3");
  if (tz != nextTz) {
    tz = nextTz;
    resetBootSyncFlag();
  }
}

bool TimeSyncManager::isTimeValid(time_t minValidEpoch) {
  return time(nullptr) >= minValidEpoch;
}

void TimeSyncManager::resetBootSyncFlag() {
  syncedThisBoot = false;
  syncedTimezoneThisBoot = "";
}

bool TimeSyncManager::ensureTimeSynced(uint32_t timeoutMs, uint32_t pollMs) {
  if (syncedThisBoot && syncedTimezoneThisBoot == tz) {
    return true;
  }

#if defined(ESP8266)
  configTime(tz.c_str(), ntp1.c_str(), ntp2.c_str());
#elif defined(ESP32)
  configTzTime(tz.c_str(), ntp1.c_str(), ntp2.c_str());
#else
  return false;
#endif

  if (isTimeValid()) {
    syncedThisBoot = true;
    syncedTimezoneThisBoot = tz;
    return true;
  }

  unsigned long startMs = millis();
  while ((millis() - startMs) < timeoutMs) {
    if (isTimeValid()) {
      syncedThisBoot = true;
      syncedTimezoneThisBoot = tz;
      return true;
    }
    delay(pollMs);
    yield();
  }

  return false;
}
