# TimeSyncManager

Libreria leggera per sincronizzare l'orario NTP in modo idempotente.

## Obiettivo
- Evitare chiamate duplicate a `configTime` quando piu' moduli (es. EspOTA e DeviceSetupManager) richiedono la sync nello stesso boot.
- Ridurre il consumo batteria con timeout breve e ritorno rapido se l'orario e' gia' valido.

## API
- `ensureTimeSynced(timeoutMs, pollMs)`
  - Ritorna subito `true` se la sync e' gia' stata fatta nello stesso boot o se l'ora e' gia' valida.
  - Altrimenti avvia NTP e attende fino al timeout.
- `isTimeValid(minValidEpoch)`
- `resetBootSyncFlag()`
- `setTimezone("...")`

## Timezone default
- Default (Roma): `CET-1CEST,M3.5.0/2,M10.5.0/3`
- Se vuoi UTC puro usa: `UTC0`

## Esempio
```cpp
#include "TimeSyncManager.h"

TimeSyncManager timeSync; // default Roma
// timeSync.setTimezone("UTC0"); // opzionale

void setup() {
  bool ok = timeSync.ensureTimeSynced(2000, 100);
  // if (!ok) puoi continuare in fail-fast o dormire per risparmiare batteria
}
```

## Nota integrazione
Usala da entrambi i flussi (EspOTA e DeviceSetupManager): non fara' doppia sync nello stesso boot.
