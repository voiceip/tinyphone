#include "stdafx.h"
#include <boost/format.hpp> 
#include "config.h"
#include "net.h"
#include "utils.h"
#include "crypt.h"
#include <boost/algorithm/string.hpp>

#ifdef __APPLE__
#include "Tinyphone-C-Interface.h"
#endif

#define ALLOW_OFFLINE_CONFIG true
#define LOCAL_CONFIG_FILE "config.json"

namespace tp {

	appConfig ApplicationConfig;
	EncryptDecrypt crypt("OPKUhsJCvkuxfAcLLf8Dhn079QYw79l9", "0123456789123456");

	void from_json(const nlohmann::json& j, AccountConfig& p) {
		std::string encPass;

		j.at("username").get_to(p.username);
		j.at("domain").get_to(p.domain);
		
		if (j.find("enc_password") != j.end()) {
			j.at("enc_password").get_to(encPass);
			p.password = crypt.Decrypt(encPass);
			//std::cout << "Decrypted Password: " << p.password << std::endl;
		}

		FROM_JSON_OPTIONAL(password);
		FROM_JSON_OPTIONAL(login);
		FROM_JSON_OPTIONAL(proxy);
	}

	void to_json(nlohmann::json& j, const AccountConfig& p) {
		std::string encPass = crypt.Encrypt(p.password);

		j = nlohmann::json{
			{"username", p.username },
			{"login", p.login },
			{"domain", p.domain },
			{"enc_password", encPass },
		};

		if (!p.proxy.empty()) {
			j["proxy"] = p.proxy;
		}
	}

	void InitConfig() {

		std::cout << "Config Load From Primary : " << REMOTE_CONFIG_URL << std::endl;

		tp::HttpResponse remoteConfig = http_get(REMOTE_CONFIG_URL);
		std::string jsonConfig;
		std::string message;
		std::string contentType;

		//for (auto i = remoteConfig.headers.begin(); i != remoteConfig.headers.end(); ++i)
		//	std::cout << i->first << ":" << i->second << ' ' << std::endl;

		auto contentTypeIt = std::find_if(remoteConfig.headers.rbegin(), remoteConfig.headers.rend(),
			[](const std::pair<std::string, std::string>& element) { return boost::iequals(element.first,"Content-Type"); });
		
		if (contentTypeIt != remoteConfig.headers.rend()){
			contentType = contentTypeIt->second;
		}

		if (remoteConfig.code / 100 != 2 || contentType.rfind("application/json", 0) != 0 ) {
			//Try Secondary Location
			message = "ERROR: Failed to fetch Remote Config from Primary!";
			if (remoteConfig.error != "")
				message += "\nERROR:" + remoteConfig.error;
				
			std::cout << "Config Load From Primary Failed :  Response Code " << remoteConfig.code << ", Content-Type: " <<  contentType << std::endl;
			std::string productVersion;
			#ifdef _DEBUG
			productVersion = "master";
			#else
			GetProductVersion(productVersion);
			productVersion = "v" + productVersion;
			#endif

			std::string url = str(boost::format(REMOTE_CONFIG_URL_SECONDARY) % (productVersion));
			std::cout << "Config Load From Secondary : " << url << std::endl;
			remoteConfig = http_get(url);
		}

		if (remoteConfig.code / 100 != 2) {
			message += "\nERROR: Failed to fetch Remote Config from Secondary!";
			if (remoteConfig.error != "")
				message += "\nERROR:" + remoteConfig.error;
			if (remoteConfig.code >= -1) {
				message += "\nReturn Code:" + std::to_string(remoteConfig.code);
			}

#ifndef ALLOW_OFFLINE_CONFIG
			tp::DisplayError(message);
			exit(1);
#endif // NOT_ALLOW_OFFLINE_CONFIG
		}
		else {
			jsonConfig = remoteConfig.body;
		}

#ifdef ALLOW_OFFLINE_CONFIG
        std::string local_file_path = LOCAL_CONFIG_FILE;
#ifdef __APPLE__
        auto apple_config_file = GetAppSupportFilePath(local_file_path.c_str());
        std::cout << "Checking if Local File Exists " << apple_config_file << std::endl;

        if(apple_config_file != nullptr){
            std::string lp(apple_config_file);
            local_file_path.swap(lp);
        }
#endif
		if(file_exists(local_file_path)){
			jsonConfig = file_get_contents(local_file_path);
			std::cout << "Config Override From Local File: " << local_file_path << std::endl;
		} else if (jsonConfig.size() == 0 ){
			message += "\nERROR: Local Config Fallback also failed.";
			tp::DisplayError(message, OPS::SYNC);
			exit(1);
		}
#endif // ALLOW_OFFLINE_CONFIG

		#ifdef _WIN32
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		#endif
		try {
			auto j = nlohmann::json::parse(jsonConfig);
			ApplicationConfig = j.get<tp::appConfig>();

			nlohmann::json k = ApplicationConfig;

			#ifdef _WIN32
			SetConsoleTextAttribute(hConsole, FOREGROUND_YELLOW);
			#endif
			std::cout << "======= Application Config ======" << std::endl << k.dump(4) << std::endl;
		}
		catch (nlohmann::detail::parse_error& ex) {
			#ifdef _WIN32
			SetConsoleTextAttribute(hConsole, FOREGROUND_YELLOW);
			#endif
			std::cout << "======= Remote Config ======" << std::endl << jsonConfig << std::endl;
			std::cout << "Parse Error " << ex.what()  << std::endl;

			tp::DisplayError("Failed Parsing Config! Please contact support.", OPS::SYNC);
			exit(1);
		}
		
		#ifdef _WIN32
		SetConsoleTextAttribute(hConsole, FOREGROUND_WHITE);
		#endif
	}
}
