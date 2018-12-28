#pragma once

#ifndef CONSTS_HEADER_FILE_H
#define CONSTS_HEADER_FILE_H

#include <string.h>

#define APP_VERSION "0.1"
#define FOREGROUND_WHITE (FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN)
#define FOREGROUND_YELLOW 14
#define SIP_ACCOUNT_NAME(username, domain)  username + "@" + domain

#define UNUSED_ARG(arg)  (void)arg

#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define STRING_REMOVE_CHAR(str, ch) str.erase(std::remove(str.begin(), str.end(), ch), str.end())

#define REMOTE_CONFIG_URL "http://10.47.2.22/config-store/tinyphone-config.json"

#endif