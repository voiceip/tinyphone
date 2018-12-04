#pragma once

#ifndef CONSTS_HEADER_FILE_H
#define CONSTS_HEADER_FILE_H

#include "stdafx.h"

#define FOREGROUND_WHITE (FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN)
#define DEFAULT_UA_STRING "TinyPhone Pjsua2 v0"
#define SIP_ACCOUNT_NAME(username, domain)  username + "@" + domain


#define SIP_REG_DURATION 180
#define SIP_REG_RETRY_INTERVAL 30
#define SIP_REG_FIRST_RETRY_INTERVAL 15

#define SIP_ALLOWED_AUDIO_CODECS "PCMA/8000/1 PCMU/8000/1"


#endif