#pragma once

#ifndef CONFIG_HEADER_FILE_H
#define CONFIG_HEADER_FILE_H

#include <iostream>
#include <string>
#include "json.h"
#include "consts.h"
#include "utils.h"
#include "net.h"

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

namespace tp {

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
		j = nlohmann::json{
			{"transport", p.transport },
			{"timeoutSec", p.timeoutSec },
			{"retryIntervalSec", p.retryIntervalSec },
			{"firstRetryIntervalSec", p.firstRetryIntervalSec },
			{"dropCallsOnFail", p.dropCallsOnFail },
			{"uaPrefix", p.uaPrefix },
			{"maxCalls", p.maxCalls },
			{"maxAccounts", p.maxAccounts },
			{"audioCodecs", p.audioCodecs },
			{"pjLogLevel", p.pjLogLevel },
		};
    }

   static void from_json(const nlohmann::json& j, appConfig& p) {
		j.at("transport").get_to(p.transport);
		j.at("timeoutSec").get_to(p.timeoutSec);
		j.at("retryIntervalSec").get_to(p.retryIntervalSec);
		j.at("firstRetryIntervalSec").get_to(p.firstRetryIntervalSec);
		j.at("dropCallsOnFail").get_to(p.dropCallsOnFail);
		j.at("uaPrefix").get_to(p.uaPrefix);
		j.at("maxCalls").get_to(p.maxCalls);
		j.at("maxAccounts").get_to(p.maxAccounts);
		j.at("audioCodecs").get_to(p.audioCodecs);
		j.at("pjLogLevel").get_to(p.pjLogLevel);
    }

   static void InitConfig() {
	   // conversion: ApplicationConfig -> json
	   tp::HttpResponse remoteConfig = file_get_contents(REMOTE_CONFIG_URL);
	   if (remoteConfig.code / 100 != 2) {
		   tp::DisplayError("Failed to fetch Remote Config! Return Code: " + std::to_string(remoteConfig.code));
		   exit(1);
	   }
	   std::cout << "======= Remote Application Config ======" << std::endl << remoteConfig.body << std::endl;

	   try {
		   auto j = nlohmann::json::parse(remoteConfig.body);
		   ApplicationConfig = j.get<tp::appConfig>();
	   }
	   catch (...) {
		   tp::DisplayError("Failed Parsing Remote Config! Please contact support.");
		   exit(1);
	   }
   }
}

#endif