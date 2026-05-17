#pragma once

#include <Arduino.h>
#include <time.h>

class TimeSyncManager {
public:
  TimeSyncManager(
      const char *tz = "CET-1CEST,M3.5.0/2,M10.5.0/3",
      const char *ntp1 = "europe.pool.ntp.org",
      const char *ntp2 = "pool.ntp.org",
      uint32_t updateIntervalMinutes = 24UL * 60UL);
  explicit TimeSyncManager(uint32_t updateIntervalMinutes);

  void setTimezone(const char *tzValue);

  // Idempotent per boot: if time is already valid or already synced in this
  // boot, it returns immediately.
  bool ensureTimeSynced(uint32_t timeoutMs = 2500, uint32_t pollMs = 100);

  // Checks if the periodic update interval has elapsed and syncs time again.
  // Useful in loop(); returns immediately when no update is due.
  bool updateTime(uint32_t timeoutMs = 2500, uint32_t pollMs = 100);

  // Checks if system time is plausible (default: 2024-01-01 UTC).
  static bool isTimeValid(time_t minValidEpoch = 1704067200);

  // Optional utility for tests or custom flows.
  static void resetBootSyncFlag();

private:
  String tz;
  String ntp1;
  String ntp2;
  uint32_t updateIntervalMs;
  unsigned long lastUpdateMs;
  bool hasLastUpdate;

  bool syncTime(uint32_t timeoutMs, uint32_t pollMs, bool force);
  void markSyncedNow();

  static bool syncedThisBoot;
  static String syncedTimezoneThisBoot;
};
