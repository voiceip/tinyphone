#pragma once

#ifndef PHONE_HEADER_FILE_H
#define PHONE_HEADER_FILE_H

#include "account.h"
#include "channel.h"
#include <algorithm>
#include <string>
#include <stdexcept>
#include "events.h"
#include "utils.h"
#include "config.h"
#include "json.h"

#include <map> 
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

namespace tp {

	struct AccountConfig {
		string username;
		string domain;
		string password;
		string proxy;
	};

	void from_json(const nlohmann::json& j, AccountConfig& p);

	class TinyPhone
	{
		typedef std::map<string, SIPAccount*> map_string_acc;
		map_string_acc accounts;
		pj::Endpoint* endpoint;
		EventStream* eventStream;

	private:
	    std::recursive_mutex add_acc_mutex;

		std::string addTransportSuffix(std::string &str) {
			tp::AddTransportSuffix(str, ApplicationConfig.transport);
			return str;
		}

	public:
		int input_audio_dev = 0, output_audio_dev = 0;

		TinyPhone(pj::Endpoint* ep) {
			endpoint = ep;
		}

		~TinyPhone() {
			pj_thread_auto_register();
			std::cout << "Shutting Down TinyPhone" << std::endl;
			if (endpoint->libGetState() == PJSUA_STATE_RUNNING) {
				HangupAllCalls();
				Logout();
			}
			delete eventStream;
			std::cout << "Shutting Down TinyPhone Complete" << std::endl;
		}

		void CreateEventStream(channel<std::string>* ch) {
			eventStream = new EventStream(ch);
		}

		void SetCodecs();

		void EnableCodec(std::string codec_name, pj_uint8_t priority);

		bool TestAudioDevice();

		void ConfigureAudioDevices();

		void refreshDevices();

		void InitMetricsClient();

		std::vector<SIPAccount *> Accounts();

		bool HasAccounts() ;

		SIPAccount* PrimaryAccount();

		SIPAccount* AccountByName(string name) ;

		void Logout(SIPAccount* acc) throw(pj::Error);

		void Logout() throw(pj::Error) ;

		void EnableAccount(SIPAccount* account) throw (std::exception) ;

		std::future<int> AddAccount(AccountConfig& config) throw (std::exception);

		SIPCall* MakeCall(string uri) throw(pj::Error) ;

		SIPCall* MakeCall(string uri, SIPAccount* account) throw(pj::Error);

		std::vector<SIPCall*> Calls();

		SIPCall* CallById(int call_id) ;

		void HoldOtherCalls(SIPCall* call);

		void Hangup(SIPCall* call);

		void HangupAllCalls();
	};

}
#endif
