#pragma once

#ifndef CONFIG_HEADER_FILE_H
#define CONFIG_HEADER_FILE_H

#include <iostream>
#include "json.h"
#include "consts.h"
#include "utils.h"

#define SIP_REG_DURATION 180
#define SIP_REG_RETRY_INTERVAL 30
#define SIP_REG_FIRST_RETRY_INTERVAL 15
#define SIP_ALLOWED_AUDIO_CODECS "PCMA/8000/1 PCMU/8000/1"

#define DEFAULT_UA_PREFIX_STRING "TinyPhone Pjsua2 v" 

#define SIP_MAX_CALLS	4

#ifdef _DEBUG
#define SIP_MAX_ACC	3
#else
#define SIP_MAX_ACC	1
#endif

#define DEFUALT_PJ_LOG_LEVEL 3

namespace pj {

	struct appConfig {
		pjsip_transport_type_e transport;
		int timeoutSec;
		int retryIntervalSec;
		int firstRetryIntervalSec;
		bool dropCallsOnFail;

		std::string uaPrefix;
		size_t maxCalls;
		size_t maxAccounts;
		std::vector<std::string> audioCodecs;

		int pjLogLevel;

		std::string ua(){
			return uaPrefix + APP_VERSION;
		};
	};

	static auto _default_codes = splitString(SIP_ALLOWED_AUDIO_CODECS, ' ');

	static appConfig ApplicationConfig = {
		PJSIP_TRANSPORT_UDP,
		SIP_REG_DURATION,
		SIP_REG_RETRY_INTERVAL,
		SIP_REG_FIRST_RETRY_INTERVAL,
		false,
		DEFAULT_UA_PREFIX_STRING,
		SIP_MAX_CALLS,
		SIP_MAX_ACC,
		_default_codes,
		DEFUALT_PJ_LOG_LEVEL
	};

	static void to_json(nlohmann::json& j, const appConfig& p) {
        //j = nlohmann::json{{"name", p.name}, {"address", p.address}, {"age", p.age}};
    }

   static void from_json(const nlohmann::json& j, appConfig& p) {
        //j.at("name").get_to(p.name);
        
    }
}

#endif