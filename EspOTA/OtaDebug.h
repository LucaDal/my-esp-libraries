#ifndef OTA_DEBUG_H
#define OTA_DEBUG_H

#include <CommonDebug.h>

#define OTA_LOG(msg) DBG_LOG("*OTA:", msg)
#define OTA_LOGF(fmt, ...) DBG_LOGF("*OTA:", fmt, ##__VA_ARGS__)

#endif
