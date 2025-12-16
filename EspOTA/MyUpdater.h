#ifndef MYUPDATER_H
#define MYUPDATER_H

#include "MyUpdater.h"
#include "CommonOta.h"

#ifdef ESP8266
    #include <ESP8266httpUpdate.h>
#endif
#ifdef ESP32
    #include <HTTPUpdate.h>
#endif

class MyUpdater{
    public:
        MyUpdater(String md5Checksum);
        bool startUpdate(HTTPClient &https, String currentFirmwareVersion);
};

#endif
