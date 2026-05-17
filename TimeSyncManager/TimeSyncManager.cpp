#include "TimeSyncManager.h"

bool TimeSyncManager::syncedThisBoot = false;
String TimeSyncManager::syncedTimezoneThisBoot = "";

TimeSyncManager::TimeSyncManager(const char *tz, const char *ntp1,
                                 const char *ntp2,
                                 uint32_t updateIntervalMinutes)
    : tz((tz && tz[0] != '\0') ? tz : "CET-1CEST,M3.5.0/2,M10.5.0/3"),
      ntp1((ntp1 && ntp1[0] != '\0') ? ntp1 : "europe.pool.ntp.org"),
      ntp2((ntp2 && ntp2[0] != '\0') ? ntp2 : "pool.ntp.org"),
      updateIntervalMs((updateIntervalMinutes > 0 ? updateIntervalMinutes
                                                   : 24UL * 60UL) *
                       60000UL),
      lastUpdateMs(0), hasLastUpdate(false) {}

TimeSyncManager::TimeSyncManager(uint32_t updateIntervalMinutes)
    : TimeSyncManager("CET-1CEST,M3.5.0/2,M10.5.0/3", "europe.pool.ntp.org",
                      "pool.ntp.org", updateIntervalMinutes) {}

void TimeSyncManager::setTimezone(const char *tzValue) {
  String nextTz = (tzValue && tzValue[0] != '\0')
                      ? String(tzValue)
                      : String("CET-1CEST,M3.5.0/2,M10.5.0/3");
  if (tz != nextTz) {
    tz = nextTz;
    resetBootSyncFlag();
    lastUpdateMs = 0;
    hasLastUpdate = false;
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
  return syncTime(timeoutMs, pollMs, false);
}

bool TimeSyncManager::updateTime(uint32_t timeoutMs, uint32_t pollMs) {
  unsigned long nowMs = millis();
  if (hasLastUpdate && (nowMs - lastUpdateMs) < updateIntervalMs) {
    return true;
  }

  lastUpdateMs = nowMs;
  hasLastUpdate = true;
  return syncTime(timeoutMs, pollMs, true);
}

bool TimeSyncManager::syncTime(uint32_t timeoutMs, uint32_t pollMs,
                               bool force) {
  if (!force && syncedThisBoot && syncedTimezoneThisBoot == tz) {
    markSyncedNow();
    return true;
  }

  if (!force && isTimeValid()) {
    syncedThisBoot = true;
    syncedTimezoneThisBoot = tz;
    markSyncedNow();
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
    markSyncedNow();
    return true;
  }

  unsigned long startMs = millis();
  while ((millis() - startMs) < timeoutMs) {
    if (isTimeValid()) {
      syncedThisBoot = true;
      syncedTimezoneThisBoot = tz;
      markSyncedNow();
      return true;
    }
    delay(pollMs);
    yield();
  }

  return false;
}

void TimeSyncManager::markSyncedNow() {
  lastUpdateMs = millis();
  hasLastUpdate = true;
}
