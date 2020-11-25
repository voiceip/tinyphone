#pragma once

#ifndef APP_HEADER_FILE_H
#define APP_HEADER_FILE_H

#include <string.h>
#include <vector>
#include "phone.h"

namespace tp {

    extern tm* launchDate;
    extern string sipLogFile;
    extern string httpLogFile;
    extern TinyPhoneHttpServer* tpHttpServer;

    void StartApp();
    void StopApp();
    TinyPhone* GetPhone();
    std::vector<std::string> GetLocalDNSServers();
}

#endif
