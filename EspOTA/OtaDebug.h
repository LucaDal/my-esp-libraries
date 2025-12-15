#pragma once

#include <CommonDebug.h>

#define OTA_LOG(msg) DBG_LOG("*OTA:", msg)
#define OTA_LOGF(fmt, ...) DBG_LOGF("*OTA:", fmt, ##__VA_ARGS__)
