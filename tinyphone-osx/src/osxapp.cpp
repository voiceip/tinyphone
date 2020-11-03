//
//  osxapp.cpp
//  Tinyphone
//
//  Created by Kinshuk  Bairagi on 03/11/20.
//  Copyright © 2020 Kinshuk  Bairagi. All rights reserved.
//

#include <stdio.h>

#include <ctime>

#include "server.h"
#include "utils.h"
#include "net.h"
#include "consts.h"
#include "config.h"
#include "log.h"

#include <algorithm>

using namespace std;
using namespace pj;
using namespace tp;

namespace tp {
    string sipLogFile;
    string httpLogFile;
    tm* launchDate;
    TinyPhoneHttpServer* tpHttpServer;
}
