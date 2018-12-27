#include "stdafx.h"
#include "config.h"
#include "net.h"

namespace tp {

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
		false
	};

	void InitConfig() {
		tp::HttpResponse remoteConfig = url_get_contents(REMOTE_CONFIG_URL);
		if (remoteConfig.code / 100 != 2) {
			std::string message = "Failed to fetch Remote Config!";
			if (remoteConfig.error != "")
				message += "\nERROR:" + remoteConfig.error;
			if (remoteConfig.code >= -1) {
				message += "\nReturn Code:" + std::to_string(remoteConfig.code);
			}
			tp::DisplayError(message);
			exit(1);
		}
		std::cout << "======= Remote Config ======" << std::endl << remoteConfig.body << std::endl;

		try {
			auto j = nlohmann::json::parse(remoteConfig.body);
			ApplicationConfig = j.get<tp::appConfig>();

			nlohmann::json k = ApplicationConfig;

			std::cout << "======= Application Config ======" << std::endl << k.dump(4) << std::endl;


		}
		catch (...) {
			tp::DisplayError("Failed Parsing Remote Config! Please contact support.");
			exit(1);
		}
	}
}