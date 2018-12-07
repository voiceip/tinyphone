#pragma once

#ifndef CONSTS_HEADER_FILE_H
#define CONSTS_HEADER_FILE_H

#include "stdafx.h"
#include <string.h>

#define FOREGROUND_WHITE (FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN)
#define DEFAULT_UA_STRING "TinyPhone Pjsua2 v0"
#define SIP_ACCOUNT_NAME(username, domain)  username + "@" + domain


#define SIP_REG_DURATION 180
#define SIP_REG_RETRY_INTERVAL 30
#define SIP_REG_FIRST_RETRY_INTERVAL 15

#define SIP_ALLOWED_AUDIO_CODECS "PCMA/8000/1 PCMU/8000/1"

#define UNUSED_ARG(arg)  (void)arg

#define PJSUA_MAX_CALLS	4

#ifdef _DEBUG
#define PJSUA_MAX_ACC	3
#else
#define PJSUA_MAX_ACC	1
#endif

#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define STRING_REMOVE_CHAR(str, ch) str.erase(std::remove(str.begin(), str.end(), ch), str.end())

#endif