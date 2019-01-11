#include "stdafx.h"
#include "config.h"
#include "net.h"
#include "utils.h"

#define SIP_REG_DURATION 180
#define SIP_REG_RETRY_INTERVAL 30
#define SIP_REG_FIRST_RETRY_INTERVAL 15
#define SIP_ALLOWED_AUDIO_CODECS "PCMA/8000/1 PCMU/8000/1"
#define DEFAULT_UA_PREFIX_STRING "TinyPhone Pjsua2 v" 

#define ALLOW_OFFLINE_CONFIG true
#define LOCAL_CONFIG_FILE "config.json"

namespace tp {

	auto _default_codes = splitString(SIP_ALLOWED_AUDIO_CODECS, ' ');

	appConfig ApplicationConfig = {
		PJSIP_TRANSPORT_UDP,
		SIP_REG_DURATION,
		SIP_REG_DURATION / 2,
		SIP_REG_RETRY_INTERVAL,
		SIP_REG_FIRST_RETRY_INTERVAL,
		false,
		DEFAULT_UA_PREFIX_STRING,
		SIP_MAX_CALLS,
		SIP_MAX_ACC,
		_default_codes,
		DEFUALT_PJ_LOG_LEVEL,
		false,
		false,
		{ "sound", "usb" , "headphone", "audio" , "microphone" , "speakers" },
		"some-random-security-code"
	};

	void InitConfig() {
		tp::HttpResponse remoteConfig = url_get_contents(REMOTE_CONFIG_URL);
		std::string jsonConfig;
		std::string message;
		if (remoteConfig.code / 100 != 2) {
			message = "Failed to fetch Remote Config!";
			if (remoteConfig.error != "")
				message += "\nERROR:" + remoteConfig.error;
			if (remoteConfig.code >= -1) {
				message += "\nReturn Code:" + std::to_string(remoteConfig.code);
			}

#ifndef ALLOW_OFFLINE_CONFIG
			tp::DisplayError(message);
			exit(1);
#endif // ALLOW_OFFLINE_CONFIG
		}
		else {
			jsonConfig = remoteConfig.body;
		}

#ifdef ALLOW_OFFLINE_CONFIG
		if(file_exists(LOCAL_CONFIG_FILE)){
			jsonConfig = file_get_contents(LOCAL_CONFIG_FILE);
		} else if (jsonConfig.size() == 0 ){
			message += "\nLocal Config Fallback also failed.";
			tp::DisplayError(message);
			exit(1);
		}
#endif // ALLOW_OFFLINE_CONFIG

		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		try {
			auto j = nlohmann::json::parse(jsonConfig);
			ApplicationConfig = j.get<tp::appConfig>();

			nlohmann::json k = ApplicationConfig;

			SetConsoleTextAttribute(hConsole, FOREGROUND_YELLOW);
			std::cout << "======= Application Config ======" << std::endl << k.dump(4) << std::endl;
		}
		catch (...) {
			SetConsoleTextAttribute(hConsole, FOREGROUND_YELLOW);
			std::cout << "======= Remote Config ======" << std::endl << jsonConfig << std::endl;

			tp::DisplayError("Failed Parsing Config! Please contact support.");
			exit(1);
		}
		SetConsoleTextAttribute(hConsole, FOREGROUND_WHITE);

	}
}