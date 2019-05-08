#include "stdafx.h"
#include "config.h"
#include "net.h"
#include "utils.h"
#include <boost/format.hpp> 

#define ALLOW_OFFLINE_CONFIG true
#define LOCAL_CONFIG_FILE "config.json"

namespace tp {

	appConfig ApplicationConfig;

	void InitConfig() {
		tp::HttpResponse remoteConfig = url_get_contents(REMOTE_CONFIG_URL);
		std::string jsonConfig;
		std::string message;

		//for (auto i = remoteConfig.headers.begin(); i != remoteConfig.headers.end(); ++i)
		//	std::cout << i->first << ":" << i->second << ' ' << std::endl;

		auto contentType = std::find_if(remoteConfig.headers.begin(), remoteConfig.headers.end(), 
			[](const std::pair<std::string, std::string>& element) { return element.first == "Content-Type"; });

		if (remoteConfig.code / 100 != 2 || (contentType != remoteConfig.headers.end() && contentType->second != "application/json" )) {
			//Try Secondary Location
			std::string productVersion;
			#ifdef _DEBUG
			productVersion = "HEAD";
			#else
			GetProductVersion(productVersion);
			productVersion = "v" + productVersion;
			#endif

			std::string url = str(boost::format(REMOTE_CONFIG_URL_SECONDARY) % (productVersion));
			std::cout << "Config Load From Secondary : " << url << std::endl;
			remoteConfig = url_get_contents(url);
		}

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
			std::cout << "Config Override From Local File" << std::endl;
		} else if (jsonConfig.size() == 0 ){
			message += "\nERROR: Local Config Fallback also failed.";
			tp::DisplayError(message, OPS::SYNC);
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

			tp::DisplayError("Failed Parsing Config! Please contact support.", OPS::SYNC);
			exit(1);
		}
		SetConsoleTextAttribute(hConsole, FOREGROUND_WHITE);

	}
}