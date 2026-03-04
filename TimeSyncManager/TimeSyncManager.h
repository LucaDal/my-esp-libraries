#pragma once

#include <Arduino.h>
#include <time.h>

class TimeSyncManager {
public:
  TimeSyncManager(
      const char *tz = "CET-1CEST,M3.5.0/2,M10.5.0/3",
      const char *ntp1 = "europe.pool.ntp.org",
      const char *ntp2 = "pool.ntp.org");

  void setTimezone(const char *tzValue);

  // Idempotent per boot: if time is already valid or already synced in this
  // boot, it returns immediately.
  bool ensureTimeSynced(uint32_t timeoutMs = 2500, uint32_t pollMs = 100);

  // Checks if system time is plausible (default: 2024-01-01 UTC).
  static bool isTimeValid(time_t minValidEpoch = 1704067200);

  // Optional utility for tests or custom flows.
  static void resetBootSyncFlag();

private:
  String tz;
  String ntp1;
  String ntp2;

  static bool syncedThisBoot;
  static String syncedTimezoneThisBoot;
};
